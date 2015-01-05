/*
 * Copyright 2008 mackerintel
 *           2010 mojodojo,
 *           2012 slice
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
#include "state_generator.h"

#ifndef DEBUG_ACPI
#define DEBUG_ACPI 0
#endif

#if DEBUG_ACPI==2
#define DBG(x...)  {printf(x); sleep(1);}
#elif DEBUG_ACPI==1
#define DBG(x...)  printf(x)
#else
#define DBG(x...)  msglog(x)
#endif

// Slice: New signature compare function
boolean_t tableSign(void *table, const char *sgn)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		if ((((char *)table)[i] & ~0x20) != (sgn[i] & ~0x20))
		{
			return false;
		}
	}
	return true;
}

uint32_t EBDA_RANGE_START = EBDA_RANGE_MIN;

uint64_t acpi10_p = 0;
uint64_t acpi20_p = 0;

/* Gets the ACPI 1.0 RSDP address */
static struct acpi_2_rsdp *getAddressOfAcpiTable()
{
    /* Before searching the BIOS space we are supposed to search the first 1K of the EBDA */
    EBDA_RANGE_START = /* (uint32_t)swapUint16(*(uint16_t *)BDA_EBDA_START) << 4 */ EBDA_RANGE_MIN;
    verbose("ACPIpatcher: scanning EBDA [%08X-%08X] for RSDP 1.0... ", EBDA_RANGE_START, EBDA_RANGE_END);
    void *acpi_addr = (void*)EBDA_RANGE_START;
	for (; acpi_addr < (void*)EBDA_RANGE_END; acpi_addr++) {
		if (*(uint64_t *)acpi_addr == ACPI_SIGNATURE_UINT64_LE) {
            break;
		}
	}
    
    if (acpi_addr >= (void*)EBDA_RANGE_END) {
        verbose("Nothing found.\n");
        verbose("ACPIpatcher: scanning BIOS area [%08X-%08X] for RSDP 1.0...\n", ACPI_RANGE_START, ACPI_RANGE_END);
        acpi_addr = (void*)ACPI_RANGE_START;
        for (; acpi_addr < (void*)ACPI_RANGE_END; acpi_addr += 16) {
            if (*(uint64_t *)acpi_addr == ACPI_SIGNATURE_UINT64_LE) {
                break;
            }
        }
    } else {
        verbose("\n");
    }
    
    uint8_t csum = checksum8(acpi_addr, 20);
    
    if (csum == 0) {
        // Only return the table if it is a true version 1.0 table (Revision 0)
        if(((struct acpi_2_rsdp*)acpi_addr)->Revision == 0) {
            return acpi_addr;
        }
    }
	
	return NULL;
}

/* Gets the ACPI 2.0 RSDP address */
static struct acpi_2_rsdp *getAddressOfAcpi20Table()
{
    /* Before searching the BIOS space we are supposed to search the first 1K of the EBDA */
	EBDA_RANGE_START = /* (uint32_t)swapUint16(*(uint16_t *)BDA_EBDA_START) << 4 */ EBDA_RANGE_MIN;
    verbose("ACPIpatcher: scanning EBDA [%08X-%08X] for RSDP 2.0 or newer... ", EBDA_RANGE_START, EBDA_RANGE_END);
    void *acpi_addr = (void *)EBDA_RANGE_START;
	for (; acpi_addr < (void *)EBDA_RANGE_END; acpi_addr++) {
		if (*(uint64_t *)acpi_addr == ACPI_SIGNATURE_UINT64_LE) {
            break;
		}
	}
    
    if (acpi_addr >= (void *)EBDA_RANGE_END) {
        verbose("Nothing found.\n");
        verbose("ACPIpatcher: scanning BIOS area [%08X-%08X] for RSDP 2.0 or newer...\n", ACPI_RANGE_START, ACPI_RANGE_END);
        acpi_addr = (void *)ACPI_RANGE_START;
        for (; acpi_addr <= (void *)ACPI_RANGE_END; acpi_addr += 16) {
            if(*(uint64_t *)acpi_addr == ACPI_SIGNATURE_UINT64_LE) {
                break;
            }
        }
    } else {
        verbose("\n");
    }
    
    uint8_t csum = checksum8(acpi_addr, 20);
    
    /* Only assume this is a 2.0 or better table if the revision is greater than 0
     * NOTE: ACPI 3.0 spec only seems to say that 1.0 tables have revision 1
     * and that the current revision is 2.. I am going to assume that rev > 0 is 2.0.
     */
    
    if(csum == 0 && (((struct acpi_2_rsdp*)acpi_addr)->Revision > 0)) {
        uint8_t csum2 = checksum8(acpi_addr, sizeof(struct acpi_2_rsdp));
        if(csum2 == 0) {
            return acpi_addr;
        }
    }
	
	return NULL;
}

/* The folowing ACPI Table search algo. should be reused anywhere needed:*/
/* WARNING: outDirspec string will be overwritten by subsequent calls! */
int search_and_get_acpi_fd(const char *filename, const char **outDirspec)
{
	int fd = 0;
	static char dirSpec[512];

	// Try finding 'filename' in the usual places
	// Start searching any potential location for ACPI Table
    snprintf(dirSpec, sizeof(dirSpec), "%s", filename);
    fd = open(dirSpec, 0);
	if (fd < 0) {
		snprintf(dirSpec, sizeof(dirSpec), "/Extra/%s", filename); 
		fd = open(dirSpec, 0);
		if (fd < 0) {
            snprintf(dirSpec, sizeof(dirSpec), "/Extra/Acpi/%s", filename);
            fd = open(dirSpec, 0);
            if (fd < 0) {
                snprintf(dirSpec, sizeof(dirSpec), "bt(0,0)/Extra/%s", filename);
                fd = open(dirSpec, 0);
                if (fd < 0) {
                    snprintf(dirSpec, sizeof(dirSpec), "bt(0,0)/Extra/Acpi/%s", filename);
                    fd = open(dirSpec, 0);
                    if (fd < 0) {
                        // NOT FOUND:
                        dirSpec[0] = 0;
                    }
                }
			}
		}
	}

	if (outDirspec) *outDirspec = dirSpec; 
	return fd;
}

void *loadACPITable (const char *filename)
{
	void *tableAddr;
	const char *dirspec = NULL;

	int fd = search_and_get_acpi_fd(filename, &dirspec);

	if (fd >= 0)
	{
		tableAddr = (void *)AllocateKernelMemory(file_size(fd));
		if (tableAddr)
		{
			if (read(fd, tableAddr, file_size(fd)) != file_size(fd))
			{
				verbose("loadACPITable: Couldn't read table from: %s.\n", dirspec);
				free(tableAddr);
				close(fd);
				return NULL;
			}
			//verbose("ACPIpatcher: Table %s read and stored at: 0x%08X\n", dirspec, tableAddr);
			close(fd);
			return tableAddr;
		}
		close(fd);
		verbose("loadACPITable: Couldn't allocate memory for table: %s.\n", dirspec);
    }
    
	return NULL;
}
/***
uint8_t	acpi_cpu_count = 0;
char *acpi_cpu_name[32];
uint32_t acpi_cpu_p_blk = 0;

void get_acpi_cpu_names(unsigned char *dsdt, uint32_t length)
{
	uint32_t i;

	verbose("ACPIpatcher: start finding cpu names. Length %d\n", length);

	for (i=0; i<length-7; i++)
	{
		if (dsdt[i] == 0x5B && dsdt[i+1] == 0x83) // ProcessorOP
		{
			verbose("ACPIpatcher: DSDT[%X%X]\n", dsdt[i], dsdt[i+1]);

			uint32_t offset = i + 3 + (dsdt[i+2] >> 6);

			bool add_name = true;

			uint8_t j;

			for (j=0; j<4; j++)
			{
				char c = dsdt[offset+j];

				if (!aml_isvalidchar(c))
				{
					add_name = false;
					verbose("ACPIpatcher: invalid character found in ProcessorOP '0x%X'!\n", c);
					break;
				}
			}

			if (add_name)
			{
				acpi_cpu_name[acpi_cpu_count] = malloc(4);
				memcpy(acpi_cpu_name[acpi_cpu_count], dsdt+offset, 4);
				i = offset + 5;

				if (acpi_cpu_count == 0)
					acpi_cpu_p_blk = dsdt[i] | (dsdt[i+1] << 8);

				verbose("ACPIpatcher: found ACPI CPU [%c%c%c%c]\n", acpi_cpu_name[acpi_cpu_count][0], acpi_cpu_name[acpi_cpu_count][1], acpi_cpu_name[acpi_cpu_count][2], acpi_cpu_name[acpi_cpu_count][3]);

				if (++acpi_cpu_count == 32) {
					return;
				}
			}
		}
	}

	verbose("ACPIpatcher: finished finding cpu names. Found: %d.\n", acpi_cpu_count);
}

struct acpi_2_ssdt *generate_cst_ssdt(struct acpi_2_fadt* fadt)
{
	char ssdt_header[] =
	{
		0x53, 0x53, 0x44, 0x54, 0xE7, 0x00, 0x00, 0x00, // SSDT....
		0x01, 0x17, 0x50, 0x6D, 0x52, 0x65, 0x66, 0x41, // ..PmRefA
		0x43, 0x70, 0x75, 0x43, 0x73, 0x74, 0x00, 0x00, // CpuCst..
		0x00, 0x10, 0x00, 0x00, 0x49, 0x4E, 0x54, 0x4C, // ....INTL
		0x31, 0x03, 0x10, 0x20							// 1.._
	};
	
	char resource_template_register_fixedhw[] =
	{
		0x11, 0x14, 0x0A, 0x11, 0x82, 0x0C, 0x00, 0x7F,
		0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x01, 0x79, 0x00
	};

	char resource_template_register_systemio[] =
	{
		0x11, 0x14, 0x0A, 0x11, 0x82, 0x0C, 0x00, 0x01,
		0x08, 0x00, 0x00, 0x15, 0x04, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x79, 0x00,
	};

	if (Platform.CPU.Vendor != CPUID_VENDOR_INTEL) { // Intel = 0x756E6547
		verbose("ACPIpatcher: not an Intel platform, C-States will not be generated!\n");
		return NULL;
	}

	if (fadt == NULL) {
		verbose("ACPIpatcher: FACP not exists, C-States will not be generated!\n");
		return NULL;
	}

	struct acpi_2_dsdt *dsdt = (struct acpi_2_dsdt *)fadt->DSDT;

	if (dsdt == NULL) {
		verbose("ACPIpatcher: DSDT not found, C-States will not be generated!\n");
		return NULL;
	}

	if (acpi_cpu_count == 0)
		get_acpi_cpu_names((void*)dsdt, dsdt->Length);

	if (acpi_cpu_count > 0)
	{
		bool c2_enabled = false;
		bool c3_enabled = false;
		bool c4_enabled = false;
		bool cst_using_systemio = false;

		getBoolForKey(kEnableC2State, &c2_enabled, &bootInfo->chameleonConfig);
		getBoolForKey(kEnableC3State, &c3_enabled, &bootInfo->chameleonConfig);
		getBoolForKey(kEnableC4State, &c4_enabled, &bootInfo->chameleonConfig);
		getBoolForKey(kCSTUsingSystemIO, &cst_using_systemio, &bootInfo->chameleonConfig);

		c2_enabled = c2_enabled | (fadt->C2_Latency < 100);
		c3_enabled = c3_enabled | (fadt->C3_Latency < 1000);

		unsigned char cstates_count = 1 + (c2_enabled ? 1 : 0) + (c3_enabled ? 1 : 0);

		AML_CHUNK* root = aml_create_node(NULL);
		aml_add_buffer(root, ssdt_header, sizeof(ssdt_header)); // SSDT header
		AML_CHUNK* scop = aml_add_scope(root, "\\_PR_");
		AML_CHUNK* name = aml_add_name(scop, "CST_");
		AML_CHUNK* pack = aml_add_package(name);
		aml_add_byte(pack, cstates_count);

		AML_CHUNK* tmpl = aml_add_package(pack);
		if (cst_using_systemio) {
			// C1
			resource_template_register_fixedhw[8] = 0x00;
			resource_template_register_fixedhw[9] = 0x00;
			resource_template_register_fixedhw[18] = 0x00;
			aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
			aml_add_byte(tmpl, 0x01);		// C1
			aml_add_word(tmpl, 0x0001);		// Latency
			aml_add_dword(tmpl, 0x000003e8);	// Power

			uint8_t p_blk_lo, p_blk_hi;

			if (c2_enabled) // C2
			{
				p_blk_lo = acpi_cpu_p_blk + 4;
				p_blk_hi = (acpi_cpu_p_blk + 4) >> 8;

				tmpl = aml_add_package(pack);
				resource_template_register_systemio[11] = p_blk_lo; // C2
				resource_template_register_systemio[12] = p_blk_hi; // C2
				aml_add_buffer(tmpl, resource_template_register_systemio, sizeof(resource_template_register_systemio));
				aml_add_byte(tmpl, 0x02);		// C2
				aml_add_word(tmpl, 0x0040);		// Latency
				aml_add_dword(tmpl, 0x000001f4);	// Power
			}

			if (c4_enabled) // C4
			{
				p_blk_lo = acpi_cpu_p_blk + 5;
				p_blk_hi = (acpi_cpu_p_blk + 5) >> 8;

				tmpl = aml_add_package(pack);
				resource_template_register_systemio[11] = p_blk_lo; // C4
				resource_template_register_systemio[12] = p_blk_hi; // C4
				aml_add_buffer(tmpl, resource_template_register_systemio, sizeof(resource_template_register_systemio));
				aml_add_byte(tmpl, 0x04);		// C4
				aml_add_word(tmpl, 0x0080);		// Latency
				aml_add_dword(tmpl, 0x000000C8);	// Power
			}
			else if (c3_enabled) // C3
			{
				p_blk_lo = acpi_cpu_p_blk + 5;
				p_blk_hi = (acpi_cpu_p_blk + 5) >> 8;

				tmpl = aml_add_package(pack);
				resource_template_register_systemio[11] = p_blk_lo; // C3
				resource_template_register_systemio[12] = p_blk_hi; // C3
				aml_add_buffer(tmpl, resource_template_register_systemio, sizeof(resource_template_register_systemio));
				aml_add_byte(tmpl, 0x03);		// C3
				aml_add_word(tmpl, 0x0060);		// Latency
				aml_add_dword(tmpl, 0x0000015e);	// Power
			}
		}
		else
		{
			// C1
			resource_template_register_fixedhw[11] = 0x00; // C1
			aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
			aml_add_byte(tmpl, 0x01);		// C1
			aml_add_word(tmpl, 0x0001);		// Latency
			aml_add_dword(tmpl, 0x000003e8);	// Power

			resource_template_register_fixedhw[18] = 0x03;

			if (c2_enabled) // C2
			{
				tmpl = aml_add_package(pack);
				resource_template_register_fixedhw[11] = 0x10; // C2
				aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
				aml_add_byte(tmpl, 0x02);		// C2
				aml_add_word(tmpl, 0x0040);		// Latency
				aml_add_dword(tmpl, 0x000001f4);	// Power
			}

			if (c4_enabled) // C4
			{
				tmpl = aml_add_package(pack);
				resource_template_register_fixedhw[11] = 0x30; // C4
				aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
				aml_add_byte(tmpl, 0x04);		// C4
				aml_add_word(tmpl, 0x0080);		// Latency
				aml_add_dword(tmpl, 0x000000C8);	// Power
			}
			else if (c3_enabled)
			{
				tmpl = aml_add_package(pack);
				resource_template_register_fixedhw[11] = 0x20; // C3
				aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
				aml_add_byte(tmpl, 0x03);		// C3
				aml_add_word(tmpl, 0x0060);		// Latency
				aml_add_dword(tmpl, 0x0000015e);	// Power
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

		// dumpPhysAddr("C-States SSDT content: ", ssdt, ssdt->Length);
		
		verbose("ACPIpatcher: SSDT with CPU C-States generated successfully.\n");

		return ssdt;
	} else {
		verbose("ACPIpatcher: ACPI CPUs not found: C-States not generated!\n");
	}

	return NULL;
}

struct acpi_2_ssdt *generate_pss_ssdt(struct acpi_2_dsdt* dsdt)
{
	char ssdt_header[] =
	{
		0x53, 0x53, 0x44, 0x54, 0x7E, 0x00, 0x00, 0x00, // SSDT....
		0x01, 0x6A, 0x50, 0x6D, 0x52, 0x65, 0x66, 0x00, // ..PmRef.
		0x43, 0x70, 0x75, 0x50, 0x6D, 0x00, 0x00, 0x00, // CpuPm...
		0x00, 0x30, 0x00, 0x00, 0x49, 0x4E, 0x54, 0x4C, // .0..INTL
		0x31, 0x03, 0x10, 0x20,							// 1.._
	};

	if (Platform.CPU.Vendor != CPUID_VENDOR_INTEL) { // Intel = 0x756E6547
		verbose("ACPIpatcher: not an Intel platform: P-States will not be generated!\n");
		return NULL;
	}

	if (!(Platform.CPU.Features & CPU_FEATURE_MSR)) {
		verbose("ACPIpatcher: unsupported CPU: P-States will not be generated! No MSR support.\n");
		return NULL;
	}

	if (acpi_cpu_count == 0)
		get_acpi_cpu_names((void*)dsdt, dsdt->Length);

	if (acpi_cpu_count > 0)
	{
		struct p_state initial, maximum, minimum, p_states[32];
		uint8_t p_states_count = 0;

		// Retrieving P-States, ported from code by superhai (c)
		switch (Platform.CPU.Family) {
			case 0x06:
			{
				switch (Platform.CPU.Model) 
				{
					case CPUID_MODEL_DOTHAN:	// Intel Pentium M
					case CPUID_MODEL_YONAH:	// Intel Mobile Core Solo, Duo
					case CPUID_MODEL_MEROM:	// Intel Mobile Core 2 Solo, Duo, Xeon 30xx, Xeon 51xx, Xeon X53xx, Xeon E53xx, Xeon X32xx
					case CPUID_MODEL_PENRYN:	// Intel Core 2 Solo, Duo, Quad, Extreme, Xeon X54xx, Xeon X33xx
					case CPUID_MODEL_ATOM:	// Intel Atom (45nm)
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
						if (maximum.CID < minimum.CID) {
							verbose("P-States: Insane FID values!");
							p_states_count = 0;
						} else {
							uint8_t vidstep;
							uint8_t i = 0, u, invalid = 0;
							// Finalize P-States
							// Find how many P-States machine supports
							p_states_count = (uint8_t)(maximum.CID - minimum.CID + 1);

							if (p_states_count > 32) {
								p_states_count = 32;
							}

							vidstep = ((maximum.VID << 2) - (minimum.VID << 2)) / (p_states_count - 1);

							for (u = 0; u < p_states_count; u++) {
								i = u - invalid;

								p_states[i].CID = maximum.CID - u;
								p_states[i].FID = (uint8_t)(p_states[i].CID >> 1);

								if (p_states[i].FID < 0x6) {
									if (cpu_dynamic_fsb) {
										p_states[i].FID = (p_states[i].FID << 1) | 0x80;
									}
								} else if (cpu_noninteger_bus_ratio) {
									p_states[i].FID = p_states[i].FID | (0x40 * (p_states[i].CID & 0x1));
								}

								if (i && p_states[i].FID == p_states[i-1].FID) {
									invalid++;
								}
								p_states[i].VID = ((maximum.VID << 2) - (vidstep * u)) >> 2;
								uint32_t multiplier = p_states[i].FID & 0x1f;		// = 0x08
								bool half = p_states[i].FID & 0x40;					// = 0x01
								bool dfsb = p_states[i].FID & 0x80;					// = 0x00
								uint32_t fsb = (uint32_t)(Platform.CPU.FSBFrequency / 1000000); // = 400
								uint32_t halffsb = (fsb + 1) >> 1;					// = 200
								uint32_t frequency = (multiplier * fsb);			// = 3200

								p_states[i].Frequency = (uint32_t)(frequency + (half * halffsb)) >> dfsb;	// = 3200 + 200 = 3400
							}

							p_states_count -= invalid;
						}

						break;
					}
					case CPUID_MODEL_FIELDS:		// Intel Core i5, i7, Xeon X34xx LGA1156 (45nm)
					case CPUID_MODEL_DALES:		
					case CPUID_MODEL_DALES_32NM:	// Intel Core i3, i5 LGA1156 (32nm)
					case CPUID_MODEL_NEHALEM:		// Intel Core i7, Xeon W35xx, Xeon X55xx, Xeon E55xx LGA1366 (45nm)
					case CPUID_MODEL_NEHALEM_EX:	// Intel Xeon X75xx, Xeon X65xx, Xeon E75xx, Xeon E65xx
					case CPUID_MODEL_WESTMERE:	// Intel Core i7, Xeon X56xx, Xeon E56xx, Xeon W36xx LGA1366 (32nm) 6 Core
					case CPUID_MODEL_WESTMERE_EX:	// Intel Xeon E7
					case CPUID_MODEL_SANDYBRIDGE:	// Intel Core i3, i5, i7 LGA1155 (32nm)
					case CPUID_MODEL_JAKETOWN:// Intel Core i7, Xeon E5 LGA2011 (32nm)
					case CPUID_MODEL_IVYBRIDGE:	// Intel Core i3, i5, i7 LGA1155 (22nm)
					case CPUID_MODEL_HASWELL:	//
					case CPUID_MODEL_IVYBRIDGE_XEON:  //
					//case CPUID_MODEL_HASWELL_H:	//
					case CPUID_MODEL_HASWELL_SVR:	//
					case CPUID_MODEL_HASWELL_ULT:	//
					case CPUID_MODEL_CRYSTALWELL:	//

					{
					if ((Platform.CPU.Model == CPUID_MODEL_SANDYBRIDGE) || (Platform.CPU.Model == CPUID_MODEL_JAKETOWN) ||
						(Platform.CPU.Model == CPUID_MODEL_IVYBRIDGE) || (Platform.CPU.Model == CPUID_MODEL_HASWELL) ||
						(Platform.CPU.Model == CPUID_MODEL_IVYBRIDGE_XEON) || (Platform.CPU.Model == CPUID_MODEL_HASWELL_SVR) ||
						(Platform.CPU.Model == CPUID_MODEL_HASWELL_ULT) || (Platform.CPU.Model == CPUID_MODEL_CRYSTALWELL))
					{
						maximum.Control = (rdmsr64(MSR_IA32_PERF_STATUS) >> 8) & 0xff;
					} else {
						maximum.Control = rdmsr64(MSR_IA32_PERF_STATUS) & 0xff;
					}

					minimum.Control = (rdmsr64(MSR_PLATFORM_INFO) >> 40) & 0xff;

						verbose("ACPIpatcher: P-States: min=0x%X, max=0x%X.", minimum.Control, maximum.Control);

						// Sanity check
						if (maximum.Control < minimum.Control) {
							verbose(" Insane control values!\n");
							p_states_count = 0;
						} else {
							uint8_t i;
							p_states_count = 0;

							for (i = maximum.Control; i >= minimum.Control; i--) {
								p_states[p_states_count].Control = i;
								p_states[p_states_count].CID = p_states[p_states_count].Control << 1;
								p_states[p_states_count].Frequency = (Platform.CPU.FSBFrequency / 1000000) * i;
								p_states_count++;
							}
                            verbose("\n");
						}

						break;
					}
					default:
						verbose("ACPIpatcher: unsupported CPU (0x%X): P-States not generated !!!\n", Platform.CPU.Family);
						break;
				}
			}
		}

		// Generating SSDT
		if (p_states_count > 0) {
			int i;

			AML_CHUNK* root = aml_create_node(NULL);
				aml_add_buffer(root, ssdt_header, sizeof(ssdt_header)); // SSDT header
					AML_CHUNK* scop = aml_add_scope(root, "\\_PR_");
						AML_CHUNK* name = aml_add_name(scop, "PSS_");
							AML_CHUNK* pack = aml_add_package(name);

								for (i = 0; i < p_states_count; i++) {
									AML_CHUNK* pstt = aml_add_package(pack);

									aml_add_dword(pstt, p_states[i].Frequency);
									aml_add_dword(pstt, 0x00000000); // Power
									aml_add_dword(pstt, 0x0000000A); // Latency
									aml_add_dword(pstt, 0x0000000A); // Latency
									aml_add_dword(pstt, p_states[i].Control);
									aml_add_dword(pstt, i+1); // Status
								}

			// Add aliaces
			for (i = 0; i < acpi_cpu_count; i++) {
				char name[9];
				sprintf(name, "_PR_%c%c%c%c", acpi_cpu_name[i][0], acpi_cpu_name[i][1], acpi_cpu_name[i][2], acpi_cpu_name[i][3]);

				scop = aml_add_scope(root, name);
				aml_add_alias(scop, "PSS_", "_PSS");
			}

			aml_calculate_size(root);

			struct acpi_2_ssdt *ssdt = (struct acpi_2_ssdt *)AllocateKernelMemory(root->Size);

			aml_write_node(root, (void*)ssdt, 0);

			ssdt->Length = root->Size;
			ssdt->Checksum = 0;
			ssdt->Checksum = 256 - (uint8_t)(checksum8(ssdt, ssdt->Length));

			aml_destroy_node(root);

			//dumpPhysAddr("P-States SSDT content: ", ssdt, ssdt->Length);

			verbose("ACPIpatcher: SSDT with CPU P-States generated successfully\n");

			return ssdt;
		}
	} else {
		verbose("ACPIpatcher: ACPI CPUs not found: P-States not generated!\n");
	}

	return NULL;
}
***/
struct acpi_2_fadt *patch_fadt(struct acpi_2_fadt *fadt, struct acpi_2_dsdt *new_dsdt)
{
	// extern void setupSystemType();

	struct acpi_2_fadt *fadt_mod = NULL;
	bool fadt_rev2_needed = false;
	bool fix_restart = false;
	bool fix_restart_ps2 = false;
	int value = 1;
    static bool ver_20 = false;

	// Restart Fix
	if (Platform.CPU.Vendor == CPUID_VENDOR_INTEL) { // Intel=0x756E6547
		fix_restart = true;
		fix_restart_ps2 = false;
		if ( getBoolForKey(kPS2RestartFix, &fix_restart_ps2, &bootInfo->chameleonConfig) && fix_restart_ps2) {
			fix_restart = true;
		} else {
			getBoolForKey(kRestartFix, &fix_restart, &bootInfo->chameleonConfig);
		}
	} else {
		verbose("\tNot an Intel platform, FACP Restart Fix will not be applied!\n");
		fix_restart = false;
	}

	if (fix_restart) {
		fadt_rev2_needed = true;
	}

	// Allocate new fadt table
	if ((fadt->Length < 0x84) && fadt_rev2_needed)
	{
		fadt_mod = (struct acpi_2_fadt *)AllocateKernelMemory(0x84);
		memcpy(fadt_mod, fadt, fadt->Length);
		fadt_mod->Length   = 0x84;
		fadt_mod->Revision = 0x02; // FACP rev 2 (ACPI 1.0B MS extensions)
	} else {
		fadt_mod = (struct acpi_2_fadt *)AllocateKernelMemory(fadt->Length);
		memcpy(fadt_mod, fadt, fadt->Length);
	}
    verbose("\tNew FACP loaded @%08X, length=%d\n", (uint32_t)fadt_mod, fadt_mod->Length);

	// Bungo: Determine PM Profile
	verbose("\tPM Profile=0x%02X", fadt_mod->PM_Profile);
	if (getIntForKey(kSystemType, &value, &bootInfo->chameleonConfig)) {
		verbose(", overriding with: 0x%02X.\n", (uint8_t)value);
		fadt_mod->PM_Profile = (uint8_t)value; // user has overriden the PM Profile so take care of it in FACP
	} else {
        switch (fadt_mod->PM_Profile) { // check if PM Profile is correct (1..3)
            case 1:
            case 2:
            case 3:
                verbose(": using.\n");
                break;
            default:
                // use SMBIOS chassisType to determine PM Profile (saved previously for us)
                verbose(", expected value: 1, 2 or 3, setting to 0x%02X.\n", Platform.Type);
                fadt_mod->PM_Profile = Platform.Type; // take care of modified FACP's PM Profile entry
                break;
        }
    }
	Platform.Type = fadt_mod->PM_Profile; // Save fixed PM Profile (system-type)
    
/*  Bungo: Moved into fake_efi.c
    // Setup system-type: We now have to write the systemm-type in ioregs: we cannot do it before in setupDeviceTree()
	// because we need to take care of FACP original content, if it is correct.
	setupSystemType();
*/
	// Patch FACP to fix restart
	if (fix_restart) {
		if (fix_restart_ps2) {
			fadt_mod->Flags|= 0x400;              // Reset register supported
			fadt_mod->Reset_SpaceID		= 0x01;   // System I/O
			fadt_mod->Reset_BitWidth	= 0x08;   // 1 byte
			fadt_mod->Reset_BitOffset	= 0x00;   // Offset 0
			fadt_mod->Reset_AccessWidth	= 0x01;   // Byte access
			fadt_mod->Reset_Address		= 0x64;   // Address of the register
			fadt_mod->Reset_Value		= 0xfe;   // Value to write to reset the system
			verbose("\tFACP PS2 Restart Fix applied!\n");
		} else {
			fadt_mod->Flags|= 0x400;              // Reset register supported
			fadt_mod->Reset_SpaceID		= 0x01;   // System I/O
			fadt_mod->Reset_BitWidth	= 0x08;   // 1 byte
			fadt_mod->Reset_BitOffset	= 0x00;   // Offset 0
			fadt_mod->Reset_AccessWidth	= 0x01;   // Byte access
			fadt_mod->Reset_Address		= 0x0cf9; // Address of the register
			fadt_mod->Reset_Value		= 0x06;   // Value to write to reset the system
			verbose("\tFACP Restart Fix applied!\n");
		}
	} else {
        //verbose("\tRestart Fix: No.\n");
    }
    
    // Bungo: FACS table fix and load
    verbose("\tOEM table FACS@%08X, length=%d: ", fadt_mod->FACS, ((struct acpi_2_facs *)fadt_mod->FACS)->Length);
    if ((fadt_mod->FACS > 0) && (fadt_mod->FACS < 0xFFFFFFFF) && (((struct acpi_2_facs *)fadt_mod->FACS)->Length >= 64)) {
        verbose("using.\n");
    } else {
        verbose(" incorrect!\n");
    }
    if (ver_20 && (((uint32_t)(&(fadt_mod->X_FACS)) - (uint32_t)fadt_mod + 8) <= fadt_mod->Length)) {
        verbose("\tOEM table X_FACS@%08X%08X, length=%d: ", (uint32_t)(fadt_mod->X_FACS >> 32), (uint32_t)(fadt_mod->X_FACS & 0xFFFFFFFF), ((struct acpi_2_facs *)fadt_mod->X_FACS)->Length);
        if (fadt_mod->FACS != fadt_mod->X_FACS) {
            verbose("differes from FACS - fixing");
            if ((fadt_mod->X_FACS > 0) && (fadt_mod->X_FACS < 0xFFFFFFFF) && (((struct acpi_2_facs *)(uint32_t)fadt_mod->X_FACS)->Length >= 64)) {
                // Bungo: in my case only from X_FACS loading correct table (64 bytes) into IOReg
                fadt_mod->FACS = (uint32_t)fadt_mod->X_FACS;
            } else {
                fadt_mod->X_FACS = (uint64_t)fadt_mod->FACS;
            }
            verbose(" \tUsing FACS@%08X = X_FACS@%08X\n", fadt_mod->FACS, (uint32_t)fadt_mod->X_FACS);
        } else {
            verbose("using.\n");
        }
    }
    
    // Bungo: Save Hardware Signature (machine-signature)
    if ((fadt_mod->FACS > 0) && (fadt_mod->FACS < 0xFFFFFFFF) && (((struct acpi_2_facs *)fadt_mod->FACS)->Length >= 64)) {
        Platform.HWSignature = ((struct acpi_2_facs *)fadt_mod->FACS)->HWSignature;
        verbose("\tHardware Signature=0x%08X: using.\n", Platform.HWSignature);
    } else {
        Platform.HWSignature = 0;
        verbose("\tFixing Hardware Signature=0x%08X.\n", Platform.HWSignature);
    }
	
    verbose("\tOEM table DSDT@%08X, length=%d: %susing.\n", fadt_mod->DSDT, ((struct acpi_2_dsdt *)fadt_mod->DSDT)->Length, new_dsdt ? "not " : "");
    if (ver_20 && (((uint32_t)(&(fadt_mod->X_DSDT)) - (uint32_t)fadt_mod + 8) <= fadt_mod->Length)) {
        verbose("\tOEM table X_DSDT@%08X%08X, length=%d: %susing.\n", (uint32_t)(fadt_mod->X_DSDT >> 32), (uint32_t)(fadt_mod->X_DSDT & 0xFFFFFFFF), ((struct acpi_2_dsdt *)fadt_mod->X_DSDT)->Length, new_dsdt ? "not " : "");
    }
	// Patch DSDT address if we have loaded DSDT.aml
	if (new_dsdt) {
        fadt_mod->DSDT = (uint32_t)new_dsdt;
        verbose("\tFACP uses custom DSDT@%08X", fadt_mod->DSDT);
		if (ver_20 && (((uint32_t)(&(fadt_mod->X_DSDT)) - (uint32_t)fadt_mod + 8) <= fadt_mod->Length)) {
			fadt_mod->X_DSDT = (uint64_t)new_dsdt;
            verbose(" / X_DSDT@%08X%08X", (uint32_t)(fadt_mod->X_DSDT >> 32), (uint32_t)(fadt_mod->X_DSDT & 0xFFFFFFFF));
		}
        verbose(", length=%d\n", ((struct acpi_2_dsdt *)fadt_mod->DSDT)->Length);
	}
    
	// Correct the checksum
	fadt_mod->Checksum=0;
	fadt_mod->Checksum=256-checksum8(fadt_mod, fadt_mod->Length);

    ver_20 = TRUE;
	return fadt_mod;
}
// Bung: Unused
/* Setup ACPI without replacing DSDT.
int setupAcpiNoMod()
{
//	addConfigurationTable(&gEfiAcpiTableGuid, getAddressOfAcpiTable(), "ACPI");
//	addConfigurationTable(&gEfiAcpi20TableGuid, getAddressOfAcpi20Table(), "ACPI_20");
	// XXX aserebln why uint32 cast if pointer is uint64 ?
	acpi10_p = (uint64_t)(uint32_t)getAddressOfAcpiTable();
	acpi20_p = (uint64_t)(uint32_t)getAddressOfAcpi20Table();
	// addConfigurationTable(&gEfiAcpiTableGuid, &acpi10_p, "ACPI");
	if(acpi20_p) {
		// addConfigurationTable(&gEfiAcpi20TableGuid, &acpi20_p, "ACPI_20");
	} else {
		DBG("ACPIpatcher: version 2.0 not found.\n");
	}
	return 1;
}
*/
/* Setup ACPI. Replace DSDT if DSDT.aml is found */
int setupAcpi(void)
{
	int version;
	const char *filename;
	char dirSpec[512];
	int len = 0;
    
	// always reset cpu count to 0 when injecting new acpi
	acpi_cpu_count = 0;

    verbose("\nACPIpatcher: Start patching...\n");
    
    if (new_dsdt != NULL) {
        verbose("ACPIpatcher: custom table DSDT already loaded @%08X, length=%d: using.\n", new_dsdt, ((struct acpi_2_header *)new_dsdt)->Length);
    } else {
        // Try using the file specified with the DSDT option
        if (getValueForKey(kDSDT, &filename, &len, &bootInfo->chameleonConfig) && len) {
            snprintf(dirSpec, sizeof(dirSpec), filename);
        } else {
            sprintf(dirSpec, "DSDT.aml");
        }
        
        verbose("ACPIpatcher: attempting to load custom table DSDT...\n", dirSpec);
        if ((new_dsdt = loadACPITable(dirSpec))) {
            verbose("ACPIpatcher: custom table DSDT loaded @%08X, length=%d.\n", new_dsdt, ((struct acpi_2_header *)new_dsdt)->Length);
        } else {
            //verbose("ACPIpatcher: custom DSDT not found!.\n");
        }
    }

	/* Try using the file specified with the DSDT option
	if (getValueForKey(kDSDT, &filename, &len, &bootInfo->chameleonConfig)) {
		snprintf(dirSpec, sizeof(dirSpec), filename);
	} else {
		sprintf(dirSpec, "DSDT.aml");
		//DBG("dirSpec, DSDT.aml");
	}
	// Load replacement DSDT
	new_dsdt = loadACPITable(dirSpec);
*/
    
    // Load ECDT table
    if (new_ecdt != NULL) {
        verbose("ACPIpatcher: custom table ECDT already loaded @%08X, length=%d: using.\n", new_ecdt, ((struct acpi_2_header *)new_ecdt)->Length);
    } else {
        sprintf(dirSpec, "ECDT.aml");
        filename = "ECDT.aml";
        verbose("ACPIpatcher: attempting to load custom table ECDT...\n");
        if ((new_ecdt = loadACPITable(filename))) {
            verbose("ACPIpatcher: custom table ECDT loaded @%08X, length=%d.\n", new_ecdt, ((struct acpi_2_header *)new_ecdt)->Length);
        } else {
            //verbose("ACPIpatcher: custom ECDT not found!.\n");
        }
    }
    
    // Mozodojo: Load additional SSDTs
	struct acpi_2_ssdt *new_ssdt[32]; // 26 custom + 4 subssdt + 2 for pss & cst
	int  ssdtotal_number = 0;
	{
		int i;
		for (i = 0; i < 26; i++) {
			//char filename[512];
            
			sprintf(dirSpec, (i > 0)? "SSDT-%d.aml":"SSDT.aml", i);
            
			if ((new_ssdt[ssdtotal_number] = loadACPITable(dirSpec))) {
                verbose("ACPIpatcher: custom table %s loaded @%08X, length=%d\n", dirSpec, new_ssdt[ssdtotal_number], new_ssdt[ssdtotal_number]->Length);
                ssdtotal_number++;
			} else {
                //verbose("ACPIpatcher: custom table '%s' not found.\n", filename);
                // Bungo: load up to 26 custom tables enabled now
				//break;
			}
		}
	}
    
	// Mozodojo: going to patch FACP and load SSDTs even if DSDT.aml is not present
	/*if (!new_dsdt)
	 {
	 return setupAcpiNoMod();
	 }*/
    
	// SSDT options
	bool drop_ssdt = false, generate_pstates = false, generate_cstates = false;

	getBoolForKey(kDropSSDT, &drop_ssdt, &bootInfo->chameleonConfig);
	getBoolForKey(kGeneratePStates, &generate_pstates, &bootInfo->chameleonConfig);
	getBoolForKey(kGenerateCStates, &generate_cstates, &bootInfo->chameleonConfig);
    verbose("ACPIpatcher: drop SSDT tables: %s.\n", drop_ssdt ? "Yes" : "No");
	verbose("ACPIpatcher: generate P-States: %s.\n", generate_pstates ? "Yes" : "No");
	verbose("ACPIpatcher: generate C-States: %s.\n", generate_cstates ? "Yes" : "No");
    
    bool getSubSSDT = !generate_pstates && !generate_cstates;

	// Do the same procedure for both versions of ACPI
	for (version = 0; version < 2; version++) {
		struct acpi_2_rsdp *rsdp, *rsdp_mod;
		struct acpi_2_rsdt *rsdt, *rsdt_mod;
        struct acpi_2_xsdt *xsdt, *xsdt_mod;
        struct acpi_2_fadt *fadt_mod;
        uint32_t *rsdt_entries;
        uint64_t *xsdt_entries;

		// Find original rsdp
		rsdp = (struct acpi_2_rsdp *)(version ? getAddressOfAcpi20Table() : getAddressOfAcpiTable());
		if (!rsdp) {
            /*
			if (version) {
				addConfigurationTable(&gEfiAcpi20TableGuid, NULL, "ACPI_20");
			} else {
				addConfigurationTable(&gEfiAcpiTableGuid, NULL, "ACPI");
			}
            */
            verbose("ACPIpatcher: ACPI version %d.0 not found. Not patching.\n", version + 1);
			continue;
		}

        int rsdplength = version ? rsdp->Length : 20;
		int l = version ? 20 : 0;
		verbose("ACPIpatcher: OEM table RSDP@%08X, length=%d. ACPI version %d.0: patching.\n", rsdp, rsdplength, version + 1);

		/* FIXME: no check that memory allocation succeeded 
		 * Copy and patch RSDP, RSDT, XSDT and FADT
		 * For more info see ACPI Specification pages 110 and following
		 */

		rsdp_mod = (struct acpi_2_rsdp *)(l + AllocateKernelMemory(l + rsdplength));
		memcpy(rsdp_mod, rsdp, rsdplength);

		rsdt = (struct acpi_2_rsdt *)rsdp->RsdtAddress;
		verbose("ACPIpatcher: OEM table RSDT@%08X, length=%d: ", rsdp->RsdtAddress, rsdt->Length);
		
		if ((rsdp->RsdtAddress > 0) && (rsdp->RsdtAddress < 0xFFFFFFFF) && (rsdt->Length < 0x10000)) {
            verbose("using.\n");
			int rsdt_entries_num;
			int dropoffset = 0, i, j;

			// mozo: using malloc cos I didn't found how to free already allocated kernel memory
			rsdt_mod = (struct acpi_2_rsdt *)malloc(rsdt->Length);
			memcpy(rsdt_mod, rsdt, rsdt->Length);
			rsdp_mod->RsdtAddress = (uint32_t)rsdt_mod;
			rsdt_entries_num = (rsdt_mod->Length - sizeof(struct acpi_2_rsdt)) / 4;
			rsdt_entries = (uint32_t *)(rsdt_mod + 1);
            
			for (i = 0; i < rsdt_entries_num; i++) {
                struct acpi_2_header *oemTable = (struct acpi_2_header *)rsdt_entries[i];
                verbose("ACPIpatcher: OEM table %c%c%c%c@%08X, length=%d: ", oemTable->Signature[0], oemTable->Signature[1], oemTable->Signature[2], oemTable->Signature[3], oemTable, oemTable->Length);
                
				if (!(rsdt_entries[i] > 0) || !(rsdt_entries[i] < 0xFFFFFFFF)) {
                    verbose("incorrect! Dropping.\n");
                    dropoffset++;
                    continue;
				}
                
				if (tableSign(oemTable, "SSDT")) {
                    if (drop_ssdt) {
                        verbose("dropping.\n");
                        dropoffset++;
                    } else {
                        verbose("using.\n");
                        rsdt_entries[i-dropoffset] = rsdt_entries[i];
                        // get rest of ssdt tables from inside ssdt_pmref
                        if (getSubSSDT) { // prevent from extracting originals if user choosed generatind PSS and/or CSS tables
                            struct ssdt_pmref *subSSDT = (struct ssdt_pmref *)(rsdt_entries[i] + sizeof(struct acpi_2_header) + 15);
                            uint8_t tabNum = *((uint8_t *)subSSDT - 2) / 3; // e.g Name (SSDT, Package (0x0C) -> 0x0C / 3 = number of sub SSDTs
                            for (j = 0; (j < tabNum) && (ssdtotal_number < 30); j++) {
                                verbose("ACPIpatcher: OEM table SSDT_%s@%08X, length=%d: ", ((struct acpi_2_ssdt *)subSSDT[j].addr)->OEMTableId, subSSDT[j].addr, ((struct acpi_2_ssdt *)subSSDT[j].addr)->Length);
                                if (!(subSSDT[j].addr > 0) || !(subSSDT[j].addr < 0xFFFFFFFF)) {
                                    verbose("incorrect! Dropping.\n");
                                    continue;
                                }
                                verbose("using.\n");
                                new_ssdt[ssdtotal_number] = (struct acpi_2_ssdt *)subSSDT[j].addr;
                                ssdtotal_number++;
                            }
                        }
                    }
                    continue;
				}

                /* Bungo: According to ACPI Spec. no DSDT in RSDT, so what this for?
                 
                if (tableSign(oemTable, "DSDT")) {
					if (new_dsdt) {
						rsdt_entries[i-dropoffset] = (uint32_t)new_dsdt;
						verbose("custom table added.\n");
					}
					continue;
				}
                
                So, suggest to drop, it should be in FACP */
                
                if (tableSign(oemTable, "DSDT")) {
                    verbose("dropping.\n");
                    dropoffset++;
                    continue;
				}
                
                if (tableSign(oemTable, "ECDT") && new_ecdt) {
                    verbose("dropping.\n");
                    dropoffset++;
                    continue;
				}

				if (tableSign(oemTable, "FACP")) {
					if (oemTable->Length > 0x10000) {
						verbose("incorrect. Not modifying.\n");
						continue;
					}
                    
					verbose("patching.\n");
					fadt_mod = patch_fadt((struct acpi_2_fadt *)oemTable, new_dsdt);
					rsdt_entries[i-dropoffset] = (uint32_t)fadt_mod;
					
					// Generate _CST SSDT
					if (generate_cstates && (new_ssdt[ssdtotal_number] = generate_cst_ssdt(fadt_mod))) {
						verbose("\tC-States generated.\n");
						generate_cstates = false; // Generate SSDT only once!
						ssdtotal_number++;
					}

					// Generating _PSS SSDT
					if (generate_pstates && (new_ssdt[ssdtotal_number] = generate_pss_ssdt((void*)fadt_mod->DSDT))) {
						verbose("\tP-States generated.\n");
						generate_pstates = false; // Generate SSDT only once!
						ssdtotal_number++;
					}
					continue;
				}
                
                verbose("using.\n");
                rsdt_entries[i-dropoffset] = rsdt_entries[i];
			}
            
            // For moded rsdt calculate new lenght
            if (new_ecdt)
                rsdt_mod->Length += 4*ssdtotal_number - 4*dropoffset + 4;  // custom - dropped + ECDT
			else
                rsdt_mod->Length += 4*ssdtotal_number - 4*dropoffset;
            // Allocate moded rsdt in Kernel memory area
			struct acpi_2_rsdt *rsdt_copy = (struct acpi_2_rsdt *)AllocateKernelMemory(rsdt_mod->Length);
            memcpy(rsdt_copy, rsdt_mod, rsdt_mod->Length); // should be rsdt_mod->Length - 4*ssdtotal_number - 4 but don't care
			free(rsdt_mod);
			rsdt_mod = rsdt_copy;
			rsdp_mod->RsdtAddress = (uint32_t)rsdt_mod;
			rsdt_entries_num = (rsdt_mod->Length-sizeof(struct acpi_2_rsdt)) / 4;
			rsdt_entries = (uint32_t *)(rsdt_mod + 1);

			// Mozodojo: Insert additional SSDTs into RSDT
            for (j = 0; j < ssdtotal_number; j++) {
                rsdt_entries[i-dropoffset+j] = (uint32_t)new_ssdt[j];
            }
            verbose("ACPIpatcher: added %d custom SSDT table%s into RSDT.\n", ssdtotal_number, (ssdtotal_number != 1) ? "s" : "");
			
			if (new_ecdt) {
				rsdt_entries[i - dropoffset + j] = (uint32_t)new_ecdt;
				verbose("ACPIpatcher: added custom table %s @%08X into RSDT.\n", "ECDT", new_ecdt);
			}

			// Correct the checksum of RSDT
			verbose("ACPIpatcher: modified RSDT@%08X, length=%d. Checksum: old=%d, ", rsdt_mod, rsdt_mod->Length, rsdt_mod->Checksum);
			rsdt_mod->Checksum=0;
			rsdt_mod->Checksum=256-checksum8(rsdt_mod,rsdt_mod->Length);
			verbose("new=%d.\n", rsdt_mod->Checksum);
		} else {
			rsdp_mod->RsdtAddress = 0;
			verbose("not found or incorrect!\n");
		}
		verbose("\n");
        
        if (gVerboseMode) pause("");

		if (version) {
			// FIXME: handle 64-bit address correctly
			xsdt = (struct acpi_2_xsdt *)(uint32_t)rsdp->XsdtAddress;
            verbose("ACPIpatcher: OEM table XSDT@%08X%08X, length=%d: ", (uint32_t)(rsdp->XsdtAddress >> 32), (uint32_t)(rsdp->XsdtAddress & 0xFFFFFFFF), xsdt->Length);

			if ((rsdp->XsdtAddress > 0) && (rsdp->XsdtAddress < 0xFFFFFFFF) && (xsdt->Length < 0x10000)) {
                verbose("using.\n");
				int xsdt_entries_num, i, j;
				int dropoffset = 0;

				// mozo: using malloc cos I didn't found how to free already allocated kernel memory
				xsdt_mod = (struct acpi_2_xsdt *)malloc(xsdt->Length);
				memcpy(xsdt_mod, xsdt, xsdt->Length);
                rsdp_mod->XsdtAddress = (uint64_t)xsdt_mod;
				xsdt_entries_num = (xsdt_mod->Length - sizeof(struct acpi_2_xsdt)) / 8;
				xsdt_entries = (uint64_t *)(xsdt_mod + 1);
                
				for (i = 0; i < xsdt_entries_num; i++) {
                    struct acpi_2_header *oemTable = (struct acpi_2_header *)(uint32_t)xsdt_entries[i];
                    verbose("ACPIpatcher: OEM table %c%c%c%c@%08X%08X, length=%d", oemTable->Signature[0], oemTable->Signature[1], oemTable->Signature[2], oemTable->Signature[3], (uint32_t)(xsdt_entries[i] >> 32), (uint32_t)(xsdt_entries[i] & 0xFFFFFFFF), oemTable->Length);
                    
					if (!(xsdt_entries[i] > 0) || !(xsdt_entries[i] < 0xFFFFFFFF)) {
                        verbose(": incorrect! Dropping.\n");
                        dropoffset++;
						continue;
					}
                    
                    bool inRSDT = (uint32_t)oemTable == ((uint32_t *)(rsdt + 1))[i]; // check if already in RSDT
                    if (inRSDT) {
                        verbose(" (already in RSDT)");
                    }

                    if (tableSign(oemTable, "SSDT") && !inRSDT) {
                        if (drop_ssdt) {
                            verbose(": dropping.\n");
                            dropoffset++;
                        } else {
                            verbose(": using.\n");
                            xsdt_entries[i - dropoffset] = xsdt_entries[i];
                            // Get rest of ssdts from ssdt_pmref
                            if (getSubSSDT) {
                                struct ssdt_pmref *subSSDT = (struct ssdt_pmref *)((uint32_t)xsdt_entries[i - dropoffset] + sizeof(struct acpi_2_header) + 15);
                                uint8_t tabNum = *((uint8_t *)subSSDT - 2) / 3; // e.g: Name (SSDT, Package (0x0C) -> 0x0C / 3 = 4 is number of sub SSDTs
                                for (j = 0; (j < tabNum) && (ssdtotal_number < 30); j++) {
                                    verbose("ACPIpatcher: OEM table SSDT_%s@%08X, length=%d", ((struct acpi_2_ssdt *)subSSDT[j].addr)->OEMTableId, subSSDT[j].addr, ((struct acpi_2_ssdt *)subSSDT[j].addr)->Length);
                                    if (!(subSSDT[j].addr > 0) || !(subSSDT[j].addr < 0xFFFFFFFF)) {
                                        verbose(": incorrect! Dropping.\n");
                                        continue;
                                    }
                                    new_ssdt[ssdtotal_number] = (struct acpi_2_ssdt *)subSSDT[j].addr;
                                    ssdtotal_number++;
                                    verbose(": using.\n");
                                }
                            }
                        }
                        continue;
                    }

                    // Bungo: According to ACPI Spec. no DSDT in RSDT, so what this for?
                    /*
                     if (tableSign(oemTable, "DSDT")) {
                     if (new_dsdt) {
                     xsdt_entries[i-dropoffset] = (uint64_t)new_dsdt;
                     verbose("custom table added.\n");
                     }
                     continue;
                     }
                     */
                    // Suggest to drop, it should be in FACP
                    if (tableSign(oemTable, "DSDT") && !inRSDT) {
                        verbose(": dropping.\n");
                        dropoffset++;
                        continue;
                    }
                    
					if (tableSign(oemTable, "FACP") && !inRSDT) {
						if (oemTable->Length > 0x10000) {
							goto drop_xsdt;
						}

                        verbose(": patching.\n");
						fadt_mod = patch_fadt((struct acpi_2_fadt *)oemTable, new_dsdt);
						xsdt_entries[i - dropoffset] = (uint64_t)fadt_mod;

						// Generate _CST SSDT
						if (generate_cstates && (new_ssdt[ssdtotal_number] = generate_cst_ssdt(fadt_mod))) {
							verbose("\tC-States generated\n");
							generate_cstates = false; // Generate SSDT only once!
							ssdtotal_number++;
						}

						// Generating _PSS SSDT
						if (generate_pstates && (new_ssdt[ssdtotal_number] = generate_pss_ssdt((void *)fadt_mod->DSDT))) {
							verbose("\tP-States generated\n");
							generate_pstates = false; // Generate SSDT only once!
							ssdtotal_number++;
						}
						continue;
					}
                    
                    verbose(": using.\n");
                    xsdt_entries[i - dropoffset] = xsdt_entries[i];
				}
                
                // For moded xsdt calculate new lenght
                if (new_ecdt)
                    xsdt_mod->Length += 8*ssdtotal_number - 8*dropoffset + 8;  // custom - dropped + ECDT
                else
                    xsdt_mod->Length += 8*ssdtotal_number - 8*dropoffset;
                // Allocate xsdt in Kernel memory area
				struct acpi_2_xsdt *xsdt_copy = (struct acpi_2_xsdt *)AllocateKernelMemory(xsdt_mod->Length);
				memcpy(xsdt_copy, xsdt_mod, xsdt_mod->Length); // should be: rsdt_mod->Length - 8*ssdtotal_number - 8 but don't care
				free(xsdt_mod);
                xsdt_mod = xsdt_copy;
				rsdp_mod->XsdtAddress = (uint64_t)xsdt_mod;
				xsdt_entries_num = (xsdt_mod->Length - sizeof(struct acpi_2_xsdt)) / 8;
				xsdt_entries = (uint64_t *)(xsdt_mod + 1);

				// Mozodojo: Insert additional SSDTs into XSDT
                for (j = 0; j < ssdtotal_number; j++) {
                    xsdt_entries[i-dropoffset+j] = (uint64_t)new_ssdt[j];
                }
                verbose("ACPIpatcher: added %d custom SSDT table%s into XSDT.\n", ssdtotal_number, (ssdtotal_number != 1) ? "s" : "");

				if (new_ecdt) {
					xsdt_entries[i - dropoffset + j] = (uint64_t)new_ecdt;
					verbose("ACPIpatcher: added custom table %s@%016X into XSDT.\n", "ECDT", new_ecdt);
				}
                
				// Correct the checksum of XSDT
				verbose("ACPIpatcher: modified XSDT@%016X, length=%d. Checksum: old=%d, ", xsdt_mod, xsdt_mod->Length, xsdt_mod->Checksum);
                xsdt_mod->Checksum=0;
                xsdt_mod->Checksum=256-checksum8(xsdt_mod, xsdt_mod->Length);
                verbose("new=%d.\n", xsdt_mod->Checksum);
			} else {
			drop_xsdt:
				/*FIXME: Now we just hope that if MacOS doesn't find XSDT it reverts to RSDT. 
				 * A Better strategy would be to generate
				 */
                verbose("not found or incorrect!\n");
				rsdp_mod->XsdtAddress=0xffffffffffffffffLL;
			}
            
            if (gVerboseMode) pause("");
		}

		// Correct the checksum of RSDP
		verbose("ACPIpatcher: modified RSDP@%08X, checksum: old=%d, ", rsdp_mod, rsdp_mod->Checksum);
		rsdp_mod->Checksum=0;
		rsdp_mod->Checksum=256-checksum8(rsdp_mod, 20);
		verbose("new=%d", rsdp_mod->Checksum);

		if (version) {
			verbose("; extended checksum: old=%d, ", rsdp_mod->ExtendedChecksum);
			rsdp_mod->ExtendedChecksum=0;
			rsdp_mod->ExtendedChecksum=256-checksum8(rsdp_mod,rsdp_mod->Length);
			verbose("new=%d.\n", rsdp_mod->ExtendedChecksum);
		} else {
            verbose(".\n");
        }

		if (version) {
			acpi20_p = (uint64_t)(uint32_t)rsdp_mod;    // efi configuration table pointer to ACPI_20 RSDP
            acpi10_p = acpi20_p - 20;                   // efi configuration table pointer to ACPI RSDP
            memcpy((struct acpi_2_rsdp *)(uint32_t)acpi10_p, (struct acpi_2_rsdp *)(uint32_t)acpi20_p, 20);
			//addConfigurationTable(&gEfiAcpi20TableGuid, &acpi20_p, "ACPI_20");
		} else {
			acpi10_p = (uint64_t)(uint32_t)rsdp_mod; // efi configuration table pointer to ACPI RSDP
			//addConfigurationTable(&gEfiAcpiTableGuid, &acpi10_p, "ACPI");
		}
        
        verbose("ACPIpatcher: acpi version %d.0 patching finished.\n", version + 1);
	}
    
#if DEBUG_ACPI
    pause("[DEBUG ACPI] ");
#else
    if (gVerboseMode) pause("");
#endif
    
	return 1;
}

struct acpi_2_rsdp *getRSDPaddress()
{
    bool found = false;
    /* Before searching the BIOS space we are supposed to search the first 1K of the EBDA */
	EBDA_RANGE_START = (uint32_t)swapUint16(*(uint16_t *)BDA_EBDA_START) << 4;
    verbose("getRSDPaddress: scanning EBDA [%08X:%08X] for RSDP... ", EBDA_RANGE_START, EBDA_RANGE_END);
    void *rsdp_addr = (void*)EBDA_RANGE_START;
	for (; rsdp_addr <= (void*)EBDA_RANGE_END; rsdp_addr += 1) {
		if (*(uint64_t *)rsdp_addr == ACPI_SIGNATURE_UINT64_LE) {
            found = true;
            break;
		}
	}
    
    if (!found) {
        verbose("Nothing found.\n");
        verbose("getRSDPaddress: scanning BIOS area [%08X:%08X] for RSDP... ", ACPI_RANGE_START, ACPI_RANGE_END);
        rsdp_addr = (void*)ACPI_RANGE_START;
        for (; rsdp_addr <= (void*)ACPI_RANGE_END; rsdp_addr += 16) {
            if (*(uint64_t *)rsdp_addr == ACPI_SIGNATURE_UINT64_LE) {
                found = true;
                break;
            }
        }
    }
    
    if (found) {
        verbose("Found @0%08X, Rev.: %d.0).\n", rsdp_addr, ((struct acpi_2_rsdp *)rsdp_addr)->Revision + 1);
        uint8_t csum = checksum8(rsdp_addr, 20);
        if (csum == 0) {
            if (((struct acpi_2_rsdp *)rsdp_addr)->Revision == 0) return rsdp_addr;
            csum = checksum8(rsdp_addr, sizeof(struct acpi_2_rsdp));
            if (csum == 0) return rsdp_addr;
            else verbose("getRSDPaddress: RSDP extended checksum incorrect: %d.\n", csum);
        } else verbose("getRSDPaddress: RSDP checksum incorrect: %d.\n", csum);
    } else verbose("Nothing found.\n");
    
	return NULL;
}
