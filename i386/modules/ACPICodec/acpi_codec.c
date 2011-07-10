/*
 * Copyright 2008 mackerintel
 */

/*
 * Copyright (c) 2011 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "acpi.h"
#include "acpidecode.h"
#include "acpicode.h"
#include "efi_tables.h"
#include "fake_efi.h"
#include "acpi_codec.h"
#include "platform.h"
#include "cpu.h"
#include "aml_generator.h"
#include "xml.h"
#include "pci_root.h"
#include "sl.h"

U64 rsd_p;
ACPI_TABLES acpi_tables;

#ifndef DEBUG_ACPI
#define DEBUG_ACPI 0
#endif

#if DEBUG_ACPI==2
#define DBG(x...)  {printf(x); sleep(1);}
#elif DEBUG_ACPI==1
#define DBG(x...)  printf(x)
#else
#define DBG(x...)
#endif

extern EFI_STATUS addConfigurationTable();

extern EFI_GUID gEfiAcpiTableGuid;
extern EFI_GUID gEfiAcpi20TableGuid;

#define MAX_NON_SSDT_TABLE 15
#define MAX_SSDT_TABLE 15 // 15 additional SSDT tables  
#define MAX_ACPI_TABLE MAX_NON_SSDT_TABLE + MAX_SSDT_TABLE

// Security space for SSDT & FACP generation,
// the size can be increased 
// note: the table will not placed in the reserved space if the 'normal' space is not full
#define RESERVED_AERA 3  

#define ACPI_TABLE_LIST_FULL MAX_ACPI_TABLE + RESERVED_AERA + 1

#define ACPI_TABLE_LIST_FULL_NON_RESERVED MAX_ACPI_TABLE + 1


ACPI_TABLE_FADT *
patch_fadt(ACPI_TABLE_FADT *fadt, ACPI_TABLE_DSDT *new_dsdt, bool UpdateFADT);

#define __RES(s, u)												\
inline unsigned u										\
resolve_##s(unsigned u defaultentry, char *str, int base)       \
{																\
unsigned u entry  = defaultentry;							\
if (str && (strcmp(str,"Default") != 0)) {					\
entry  = strtoul((const char *)str, NULL,base);				\
}															\
return entry;												\
}

__RES(pss, long)    
__RES(cst, int)  


ACPI_TABLE_HEADER * get_new_table_in_list(U32 *new_table_list, U32 Signature, U8 *retIndex )
{
	ACPI_TABLE_HEADER **table_array = (ACPI_TABLE_HEADER **) new_table_list;
	U8 index ;
	*retIndex = 0;
	
	for (index = 0; index < (MAX_ACPI_TABLE + RESERVED_AERA); index++) {
		if (*(U32 *) (table_array[index]->Signature) == Signature) {
			*retIndex = index;
			return table_array[index] ;
		}
	}
	return (void*)0ul;
}

U8 get_0ul_index_in_list(U32 *new_table_list, bool reserved )
{
	U8 index ;
	
	U8 maximum = (reserved == true) ? MAX_ACPI_TABLE + RESERVED_AERA : MAX_ACPI_TABLE;
	
	for (index = 0; index < maximum; index++) {
		if (new_table_list[index] == 0ul) {
			return index ;
		}
	}
	return (reserved == true)? ACPI_TABLE_LIST_FULL : ACPI_TABLE_LIST_FULL_NON_RESERVED;
}

/* cparm : This time we check it by the acpi signature */
void sanitize_new_table_list(U32 *new_table_list )
{
	ACPI_TABLE_HEADER **table_array = (ACPI_TABLE_HEADER **) new_table_list;
	U8 index ;
	
	for (index = 0; index < MAX_ACPI_TABLE; index++) {
		U32 current_sig = *(U32 *) (table_array[index]->Signature);
		
		if ((current_sig == NAMESEG(ACPI_SIG_FACS)/* not supported for now */) 
			|| (current_sig == NAMESEG(ACPI_SIG_XSDT)) 
			|| (current_sig == NAMESEG(ACPI_SIG_RSDT)) || (*(volatile U64 *)table_array[index] == NAMESEG64(ACPI_SIG_RSDP)) ) {
			
			void *buf = (void*)new_table_list[index];
			free(buf);
			new_table_list[index] = 0ul ;
		}
	}
}

/* cparm : move all table to kernel memory */
void move_table_list_to_kmem(U32 *new_table_list )
{
	ACPI_TABLE_HEADER **table_array = (ACPI_TABLE_HEADER **) new_table_list;
	U8 index ;
	
	for (index = 0; index < MAX_ACPI_TABLE; index++) {
		if (new_table_list[index] != 0ul) {

			U32 current_sig = *(U32 *) (table_array[index]->Signature);
			if ((current_sig != NAMESEG(ACPI_SIG_FACS)/* not supported for now */) 
				&& (current_sig != NAMESEG(ACPI_SIG_XSDT)) 
				&& (current_sig != NAMESEG(ACPI_SIG_RSDT)) && (*(volatile U64 *)table_array[index] != NAMESEG64(ACPI_SIG_RSDP))
				&& (GetChecksum(table_array[index], table_array[index]->Length) == 0)) {
				
				void *tableAddr=(void*)AllocateKernelMemory(table_array[index]->Length);
				bcopy(table_array[index], tableAddr, table_array[index]->Length);
				new_table_list[index] = 0ul ;
				new_table_list[index] = (U32)tableAddr ;
				
			} else {
				
				void *buf = (void*)new_table_list[index];
				free(buf);
				new_table_list[index] = 0ul ;
			}			
		}
	}
}

ACPI_TABLE_RSDP * gen_alloc_rsdp_v2_from_v1(ACPI_TABLE_RSDP *rsdp )
{
	
	ACPI_TABLE_RSDP * rsdp_conv = (ACPI_TABLE_RSDP *)AllocateKernelMemory(sizeof(ACPI_TABLE_RSDP));
	bzero(rsdp_conv, sizeof(ACPI_TABLE_RSDP));
    memcpy(rsdp_conv, rsdp, ACPI_RSDP_REV0_SIZE);
    
    /* Add/change fields */
    rsdp_conv->Revision = 2; /* ACPI version 3 */
    rsdp_conv->Length = sizeof(ACPI_TABLE_RSDP);
    
    /* Correct checksums */    
    setRsdpchecksum(rsdp_conv);
    setRsdpXchecksum(rsdp_conv);    
    
    return rsdp_conv;
}

ACPI_TABLE_RSDT * gen_alloc_rsdt_from_xsdt(ACPI_TABLE_XSDT *xsdt)
{
    U32 index;
    U32 num_tables;
			
    num_tables= get_num_tables64(xsdt);
    
    ACPI_TABLE_RSDT * rsdt_conv=(ACPI_TABLE_RSDT *)AllocateKernelMemory(sizeof(ACPI_TABLE_HEADER)+(num_tables * 4));
	bzero(rsdt_conv, sizeof(ACPI_TABLE_HEADER)+(num_tables * 4));
    memcpy(&rsdt_conv->Header, &xsdt->Header, sizeof(ACPI_TABLE_HEADER));
    
    rsdt_conv->Header.Signature[0] = 'R';
    rsdt_conv->Header.Signature[1] = 'S';
    rsdt_conv->Header.Signature[2] = 'D';
    rsdt_conv->Header.Signature[3] = 'T';
    rsdt_conv->Header.Length = sizeof(ACPI_TABLE_HEADER)+(num_tables * 4);
	
	ACPI_TABLE_HEADER *table = (ACPI_TABLE_HEADER *) xsdt->TableOffsetEntry;	
    
	for (index=0;index<num_tables;index++)
    {
		if (GetChecksum(table, table->Length) == 0)
		{
			if (((U32) (table->Signature) == NAMESEG(ACPI_SIG_FADT))) {
				ACPI_TABLE_FADT *fadt=(ACPI_TABLE_FADT *)((U32)table);
				
				DBG("Downgrading ACPI V%d FADT to ACPI V1 FADT \n", fadt->Header.Revision);
				ACPI_TABLE_FADT *fadt_conv=(ACPI_TABLE_FADT *)malloc(0x74);
				memcpy(fadt_conv, fadt, 0x74);
				fadt_conv->Header.Length   = 0x74;
				fadt_conv->Header.Revision = 0x01;
				
				SetChecksum(&fadt_conv->Header);
				ACPI_TABLE_FADT *fadt_mod = patch_fadt(fadt_conv, ((ACPI_TABLE_DSDT*)((U32)fadt->XDsdt)), false); 
				if (fadt_mod == (void*)0ul) {
					printf("Error: Failed to patch FADT Table, fallback to fadt original pointer\n");
					fadt_mod = fadt;
				}
				
				rsdt_conv->TableOffsetEntry[index] = ((U32)fadt_mod);
				// Move array pointer to next 64-bit pointer
				table = (ACPI_TABLE_HEADER *) ((U32) table + sizeof(U64));
				continue;
			}
			
			rsdt_conv->TableOffsetEntry[index] = (U32)table;
		}
        // Move array pointer to next 64-bit pointer
        table = (ACPI_TABLE_HEADER *) ((U32) table + sizeof(U64));		
		
    }
    
    SetChecksum(&rsdt_conv->Header);
    
    return rsdt_conv;
}

ACPI_TABLE_XSDT * gen_alloc_xsdt_from_rsdt(ACPI_TABLE_RSDT *rsdt)
{
	
	
	U32 index;
    U32 num_tables;
			
    num_tables= get_num_tables(rsdt);
    
    ACPI_TABLE_XSDT * xsdt_conv=(ACPI_TABLE_XSDT *)AllocateKernelMemory(sizeof(ACPI_TABLE_HEADER)+(num_tables * 8));
	bzero(xsdt_conv, sizeof(ACPI_TABLE_HEADER)+(num_tables * 8));
    memcpy(&xsdt_conv->Header, &rsdt->Header, sizeof(ACPI_TABLE_HEADER));
    
    xsdt_conv->Header.Signature[0] = 'X';
    xsdt_conv->Header.Signature[1] = 'S';
    xsdt_conv->Header.Signature[2] = 'D';
    xsdt_conv->Header.Signature[3] = 'T';
    xsdt_conv->Header.Length = sizeof(ACPI_TABLE_HEADER)+(num_tables * 8);
    
	ACPI_TABLE_HEADER **table_array = (ACPI_TABLE_HEADER **) rsdt->TableOffsetEntry;
	
    for (index=0;index<num_tables;index++)
    {
		if (GetChecksum(table_array[index], table_array[index]->Length) == 0) {
			
			if ((*(U32 *) (table_array[index]->Signature) == NAMESEG(ACPI_SIG_FADT))){
				ACPI_TABLE_FADT *FacpPointer = ((ACPI_TABLE_FADT*)table_array[index]);
				ACPI_TABLE_FADT *fadt_mod = (ACPI_TABLE_FADT *)patch_fadt(FacpPointer,((ACPI_TABLE_DSDT*)FacpPointer->Dsdt),true);
				if (fadt_mod == (void*)0ul) {
					printf("Error: Failed to patch (& update) FADT Table, fallback to original fadt pointer\n");
					fadt_mod = FacpPointer;
				}
				xsdt_conv->TableOffsetEntry[index] = ((U64)((U32)fadt_mod));
				
				continue;
			}
			xsdt_conv->TableOffsetEntry[index] = ((U64)((U32)table_array[index])); 
        }
        
    }
    
    SetChecksum(&xsdt_conv->Header);
    
    return xsdt_conv;
}

void *loadACPITable(char *dirspec, char *filename )
{	
	int fd = -1;
	char acpi_file[512];
    
	DBG("Searching for %s file ...\n", filename);
	// Check booting partition	
    	
	sprintf(acpi_file, "%s%s",dirspec, filename); 
	
	fd=open(acpi_file);
	
	if (fd<0)
	{							
		DBG("Couldn't open ACPI Table: %s\n", acpi_file);
		return (void *)0ul ;				
	}		
	
	void *tableAddr=(void*)malloc(file_size (fd));

	if (tableAddr)
	{
		if (read (fd, tableAddr, file_size (fd))!=file_size (fd))
		{
			printf("Couldn't read table %s\n",acpi_file);
			free (tableAddr);
			close (fd);
			return (void *)0ul ;
		}
		
		close (fd);
		
		ACPI_TABLE_HEADER * header = (ACPI_TABLE_HEADER *)tableAddr;
		
		if (GetChecksum(header, header->Length) == 0) {
			DBG("Found valid ACPI file : %s", filename);
			DBG(", Table %s read and stored at: %x", acpi_file, tableAddr);
			DBG("\n");
			return tableAddr;
		} else {
			printf("Warning : Incorrect cheksum for the file : %s,");
			printf("		  this file will be dropped.\n");
			free(tableAddr);
			return (void*)0ul;
		}		
	}
	
	printf("Couldn't allocate memory for table %s\n", acpi_file);
	close (fd);
	
	return (void *)0ul ;
}

/*
 * License for
 * print_nameseg, generate_cpu_map_from_acpi, sprintf_nameseg .
 *
 Copyright (c) 2010, Intel Corporation
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 * Neither the name of Intel Corporation nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
static U32 pmbase;
static short cpuNamespace;
PROCESSOR_NUMBER_TO_NAMESEG cpu_map[CPU_MAP_LIMIT];
unsigned int cpu_map_count;
int cpu_map_error;

#if DEBUG_ACPI
static void print_nameseg(U32 i)
{
    printf("%c%c%c%c",
           (int)(i & 0x000000ff),
           (int)((i & 0x0000ff00) >> 8),
           (int)((i & 0x00ff0000) >> 16),
           (int)(i >> 24));
}

static void sprintf_nameseg(char* dest, U32 src)
{
    sprintf(dest,"%c%c%c%c",
            (int)(src & 0x000000ff),
            (int)((src & 0x0000ff00) >> 8),
            (int)((src & 0x00ff0000) >> 16),
            (int)(src >> 24));
}
#endif

static int generate_cpu_map_from_acpi(ACPI_TABLE_DSDT * DsdtPointer)
{
    PROCESSOR_NUMBER_TO_NAMESEG *map = cpu_map;
    U32 processor_namespace = 0;
    U32 cpu;
    U8 *current, *end;
    ACPI_TABLE_HEADER *header;
    struct acpi_namespace ns;
		
	if ((cpu_map_error == 1) || (DsdtPointer == (void*)0ul)) 
		return 1;
	else if (cpu_map_count > 0) 
		return 0;
	
    DBG("Attempting to autodetect CPU map from ACPI DSDT; wish me luck\n");	
    
    current = (U8 *) DsdtPointer;
    current = decodeTableHeader(current, &header);
    end = current - sizeof(*header) + header->Length;
    ns.depth = 0;
    acpi_processor_count = 0;
	DBG("* DSDT debug start\n");
    parse_acpi_termlist(&ns, current, end);
	DBG("* DSDT debug end\n");
	
    if (acpi_processor_count > CPU_MAP_LIMIT){
		verbose("Too many processors: found %u processors\n", acpi_processor_count);
        return (cpu_map_error = 1);
	}
    if (acpi_processor_count == 0){
		verbose( "Found no processors in ACPI\n");
        return (cpu_map_error = 1);
	}
    for (cpu = 0; cpu < acpi_processor_count; cpu++) {
        U32 nameseg;
        if (acpi_processors[cpu].pmbase) {
            U32 cpu_pmbase = acpi_processors[cpu].pmbase - 0x10;
            if (pmbase && cpu_pmbase != pmbase){
				verbose("Found inconsistent pmbase addresses in ACPI: 0x%x and 0x%x\n", pmbase, cpu_pmbase);
				return (cpu_map_error = 1);
			}
            pmbase = cpu_pmbase;
        }
        if (acpi_processors[cpu].ns.depth > MAX_SUPPORTED_CPU_NAMESEGS + 1){
			verbose("Processor path too deep: depth %u\n", acpi_processors[cpu].ns.depth);
			return (cpu_map_error = 1);
		}
        if (processor_namespace && acpi_processors[cpu].ns.nameseg[0] != processor_namespace){
			verbose("Processor namespaces inconsistent\n");
			return (cpu_map_error = 1);
		}
        processor_namespace = acpi_processors[cpu].ns.nameseg[0];
        map->acpi_processor_number = acpi_processors[cpu].id;
        map->seg_count = acpi_processors[cpu].ns.depth - 1;
        for (nameseg = 0; nameseg < map->seg_count; nameseg++)
            map->nameseg[nameseg] = acpi_processors[cpu].ns.nameseg[nameseg + 1];
        map++;
    }
    if (!pmbase){
		verbose("No pmbase found in ACPI\n");
		return (cpu_map_error = 1);
	}
    if (processor_namespace == NAMESEG("_PR_"))
        cpuNamespace = CPU_NAMESPACE_PR;
    else if (processor_namespace == NAMESEG("_SB_"))
        cpuNamespace = CPU_NAMESPACE_SB;
    else {
        verbose("Found processors in invalid namespace; not _PR_ or _SB_\n");
		return (cpu_map_error = 1);
	}
    cpu_map_count = map - cpu_map;
	
#if DEBUG_ACPI
	char pns[3];  
	sprintf_nameseg(pns,processor_namespace);
	verbose("Found %d processors in ACPI, pmbase : 0x%x, cpu_map_count : %d, namespace : %s \n",acpi_processor_count, pmbase, cpu_map_count ,pns );
	
    getc();
    U32 i;
    verbose("Found processors name : \n" );
    for ( i = 0; i<cpu_map_count; i++) {			
        char pname[3];  
        sprintf_nameseg(pname, *cpu_map[i].nameseg);
        verbose(" %s ",pname );
        
    }
    verbose("\n");
	getc();
#endif
	
	// TODO: Save the cpu map into the device tree
    return (cpu_map_error = 0);
}

ACPI_TABLE_SSDT *generate_cst_ssdt(ACPI_TABLE_FADT* fadt)
{	
	char ssdt_header[] =
	{
		0x53, 0x53, 0x44, 0x54, 0xE7, 0x00, 0x00, 0x00, /* SSDT.... */
		0x01, 0x17, 0x50, 0x6D, 0x52, 0x65, 0x66, 0x41, /* ..PmRefA */
		0x43, 0x70, 0x75, 0x43, 0x73, 0x74, 0x00, 0x00, /* CpuCst.. */
		0x00, 0x10, 0x00, 0x00, 0x49, 0x4E, 0x54, 0x4C, /* ....INTL */
		0x31, 0x03, 0x10, 0x20 							/* 1.._		*/
	};
	
	char cstate_resource_template[] = 
	{
		0x11, 0x14, 0x0A, 0x11, 0x82, 0x0C, 0x00, 0x7F, 
		0x01, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x79, 0x00
	};
	
	if (Platform->CPU.Vendor != 0x756E6547) {
		verbose ("Not an Intel platform: C-States will not be generated !!!\n");
		return (void *)0ul;
	}
	
	if (fadt == (void *)0ul) {
		verbose ("FACP not exists: C-States will not be generated !!!\n");
		return (void *)0ul;
	}
    
	ACPI_TABLE_DSDT* dsdt = ((fadt->Header.Revision >= 3) && (fadt->XDsdt != 0)) ? (ACPI_TABLE_DSDT*)((U32)fadt->XDsdt):(ACPI_TABLE_DSDT*)fadt->Dsdt;
	if (dsdt == (void *)0ul) {
		verbose ("DSDT not found: C-States will not be generated !!!\n");
		return (void *)0ul;
	}
		
	if (generate_cpu_map_from_acpi(dsdt) == 0) 
	{
		bool c2_enabled = fadt->C2Latency < 100;
		bool c3_enabled = fadt->C3Latency < 1000;
		bool c4_enabled = false;
		
		getBoolForKey(kEnableC4State, &c4_enabled, &bootInfo->bootConfig);
        
		unsigned char cstates_count = 1 + (c2_enabled ? 1 : 0) + ((c3_enabled || c4_enabled) ? 1 : 0);
		char *Lat = NULL, *Pw = NULL, *tmpstr =NULL;
		int base = 16;
		TagPtr personality = XMLCastDict(XMLGetProperty(bootInfo->bootConfig.dictionary, (const char*)"C-States"));
		
		if ((tmpstr = XMLCastString(XMLGetProperty(personality, (const char*)"Base")))) {
			
			int mybase = strtol(tmpstr, NULL, 10);	
			
			if (mybase == 8 || mybase == 10 || mybase == 16 ) 
				base = mybase;									
		}
		
		struct aml_chunk* root = aml_create_node(NULL);
        aml_add_buffer(root, ssdt_header, sizeof(ssdt_header)); // SSDT header
        struct aml_chunk* scop;
		if (cpuNamespace == CPU_NAMESPACE_PR) 
			scop = aml_add_scope(root, "\\_PR_");
		else if (cpuNamespace == CPU_NAMESPACE_SB) 
			scop = aml_add_scope(root, "\\_SB_");
		else 
		{
			aml_destroy_node(root);	
			goto out;
		}
        struct aml_chunk* name = aml_add_name(scop, "CST_");
        struct aml_chunk* pack = aml_add_package(name);
        aml_add_byte(pack, cstates_count);
		
        struct aml_chunk* tmpl = aml_add_package(pack);
        TagPtr match_Status = XMLGetProperty(personality, (const char*)"C1");
        if (match_Status) {	
            Pw   = XMLCastString(XMLGetProperty(match_Status, (const char*)"Power"));
            Lat  = XMLCastString(XMLGetProperty(match_Status, (const char*)"Latency"));
        }
        cstate_resource_template[11] = 0x00; // C1
        aml_add_buffer(tmpl, cstate_resource_template, sizeof(cstate_resource_template));
        aml_add_byte(tmpl, 0x01); // C1
        aml_add_byte(tmpl, (unsigned char)resolve_cst(0x01, Lat, base));  // Latency
        aml_add_word(tmpl, resolve_cst(0x03e8, Pw, base)); // Power
		// C2
		if (c2_enabled) 
		{
			tmpl = aml_add_package(pack);
			Lat = NULL; 
			Pw = NULL;
			match_Status = XMLGetProperty(personality, (const char*)"C2");
			if (match_Status) {	
				Pw   = XMLCastString(XMLGetProperty(match_Status, (const char*)"Power"));
				Lat  = XMLCastString(XMLGetProperty(match_Status, (const char*)"Latency"));
			}
			
			cstate_resource_template[11] = 0x10; // C2
			aml_add_buffer(tmpl, cstate_resource_template, sizeof(cstate_resource_template));
			aml_add_byte(tmpl, 0x02); // C2				
			aml_add_word(tmpl, resolve_cst(fadt->C2Latency, Lat, base));  // Latency
			aml_add_word(tmpl, resolve_cst(0x01f4, Pw, base)); // Power
		}
		
		// C4
		if (c4_enabled) 
		{
			tmpl = aml_add_package(pack);
			Lat = NULL; 
			Pw = NULL;
			match_Status = XMLGetProperty(personality, (const char*)"C4"); 
			if (match_Status) {	
				Pw   = XMLCastString(XMLGetProperty(match_Status, (const char*)"Power"));
				Lat  = XMLCastString(XMLGetProperty(match_Status, (const char*)"Latency"));
			}
			cstate_resource_template[11] = 0x30; // C4
			aml_add_buffer(tmpl, cstate_resource_template, sizeof(cstate_resource_template));
			aml_add_byte(tmpl, 0x04); // C4	
			aml_add_word(tmpl, resolve_cst(fadt->C3Latency / 2, Lat, base));  // TODO: right latency for C4
			aml_add_word(tmpl, resolve_cst(0xfa, Pw, base)); // Power
			
		}		
		// C3
		else if (c3_enabled) 
		{
			tmpl = aml_add_package(pack);
			Lat = NULL; 
			Pw = NULL;
			match_Status = XMLGetProperty(personality, (const char*)"C3"); 
			if (match_Status) {	
				Pw   = XMLCastString(XMLGetProperty(match_Status, (const char*)"Power"));
				Lat  = XMLCastString(XMLGetProperty(match_Status, (const char*)"Latency"));
			}
			cstate_resource_template[11] = 0x20; // C3
			aml_add_buffer(tmpl, cstate_resource_template, sizeof(cstate_resource_template));
			aml_add_byte(tmpl, 0x03); // C3				
			aml_add_word(tmpl, resolve_cst(fadt->C3Latency , Lat, base));  
			aml_add_word(tmpl, resolve_cst(0x015e, Pw, base)); // Power
			
		}
		        
        // Aliaces
        unsigned int i;
        for (i = 0; i < cpu_map_count; i++) 
        {
            char name[9];
            U32 nseg = *(U32*)cpu_map[i].nameseg;
            if (cpuNamespace == CPU_NAMESPACE_PR) {
                sprintf(name, "_PR_%c%c%c%c",
                        (int)(nseg & 0x000000ff),
                        (int)((nseg & 0x0000ff00) >> 8),
                        (int)((nseg & 0x00ff0000) >> 16),
                        (int)(nseg >> 24));
            } else if (cpuNamespace == CPU_NAMESPACE_SB) {
                sprintf(name, "_SB_%c%c%c%c",
                        (int)(nseg & 0x000000ff),
                        (int)((nseg & 0x0000ff00) >> 8),
                        (int)((nseg & 0x00ff0000) >> 16),
                        (int)(nseg >> 24));
            } else {
                aml_destroy_node(root);	
                goto out;
            }
            
            scop = aml_add_scope(root, name);
            aml_add_alias(scop, "CST_", "_CST");
        }
		
		aml_calculate_size(root);
		
		ACPI_TABLE_SSDT *ssdt = (ACPI_TABLE_SSDT *)AllocateKernelMemory(root->Size);
        
		aml_write_node(root, (void*)ssdt, 0);
		
		ssdt->Header.Length = root->Size;        
		
		SetChecksum(&ssdt->Header);
        
		aml_destroy_node(root);		
        
		verbose ("SSDT with CPU C-States generated successfully\n");
		
		return ssdt;
	}
	else 
	{
out:
		verbose ("ACPI CPUs not found: C-States will not be generated !!!\n");
	}
    
	return (void *)0ul;
}

ACPI_TABLE_SSDT *generate_pss_ssdt(ACPI_TABLE_DSDT* dsdt)
{		
	
	char ssdt_header[] =
	{
		0x53, 0x53, 0x44, 0x54, 0x7E, 0x00, 0x00, 0x00, /* SSDT.... */
		0x01, 0x6A, 0x50, 0x6D, 0x52, 0x65, 0x66, 0x00, /* ..PmRef. */
		0x43, 0x70, 0x75, 0x50, 0x6D, 0x00, 0x00, 0x00, /* CpuPm... */
		0x00, 0x30, 0x00, 0x00, 0x49, 0x4E, 0x54, 0x4C, /* .0..INTL */
		0x31, 0x03, 0x10, 0x20,							/* 1.._		*/
	};
    
	if (Platform->CPU.Vendor != 0x756E6547) {
		verbose ("Not an Intel platform: P-States will not be generated !!!\n");
		return (void *)0ul;
	}
	
	if (!(Platform->CPU.Features & CPUID_FEATURE_MSR)) {
		verbose ("Unsupported CPU: P-States will not be generated !!!\n");
		return (void *)0ul;
	}
	
	if (dsdt == (void *)0ul) {
		verbose ("DSDT not found: P-States will not be generated !!!\n");
		return (void *)0ul;
	}
		   
	if (generate_cpu_map_from_acpi(dsdt) == 0 ) 
	{
		struct p_state initial, maximum, minimum, p_states[32];
		U8 p_states_count = 0;		
		
		// Retrieving P-States, ported from code by superhai (c)
		switch (Platform->CPU.Family) {
			case 0x06: 
			{
				switch (Platform->CPU.Model) 
				{
					case CPUID_MODEL_DOTHAN: 
					case CPUID_MODEL_YONAH: // Yonah
					case CPUID_MODEL_MEROM: // Merom
					case CPUID_MODEL_PENRYN: // Penryn
					case CPUID_MODEL_ATOM: // Intel Atom (45nm)
					{
						bool cpu_dynamic_fsb = false;
						
						if (rdmsr64(MSR_IA32_EXT_CONFIG) & (1 << 27)) 
						{
							wrmsr64(MSR_IA32_EXT_CONFIG, (rdmsr64(MSR_IA32_EXT_CONFIG) | (1 << 28))); 
							delay(1);
							cpu_dynamic_fsb = rdmsr64(MSR_IA32_EXT_CONFIG) & (1 << 28);
						}
						
						bool cpu_noninteger_bus_ratio = (rdmsr64(MSR_IA32_PERF_STATUS) & (1ULL << 46));
						
						initial.Control = rdmsr64(MSR_IA32_PERF_STATUS);
						
						maximum.Control = ((rdmsr64(MSR_IA32_PERF_STATUS) >> 32) & 0x1F3F) | (0x4000 * cpu_noninteger_bus_ratio);
						maximum.CID = ((maximum.FID & 0x1F) << 1) | cpu_noninteger_bus_ratio;
						
						minimum.FID = ((rdmsr64(MSR_IA32_PERF_STATUS) >> 24) & 0x1F) | (0x80 * cpu_dynamic_fsb);
						minimum.VID = ((rdmsr64(MSR_IA32_PERF_STATUS) >> 48) & 0x3F);
						
						if (minimum.FID == 0) 
						{
							U64 msr;
							U8 i;
							// Probe for lowest fid
							for (i = maximum.FID; i >= 0x6; i--) 
							{
								msr = rdmsr64(MSR_IA32_PERF_CONTROL);
								wrmsr64(MSR_IA32_PERF_CONTROL, (msr & 0xFFFFFFFFFFFF0000ULL) | (i << 8) | minimum.VID);
								intel_waitforsts();
								minimum.FID = (rdmsr64(MSR_IA32_PERF_STATUS) >> 8) & 0x1F; 
								delay(1);
							}
							
							msr = rdmsr64(MSR_IA32_PERF_CONTROL);
							wrmsr64(MSR_IA32_PERF_CONTROL, (msr & 0xFFFFFFFFFFFF0000ULL) | (maximum.FID << 8) | maximum.VID);
							intel_waitforsts();
						}
						
						if (minimum.VID == maximum.VID) 
						{	
							U64 msr;
							U8 i;
							// Probe for lowest vid
							for (i = maximum.VID; i > 0xA; i--) 
							{
								msr = rdmsr64(MSR_IA32_PERF_CONTROL);
								wrmsr64(MSR_IA32_PERF_CONTROL, (msr & 0xFFFFFFFFFFFF0000ULL) | (minimum.FID << 8) | i);
								intel_waitforsts();
								minimum.VID = rdmsr64(MSR_IA32_PERF_STATUS) & 0x3F; 
								delay(1);
							}
							
							msr = rdmsr64(MSR_IA32_PERF_CONTROL);
							wrmsr64(MSR_IA32_PERF_CONTROL, (msr & 0xFFFFFFFFFFFF0000ULL) | (maximum.FID << 8) | maximum.VID);
							intel_waitforsts();
						}
						
						minimum.CID = ((minimum.FID & 0x1F) << 1) >> cpu_dynamic_fsb;
						
						// Sanity check
						if (maximum.CID < minimum.CID) 
						{
							DBG("Insane FID values!");
							p_states_count = 0;
						}
						else
						{
							// Finalize P-States
							// Find how many P-States machine supports
							p_states_count = maximum.CID - minimum.CID + 1;
							
							if (p_states_count > 32) 
								p_states_count = 32;
							
							U8 vidstep;
							U8 i = 0, u, invalid = 0;
							
							vidstep = ((maximum.VID << 2) - (minimum.VID << 2)) / (p_states_count - 1);
							
							for (u = 0; u < p_states_count; u++) 
							{
								i = u - invalid;
								
								p_states[i].CID = maximum.CID - u;
								p_states[i].FID = (p_states[i].CID >> 1);
								
								if (p_states[i].FID < 0x6) 
								{
									if (cpu_dynamic_fsb) 
										p_states[i].FID = (p_states[i].FID << 1) | 0x80;
								} 
								else if (cpu_noninteger_bus_ratio) 
								{
									p_states[i].FID = p_states[i].FID | (0x40 * (p_states[i].CID & 0x1));
								}
								
								if (i && p_states[i].FID == p_states[i-1].FID)
									invalid++;
								
								p_states[i].VID = ((maximum.VID << 2) - (vidstep * u)) >> 2;
								
								U32 multiplier = p_states[i].FID & 0x1f;		// = 0x08
								bool half = p_states[i].FID & 0x40;					// = 0x01
								bool dfsb = p_states[i].FID & 0x80;					// = 0x00
								U32 fsb = Platform->CPU.FSBFrequency / 1000000; // = 400
								U32 halffsb = (fsb + 1) >> 1;					// = 200
								U32 frequency = (multiplier * fsb);			// = 3200
								
								p_states[i].Frequency = (frequency + (half * halffsb)) >> dfsb;	// = 3200 + 200 = 3400
							}
							
							p_states_count -= invalid;
						}
						break;
					} 
					case CPUID_MODEL_FIELDS:
					case CPUID_MODEL_DALES:
					case CPUID_MODEL_DALES_32NM:
					case CPUID_MODEL_NEHALEM: 
					case CPUID_MODEL_NEHALEM_EX:
					case CPUID_MODEL_WESTMERE:
					case CPUID_MODEL_WESTMERE_EX:
					case CPUID_MODEL_SANDYBRIDGE:
                    case CPUID_MODEL_JAKETOWN:
					{		
						maximum.Control = rdmsr64(MSR_IA32_PERF_STATUS) & 0xff; // Seems it always contains maximum multiplier value (with turbo, that's we need)...
						minimum.Control = (rdmsr64(MSR_PLATFORM_INFO) >> 40) & 0xff;
						
						verbose("P-States: min 0x%x, max 0x%x\n", minimum.Control, maximum.Control);
						
						// Sanity check
						if (maximum.Control < minimum.Control) 
						{
							DBG("Insane control values!");
							p_states_count = 0;
						}
						else
						{
							U8 i;
							p_states_count = 0;
							
							for (i = maximum.Control; i >= minimum.Control; i--) 
							{
								p_states[p_states_count].Control = i;
								p_states[p_states_count].CID = p_states[p_states_count].Control << 1;
								p_states[p_states_count].Frequency = (Platform->CPU.FSBFrequency / 1000000) * i;
								p_states_count++;
							}
						}
						
						break;
					}
					default:
						verbose ("Unsupported CPU: P-States will not be generated !!!\n");
						break;
				}
			}
			default:
				break;
		}
		
		// Generating SSDT
		if (p_states_count) 
		{	
			unsigned int i;
			
			struct aml_chunk* root = aml_create_node(NULL);
            aml_add_buffer(root, ssdt_header, sizeof(ssdt_header)); // SSDT header
			struct aml_chunk* scop;
			if (cpuNamespace == CPU_NAMESPACE_PR) 
					scop = aml_add_scope(root, "\\_PR_");
			else if (cpuNamespace == CPU_NAMESPACE_SB) 
					scop = aml_add_scope(root, "\\_SB_");
            else 
			{
                    aml_destroy_node(root);	
                    goto out;
            }				
            struct aml_chunk* name = aml_add_name(scop, "PSS_");
            struct aml_chunk* pack = aml_add_package(name);
			
            U8 minPSratio = (p_states[p_states_count-1].Frequency / (Platform->CPU.FSBFrequency / 10000000 ));
            U8 maxPSratio = (p_states[0].Frequency / (Platform->CPU.FSBFrequency / 10000000 ));
            
            U8 cpu_div = Platform->CPU.CurrDiv;
            U8 cpu_ratio = 0;
			
            if (cpu_div) 								
                cpu_ratio = (Platform->CPU.CurrCoef * 10) + 5;								
            else 								
                cpu_ratio = Platform->CPU.CurrCoef * 10;
            
			
            int user_max_ratio = 0;
            getIntForKey(kMaxRatio, &user_max_ratio, &bootInfo->bootConfig);
            if (user_max_ratio >= minPSratio && maxPSratio >= user_max_ratio) {									
				
                U8 maxcurrdiv = 0, maxcurrcoef = (int)(user_max_ratio / 10);									
				
                U8 maxdiv = user_max_ratio - (maxcurrcoef * 10);
                if (maxdiv > 0)
                    maxcurrdiv = 1;
				
                if (maxcurrdiv) 									
                    cpu_ratio = (maxcurrcoef * 10) + 5;									
                else 									
                    cpu_ratio = maxcurrcoef * 10;																
            }
			
            int user_min_ratio = 0;
            getIntForKey(kMinRatio, &user_min_ratio, &bootInfo->bootConfig);
            if (user_min_ratio >= minPSratio && cpu_ratio >= user_min_ratio) {
				
                U8 mincurrdiv = 0, mincurrcoef = (int)(user_min_ratio / 10);									
				
                U8 mindiv = user_min_ratio - (mincurrcoef * 10);
				
                if (mindiv > 0)
                    mincurrdiv = 1;									
				
                if (mincurrdiv) 									
                    minPSratio = (mincurrcoef * 10) + 5;									
                else 									
                    minPSratio = mincurrcoef * 10;																		
				
            }
			
            if (maxPSratio >= cpu_ratio && cpu_ratio >= minPSratio)	maxPSratio = cpu_ratio;													
			
            TagPtr personality = XMLCastDict(XMLGetProperty(bootInfo->bootConfig.dictionary, (const char*)"P-States"));
            char* MatchStat = 0;
            int dropPSS = 0, Pstatus = 0, base = 16;								
            int expert = 0;/* Default: 0 , mean mixed mode | expert mode : 1 , mean add only p-states found in boot.plist*/
            char *tmpstr = XMLCastString(XMLGetProperty(personality, (const char*)"Mode"));
            
            if (strcmp(tmpstr,"Expert") == 0) {
                p_states_count = (XMLTagCount(personality)) - 1 ; // - 1 = - ("Mode" tag) 										
                expert = 1;
            }
			
            
            if ((tmpstr = XMLCastString(XMLGetProperty(personality, (const char*)"Base")))) {
                
                if (expert) p_states_count--; // -=  ("Base" tag) 
                
                int mybase = strtol(tmpstr, NULL, 10);	
                
                if (mybase == 8 || mybase == 10 || mybase == 16 )
                    base = mybase;									
            }
			
            for (i = 0; i < p_states_count; i++) 
            {									
                sprintf(MatchStat, "%d",i);
                TagPtr match_Status = XMLGetProperty(personality, (const char*)MatchStat); 								   
                
                char *Lat1 = NULL, *clk = NULL, *Pw = NULL, *Lat2 = NULL, *Ctrl = NULL ;
                
                if (match_Status) {												
                    
                    clk  = XMLCastString(XMLGetProperty(match_Status, (const char*)"CoreFreq"));
                    Pw   = XMLCastString(XMLGetProperty(match_Status, (const char*)"Power"));
                    Lat1 = XMLCastString(XMLGetProperty(match_Status, (const char*)"Transition Latency"));
                    Lat2 = XMLCastString(XMLGetProperty(match_Status, (const char*)"Bus Master Latency"));
                    Ctrl = XMLCastString(XMLGetProperty(match_Status, (const char*)"Control"));
                    
                    
                } else if (expert) 
                    continue;
                
                
                unsigned long Frequency  = 0x00000000;
                
                if (!expert) Frequency  = p_states[i].Frequency;
                
                if (clk) 
                    Frequency  = strtoul((const char *)clk, NULL,base);
                
                if (!Frequency || Frequency > p_states[0].Frequency ) continue;
                
                U8 curr_ratio = (Frequency / (Platform->CPU.FSBFrequency / 10000000 ));
                
                if (curr_ratio > maxPSratio || minPSratio > curr_ratio)
                    goto dropPstate;
                
                struct aml_chunk* pstt = aml_add_package(pack);																											
                aml_add_dword(pstt, Frequency); // CoreFreq (in MHz).																		
                aml_add_dword(pstt, resolve_pss(0x00000000, Pw, base)); // Power (in milliWatts)									
                aml_add_dword(pstt, resolve_pss(0x0000000A, Lat1, base)); // Transition Latency (in microseconds).									
                aml_add_dword(pstt, resolve_pss(0x0000000A, Lat2, base)); // Bus Master Latency (in microseconds).									
                unsigned long Control  = 0x00000000;
                if (!expert) Control = p_states[i].Control;									
                aml_add_dword(pstt, resolve_pss(Control, Ctrl, base)); // Control									
                Pstatus++;
                aml_add_dword(pstt, Pstatus); // Status									
                continue;															
            dropPstate:
                
                DBG("state with cpu frequency :%d and ratio :%d will be dropped\n",p_states[i].Frequency,curr_ratio);		
                
                dropPSS++;
                
                
            }			
			
                        
			// Add aliaces
			for (i = 0; i < cpu_map_count; i++) 
			{
				char name[9];
                U32 nseg = *(U32*)cpu_map[i].nameseg;
                if (cpuNamespace == CPU_NAMESPACE_PR) {
                    sprintf(name, "_PR_%c%c%c%c",
                            (int)(nseg & 0x000000ff),
                            (int)((nseg & 0x0000ff00) >> 8),
                            (int)((nseg & 0x00ff0000) >> 16),
                            (int)(nseg >> 24));
                } else if (cpuNamespace == CPU_NAMESPACE_SB) {
                    sprintf(name, "_SB_%c%c%c%c",
                            (int)(nseg & 0x000000ff),
                            (int)((nseg & 0x0000ff00) >> 8),
                            (int)((nseg & 0x00ff0000) >> 16),
                            (int)(nseg >> 24));
                } else {
                    aml_destroy_node(root);	
                    goto out;
                }
                
				scop = aml_add_scope(root, name);
				aml_add_alias(scop, "PSS_", "_PSS");
			}
			
			aml_calculate_size(root);
			
			ACPI_TABLE_SSDT *ssdt = (ACPI_TABLE_SSDT *)AllocateKernelMemory(root->Size);
			
			aml_write_node(root, (void*)ssdt, 0);
			
			ssdt->Header.Length = root->Size;
			
            SetChecksum(&ssdt->Header);
			
			aml_destroy_node(root);			
			
			verbose ("SSDT with CPU P-States generated successfully");
			
			if (dropPSS)
                verbose(", %d P-state(s) dropped",dropPSS);
			
			verbose("\n");
			
			return ssdt;
		}
	}
	else 
	{
out:
		verbose("ACPI CPUs not found: P-States will not be generated !!!\n");
	}
	
	return (void *)0ul;
}
#if UNUSED
ACPI_TABLE_FACS* generate_facs(bool updatefacs )
{
    ACPI_TABLE_FACS* facs_mod=(ACPI_TABLE_FACS *)AllocateKernelMemory(sizeof(ACPI_TABLE_FACS));
    bzero(facs_mod, sizeof(ACPI_TABLE_FACS));
	
	ACPI_TABLE_FACS * FacsPointer =(acpi_tables.FacsPointer64 != (void *)0ul) ? 
	(ACPI_TABLE_FACS *)acpi_tables.FacsPointer64 : (ACPI_TABLE_FACS *)acpi_tables.FacsPointer;
	
    memcpy(facs_mod, FacsPointer , FacsPointer->Length);
    facs_mod->Length = sizeof(ACPI_TABLE_FACS);
		    
	if (FacsPointer->Length < sizeof(ACPI_TABLE_FACS)) {
		facs_mod->FirmwareWakingVector = 0;
		facs_mod->GlobalLock = 0;
		facs_mod->Flags = 0;
	}		
    
    if (updatefacs && FacsPointer->Version < 2) {
		if (FacsPointer->Version > 0) {
			facs_mod->XFirmwareWakingVector = FacsPointer->XFirmwareWakingVector;
		} else {
			facs_mod->XFirmwareWakingVector = (U64)facs_mod->FirmwareWakingVector;
		}

		facs_mod->Version = 2; /* ACPI 1.0: 0, ACPI 2.0/3.0: 1, ACPI 4.0: 2 */
		
	}

    return facs_mod;
}
#endif
  
ACPI_GENERIC_ADDRESS FillGASStruct(U32 Address, U8 Length)
{
	ACPI_GENERIC_ADDRESS TmpGAS;
	
	TmpGAS.SpaceId = 1; /* I/O Address */
	
	if (Address == 0)
	{
		TmpGAS.BitWidth = 0;
	} else {
		TmpGAS.BitWidth = Length * 8;
	}
	
	TmpGAS.BitOffset = 0;
	TmpGAS.AccessWidth = 0; /* Not set for Legacy reasons... */
	TmpGAS.Address = (U64)Address;
	
	return (TmpGAS);
}

ACPI_TABLE_FADT *
patch_fadt(ACPI_TABLE_FADT *fadt, ACPI_TABLE_DSDT *new_dsdt, bool UpdateFADT)
{		
	ACPI_TABLE_FADT *fadt_mod = (void*)0;
	bool fadt_rev2_needed = false;
	bool fix_restart = false;	
	const char * value;	

	// Restart Fix
	if (Platform->CPU.Vendor == 0x756E6547) {	/* Intel */
		fix_restart = true;
		getBoolForKey(kRestartFix, &fix_restart, &bootInfo->bootConfig);
		
	} else {
		verbose ("Not an Intel platform: Restart Fix disabled !!!\n");
	}
	
	if (fix_restart)
		fadt_rev2_needed = true;
			
	// Allocate new fadt table
	if (UpdateFADT)        
	{      
       if (fadt->Header.Length < 0xF4)
	   {
			fadt_mod=(ACPI_TABLE_FADT *)AllocateKernelMemory(0xF4);
		    bzero(fadt_mod, 0xF4);
			memcpy(fadt_mod, fadt, fadt->Header.Length);
			fadt_mod->Header.Length = 0xF4;
		}
		else
		{			
			fadt_mod=(ACPI_TABLE_FADT *)AllocateKernelMemory(fadt->Header.Length);	
			memcpy(fadt_mod, fadt, fadt->Header.Length);
		}		   
				
        
		//fadt_mod->Header.Revision = 0x04; // FADT rev 4
		fadt_mod->ResetRegister = FillGASStruct(0, 0);
		fadt_mod->ResetValue = 0;
		fadt_mod->Reserved4[0] = 0;
		fadt_mod->Reserved4[1] = 0;
		fadt_mod->Reserved4[2] = 0;
        
        fadt_mod->XPm1aEventBlock = FillGASStruct(fadt_mod->Pm1aEventBlock, fadt_mod->Pm1EventLength);
		fadt_mod->XPm1bEventBlock = FillGASStruct(fadt_mod->Pm1bEventBlock, fadt_mod->Pm1EventLength);
		fadt_mod->XPm1aControlBlock = FillGASStruct(fadt_mod->Pm1aControlBlock, fadt_mod->Pm1ControlLength);
		fadt_mod->XPm1bControlBlock = FillGASStruct(fadt_mod->Pm1bControlBlock, fadt_mod->Pm1ControlLength);
		fadt_mod->XPm2ControlBlock = FillGASStruct(fadt_mod->Pm2ControlBlock, fadt_mod->Pm2ControlLength);
		fadt_mod->XPmTimerBlock = FillGASStruct(fadt_mod->PmTimerBlock, fadt_mod->PmTimerLength);
		fadt_mod->XGpe0Block = FillGASStruct(fadt_mod->Gpe0Block, fadt_mod->Gpe0BlockLength);
		fadt_mod->XGpe1Block = FillGASStruct(fadt_mod->Gpe1Block, fadt_mod->Gpe1BlockLength);	
        if (fadt->Header.Revision < 4) {					
			fadt_mod->Header.Revision = 0x04; // FADT rev 4
			verbose("Converted ACPI V%d FADT to ACPI V4 FADT\n", fadt->Header.Revision);

		}
	} else {
		
		if (fadt_rev2_needed)
		{
			if (fadt->Header.Length < 0x84 )
			{
				fadt_mod=(ACPI_TABLE_FADT *)AllocateKernelMemory(0x84);
				bzero(fadt_mod, 0x84);
				memcpy(fadt_mod, fadt, fadt->Header.Length);
				fadt_mod->Header.Length   = 0x84;
			} else {
				fadt_mod=(ACPI_TABLE_FADT *)AllocateKernelMemory(fadt->Header.Length);
				memcpy(fadt_mod, fadt, fadt->Header.Length);
			}			
			
			if (fadt->Header.Revision < 2) {					
				fadt_mod->Header.Revision = 0x02; // FADT rev 2 (ACPI 1.0B MS extensions) 
				verbose("Converted ACPI V%d FADT to ACPI V2 FADT\n", fadt->Header.Revision );
			}
		} else {
			if (fadt->Header.Length < 0x74 )
			{
				fadt_mod=(ACPI_TABLE_FADT *)AllocateKernelMemory(0x74);
				bzero(fadt_mod, 0x74);
				memcpy(fadt_mod, fadt, fadt->Header.Length);
				fadt_mod->Header.Length   = 0x74;
				fadt_mod->Header.Revision = 0x01; 
				verbose("Warning: ACPI FADT length was < 0x74 which is the minimum for the ACPI FADT V1 specification, \n", fadt->Header.Revision );
				verbose("         trying to convert it to Version 1. \n");				

			} else {
				fadt_mod=(ACPI_TABLE_FADT *)AllocateKernelMemory(fadt->Header.Length);
				memcpy(fadt_mod, fadt, fadt->Header.Length);
			}
		}		 		
	}
	bool intelfadtspec = true;
	U8 Type = PMProfileError;
	// Determine system type / PM_Model
	
	// Fix System-type if needed (should never happen)
	if (Platform->Type > MaxSupportedPMProfile)  
	{
		if(fadt_mod->PreferredProfile <= MaxSupportedPMProfile)
			Platform->Type = fadt_mod->PreferredProfile; // get the fadt if correct
		else 
			Platform->Type = 1;		/* Set a fixed value (Desktop) */
	}
	
	// If needed, set System-type from PM_Profile (if valid) else set PM_Profile with a fixed the System-type  
	// Give prior to the FADT pm profile, allow to also control this value with a patched FADT table
	if (fadt_mod->PreferredProfile != Platform->Type) 
	{
		bool val = false;  
		getBoolForKey("PreferInternalProfileDetect", &val, &bootInfo->bootConfig); // if true Give prior to the profile resolved trought the CPU model
		
		val = Platform->CPU.isServer ;
		
		if (fadt_mod->PreferredProfile <= MaxSupportedPMProfile && !val) {
			Platform->Type = fadt_mod->PreferredProfile;
		} else {
			fadt_mod->PreferredProfile = Platform->Type;
		}		
		
	}
	
	// Set PM_Profile and System-type if user wanted this value to be forced
	if ( (value=getStringForKey("SystemType", &bootInfo->bootConfig))!=NULL) {
		if ((Type = (unsigned char) strtoul(value, NULL, 10) ) <= MaxSupportedPMProfile)
		{
			if (fadt_mod->PreferredProfile != Type) {
				verbose("FADT: changing Preferred_PM_Profile from %d to %d\n", fadt->PreferredProfile, Type);
				fadt_mod->PreferredProfile = Platform->Type = Type;
			} else {
				DBG("FADT: Preferred_PM_Profile was already set to %d, no need to be changed\n",Type);
			}
			
		} else printf("Error: system-type must be 0..6. Defaulting to %d !\n", Platform->Type);
	}		
	
	getBoolForKey(KIntelFADT, &intelfadtspec, &bootInfo->bootConfig);
	if ((pmbase == 0) && (cpu_map_error == 0) && (intelfadtspec == true)) 
	{
		ACPI_TABLE_DSDT *DsdtPointer ;
		if (new_dsdt != (void*)0ul) 
			DsdtPointer = new_dsdt;
		else if ((fadt_mod->Header.Revision >= 3) && (fadt_mod->XDsdt != 0ul))
			DsdtPointer = (ACPI_TABLE_DSDT *)((U32)fadt_mod->XDsdt);
		else
			DsdtPointer = (ACPI_TABLE_DSDT *)fadt_mod->Dsdt;
		
		generate_cpu_map_from_acpi(DsdtPointer);
	}	
	
	// Patch DSDT Address if we have loaded a DSDT table
	if(new_dsdt != (void*)0ul)		
		fadt_mod->Dsdt=(U32)new_dsdt;	
	
	fadt_mod->Facs= fadt->Facs;
	//fadt_mod->Facs=(U32)generate_facs(false);
    
	// Patch FADT to fix restart
	if (fadt_mod->Header.Revision >= 2 && fix_restart)
	{		
        fadt_mod->Flags|= 0x400;		
		
		int type = PCI_RESET_TYPE;
		getIntForKey(KResetType, &type, &bootInfo->bootConfig);
		if (type == KEYBOARD_RESET_TYPE) {
			//Azi: keyboard reset; http://forum.voodooprojects.org/index.php/topic,1056.msg9802.html#msg9802
			fadt_mod->ResetRegister = FillGASStruct(0x64, 1);
			fadt_mod->ResetValue = 0xfe;
		} else {
			fadt_mod->ResetRegister = FillGASStruct(0x0cf9, 1);
			fadt_mod->ResetValue = 0x06;
		}
		verbose("FADT: Restart Fix applied (Type : %s) !\n", (type == 0) ? "PCI": "KEYBOARD");
	}
	    
    if (fadt_mod->Header.Revision >= 3) {                
        
        
        if (UpdateFADT) {  
            
			//fadt_mod->XFacs= (U64)((U32)generate_facs(true));
            fadt_mod->XFacs=(U64)fadt->Facs;             
        
        } else {
			fadt_mod->XFacs=(U64)fadt->XFacs;
		}

        
        if(new_dsdt != (void*)0ul)
                fadt_mod->XDsdt=((U64)(U32)new_dsdt);
		else if (UpdateFADT)
				fadt_mod->XDsdt=(U64)fadt_mod->Dsdt;
        
        
        Platform->hardware_signature = ((ACPI_TABLE_FACS *)((U32)fadt_mod->XFacs))->HardwareSignature;
                
    } else {
        
        Platform->hardware_signature = 	((ACPI_TABLE_FACS *)fadt_mod->Facs)->HardwareSignature;
    
    }        
	

	DBG("setting hardware_signature to %x \n",Platform->hardware_signature);    
			
		
    
	if (pmbase && (intelfadtspec == true))
		ProcessFadt(fadt_mod, pmbase); // The checksum correction will be done by ProcessFadt
	else
		SetChecksum(&fadt_mod->Header); // Correct the checksum
    
	return fadt_mod;
}

void process_xsdt (ACPI_TABLE_RSDP *rsdp_mod , U32 *new_table_list){
	TagPtr DropTables_p = XMLCastDict(XMLGetProperty(bootInfo->bootConfig.dictionary, (const char*)"ACPIDropTables"));
	U32 new_table = 0ul;
	U8 new_table_index = 0, table_added = 0;
	ACPI_TABLE_XSDT *xsdt = (void*)0ul, *xsdt_mod = (void*)0ul;
	ACPI_TABLE_RSDT *rsdt_conv	= (void *)0ul;
	
	// FIXME: handle 64-bit address correctly
	
	xsdt=(ACPI_TABLE_XSDT *)acpi_tables.XsdtPointer;
	
	verbose("* Processing XSDT: \n");
	
	DBG("XSDT @%x, Length=%d\n", (U32)xsdt,
		xsdt->Header.Length);
	
	if (xsdt != (void *)0ul)
	{				
		U32 dropoffset=0, index;
		table_added = 0;
		
		xsdt_mod=(ACPI_TABLE_XSDT *)AllocateKernelMemory(xsdt->Header.Length); 
		bzero(xsdt_mod, xsdt->Header.Length);
		memcpy(&xsdt_mod->Header, &xsdt->Header, sizeof(ACPI_TABLE_HEADER));
		
		U32 num_tables=get_num_tables64(xsdt);
		
		ACPI_TABLE_HEADER *table = (ACPI_TABLE_HEADER *) xsdt->TableOffsetEntry;
		
		for (index = 0; index < num_tables; index++) {
			if (GetChecksum(table, table->Length) == 0) {
				
				xsdt_mod->TableOffsetEntry[index-dropoffset]=xsdt->TableOffsetEntry[index];
				
				char tableSig[4];
				bool oem = false;
				char oemOption[OEMOPT_SIZE];
				
				strlcpy(tableSig, (char*)((U32)(xsdt->TableOffsetEntry[index])), sizeof(tableSig)+1);
				
				sprintf(oemOption, "oem%s",tableSig );
				if (getBoolForKey(oemOption, &oem, &bootInfo->bootConfig) && oem) { // This method don't work for DSDT and FACS
					
					if (get_new_table_in_list(new_table_list,((U32) (table->Signature)), &new_table_index) != (void*)0ul) 
						new_table_list[new_table_index] = 0ul; // This way new table will not be added to the new rsdt list !!
					
					continue;
				}
				
				TagPtr match_drop = XMLGetProperty(DropTables_p, (const char*)tableSig);                    
				if ( match_drop ) {
					char *tmp = XMLCastString(match_drop);
					if (strcmp(tmp,"No") != 0) {
						dropoffset++;
						DBG("%s table dropped\n",tableSig);
						continue;
					}
				}
				
				if ((new_table = (U32)get_new_table_in_list(new_table_list,((U32) (table->Signature)), &new_table_index)) != 0ul)
				{
					xsdt_mod->TableOffsetEntry[index-dropoffset]=(U64)new_table;
					new_table_list[new_table_index] = 0ul; // table replaced !!
					continue;
				}
			}
			// Move array pointer to next 64-bit pointer
			table = (ACPI_TABLE_HEADER *) ((U32) table + sizeof(U64));
		}                
		
		
		{
			U8 i;
			for (i = 0; i< (MAX_ACPI_TABLE + RESERVED_AERA); i++) {
				if (new_table_list[i] != 0ul) {
					xsdt_mod->TableOffsetEntry[index-dropoffset]=(U64)new_table_list[i];
					table_added++;
					index++;
				}
			}
		}
		
		// Correct the checksum of XSDT
		xsdt_mod->Header.Length-=8*dropoffset;
		xsdt_mod->Header.Length+=8*table_added;
		
		SetChecksum(&xsdt_mod->Header);
		
		update_rsdp_with_xsdt(rsdp_mod, xsdt_mod);
		
		verbose("* Creating new RSDT from XSDT table\n");
		
		rsdt_conv = (ACPI_TABLE_RSDT *)gen_alloc_rsdt_from_xsdt(xsdt_mod);
		update_rsdp_with_rsdt(rsdp_mod, rsdt_conv);				
		
		
		
	}
	else
	{				
		DBG("About to drop XSDT\n");
		
		/*FIXME: Now we just hope that if MacOS doesn't find XSDT it reverts to RSDT. 
		 * A Better strategy would be to generate
		 */
		
		rsdp_mod->XsdtPhysicalAddress=0xffffffffffffffffLL;
		verbose("XSDT not found or XSDT incorrect\n");
	}

}

void process_rsdt(ACPI_TABLE_RSDP *rsdp_mod , bool gen_xsdt, U32 *new_table_list)
{			
	TagPtr DropTables_p = XMLCastDict(XMLGetProperty(bootInfo->bootConfig.dictionary, (const char*)"ACPIDropTables"));
	U32 new_table = 0ul;
	U8 new_table_index = 0, table_added = 0;
	U32 dropoffset=0, index;
	ACPI_TABLE_RSDT *rsdt		 = (void *)0ul, *rsdt_mod	= (void *)0ul;
	ACPI_TABLE_XSDT *xsdt_conv	 = (void *)0ul;
	
	rsdt=(ACPI_TABLE_RSDT *)acpi_tables.RsdtPointer;
	
	DBG("RSDT @%x, Length %d\n",rsdt, rsdt->Header.Length);	
	
	rsdt_mod=(ACPI_TABLE_RSDT *)AllocateKernelMemory(rsdt->Header.Length);
	bzero(rsdt_mod, rsdt->Header.Length);
	memcpy (&rsdt_mod->Header, &rsdt->Header, sizeof(ACPI_TABLE_HEADER));
	
	U32 num_tables = get_num_tables(rsdt);                        
	
	verbose("* Processing RSDT: \n");			
	ACPI_TABLE_HEADER **table_array = (ACPI_TABLE_HEADER **) rsdt->TableOffsetEntry;
	
	// Compute number of table pointers included in RSDT
	num_tables = get_num_tables(rsdt);
	
	for (index = 0; index < num_tables; index++) {
		if (GetChecksum(table_array[index], table_array[index]->Length) == 0) {
			
			rsdt_mod->TableOffsetEntry[index-dropoffset]=rsdt->TableOffsetEntry[index];
			
			char tableSig[4];
			bool oem = false;
			char oemOption[OEMOPT_SIZE];
			
			strlcpy(tableSig, (char*)(rsdt->TableOffsetEntry[index]), sizeof(tableSig)+1);
			
			sprintf(oemOption, "oem%s",tableSig );
			if (getBoolForKey(oemOption, &oem, &bootInfo->bootConfig) && oem) { // This method don't work for DSDT and FACS
				
				if (get_new_table_in_list(new_table_list,(*(U32 *) (table_array[index]->Signature)), &new_table_index) != (void*)0ul ) 
					new_table_list[new_table_index] = 0ul; // This way new table will not be added to the new rsdt list !!
				
				continue;
			}
			
			TagPtr match_drop = XMLGetProperty(DropTables_p, (const char*)tableSig);
			if ( match_drop ) {
				char *tmp = XMLCastString(match_drop);
				if (strcmp(tmp,"No") != 0) {						
					dropoffset++;
					DBG("%s table dropped\n",tableSig);
					continue;
				}
			}				
			
			if ((new_table = (U32)get_new_table_in_list(new_table_list,(*(U32 *) (table_array[index]->Signature)), &new_table_index)) != 0ul)
			{					
				rsdt_mod->TableOffsetEntry[index-dropoffset]=new_table;
				new_table_list[new_table_index] = 0ul; // table replaced !!
				continue;
			}
			
		}
	}			
	DBG("\n");
	
	{
		U8 i;
		for (i = 0; i< (MAX_ACPI_TABLE + RESERVED_AERA); i++) {
			if (new_table_list[i] != 0ul) {
				rsdt_mod->TableOffsetEntry[index-dropoffset]=new_table_list[i];
				table_added++;
				index++;
			}
		}
	}		
	
	// Correct the checksum of RSDT
	rsdt_mod->Header.Length-=4*dropoffset;
	rsdt_mod->Header.Length+=4*table_added;		
	
	DBG("RSDT: Original checksum %d\n", rsdt_mod->Header.Checksum);			
	
	SetChecksum(&rsdt_mod->Header);
	
	DBG("New checksum %d at %x\n", rsdt_mod->Header.Checksum,rsdt_mod);
	
	update_rsdp_with_rsdt(rsdp_mod, rsdt_mod);
	
	if (gen_xsdt)
	{
		verbose("* Creating new XSDT from RSDT table\n");
		xsdt_conv = (ACPI_TABLE_XSDT *)gen_alloc_xsdt_from_rsdt(rsdt_mod);
		update_rsdp_with_xsdt(rsdp_mod, xsdt_conv); 				
		
	}	
}	



EFI_STATUS setupAcpi(void)
{	
	U8 Revision = 0;
	
    cpu_map_error = 0;
    cpu_map_count = 0;
    pmbase = 0;
    
    EFI_STATUS Status = EFI_ABORTED;
	
	U32 new_table_list[MAX_ACPI_TABLE + RESERVED_AERA]; //max table + reserved aera 
	U8 new_table_index = 0;
	
	ACPI_TABLE_DSDT* DsdtPtr	 = (void *)0ul; // a Pointer to the dsdt table present in fadt_mod
	
	ACPI_TABLE_DSDT *new_dsdt	 = (void *)0ul;	// a Pointer to the dsdt file	
	ACPI_TABLE_FADT *fadt_mod	 = (void *)0ul; // a Pointer to the patched FACP table
	ACPI_TABLE_FADT *fadt_file	 = (void *)0ul; // a Pointer to the (non-patched) fadt file 
	ACPI_TABLE_FADT *FacpPointer = (void *)0ul; // a Pointer to the non-patched FACP table, it can be a file or the FACP table found in the RSDT/XSDT
	ACPI_TABLE_RSDP *rsdp_mod	 = (void *)0ul, *rsdp_conv	= (void *)0ul;
	
	
	U32 rsdplength;
	
	bool update_acpi=false, gen_xsdt=false;
	
	bool gen_csta=false, gen_psta=false, speed_step=false;
	
	bool oem_dsdt=false, oem_fadt=false;
	
	// Find original rsdp        
	if (!FindAcpiTables(&acpi_tables)){
		printf("Error: AcpiCodec Failed to detect ACPI tables.\n");
		getc();
		return EFI_NOT_FOUND;
	}
	
	{
		U8 i;
		
		for (i=0; i<(MAX_ACPI_TABLE + RESERVED_AERA); i++) {
			new_table_list[i] = 0ul;
		}
		bool tmpval;
		
		oem_dsdt=getBoolForKey(kOEMDSDT, &tmpval, &bootInfo->bootConfig)&&tmpval;
		oem_fadt=getBoolForKey(kOEMFADT, &tmpval, &bootInfo->bootConfig)&&tmpval;

		
		gen_csta=getBoolForKey(kGenerateCStates, &tmpval, &bootInfo->bootConfig)&&tmpval;
		gen_psta=getBoolForKey(kGeneratePStates, &tmpval, &bootInfo->bootConfig)&&tmpval;
        
		update_acpi=getBoolForKey(kUpdateACPI, &tmpval, &bootInfo->bootConfig)&&tmpval;
		
		speed_step=getBoolForKey(kSpeedstep, &tmpval, &bootInfo->bootConfig)&&tmpval;
		
	} 
		
	{
		char* name;
		long flags;
		long time;
		long ret = -1;
		U8 i = 0;
		char dirspec[512];
		bool acpidir_found = false;
		
		ret = GetFileInfo("/Extra/", "Acpi", &flags, &time);
		if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeDirectory)) {
			sprintf(dirspec, "/Extra/Acpi/");
			acpidir_found = true;

		} else {
			ret = GetFileInfo("bt(0,0)/Extra/", "Acpi", &flags, &time);
			if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeDirectory)) {
				sprintf(dirspec, "bt(0,0)/Extra/Acpi/");
				acpidir_found = true;

			} 
		}

		if (acpidir_found == true) {
			struct dirstuff* moduleDir = opendir(dirspec);
			while(readdir(moduleDir, (const char**)&name, &flags, &time) >= 0)
			{		
				if((strcmp(&name[strlen(name) - sizeof("aml")], ".aml") == 0) && ((strlen(dirspec)+strlen(name)) < 512))
				{					
					// Some simple verifications to save time in case of those tables simply named as follow:
					if ((strncmp(name, "RSDT", 4) == 0) || (strncmp(name, "rsdt", 4) == 0) ||
						(strncmp(name, "XSDT", 4) == 0) || (strncmp(name, "xsdt", 4) == 0) ||
						(strncmp(name, "RSDP", 4) == 0) || (strncmp(name, "rsdp", 4) == 0))
					{ 
						continue;
					}
					
					if ((strncmp(name, "FACS", 4) == 0) || (strncmp(name, "facs", 4) == 0)) { // For now FACS is not supported
						continue;
					}					
															
					char* tmp = malloc(strlen(name) + 1);
					strcpy(tmp, name);
					
					DBG("* Attempting to load acpi table: %s\n", tmp);			
					if( (new_table_list[i]=(U32)loadACPITable(dirspec,tmp)))
					{
						if (i < MAX_ACPI_TABLE) {
							i++;
						} else {
							break;
						}

						
					} else {
						free(tmp);
					}

				}
#if DEBUG_ACPI
				else 
				{
					DBG("Ignoring %s\n", name);
				}
#endif
				
			}
			
			if (i) {
				//sanitize the new tables list 
				sanitize_new_table_list(new_table_list);
				
				//move to kernel memory 
				move_table_list_to_kmem(new_table_list);
				
				DBG("New ACPI tables Loaded in memory\n");
			}
		}
		
	}			

	// TODO : Add an option for that
    //cpuNamespace = CPU_NAMESPACE_PR; //Default
    
    
	if (speed_step) {
		gen_psta= true;
		gen_csta= true;
	} 		
	    
	
	ACPI_TABLE_RSDP *rsdp=(ACPI_TABLE_RSDP *)acpi_tables.RsdPointer;
	
	if (rsdp == (void*)0ul || (GetChecksum(rsdp, (rsdp->Revision == 0) ? ACPI_RSDP_REV0_SIZE:sizeof(ACPI_TABLE_RSDP)) != 0) ) {
		printf("Error : ACPI RSD PTR Revision %d checksum is incorrect or table not found \n",rsdp->Revision );
		return EFI_UNSUPPORTED;
	}
	
	if ((update_acpi) && (rsdp->Revision == 0))
	{
		
		rsdp_conv = (ACPI_TABLE_RSDP *)gen_alloc_rsdp_v2_from_v1(rsdp);
		if (rsdp_conv != (void *)0ul) {
			gen_xsdt = true;            
			rsdp = rsdp_conv;
			verbose("Converted ACPI RSD PTR Revision 0 to Revision 2\n");
		}
		
	}
	
	Revision = rsdp->Revision  ;
	rsdplength=(Revision == 2)?rsdp->Length:ACPI_RSDP_REV0_SIZE;
	
	DBG("RSDP Revision %d found @%x. Length=%d\n",Revision,rsdp,rsdplength);
	
	/* FIXME: no check that memory allocation succeeded 
	 * Copy and patch RSDP,RSDT, XSDT and FADT
	 * For more info see ACPI Specification pages 110 and following
	 */
	
	if (gen_xsdt)
	{
		rsdp_mod=rsdp_conv;
	} else {
		rsdp_mod=(ACPI_TABLE_RSDP *) AllocateKernelMemory(rsdplength);
		memcpy(rsdp_mod, rsdp, rsdplength);
	}	
	
		
	if ((fadt_file = (ACPI_TABLE_FADT *)get_new_table_in_list(new_table_list, NAMESEG("FACP"), &new_table_index)) != (void *)0ul) {
		
		if (oem_fadt == false)
			FacpPointer = (ACPI_TABLE_FADT *)fadt_file;
		
		new_table_list[new_table_index] = 0ul; // This way, the non-patched table will not be added in our new rsdt/xsdt table list
		
	} else
		FacpPointer = (acpi_tables.FacpPointer64 != (void *)0ul) ? 
		(ACPI_TABLE_FADT *)acpi_tables.FacpPointer64 : (ACPI_TABLE_FADT *)acpi_tables.FacpPointer;			
	
#if DEBUG_ACPI
	if ((FacpPointer != (void *)0ul) || (oem_fadt == false)) {
		printf("FADT found @%x, Length %d\n",FacpPointer, FacpPointer->Header.Length);
		printf("Attempting to patch FADT entry of %s\n",(acpi_tables.FacpPointer64 != (void *)0ul) ? ACPI_SIG_XSDT : ACPI_SIG_RSDT);
	}
#endif	
	
	if ((new_dsdt = (ACPI_TABLE_DSDT *)get_new_table_in_list(new_table_list, NAMESEG("DSDT"), &new_table_index)) != (void*)0ul ) {
		new_table_list[new_table_index] = 0ul; // This way, the DSDT file will not be added in our new rsdt/xsdt table list, and it shouldn't be anyway
	}
	
	if (oem_fadt == false) {
		
		fadt_mod = patch_fadt(FacpPointer, (oem_dsdt == false) ? new_dsdt : (void*)0ul , (acpi_tables.FacpPointer64 != (void *)0ul ));	
		
		
		DsdtPtr = ((fadt_mod->Header.Revision >= 3) && (fadt_mod->XDsdt != 0)) ? (ACPI_TABLE_DSDT*)((U32)fadt_mod->XDsdt):(ACPI_TABLE_DSDT*)fadt_mod->Dsdt;
		
		if (fadt_mod != (void*)0ul) {
			
			U8 empty = get_0ul_index_in_list(new_table_list,true);
			if (empty != ACPI_TABLE_LIST_FULL) {
				new_table_list[empty] = (U32)fadt_mod; // add the patched table to the list
			} else {
				printf("Error: not enought reserved space in the new acpi list for the Patched FACP table,\n ");
				printf("       please increase the RESERVED_AERA\n");
			}			
			
		} else {
			printf("Error: Failed to patch the FADT Table, trying fallback to the FADT original pointer\n");
			fadt_mod = (acpi_tables.FacpPointer64 != (void *)0ul) ? 
			(ACPI_TABLE_FADT *)acpi_tables.FacpPointer64 : (ACPI_TABLE_FADT *)acpi_tables.FacpPointer;
			
			U8 empty = get_0ul_index_in_list(new_table_list,true);
			if (empty != ACPI_TABLE_LIST_FULL) {
				new_table_list[empty] = (U32)fadt_mod; 
			} else {
				printf("Error: not enought reserved space in the new acpi list for the FACP table,\n ");
				printf("       please increase the RESERVED_AERA\n");
			}
		}	
		
		if (oem_dsdt == false) {
			if (generate_cpu_map_from_acpi(DsdtPtr) == 0){
				U8 new_uid = (U8)getPciRootUID();
				
				/* WARNING: THIS METHOD WORK PERFECTLY BUT IT CAN RESULT TO AN INCORRECT CHECKSUM */
				
				if (ProcessDsdt(DsdtPtr, UIDPointer, new_uid)) {
					printf("PCI0 _UID patched to %d in the DSDT table\n", new_uid);
				}				
				
			}
		}
		
		 
	} else {
		
		// here we use the variable fadt_mod only for SSDT Generation
		
		fadt_mod = (acpi_tables.FacpPointer64 != (void *)0ul) ? 
		(ACPI_TABLE_FADT *)acpi_tables.FacpPointer64 : (ACPI_TABLE_FADT *)acpi_tables.FacpPointer;
		
		DsdtPtr = ((fadt_mod->Header.Revision >= 3) && (fadt_mod->XDsdt != 0)) ? (ACPI_TABLE_DSDT*)((U32)fadt_mod->XDsdt)
																										:(ACPI_TABLE_DSDT*)fadt_mod->Dsdt;
	}
	
	if (speed_step || gen_csta || gen_psta) {
		U8 empty = get_0ul_index_in_list(new_table_list, true);
		
		// Generate _CST SSDT
		if ( speed_step || gen_csta)
		{
			if (empty != ACPI_TABLE_LIST_FULL) {
				if (new_table_list[empty] =(U32)generate_cst_ssdt(fadt_mod))
				{
					if (speed_step || gen_psta)
						empty = get_0ul_index_in_list(new_table_list,true);
				}
			} else {
				printf("Error: not enought reserved space in the new acpi list for the _CST SSDT table,\n ");
				printf("       please increase the RESERVED_AERA\n");
			}
		}
		
		
		// Generating _PSS SSDT
		if (speed_step || gen_psta)
		{
			if (empty != ACPI_TABLE_LIST_FULL) {
				
				new_table_list[empty] =(U32)generate_pss_ssdt(DsdtPtr);
				
			} else {
				printf("Error: not enought reserved space in the new acpi list for the _PSS SSDT table,\n ");
				printf("       please increase the RESERVED_AERA\n");
			}
		}
	}

	if ((rsdp_mod != (void *)0ul) && (rsdp_mod->Length >= ACPI_RSDP_REV0_SIZE) ) 
	{
		if ((rsdp_mod->Revision == 0) || (gen_xsdt == true)) {
			process_rsdt(rsdp_mod, gen_xsdt, new_table_list);
			goto out;
		}
		
	} else {
		printf("Error: Incorect ACPI RSD PTR or not found \n");
		return EFI_UNSUPPORTED;
	}

	if ((GetChecksum(rsdp_mod, sizeof(ACPI_TABLE_RSDP)) == 0) &&
		(Revision == 2) &&
		(rsdplength == sizeof(ACPI_TABLE_RSDP)))
	{
		process_xsdt(rsdp_mod, new_table_list);
	} else {
		printf("Warning : ACPI RSD PTR Revision 2 is incorrect, \n");
		printf("          trying to fallback to Revision 1\n");
		if ((rsdp_mod != (void *)0ul) && (rsdp_mod->Length >= ACPI_RSDP_REV0_SIZE) ) 
		{			
			process_rsdt(rsdp_mod, false, new_table_list);
						
		} else {
			printf("Error: Incorect ACPI RSD PTR or not found \n");
			return EFI_UNSUPPORTED;
		}
	}

out:
	// Correct the checksum of RSDP      
	
	DBG("RSDP: Original checksum %d\n", rsdp_mod->Checksum);		
	
	setRsdpchecksum(rsdp_mod);
	
	DBG("New checksum %d\n", rsdp_mod->Checksum);
	
	if (Revision == 2)
	{
		DBG("RSDP: Original extended checksum %d\n", rsdp_mod->ExtendedChecksum);			
		
		setRsdpXchecksum(rsdp_mod);
		
		DBG("New extended checksum %d\n", rsdp_mod->ExtendedChecksum);
		
	}
	
	verbose("ACPI Revision %d successfully patched\n", Revision);
	
	if (Revision == 2)
	{
		rsd_p = ((U64)((U32)rsdp_mod));
		if (rsd_p)
			Status = addConfigurationTable(&gEfiAcpi20TableGuid, &rsd_p, "ACPI_20");
	}
	else
	{
		rsd_p = ((U64)((U32)rsdp_mod));
		if (rsd_p)
			Status = addConfigurationTable(&gEfiAcpiTableGuid, &rsd_p, "ACPI");		

	}
	
#if DEBUG_ACPI
	printf("Press a key to continue... (DEBUG_DSDT)\n");
	getc();
#endif
	return Status;
}
