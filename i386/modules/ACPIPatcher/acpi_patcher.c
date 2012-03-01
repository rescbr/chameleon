/*
 * Copyright 2008 mackerintel
 */

/*
 * Copyright (c) 2010 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#include "libsaio.h"
#include "bootstruct.h"
#include "acpi.h"
#include "efi_tables.h"
#include "fake_efi.h"
#include "acpi_patcher.h"
#include "platform.h"
#include "cpu.h"
#include "aml_generator.h"
#include "xml.h"

uint64_t acpi10_p;
uint64_t acpi20_p;

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

#define EBDA_SEG_ADDR			0x40E
#define EBDA_SEG_LEN			0x400
#define CMOS_BASE_MEMORY		0x15

extern EFI_STATUS addConfigurationTable();

extern EFI_GUID gEfiAcpiTableGuid;
extern EFI_GUID gEfiAcpi20TableGuid;
struct acpi_2_fadt *
patch_fadt(struct acpi_2_fadt *fadt, struct acpi_2_dsdt *new_dsdt, bool UpdateFADT);
struct acpi_2_gas FillGASStruct(uint32_t Address, uint8_t Length);
struct acpi_2_ssdt *generate_pss_ssdt(struct acpi_2_dsdt* dsdt);
struct acpi_2_ssdt *generate_cst_ssdt(struct acpi_2_fadt* fadt);
void get_acpi_cpu_names(unsigned char* dsdt, uint32_t length);
void *loadACPITable(char *key);
void *loadSSDTTable(int ssdt_number);

#define tableSign(table, sgn) (table[0]==sgn[0] && table[1]==sgn[1] && table[2]==sgn[2] && table[3]==sgn[3])

#define CMOS_WRITE_BYTE(x, y)	cmos_write_byte(x, y)
#define CMOS_READ_BYTE(x)	cmos_read_byte(x)

#define RSDP_CHECKSUM_LENGTH 20

/*-
 *	FOR biosacpi_search_rsdp AND biosacpi_find_rsdp
 *
 * Copyright (c) 2001 Michael Smith <msmith@freebsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/sys/boot/i386/libi386/biosacpi.c,v 1.7 2003/08/25 23:28:31 obrien Exp $
 * $DragonFly: src/sys/boot/pc32/libi386/biosacpi.c,v 1.5 2007/01/17 17:31:19 y0netan1 Exp $
 */

static struct acpi_2_rsdp *
biosacpi_search_rsdp(char *base, int length, int rev)
{
    struct acpi_2_rsdp *rsdp;
    int			ofs;
	
    /* search on 16-byte boundaries */
    for (ofs = 0; ofs < length; ofs += 16) {
		rsdp = (struct acpi_2_rsdp*)ptov(base + ofs);
		
		/* compare signature, validate checksum */
		if (!strncmp(rsdp->Signature, ACPI_SIG_RSDP, strlen(ACPI_SIG_RSDP))) {
			
			uint8_t csum = checksum8(rsdp, RSDP_CHECKSUM_LENGTH);
            if(csum == 0)
            {
				/* Only assume this is a 2.0 or better table if the revision is greater than 0
				 * NOTE: ACPI 3.0 spec only seems to say that 1.0 tables have revision 1
				 * and that the current revision is 2.. I am going to assume that rev > 0 is 2.0.
				 */
				
				if((rsdp->Revision > 0) && rev > 0)
				{					
					uint8_t csum2 = checksum8(rsdp, sizeof(struct acpi_2_rsdp));
					if(csum2 == 0)
						return(rsdp);
				}
				
                // Only return the table if it is a true version 1.0 table (Revision 0)
                if((rsdp->Revision == 0) && rev == 0)
                    return(rsdp);
            }
			
		}
    }
    return(NULL);
}

/*
 * Find the RSDP in low memory.  See section 5.2.2 of the ACPI spec.
 */
static struct acpi_2_rsdp *
biosacpi_find_rsdp(int rev)
{
    struct acpi_2_rsdp *rsdp;
    uint16_t		*addr;
	
    /* EBDA is the 1 KB addressed by the 16 bit pointer at 0x40E. */
    addr = (uint16_t *)ptov(EBDA_SEG_ADDR);
    if ((rsdp = biosacpi_search_rsdp((char *)(*addr << 4), EBDA_SEG_LEN, rev)) != NULL)
		return (rsdp);
	
	unsigned mem_lower = ((CMOS_READ_BYTE(CMOS_BASE_MEMORY+1) << 8)
						  | CMOS_READ_BYTE(CMOS_BASE_MEMORY))       << 10;
	
	if ((rsdp = biosacpi_search_rsdp((char *)mem_lower, EBDA_SEG_LEN, rev)) != NULL)
		return (rsdp);
	
	
    /* Check the upper memory BIOS space, 0xe0000 - 0xfffff. */
    if ((rsdp = biosacpi_search_rsdp((char *)0xe0000, 0x20000, rev)) != NULL)
		return (rsdp);
	
    return (NULL);
}



#define __RES(s, u)												\
static inline unsigned u										\
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


static void setchecksum(struct acpi_common_header *header)
{        
    header->Checksum = 0;
    header->Checksum = 256-checksum8(header, header->Length);
}

static void setRsdpchecksum(struct acpi_2_rsdp *rsdp)
{        
    rsdp->Checksum = 0;
    rsdp->Checksum = 256-checksum8(rsdp, 20);
}

static void setRsdpXchecksum(struct acpi_2_rsdp *rsdp)
{        
    rsdp->ExtendedChecksum = 0;
    rsdp->ExtendedChecksum = 256-checksum8(rsdp, rsdp->Length);
}

static struct acpi_2_rsdp * gen_rsdp_v2_from_v1(struct acpi_2_rsdp *rsdp)
{
    struct acpi_2_rsdp * rsdp_conv = (struct acpi_2_rsdp *)AllocateKernelMemory(sizeof(struct acpi_2_rsdp));
    memcpy(rsdp_conv, rsdp, 20);
    
    /* Add/change fields */
    rsdp_conv->Revision = 2; /* ACPI version 3 */
    rsdp_conv->Length = sizeof(struct acpi_2_rsdp);
    
    /* Correct checksums */    
    setRsdpchecksum(rsdp_conv);
    setRsdpXchecksum(rsdp_conv);
    
    rsdp = rsdp_conv;
    
    return rsdp_conv;
}

static struct acpi_2_xsdt * gen_xsdt_from_rsdt(struct acpi_2_rsdt *rsdt)
{
    int i,rsdt_entries_num=(rsdt->Length-sizeof(struct acpi_2_rsdt))/4;
    
    uint32_t *rsdt_entries=(uint32_t *)(rsdt+1);
    
    struct acpi_2_xsdt * xsdt_conv=(struct acpi_2_xsdt *)AllocateKernelMemory(sizeof(struct acpi_2_xsdt)+(rsdt_entries_num * 8));
    memcpy(xsdt_conv, rsdt, sizeof(struct acpi_2_rsdt));
    
    xsdt_conv->Signature[0] = 'X';
    xsdt_conv->Signature[1] = 'S';
    xsdt_conv->Signature[2] = 'D';
    xsdt_conv->Signature[3] = 'T';
    xsdt_conv->Length = sizeof(struct acpi_2_xsdt)+(rsdt_entries_num * 8);
    
    uint64_t *xsdt_conv_entries=(uint64_t *)(xsdt_conv+1);
    
    for (i=0;i<rsdt_entries_num;i++)
    {
        xsdt_conv_entries[i] = (uint64_t)rsdt_entries[i];
    }
    
    setchecksum((struct acpi_common_header *)xsdt_conv);
    
    return xsdt_conv;
}

static void update_rsdp_with_xsdt(struct acpi_2_rsdp *rsdp, struct acpi_2_xsdt *xsdt)
{
    rsdp->XsdtAddress = (uint32_t)xsdt;
    
    setRsdpXchecksum(rsdp);
}

static void update_rsdp_with_rsdt(struct acpi_2_rsdp *rsdp, struct acpi_2_rsdt *rsdt)
{
    rsdp->RsdtAddress = (uint32_t)rsdt;    
    
    setRsdpchecksum(rsdp);
    
}


void *loadSSDTTable(int ssdt_number)
{	
	int fd = -1;
	char dirspec[512];
	char filename[512];
	const char * overriden_pathname=NULL;
	int len=0;
	
	// Check booting partition
	
	// Rek: if user specified a full path name then take it in consideration
	if (getValueForKey(kSSDT, &overriden_pathname, &len,  
					   DEFAULT_BOOT_CONFIG))
	{
		sprintf(filename, "%s-%d.aml", overriden_pathname, ssdt_number); // start searching root		
	}
	else
		sprintf(filename, "SSDT-%d.aml", ssdt_number);
	
	sprintf(dirspec, "/%s", filename); // start searching root
	
	fd=open (dirspec);
	
	if (fd<0)
	{	// Check Extra on booting partition
		sprintf(dirspec,"/Extra/%s",filename);
		fd=open (dirspec);
		if (fd<0)
		{	// Fall back to booter partition
			sprintf(dirspec,"bt(0,0)/Extra/%s",filename);
			fd=open (dirspec);
			if (fd<0)
			{
				DBG("SSDT Table not found: %s\n", filename);
				return NULL;
			}
		}
	}	
	
	void *tableAddr=(void*)AllocateKernelMemory(file_size (fd));
	if (tableAddr)
	{
		if (read (fd, tableAddr, file_size (fd))!=file_size (fd))
		{
			printf("Couldn't read table %s\n",dirspec);
			free (tableAddr);
			close (fd);
			return NULL;
		}
		
		printf("Valid SSDT Table found: %s\n", filename);
		DBG("Table %s read and stored at: %x\n", dirspec, tableAddr);
		close (fd);
		return tableAddr;
	}
	
	printf("Couldn't allocate memory for table %s\n", dirspec);
	close (fd);
	
	return NULL;
}

void *loadACPITable(char *key)
{	
	int fd = -1;
	char dirspec[512];
	char filename[512];
	const char * overriden_pathname=NULL;
	int len=0;
    
	DBG("Searching for %s.aml file ...\n", key);
	// Check booting partition	
    
	// Rek: if user specified a full path name then take it in consideration
	if (getValueForKey(key, &overriden_pathname, &len,  
					   DEFAULT_BOOT_CONFIG))
	{		
		sprintf(filename, "%s", overriden_pathname);		
	}
	else		
		sprintf(filename, "%s.aml", key);
    
	
	sprintf(dirspec, "/%s", filename); // start searching root
	
	fd=open (dirspec);
	
	if (fd<0)
	{	
		// Check Extra on booting partition
		sprintf(dirspec,"/Extra/%s",filename);
		fd=open (dirspec);
		if (fd<0)
		{	// Fall back to booter partition
			sprintf(dirspec,"bt(0,0)/Extra/%s",filename);
			fd=open (dirspec);
			if (fd<0)
			{				
				DBG("ACPI Table not found: %s\n", key);
				return NULL;
			}
			
		}
		
	}		
	
	void *tableAddr=(void*)AllocateKernelMemory(file_size (fd));
	if (tableAddr)
	{
		if (read (fd, tableAddr, file_size (fd))!=file_size (fd))
		{
			printf("Couldn't read table %s\n",key);
			free (tableAddr);
			close (fd);
			return NULL;
		}
		verbose("Valid ACPI Table found: %s\n", key);
		//DBG("Table %s read and stored at: %x\n", key, tableAddr);
		close (fd);
		return tableAddr;
	}
	
	printf("Couldn't allocate memory for table %s\n", key);
	close (fd);
	
	return NULL;
}

uint8_t	acpi_cpu_count = 0;
char* acpi_cpu_name[32];

void get_acpi_cpu_names(unsigned char* dsdt, uint32_t length)
{
	uint32_t i;
	
	for (i=0; i<length-7; i++) 
	{
		if (dsdt[i] == 0x5B && dsdt[i+1] == 0x83) // ProcessorOP
		{
			uint32_t offset = i + 3 + (dsdt[i+2] >> 6);
			
			bool add_name = true;
            
			uint8_t j;
			
			for (j=0; j<4; j++) 
			{
				char c = dsdt[offset+j];
				
				if (!aml_isvalidchar(c)) 
				{
					add_name = false;
					verbose("Invalid character found in ProcessorOP 0x%x!\n", c);
					break;
				}
			}
			
			if (add_name) 
			{
				acpi_cpu_name[acpi_cpu_count] = malloc(4);
				memcpy(acpi_cpu_name[acpi_cpu_count], dsdt+offset, 4);
				i = offset + 5;
				
				verbose("Found ACPI CPU: %c%c%c%c\n", acpi_cpu_name[acpi_cpu_count][0], acpi_cpu_name[acpi_cpu_count][1], acpi_cpu_name[acpi_cpu_count][2], acpi_cpu_name[acpi_cpu_count][3]);
				
				if (++acpi_cpu_count == 32) return;
			}
		}
	}
}


struct acpi_2_ssdt *generate_cst_ssdt(struct acpi_2_fadt* fadt)
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
	
	if (get_env(envVendor) != CPUID_VENDOR_INTEL) {
		verbose ("Not an Intel platform: C-States will not be generated !!!\n");
		return NULL;
	}
	
	if (fadt == NULL) {
		verbose ("FACP not exists: C-States will not be generated !!!\n");
		return NULL;
	}
    
	struct acpi_2_dsdt* dsdt = (void*)fadt->DSDT;
	
	if (dsdt == NULL) {
		verbose ("DSDT not found: C-States will not be generated !!!\n");
		return NULL;
	}
	
	if (acpi_cpu_count == 0) 
		get_acpi_cpu_names((void*)dsdt, dsdt->Length);
	
	if (acpi_cpu_count > 0) 
	{
		bool c2_enabled = fadt->C2_Latency < 100;
		bool c3_enabled = fadt->C3_Latency < 1000;
		bool c4_enabled = false;
		
		getBoolForKey(kEnableC4State, &c4_enabled, DEFAULT_BOOT_CONFIG);
        
		unsigned char cstates_count = 1 + (c2_enabled ? 1 : 0) + ((c3_enabled || c4_enabled) ? 1 : 0);
		char *Lat = NULL, *Pw = NULL, *tmpstr =NULL;
		int base = 16;
		TagPtr personality = XMLCastDict(XMLGetProperty(DEFAULT_BOOT_CONFIG_DICT, (const char*)"C-States"));
		
		if ((tmpstr = XMLCastString(XMLGetProperty(personality, (const char*)"Base")))) {
			
			int mybase = strtol(tmpstr, NULL, 10);	
			
			if (mybase == 8 || mybase == 10 || mybase == 16 ) 
				base = mybase;									
		}
		
		struct aml_chunk* root = aml_create_node(NULL);
        aml_add_buffer(root, ssdt_header, sizeof(ssdt_header)); // SSDT header
        struct aml_chunk* scop = aml_add_scope(root, "\\_PR_");
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
			aml_add_word(tmpl, resolve_cst(fadt->C2_Latency, Lat, base));  // Latency
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
			aml_add_word(tmpl, resolve_cst(fadt->C3_Latency / 2, Lat, base));  // TODO: right latency for C4
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
			aml_add_word(tmpl, resolve_cst(fadt->C3_Latency , Lat, base));  
			aml_add_word(tmpl, resolve_cst(0x015e, Pw, base)); // Power
			
		}
		
        
        // Aliaces
        int i;
        for (i = 0; i < acpi_cpu_count; i++) 
        {
            char name[9];
            sprintf(name, "_PR_%c%c%c%c", acpi_cpu_name[i][0], acpi_cpu_name[i][1], acpi_cpu_name[i][2], acpi_cpu_name[i][3]);
            
            scop = aml_add_scope(root, name);
            aml_add_alias(scop, "CST_", "_CST");
        }
		
		aml_calculate_size(root);
		
		struct acpi_2_ssdt *ssdt = (struct acpi_2_ssdt *)AllocateKernelMemory(root->Size);
        
		aml_write_node(root, (void*)ssdt, 0);
		
		ssdt->Length = root->Size;        
		
		setchecksum((struct acpi_common_header *)ssdt);
        
		aml_destroy_node(root);		
        
		verbose ("SSDT with CPU C-States generated successfully\n");
		
		return ssdt;
	}
	else 
	{
		verbose ("ACPI CPUs not found: C-States not generated !!!\n");
	}
    
	return NULL;
}

struct acpi_2_ssdt *generate_pss_ssdt(struct acpi_2_dsdt* dsdt)
{		
	
	char ssdt_header[] =
	{
		0x53, 0x53, 0x44, 0x54, 0x7E, 0x00, 0x00, 0x00, /* SSDT.... */
		0x01, 0x6A, 0x50, 0x6D, 0x52, 0x65, 0x66, 0x00, /* ..PmRef. */
		0x43, 0x70, 0x75, 0x50, 0x6D, 0x00, 0x00, 0x00, /* CpuPm... */
		0x00, 0x30, 0x00, 0x00, 0x49, 0x4E, 0x54, 0x4C, /* .0..INTL */
		0x31, 0x03, 0x10, 0x20,							/* 1.._		*/
	};
    
	if (get_env(envVendor) != CPUID_VENDOR_INTEL) {
		verbose ("Not an Intel platform: P-States will not be generated !!!\n");
		return NULL;
	}
	
	if (!(get_env(envFeatures) & CPUID_FEATURE_MSR)) {
		verbose ("Unsupported CPU: P-States will not be generated !!!\n");
		return NULL;
	}
	
	if (acpi_cpu_count == 0) 
		get_acpi_cpu_names((void*)dsdt, dsdt->Length);
	
	if (acpi_cpu_count > 0) 
	{
		struct p_state initial, maximum, minimum, p_states[32];
		uint8_t p_states_count = 0;		
		
		// Retrieving P-States, ported from code by superhai (c)
		switch (get_env(envFamily)) {
			case 0x06: 
			{
				switch (get_env(envModel)) 
				{
					case CPUID_MODEL_DOTHAN: // ?
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
							uint64_t msr;
							uint8_t i;
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
							uint64_t msr;
							uint8_t i;
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
							
							uint8_t vidstep;
							uint8_t i = 0, u, invalid = 0;
							
							vidstep = ((maximum.VID << 2) - (minimum.VID << 2)) / (p_states_count - 1);
							
                            uint32_t fsb = (uint32_t)get_env(envFSBFreq) / 1000000;
                            
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
								
								uint32_t multiplier = p_states[i].FID & 0x1f;		// = 0x08
								bool half = p_states[i].FID & 0x40;					// = 0x01
								bool dfsb = p_states[i].FID & 0x80;					// = 0x00
								//uint32_t fsb = Platform->CPU.FSBFrequency / 1000000; // = 400
								uint32_t halffsb = (fsb + 1) >> 1;					// = 200
								uint32_t frequency = (multiplier * fsb);			// = 3200
								
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
							uint8_t i;
							p_states_count = 0;
							uint64_t fsb = (get_env(envFSBFreq) / 1000000);
							for (i = maximum.Control; i >= minimum.Control; i--) 
							{
								p_states[p_states_count].Control = i;
								p_states[p_states_count].CID = p_states[p_states_count].Control << 1;
								p_states[p_states_count].Frequency = (uint32_t) fsb * i;
								p_states_count++;
							}
						}
						
						break;
					}
					default:
						verbose ("Unsupported CPU: P-States not generated !!!\n");
						break;
				}
			}
			default:
				break;
		}
		
		// Generating SSDT
		if (p_states_count) 
		{	
			int i;
			
			struct aml_chunk* root = aml_create_node(NULL);
            aml_add_buffer(root, ssdt_header, sizeof(ssdt_header)); // SSDT header
            struct aml_chunk* scop = aml_add_scope(root, "\\_PR_");
            struct aml_chunk* name = aml_add_name(scop, "PSS_");
            struct aml_chunk* pack = aml_add_package(name);
			
            uint64_t FSBFreq = get_env(envFSBFreq);
            
            uint8_t minPSratio = (p_states[p_states_count-1].Frequency / (FSBFreq / 10000000 ));
            uint8_t maxPSratio = (p_states[0].Frequency / (FSBFreq / 10000000 ));
            
            uint8_t cpu_div = (uint8_t)get_env(envCurrDiv);
            uint8_t cpu_coef = (uint8_t)get_env(envCurrCoef);

            uint8_t cpu_ratio = 0;
			
            if (cpu_div) 								
                cpu_ratio = (cpu_coef * 10) + 5;								
            else 								
                cpu_ratio = cpu_coef * 10;
            
			
            int user_max_ratio = 0;
            getIntForKey(kMaxRatio, &user_max_ratio, DEFAULT_BOOT_CONFIG);
            if (user_max_ratio >= minPSratio && maxPSratio >= user_max_ratio) {									
				
                uint8_t maxcurrdiv = 0, maxcurrcoef = (int)(user_max_ratio / 10);									
				
                uint8_t maxdiv = user_max_ratio - (maxcurrcoef * 10);
                if (maxdiv > 0)
                    maxcurrdiv = 1;
				
                if (maxcurrdiv) 									
                    cpu_ratio = (maxcurrcoef * 10) + 5;									
                else 									
                    cpu_ratio = maxcurrcoef * 10;																
            }
			
            int user_min_ratio = 0;
            getIntForKey(kMinRatio, &user_min_ratio, DEFAULT_BOOT_CONFIG);
            if (user_min_ratio >= minPSratio && cpu_ratio >= user_min_ratio) {
				
                uint8_t mincurrdiv = 0, mincurrcoef = (int)(user_min_ratio / 10);									
				
                uint8_t mindiv = user_min_ratio - (mincurrcoef * 10);
				
                if (mindiv > 0)
                    mincurrdiv = 1;									
				
                if (mincurrdiv) 									
                    minPSratio = (mincurrcoef * 10) + 5;									
                else 									
                    minPSratio = mincurrcoef * 10;																		
				
            }
			
            if (maxPSratio >= cpu_ratio && cpu_ratio >= minPSratio)	maxPSratio = cpu_ratio;													
			
            TagPtr personality = XMLCastDict(XMLGetProperty(DEFAULT_BOOT_CONFIG_DICT, (const char*)"P-States"));
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
			
            uint64_t fsb = (get_env(envFSBFreq) / 10000000 );
            
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
                
                uint8_t curr_ratio = (uint8_t)(Frequency / fsb);
                
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
#if DEBUG_ACPI								
                verbose("state with cpu frequency :%d and ratio :%d will be dropped\n",p_states[i].Frequency,curr_ratio);									
#endif
                dropPSS++;
                
                
            }			
			
			// Add aliaces
			for (i = 0; i < acpi_cpu_count; i++) 
			{
				char name[9];
				sprintf(name, "_PR_%c%c%c%c", acpi_cpu_name[i][0], acpi_cpu_name[i][1], acpi_cpu_name[i][2], acpi_cpu_name[i][3]);
				
				scop = aml_add_scope(root, name);
				aml_add_alias(scop, "PSS_", "_PSS");
			}
			
			aml_calculate_size(root);
			
			struct acpi_2_ssdt *ssdt = (struct acpi_2_ssdt *)AllocateKernelMemory(root->Size);
			
			aml_write_node(root, (void*)ssdt, 0);
			
			ssdt->Length = root->Size;
			
            setchecksum((struct acpi_common_header *)ssdt);
			
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
		verbose("ACPI CPUs not found: P-States not generated !!!\n");
	}
	
	return NULL;
}

  
struct acpi_2_gas FillGASStruct(uint32_t Address, uint8_t Length)
{
	struct acpi_2_gas TmpGAS;
	
	TmpGAS.Address_Space_ID = 1; /* I/O Address */
	
	if (Address == 0)
	{
		TmpGAS.Register_Bit_Width = 0;
	} else {
		TmpGAS.Register_Bit_Width = Length * 8;
	}
	
	TmpGAS.Register_Bit_Offset = 0;
	TmpGAS.Access_Size = 0; /* Not set for Legacy reasons... */
	TmpGAS.Address = (uint64_t)Address;
	
	return(TmpGAS);
}

struct acpi_2_fadt *
patch_fadt(struct acpi_2_fadt *fadt, struct acpi_2_dsdt *new_dsdt, bool UpdateFADT)
{		
	
	struct acpi_2_fadt *fadt_mod;
	struct acpi_2_fadt *fadt_file = (struct acpi_2_fadt *)loadACPITable(kFADT);        
	bool fadt_rev2_needed = false;
	bool fix_restart = false;	
	const char * value;
	bool aspmOff = true;
	bool msiOff = true;

	// Restart Fix
	if (get_env(envVendor) == CPUID_VENDOR_INTEL) {	/* Intel */
		fix_restart = true;
		getBoolForKey(kRestartFix, &fix_restart, DEFAULT_BOOT_CONFIG);
		
	} else {
		verbose ("Not an Intel platform: Restart Fix not applied !!!\n");
	}
	
	if (fix_restart)
		fadt_rev2_needed = true;
			
	// Allocate new fadt table
	if ((UpdateFADT) && (((fadt_file) && (fadt_file->Length < sizeof(struct acpi_2_fadt))) ||
						 ((!fadt_file) && (fadt->Length < sizeof(struct acpi_2_fadt)))))        
	{
        getBoolForKey(kDisableMSI, &msiOff, DEFAULT_BOOT_CONFIG) ;
        getBoolForKey(kDisableASPM, &aspmOff, DEFAULT_BOOT_CONFIG);
        
		if (fadt_file)
		{
			if (fadt_file->Length < 0xF4)
			{
				fadt_mod=(struct acpi_2_fadt *)AllocateKernelMemory(0xF4);
				memcpy(fadt_mod, fadt_file, fadt_file->Length);
				fadt_mod->Length = 0xF4;
			}
			else
			{
				
				fadt_mod=(struct acpi_2_fadt *)AllocateKernelMemory(fadt_file->Length);
				memcpy(fadt_mod, fadt_file, fadt_file->Length);
			}
		}
		else 
		{
			if (fadt->Length < 0xF4)
			{
				fadt_mod=(struct acpi_2_fadt *)AllocateKernelMemory(0xF4);
				memcpy(fadt_mod, fadt, fadt->Length);
				fadt_mod->Length = 0xF4;
			}
			else
			{
				
				fadt_mod=(struct acpi_2_fadt *)AllocateKernelMemory(fadt->Length);	
				memcpy(fadt_mod, fadt, fadt->Length);
			}		   
		}		
        
		fadt_mod->Revision = 0x04; // FADT rev 4
		fadt_mod->RESET_REG = FillGASStruct(0, 0);
		fadt_mod->Reset_Value = 0;
		fadt_mod->Reserved[0] = 0;
		fadt_mod->Reserved[1] = 0;
		fadt_mod->Reserved[2] = 0;
        		
			fadt_mod->X_PM1a_EVT_BLK = FillGASStruct(fadt_mod->PM1A_Event_Block_Address, fadt_mod->PM1_Event_Block_Length);
			fadt_mod->X_PM1b_EVT_BLK = FillGASStruct(fadt_mod->PM1B_Event_Block_Address, fadt_mod->PM1_Event_Block_Length);
			fadt_mod->X_PM1a_CNT_BLK = FillGASStruct(fadt_mod->PM1A_Control_Block_Address, fadt_mod->PM1_Control_Block_Length);
			fadt_mod->X_PM1b_CNT_BLK = FillGASStruct(fadt_mod->PM1B_Control_Block_Address, fadt_mod->PM1_Control_Block_Length);
			fadt_mod->X_PM2_CNT_BLK = FillGASStruct(fadt_mod->PM2_Control_Block_Address, fadt_mod->PM2_Control_Block_Length);
			fadt_mod->X_PM_TMR_BLK = FillGASStruct(fadt_mod->PM_Timer_Block_Address, fadt_mod->PM_Timer_Block_Length);
			fadt_mod->X_GPE0_BLK = FillGASStruct(fadt_mod->GPE0_Block_Address, fadt_mod->GPE0_Block_Length);
			fadt_mod->X_GPE1_BLK = FillGASStruct(fadt_mod->GPE1_Block_Address, fadt_mod->GPE1_Block_Length);
	        
		verbose("Converted ACPI V%d FADT to ACPI V4 FADT\n", (fadt) ? fadt->Revision : fadt->Revision);
	} else {
		
		if (fadt_file) {
			if (fadt_file->Length < 0x84 && fadt_rev2_needed)
			{
				fadt_mod=(struct acpi_2_fadt *)AllocateKernelMemory(0x84);
				memcpy(fadt_mod, fadt_file, fadt_file->Length);
				fadt_mod->Length   = 0x84;
				fadt_mod->Revision = 0x02; // FADT rev 2 (ACPI 1.0B MS extensions)
			} else {
				fadt_mod=(struct acpi_2_fadt *)AllocateKernelMemory(fadt_file->Length);
				memcpy(fadt_mod, fadt_file, fadt_file->Length);
			}
            
		} else {
			if (fadt->Length < 0x84 && fadt_rev2_needed)
			{
				fadt_mod=(struct acpi_2_fadt *)AllocateKernelMemory(0x84);
				memcpy(fadt_mod, fadt, fadt->Length);
				fadt_mod->Length   = 0x84;
				fadt_mod->Revision = 0x02; // FADT rev 2 (ACPI 1.0B MS extensions)
			} else {
				fadt_mod=(struct acpi_2_fadt *)AllocateKernelMemory(fadt->Length);
				memcpy(fadt_mod, fadt, fadt->Length);
			}
            
		}        
        
	}
    
	uint8_t Type = PMProfileError;
	// Determine system type / PM_Model
	
	// Fix System-type if needed (should never happen)
	if (get_env(envType) > MaxSupportedPMProfile)  
	{
		if(fadt_mod->PM_Profile <= MaxSupportedPMProfile)
			safe_set_env(envType,fadt_mod->PM_Profile); // get the fadt if correct
		else 
			safe_set_env(envType, 1);		/* Set a fixed value (Desktop) */
	}
	
	// If needed, set System-type from PM_Profile (if valid) else set PM_Profile with a fixed the System-type  
	// Give prior to the FADT pm profile, allow to also control this value with a patched FADT table
	if (fadt_mod->PM_Profile != get_env(envType)) 
	{
		bool val = false;  
		getBoolForKey("PreferInternalProfileDetect", &val, DEFAULT_BOOT_CONFIG); // if true Give prior to the profile resolved trought the CPU model
		
		val = get_env(envIsServer) ;
		
		if (fadt_mod->PM_Profile <= MaxSupportedPMProfile && !val) {
			safe_set_env(envType, fadt_mod->PM_Profile);
		} else {
			fadt_mod->PM_Profile = (uint8_t)get_env(envType);
		}		
		
	}
	
	// Set PM_Profile and System-type if user wanted this value to be forced
	if ( (value=getStringForKey("SystemType", DEFAULT_BOOT_CONFIG))!=NULL) {
		if ((Type = (unsigned char) strtoul(value, NULL, 10) ) <= MaxSupportedPMProfile)
		{
			verbose("FADT: changing Preferred_PM_Profile from 0x%02x to 0x%02x\n", fadt->PM_Profile, Type);			
            safe_set_env(envType,(fadt_mod->PM_Profile = Type));
		} else verbose("Error: system-type must be 0..6. Defaulting to %d !\n", (uint8_t)get_env(envType));
	}
    
	// Patch FADT to fix restart
	if (fix_restart)
	{		
        fadt_mod->Flags|= 0x400;				
		
		fadt_mod->RESET_REG = FillGASStruct(0x0cf9, 1);
		fadt_mod->Reset_Value = 0x06;
		verbose("FADT: Restart Fix applied !\n");
	}
	
	
	if (aspmOff) {
		fadt_mod->Boot_Flags |= 1 << 4; 
	} else {
		fadt_mod->Boot_Flags &= 0xFFEF; 
        
	}			
    
	if (msiOff) {
		fadt_mod->Boot_Flags |= 1 << 3; 
	} else {
		fadt_mod->Boot_Flags &= 0xFFF7;
	}
	
		fadt_mod->FIRMWARE_CTRL=(uint32_t)fadt->FIRMWARE_CTRL;
		if ((uint32_t)(&(fadt_mod->X_FIRMWARE_CTRL))-(uint32_t)fadt_mod+8<=fadt_mod->Length)
			fadt_mod->X_FIRMWARE_CTRL=(uint32_t)fadt->FIRMWARE_CTRL;
        
    
    safe_set_env(envHardwareSignature,((struct acpi_2_facs *)fadt->FIRMWARE_CTRL)->hardware_signature);

	DBG("setting hardware_signature to %x \n",(uint32_t)get_env(envHardwareSignature));
    	
	// Patch DSDT Address if we have loaded a DSDT table
	if(new_dsdt)
	{		
		
		fadt_mod->DSDT=(uint32_t)new_dsdt;
		if ((uint32_t)(&(fadt_mod->X_DSDT))-(uint32_t)fadt_mod+8<=fadt_mod->Length)
			fadt_mod->X_DSDT=(uint32_t)new_dsdt;		
		
	}
	
	// Correct the checksum	
    setchecksum((struct acpi_common_header *)fadt_mod);
    
	return fadt_mod;
}


/* Setup ACPI */
EFI_STATUS setupAcpi(void)
{	
	int version;
    EFI_STATUS Status = EFI_ABORTED;
	void *new_dsdt=NULL, *new_hpet=NULL, *new_sbst=NULL, *new_ecdt=NULL, *new_asft=NULL, *new_dmar=NULL, *new_apic=NULL, *new_mcfg=NULL, *new_ssdts[14];

	struct acpi_2_ssdt *new_ssdt[17]; // 15 + 2 additional tables for pss & cst 
	struct acpi_2_fadt *fadt; // will be used in CST generator
	struct acpi_2_fadt *fadt_mod=NULL;
	bool oem_dsdt=false, oem_ssdt=false, oem_hpet=false, oem_sbst=false, oem_ecdt=false, oem_asft=false, oem_dmar=false, oem_apic=false, oem_mcfg=false;
	bool update_acpi=false, gen_xsdt=false;
	bool hpet_replaced=false, sbst_replaced=false, ecdt_replaced=false, asft_replaced=false, dmar_replaced=false, apic_replaced=false, mcfg_replaced=false;
	bool hpet_added=false, sbst_added=false, ecdt_added=false, asft_added=false, dmar_added=false, apic_added=false, mcfg_added=false;
	bool gen_csta=false, gen_psta=false, speed_step=false;
	
	bool quick_ssdt= false;  
	// Quick ssdt search, 
	// first try to find ssdt-0.aml, if the file do not exist we
	// stop searching here, else ssdt-0.aml is loaded and we try 
	// to find ssdt-1.aml, etc .........
	
	int curssdt=0, loadtotssdt=0, totssdt=0, newtotssdt=0;
	
	{
		bool tmpval;
		
		oem_dsdt=getBoolForKey(kOEMDSDT, &tmpval, DEFAULT_BOOT_CONFIG)&&tmpval;
		oem_ssdt=getBoolForKey(kOEMSSDT, &tmpval, DEFAULT_BOOT_CONFIG)&&tmpval;
		oem_hpet=getBoolForKey(kOEMHPET, &tmpval, DEFAULT_BOOT_CONFIG)&&tmpval;
		oem_sbst=getBoolForKey(kOEMSBST, &tmpval, DEFAULT_BOOT_CONFIG)&&tmpval;
		oem_ecdt=getBoolForKey(kOEMECDT, &tmpval, DEFAULT_BOOT_CONFIG)&&tmpval;
		oem_asft=getBoolForKey(kOEMASFT, &tmpval, DEFAULT_BOOT_CONFIG)&&tmpval;
		oem_dmar=getBoolForKey(kOEMDMAR, &tmpval, DEFAULT_BOOT_CONFIG)&&tmpval;
		oem_apic=getBoolForKey(kOEMAPIC, &tmpval, DEFAULT_BOOT_CONFIG)&&tmpval;
		oem_mcfg=getBoolForKey(kOEMMCFG, &tmpval, DEFAULT_BOOT_CONFIG)&&tmpval;

		gen_csta=getBoolForKey(kGenerateCStates, &tmpval, DEFAULT_BOOT_CONFIG)&&tmpval;
		gen_psta=getBoolForKey(kGeneratePStates, &tmpval, DEFAULT_BOOT_CONFIG)&&tmpval;
        
		update_acpi=getBoolForKey(kUpdateACPI, &tmpval, DEFAULT_BOOT_CONFIG)&&tmpval;
		
		quick_ssdt=getBoolForKey(kQSSDT, &tmpval, DEFAULT_BOOT_CONFIG)&&tmpval;
		
		speed_step=getBoolForKey(kSpeedstep, &tmpval, DEFAULT_BOOT_CONFIG)&&tmpval;
		
	} 	
    
	if (speed_step) {
		gen_psta= true;
		gen_csta= true;
	} 

    
	// Load replacement ACPI tables
	if (!oem_dsdt)
		new_dsdt=loadACPITable(kDSDT);
	
	
	if (!oem_hpet)
		new_hpet=loadACPITable(kHPET);
	
	
	if (!oem_sbst)
		new_sbst=loadACPITable(kSBST);
	
	
	if (!oem_ecdt)
		new_ecdt=loadACPITable(kECDT);
	
	
	if (!oem_asft)
		new_asft=loadACPITable(kASFT);
	
	
	if (!oem_dmar)
		new_dmar=loadACPITable(kDMAR);
	
	
	if (!oem_apic)
		new_apic=loadACPITable(kAPIC);
	
	
	if (!oem_mcfg)
		new_mcfg=loadACPITable(kMCFG);
	
	if (!oem_ssdt)
	{                
		
		for (curssdt=0;curssdt<15;curssdt++)
		{
			new_ssdt[curssdt]=loadSSDTTable(curssdt);
			
			if (new_ssdt[curssdt])
				loadtotssdt++;			
			else if (quick_ssdt)				
				break;		
			
		}
        
		curssdt=0;
		
	}

	DBG("New ACPI tables Loaded in memory\n");
	TagPtr DropTables_p = XMLCastDict(XMLGetProperty(DEFAULT_BOOT_CONFIG_DICT, (const char*)"ACPIDropTables"));
	// Do the same procedure for both versions of ACPI
	for (version=0; version<2; version++) {
		struct acpi_2_rsdp *rsdp_mod, *rsdp_conv=(struct acpi_2_rsdp *)0;
		struct acpi_2_rsdt *rsdt, *rsdt_mod;
		struct acpi_2_xsdt *xsdt_conv = (struct acpi_2_xsdt *)0;
		int rsdplength;
        
		// Find original rsdp        
        struct acpi_2_rsdp *rsdp=(struct acpi_2_rsdp *)(version?biosacpi_find_rsdp(2):biosacpi_find_rsdp(0));
		
		if ((update_acpi) && (rsdp->Revision == 0))
		{

			rsdp_conv = gen_rsdp_v2_from_v1(rsdp);
			gen_xsdt = true;
			version = 1;
			
			addConfigurationTable(&gEfiAcpiTableGuid, NULL, "ACPI");
			
			verbose("Converted ACPI RSD PTR version 1 to version 3\n");
		}
        
		if (!rsdp)
		{
			DBG("No ACPI version %d found. Ignoring\n", version+1);
			if (version)
				addConfigurationTable(&gEfiAcpi20TableGuid, NULL, "ACPI_20");
			else
				addConfigurationTable(&gEfiAcpiTableGuid, NULL, "ACPI");
			continue;
		}
		rsdplength=version?rsdp->Length:20;
		
		DBG("RSDP version %d found @%x. Length=%d\n",version+1,rsdp,rsdplength);
		
		/* FIXME: no check that memory allocation succeeded 
		 * Copy and patch RSDP,RSDT, XSDT and FADT
		 * For more info see ACPI Specification pages 110 and following
		 */
		
		if (gen_xsdt)
		{
			rsdp_mod=rsdp_conv;
		} else {
			rsdp_mod=(struct acpi_2_rsdp *) AllocateKernelMemory(rsdplength);
			memcpy(rsdp_mod, rsdp, rsdplength);
		}
		
		rsdt=(struct acpi_2_rsdt *)(rsdp->RsdtAddress);
		
		DBG("RSDT @%x, Length %d\n",rsdt, rsdt->Length);
		
		if (rsdt && (uint32_t)rsdt !=0xffffffff && rsdt->Length<0x10000)
		{			
			int dropoffset=0, i;
			
			rsdt_mod=(struct acpi_2_rsdt *)AllocateKernelMemory(rsdt->Length); 
			memcpy (rsdt_mod, rsdt, rsdt->Length);
            
            update_rsdp_with_rsdt(rsdp_mod, rsdt_mod);
			
			int rsdt_entries_num=(rsdt_mod->Length-sizeof(struct acpi_2_rsdt))/4;
			uint32_t *rsdt_entries=(uint32_t *)(rsdt_mod+1);
			
			if (gen_xsdt)
			{
                
                xsdt_conv=gen_xsdt_from_rsdt(rsdt_mod);
                update_rsdp_with_xsdt(rsdp, xsdt_conv);        
				
				
				verbose("Converted RSDT table to XSDT table\n");
			}
			
			for (i=0;i<rsdt_entries_num;i++)
			{
				char *table=(char *)(rsdt_entries[i]);
				if (!table)
					continue;
                                				
				DBG("TABLE %c%c%c%c,",table[0],table[1],table[2],table[3]);
				
				rsdt_entries[i-dropoffset]=rsdt_entries[i];
				
				char table4[5];
				strlcpy(table4, table, sizeof(table4));
				TagPtr match_drop = XMLGetProperty(DropTables_p, (const char*)table4);
				if ( match_drop ) {
					char *tmpstr = XMLCastString(match_drop);
					if (strcmp(tmpstr,"No") != 0) {						
						dropoffset++;
						DBG("%s table dropped\n",table4);
						continue;
					}
				}
                                
				if ((!(oem_hpet)) && tableSign(table, "HPET"))
				{
					DBG("HPET found\n");
					if (new_hpet)
					{
						rsdt_entries[i-dropoffset]=(uint32_t)new_hpet;
						hpet_replaced=true;
					}
					continue;
				}
                                
				if ((!(oem_sbst)) && tableSign(table, "SBST"))
				{
					DBG("SBST found\n");
					if (new_sbst)
					{
						rsdt_entries[i-dropoffset]=(uint32_t)new_sbst;
						sbst_replaced=true;
					}
					continue;
				}
                               
				if ((!(oem_ecdt)) && tableSign(table, "ECDT"))
				{
					DBG("ECDT found\n");
					
					if (new_ecdt)
					{
						rsdt_entries[i-dropoffset]=(uint32_t)new_ecdt;
						ecdt_replaced=true;
					}
					
					continue;
				}
                                
				if ((!(oem_asft)) && tableSign(table, "ASF!"))
				{
					DBG("ASF! found\n");
					if (new_asft)
					{
						rsdt_entries[i-dropoffset]=(uint32_t)new_asft;
						asft_replaced=true;
					}
					continue;
				}
                                
				if ((!(oem_dmar)) && tableSign(table, "DMAR"))
				{
					DBG("DMAR found\n");
					if (new_dmar)
					{
						rsdt_entries[i-dropoffset]=(uint32_t)new_dmar;
						dmar_replaced=true;
					}
					continue;
				}
                                
				if ((!(oem_apic)) && tableSign(table, "APIC"))
				{
					DBG("APIC found\n");
					if (new_apic)
					{
						rsdt_entries[i-dropoffset]=(uint32_t)new_apic;
						apic_replaced=true;
					}
					continue;
				}
                                
				if ((!(oem_mcfg)) && tableSign(table, "MCFG"))
				{
					DBG("MCFG found\n");
					if (new_mcfg)
					{
						rsdt_entries[i-dropoffset]=(uint32_t)new_mcfg;
						mcfg_replaced=true;
					}
					continue;
				}
                                
				if ((!(oem_ssdt))  && tableSign(table, "SSDT"))
				{
					DBG("SSDT %d found", curssdt);
					if (new_ssdts[curssdt])
					{
						DBG(" and replaced");
						rsdt_entries[i-dropoffset]=(uint32_t)new_ssdts[curssdt];
						totssdt++;
					}
					DBG("\n");
					curssdt++;
					continue;
				}
                                
				if ((!(oem_dsdt)) && tableSign(table, "DSDT"))
				{										
					DBG("DSDT found\n");
					rsdt_entries[i-dropoffset]=(uint32_t)new_dsdt;
					continue;				
				}
                                
				if (tableSign(table, "FACP"))
				{					
					fadt=(struct acpi_2_fadt *)rsdt_entries[i];
					
					DBG("FADT found @%x, Length %d\n",fadt, fadt->Length);
					
					if (!fadt || (uint32_t)fadt == 0xffffffff || fadt->Length>0x10000)
					{
						printf("FADT incorrect. Not modified\n");
						continue;
					}
					verbose("Attempting to patch FADT entry of RSDT\n");
					fadt_mod = patch_fadt(fadt, new_dsdt, update_acpi);
					
					rsdt_entries[i-dropoffset]=(uint32_t)fadt_mod;
					
					if (!oem_ssdt)
					{
						// Generate _CST SSDT
						if ( gen_csta && (new_ssdt[loadtotssdt] = generate_cst_ssdt(fadt_mod)))
						{
							gen_csta= false;
							loadtotssdt++;
						}
						
						// Generating _PSS SSDT
                        if (gen_psta && (new_ssdt[loadtotssdt] = generate_pss_ssdt((void*)fadt_mod->DSDT)))
                        {
                            gen_psta= false;
                            loadtotssdt++;
                        }										
						
					}
					
					continue;
				}
			}
			DBG("\n");
			
			if ((!oem_hpet) && (!hpet_replaced))
			{
				if (new_hpet)
				{
					rsdt_entries[i-dropoffset]=(uint32_t)new_hpet;
					hpet_added=true;
					i++;
				}
			}
			
			if ((!oem_sbst) && (!sbst_replaced))
			{
				if (new_sbst)
				{
					rsdt_entries[i-dropoffset]=(uint32_t)new_sbst;
					sbst_added=true;
					i++;
				}
			}
			
			if ((!oem_ecdt) && (!ecdt_replaced))
			{
				if (new_ecdt)
				{										
					rsdt_entries[i-dropoffset]=(uint32_t)new_ecdt;
					ecdt_added=true;
					i++;
				}
			}
			
			if ((!oem_asft) && (!asft_replaced))
			{
				if (new_asft)
				{
					rsdt_entries[i-dropoffset]=(uint32_t)new_asft;
					asft_added=true;
					i++;
				}
			}
			
			if ((!oem_dmar) && (!dmar_replaced))
			{
				if (new_dmar)
				{
					rsdt_entries[i-dropoffset]=(uint32_t)new_dmar;
					dmar_added=true;
					i++;
				}
			}
			
			if ((!oem_apic) && (!apic_replaced))
			{
				if (new_apic)
				{
					rsdt_entries[i-dropoffset]=(uint32_t)new_apic;
					apic_added=true;
					i++;
				}
			}
			
			if ((!oem_mcfg) && (!mcfg_replaced))
			{
				if (new_mcfg)
				{
					rsdt_entries[i-dropoffset]=(uint32_t)new_mcfg;
					mcfg_added=true;
					i++;
				}
			}
			
			if (!oem_ssdt)
			{ 			
				while ((totssdt < loadtotssdt) && (curssdt < 17))
				{
					if (new_ssdt[curssdt])
					{
						DBG("adding SSDT %d\n", curssdt);						
						rsdt_entries[i-dropoffset]=(uint32_t)new_ssdt[curssdt];
						totssdt++;
						newtotssdt++;
						i++;
					} else if (quick_ssdt) {				
						break;								
					}
					
					curssdt++;
				}
			}
			
			// Correct the checksum of RSDT
			rsdt_mod->Length-=4*dropoffset;
			rsdt_mod->Length+=4*newtotssdt;
			if (hpet_added)
				rsdt_mod->Length+=4;
			if (sbst_added)
				rsdt_mod->Length+=4;
			if (ecdt_added)
				rsdt_mod->Length+=4;
			if (asft_added)
				rsdt_mod->Length+=4;
			if (dmar_added)
				rsdt_mod->Length+=4;
			if (apic_added)
				rsdt_mod->Length+=4;
			if (mcfg_added)
				rsdt_mod->Length+=4;
			
			DBG("RSDT: Original checksum %d, ", rsdt_mod->Checksum);			
			            
            setchecksum((struct acpi_common_header *)rsdt_mod);
			
			DBG("New checksum %d at %x\n", rsdt_mod->Checksum,rsdt_mod);
			
		}
		else
		{
			rsdp_mod->RsdtAddress=0;
			printf("RSDT not found or RSDT incorrect\n");
		}
		
		if (version)
		{
			struct acpi_2_xsdt *xsdt, *xsdt_mod;
			
			// FIXME: handle 64-bit address correctly
			
			if (gen_xsdt)
				xsdt=xsdt_conv;
			else
				xsdt=(struct acpi_2_xsdt*) ((uint32_t)rsdp->XsdtAddress);
			
			DBG("XSDT @%x;%x, Length=%d\n", (uint32_t)(rsdp->XsdtAddress>>32),(uint32_t)rsdp->XsdtAddress,
				xsdt->Length);
			if (xsdt && (uint64_t)rsdp->XsdtAddress<0xffffffff && xsdt->Length<0x10000)
			{				
				int dropoffset=0, i;
				curssdt=0, totssdt=0, newtotssdt=0;
				hpet_replaced=false, hpet_added=false;
				sbst_replaced=false, sbst_added=false;
				ecdt_replaced=false, ecdt_added=false;
				asft_replaced=false, asft_added=false;
				dmar_replaced=false, dmar_added=false;
				apic_replaced=false, apic_added=false;
				mcfg_replaced=false, mcfg_added=false;
				
				if (gen_xsdt)
					xsdt_mod=xsdt;
				else
				{
					xsdt_mod=(struct acpi_2_xsdt*)AllocateKernelMemory(xsdt->Length); 
					memcpy(xsdt_mod, xsdt, xsdt->Length);
				}
               
                update_rsdp_with_xsdt(rsdp_mod, xsdt_mod);
                
				int xsdt_entries_num=(xsdt_mod->Length-sizeof(struct acpi_2_xsdt))/8;
				uint64_t *xsdt_entries=(uint64_t *)(xsdt_mod+1);
				for (i=0;i<xsdt_entries_num;i++)
				{
                    
					char *table=(char *)((uint32_t)(xsdt_entries[i]));
					if (!table)
						continue;
					                                        
					xsdt_entries[i-dropoffset]=xsdt_entries[i];
					
					char table4[5];
					strlcpy(table4, table, sizeof(table4));
					TagPtr match_drop = XMLGetProperty(DropTables_p, (const char*)table4);                    
					if ( match_drop ) {
						char *tmpstr = XMLCastString(match_drop);
						if (strcmp(tmpstr,"No") != 0) {
							dropoffset++;
							DBG("%s table dropped\n",table4);
							continue;
						}
					}
					                    
                    if ((!(oem_hpet)) && tableSign(table, "HPET"))
					{
						DBG("HPET found\n");
						if (new_hpet)
						{
							xsdt_entries[i-dropoffset]=(uint32_t)new_hpet;
							hpet_replaced=true;
						}
						continue;
					}
                                                           
                    if ((!(oem_sbst)) && tableSign(table, "SBST"))
					{
						DBG("SBST found\n");
						if (new_sbst)
						{
							xsdt_entries[i-dropoffset]=(uint32_t)new_sbst;
							sbst_replaced=true;
						}
						continue;
					}
                                        
                    if ((!(oem_ecdt)) && tableSign(table, "ECDT"))
					{
						DBG("ECDT found\n");
						
						if (new_ecdt)
						{
							xsdt_entries[i-dropoffset]=(uint32_t)new_ecdt;
							ecdt_replaced=true;
						}
						
						continue;
					}
                                        
                    if ((!(oem_asft)) && tableSign(table, "ASF!"))
					{
						DBG("ASF! found\n");
						if (new_asft)
						{
							xsdt_entries[i-dropoffset]=(uint32_t)new_asft;
							asft_replaced=true;
						}
						continue;
					}
                    
                                       
                    if ((!(oem_dmar)) && tableSign(table, "DMAR"))
					{
						DBG("DMAR found\n");
						if (new_dmar)
						{
							xsdt_entries[i-dropoffset]=(uint32_t)new_dmar;
							dmar_replaced=true;
						}
						continue;
					}
                                        
                    if ((!(oem_apic)) && tableSign(table, "APIC"))
					{
						DBG("APIC found\n");
						if (new_apic)
						{
							xsdt_entries[i-dropoffset]=(uint32_t)new_apic;
							apic_replaced=true;
						}
						continue;
					}                    
                    
                    if ((!(oem_mcfg)) && tableSign(table, "MCFG"))
					{
						DBG("MCFG found\n");
						if (new_mcfg)
						{
							xsdt_entries[i-dropoffset]=(uint32_t)new_mcfg;
							mcfg_replaced=true;
						}
						continue;
					}
                    
                    if ((!(oem_ssdt))  && tableSign(table, "SSDT"))
					{
						DBG("SSDT %d found", curssdt);
						if (new_ssdts[curssdt])
						{
							DBG(" and replaced");
							xsdt_entries[i-dropoffset]=(uint32_t)new_ssdts[curssdt];
							totssdt++;
						}
						DBG("\n");
						curssdt++;
						continue;
					}
                    
                    if ((!(oem_dsdt)) && tableSign(table, "DSDT"))
					{
						DBG("DSDT found\n");
						
						xsdt_entries[i-dropoffset]=(uint32_t)new_dsdt;
						
						DBG("TABLE %c%c%c%c@%x,",table[0],table[1],table[2],table[3],xsdt_entries[i]);
						
						continue;
						
					}
                    
                    if (tableSign(table, "FACP"))
					{
						fadt=(struct acpi_2_fadt *)(uint32_t)xsdt_entries[i];
						
						DBG("FADT found @%x,%x, Length %d\n",(uint32_t)(xsdt_entries[i]>>32),fadt, 
							fadt->Length);
						
						if (!fadt || (uint64_t)xsdt_entries[i] >= 0xffffffff || fadt->Length>0x10000)
						{
							verbose("FADT incorrect or after 4GB. Dropping XSDT\n");
							goto drop_xsdt;
						}
						verbose("Attempting to patch FADT entry of XSDT\n");
						fadt_mod = patch_fadt(fadt, new_dsdt, update_acpi);
						
						
						xsdt_entries[i-dropoffset]=(uint32_t)fadt_mod;
						
						//DBG("TABLE %c%c%c%c@%x,",table[0],table[1],table[2],table[3],xsdt_entries[i]);
						
						if (!oem_ssdt)
                        {
                            // Generate _CST SSDT
                            if ( gen_csta && (new_ssdt[loadtotssdt] = generate_cst_ssdt(fadt_mod)))
                            {
                                gen_csta= false;
                                loadtotssdt++;
                            }
                            
                            // Generating _PSS SSDT
                            if (gen_psta && (new_ssdt[loadtotssdt] = generate_pss_ssdt((void*)fadt_mod->DSDT)))
                            {
                                gen_psta= false;
                                loadtotssdt++;
                            }
                        }
						
						
						continue;
					}
					
					//DBG("TABLE %c%c%c%c@%x,",table[0],table[1],table[2],table[3],xsdt_entries[i]);
					
				}
				
				if ((!oem_hpet) && (!hpet_replaced))
				{
					if (new_hpet)
					{
						xsdt_entries[i-dropoffset]=(uint32_t)new_hpet;
						hpet_added=true;
						i++;
					}
				}
				
				if ((!oem_sbst) && (!sbst_replaced))
				{
					if (new_sbst)
					{
						xsdt_entries[i-dropoffset]=(uint32_t)new_sbst;
						sbst_added=true;
						i++;
					}
				}
				
				if ((!oem_ecdt) && (!ecdt_replaced))
				{
					if (new_ecdt)
					{
						
						xsdt_entries[i-dropoffset]=(uint32_t)new_ecdt;
						ecdt_added=true;
						i++;
					}
				}
				
				if ((!oem_asft) && (!asft_replaced))
				{
					if (new_asft)
					{
						xsdt_entries[i-dropoffset]=(uint32_t)new_asft;
						asft_added=true;
						i++;
					}
				}
				
				if ((!oem_dmar) && (!dmar_replaced))
				{
					if (new_dmar)
					{
						xsdt_entries[i-dropoffset]=(uint32_t)new_dmar;
						dmar_added=true;
						i++;
					}
				}
				
				if ((!oem_apic) && (!apic_replaced))
				{
					if (new_apic)
					{
						xsdt_entries[i-dropoffset]=(uint32_t)new_apic;
						apic_added=true;
						i++;
					}
				}
				
				if ((!oem_mcfg) && (!mcfg_replaced))
				{
					if (new_mcfg)
					{
						xsdt_entries[i-dropoffset]=(uint32_t)new_mcfg;
						mcfg_added=true;
						i++;
					}
				}
				
				if (!oem_ssdt)
				{ 			
					while ((totssdt < loadtotssdt) && (curssdt < 17))
					{
						if (new_ssdt[curssdt])
						{
							DBG("adding SSDT %d\n", curssdt);
							xsdt_entries[i-dropoffset]=(uint32_t)new_ssdt[curssdt];
							totssdt++;
							newtotssdt++;
							i++;
						} else if (quick_ssdt){				
							break;								
						}
						
						curssdt++;
					}
				}
				
				// Correct the checksum of XSDT
				xsdt_mod->Length-=8*dropoffset;
				xsdt_mod->Length+=8*newtotssdt;
				if (hpet_added)
					xsdt_mod->Length+=8;
				if (sbst_added)
					xsdt_mod->Length+=8;
				if (ecdt_added)
					xsdt_mod->Length+=8;
				if (asft_added)
					xsdt_mod->Length+=8;
				if (dmar_added)
					xsdt_mod->Length+=8;
				if (apic_added)
					xsdt_mod->Length+=8;
				if (mcfg_added)
					xsdt_mod->Length+=8;				
                
                setchecksum((struct acpi_common_header *)xsdt_mod);
			}
			else
			{
			drop_xsdt:
				
				DBG("About to drop XSDT\n");
				
				/*FIXME: Now we just hope that if MacOS doesn't find XSDT it reverts to RSDT. 
				 * A Better strategy would be to generate
				 */
				
				rsdp_mod->XsdtAddress=0xffffffffffffffffLL;
				verbose("XSDT not found or XSDT incorrect\n");
			}
		}
		
		// Correct the checksum of RSDP      
		
		DBG("RSDP: Original checksum %d, ", rsdp_mod->Checksum);		
		        
        setRsdpchecksum(rsdp_mod);
		
		DBG("New checksum %d\n", rsdp_mod->Checksum);
		
		if (version)
		{
			DBG("RSDP: Original extended checksum %d", rsdp_mod->ExtendedChecksum);			
            
            setRsdpXchecksum(rsdp_mod);
			
			DBG("New extended checksum %d\n", rsdp_mod->ExtendedChecksum);
			
		}
		
		verbose("Patched ACPI version %d\n", version+1);
		
		
        Status = Register_Acpi_Efi(rsdp_mod, version+1);

	}

#if DEBUG_DSDT
	printf("Press a key to continue... (DEBUG_DSDT)\n");
	getc();
#endif
	return Status ;
}
