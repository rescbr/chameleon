/*
 * Copyright 2008 mackerintel
 * 2010 mojodojo, 2012 slice
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

#if DEBUG_ACPI==2
	#define DBG(x...)  {printf(x); sleep(1);}
#elif DEBUG_ACPI==1
	#define DBG(x...)  printf(x)
#else
	#define DBG(x...)  msglog(x)
#endif

// Slice: New signature compare function
boolean_t tableSign(char *table, const char *sgn)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		if ((table[i] & ~0x20) != (sgn[i] & ~0x20))
		{
			return false;
		}
	}
	return true;
}

/* Gets the ACPI 1.0 RSDP address */
static struct acpi_2_rsdp *getAddressOfAcpiTable()
{
	/* TODO: Before searching the BIOS space we are supposed to search the first 1K of the EBDA */

	void *acpi_addr = (void*)ACPI_RANGE_START;

	for(; acpi_addr <= (void*)ACPI_RANGE_END; acpi_addr += 16)
	{
		if(*(uint64_t *)acpi_addr == ACPI_SIGNATURE_UINT64_LE)
		{
			uint8_t csum = checksum8(acpi_addr, 20);
			if (csum == 0)
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
static struct acpi_2_rsdp *getAddressOfAcpi20Table()
{
	/* TODO: Before searching the BIOS space we are supposed to search the first 1K of the EBDA */

	void *acpi_addr = (void *)ACPI_RANGE_START;

	for(; acpi_addr <= (void *)ACPI_RANGE_END; acpi_addr += 16)
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
				{
					return acpi_addr;
				}
			}
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
	strncpy(dirSpec, filename, sizeof(dirSpec) );
	fd = open(dirSpec, 0);
	if (fd < 0)
	{
		snprintf(dirSpec, sizeof(dirSpec), "/Extra/%s", filename); 
		fd = open(dirSpec, 0);
		if (fd < 0)
		{
			snprintf(dirSpec, sizeof(dirSpec), "bt(0,0)/Extra/%s", filename);
			fd = open(dirSpec, 0);
			if (fd < 0)
			{
				// NOT FOUND:
				DBG("\tACPI Table not found: %s\n", filename);
				*dirSpec = '\0';
			}
		}
	}

	if (outDirspec) *outDirspec = dirSpec; 
	return fd;
}

void *loadACPITable (const char *filename)
{
	const char *dirspec = NULL;

	int fd = search_and_get_acpi_fd(filename, &dirspec);

	if (fd >= 0)
	{
		void *tableAddr = (void *)AllocateKernelMemory(file_size(fd));
		if (tableAddr)
		{
			if (read(fd, tableAddr, file_size(fd)) != file_size(fd))
			{
				DBG("\tCouldn't read table %s\n", dirspec);
				free(tableAddr);
				close(fd);
				return NULL;
			}

			DBG("\tTable %s read and stored at: %x\n", dirspec, tableAddr);
			close(fd);
			return tableAddr;
		}
		close(fd);
		DBG("\tCouldn't allocate memory for table: %s.\n", dirspec);
	}
	//printf("Couldn't find table %s\n", filename);
	return NULL;
}

struct acpi_2_fadt *patch_fadt(struct acpi_2_fadt *fadt, struct acpi_2_dsdt *new_dsdt)
{
	extern void setupSystemType();

	struct acpi_2_fadt *fadt_mod = NULL;
	bool fadt_rev2_needed = false;
	bool fix_restart;
	bool fix_restart_ps2;
	const char * value;

	// Restart Fix
	if (Platform.CPU.Vendor == 0x756E6547)
	{	/* Intel */
		fix_restart = true;
		fix_restart_ps2 = false;
		if ( getBoolForKey(kPS2RestartFix, &fix_restart_ps2, &bootInfo->chameleonConfig) && fix_restart_ps2)
		{
			fix_restart = true;
		}
		else
		{
			getBoolForKey(kRestartFix, &fix_restart, &bootInfo->chameleonConfig);
		}
	}
	else
	{
		DBG("\tNot an Intel platform, FACP Restart Fix will not be applied!\n");
		fix_restart = false;
	}

	if (fix_restart)
	{
		fadt_rev2_needed = true;
	}

	// Allocate new fadt table
	if ((fadt->Length < 0x84) && fadt_rev2_needed)
	{
		fadt_mod = (struct acpi_2_fadt *)AllocateKernelMemory(0x84);
		memcpy(fadt_mod, fadt, fadt->Length);
		fadt_mod->Length   = 0x84;
		fadt_mod->Revision = 0x02; // FACP rev 2 (ACPI 1.0B MS extensions)
	}
	else
	{
		fadt_mod = (struct acpi_2_fadt *)AllocateKernelMemory(fadt->Length);
		memcpy(fadt_mod, fadt, fadt->Length);
	}
	// Determine system type / PM_Model
	if ( (value=getStringForKey(kSystemType, &bootInfo->chameleonConfig))!=NULL)
	{
		if (Platform.Type > 6)
		{
			if(fadt_mod->PM_Profile<=6)
			{
				Platform.Type = fadt_mod->PM_Profile; // get the fadt if correct
			}
			else
			{
				Platform.Type = 1;		/* Set a fixed value (Desktop) */
			}
			DBG("\tError: system-type must be 0..6. Defaulting to %d !\n", Platform.Type);
		}
		else
		{
			Platform.Type = (unsigned char) strtoul(value, NULL, 10);
		}
	}
	// Set PM_Profile from System-type if only user wanted this value to be forced
	if (fadt_mod->PM_Profile != Platform.Type)
	{
		if (value)
		{
			// user has overriden the SystemType so take care of it in FACP
			DBG("\tFADT: changing PM_Profile from 0x%02x to 0x%02x\n", fadt_mod->PM_Profile, Platform.Type);
			fadt_mod->PM_Profile = Platform.Type;
		}
		else
		{
			// PM_Profile has a different value and no override has been set, so reflect the user value to ioregs
			Platform.Type = fadt_mod->PM_Profile <= 6 ? fadt_mod->PM_Profile : 1;
		}
	}
	// We now have to write the systemm-type in ioregs: we cannot do it before in setupDeviceTree()
	// because we need to take care of FACP original content, if it is correct.
	setupSystemType();

	// Patch FACP to fix restart
	if (fix_restart)
	{
		if (fix_restart_ps2)
		{
			fadt_mod->Flags|= 0x400;
			fadt_mod->Reset_SpaceID		= 0x01;   // System I/O
			fadt_mod->Reset_BitWidth	= 0x08;   // 1 byte
			fadt_mod->Reset_BitOffset	= 0x00;   // Offset 0
			fadt_mod->Reset_AccessWidth	= 0x01;   // Byte access
			fadt_mod->Reset_Address		= 0x64;   // Address of the register
			fadt_mod->Reset_Value		= 0xfe;   // Value to write to reset the system
			DBG("\tFACP PS2 Restart Fix applied!\n");
		}
		else
		{
			fadt_mod->Flags|= 0x400;
			fadt_mod->Reset_SpaceID		= 0x01;   // System I/O
			fadt_mod->Reset_BitWidth	= 0x08;   // 1 byte
			fadt_mod->Reset_BitOffset	= 0x00;   // Offset 0
			fadt_mod->Reset_AccessWidth	= 0x01;   // Byte access
			fadt_mod->Reset_Address		= 0x0cf9; // Address of the register
			fadt_mod->Reset_Value		= 0x06;   // Value to write to reset the system
			DBG("\tFACP Restart Fix applied!\n");
		}

	}

	// Bungo: Save Hardware Signature (machine-signature)
	if ((fadt_mod->FACS > 0) && (fadt_mod->FACS < 0xFFFFFFFF) && (((struct acpi_2_facs *)fadt_mod->FACS)->Length >= 64))
	{
		Platform.HWSignature = ((struct acpi_2_facs *)fadt_mod->FACS)->HWSignature;
		DBG("\tHardware Signature=0x%08X: using.\n", Platform.HWSignature);
	}
	else
	{
		Platform.HWSignature = 0;
		DBG("\tFixing Hardware Signature=0x%08X.\n", Platform.HWSignature);
	}

	// Patch DSDT address if we have loaded DSDT.aml
	if (new_dsdt)
	{
		DBG("\tDSDT: Old @%x,%x, ",fadt_mod->DSDT,fadt_mod->X_DSDT);

		fadt_mod->DSDT = (uint32_t)new_dsdt;
		if ((uint32_t)(&(fadt_mod->X_DSDT)) - (uint32_t)fadt_mod + 8<=fadt_mod->Length)
		{
			fadt_mod->X_DSDT = (uint32_t)new_dsdt;
		}

		DBG("\tNew @%x,%x\n",fadt_mod->DSDT,fadt_mod->X_DSDT);

		DBG("\tFADT: Using custom DSDT!\n");
	}

	// Correct the checksum
	fadt_mod->Checksum=0;
	fadt_mod->Checksum=256-checksum8(fadt_mod,fadt_mod->Length);

	return fadt_mod;
}

/* Setup ACPI without replacing DSDT. */
int setupAcpiNoMod()
{
//	addConfigurationTable(&gEfiAcpiTableGuid, getAddressOfAcpiTable(), "ACPI");
//	addConfigurationTable(&gEfiAcpi20TableGuid, getAddressOfAcpi20Table(), "ACPI_20");
	/* XXX aserebln why uint32 cast if pointer is uint64 ? */
	acpi10_p = (uint64_t)(uint32_t)getAddressOfAcpiTable();
	acpi20_p = (uint64_t)(uint32_t)getAddressOfAcpi20Table();
	addConfigurationTable(&gEfiAcpiTableGuid, &acpi10_p, "ACPI");
	if(acpi20_p)
	{
		addConfigurationTable(&gEfiAcpi20TableGuid, &acpi20_p, "ACPI_20");
	}
	else
	{
		DBG("\tNo ACPI 2.\n");
	}
	return 1;
}

/* Setup ACPI. Replace DSDT if DSDT.aml is found */
int setupAcpi(void)
{
	verbose("[ ACPI PATCHER ]\n");
	int version;
	void *new_dsdt = NULL;


	const char *filename;
	char dirSpec[128];
	int len = 0;

	// always reset cpu count to 0 when injecting new acpi
	acpi_cpu_count = 0;

	/* Try using the file specified with the DSDT option */
	if (getValueForKey(kDSDT, &filename, &len, &bootInfo->chameleonConfig))
	{
		strncpy(dirSpec, filename, sizeof(dirSpec) );
	}
	else
	{
		strcpy(dirSpec, "DSDT.aml");
		//verbose("dirSpec, DSDT.aml");
	}

	// Load replacement DSDT
	new_dsdt = loadACPITable(dirSpec);

	// Mozodojo: going to patch FACP and load SSDT's even if DSDT.aml is not present
	/*if (!new_dsdt)
	 {
	 return setupAcpiNoMod();
	 }*/

	// Mozodojo: Load additional SSDTs
	struct acpi_2_ssdt *new_ssdt[32]; // 30 + 2 additional tables for pss & cst
	int  ssdt_count=0;

	// SSDT Options
	bool drop_ssdt = false, generate_pstates = false, generate_cstates = false;

	getBoolForKey(kDropSSDT, &drop_ssdt, &bootInfo->chameleonConfig);
	getBoolForKey(kGeneratePStates, &generate_pstates, &bootInfo->chameleonConfig);
	getBoolForKey(kGenerateCStates, &generate_cstates, &bootInfo->chameleonConfig);
	//getBoolForKey(kGenerateTStates, &generate_tstates, &bootInfo->chameleonConfig);

	DBG("\tGenerating P-States config: %s\n", generate_pstates ? "Yes" : "No");
	DBG("\tGenerating C-States config: %s\n", generate_cstates ? "Yes" : "No");
	//DBG("Generating T-States config: %s\n", generate_tstates ? "Yes" : "No");

	{
		int i;

		for (i = 0; i < 30; i++)
		{
			char filename[512];

			if (i > 0)
			{
				sprintf(filename, "SSDT-%d.aml", i);
			}
			else
			{
				strcpy(filename, "SSDT.aml");
			}

			if ( (new_ssdt[ssdt_count] = loadACPITable(filename)) )
			{
				ssdt_count++;
			}
			else
			{
				break;
			}
		}
	}

	// Do the same procedure for both versions of ACPI
	for (version = 0; version < 2; version++)
	{
		struct acpi_2_rsdp *rsdp, *rsdp_mod;
		struct acpi_2_rsdt *rsdt, *rsdt_mod;
		int rsdplength;

		// Find original rsdp
		rsdp = (struct acpi_2_rsdp *)(version ? getAddressOfAcpi20Table() : getAddressOfAcpiTable());
		if (!rsdp)
		{
			DBG("\tNo ACPI version %d found. Ignoring\n", version+1);
			if (version)
			{
				addConfigurationTable(&gEfiAcpi20TableGuid, NULL, "ACPI_20");
			}
			else
			{
				addConfigurationTable(&gEfiAcpiTableGuid, NULL, "ACPI");
			}
			continue;
		}
		rsdplength = version ? rsdp->Length : 20;

		DBG("\tRSDP version %d found @%x. Length=%d\n",version+1,rsdp,rsdplength);

		/* FIXME: no check that memory allocation succeeded 
		 * Copy and patch RSDP,RSDT, XSDT and FADT
		 * For more info see ACPI Specification pages 110 and following
		 */

		rsdp_mod = (struct acpi_2_rsdp *) AllocateKernelMemory(rsdplength);
		memcpy(rsdp_mod, rsdp, rsdplength);

		rsdt = (struct acpi_2_rsdt *)rsdp->RsdtAddress;

		DBG("\tRSDT @%x, Length %d\n",rsdt, rsdt ? rsdt->Length : 0);

		if (rsdt && (uint32_t)rsdt !=0xffffffff && rsdt->Length < 0x10000)
		{
			uint32_t *rsdt_entries;
			int rsdt_entries_num;
			int dropoffset = 0, i;

			// mozo: using malloc cos I didn't found how to free already allocated kernel memory
			rsdt_mod = (struct acpi_2_rsdt *)malloc(rsdt->Length);
			memcpy(rsdt_mod, rsdt, rsdt->Length);
			rsdp_mod->RsdtAddress = (uint32_t)rsdt_mod;
			rsdt_entries_num = (rsdt_mod->Length - sizeof(struct acpi_2_rsdt)) / 4;
			rsdt_entries = (uint32_t *)(rsdt_mod + 1);
			for (i = 0; i < rsdt_entries_num; i++)
			{
				char *table=(char *)(rsdt_entries[i]);
				if (!table)
				{
					continue;
				}

				DBG("\tTABLE %c%c%c%c,",table[0],table[1],table[2],table[3]);

				rsdt_entries[i-dropoffset]=rsdt_entries[i];

				// Dropping DMAR (DMA Remapping table) table can fix stuck on "waitForSystemMapper" or "PCI configuration begin" if nothing wokrks except rolling old AppleACPIplatform.kext back. Needs testing with current OEM AppleACPIplatform.kext not with old rolled back one
				if (tableSign(table, "DMAR"))
				{
					DBG("\tOEM DMAR table was dropped\n");
					dropoffset++;
					continue;
				}

				if (drop_ssdt && tableSign(table, "SSDT"))
				{
					DBG("\tOEM SSDT tables was dropped\n");
					dropoffset++;
					continue;
				}

				if (tableSign(table, "DSDT"))
				{
					DBG("\tDSDT found\n");
					DBG("\tCustom DSDT table was found\n");
					if(new_dsdt)
					{
						rsdt_entries[i-dropoffset] = (uint32_t)new_dsdt;
					}

					continue;
				}

				if (tableSign(table, "FACP"))
				{
					struct acpi_2_fadt *fadt, *fadt_mod;
					fadt=(struct acpi_2_fadt *)rsdt_entries[i];

					DBG("\tFADT found @%x, Length %d\n",fadt, fadt->Length);

					if (!fadt || (uint32_t)fadt == 0xffffffff || fadt->Length>0x10000)
					{
						DBG("\tFADT incorrect. Not modified\n");
						continue;
					}
					
					fadt_mod = patch_fadt(fadt, new_dsdt);
					rsdt_entries[i-dropoffset] = (uint32_t)fadt_mod;
					
					// Generate _CST SSDT
					if (generate_cstates && (new_ssdt[ssdt_count] = generate_cst_ssdt(fadt_mod)))
					{
						DBG("\tC-States generated.\n");
						generate_cstates = false; // Generate SSDT only once!
						ssdt_count++;
					}

					// Generating _PSS SSDT
					if (generate_pstates && (new_ssdt[ssdt_count] = generate_pss_ssdt((void*)fadt_mod->DSDT)))
					{
						DBG("\tP-States generated.\n");
						generate_pstates = false; // Generate SSDT only once!
						ssdt_count++;
					}
					continue;
				}
			}
			DBG("\n");

			// Allocate rsdt in Kernel memory area
			rsdt_mod->Length += 4*ssdt_count - 4*dropoffset;
			struct acpi_2_rsdt *rsdt_copy = (struct acpi_2_rsdt *)AllocateKernelMemory(rsdt_mod->Length);
			memcpy(rsdt_copy, rsdt_mod, rsdt_mod->Length);
			free(rsdt_mod);
			rsdt_mod = rsdt_copy;
			rsdp_mod->RsdtAddress = (uint32_t)rsdt_mod;
			rsdt_entries_num = (rsdt_mod->Length-sizeof(struct acpi_2_rsdt)) / 4;
			rsdt_entries = (uint32_t *)(rsdt_mod + 1);

			// Mozodojo: Insert additional SSDTs into RSDT
			if(ssdt_count > 0)
			{
				int j;

				for (j=0; j<ssdt_count; j++)
				{
					rsdt_entries[i-dropoffset+j] = (uint32_t)new_ssdt[j];
				}

				DBG("\tRSDT: Added %d SSDT table(s)\n", ssdt_count);

			}

			// Correct the checksum of RSDT
			DBG("\tRSDT: Original checksum %d, ", rsdt_mod->Checksum);

			rsdt_mod->Checksum=0;
			rsdt_mod->Checksum=256-checksum8(rsdt_mod,rsdt_mod->Length);

			DBG("\tNew checksum %d at %x\n", rsdt_mod->Checksum,rsdt_mod);
		}
		else
		{
			rsdp_mod->RsdtAddress=0;
			DBG("\tRSDT not found or RSDT incorrect\n");
		}
		DBG("\n");

		if (version)
		{
			struct acpi_2_xsdt *xsdt, *xsdt_mod;

			// FIXME: handle 64-bit address correctly

			xsdt=(struct acpi_2_xsdt*) ((uint32_t)rsdp->XsdtAddress);
			DBG("\tXSDT @%x;%x, Length=%d\n", (uint32_t)(rsdp->XsdtAddress>>32),(uint32_t)rsdp->XsdtAddress, xsdt->Length);

			if (xsdt && (uint64_t)rsdp->XsdtAddress<0xffffffff && xsdt->Length<0x10000)
			{
				uint64_t *xsdt_entries;
				int xsdt_entries_num, i;
				int dropoffset = 0;

				// mozo: using malloc cos I didn't found how to free already allocated kernel memory
				xsdt_mod = (struct acpi_2_xsdt *)malloc(xsdt->Length);
				memcpy(xsdt_mod, xsdt, xsdt->Length);

				rsdp_mod->XsdtAddress = (uint32_t)xsdt_mod;
				xsdt_entries_num = (xsdt_mod->Length - sizeof(struct acpi_2_xsdt)) / 8;
				xsdt_entries = (uint64_t *)(xsdt_mod + 1);
				for (i = 0;i < xsdt_entries_num; i++)
				{
					char *table = (char *)((uint32_t)(xsdt_entries[i]));
					if (!table)
					{
						continue;
					}
					xsdt_entries[i - dropoffset] = xsdt_entries[i];

					// Dropping DMAR (DMA Remapping table) table can fix stuck on "waitForSystemMapper" or "PCI configuration begin" if nothing wokrks except rolling old AppleACPIplatform.kext back. Needs testing with current OEM AppleACPIplatform.kext not with old rolled back one
					if (tableSign(table, "DMAR"))
					{
						DBG("\tOEM DMAR table was dropped\n");
						dropoffset++;
						continue;
					}

					if (drop_ssdt && tableSign(table, "SSDT"))
					{
						DBG("\tOEM SSDT tables was dropped\n");
						dropoffset++;
						continue;
					}
					if (tableSign(table, "DSDT"))
					{
						DBG("\tDSDT found\n");

						if (new_dsdt)
						{
							xsdt_entries[i-dropoffset] = (uint32_t)new_dsdt;
							DBG("\tcustom table added.\n");
						}

						DBG("\tTABLE %c%c%c%c@%x \n", table[0],table[1],table[2],table[3],xsdt_entries[i]);

						continue;
					}
					if (tableSign(table, "FACP"))
					{
						struct acpi_2_fadt *fadt, *fadt_mod;
						fadt=(struct acpi_2_fadt *)(uint32_t)xsdt_entries[i];

						DBG("\tFADT found @%x%x, Length %d\n",(uint32_t)(xsdt_entries[i]>>32),fadt, 
							fadt->Length);

						if (!fadt || (uint64_t)xsdt_entries[i] >= 0xffffffff || fadt->Length>0x10000)
						{
							DBG("\tFADT incorrect or after 4GB. Dropping XSDT\n");
							goto drop_xsdt;
						}

						fadt_mod = patch_fadt(fadt, new_dsdt);
						xsdt_entries[i - dropoffset] = (uint32_t)fadt_mod;

						// DBG("\tTABLE %c%c%c%c@%x \n", table[0],table[1],table[2],table[3],xsdt_entries[i]);

						// Generate _CST SSDT
						if (generate_cstates && (new_ssdt[ssdt_count] = generate_cst_ssdt(fadt_mod)))
						{
							DBG("\tC-States generated.\n");
							generate_cstates = false; // Generate SSDT only once!
							ssdt_count++;
						}

						// Generating _PSS SSDT
						if (generate_pstates && (new_ssdt[ssdt_count] = generate_pss_ssdt((void*)fadt_mod->DSDT)))
						{
							DBG("\tP-States generated.\n");
							generate_pstates = false; // Generate SSDT only once!
							ssdt_count++;
						}

						// Generating _TSS SSDT
						/*if (generate_tstates && (new_ssdt[ssdt_count] = generate_tss_ssdt((void*)fadt_mod->DSDT)))
						{
							generate_tstates = false; // Generate SSDT only once!
							ssdt_count++;
						}*/
						continue;
					}
					DBG("\tcopied (OEM)\n");
					// DBG("\tTABLE %c%c%c%c@%x \n", table[0],table[1],table[2],table[3],xsdt_entries[i]);
				}

				// Allocate xsdt in Kernel memory area
				xsdt_mod->Length += 8*ssdt_count - 8*dropoffset;
				struct acpi_2_xsdt *xsdt_copy = (struct acpi_2_xsdt *)AllocateKernelMemory(xsdt_mod->Length);
				memcpy(xsdt_copy, xsdt_mod, xsdt_mod->Length);
				free(xsdt_mod);
				xsdt_mod = xsdt_copy;
				rsdp_mod->XsdtAddress = (uint32_t)xsdt_mod;
				xsdt_entries_num = (xsdt_mod->Length - sizeof(struct acpi_2_xsdt)) / 8;
				xsdt_entries = (uint64_t *)(xsdt_mod + 1);

				// Mozodojo: Insert additional SSDTs into XSDT
				if(ssdt_count > 0)
				{
					int j;

					for (j=0; j<ssdt_count; j++)
					{
						xsdt_entries[i - dropoffset + j] = (uint32_t)new_ssdt[j];
					}

					verbose("\tAdded %d SSDT table(s) into XSDT\n", ssdt_count);

				}

				// Correct the checksum of XSDT
				xsdt_mod->Checksum=0;
				xsdt_mod->Checksum=256-checksum8(xsdt_mod,xsdt_mod->Length);
			}
			else
			{
			drop_xsdt:

				DBG("\tAbout to drop XSDT\n");

				/*FIXME: Now we just hope that if MacOS doesn't find XSDT it reverts to RSDT. 
				 * A Better strategy would be to generate
				 */

				rsdp_mod->XsdtAddress=0xffffffffffffffffLL;
				verbose("\tXSDT not found or XSDT incorrect\n");
			}
		}
		DBG("\n");

		// Correct the checksum of RSDP

		DBG("\tRSDP: Original checksum %d, ", rsdp_mod->Checksum);

		rsdp_mod->Checksum=0;
		rsdp_mod->Checksum=256-checksum8(rsdp_mod,20);

		DBG("\tNew checksum %d\n", rsdp_mod->Checksum);

		if (version)
		{
			DBG("\tRSDP: Original extended checksum %d, ", rsdp_mod->ExtendedChecksum);

			rsdp_mod->ExtendedChecksum=0;
			rsdp_mod->ExtendedChecksum=256-checksum8(rsdp_mod,rsdp_mod->Length);

			DBG("\tNew extended checksum %d\n", rsdp_mod->ExtendedChecksum);

		}

		if (version)
		{
			/* XXX aserebln why uint32 cast if pointer is uint64 ? */
			acpi20_p = (uint64_t)(uint32_t)rsdp_mod;
			addConfigurationTable(&gEfiAcpi20TableGuid, &acpi20_p, "ACPI_20");
		}
		else
		{
			/* XXX aserebln why uint32 cast if pointer is uint64 ? */
			acpi10_p = (uint64_t)(uint32_t)rsdp_mod;
			addConfigurationTable(&gEfiAcpiTableGuid, &acpi10_p, "ACPI");
		}
		DBG("\tACPI version %d patching finished\n\n", version + 1);
	}
#if DEBUG_ACPI
	printf("Press a key to continue... (DEBUG_ACPI)\n");
	getchar();
#endif
	return 1;
}
