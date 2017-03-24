/*
 * Copyright 2008 mackerintel
 *
 *  state_generator.h
 *  Chameleon
 *
 *  Created by Mozodojo on 20/07/10.
 *  Copyright 2010 mozodojo. All rights reserved.
 *
 */

#include "config.h"
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

#if DEBUG_STATE==2
#define DBG(x...)  {printf(x); sleep(1);}
#elif DEBUG_STATE==1
	#define DBG(x...)  printf(x)
#else
	#define DBG(x...)  msglog(x)
#endif

uint8_t acpi_cpu_count	= 0;
uint32_t acpi_cpu_p_blk	= 0;
char *acpi_cpu_name[32];

void get_acpi_cpu_names(unsigned char *dsdt, uint32_t length)
{
	uint32_t i;

	DBG("\tACPI patcher: start finding cpu names. Length %d\n", length);

	for (i=0; i<length-7; i++)
	{
		if (dsdt[i] == 0x5B && dsdt[i+1] == 0x83) // ProcessorOP
		{
			DBG("\tACPIpatcher: DSDT[%X%X]\n", dsdt[i], dsdt[i+1]);

			uint32_t offset = i + 3 + (dsdt[i+2] >> 6);

			bool add_name = true;

			uint8_t j;

			for (j=0; j<4; j++)
			{
				char c = dsdt[offset+j];

				if (!aml_isvalidchar(c))
				{
					add_name = false;
					DBG("\tACPI patcher: invalid character found in ProcessorOP '0x%X'!\n", c);
					break;
				}
			}

			if (add_name)
			{
				acpi_cpu_name[acpi_cpu_count] = malloc(4);
				memcpy(acpi_cpu_name[acpi_cpu_count], dsdt+offset, 4);
				i = offset + 5;

				if (acpi_cpu_count == 0)
				{
					acpi_cpu_p_blk = dsdt[i] | (dsdt[i+1] << 8);
				}

				DBG("\tACPI patcher: found ACPI CPU [%c%c%c%c]\n", acpi_cpu_name[acpi_cpu_count][0], acpi_cpu_name[acpi_cpu_count][1], acpi_cpu_name[acpi_cpu_count][2], acpi_cpu_name[acpi_cpu_count][3]);

				if (++acpi_cpu_count == 32)
				{
					return;
				}
			}
		}
	}

	DBG("\tACPIpatcher: finished finding cpu names. Found: %d.\n", acpi_cpu_count);
}

static char const pss_ssdt_header[] =
{
	0x53, 0x53, 0x44, 0x54, 0x7E, 0x00, 0x00, 0x00, /* SSDT.... */
	0x01, 0x6A, 0x50, 0x6D, 0x52, 0x65, 0x66, 0x00, /* ..PmRef. */
	0x43, 0x70, 0x75, 0x50, 0x6D, 0x00, 0x00, 0x00, /* CpuPm... */
	0x00, 0x30, 0x00, 0x00, 0x49, 0x4E, 0x54, 0x4C, /* .0..INTL */
	0x31, 0x03, 0x10, 0x20,				/* 1.._		*/
};

static char const cst_ssdt_header[] =
{
	0x53, 0x53, 0x44, 0x54, 0xE7, 0x00, 0x00, 0x00, /* SSDT.... */
	0x01, 0x17, 0x50, 0x6D, 0x52, 0x65, 0x66, 0x41, /* ..PmRefA */
	0x43, 0x70, 0x75, 0x43, 0x73, 0x74, 0x00, 0x00, /* CpuCst.. */
	0x00, 0x10, 0x00, 0x00, 0x49, 0x4E, 0x54, 0x4C, /* ....INTL */
	0x31, 0x03, 0x10, 0x20				/* 1.._		*/
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

struct acpi_2_ssdt *generate_pss_ssdt(struct acpi_2_dsdt *dsdt)
{
	verbose("[ GENERATE P-STATES ]\n");

	if (Platform.CPU.Vendor != CPUID_VENDOR_INTEL) // 0x756E6547
	{
		DBG("\tNot an Intel platform: P-States will not be generated !!!\n");
		return NULL;
	}

	if (!(Platform.CPU.Features & CPU_FEATURE_MSR))
	{
		DBG("\tUnsupported CPU: P-States will not be generated !!! No MSR support\n");
		return NULL;
	}

	if (acpi_cpu_count == 0)
	{
		get_acpi_cpu_names((void*)dsdt, dsdt->Length);
	}

	if (acpi_cpu_count > 0)
	{
		struct p_state initial, maximum, minimum, p_states[32];
		uint8_t p_states_count = 0;

		// Retrieving P-States, ported from code by superhai (c)
		switch (Platform.CPU.Family)
		{
			case 0x06:
			{
				switch (Platform.CPU.Model)
				{
					case CPUID_MODEL_DOTHAN:	// Intel Pentium M
					case CPUID_MODEL_YONAH:		// Intel Mobile Core Solo, Duo
					case CPUID_MODEL_MEROM:		// Intel Mobile Core 2 Solo, Duo, Xeon 30xx, Xeon 51xx, Xeon X53xx, Xeon E53xx, Xeon X32xx
					case CPUID_MODEL_PENRYN:	// Intel Core 2 Solo, Duo, Quad, Extreme, Xeon X54xx, Xeon X33xx
					case CPUID_MODEL_ATOM:		// Intel Atom (45nm)
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
							DBG("\tP-States: Insane FID values!");
							p_states_count = 0;
						}
						else
						{
							uint8_t vidstep;
							uint8_t u, invalid = 0;
							// Finalize P-States
							// Find how many P-States machine supports
							p_states_count = (uint8_t)(maximum.CID - minimum.CID + 1);

							if (p_states_count > 32)
							{
								p_states_count = 32;
							}
							DBG("\tPStates count=%d\n", p_states_count);

							vidstep = ((maximum.VID << 2) - (minimum.VID << 2)) / (p_states_count - 1);

							for (u = 0; u < p_states_count; u++)
							{
								uint8_t i = u - invalid;

								p_states[i].CID = maximum.CID - u;
								p_states[i].FID = (uint8_t)(p_states[i].CID >> 1);

								if (p_states[i].FID < 0x6)
								{
									if (cpu_dynamic_fsb)
									{
										p_states[i].FID = (p_states[i].FID << 1) | 0x80;
									}
								}
								else if (cpu_noninteger_bus_ratio)
								{
									p_states[i].FID = p_states[i].FID | (0x40 * (p_states[i].CID & 0x1));
								}

								if (i && p_states[i].FID == p_states[i-1].FID)
								{
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
					case CPUID_MODEL_CLARKDALE:		//
					case CPUID_MODEL_DALES:			// Intel Core i3, i5 LGA1156 (32nm)
					case CPUID_MODEL_NEHALEM:		// Intel Core i7, Xeon W35xx, Xeon X55xx, Xeon E55xx LGA1366 (45nm)
					case CPUID_MODEL_NEHALEM_EX:		// Intel Xeon X75xx, Xeon X65xx, Xeon E75xx, Xeon E65xx
					case CPUID_MODEL_WESTMERE:		// Intel Core i7, Xeon X56xx, Xeon E56xx, Xeon W36xx LGA1366 (32nm) 6 Core
					case CPUID_MODEL_WESTMERE_EX:		// Intel Xeon E7
					case CPUID_MODEL_SANDYBRIDGE:		// Intel Core i3, i5, i7 LGA1155 (32nm)
					case CPUID_MODEL_JAKETOWN:		// Intel Core i7, Xeon E5 LGA2011 (32nm)
					case CPUID_MODEL_IVYBRIDGE:		// Intel Core i3, i5, i7 LGA1155 (22nm)
					case CPUID_MODEL_HASWELL:		//
					case CPUID_MODEL_IVYBRIDGE_XEON:	//
					//case CPUID_MODEL_HASWELL_H:		//
					case CPUID_MODEL_HASWELL_SVR:		//
					case CPUID_MODEL_HASWELL_ULT:		//
					case CPUID_MODEL_HASWELL_ULX:		//
					case CPUID_MODEL_BROADWELL_HQ:
					case CPUID_MODEL_SKYLAKE_S:
					case CPUID_MODEL_ATOM_3700:

					{
					if ( (Platform.CPU.Model == CPUID_MODEL_SANDYBRIDGE) ||
						(Platform.CPU.Model == CPUID_MODEL_JAKETOWN) ||
						(Platform.CPU.Model == CPUID_MODEL_IVYBRIDGE) ||
						(Platform.CPU.Model == CPUID_MODEL_HASWELL) ||
						(Platform.CPU.Model == CPUID_MODEL_IVYBRIDGE_XEON) ||
						(Platform.CPU.Model == CPUID_MODEL_HASWELL_SVR) ||
						(Platform.CPU.Model == CPUID_MODEL_HASWELL_ULT) ||
						(Platform.CPU.Model == CPUID_MODEL_HASWELL_ULX) ||
						(Platform.CPU.Model == CPUID_MODEL_ATOM_3700) )
					{
						maximum.Control = (rdmsr64(MSR_IA32_PERF_STATUS) >> 8) & 0xff;
					}
					else
					{
						maximum.Control = rdmsr64(MSR_IA32_PERF_STATUS) & 0xff;
					}

					minimum.Control = (rdmsr64(MSR_PLATFORM_INFO) >> 40) & 0xff;

					DBG("\tP-States: min 0x%x, max 0x%x\n", minimum.Control, maximum.Control);

					// Sanity check
					if (maximum.Control < minimum.Control)
					{
						DBG("\tInsane control values!");
						p_states_count = 0;
					}
					else
					{
						uint8_t i;
						p_states_count = 0;

						for (i = maximum.Control; i >= minimum.Control; i--)
						{
							p_states[p_states_count].Control = i;
							p_states[p_states_count].CID = p_states[p_states_count].Control << 1;
							p_states[p_states_count].Frequency = (Platform.CPU.FSBFrequency / 1000000) * i;
							p_states_count++;
						}
					}

					break;
				}
				default:
					DBG("\tUnsupported CPU (0x%X): P-States not generated !!!\n", Platform.CPU.Family);
					break;
			}
		}
	}

	// Generating SSDT
	if (p_states_count > 0)
	{

		int i;

		AML_CHUNK *root = aml_create_node(NULL);
		aml_add_buffer(root, pss_ssdt_header, sizeof(pss_ssdt_header)); // SSDT header

		AML_CHUNK *scop = aml_add_scope(root, "\\_PR_");
		AML_CHUNK *name = aml_add_name(scop, "PSS_");
		AML_CHUNK *pack = aml_add_package(name);

		for (i = 0; i < p_states_count; i++)
		{
			AML_CHUNK *pstt = aml_add_package(pack);

			aml_add_dword(pstt, p_states[i].Frequency);
			aml_add_dword(pstt, 0x00000000); // Power
			aml_add_dword(pstt, 0x0000000A); // Latency
			aml_add_dword(pstt, 0x0000000A); // Latency
			aml_add_dword(pstt, p_states[i].Control);
			aml_add_dword(pstt, i+1); // Status
		}

			// Add aliaces CPUs
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
			ssdt->Checksum = 0;
			ssdt->Checksum = (uint8_t)(256 - checksum8(ssdt, ssdt->Length));

			aml_destroy_node(root);

			//dumpPhysAddr("P-States SSDT content: ", ssdt, ssdt->Length);

			DBG("\tSSDT with CPU P-States generated successfully\n");

			return ssdt;
		}
	}
	else
	{
		DBG("\tACPI CPUs not found: P-States not generated !!!\n");
	}

	verbose("\n");

	return NULL;
}

struct acpi_2_ssdt *generate_cst_ssdt(struct acpi_2_fadt *fadt)
{
	verbose("[ GENERATE C-STATES ]\n");

	if (Platform.CPU.Vendor != CPUID_VENDOR_INTEL) // 0x756E6547
	{
		DBG("\tNot an Intel platform: C-States will not be generated !!!\n");
		return NULL;
	}

	if (fadt == NULL)
	{
		DBG("\tFACP not exists: C-States will not be generated !!!\n");
		return NULL;
	}

	struct acpi_2_dsdt *dsdt = (void *)fadt->DSDT;

	if (dsdt == NULL)
	{
		DBG("\tDSDT not found: C-States will not be generated !!!\n");
		return NULL;
	}

	if (acpi_cpu_count == 0)
		get_acpi_cpu_names((void*)dsdt, dsdt->Length);

	if (acpi_cpu_count > 0)
	{
		bool c2_enabled = false;
		bool c3_enabled = false;
		bool c4_enabled = false;
		bool c6_enabled = false;
		bool c7_enabled = false;
		bool cst_using_systemio = false;

		getBoolForKey(kEnableC2State, &c2_enabled, &bootInfo->chameleonConfig);
		getBoolForKey(kEnableC3State, &c3_enabled, &bootInfo->chameleonConfig);
		getBoolForKey(kEnableC4State, &c4_enabled, &bootInfo->chameleonConfig);
		getBoolForKey(kEnableC6State, &c6_enabled, &bootInfo->chameleonConfig);
		getBoolForKey(kEnableC7State, &c7_enabled, &bootInfo->chameleonConfig);
		getBoolForKey(kCSTUsingSystemIO, &cst_using_systemio, &bootInfo->chameleonConfig);

		c2_enabled = c2_enabled | (fadt->C2_Latency < 100);
		c3_enabled = c3_enabled | (fadt->C3_Latency < 1000);

		unsigned char cstates_count = 1 + (c2_enabled ? 1 : 0) + ((c3_enabled || c4_enabled)? 1 : 0) + (c6_enabled ? 1 : 0) + (c7_enabled ? 1 : 0);

		AML_CHUNK* root = aml_create_node(NULL);
		aml_add_buffer(root, cst_ssdt_header, sizeof(cst_ssdt_header)); // SSDT header
		AML_CHUNK* scop = aml_add_scope(root, "\\_PR_");
		AML_CHUNK* name = aml_add_name(scop, "CST_");
		AML_CHUNK* pack = aml_add_package(name);
		aml_add_byte(pack, cstates_count);

		AML_CHUNK* tmpl = aml_add_package(pack);
		if (cst_using_systemio)
		{
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
				aml_add_word(tmpl, 0x0043);		// Latency
				aml_add_dword(tmpl, 0x000001F4);	// Power
			}
			if (c6_enabled) // C6
			{
				p_blk_lo = acpi_cpu_p_blk + 5;
				p_blk_hi = (acpi_cpu_p_blk + 5) >> 8;

				tmpl = aml_add_package(pack);
				resource_template_register_systemio[11] = p_blk_lo; // C6
				resource_template_register_systemio[12] = p_blk_hi; // C6
				aml_add_buffer(tmpl, resource_template_register_systemio, sizeof(resource_template_register_systemio));
				aml_add_byte(tmpl, 0x06);			// C6
				aml_add_word(tmpl, 0x0046);			// Latency
				aml_add_dword(tmpl, 0x0000015E);		// Power
			}
			if (c7_enabled) //C7
			{
				p_blk_lo = (acpi_cpu_p_blk + 6) & 0xff;
				p_blk_hi = (acpi_cpu_p_blk + 5) >> 8;

				tmpl = aml_add_package(pack);
				resource_template_register_systemio[11] = p_blk_lo; // C4 or C7
				resource_template_register_systemio[12] = p_blk_hi;
				aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
				aml_add_byte(tmpl, 0x07);			// C7
				aml_add_word(tmpl, 0xF5);			// Latency as in iMac14,1
				aml_add_dword(tmpl, 0xC8);			// Power
			}
		}
		else
		{
			// C1
			resource_template_register_fixedhw[8] = 0x01;
			resource_template_register_fixedhw[9] = 0x02;
			resource_template_register_fixedhw[18] = 0x01;

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
				aml_add_word(tmpl, 0x0043);		// Latency
				aml_add_dword(tmpl, 0x000001F4);	// Power
			}
			if (c6_enabled) // C6
			{
				tmpl = aml_add_package(pack);
				resource_template_register_fixedhw[11] = 0x20; // C6
				aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
				aml_add_byte(tmpl, 0x06);			// C6
				aml_add_word(tmpl, 0x0046);			// Latency as in MacPro6,1
				aml_add_dword(tmpl, 0x0000015E);	// Power
			}
			if (c7_enabled) // C7
			{
				tmpl = aml_add_package(pack);
				resource_template_register_fixedhw[11] = 0x30; // C4 or C7
				aml_add_buffer(tmpl, resource_template_register_fixedhw, sizeof(resource_template_register_fixedhw));
				aml_add_byte(tmpl, 0x07);			// C7
				aml_add_word(tmpl, 0xF5);			// Latency as in iMac14,1
				aml_add_dword(tmpl, 0xC8);	// Power
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

		DBG("\tSSDT with CPU C-States generated successfully\n");

		return ssdt;
	}
	else
	{
		DBG("\tACPI CPUs not found: C-States not generated !!!\n");
	}

	verbose("\n");

	return NULL;
}
