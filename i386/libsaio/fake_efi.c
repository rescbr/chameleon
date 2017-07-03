
/*
 * Copyright 2007 David F. Elliott.	 All rights reserved.
 */

#include "config.h"
#include "saio_types.h"
#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "efi.h"
#include "acpi.h"
#include "fake_efi.h"
#include "efi_tables.h"
#include "platform.h"
#include "acpi_patcher.h"
#include "smbios.h"
#include "device_inject.h"
#include "convert.h"
#include "pci.h"
#include "sl.h"
#include "vers.h"

#if DEBUG_EFI
	#define DBG(x...)	printf(x)
#else
	#define DBG(x...)
#endif

extern void setup_pci_devs(pci_dt_t *pci_dt);

/*
 * Modern Darwin kernels require some amount of EFI because Apple machines all
 * have EFI. Modifying the kernel source to not require EFI is of course
 * possible but would have to be maintained as a separate patch because it is
 * unlikely that Apple wishes to add legacy support to their kernel.
 *
 * As you can see from the Apple-supplied code in bootstruct.c, it seems that
 * the intention was clearly to modify this booter to provide EFI-like structures
 * to the kernel rather than modifying the kernel to handle non-EFI stuff. This
 * makes a lot of sense from an engineering point of view as it means the kernel
 * for the as yet unreleased EFI-only Macs could still be booted by the non-EFI
 * DTK systems so long as the kernel checked to ensure the boot tables were
 * filled in appropriately.	Modern xnu requires a system table and a runtime
 * services table and performs no checks whatsoever to ensure the pointers to
 * these tables are non-NULL. Therefore, any modern xnu kernel will page fault
 * early on in the boot process if the system table pointer is zero.
 *
 * Even before that happens, the tsc_init function in modern xnu requires the FSB
 * Frequency to be a property in the /efi/platform node of the device tree or else
 * it panics the bootstrap process very early on.
 *
 * As of this writing, the current implementation found here is good enough
 * to make the currently available xnu kernel boot without modification on a
 * system with an appropriate processor. With a minor source modification to
 * the tsc_init function to remove the explicit check for Core or Core 2
 * processors the kernel can be made to boot on other processors so long as
 * the code can be executed by the processor and the machine contains the
 * necessary hardware.
 */

/*==========================================================================
 * Utility function to make a device tree string from an EFI_GUID
 */
static inline char * mallocStringForGuid(EFI_GUID const *pGuid)
{
	char *string = malloc(37);
	efi_guid_unparse_upper(pGuid, string);
	return string;
}

/*==========================================================================
 * Function to map 32 bit physical address to 64 bit virtual address
 */
static uint64_t ptov64(uint32_t addr)
{
	return ((uint64_t)addr | 0xFFFFFF8000000000ULL);
}

// ==========================================================================

EFI_UINT32 getCPUTick(void)
{
	uint32_t out;
	/*
	 * Note: shl $32, %edx leaves 0 in %edx, and or to %eax does nothing - zenith432
	 */
	__asm__ volatile (
		"rdtsc\n"
		"shl $32,%%edx\n"
		"or %%edx,%%eax\n"
		: "=a" (out)
		:
		: "%edx"
	);
	return out;
}

/*==========================================================================
 * Fake EFI implementation
 */

/* Identify ourselves as the EFI firmware vendor */
static EFI_CHAR16 const FIRMWARE_VENDOR[] = {'C','h','a','m','e','l','e','o','n','_','2','.','3', 0};

static EFI_UINT32 const FIRMWARE_REVISION = EFI_SYSTEM_TABLE_REVISION;

/* Default platform system_id (fix by IntVar)
 static EFI_CHAR8 const SYSTEM_ID[] = "0123456789ABCDEF"; //random value gen by uuidgen
 */

/* Just a ret instruction */
static uint8_t const VOIDRET_INSTRUCTIONS[] = {0xc3};

/* movl $0x80000003,%eax; ret */
static uint8_t const UNSUPPORTEDRET_INSTRUCTIONS_32[] = {0xb8, 0x03, 0x00, 0x00, 0x80, 0xc3};
static uint8_t const UNSUPPORTEDRET_INSTRUCTIONS_64[] = {0x48, 0xb8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc3};

EFI_SYSTEM_TABLE_32 *gST32 = NULL;
EFI_SYSTEM_TABLE_64 *gST64 = NULL;
Node *gEfiConfigurationTableNode = NULL;

// ==========================================================================

extern EFI_STATUS addConfigurationTable(EFI_GUID const *pGuid, void *table, char const *alias)
{
	EFI_UINTN i = 0;

	//Azi: as is, cpu's with em64t will use EFI64 on pre 10.6 systems,
	// wich seems to cause no problem. In case it does, force i386 arch.
	if (archCpuType == CPU_TYPE_I386)
	{
		i = gST32->NumberOfTableEntries;
	}
	else
	{
		i = gST64->NumberOfTableEntries;
	}

	// We only do adds, not modifications and deletes like InstallConfigurationTable
	if (i >= MAX_CONFIGURATION_TABLE_ENTRIES)
	{
		stop("Fake EFI [ERROR]: Ran out of space for configuration tables [%d]. Increase the reserved size in the code.\n", i);
	}

	if (pGuid == NULL)
	{
		return EFI_INVALID_PARAMETER;
	}

	if (table != NULL)
	{
		// FIXME
		//((EFI_CONFIGURATION_TABLE_64 *)gST->ConfigurationTable)[i].VendorGuid = *pGuid;
		//((EFI_CONFIGURATION_TABLE_64 *)gST->ConfigurationTable)[i].VendorTable = (EFI_PTR64)table;

		//++gST->NumberOfTableEntries;

		Node *tableNode = DT__AddChild(gEfiConfigurationTableNode, mallocStringForGuid(pGuid));

		// Use the pointer to the GUID we just stuffed into the system table
		DT__AddProperty(tableNode, "guid", sizeof(EFI_GUID), (void *)pGuid);

		// The "table" property is the 32-bit (in our implementation) physical address of the table
		DT__AddProperty(tableNode, "table", sizeof(void *) * 2, table);

		// Assume the alias pointer is a global or static piece of data
		if (alias != NULL)
		{
			DT__AddProperty(tableNode, "alias", strlen(alias)+1, (char *)alias);
		}

		return EFI_SUCCESS;
	}
	return EFI_UNSUPPORTED;
}

// ==========================================================================

//Azi: crc32 done in place, on the cases were it wasn't.
/*static inline void fixupEfiSystemTableCRC32(EFI_SYSTEM_TABLE_64 *efiSystemTable)
{
	efiSystemTable->Hdr.CRC32 = 0;
	efiSystemTable->Hdr.CRC32 = crc32(0L, efiSystemTable, efiSystemTable->Hdr.HeaderSize);
}*/

/*
 * What we do here is simply allocate a fake EFI system table and a fake EFI
 * runtime services table.
 *
 * Because we build against modern headers with kBootArgsRevision 4 we
 * also take care to set efiMode = 32.
 */
void setupEfiTables32(void)
{
	// We use the fake_efi_pages struct so that we only need to do one kernel
	// memory allocation for all needed EFI data.  Otherwise, small allocations
	// like the FIRMWARE_VENDOR string would take up an entire page.
	// NOTE WELL: Do NOT assume this struct has any particular layout within itself.
	// It is absolutely not intended to be publicly exposed anywhere
	// We say pages (plural) although right now we are well within the 1 page size
	// and probably will stay that way.
	struct fake_efi_pages
	{
		EFI_SYSTEM_TABLE_32 efiSystemTable;
		EFI_RUNTIME_SERVICES_32 efiRuntimeServices;
		EFI_CONFIGURATION_TABLE_32 efiConfigurationTable[MAX_CONFIGURATION_TABLE_ENTRIES];
		EFI_CHAR16 firmwareVendor[sizeof(FIRMWARE_VENDOR)/sizeof(EFI_CHAR16)];
		uint8_t voidret_instructions[sizeof(VOIDRET_INSTRUCTIONS)/sizeof(uint8_t)];
		uint8_t unsupportedret_instructions[sizeof(UNSUPPORTEDRET_INSTRUCTIONS_32)/sizeof(uint8_t)];
	};

	struct fake_efi_pages *fakeEfiPages = (struct fake_efi_pages *)AllocateKernelMemory(sizeof(struct fake_efi_pages));

	// Zero out all the tables in case fields are added later
	//bzero(fakeEfiPages, sizeof(struct fake_efi_pages));
	
	// --------------------------------------------------------------------
	// Initialize some machine code that will return EFI_UNSUPPORTED for
	// functions returning int and simply return for void functions.
	memcpy(fakeEfiPages->voidret_instructions, VOIDRET_INSTRUCTIONS, sizeof(VOIDRET_INSTRUCTIONS));
	memcpy(fakeEfiPages->unsupportedret_instructions, UNSUPPORTEDRET_INSTRUCTIONS_32, sizeof(UNSUPPORTEDRET_INSTRUCTIONS_32));
	
	// --------------------------------------------------------------------
	// System table
	EFI_SYSTEM_TABLE_32 *efiSystemTable = gST32 = &fakeEfiPages->efiSystemTable;
	efiSystemTable->Hdr.Signature = EFI_SYSTEM_TABLE_SIGNATURE;
	efiSystemTable->Hdr.Revision = EFI_SYSTEM_TABLE_REVISION;
	efiSystemTable->Hdr.HeaderSize = sizeof(EFI_SYSTEM_TABLE_32);
	efiSystemTable->Hdr.CRC32 = 0; // Initialize to zero and then do CRC32
	efiSystemTable->Hdr.Reserved = 0;
	
	efiSystemTable->FirmwareVendor = (EFI_PTR32)&fakeEfiPages->firmwareVendor;
	memcpy(fakeEfiPages->firmwareVendor, FIRMWARE_VENDOR, sizeof(FIRMWARE_VENDOR));
	efiSystemTable->FirmwareRevision = FIRMWARE_REVISION;
	
	// XXX: We may need to have basic implementations of ConIn/ConOut/StdErr
	// The EFI spec states that all handles are invalid after boot services have been
	// exited so we can probably get by with leaving the handles as zero.
	efiSystemTable->ConsoleInHandle = 0;
	efiSystemTable->ConIn = 0;

	efiSystemTable->ConsoleOutHandle = 0;
	efiSystemTable->ConOut = 0;

	efiSystemTable->StandardErrorHandle = 0;
	efiSystemTable->StdErr = 0;

	efiSystemTable->RuntimeServices = (EFI_PTR32)&fakeEfiPages->efiRuntimeServices;

	// According to the EFI spec, BootServices aren't valid after the
	// boot process is exited so we can probably do without it.
	// Apple didn't provide a definition for it in pexpert/i386/efi.h
	// so I'm guessing they don't use it.
	efiSystemTable->BootServices = 0;

	efiSystemTable->NumberOfTableEntries = 0;
	efiSystemTable->ConfigurationTable = (EFI_PTR32)fakeEfiPages->efiConfigurationTable;

	// We're done. Now CRC32 the thing so the kernel will accept it.
	// Must be initialized to zero before CRC32, done above.
	gST32->Hdr.CRC32 = crc32(0L, gST32, gST32->Hdr.HeaderSize);

	// --------------------------------------------------------------------
	// Runtime services
	EFI_RUNTIME_SERVICES_32 *efiRuntimeServices = &fakeEfiPages->efiRuntimeServices;
	efiRuntimeServices->Hdr.Signature = EFI_RUNTIME_SERVICES_SIGNATURE;
	efiRuntimeServices->Hdr.Revision = EFI_RUNTIME_SERVICES_REVISION;
	efiRuntimeServices->Hdr.HeaderSize = sizeof(EFI_RUNTIME_SERVICES_32);
	efiRuntimeServices->Hdr.CRC32 = 0;
	efiRuntimeServices->Hdr.Reserved = 0;

	// There are a number of function pointers in the efiRuntimeServices table.
	// These are the Foundation (e.g. core) services and are expected to be present on
	// all EFI-compliant machines.	Some kernel extensions (notably AppleEFIRuntime)
	// will call these without checking to see if they are null.
	//
	// We don't really feel like doing an EFI implementation in the bootloader
	// but it is nice if we can at least prevent a complete crash by
	// at least providing some sort of implementation until one can be provided
	// nicely in a kext.
	void (*voidret_fp)() = (void *)fakeEfiPages->voidret_instructions;
	void (*unsupportedret_fp)() = (void *)fakeEfiPages->unsupportedret_instructions;
	efiRuntimeServices->GetTime = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->SetTime = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->GetWakeupTime = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->SetWakeupTime = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->SetVirtualAddressMap = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->ConvertPointer = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->GetVariable = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->GetNextVariableName = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->SetVariable = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->GetNextHighMonotonicCount = (EFI_PTR32)unsupportedret_fp;
	efiRuntimeServices->ResetSystem = (EFI_PTR32)voidret_fp;

	// We're done.	Now CRC32 the thing so the kernel will accept it
	efiRuntimeServices->Hdr.CRC32 = crc32(0L, efiRuntimeServices, efiRuntimeServices->Hdr.HeaderSize);

	// --------------------------------------------------------------------
	// Finish filling in the rest of the boot args that we need.
	bootArgs->efiSystemTable = (uint32_t)efiSystemTable;
	bootArgs->efiMode = kBootArgsEfiMode32;

	// The bootArgs structure as a whole is bzero'd so we don't need to fill in
	// things like efiRuntimeServices* and what not.
	//
	// In fact, the only code that seems to use that is the hibernate code so it
	// knows not to save the pages.	 It even checks to make sure its nonzero.
}

void setupEfiTables64(void)
{
	struct fake_efi_pages
	{
		EFI_SYSTEM_TABLE_64 efiSystemTable;
		EFI_RUNTIME_SERVICES_64 efiRuntimeServices;
		EFI_CONFIGURATION_TABLE_64 efiConfigurationTable[MAX_CONFIGURATION_TABLE_ENTRIES];
		EFI_CHAR16 firmwareVendor[sizeof(FIRMWARE_VENDOR)/sizeof(EFI_CHAR16)];
		uint8_t voidret_instructions[sizeof(VOIDRET_INSTRUCTIONS)/sizeof(uint8_t)];
		uint8_t unsupportedret_instructions[sizeof(UNSUPPORTEDRET_INSTRUCTIONS_64)/sizeof(uint8_t)];
	};

	struct fake_efi_pages *fakeEfiPages = (struct fake_efi_pages *)AllocateKernelMemory(sizeof(struct fake_efi_pages));

	// Zero out all the tables in case fields are added later
	//bzero(fakeEfiPages, sizeof(struct fake_efi_pages));

	// --------------------------------------------------------------------
	// Initialize some machine code that will return EFI_UNSUPPORTED for
	// functions returning int and simply return for void functions.
	memcpy(fakeEfiPages->voidret_instructions, VOIDRET_INSTRUCTIONS, sizeof(VOIDRET_INSTRUCTIONS));
	memcpy(fakeEfiPages->unsupportedret_instructions, UNSUPPORTEDRET_INSTRUCTIONS_64, sizeof(UNSUPPORTEDRET_INSTRUCTIONS_64));

	// --------------------------------------------------------------------
	// System table
	EFI_SYSTEM_TABLE_64 *efiSystemTable = gST64 = &fakeEfiPages->efiSystemTable;
	efiSystemTable->Hdr.Signature = EFI_SYSTEM_TABLE_SIGNATURE;
	efiSystemTable->Hdr.Revision = EFI_SYSTEM_TABLE_REVISION;
	efiSystemTable->Hdr.HeaderSize = sizeof(EFI_SYSTEM_TABLE_64);
	efiSystemTable->Hdr.CRC32 = 0; // Initialize to zero and then do CRC32
	efiSystemTable->Hdr.Reserved = 0;

	efiSystemTable->FirmwareVendor = ptov64((EFI_PTR32)&fakeEfiPages->firmwareVendor);
	memcpy(fakeEfiPages->firmwareVendor, FIRMWARE_VENDOR, sizeof(FIRMWARE_VENDOR));
	efiSystemTable->FirmwareRevision = FIRMWARE_REVISION;

	// XXX: We may need to have basic implementations of ConIn/ConOut/StdErr
	// The EFI spec states that all handles are invalid after boot services have been
	// exited so we can probably get by with leaving the handles as zero.
	efiSystemTable->ConsoleInHandle = 0;
	efiSystemTable->ConIn = 0;

	efiSystemTable->ConsoleOutHandle = 0;
	efiSystemTable->ConOut = 0;

	efiSystemTable->StandardErrorHandle = 0;
	efiSystemTable->StdErr = 0;

	efiSystemTable->RuntimeServices = ptov64((EFI_PTR32)&fakeEfiPages->efiRuntimeServices);
	// According to the EFI spec, BootServices aren't valid after the
	// boot process is exited so we can probably do without it.
	// Apple didn't provide a definition for it in pexpert/i386/efi.h
	// so I'm guessing they don't use it.
	efiSystemTable->BootServices = 0;

	efiSystemTable->NumberOfTableEntries = 0;
	efiSystemTable->ConfigurationTable = ptov64((EFI_PTR32)fakeEfiPages->efiConfigurationTable);

	// We're done.	Now CRC32 the thing so the kernel will accept it
	gST64->Hdr.CRC32 = crc32(0L, gST64, gST64->Hdr.HeaderSize);

	// --------------------------------------------------------------------
	// Runtime services
	EFI_RUNTIME_SERVICES_64 *efiRuntimeServices = &fakeEfiPages->efiRuntimeServices;
	efiRuntimeServices->Hdr.Signature = EFI_RUNTIME_SERVICES_SIGNATURE;
	efiRuntimeServices->Hdr.Revision = EFI_RUNTIME_SERVICES_REVISION;
	efiRuntimeServices->Hdr.HeaderSize = sizeof(EFI_RUNTIME_SERVICES_64);
	efiRuntimeServices->Hdr.CRC32 = 0;
	efiRuntimeServices->Hdr.Reserved = 0;

	// There are a number of function pointers in the efiRuntimeServices table.
	// These are the Foundation (e.g. core) services and are expected to be present on
	// all EFI-compliant machines.	Some kernel extensions (notably AppleEFIRuntime)
	// will call these without checking to see if they are null.
	//
	// We don't really feel like doing an EFI implementation in the bootloader
	// but it is nice if we can at least prevent a complete crash by
	// at least providing some sort of implementation until one can be provided
	// nicely in a kext.

	void (*voidret_fp)() = (void *)fakeEfiPages->voidret_instructions;
	void (*unsupportedret_fp)() = (void *)fakeEfiPages->unsupportedret_instructions;
	efiRuntimeServices->GetTime = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->SetTime = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->GetWakeupTime = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->SetWakeupTime = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->SetVirtualAddressMap = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->ConvertPointer = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->GetVariable = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->GetNextVariableName = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->SetVariable = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->GetNextHighMonotonicCount = ptov64((EFI_PTR32)unsupportedret_fp);
	efiRuntimeServices->ResetSystem = ptov64((EFI_PTR32)voidret_fp);

	// We're done.	Now CRC32 the thing so the kernel will accept it
	efiRuntimeServices->Hdr.CRC32 = crc32(0L, efiRuntimeServices, efiRuntimeServices->Hdr.HeaderSize);

	// --------------------------------------------------------------------
	// Finish filling in the rest of the boot args that we need.
	bootArgs->efiSystemTable = (uint32_t)efiSystemTable;
	bootArgs->efiMode = kBootArgsEfiMode64;

	// The bootArgs structure as a whole is bzero'd so we don't need to fill in
	// things like efiRuntimeServices* and what not.
	//
	// In fact, the only code that seems to use that is the hibernate code so it
	// knows not to save the pages.	 It even checks to make sure its nonzero.
}

/*
 * In addition to the EFI tables there is also the EFI device tree node.
 * In particular, we need /efi/platform to have an FSBFrequency key. Without it,
 * the tsc_init function will panic very early on in kernel startup, before
 * the console is available.
 */

/*==========================================================================
 * FSB Frequency detection
 */

/* These should be const but DT__AddProperty takes char* */
static const char TSC_Frequency_prop[] = "TSCFrequency";
static const char FSB_Frequency_prop[] = "FSBFrequency";
static const char CPU_Frequency_prop[] = "CPUFrequency";

/*==========================================================================
 * SMBIOS
 */

/* From Foundation/Efi/Guid/Smbios/SmBios.c */
EFI_GUID const  gEfiSmbiosTableGuid = EFI_SMBIOS_TABLE_GUID;

#define SMBIOS_RANGE_START		0x000F0000
#define SMBIOS_RANGE_END		0x000FFFFF

/* '_SM_' in little endian: */
#define SMBIOS_ANCHOR_UINT32_LE 0x5f4d535f

EFI_GUID gEfiAcpiTableGuid = EFI_ACPI_TABLE_GUID;
EFI_GUID gEfiAcpi20TableGuid = EFI_ACPI_20_TABLE_GUID;


/*==========================================================================
 * Fake EFI implementation
 */

/* These should be const but DT__AddProperty takes char* */
static const char FIRMWARE_REVISION_PROP[] = "firmware-revision";
static const char FIRMWARE_ABI_PROP[] = "firmware-abi";
static const char FIRMWARE_VENDOR_PROP[] = "firmware-vendor";
static const char FIRMWARE_ABI_32_PROP_VALUE[] = "EFI32";
static const char FIRMWARE_ABI_64_PROP_VALUE[] = "EFI64";
static const char EFI_MODE_PROP[] = "efi-mode";  //Bungo
static const char SYSTEM_ID_PROP[] = "system-id";
static const char SYSTEM_SERIAL_PROP[] = "SystemSerialNumber";
static const char SYSTEM_TYPE_PROP[] = "system-type";
static const char MODEL_PROP[] = "Model";
static const char BOARDID_PROP[] = "board-id";
static const char DEV_PATH_SUP[] = "DevicePathsSupported";
static const char START_POWER_EV[] = "StartupPowerEvents";
static const char MACHINE_SIG_PROP[] = "machine-signature";
static EFI_UINT8 const DEVICE_PATHS_SUPPORTED[] = { 0x01, 0x00, 0x00, 0x00 };
static EFI_UINT8 const STARTUP_POWER_EVENTS[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static EFI_UINT8 const COMPAT_MODE[] = { 0x01, 0x00, 0x00, 0x00 };

// Pike R. Alpha
static EFI_UINT8 const BOOT_DEVICE_PATH[] =
{
    0x02, 0x01, 0x0C, 0x00, 0xD0, 0x41, 0x08, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x06, 0x00,
    0x02, 0x1F, 0x03, 0x12, 0x0A, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x2A, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x28, 0x40, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x0B, 0x63, 0x34,
    0x00, 0x00, 0x00, 0x00, 0x65, 0x8C, 0x53, 0x3F, 0x1B, 0xCA, 0x83, 0x38, 0xA9, 0xD0, 0xF0, 0x46,
    0x19, 0x14, 0x8E, 0x31, 0x02, 0x02, 0x7F, 0xFF, 0x04, 0x00
};
// Pike R. Alpha
static EFI_UINT8 const BOOT_FILE_PATH[] =
{
    0x04, 0x04, 0x50, 0x00, 0x5c, 0x00, 0x53, 0x00, 0x79, 0x00, 0x73, 0x00, 0x74, 0x00, 0x65, 0x00,
    0x6d, 0x00, 0x5c, 0x00, 0x4c, 0x00, 0x69, 0x00, 0x62, 0x00, 0x72, 0x00, 0x61, 0x00, 0x72, 0x00,
    0x79, 0x00, 0x5c, 0x00, 0x43, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x65, 0x00, 0x53, 0x00, 0x65, 0x00,
    0x72, 0x00, 0x76, 0x00, 0x69, 0x00, 0x63, 0x00, 0x65, 0x00, 0x73, 0x00, 0x5c, 0x00, 0x62, 0x00,
    0x6f, 0x00, 0x6f, 0x00, 0x74, 0x00, 0x2e, 0x00, 0x65, 0x00, 0x66, 0x00, 0x69, 0x00, 0x00, 0x00,
    0x7f, 0xff, 0x04, 0x00
};

/*
 * Get an smbios option string option to convert to EFI_CHAR16 string
 */
static EFI_CHAR16 *getSmbiosChar16(const char *key, size_t *len)
{
	const char	*src = getStringForKey(key, &bootInfo->smbiosConfig);
	EFI_CHAR16	*dst = 0;
	size_t		 i = 0;

	if (!key || !(*key) || !len || !src)
	{
		return 0;
	}
	
	*len = strlen(src);
	dst = (EFI_CHAR16 *) malloc( ((*len)+1) * 2 );
	for (; i < (*len); i++)
	{
		dst[i] = src[i];
	}
	dst[(*len)] = '\0';
	*len = ((*len)+1)*2; // return the CHAR16 bufsize including zero terminated CHAR16
	return dst;
}

/*
 * Must be called AFTER setupAcpi because we need to take care of correct
 * FACP content to reflect in ioregs
 */
void setupSystemType()
{
	Node *node = DT__FindNode("/", false);
	if (node == 0)
	{
		stop("Couldn't get root '/' node");
	}
	// we need to write this property after facp parsing
	// Export system-type only if it has been overrriden by the SystemType option
	DT__AddProperty(node, SYSTEM_TYPE_PROP, sizeof(Platform.Type), &Platform.Type);
}

static void setupEfiDeviceTree(void)
{
	// EFI_CHAR8	*ret = 0;  Bungo: not used
	EFI_CHAR16	*ret16 = 0;
	size_t		 len = 0;
	Node		*node;

	node = DT__FindNode("/", false);

	if (node == 0)
	{
		stop("Couldn't get root node");
	}

	// We could also just do DT__FindNode("/efi/platform", true)
	// But I think eventually we want to fill stuff in the efi node
	// too so we might as well create it so we have a pointer for it too.
	node = DT__AddChild(node, "efi");

	if (archCpuType == CPU_TYPE_I386)
	{
		DT__AddProperty(node, FIRMWARE_ABI_PROP, sizeof(FIRMWARE_ABI_32_PROP_VALUE), (char *)FIRMWARE_ABI_32_PROP_VALUE);
	}
	else
	{
		DT__AddProperty(node, FIRMWARE_ABI_PROP, sizeof(FIRMWARE_ABI_64_PROP_VALUE), (char *)FIRMWARE_ABI_64_PROP_VALUE);
	}

	DT__AddProperty(node, EFI_MODE_PROP, sizeof(EFI_UINT8), (EFI_UINT8 *)&bootArgs->efiMode);

	DT__AddProperty(node, FIRMWARE_REVISION_PROP, sizeof(FIRMWARE_REVISION), (EFI_UINT32 *)&FIRMWARE_REVISION);
	DT__AddProperty(node, FIRMWARE_VENDOR_PROP, sizeof(FIRMWARE_VENDOR), (EFI_CHAR16 *)FIRMWARE_VENDOR);

	// TODO: Fill in other efi properties if necessary

	// Set up the /efi/runtime-services table node similar to the way a child node of configuration-table
	// is set up.  That is, name and table properties
	Node *runtimeServicesNode = DT__AddChild(node, "runtime-services");

	if (archCpuType == CPU_TYPE_I386)
	{
		// The value of the table property is the 32-bit physical address for the RuntimeServices table.
		// Since the EFI system table already has a pointer to it, we simply use the address of that pointer
		// for the pointer to the property data.  Warning.. DT finalization calls free on that but we're not
		// the only thing to use a non-malloc'd pointer for something in the DT

		DT__AddProperty(runtimeServicesNode, "table", sizeof(uint64_t), &gST32->RuntimeServices);
	}
	else
	{
		DT__AddProperty(runtimeServicesNode, "table", sizeof(uint64_t), &gST64->RuntimeServices);
	}

	// Set up the /efi/configuration-table node which will eventually have several child nodes for
	// all of the configuration tables needed by various kernel extensions.
	gEfiConfigurationTableNode = DT__AddChild(node, "configuration-table");

	// New node: /efi/kernel-compatibility
	Node *efiKernelComNode = DT__AddChild(node, "kernel-compatibility");

	if (MacOSVerCurrent >= MacOSVer2Int("10.9"))
	{
		DT__AddProperty(efiKernelComNode, "x86_64", sizeof(COMPAT_MODE), (EFI_UINT8 *) &COMPAT_MODE);
	}
	else
	{
		DT__AddProperty(efiKernelComNode, "i386", sizeof(COMPAT_MODE), (EFI_UINT8 *) &COMPAT_MODE);
		DT__AddProperty(efiKernelComNode, "x86_64", sizeof(COMPAT_MODE), (EFI_UINT8 *) &COMPAT_MODE);
	}

	// Now fill in the /efi/platform Node
	Node *efiPlatformNode = DT__AddChild(node, "platform"); // "/efi/platform"

	// NOTE WELL: If you do add FSB Frequency detection, make sure to store
	// the value in the fsbFrequency global and not an malloc'd pointer
	// because the DT_AddProperty function does not copy its args.

	if (Platform.CPU.FSBFrequency != 0)
	{
		DT__AddProperty(efiPlatformNode, FSB_Frequency_prop, sizeof(uint64_t), &Platform.CPU.FSBFrequency);
	}

	// Export TSC and CPU frequencies for use by the kernel or KEXTs
	if (Platform.CPU.TSCFrequency != 0)
	{
		DT__AddProperty(efiPlatformNode, TSC_Frequency_prop, sizeof(uint64_t), &Platform.CPU.TSCFrequency);
	}

	if (Platform.CPU.CPUFrequency != 0)
	{
		DT__AddProperty(efiPlatformNode, CPU_Frequency_prop, sizeof(uint64_t), &Platform.CPU.CPUFrequency);
	}

	DT__AddProperty(efiPlatformNode, START_POWER_EV, sizeof(STARTUP_POWER_EVENTS), (EFI_UINT8 *) &STARTUP_POWER_EVENTS);

	DT__AddProperty(efiPlatformNode, DEV_PATH_SUP, sizeof(DEVICE_PATHS_SUPPORTED), (EFI_UINT8 *) &DEVICE_PATHS_SUPPORTED);

	DT__AddProperty(efiPlatformNode, SYSTEM_ID_PROP, UUID_LEN, (EFI_UINT32 *)Platform.UUID);

	// Export SystemSerialNumber if present
	if ((ret16=getSmbiosChar16("SMserial", &len)))
	{
		DT__AddProperty(efiPlatformNode, SYSTEM_SERIAL_PROP, len, ret16);
	}

	// Export Model if present
	if ((ret16=getSmbiosChar16("SMproductname", &len)))
	{
		DT__AddProperty(efiPlatformNode, MODEL_PROP, len, ret16);
	}

	// Fill /efi/device-properties node.
	setupDeviceProperties(node);
}

/*
 * Must be called AFTER getSmbios
 */
void setupBoardId()
{
	Node *node;
	node = DT__FindNode("/", false);
	if (node == 0)
	{
		stop("Couldn't get root '/' node");
	}
	const char *boardid = getStringForKey("SMboardproduct", &bootInfo->smbiosConfig); // SMboardserial
	if (boardid)
	{
		DT__AddProperty(node, BOARDID_PROP, strlen(boardid)+1, (EFI_CHAR16 *)boardid);
	}
}

/*
 * Populate the chosen node
 */
void setupChosenNode()
{
	Node *chosenNode;
	chosenNode = DT__FindNode("/chosen", false);
	unsigned long adler32 = 0;

	if (chosenNode == NULL)
	{
		stop("setupChosenNode: Couldn't get '/chosen' node");
	}

	// Only accept a UUID with the correct length.
	if (strlen(gBootUUIDString) == 36)
	{
		DT__AddProperty(chosenNode, "boot-uuid", 37, gBootUUIDString);
	}

	DT__AddProperty(chosenNode, "boot-args", sizeof(bootArgs->CommandLine), (EFI_UINT8 *)bootArgs->CommandLine);

	// Adding the default kernel name (mach_kernel) for kextcache.
	DT__AddProperty(chosenNode, "boot-file", sizeof(bootInfo->bootFile), bootInfo->bootFile);

	DT__AddProperty(chosenNode, "boot-file-path", sizeof(BOOT_FILE_PATH), (EFI_UINT8 *) &BOOT_FILE_PATH);

	// Adding the root path for kextcache.
	DT__AddProperty(chosenNode, "boot-device-path", sizeof(BOOT_DEVICE_PATH), (EFI_UINT8 *) &BOOT_DEVICE_PATH);

	DT__AddProperty(chosenNode, "boot-kernelcache-adler32", sizeof(unsigned long), &adler32);

	DT__AddProperty(chosenNode, MACHINE_SIG_PROP, sizeof(Platform.HWSignature), (EFI_UINT32 *)&Platform.HWSignature);

	if ( MacOSVerCurrent >= MacOSVer2Int("10.10") ) // Yosemite+
	{
		//
		// Pike R. Alpha - 12 October 2014
		//
		UInt8 index = 0;
		EFI_UINT16 PMTimerValue = 0, PMRepeatCount = 0xffff;

#if RANDOMSEED
		EFI_UINT32 randomValue = 0, cpuTick = 0;
		EFI_UINT32 ecx = 0, edx = 0, esi = 0, edi = 0;
#else
		EFI_UINT32 randomValue, tempValue, cpuTick;
		EFI_UINT32 ecx, esi, edi = 0;
		EFI_UINT64 rcx, rdx, rsi, rdi;

		randomValue = tempValue = ecx = esi = edi = 0;					// xor		%ecx,	%ecx
		cpuTick = rcx = rdx = rsi = rdi = 0;
#endif
		// LEAF_1 - Feature Information (Function 01h).
		if (Platform.CPU.CPUID[CPUID_1][2] & 0x40000000)				// Checking ecx:bit-30
		{
			//
			// i5/i7 Ivy Bridge and Haswell processors with RDRAND support.
			//
			EFI_UINT32 seedBuffer[16] = {0};
			//
			// Main loop to get 16 dwords (four bytes each).
			//
			for (index = 0; index < 16; index++)					// 0x17e12:
			{
				randomValue = computeRand();					// callq	0x18e20
				cpuTick = (EFI_UINT32) getCPUTick();				// callq	0x121a7
				randomValue = (randomValue ^ cpuTick);				// xor		%rdi,	%rax
				seedBuffer[index] = randomValue;				// mov		%rax,(%r15,%rsi,8)
			}									// jb		0x17e12

			DT__AddProperty(chosenNode, "random-seed", sizeof(seedBuffer), (EFI_UINT32 *) &seedBuffer);
		}
		else
		{
			//
			// All other processors without RDRAND support.
			//
			EFI_UINT8 seedBuffer[64] = {0};
			//
			// Main loop to get the 64 bytes.
			//
			do									// 0x17e55:
			{
				//
				// FIXME: PM Timer is usually @ 0x408, but its position is relocatable
				//   via PCI-to-ISA bridge.  The location is reported in ACPI FADT,
				//   PM Timer Block address - zenith432
				//
				PMTimerValue = inw(0x408);					// in		(%dx),	%ax
				esi = PMTimerValue;						// movzwl	%ax,	%esi

				if (esi < ecx)							// cmp		%ecx,	%esi
				{
					/*
					 * This is a workaround to prevent an infinite loop
					 *   if PMTimer is not at port 0x408 - zenith432
					 */
					if (PMRepeatCount)
					{
						--PMRepeatCount;
						continue;						// jb		0x17e55		(retry)
					}
				}
				else
				{
					PMRepeatCount = 0xffff;
				}

				cpuTick = (EFI_UINT32) getCPUTick();				// callq	0x121a7
//				printf("value: 0x%x\n", getCPUTick());

#if RANDOMSEED
				ecx = (cpuTick >> 8);						// mov		%rax,	%rcx
				// shr		$0x8,	%rcx
				edx = (cpuTick >> 0x10);					// mov		%rax,	%rdx
				// shr		$0x10,	%rdx
				edi = esi;							// mov		%rsi,	%rdi
				edi = (edi ^ cpuTick);						// xor		%rax,	%rdi
				edi = (edi ^ ecx);						// xor		%rcx,	%rdi
				edi = (edi ^ edx);						// xor		%rdx,	%rdi

				seedBuffer[index] = (edi & 0xff);
#else
				rcx = (cpuTick >> 8);						// mov		%rax,	%rcx
				// shr		$0x8,	%rcx
				rdx = (cpuTick >> 0x10);					// mov		%rax,	%rdx
				// shr		$0x10,	%rdx
				/*
				 * Note: In x86 assembly, rXX is upper part of eXX register.
				 *   In C they're different variables.
				 *   The code is identical with or without RANDOMSEED. - zenith432
				 */
				rdi = rsi = esi;						// mov		%rsi,	%rdi
				rdi = (rdi ^ cpuTick);						// xor		%rax,	%rdi
				rdi = (rdi ^ rcx);						// xor		%rcx,	%rdi
				rdi = (rdi ^ rdx);						// xor		%rdx,	%rdi
				edi = (EFI_UINT32) rdi;

				seedBuffer[index] = (rdi & 0xff);				// mov		%dil,	(%r15,%r12,1)
#endif
				edi = (edi & 0x2f);						// and		$0x2f,	%edi
				edi = (edi + esi);						// add		%esi,	%edi
				index++;							// inc		r12
				ecx = (edi & 0xffff);						// movzwl	%di,	%ecx

			} while (index < 64);							// cmp		%r14d,	%r12d
			// jne		0x17e55		(next)

			DT__AddProperty(chosenNode, "random-seed", sizeof(seedBuffer), (EFI_UINT8 *) &seedBuffer);

		}
	}

	// Micky1979 : MIMIC booter entry for El Capitan
	if ( MacOSVerCurrent >= MacOSVer2Int("10.11") ) // El Capitan
	{

		verbose("Adding booter spec to the Platform Expert \n");

		// booter-build-time (Fri Apr 14 16:21:16 PDT 2017) 10.12.5
		DT__AddProperty(chosenNode, "booter-build-time", sizeof(I386BOOT_BUILDDATE), I386BOOT_BUILDDATE);

		// booter-name
		DT__AddProperty(chosenNode, "booter-name", sizeof("Chameleon"), "Chameleon");

		// booter-version (version:324.50.13) 10.12.5
		DT__AddProperty(chosenNode, "booter-version", sizeof(I386BOOT_CHAMELEONREVISION), I386BOOT_CHAMELEONREVISION);
	}
}

/*
 * Load the smbios.plist override config file if any
 */
static void setupSmbiosConfigFile(const char *filename)
{
	char		dirSpecSMBIOS[128];
	const char	*override_pathname = NULL;
	int		len = 0, err = 0;
	extern void scan_mem();

	// Take in account user overriding
	if (getValueForKey(kSMBIOSKey, &override_pathname, &len, &bootInfo->chameleonConfig) && len > 0)
	{
		// Specify a path to a file, e.g. SMBIOS=/Extra/macProXY.plist
		strcpy(dirSpecSMBIOS, override_pathname);
		err = loadConfigFile(dirSpecSMBIOS, &bootInfo->smbiosConfig);
	}
	else
	{
		// Check selected volume's Extra.
		sprintf(dirSpecSMBIOS, "/Extra/%s", filename);
		err = loadConfigFile(dirSpecSMBIOS, &bootInfo->smbiosConfig);
	}

	if (err)
	{
		verbose("No SMBIOS replacement found.\n");
	}

	// get a chance to scan mem dynamically if user asks for it while having the config options
	// loaded as well, as opposed to when it was in scan_platform(); also load the orig. smbios
	// so that we can access dmi info, without patching the smbios yet.
	scan_mem(); 
}

/*
 * Installs all the needed configuration table entries
 */
static void setupEfiConfigurationTable()
{
	smbios_p = (EFI_PTR32)getSmbios(SMBIOS_PATCHED);
	addConfigurationTable(&gEfiSmbiosTableGuid, &smbios_p, NULL);

	setupBoardId(); //need to be called after getSmbios

	// Setup ACPI with DSDT overrides (mackerintel's patch)
	setupAcpi();

	// We've obviously changed the count.. so fix up the CRC32
	if (archCpuType == CPU_TYPE_I386)
	{
		gST32->Hdr.CRC32 = 0;
		gST32->Hdr.CRC32 = crc32(0L, gST32, gST32->Hdr.HeaderSize);
	}
	else
	{
		gST64->Hdr.CRC32 = 0;
		gST64->Hdr.CRC32 = crc32(0L, gST64, gST64->Hdr.HeaderSize);
	}

	// Setup the chosen node
	setupChosenNode();
}

void saveOriginalSMBIOS(void)
{
	Node *node;
	SMBEntryPoint *origeps;
	void *tableAddress;

	node = DT__FindNode("/efi/platform", false);
	if (!node)
	{
		DBG("saveOriginalSMBIOS: '/efi/platform' node not found\n");
		return;
	}

	origeps = getSmbios(SMBIOS_ORIGINAL);
	if (!origeps)
	{
		DBG("saveOriginalSMBIOS: original SMBIOS not found\n");
		return;
	}

	tableAddress = (void *)AllocateKernelMemory(origeps->dmi.tableLength);
	if (!tableAddress)
	{
		DBG("saveOriginalSMBIOS: can not allocate memory for original SMBIOS\n");
		return;
	}

	memcpy(tableAddress, (void *)origeps->dmi.tableAddress, origeps->dmi.tableLength);
	DT__AddProperty(node, "SMBIOS", origeps->dmi.tableLength, tableAddress);
}

/*
 * Entrypoint from boot.c
 */
void setupFakeEfi(void)
{
	// Generate efi device strings
	setup_pci_devs(root_pci_dev);

	readSMBIOSInfo(getSmbios(SMBIOS_ORIGINAL));

	// load smbios.plist file if any
	setupSmbiosConfigFile("smbios.plist");

	setupSMBIOSTable();

	// Initialize the base table
	if (archCpuType == CPU_TYPE_I386)
	{
		setupEfiTables32();
	}
	else
	{
		setupEfiTables64();
	}

	// Initialize the device tree
	setupEfiDeviceTree();

	saveOriginalSMBIOS();

	// Add configuration table entries to both the services table and the device tree
	setupEfiConfigurationTable();
}
