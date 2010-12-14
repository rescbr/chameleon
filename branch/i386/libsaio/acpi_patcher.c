/*
 * Copyright 2008 mackerintel
 */

#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "acpi.h"
#include "efi_tables.h"
#include "fake_efi.h"
#include "acpi_patcher.h"
#include "platform.h"
#include "cpu.h"
#include "aml_generator.h"

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

uint64_t acpi10_p;
uint64_t acpi20_p;

/* Gets the ACPI 1.0 RSDP address */
static struct acpi_2_rsdp* getAddressOfAcpiTable()
{
    /* TODO: Before searching the BIOS space we are supposed to search the first 1K of the EBDA */
	
    void *acpi_addr = (void*)ACPI_RANGE_START;
    for(; acpi_addr <= (void*)ACPI_RANGE_END; acpi_addr += 16)
    {
        if(*(uint64_t *)acpi_addr == ACPI_SIGNATURE_UINT64_LE)
        {
            uint8_t csum = checksum8(acpi_addr, 20);
            if(csum == 0)
            {
                // Only return the table if it is a true version 1.0 table (Revision 0)
                if(((struct acpi_2_rsdp*)acpi_addr)->Revision == 0)
                    return acpi_addr;
            }
        }
    }
    return NULL;
}

/* Gets the ACPI 2.0 RSDP address */
static struct acpi_2_rsdp* getAddressOfAcpi20Table()
{
    /* TODO: Before searching the BIOS space we are supposed to search the first 1K of the EBDA */
	
    void *acpi_addr = (void*)ACPI_RANGE_START;
    for(; acpi_addr <= (void*)ACPI_RANGE_END; acpi_addr += 16)
    {
        if(*(uint64_t *)acpi_addr == ACPI_SIGNATURE_UINT64_LE)
        {
            uint8_t csum = checksum8(acpi_addr, 20);

            /* Only assume this is a 2.0 or better table if the revision is greater than 0
             * NOTE: ACPI 3.0 spec only seems to say that 1.0 tables have revision 1
             * and that the current revision is 2.. I am going to assume that rev > 0 is 2.0.
             */

            if(csum == 0 && (((struct acpi_2_rsdp*)acpi_addr)->Revision > 0))
            {
                uint8_t csum2 = checksum8(acpi_addr, sizeof(struct acpi_2_rsdp));
                if(csum2 == 0)
                    return acpi_addr;
            }
        }
    }
    return NULL;
}

/* The folowing ACPI Table search algo. should be reused anywhere needed:*/
int search_and_get_acpi_fd(const char * filename, const char ** outDirspec)
{
	int fd=0;
	const char * overriden_pathname=NULL;
	static char dirspec[512]="";
	int len=0;
	
	// Take in accound user overriding if it's DSDT only
	if (strstr(filename, "DSDT") && 
		getValueForKey(kDSDT, &overriden_pathname, &len, &bootInfo->bootConfig))
    {
		sprintf(dirspec, "%s", overriden_pathname); // start searching root
		fd=open (dirspec,0);
		if (fd>=0) goto success_fd;
    }
	
	// Start searching any potential location for ACPI Table
	sprintf(dirspec, "/%s", filename); // start searching root
	fd=open (dirspec,0);
	if (fd>=0) goto success_fd;
	
	sprintf(dirspec, "%s", filename); // start current dir
	fd=open (dirspec,0);
	if (fd>=0) goto success_fd;
	
	sprintf(dirspec,"/Extra/%s",filename);
	fd=open (dirspec,0);
	if (fd>=0) goto success_fd;
	
	sprintf(dirspec,"bt(0,0)/Extra/%s",filename);
	fd=open (dirspec,0);
	if (fd>=0) goto success_fd;
	
	// NOT FOUND:
	//verbose("ACPI Table not found: %s\n", filename);
	if (outDirspec) *outDirspec = "";
	return -1;
	// FOUND
success_fd:
	if (outDirspec) *outDirspec = dirspec; 
	return fd;
}

void *loadACPITable(char *key)
{
	void *tableAddr;
	int fd = -1;
	char dirspec[512];
	char filename[512];
	const char * overriden_pathname=NULL;
	int len=0;

	sprintf(filename, "%s.aml", key);

	// Checking boot partition

	// Rek: if user specified a full path name then take it in consideration
	if (getValueForKey(key, &overriden_pathname, &len, &bootInfo->bootConfig))
	{
		sprintf(dirspec, "%s", overriden_pathname); // start searching root
		//printf("Using custom %s path %s\n", key, dirspec);
		//getc();
	}
	else
		sprintf(dirspec, "/%s", filename); // start searching root

	fd=open (dirspec,0);

	if (fd<0)
	{	// Check Extra on booting partition
	        //verbose("Searching for %s.aml file ...\n", key);
		sprintf(dirspec,"/Extra/%s",filename);
		fd=open (dirspec,0);
		if (fd<0)
		{	// Fall back to booter partition
			sprintf(dirspec,"bt(0,0)/Extra/%s",filename);
			fd=open (dirspec,0);
			if (fd<0)
			{
				//verbose("ACPI Table not found: %s\n", filename);
				return NULL;
			}
		}
	}

	tableAddr=(void*)AllocateKernelMemory(file_size (fd));
	if (tableAddr)
	{
		if (read (fd, tableAddr, file_size (fd))!=file_size (fd))
		{
			printf("Couldn't read table %s\n",dirspec);
			free (tableAddr);
			close (fd);
			return NULL;
		}

		DBG("Table %s read and stored at: %x\n", dirspec, tableAddr);
		close (fd);
		return tableAddr;
	}

	printf("Couldn't allocate memory for table %s\n", dirspec);
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

	if (Platform.CPU.Vendor != 0x756E6547) {
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
		//valv: c2 & c3 are enabled by default; else don't use EnableCStates
		bool c2_enabled, c3_enabled, c4_enabled, c6_enabled, tmpval2, tmpval3, tmpval4, tmpval6;
		c2_enabled = c3_enabled = c4_enabled = c6_enabled = false;
		unsigned char cstates_count;

		c2_enabled = (getBoolForKey(kEnableC2State, &tmpval2, &bootInfo->bootConfig)&&tmpval2) | (fadt->P_LVL2_LAT < 100);
		c3_enabled = (getBoolForKey(kEnableC3State, &tmpval3, &bootInfo->bootConfig)&&tmpval3) | (fadt->P_LVL2_LAT < 1000);
		c4_enabled = getBoolForKey(kEnableC4State, &tmpval4, &bootInfo->bootConfig)&&tmpval4;
		c6_enabled = getBoolForKey(kEnableC6State, &tmpval6, &bootInfo->bootConfig)&&tmpval6;

		if(c6_enabled) c4_enabled = true;
		if(c4_enabled) c3_enabled = true;
		if(c3_enabled) c2_enabled = true;

		if(Platform.CPU.Family == 0x06)
		{
			int intelCPU = Platform.CPU.Model;
			switch (intelCPU) 
			{
				case 0x0F: // Core (65nm)
				case 0x1A: // Core i7, Xeon 5500 series - Bloomfield, Gainstown NHM-EP - LGA1366 (45nm)
				case 0x1E: // Core i5, i7 - Clarksfield, Lynnfield, Jasper Forest - LGA1156 (45nm)
				case 0x1F: // Core i5, i7 - Nehalem
				case 0x25: // Core i3, i5, i7 - Westmere Client - Clarkdale, Arrandale - LGA1156 (32nm)
				case 0x2C: // Core i7 - Westmere EP - Gulftown - LGA1366 (32nm) 6 Core
				case 0x2E: // Westmere-Ex Xeon
				case 0x2F: // Westmere-Ex Xeon - Eagelton
					cstates_count = 1 + (c2_enabled ? 1 : 0) + (c3_enabled ? 1 : 0);
					verbose("C-State: Adding %d states: ", cstates_count);
					break;
				case 0x1C: // Atom (45nm)
				case 0x26: // Atom Lincroft
					cstates_count = 1 + (c2_enabled ? 1 : 0) + (c4_enabled ? 1 : 0) + (c6_enabled ? 1 : 0);
					verbose("C-State: Adding %d states: ", cstates_count);
					break;
				case 0x17:
					cstates_count = 1 + (c2_enabled ? 1 : 0) + (c3_enabled ? 1 : 0) + (c4_enabled ? 1 : 0) + (c6_enabled ? 1 : 0);
					verbose("C-State: Adding %d states: ", cstates_count);
					break;
				default:
					cstates_count = 1 + (c2_enabled ? 1 : 0) + (c3_enabled ? 1 : 0) + (c4_enabled ? 1 : 0);
					break;
			}
		}
		else cstates_count = 1 + (c2_enabled ? 1 : 0) + (c3_enabled ? 1 : 0) + (c4_enabled ? 1 : 0);
		
		struct aml_chunk* root = aml_create_node(NULL);
		aml_add_buffer(root, ssdt_header, sizeof(ssdt_header)); // SSDT header
		struct aml_chunk* scop = aml_add_scope(root, "\\_PR_");
		struct aml_chunk* name = aml_add_name(scop, "CST_");
		struct aml_chunk* pack = aml_add_package(name);
		aml_add_byte(pack, cstates_count);
		struct aml_chunk* tmpl = aml_add_package(pack);

		switch (Platform.CPU.Family) {
			case 0x06: 
			{
				switch (Platform.CPU.Model) 
				{
					case 0x0F: // Core (65nm)
					case 0x1A: // Core i7, Xeon 5500 series - Bloomfield, Gainstown NHM-EP - LGA1366 (45nm)
					case 0x1E: // Core i5, i7 - Clarksfield, Lynnfield, Jasper Forest - LGA1156 (45nm)
					case 0x1F: // Core i5, i7 - Nehalem
					case 0x25: // Core i3, i5, i7 - Westmere Client - Clarkdale, Arrandale - LGA1156 (32nm)
					case 0x2C: // Core i7 - Westmere EP - Gulftown - LGA1366 (32nm) 6 Core
					case 0x2E: // Westmere-Ex Xeon
					case 0x2F: // Westmere-Ex Xeon - Eagelton

							// C1
							cstate_resource_template[11] = 0x00; // C1-Nehalem
							aml_add_buffer(tmpl, cstate_resource_template, sizeof(cstate_resource_template));
							aml_add_byte(tmpl, 0x01); // C1
							aml_add_byte(tmpl, 0x03); // Latency
							aml_add_word(tmpl, 0x03e8); // Power
							verbose ("C1 ");

							// C2
							if (c2_enabled)
							{
								tmpl = aml_add_package(pack);
								cstate_resource_template[11] = 0x10; // C3-Nehalem
								aml_add_buffer(tmpl, cstate_resource_template, sizeof(cstate_resource_template));
								aml_add_byte(tmpl, 0x02); // C2
								aml_add_byte(tmpl, 0x14); // Latency
								aml_add_word(tmpl, 0x01f4); // Power
								verbose ("C3 ");
							}

							// C3
							if (c3_enabled) 
							{
								tmpl = aml_add_package(pack);
								cstate_resource_template[11] = 0x20; // C6-Nehalem
								aml_add_buffer(tmpl, cstate_resource_template, sizeof(cstate_resource_template));
								aml_add_byte(tmpl, 0x03); // C3
								aml_add_word(tmpl, 0xc8); // Latency
								aml_add_word(tmpl, 0x015e); // Power
								verbose ("C6");
							}
							verbose("\n");

						break;

					case 0x1C: // Atom (45nm)
					case 0x26: // Atom Lincroft

							// C1
							cstate_resource_template[11] = 0x00; // C1-Atom
							aml_add_buffer(tmpl, cstate_resource_template, sizeof(cstate_resource_template));
							aml_add_byte(tmpl, 0x01); // C1
							aml_add_byte(tmpl, 0x01); // Latency
							aml_add_word(tmpl, 0x03e8); // Power
							verbose ("C1 ");

							// C2
							if (c2_enabled) 
							{
								tmpl = aml_add_package(pack);
								cstate_resource_template[11] = 0x10; // C2-Atom
								aml_add_buffer(tmpl, cstate_resource_template, sizeof(cstate_resource_template));
								aml_add_byte(tmpl, 0x02); // C2
								aml_add_byte(tmpl, fadt->P_LVL2_LAT); // Latency
								aml_add_word(tmpl, 0x01f4); // Power
								verbose ("C2 ");
							}

							// C3
							/*if (c3_enabled)
							{
								tmpl = aml_add_package(pack);
								cstate_resource_template[11] = 0x20; // C3-Atom
								aml_add_buffer(tmpl, cstate_resource_template, sizeof(cstate_resource_template));
								aml_add_byte(tmpl, 0x03); // C3
								aml_add_word(tmpl, fadt->P_LVL3_LAT); // Latency
								aml_add_word(tmpl, 0x015e); // Power
								verbose ("C-State: Added C3\n");
							}*/

							// C4
							if (c4_enabled)
							{
								tmpl = aml_add_package(pack);
								cstate_resource_template[11] = 0x30; // C4-Atom
								aml_add_buffer(tmpl, cstate_resource_template, sizeof(cstate_resource_template));
								aml_add_byte(tmpl, 0x03); // C4
								aml_add_word(tmpl, 0x64); // Latency
								aml_add_word(tmpl, 0x00fa); // Power
								verbose ("C4 ");
							}

							// C6
							if (c6_enabled)
							{
								tmpl = aml_add_package(pack);
								cstate_resource_template[11] = 0x40; // C6-Atom
								aml_add_buffer(tmpl, cstate_resource_template, sizeof(cstate_resource_template));
								aml_add_byte(tmpl, 0x03); // C6
								aml_add_word(tmpl, 0xC8); // Latency
								aml_add_word(tmpl, 0x0096); // Power
								verbose ("C6 ");
							}
							verbose("\n");

						break;

					case 0x17:
					default:

							// C1
							cstate_resource_template[11] = 0x00; // C1
							aml_add_buffer(tmpl, cstate_resource_template, sizeof(cstate_resource_template));
							aml_add_byte(tmpl, 0x01); // C1
							aml_add_byte(tmpl, 0x01); // Latency
							aml_add_word(tmpl, 0x03e8); // Power 1000
							verbose ("C1 ");

							// C2
							if (c2_enabled) 
							{
								tmpl = aml_add_package(pack);
								cstate_resource_template[11] = 0x10; // C2
								aml_add_buffer(tmpl, cstate_resource_template, sizeof(cstate_resource_template));
								aml_add_byte(tmpl, 0x02); // C2
								aml_add_byte(tmpl, 0x14); // Latency 20
								aml_add_word(tmpl, 0x01f4); // Power 500
								verbose ("C2 ");
							}

							// C3
							if (c3_enabled) 
							{
								tmpl = aml_add_package(pack);
								cstate_resource_template[11] = 0x20; // C3
								aml_add_buffer(tmpl, cstate_resource_template, sizeof(cstate_resource_template));
								aml_add_byte(tmpl, 0x03); // C3
								aml_add_word(tmpl, 0x28); // Latency 40
								aml_add_word(tmpl, 0x015e); // Power 350
								verbose ("C3 ");
							}

							// C4
							if (c4_enabled)
							{
								tmpl = aml_add_package(pack);
								cstate_resource_template[11] = 0x30; // C4
								aml_add_buffer(tmpl, cstate_resource_template, sizeof(cstate_resource_template));
								aml_add_byte(tmpl, 0x03); // C4
								aml_add_word(tmpl, 0x64); // Latency 100
								aml_add_word(tmpl, 0x00fa); // Power 250
								verbose ("C4 ");
							}

							// C6
							if (c6_enabled)
							{
								tmpl = aml_add_package(pack);
								cstate_resource_template[11] = 0x40; // C6
								aml_add_buffer(tmpl, cstate_resource_template, sizeof(cstate_resource_template));
								aml_add_byte(tmpl, 0x03); // C6
								aml_add_word(tmpl, 0xc8); // Latency 200
								aml_add_word(tmpl, 0x0096); // Power 150
								verbose ("C6");
							}
							verbose("\n");

						break;
				}
			}
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
		ssdt->Checksum = 0;
		ssdt->Checksum = 256 - checksum8(ssdt, ssdt->Length);

		aml_destroy_node(root);
		
		//dumpPhysAddr("C-States SSDT content: ", ssdt, ssdt->Length);
		
		//verbose ("SSDT with CPU C-States generated successfully\n");
		
		return ssdt;
	}
	else 
	{
		verbose ("ACPI CPUs not found: C-States not generated !!!\n");
	}

	return NULL;
}

uint8_t mV_to_VID(uint16_t mv) {
	if (Platform.CPU.Vendor == 0x756E6547)
	{
		if ((Platform.CPU.Family == 0x06) && (Platform.CPU.Model == 0x17))
			return ((mv * 10) - 7125) / 125;
		else
			return (mv - 700) / 16;
	}
	return 0;
}

uint16_t VID_to_mV(uint8_t VID) {
	if (Platform.CPU.Vendor == 0x756E6547)
	{
		if ((Platform.CPU.Family == 0x06) && (Platform.CPU.Model == 0x17))
			return (((int)VID * 125) + 7125) / 10;
		else
			return (((int)VID * 16) + 700);
	}
	return 0;
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
	
	char ssdt_pct[] =
	{
		0x08, 0x5F, 0x50, 0x43, 0x54, 0x12, 0x2C,	/* 00000030    "0._PCT.," */
		0x02, 0x11, 0x14, 0x0A, 0x11, 0x82, 0x0C, 0x00,	/* 00000038    "........" */
		0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* 00000040    "........" */
		0x00, 0x00, 0x00, 0x00, 0x79, 0x00, 0x11, 0x14,	/* 00000048    "....y..." */
		0x0A, 0x11, 0x82, 0x0C, 0x00, 0x7F, 0x00, 0x00,	/* 00000050    "........" */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* 00000058    "........" */
		0x00, 0x79, 0x00
	};

	if (Platform.CPU.Vendor != 0x756E6547) {
		verbose ("Not an Intel platform: P-States will not be generated !!!\n");
		return NULL;
	}
	
	if (!(platformCPUFeature(CPU_FEATURE_MSR))) {
		verbose ("Unsupported CPU: P-States will not be generated !!!\n");
		return NULL;
	}
	
	if (acpi_cpu_count == 0)
		get_acpi_cpu_names((void*)dsdt, dsdt->Length);
	
	if (acpi_cpu_count > 0)
	{
		
		struct p_state initial, maximum, minimum, p_states[32];
		uint8_t p_states_count = 0;

		//TODO: make use of minimum & friends instead, (busratios, freqs ...)
		int busratio_min = Platform.CPU.MinRatio;
		int busratio_max = Platform.CPU.MaxRatio;
		
		int freq_max = ((Platform.CPU.FSBFrequency / 1000000) * busratio_max) / 10;
		int freq_min = ((Platform.CPU.FSBFrequency / 1000000) * busratio_min) / 10;
		int p = ((busratio_max - busratio_min) * 2) / 10;
		int f_step = (freq_max - freq_min) / p;
		if (Platform.CPU.SLFM) freq_min = (busratio_min * f_step) / 10;
		freq_max = (((Platform.CPU.FSBFrequency / 1000000) + 1) * busratio_max) / 10;

		//valv: minimum instead of current
		//initial.Control = rdmsr64(MSR_IA32_PERF_STATUS);
		initial.Control = ((rdmsr64(MSR_IA32_PERF_STATUS) >> 48) & 0x1F3F);
		maximum.Control = ((rdmsr64(MSR_IA32_PERF_STATUS) >> 32) & 0x1F3F) | (0x4000 * Platform.CPU.MaxDiv);

		maximum.FID = (busratio_max / 0xA) & 0x1F | (0x40 * Platform.CPU.MaxDiv);
		maximum.CID = ((maximum.FID & 0x1F) << 1) | Platform.CPU.MaxDiv;
		
		// current isn't always minimum
		//minimum.FID = ((rdmsr64(MSR_IA32_PERF_STATUS) >> 24) & 0x1F) | (0x80 * cpu_dynamic_fsb);
		minimum.FID = ((rdmsr64(MSR_IA32_PERF_STATUS) >> 56) & 0x1F) | (0x80 * Platform.CPU.SLFM);
		// this may not be valid when slfm (did) is enabled. Rely on vmin instead
		minimum.VID = ((rdmsr64(MSR_IA32_PERF_STATUS) >> 48) & 0x3F);

		int umaxVolt, uminVolt, moreVolt, lessVolt, vstate;
		int maxVolt_limit = VID_to_mV(maximum.VID) + 250;
		int minVolt_limit = VID_to_mV(minimum.VID) - 250;
		uint8_t vstep = 0;

		if((getIntForKey(kmaxVoltage, &umaxVolt, &bootInfo->bootConfig)) && (umaxVolt <= maxVolt_limit))
		{
			uint8_t vmax = mV_to_VID(umaxVolt);
			maximum.Control = (((busratio_max / 0xA) & 0x1F) * 0x100 | vmax) | (0x4000 * Platform.CPU.MaxDiv);
			maximum.VID = vmax & 0x3F;
		}
		if((getIntForKey(kminVoltage, &uminVolt, &bootInfo->bootConfig)) && (uminVolt >= minVolt_limit))
		{
			uint8_t vmin = mV_to_VID(uminVolt);
			minimum.VID = vmin & 0x3F;
		}
		
		if (minimum.FID == 0) 
		{
			uint64_t msr;
			uint8_t i;
			// Probe for lowest fid
			// this is for requested FID, and not forcibly the lowest
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

		minimum.CID = ((minimum.FID & 0x1F) << 1) >> Platform.CPU.SLFM;

		// Sanity check
		if (maximum.CID < minimum.CID)
		{
			DBG("Insane FID values!");
			p_states_count = 1;
		}
		else
		{
			// Print the voltage to be used
			int minVoltage = VID_to_mV(minimum.VID);
			int maxVoltage = VID_to_mV(maximum.VID);
			verbose("Voltage: min= %dmV, max= %dmV\n", minVoltage, maxVoltage);

			// Finalize P-States
			// Find how many P-States machine supports
			
			p_states_count = maximum.CID - minimum.CID + 1;
			
			if (p_states_count > 32) 
				p_states_count = 32;
		
			uint8_t vidstep;
			uint8_t i = 0, u, invalid = 0;

			vidstep = ((maximum.VID << 2) - (minimum.VID << 2)) / (p_states_count - 1);
			for (u = 0; u < p_states_count; u++) 
			{
				i = u - invalid;
				
				p_states[i].CID = maximum.CID - u;
				p_states[i].FID = (p_states[i].CID >> 1);

				if (i && p_states[i].FID < 0x6)
				{
					if (Platform.CPU.SLFM)
					{
						p_states[i].FID = (p_states[i].FID << 1) | 0x80;
					}
					// intel doesn't allow FIDs lower that 6 on core, core2 and eventually core-i
					else invalid++;
				}

				// 6.5 isn't selectable
				else if (Platform.CPU.MaxDiv && p_states[i].FID > 0x6)
				{
					// first state only!?
					//p_states[0].FID = p_states[0].FID | (0x40 * (p_states[0].CID & 0x1));
					p_states[i].FID = p_states[i].FID | (0x40 * (p_states[i].CID & 0x1));
				}
				
				if (i && p_states[i].FID == p_states[i-1].FID)
					invalid++;
				
				p_states[i].VID = ((maximum.VID << 2) - (vidstep * u)) >> 2;

				if(getIntForKey(klessVoltage, &lessVolt, &bootInfo->bootConfig))
				{
					vstate = (VID_to_mV(p_states[i].VID)) - lessVolt;
					vstep = mV_to_VID(vstate);
					p_states[i].VID = vstep & 0x3F;
				}
				else if(getIntForKey(kmoreVoltage, &moreVolt, &bootInfo->bootConfig))
				{
					vstate = (VID_to_mV(p_states[i].VID)) + moreVolt;
					vstep = mV_to_VID(vstate);
					p_states[i].VID = vstep & 0x3F;
				}

				uint32_t multiplier = p_states[i].FID & 0x1f;		// = 0x08
				bool half = p_states[i].FID & 0x40;					// = 0x01
				bool dfsb = p_states[i].FID & 0x80;					// = 0x00
				uint32_t fsb = Platform.CPU.FSBFrequency / 1000000; // = 400
				uint32_t halffsb = (fsb + 1) >> 1;					// = 200
				uint32_t frequency = (multiplier * fsb);			// = 3200

				p_states[i].Frequency = (frequency + (half * halffsb)) >> dfsb;	// = 3200 + 200 = 3400
			}
			p_states_count -= invalid;
		}
		
		//valv: Silly! Isn't it? :)
		int pstates = 0;
		if (getIntForKey(kpstates, &pstates, &bootInfo->bootConfig) && pstates < p_states_count && pstates > 0)
		{
			verbose("P-State: Only the %d highest states will be added\n", pstates);
			pstates = pstates - 1;
		}

		int pstart = 0;
		if (getIntForKey(kpstart, &pstart, &bootInfo->bootConfig) && pstart < p_states_count && pstart > 0)
		{
			const char *newratio;
			int len;
			if (getValueForKey(kbusratio, &newratio, &len, &bootInfo->bootConfig))
			{
				verbose("P-State: Already using bus-ratio injection! PStart key will have no effect.\n");
				pstart = 0;
			}
			else verbose("P-State: Starting from state P%d\n", pstart);
		}

		// Generating SSDT
		if (p_states_count > 0)
		{	
			int i;
			
			struct aml_chunk* root = aml_create_node(NULL);
			aml_add_buffer(root, ssdt_header, sizeof(ssdt_header)); // SSDT header
			struct aml_chunk* scop = aml_add_scope(root, "\\_PR_");
			
			aml_add_buffer(scop, ssdt_pct, sizeof(ssdt_pct));
			
			struct aml_chunk* name_psd = aml_add_name(scop, "PSD_");
			struct aml_chunk* pack_psd = aml_add_package(name_psd);
			struct aml_chunk* psd = aml_add_package(pack_psd);
			aml_add_byte(psd, 0x05);
			aml_add_byte(psd, 0x00);
			aml_add_dword(psd, 0x00);
			aml_add_dword(psd, 0xfc);
			aml_add_dword(psd, Platform.CPU.NoCores);
			
			struct aml_chunk* name_pss = aml_add_name(scop, "PSS_");
			struct aml_chunk* pack_pss = aml_add_package(name_pss);

//valv: this should not be
/*			if(Platform.CPU.Turbo)
			{
				int turbo_one = Platform.CPU.Tone;
				int turbo_two = Platform.CPU.Ttwo;
				int turbo_thr = Platform.CPU.Tthr;
				int turbo_for = Platform.CPU.Tfor;

				uint8_t TFIDone = (turbo_one & 0x1F);
				uint8_t TFIDtwo = (turbo_two & 0x1F);
				uint8_t TFIDthr = (turbo_thr & 0x1F);
				uint8_t TFIDfor = (turbo_for & 0x1F);

				uint32_t fsb = Platform.CPU.FSBFrequency / 1000000;
				uint32_t tfreq_one = (TFIDone * fsb);
				uint32_t tfreq_two = (TFIDtwo * fsb);
				uint32_t tfreq_thr = (TFIDthr * fsb);
				uint32_t tfreq_for = (TFIDfor * fsb);

				if(tfreq_one > 0)
				{
					struct aml_chunk* psturbo1 = aml_add_package(pack_pss);
					aml_add_dword(psturbo1, tfreq_one);
					aml_add_dword(psturbo1, 0x00000000); // Power
					aml_add_dword(psturbo1, 0x0000000A); // Latency
					aml_add_dword(psturbo1, 0x0000000A); // Latency
					aml_add_dword(psturbo1, TFIDone);
					aml_add_dword(psturbo1, TFIDone);
					verbose("P-State: Added [TurboFreq %d MHz, FID 0x%x]\n", tfreq_one, TFIDone);
				}

				if((tfreq_one != tfreq_two) && (tfreq_two > 0))
				{
					struct aml_chunk* psturbo2 = aml_add_package(pack_pss);
					aml_add_dword(psturbo2, tfreq_two);
					aml_add_dword(psturbo2, 0x00000000); // Power
					aml_add_dword(psturbo2, 0x0000000A); // Latency
					aml_add_dword(psturbo2, 0x0000000A); // Latency
					aml_add_dword(psturbo2, TFIDtwo);
					aml_add_dword(psturbo2, TFIDtwo);
					verbose("P-State: Added [TurboFreq %d MHz, FID 0x%x]\n", tfreq_two, TFIDtwo);
				}

				if((tfreq_two != tfreq_thr) && (tfreq_thr > 0))
				{
					struct aml_chunk* psturbo3 = aml_add_package(pack_pss);
					aml_add_dword(psturbo3, tfreq_thr);
					aml_add_dword(psturbo3, 0x00000000); // Power
					aml_add_dword(psturbo3, 0x0000000A); // Latency
					aml_add_dword(psturbo3, 0x0000000A); // Latency
					aml_add_dword(psturbo3, TFIDthr);
					aml_add_dword(psturbo3, TFIDthr);
					verbose("P-State: Added [TurboFreq %d MHz, FID 0x%x]\n", tfreq_thr, TFIDthr);
				}

				if((tfreq_thr != tfreq_for) && (tfreq_for > 0))
				{
					struct aml_chunk* psturbo4 = aml_add_package(pack_pss);
					aml_add_dword(psturbo4, tfreq_for);
					aml_add_dword(psturbo4, 0x00000000); // Power
					aml_add_dword(psturbo4, 0x0000000A); // Latency
					aml_add_dword(psturbo4, 0x0000000A); // Latency
					aml_add_dword(psturbo4, TFIDfor);
					aml_add_dword(psturbo4, TFIDfor);
					verbose("P-State: Added [TurboFreq %d MHz, FID 0x%x]\n", tfreq_for, TFIDfor);
				}
			}
*/
			for (i = pstart; i < p_states_count; i++)
			{
				if ((p_states[i].Frequency <= freq_max) && (p_states[i].Frequency >= freq_min))
				{
					//valv: inspired from cparm's pss-drop; coded by me ;)
					if ((i > pstates) && (pstates > 0))
					{
						if(Platform.CPU.ISerie) verbose("P-State: [Frequency %d MHz, FID 0x%x] is the %dth state. Removed!\n",
								p_states[i].Frequency, p_states[i].FID, (i+1));
						else verbose("P-State: [Frequency %d MHz, FID 0x%x, VID 0x%x] is the %dth state. Removed!\n",
								p_states[i].Frequency, p_states[i].FID, p_states[i].VID, (i+1));
					}
					else
					{
						struct aml_chunk* pstt = aml_add_package(pack_pss);

						aml_add_dword(pstt, p_states[i].Frequency);
						aml_add_dword(pstt, 0x00000000); // Power
						aml_add_dword(pstt, 0x0000000A); // Latency
						aml_add_dword(pstt, 0x0000000A); // Latency
						if(Platform.CPU.ISerie)
						{
							aml_add_dword(pstt, p_states[i].FID);
							aml_add_dword(pstt, p_states[i].FID);
							verbose("P-State: Added [Frequency %d MHz, FID 0x%x]\n", p_states[i].Frequency, p_states[i].FID);
						}
						else
						{
							aml_add_dword(pstt, p_states[i].Control);
							aml_add_dword(pstt, p_states[i].Control);
							//aml_add_dword(pstt, i+1); // Status
							verbose("P-State: Added [Frequency %d MHz, FID 0x%x, VID 0x%x]\n",
								p_states[i].Frequency, p_states[i].FID, p_states[i].VID);
						}
					}
				}
				else
				{
					if(Platform.CPU.ISerie) verbose("P-State: [Frequency %d MHz, FID 0x%x] is over the limit. Removed!\n",
							p_states[i].Frequency, p_states[i].FID);
					else verbose("P-State: [Frequency %d MHz, FID 0x%x, VID 0x%x] is over the limit. Removed!\n",
							p_states[i].Frequency, p_states[i].FID, p_states[i].VID);
				}
			}
			
			struct aml_chunk* name_ppc = aml_add_name(scop, "PPC_");
			/*struct aml_chunk* pack_ppc = aml_add_package(name_ppc);
			struct aml_chunk* ppc = aml_add_package(pack_ppc);*/
			aml_add_byte(name_ppc, 0x00);
			
			// Add aliaces
			for (i = 0; i < acpi_cpu_count; i++) 
			{
				char name[9];
				sprintf(name, "_PR_%c%c%c%c", acpi_cpu_name[i][0], acpi_cpu_name[i][1], acpi_cpu_name[i][2], acpi_cpu_name[i][3]);
				
				scop = aml_add_scope(root, name);
				aml_add_alias(scop, "PCT_", "_PCT");
				aml_add_alias(scop, "PSD_", "_PSD");
				aml_add_alias(scop, "PSS_", "_PSS");
				aml_add_alias(scop, "PPC_", "_PPC");
			}
			
			aml_calculate_size(root);
			
			struct acpi_2_ssdt *ssdt = (struct acpi_2_ssdt *)AllocateKernelMemory(root->Size);
			
			aml_write_node(root, (void*)ssdt, 0);
			
			ssdt->Length = root->Size;
			ssdt->Checksum = 0;
			ssdt->Checksum = 256 - checksum8(ssdt, ssdt->Length);
                        
                        aml_destroy_node(root);
                        			
			//verbose ("SSDT with CPU P-States generated successfully\n");
			
			return ssdt;
		}
	}
	else {
		verbose ("ACPI CPUs not found: P-States not generated !!!\n");
	}
	
	return NULL;
}

void *loadSSDTTable(int ssdt_number)
{
	void *tableAddr;
	int fd = -1;
	char dirspec[512];
	char filename[512];
	const char * overriden_pathname=NULL;
	int len=0;

	sprintf(filename, "SSDT-%d.aml", ssdt_number);
	
	// Check booting partition
	
	// Rek: if user specified a full path name then take it in consideration
	if (getValueForKey(kSSDT, &overriden_pathname, &len, &bootInfo->bootConfig))
	{
		sprintf(dirspec, "%s-%d.aml", overriden_pathname, ssdt_number); // start searching root
		//printf("Using custom %s path %s\n", key, dirspec);
		//getc();
	}
	else
		sprintf(dirspec, "/%s", filename); // start searching root
	
	fd=open (dirspec,0);
	
	if (fd<0)
	{	// Check Extra on booting partition
		//verbose("Searching for %s.aml file ...\n", key);
		sprintf(dirspec,"/Extra/%s",filename);
		fd=open (dirspec,0);
		if (fd<0)
		{	// Fall back to booter partition
			sprintf(dirspec,"bt(0,0)/Extra/%s",filename);
			fd=open (dirspec,0);
			if (fd<0)
			{
				//verbose("SSDT Table not found: %s\n", filename);
				return NULL;
			}
		}
	}
	
	tableAddr=(void*)AllocateKernelMemory(file_size (fd));
	if (tableAddr)
	{
		if (read (fd, tableAddr, file_size (fd))!=file_size (fd))
		{
			printf("Couldn't read table %s\n",dirspec);
			free (tableAddr);
			close (fd);
			return NULL;
		}
		
		DBG("Table %s read and stored at: %x\n", dirspec, tableAddr);
		close (fd);
		return tableAddr;
	}
	
	printf("Couldn't allocate memory for table %s\n", dirspec);
	close (fd);
	
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

struct acpi_2_fadt *patch_fadt(struct acpi_2_fadt *fadt, void *new_dsdt, bool UpdateFADT)
{
    extern void setupSystemType(); 
	
	struct acpi_2_fadt *fadt_mod;
	struct acpi_2_fadt *fadt_file = (struct acpi_2_fadt *)loadACPITable(kFADT);
	bool fadt_rev2_needed = false;
	bool fix_restart;
	const char * value;

	// Restart Fix
	if (Platform.CPU.Vendor == 0x756E6547) {	/* Intel */
		fix_restart = true;
		getBoolForKey(kRestartFix, &fix_restart, &bootInfo->bootConfig);
	} else {
		verbose ("Not an Intel platform: Restart Fix not applied !!!\n");
		fix_restart = false;
	}

	if (fix_restart)
		fadt_rev2_needed = true;

	// Allocate new fadt table
	if ((UpdateFADT) && (((fadt_file) && (fadt_file->Length < sizeof(struct acpi_2_fadt))) ||
						 ((!fadt_file) && (fadt->Length < sizeof(struct acpi_2_fadt)))))
	{
		fadt_mod=(struct acpi_2_fadt *)AllocateKernelMemory(sizeof(struct acpi_2_fadt));

		if (fadt_file)
			memcpy(fadt_mod, fadt_file, fadt_file->Length);
		else
			memcpy(fadt_mod, fadt, fadt->Length);

		fadt_mod->Length = sizeof(struct acpi_2_fadt);
		fadt_mod->Revision = 0x04; // FADT rev 4
		fadt_mod->RESET_REG = FillGASStruct(0, 0);
		fadt_mod->RESET_VALUE = 0;
		fadt_mod->Reserved2[0] = 0;
		fadt_mod->Reserved2[1] = 0;
		fadt_mod->Reserved2[2] = 0;
		fadt_mod->X_PM1a_EVT_BLK = FillGASStruct(fadt_mod->PM1a_EVT_BLK, fadt_mod->PM1_EVT_LEN);
		fadt_mod->X_PM1b_EVT_BLK = FillGASStruct(fadt_mod->PM1b_EVT_BLK, fadt_mod->PM1_EVT_LEN);
		fadt_mod->X_PM1a_CNT_BLK = FillGASStruct(fadt_mod->PM1a_CNT_BLK, fadt_mod->PM1_CNT_LEN);
		fadt_mod->X_PM1b_CNT_BLK = FillGASStruct(fadt_mod->PM1b_CNT_BLK, fadt_mod->PM1_CNT_LEN);
		fadt_mod->X_PM2_CNT_BLK = FillGASStruct(fadt_mod->PM2_CNT_BLK, fadt_mod->PM2_CNT_LEN);
		fadt_mod->X_PM_TMR_BLK = FillGASStruct(fadt_mod->PM_TMR_BLK, fadt_mod->PM_TMR_LEN);
		fadt_mod->X_GPE0_BLK = FillGASStruct(fadt_mod->GPE0_BLK, fadt_mod->GPE0_BLK_LEN);
		fadt_mod->X_GPE1_BLK = FillGASStruct(fadt_mod->GPE1_BLK, fadt_mod->GPE1_BLK_LEN);
		verbose("Converted ACPI V%d FADT to ACPI V4 FADT\n", (fadt_file) ? fadt_file->Revision : fadt->Revision);
	} else {
		if (((!fadt_file) && ((fadt->Length < 0x84) && (fadt_rev2_needed))) ||
			 ((fadt_file) && ((fadt_file->Length < 0x84) && (fadt_rev2_needed))))
		{
			fadt_mod=(struct acpi_2_fadt *)AllocateKernelMemory(0x84);

			if (fadt_file)
				memcpy(fadt_mod, fadt_file, fadt_file->Length);
			else
				memcpy(fadt_mod, fadt, fadt->Length);

			fadt_mod->Length   = 0x84;
			fadt_mod->Revision = 0x02; // FADT rev 2 (ACPI 1.0B MS extensions)
		}
		else
		{
			if (fadt_file)
			{
				fadt_mod=(struct acpi_2_fadt *)AllocateKernelMemory(fadt_file->Length);
				memcpy(fadt_mod, fadt_file, fadt_file->Length);
			} else {
				fadt_mod=(struct acpi_2_fadt *)AllocateKernelMemory(fadt->Length);
				memcpy(fadt_mod, fadt, fadt->Length);
			}
		}
	}
	// Determine system type / PM_Model
	if ( (value=getStringForKey(kSystemType, &bootInfo->bootConfig))!=NULL)
	{
	  if (Platform.Type > 6)  
	  {
	    if(fadt_mod->Preferred_PM_Profile<=6)
	      Platform.Type = fadt_mod->Preferred_PM_Profile; // get the fadt if correct
	    else 
	      Platform.Type = 1;		/* Set a fixed value (Desktop) */
	    verbose("Error: system-type must be 0..6. Defaulting to %d !\n", Platform.Type);
	  }
	  else
	    Platform.Type = (unsigned char) strtoul(value, NULL, 10);
	}
	// Set Preferred_PM_Profile from System-type only if user wanted this value to be forced
	if (fadt_mod->Preferred_PM_Profile != Platform.Type) 
	{
	    if (value) 
	      { // user has overriden the SystemType so take care of it in FACP
		verbose("FADT: changing Preferred_PM_Profile from 0x%02x to 0x%02x\n", fadt_mod->Preferred_PM_Profile, Platform.Type);
		fadt_mod->Preferred_PM_Profile = Platform.Type;
	    }
	    else
	    { // Preferred_PM_Profile has a different value and no override has been set, so reflect the user value to ioregs
	      Platform.Type = fadt_mod->Preferred_PM_Profile <= 6 ? fadt_mod->Preferred_PM_Profile : 1;
	    }  
	}
	// We now have to write the systemm-type in ioregs: we cannot do it before in setupDeviceTree()
	// because we need to take care of facp original content, if it is correct.
	setupSystemType();

	// Patch FADT to fix restart
	if (fix_restart)
	{
		fadt_mod->Flags|= 0x400;
		fadt_mod->RESET_REG = FillGASStruct(0x0cf9, 1);
		fadt_mod->RESET_VALUE = 0x06;
		
		verbose("FADT: Restart Fix applied !\n");
	}

	// Patch FACS Address
	fadt_mod->FIRMWARE_CTRL=(uint32_t)fadt->FIRMWARE_CTRL;
	if ((uint32_t)(&(fadt_mod->X_FIRMWARE_CTRL))-(uint32_t)fadt_mod+8<=fadt_mod->Length)
		fadt_mod->X_FIRMWARE_CTRL=(uint32_t)fadt->FIRMWARE_CTRL;

	// Patch DSDT Address if we have one loaded
	if(new_dsdt)
	{
		fadt_mod->DSDT=(uint32_t)new_dsdt;
		if ((uint32_t)(&(fadt_mod->X_DSDT))-(uint32_t)fadt_mod+8<=fadt_mod->Length)
			fadt_mod->X_DSDT=(uint32_t)new_dsdt;
	}


	// Correct the checksum
	fadt_mod->Checksum=0;
	fadt_mod->Checksum=256-checksum8(fadt_mod,fadt_mod->Length);

	return fadt_mod;
}
#if UNUSED
/* Setup ACPI without replacing DSDT. */
int setupAcpiNoMod()
{
//	addConfigurationTable(&gEfiAcpiTableGuid, getAddressOfAcpiTable(), "ACPI");
//	addConfigurationTable(&gEfiAcpi20TableGuid, getAddressOfAcpi20Table(), "ACPI_20");
	/* XXX aserebln why uint32 cast if pointer is uint64 ? */
	acpi10_p = (uint32_t)getAddressOfAcpiTable();
	acpi20_p = (uint32_t)getAddressOfAcpi20Table();
	addConfigurationTable(&gEfiAcpiTableGuid, &acpi10_p, "ACPI");
	if(acpi20_p) addConfigurationTable(&gEfiAcpi20TableGuid, &acpi20_p, "ACPI_20");
	return 1;
}
#endif

/* Setup ACPI. Replace DSDT if DSDT.aml is found */
int setupAcpi(void)
{
	int version;
	void *new_dsdt=NULL, *new_hpet=NULL, *new_sbst=NULL, *new_ecdt=NULL, *new_asft=NULL, *new_dmar=NULL, *new_apic=NULL, *new_mcfg=NULL;//, *new_ssdt[14];

	struct acpi_2_ssdt *new_ssdt[16]; // 2 additional tables for pss & cst
	struct acpi_2_fadt *fadt; // will be used in CST generator

	bool oem_dsdt=false, oem_ssdt=false, oem_hpet=false, oem_sbst=false, oem_ecdt=false, oem_asft=false, oem_dmar=false, oem_apic=false, oem_mcfg=false;
	bool drop_ssdt=false, drop_hpet=false, drop_slic=false, drop_sbst=false, drop_ecdt=false, drop_asft=false, drop_dmar=false;
	bool update_acpi=false, gen_xsdt=false;
	bool hpet_replaced=false, sbst_replaced=false, ecdt_replaced=false, asft_replaced=false, dmar_replaced=false, apic_replaced=false, mcfg_replaced=false;
	bool hpet_added=false, sbst_added=false, ecdt_added=false, asft_added=false, dmar_added=false, apic_added=false, mcfg_added=false;
	bool gen_cst=false, gen_pss=false;

	int curssdt=0, loadtotssdt=0, totssdt=0, newtotssdt=0;

	{
		bool tmpval, tmpval2, tmpval3, tmpval4, tmpval6;

		drop_ssdt = getBoolForKey(kDropSSDT, &tmpval, &bootInfo->bootConfig)&&tmpval;
		drop_hpet = getBoolForKey(kDropHPET, &tmpval, &bootInfo->bootConfig)&&tmpval;
		drop_slic = getBoolForKey(kDropSLIC, &tmpval, &bootInfo->bootConfig)&&tmpval;
		drop_sbst = getBoolForKey(kDropSBST, &tmpval, &bootInfo->bootConfig)&&tmpval;
		drop_ecdt = getBoolForKey(kDropECDT, &tmpval, &bootInfo->bootConfig)&&tmpval;
		drop_asft = getBoolForKey(kDropASFT, &tmpval, &bootInfo->bootConfig)&&tmpval;
		drop_dmar = getBoolForKey(kDropDMAR, &tmpval, &bootInfo->bootConfig)&&tmpval;

		oem_dsdt = getBoolForKey(kOEMDSDT, &tmpval, &bootInfo->bootConfig)&&tmpval;
		oem_ssdt = getBoolForKey(kOEMSSDT, &tmpval, &bootInfo->bootConfig)&&tmpval;
		oem_hpet = getBoolForKey(kOEMHPET, &tmpval, &bootInfo->bootConfig)&&tmpval;
		oem_sbst = getBoolForKey(kOEMSBST, &tmpval, &bootInfo->bootConfig)&&tmpval;
		oem_ecdt = getBoolForKey(kOEMECDT, &tmpval, &bootInfo->bootConfig)&&tmpval;
		oem_asft = getBoolForKey(kOEMASFT, &tmpval, &bootInfo->bootConfig)&&tmpval;
		oem_dmar = getBoolForKey(kOEMDMAR, &tmpval, &bootInfo->bootConfig)&&tmpval;
		oem_apic = getBoolForKey(kOEMAPIC, &tmpval, &bootInfo->bootConfig)&&tmpval;
		oem_mcfg = getBoolForKey(kOEMMCFG, &tmpval, &bootInfo->bootConfig)&&tmpval;

		gen_cst = (getBoolForKey(kGenerateCStates, &tmpval, &bootInfo->bootConfig)&&tmpval)
				| (getBoolForKey(kEnableC2State, &tmpval2, &bootInfo->bootConfig)&&tmpval2)
				| (getBoolForKey(kEnableC3State, &tmpval3, &bootInfo->bootConfig)&&tmpval3)
				| (getBoolForKey(kEnableC4State, &tmpval4, &bootInfo->bootConfig)&&tmpval4)
				| (getBoolForKey(kEnableC6State, &tmpval6, &bootInfo->bootConfig)&&tmpval6);

		gen_pss = getBoolForKey(kGeneratePStates, &tmpval, &bootInfo->bootConfig)&&tmpval;
		update_acpi = getBoolForKey(kUpdateACPI, &tmpval, &bootInfo->bootConfig)&&tmpval;
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

	if (gen_cst || gen_pss) oem_ssdt = false;

	if (!oem_ssdt)
	{
		for (curssdt=0;curssdt<14;curssdt++)
		{
			new_ssdt[curssdt]=loadSSDTTable(curssdt);
			if (new_ssdt[curssdt])
				loadtotssdt++;
		}
		curssdt=0;
	}
/*
	if (!new_dsdt)
		return setupAcpiNoMod();
*/
	DBG("New ACPI tables Loaded in memory\n");
			
	// Do the same procedure for both versions of ACPI
	for (version=0; version<2; version++) {
		struct acpi_2_rsdp *rsdp, *rsdp_mod, *rsdp_conv=(struct acpi_2_rsdp *)0;
		struct acpi_2_rsdt *rsdt, *rsdt_mod;
		struct acpi_2_xsdt *xsdt_conv = (struct acpi_2_xsdt *)0;
		int rsdplength;

		// Find original rsdp
		rsdp=(struct acpi_2_rsdp *)(version?getAddressOfAcpi20Table():getAddressOfAcpiTable());
		if ((update_acpi) && (rsdp->Revision == 0))
		{
			rsdp_conv = (struct acpi_2_rsdp *)AllocateKernelMemory(sizeof(struct acpi_2_rsdp));
			memcpy(rsdp_conv, rsdp, 20);

			/* Add/change fields */
			strncpy(rsdp->OEMID, "Apple ", 6);
			rsdp_conv->Revision = 2; /* ACPI version 3 */
			rsdp_conv->Length = sizeof(struct acpi_2_rsdp);

			/* Correct checksums */
			rsdp_conv->Checksum = 0;
			rsdp_conv->Checksum = 256-checksum8(rsdp_conv, 20);
			rsdp_conv->ExtendedChecksum = 0;
			rsdp_conv->ExtendedChecksum = 256-checksum8(rsdp_conv, rsdp_conv->Length);

			rsdp = rsdp_conv;

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
			uint32_t *rsdt_entries;
			int rsdt_entries_num;
			int dropoffset=0, i;
			
			rsdt_mod=(struct acpi_2_rsdt *)AllocateKernelMemory(rsdt->Length); 
			memcpy (rsdt_mod, rsdt, rsdt->Length);
			rsdp_mod->RsdtAddress=(uint32_t)rsdt_mod;
			rsdt_entries_num=(rsdt_mod->Length-sizeof(struct acpi_2_rsdt))/4;
			rsdt_entries=(uint32_t *)(rsdt_mod+1);

			if (gen_xsdt)
			{
				uint64_t *xsdt_conv_entries;

				xsdt_conv=(struct acpi_2_xsdt *)AllocateKernelMemory(sizeof(struct acpi_2_xsdt)+(rsdt_entries_num * 8));
				memcpy(xsdt_conv, rsdt, sizeof(struct acpi_2_rsdt));

				xsdt_conv->Signature[0] = 'X';
				xsdt_conv->Signature[1] = 'S';
				xsdt_conv->Signature[2] = 'D';
				xsdt_conv->Signature[3] = 'T';
				xsdt_conv->Length = sizeof(struct acpi_2_xsdt)+(rsdt_entries_num * 8);

				xsdt_conv_entries=(uint64_t *)(xsdt_conv+1);

				for (i=0;i<rsdt_entries_num;i++)
				{
					xsdt_conv_entries[i] = (uint64_t)rsdt_entries[i];
				}

				xsdt_conv->Checksum = 0;
				xsdt_conv->Checksum = 256-checksum8(xsdt_conv, xsdt_conv->Length);

				rsdp->XsdtAddress = (uint32_t)xsdt_conv;

				rsdp->ExtendedChecksum = 0;
				rsdp->ExtendedChecksum = 256-checksum8(rsdp, rsdp->Length);

				verbose("Converted RSDT table to XSDT table\n");
			}

			for (i=0;i<rsdt_entries_num;i++)
			{
				char *table=(char *)(rsdt_entries[i]);
				if (!table)
					continue;

				DBG("TABLE %c%c%c%c,",table[0],table[1],table[2],table[3]);

				rsdt_entries[i-dropoffset]=rsdt_entries[i];
				if (drop_ssdt && table[0]=='S' && table[1]=='S' && table[2]=='D' && table[3]=='T')
				{
					dropoffset++;
					continue;
				}
				if (drop_hpet && table[0]=='H' && table[1]=='P' && table[2]=='E' && table[3]=='T')
				{
					dropoffset++;
					continue;
				}			
				if (drop_slic && table[0]=='S' && table[1]=='L' && table[2]=='I' && table[3]=='C')
				{
					dropoffset++;
					continue;
				}
				if (drop_sbst && table[0]=='S' && table[1]=='B' && table[2]=='S' && table[3]=='T')
				{
					dropoffset++;
					continue;
				}
				if (drop_ecdt && table[0]=='E' && table[1]=='C' && table[2]=='D' && table[3]=='T')
				{
					dropoffset++;
					continue;
				}
				if (drop_asft && table[0]=='A' && table[1]=='S' && table[2]=='F' && table[3]=='!')
				{
					dropoffset++;
					continue;
				}
				if (drop_dmar && table[0]=='D' && table[1]=='M' && table[2]=='A' && table[3]=='R')
				{
					dropoffset++;
					continue;
				}
				if ((!(oem_hpet)) && table[0]=='H' && table[1]=='P' && table[2]=='E' && table[3]=='T')
				{
					DBG("HPET found\n");
					if (new_hpet)
					{
						rsdt_entries[i-dropoffset]=(uint32_t)new_hpet;
						hpet_replaced=true;
					}
					continue;
				}
				if ((!(oem_sbst)) && table[0]=='S' && table[1]=='B' && table[2]=='S' && table[3]=='T')
				{
					DBG("SBST found\n");
					if (new_sbst)
					{
						rsdt_entries[i-dropoffset]=(uint32_t)new_sbst;
						sbst_replaced=true;
					}
					continue;
				}
				if ((!(oem_ecdt)) && table[0]=='E' && table[1]=='C' && table[2]=='D' && table[3]=='T')
				{
					DBG("ECDT found\n");
					if (new_ecdt)
					{
						rsdt_entries[i-dropoffset]=(uint32_t)new_ecdt;
						ecdt_replaced=true;
					}
					continue;
				}
				if ((!(oem_asft)) && table[0]=='A' && table[1]=='S' && table[2]=='F' && table[3]=='!')
				{
					DBG("ASF! found\n");
					if (new_asft)
					{
						rsdt_entries[i-dropoffset]=(uint32_t)new_asft;
						asft_replaced=true;
					}
					continue;
				}
				if ((!(oem_dmar)) && table[0]=='D' && table[1]=='M' && table[2]=='A' && table[3]=='R')
				{
					DBG("DMAR found\n");
					if (new_dmar)
					{
						rsdt_entries[i-dropoffset]=(uint32_t)new_dmar;
						dmar_replaced=true;
					}
					continue;
				}
				if ((!(oem_apic)) && table[0]=='A' && table[1]=='P' && table[2]=='I' && table[3]=='C')
				{
					DBG("APIC found\n");
					if (new_apic)
					{
						rsdt_entries[i-dropoffset]=(uint32_t)new_apic;
						apic_replaced=true;
					}
					continue;
				}
				if ((!(oem_mcfg)) && table[0]=='M' && table[1]=='C' && table[2]=='F' && table[3]=='G')
				{
					DBG("MCFG found\n");
					if (new_mcfg)
					{
						rsdt_entries[i-dropoffset]=(uint32_t)new_mcfg;
						mcfg_replaced=true;
					}
					continue;
				}
				if ((!(oem_ssdt)) && table[0]=='S' && table[1]=='S' && table[2]=='D' && table[3]=='T')
				{
					DBG("SSDT %d found", curssdt);
					if (new_ssdt[curssdt])
					{
						DBG(" and replaced");
						rsdt_entries[i-dropoffset]=(uint32_t)new_ssdt[curssdt];
						totssdt++;
					}
					DBG("\n");
					curssdt++;
					continue;
				}
				if (table[0]=='D' && table[1]=='S' && table[2]=='D' && table[3]=='T')
				{
					DBG("DSDT found\n");
					rsdt_entries[i-dropoffset]=(uint32_t)new_dsdt;
					continue;

				} else {
					struct acpi_2_rsdt *t = (struct acpi_2_rsdt *)table;
					strncpy(t->OEMID, "Apple ", 6);
					strncpy(t->OEMTableId, MacModel, 8);
					t->OEMRevision = ModelRev;
					t->Checksum=0;
					t->Checksum=256-checksum8(t,t->Length);
				}
				if (table[0]=='F' && table[1]=='A' && table[2]=='C' && table[3]=='P')
				{
					struct acpi_2_fadt *fadt_mod;
					fadt=(struct acpi_2_fadt *)rsdt_entries[i];

					DBG("FADT found @%x, Length %d\n",fadt, fadt->Length);

					if (!fadt || (uint32_t)fadt == 0xffffffff || fadt->Length>0x10000)
					{
						printf("FADT incorrect. Not modified\n");
						continue;
					}
					
					fadt_mod = patch_fadt(fadt, new_dsdt, update_acpi);
					rsdt_entries[i-dropoffset]=(uint32_t)fadt_mod;

					if(!oem_ssdt)
					{
						// Generate _CST SSDT
						if ( gen_cst && (new_ssdt[loadtotssdt] = generate_cst_ssdt(fadt_mod)))
						{
							gen_cst= false;
							loadtotssdt++;
						}
					
						// Generating _PSS SSDT
						if (gen_pss && (new_ssdt[loadtotssdt] = generate_pss_ssdt((void*)fadt_mod->DSDT)))
						{
							gen_pss= false;
							loadtotssdt++;
						}
					}

					continue;
				}
			}
			DBG("\n");

			strncpy(rsdt_mod->OEMID, "Apple ", 6);
			strncpy(rsdt_mod->OEMTableId, MacModel, 8);
			rsdt_mod->OEMRevision = ModelRev;

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
				while ((totssdt < loadtotssdt) && (curssdt < 14))
				{
					if (new_ssdt[curssdt])
					{
						DBG("adding SSDT %d\n", curssdt);
						rsdt_entries[i-dropoffset]=(uint32_t)new_ssdt[curssdt];
						totssdt++;
						newtotssdt++;
						i++;
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

			rsdt_mod->Checksum=0;
			rsdt_mod->Checksum=256-checksum8(rsdt_mod,rsdt_mod->Length);

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

			DBG("XSDT @%x;%x, Length=%d\n", (uint32_t)(rsdp->XsdtAddress>>32),(uint32_t)rsdp->XsdtAddress, xsdt->Length);
			if (xsdt && (uint64_t)rsdp->XsdtAddress<0xffffffff && xsdt->Length<0x10000)
			{
				uint64_t *xsdt_entries;
				int xsdt_entries_num, i;
				int dropoffset=0;
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

				rsdp_mod->XsdtAddress=(uint32_t)xsdt_mod;
				xsdt_entries_num=(xsdt_mod->Length-sizeof(struct acpi_2_xsdt))/8;
				xsdt_entries=(uint64_t *)(xsdt_mod+1);
				for (i=0;i<xsdt_entries_num;i++)
				{
					char *table=(char *)((uint32_t)(xsdt_entries[i]));
					if (!table)
						continue;
					xsdt_entries[i-dropoffset]=xsdt_entries[i];
					if (drop_ssdt && table[0]=='S' && table[1]=='S' && table[2]=='D' && table[3]=='T')
					{
						dropoffset++;
						continue;
					}	
					if (drop_hpet && table[0]=='H' && table[1]=='P' && table[2]=='E' && table[3]=='T')
					{
						dropoffset++;
						continue;
					}
					if (drop_slic && table[0]=='S' && table[1]=='L' && table[2]=='I' && table[3]=='C')
					{
						dropoffset++;
						continue;
					}
					if (drop_sbst && table[0]=='S' && table[1]=='B' && table[2]=='S' && table[3]=='T')
					{
						dropoffset++;
						continue;
					}
					if (drop_ecdt && table[0]=='E' && table[1]=='C' && table[2]=='D' && table[3]=='T')
					{
						dropoffset++;
						continue;
					}					
					if (drop_asft && table[0]=='A' && table[1]=='S' && table[2]=='F' && table[3]=='!')
					{
						dropoffset++;
						continue;
					}
					if (drop_dmar && table[0]=='D' && table[1]=='M' && table[2]=='A' && table[3]=='R')
					{
						dropoffset++;
						continue;
					}					
					if ((!(oem_hpet)) && table[0]=='H' && table[1]=='P' && table[2]=='E' && table[3]=='T')
					{
						DBG("HPET found\n");
						if (new_hpet)
						{
							xsdt_entries[i-dropoffset]=(uint32_t)new_hpet;
							hpet_replaced=true;
						}
						continue;
					}					
					if ((!(oem_sbst)) && table[0]=='S' && table[1]=='B' && table[2]=='S' && table[3]=='T')
					{
						DBG("SBST found\n");
						if (new_sbst)
						{
							xsdt_entries[i-dropoffset]=(uint32_t)new_sbst;
							sbst_replaced=true;
						}
						continue;
					}					
					if ((!(oem_ecdt)) && table[0]=='E' && table[1]=='C' && table[2]=='D' && table[3]=='T')
					{
						DBG("ECDT found\n");
						if (new_ecdt)
						{
							xsdt_entries[i-dropoffset]=(uint32_t)new_ecdt;
							ecdt_replaced=true;
						}
						continue;
					}					
					if ((!(oem_asft)) && table[0]=='A' && table[1]=='S' && table[2]=='F' && table[3]=='!')
					{
						DBG("ASF! found\n");
						if (new_asft)
						{
							xsdt_entries[i-dropoffset]=(uint32_t)new_asft;
							asft_replaced=true;
						}
						continue;
					}					
					if ((!(oem_dmar)) && table[0]=='D' && table[1]=='M' && table[2]=='A' && table[3]=='R')
					{
						DBG("DMAR found\n");
						if (new_dmar)
						{
							xsdt_entries[i-dropoffset]=(uint32_t)new_dmar;
							dmar_replaced=true;
						}
						continue;
					}					
					if ((!(oem_apic)) && table[0]=='A' && table[1]=='P' && table[2]=='I' && table[3]=='C')
					{
						DBG("APIC found\n");
						if (new_apic)
						{
							xsdt_entries[i-dropoffset]=(uint32_t)new_apic;
							apic_replaced=true;
						}
						continue;
					}					
					if ((!(oem_mcfg)) && table[0]=='M' && table[1]=='C' && table[2]=='F' && table[3]=='G')
					{
						DBG("MCFG found\n");
						if (new_mcfg)
						{
							xsdt_entries[i-dropoffset]=(uint32_t)new_mcfg;
							mcfg_replaced=true;
						}
						continue;
					}					
					if ((!(oem_ssdt)) && table[0]=='S' && table[1]=='S' && table[2]=='D' && table[3]=='T')
					{
						DBG("SSDT %d found", curssdt);
						if (new_ssdt[curssdt])
						{
							DBG(" and replaced");
							xsdt_entries[i-dropoffset]=(uint32_t)new_ssdt[curssdt];
							totssdt++;
						}
						DBG("\n");
						curssdt++;
						continue;
					}					
					if (table[0]=='D' && table[1]=='S' && table[2]=='D' && table[3]=='T')
					{
						DBG("DSDT found\n");

						xsdt_entries[i-dropoffset]=(uint32_t)new_dsdt;

						DBG("TABLE %c%c%c%c@%x,",table[0],table[1],table[2],table[3],xsdt_entries[i]);
						
						continue;
					}

					struct acpi_2_xsdt* t = (struct acpi_2_xsdt*)table;
					strncpy(t->OEMID, "Apple ", 6);
					strncpy(t->OEMTableId, MacModel, 8);
					t->OEMRevision = ModelRev;

					if (table[0]=='F' && table[1]=='A' && table[2]=='C' && table[3]=='P')
					{
						struct acpi_2_fadt *fadt_mod;
						fadt=(struct acpi_2_fadt *)(uint32_t)xsdt_entries[i];

						DBG("FADT found @%x,%x, Length %d\n",(uint32_t)(xsdt_entries[i]>>32),fadt, 
								 fadt->Length);

						if (!fadt || (uint64_t)xsdt_entries[i] >= 0xffffffff || fadt->Length>0x10000)
						{
							verbose("FADT incorrect or after 4GB. Dropping XSDT\n");
							goto drop_xsdt;
						}

						fadt_mod = patch_fadt(fadt, new_dsdt, update_acpi);
						xsdt_entries[i-dropoffset]=(uint32_t)fadt_mod;

						DBG("TABLE %c%c%c%c@%x,",table[0],table[1],table[2],table[3],xsdt_entries[i]);

						if (!oem_ssdt)
						{
							// Generate _CST SSDT
							if ( gen_cst && (new_ssdt[loadtotssdt] = generate_cst_ssdt(fadt_mod)))
							{
								gen_cst= false;
								loadtotssdt++;
							}

							// Generating _PSS SSDT
							if (gen_pss && (new_ssdt[loadtotssdt] = generate_pss_ssdt((void*)fadt_mod->DSDT)))
							{
								gen_pss= false;
								loadtotssdt++;
							}
						}

						continue;
					}

					DBG("TABLE %c%c%c%c@%x,",table[0],table[1],table[2],table[3],xsdt_entries[i]);

				}

				strncpy(xsdt_mod->OEMID, "Apple ", 6);
				strncpy(xsdt_mod->OEMTableId, MacModel, 8);
				xsdt_mod->OEMRevision = ModelRev;

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
					while ((totssdt < loadtotssdt) && (curssdt < 14))
					{
						if (new_ssdt[curssdt])
						{
							DBG("adding SSDT %d\n", curssdt);
							xsdt_entries[i-dropoffset]=(uint32_t)new_ssdt[curssdt];
							totssdt++;
							newtotssdt++;
							i++;
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

				xsdt_mod->Checksum=0;
				xsdt_mod->Checksum=256-checksum8(xsdt_mod,xsdt_mod->Length);
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

		rsdp_mod->Checksum=0;
		rsdp_mod->Checksum=256-checksum8(rsdp_mod,20);

		DBG("New checksum %d\n", rsdp_mod->Checksum);

		if (version)
		{
			DBG("RSDP: Original extended checksum %d", rsdp_mod->ExtendedChecksum);

			rsdp_mod->ExtendedChecksum=0;
			rsdp_mod->ExtendedChecksum=256-checksum8(rsdp_mod,rsdp_mod->Length);

			DBG("New extended checksum %d\n", rsdp_mod->ExtendedChecksum);

		}
		
		verbose("Patched ACPI version %d DSDT\n", version+1);
		if (version)
		{
	/* XXX aserebln why uint32 cast if pointer is uint64 ? */
			acpi20_p = (uint32_t)rsdp_mod;
			acpi10_p = (uint32_t)rsdp_mod;
			addConfigurationTable(&gEfiAcpi20TableGuid, &acpi20_p, "ACPI_20");
			addConfigurationTable(&gEfiAcpiTableGuid, &acpi10_p, "ACPI");
		}
		else
		{
	/* XXX aserebln why uint32 cast if pointer is uint64 ? */
			acpi10_p = (uint32_t)rsdp_mod;
			addConfigurationTable(&gEfiAcpiTableGuid, &acpi10_p, "ACPI");
		}
	}
#if DEBUG_ACPI
	printf("Press a key to continue... (DEBUG_ACPI)\n");
	getc();
#endif
	return 1;
}
