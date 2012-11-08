
/*
 * Copyright 2007 David F. Elliott.	 All rights reserved.
 */

/*
 * Copyright 2010,2011,2012 Cadet-petit Armel <armelcadetpetit@gmail.com>. All rights reserved.
 */

#include "libsaio.h"
#include "bootstruct.h"
#include "efi.h"
#include "acpi.h"
#include "fake_efi.h"
#include "efi_tables.h"
#include "platform.h"
#include "device_inject.h"
#include "convert.h"
#include "pci.h"
#include "sl.h"
#include "modules.h"
#include "vers.h"

#ifndef NO_SMP_SUPPORT
#include "smp-imps.h"
#endif

#ifndef DEBUG_EFI
#define DEBUG_EFI 0
#endif

#if DEBUG_EFI
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif
/*
 * Modern Darwin kernels require some amount of EFI because Apple machines all
 * have EFI.  Modifying the kernel source to not require EFI is of course
 * possible but would have to be maintained as a separate patch because it is
 * unlikely that Apple wishes to add legacy support to their kernel.
 *
 * As you can see from the Apple-supplied code in bootstruct.c, it seems that
 * the intention was clearly to modify this booter to provide EFI-like structures
 * to the kernel rather than modifying the kernel to handle non-EFI stuff.	 This
 * makes a lot of sense from an engineering point of view as it means the kernel
 * for the as yet unreleased EFI-only Macs could still be booted by the non-EFI
 * DTK systems so long as the kernel checked to ensure the boot tables were
 * filled in appropriately.	 Modern xnu requires a system table and a runtime
 * services table and performs no checks whatsoever to ensure the pointers to
 * these tables are non-NULL.	Therefore, any modern xnu kernel will page fault
 * early on in the boot process if the system table pointer is zero.
 *
 * Even before that happens, the tsc_init function in modern xnu requires the FSB
 * Frequency to be a property in the /efi/platform node of the device tree or else
 * it panics the bootstrap process very early on.
 *
 * As of this writing, the current implementation found here is good enough
 * to make the currently available xnu kernel boot without modification on a
 * system with an appropriate processor.  With a minor source modification to
 * the tsc_init function to remove the explicit check for Core or Core 2
 * processors the kernel can be made to boot on other processors so long as
 * the code can be executed by the processor and the machine contains the
 * necessary hardware.
 */
static inline char * mallocStringForGuid(EFI_GUID const *pGuid);
static VOID EFI_ST_FIX_CRC32(VOID);
static EFI_STATUS setupAcpiNoMod(VOID);
static EFI_CHAR16* getSmbiosChar16(const char * key, size_t* len);
static EFI_CHAR8* getSmbiosUUID(VOID);
static int8_t *getSystemID(VOID);
static VOID setupSystemType(VOID);
static VOID setupEfiDeviceTree(VOID);
static VOID setup_Smbios(VOID);
static VOID setup_machine_signature(VOID);
static VOID setupEfiConfigurationTable(VOID);
static EFI_STATUS EFI_FindAcpiTables(VOID);

/*==========================================================================
 * Utility function to make a device tree string from an EFI_GUID
 */

static inline char * mallocStringForGuid(EFI_GUID const *pGuid)
{
	char *string = malloc(37);
    if (!string) {
#if DEBUG_EFI
        char string_d[37];
        efi_guid_unparse_upper(pGuid, string_d, sizeof(string_d));
        printf("Couldn't allocate Guid String for %s\n", string_d);        
#endif
        return NULL;
    }
    
	efi_guid_unparse_upper(pGuid, string, 37);
	return string;
}

/*==========================================================================
 * Function to map 32 bit physical address to 64 bit virtual address
 */

#define ptov64(addr) (uint64_t)((uint64_t)addr | 0xFFFFFF8000000000ULL)

/*==========================================================================
 * Fake EFI implementation
 */

static EFI_CHAR16 const FIRMWARE_VENDOR[] = {'A','p','p','l','e', 0}; 

/* Info About the current Firmware */
#define FIRMWARE_MAINTENER "cparm, armelcadetpetit@gmail.com" 
static EFI_CHAR16 const FIRMWARE_NAME[] = {'M','a','s','h','e','r','b','r','u','m','-','2', 0};  
static EFI_UINT32 const FIRMWARE_REVISION = 0x00010800; //1.8
static EFI_UINT32 const DEVICE_SUPPORTED = 0x00000001;

/* Default platform system_id (fix by IntVar) */
static EFI_CHAR8 const SYSTEM_ID[] = "0123456789ABCDEF"; //random value gen by uuidgen

/* Just a ret instruction */
static uint8_t const VOIDRET_INSTRUCTIONS[] = {0xc3};

/* movl $0x80000003,%eax; ret */
static uint8_t const UNSUPPORTEDRET_INSTRUCTIONS[] = {0xb8, 0x03, 0x00, 0x00, 0x80, 0xc3};

EFI_SYSTEM_TABLE_32 *gST32 = NULL;
EFI_SYSTEM_TABLE_64 *gST64 = NULL;
Node *gEfiConfigurationTableNode = NULL;

/* From Foundation/Efi/Guid/Smbios/SmBios.h */
/* Modified to wrap Data4 array init with {} */
#define EFI_SMBIOS_TABLE_GUID {0xeb9d2d31, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d}}

#define EFI_ACPI_TABLE_GUID \
{ \
0xeb9d2d30, 0x2d88, 0x11d3, { 0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d } \
}

#define EFI_ACPI_20_TABLE_GUID \
{ \
0x8868e871, 0xe4f1, 0x11d3, { 0xbc, 0x22, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 } \
}

#ifndef NO_SMP_SUPPORT
#define EFI_MPS_TABLE_GUID \
{ \
0xeb9d2d2f,0x2d88,0x11d3,{0x9a,0x16,0x0,0x90,0x27,0x3f,0xc1,0x4d} \
}
#endif

/* From Foundation/Efi/Guid/Smbios/SmBios.c */
EFI_GUID const	gEfiSmbiosTableGuid = EFI_SMBIOS_TABLE_GUID;

EFI_GUID gEfiAcpiTableGuid = EFI_ACPI_TABLE_GUID;
EFI_GUID gEfiAcpi20TableGuid = EFI_ACPI_20_TABLE_GUID;

#ifndef NO_SMP_SUPPORT
EFI_GUID gEfiMpsTableGuid = EFI_MPS_TABLE_GUID;
#endif

EFI_UINT32                    gNumTables32 = 0;
EFI_UINT64                    gNumTables64 = 0;
EFI_CONFIGURATION_TABLE_32 gEfiConfigurationTable32[MAX_CONFIGURATION_TABLE_ENTRIES];
EFI_CONFIGURATION_TABLE_64 gEfiConfigurationTable64[MAX_CONFIGURATION_TABLE_ENTRIES];
EFI_STATUS addConfigurationTable(EFI_GUID const *pGuid, void *table, char const *alias)
{
	EFI_UINTN i = 0;
	
    if (pGuid == NULL || table == NULL)
		return EFI_INVALID_PARAMETER;
    
    char * GuidStr = mallocStringForGuid(pGuid);
    if (!GuidStr) {
        return EFI_OUT_OF_RESOURCES;
    }
    
	//Azi: as is, cpu's with em64t will use EFI64 on pre 10.6 systems,
	// wich seems to cause no problem. In case it does, force i386 arch.
	if (get_env(envarchCpuType) == CPU_TYPE_I386)
	{
		i = gNumTables32;
	}
	else
	{
		i = (EFI_UINTN)gNumTables64;
	}
	
	// We only do adds, not modifications and deletes like InstallConfigurationTable
	if (i >= MAX_CONFIGURATION_TABLE_ENTRIES)
	{
        
		
        printf("Ran out of space for configuration tables (max = %d). Please, increase the reserved size in the code.\n", (int)MAX_CONFIGURATION_TABLE_ENTRIES);
        return EFI_ABORTED;
    }    
    
    if (get_env(envarchCpuType) == CPU_TYPE_I386)
	{       
        
        gEfiConfigurationTable32[i].VendorGuid = *pGuid;
        gEfiConfigurationTable32[i].VendorTable = (EFI_PTR32)table;
        
		gNumTables32++;
	}
	else
	{        
        gEfiConfigurationTable64[i].VendorGuid = *pGuid;
        gEfiConfigurationTable64[i].VendorTable = (EFI_PTR32)table;
		gNumTables64++ ;
	}    
	
    
    Node *tableNode = DT__AddChild(gEfiConfigurationTableNode, GuidStr);
    
    // Use the pointer to the GUID we just stuffed into the system table
    DT__AddProperty(tableNode, "guid", sizeof(EFI_GUID), (void*)pGuid);
    
    // The "table" property is the 32-bit (in our implementation) physical address of the table
    DT__AddProperty(tableNode, "table", sizeof(void*) * 2, table);
    
    // Assume the alias pointer is a global or static piece of data
    if (alias != NULL)
        DT__AddProperty(tableNode, "alias", strlen(alias)+1, (char*)alias);
    
    return EFI_SUCCESS;
	
}

static VOID EFI_ST_FIX_CRC32(void)
{
	if (get_env(envarchCpuType) == CPU_TYPE_I386)
	{
		gST32->Hdr.CRC32 = 0;
		gST32->Hdr.CRC32 = crc32(0L, gST32, gST32->Hdr.HeaderSize);
	}
	else
	{
		gST64->Hdr.CRC32 = 0;
		gST64->Hdr.CRC32 = crc32(0L, gST64, gST64->Hdr.HeaderSize);
	}
}

void finalizeEFIConfigTable(void )
{    
    if (get_env(envarchCpuType) == CPU_TYPE_I386)
	{
		EFI_SYSTEM_TABLE_32 *efiSystemTable = gST32;        
        
        efiSystemTable->NumberOfTableEntries = gNumTables32; 
        efiSystemTable->ConfigurationTable = (EFI_PTR32)gEfiConfigurationTable32;
        
	}
	else
	{
		EFI_SYSTEM_TABLE_64 *efiSystemTable = gST64;        
        
        efiSystemTable->NumberOfTableEntries = gNumTables64; 
        efiSystemTable->ConfigurationTable = ptov64((EFI_PTR32)gEfiConfigurationTable64);        
        
	}
    EFI_ST_FIX_CRC32();
    
#if DEBUG_EFI
    EFI_UINTN i;
    EFI_UINTN num = 0;
    uint32_t table ;
    EFI_GUID Guid;
    
    if (get_env(envarchCpuType) == CPU_TYPE_I386)
	{
		num = gST32->NumberOfTableEntries;        
        
	}
	else
	{
		num = (EFI_UINTN)gST64->NumberOfTableEntries;        
        
	}
	msglog("EFI Configuration table :\n");
    for (i=0; i<num; i++)
	{
        if (get_env(envarchCpuType) == CPU_TYPE_I386)
        {
            table = gEfiConfigurationTable32[i].VendorTable;
            Guid =  gEfiConfigurationTable32[i].VendorGuid;
            
        }
        else
        {
            table = gEfiConfigurationTable64[i].VendorTable;
            Guid =  gEfiConfigurationTable64[i].VendorGuid;
            
        }
        char id[4+1];
        bzero(id,sizeof(id));        
        if (memcmp(&Guid, &gEfiSmbiosTableGuid, sizeof(EFI_GUID)) == 0)
		{
            snprintf(id, sizeof(id),"%s", "_SM_");
        }
		else if (memcmp(&Guid, &gEfiAcpiTableGuid, sizeof(EFI_GUID)) == 0)
		{
            snprintf(id,sizeof(id), "%s", "RSD1");
        }
		else if (memcmp(&Guid, &gEfiAcpi20TableGuid, sizeof(EFI_GUID)) == 0)
		{
            snprintf(id, sizeof(id),"%s", "RSD2");
        }
#ifndef NO_SMP_SUPPORT
		else if (memcmp(&Guid, &gEfiMpsTableGuid, sizeof(EFI_GUID)) == 0)
		{
            snprintf(id, sizeof(id),"%s", "_MP_");
        } 
#endif
		
        msglog("table [%d]:%s , 32Bit addr : 0x%x\n",i,id,table);
        
    }
    msglog("\n");
#endif
    
}

/*
 * What we do here is simply allocate a fake EFI system table and a fake EFI
 * runtime services table.
 *
 * Because we build against modern headers with kBootArgsRevision 4 we
 * also take care to set efiMode = 32.
 */


#define pto(mode, addr) (mode == 64) ? ptov64((EFI_PTR32)addr) : (EFI_PTR32)addr

#define setupEfiTables(mode)                           \
{ \
struct fake_efi_pages \
{\
/* We use the fake_efi_pages struct so that we only need to do one kernel
* memory allocation for all needed EFI data.  Otherwise, small allocations
* like the FIRMWARE_VENDOR string would take up an entire page.
* NOTE WELL: Do NOT assume this struct has any particular layout within itself.
* It is absolutely not intended to be publicly exposed anywhere
* We say pages (plural) although right now we are well within the 1 page size
* and probably will stay that way.
*/\
EFI_SYSTEM_TABLE_##mode efiSystemTable;\
EFI_RUNTIME_SERVICES_##mode efiRuntimeServices;\
EFI_CONFIGURATION_TABLE_##mode efiConfigurationTable[MAX_CONFIGURATION_TABLE_ENTRIES];\
EFI_CHAR16 firmwareVendor[sizeof(FIRMWARE_VENDOR)/sizeof(EFI_CHAR16)];\
uint8_t voidret_instructions[sizeof(VOIDRET_INSTRUCTIONS)/sizeof(uint8_t)];\
uint8_t unsupportedret_instructions[sizeof(UNSUPPORTEDRET_INSTRUCTIONS)/sizeof(uint8_t)];\
};\
struct fake_efi_pages *fakeEfiPages = (struct fake_efi_pages*)AllocateKernelMemory(sizeof(struct fake_efi_pages));\
/* Zero out all the tables in case fields are added later*/\
bzero(fakeEfiPages, sizeof(struct fake_efi_pages));\
/*--------------------------------------------------------------------
* Initialize some machine code that will return EFI_UNSUPPORTED for
* functions returning int and simply return for void functions.*/\
memcpy(fakeEfiPages->voidret_instructions, VOIDRET_INSTRUCTIONS, sizeof(VOIDRET_INSTRUCTIONS));\
memcpy(fakeEfiPages->unsupportedret_instructions, UNSUPPORTEDRET_INSTRUCTIONS, sizeof(UNSUPPORTEDRET_INSTRUCTIONS));\
/*--------------------------------------------------------------------
* System table*/\
EFI_SYSTEM_TABLE_##mode *efiSystemTable = gST##mode = &fakeEfiPages->efiSystemTable;\
efiSystemTable->Hdr.Signature = EFI_SYSTEM_TABLE_SIGNATURE;\
efiSystemTable->Hdr.Revision = EFI_SYSTEM_TABLE_REVISION;\
efiSystemTable->Hdr.HeaderSize = sizeof(EFI_SYSTEM_TABLE_##mode);\
efiSystemTable->Hdr.CRC32 = 0;/*Initialize to zero and then do CRC32*/ \
efiSystemTable->Hdr.Reserved = 0;\
efiSystemTable->FirmwareVendor = pto(mode, &fakeEfiPages->firmwareVendor);\
memcpy(fakeEfiPages->firmwareVendor, FIRMWARE_VENDOR, sizeof(FIRMWARE_VENDOR));\
efiSystemTable->FirmwareRevision = FIRMWARE_REVISION;\
/* XXX: We may need to have basic implementations of ConIn/ConOut/StdErr
* The EFI spec states that all handles are invalid after boot services have been
* exited so we can probably get by with leaving the handles as zero.
*/\
efiSystemTable->ConsoleInHandle = 0;\
efiSystemTable->ConIn = 0;\
efiSystemTable->ConsoleOutHandle = 0;\
efiSystemTable->ConOut = 0;\
efiSystemTable->StandardErrorHandle = 0;\
efiSystemTable->StdErr = 0;\
efiSystemTable->RuntimeServices = pto(mode,&fakeEfiPages->efiRuntimeServices) ;\
/* According to the EFI spec, BootServices aren't valid after the
* boot process is exited so we can probably do without it.
* Apple didn't provide a definition for it in pexpert/i386/efi.h
* so I'm guessing they don't use it.
*/\
efiSystemTable->BootServices = 0;\
efiSystemTable->NumberOfTableEntries = 0;\
efiSystemTable->ConfigurationTable = pto(mode,fakeEfiPages->efiConfigurationTable);\
/* We're done. Now CRC32 the thing so the kernel will accept it.
* Must be initialized to zero before CRC32, done above.
*/\
gST##mode->Hdr.CRC32 = crc32(0L, gST##mode, gST##mode->Hdr.HeaderSize);\
/*--------------------------------------------------------------------
* Runtime services*/\
EFI_RUNTIME_SERVICES_##mode *efiRuntimeServices = &fakeEfiPages->efiRuntimeServices;\
efiRuntimeServices->Hdr.Signature = EFI_RUNTIME_SERVICES_SIGNATURE;\
efiRuntimeServices->Hdr.Revision = EFI_RUNTIME_SERVICES_REVISION;\
efiRuntimeServices->Hdr.HeaderSize = sizeof(EFI_RUNTIME_SERVICES_##mode);\
efiRuntimeServices->Hdr.CRC32 = 0;\
efiRuntimeServices->Hdr.Reserved = 0;\
/* There are a number of function pointers in the efiRuntimeServices table.
* These are the Foundation (e.g. core) services and are expected to be present on
* all EFI-compliant machines.	Some kernel extensions (notably AppleEFIRuntime)
* will call these without checking to see if they are null.
*
* We don't really feel like doing an EFI implementation in the bootloader
* but it is nice if we can at least prevent a complete crash by
* at least providing some sort of implementation until one can be provided
* nicely in a kext.
*/\
void (*voidret_fp)() = (void*)fakeEfiPages->voidret_instructions;\
void (*unsupportedret_fp)() = (void*)fakeEfiPages->unsupportedret_instructions;\
efiRuntimeServices->GetTime = pto(mode,unsupportedret_fp);\
efiRuntimeServices->SetTime = pto(mode,unsupportedret_fp);\
efiRuntimeServices->GetWakeupTime = pto(mode,unsupportedret_fp);\
efiRuntimeServices->SetWakeupTime = pto(mode,unsupportedret_fp);\
efiRuntimeServices->SetVirtualAddressMap = pto(mode,unsupportedret_fp);\
efiRuntimeServices->ConvertPointer = pto(mode,unsupportedret_fp);\
efiRuntimeServices->GetVariable = pto(mode,unsupportedret_fp);\
efiRuntimeServices->GetNextVariableName = pto(mode,unsupportedret_fp);\
efiRuntimeServices->SetVariable = pto(mode,unsupportedret_fp);\
efiRuntimeServices->GetNextHighMonotonicCount = pto(mode,unsupportedret_fp);\
efiRuntimeServices->ResetSystem = pto(mode,voidret_fp);\
/*We're done.	Now CRC32 the thing so the kernel will accept it*/\
efiRuntimeServices->Hdr.CRC32 = crc32(0L, efiRuntimeServices, efiRuntimeServices->Hdr.HeaderSize);\
/*--------------------------------------------------------------------
* Finish filling in the rest of the boot args that we need.*/\
bootArgs->efiSystemTable = (uint32_t)efiSystemTable;\
bootArgs->efiMode = kBootArgsEfiMode##mode;\
/* The bootArgs structure as a whole is bzero'd so we don't need to fill in
* things like efiRuntimeServices* and what not.
*
* In fact, the only code that seems to use that is the hibernate code so it
* knows not to save the pages.	 It even checks to make sure its nonzero.
*/\
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
#if UNUSED
static const char const TSC_Frequency_prop[] = "TSCFrequency";
static const char const CPU_Frequency_prop[] = "CPUFrequency";
#endif
static const char const FSB_Frequency_prop[] = "FSBFrequency";

/*==========================================================================
 * SMBIOS
 */

static              uint64_t smbios_p;

void Register_Smbios_Efi(void* smbios)
{
    smbios_p = ((uint64_t)((uint32_t)smbios));	
}

/*==========================================================================
 * ACPI 
 */

static uint64_t     local_rsd_p			= 0;
static uint64_t     kFSBFrequency		= 0;
static uint32_t		kHardware_signature = 0;
static uint8_t		kType				= 0;
static uint32_t		kAdler32			= 0;
static ACPI_TABLES  acpi_tables;


EFI_STATUS Register_Acpi_Efi(void* rsd_p, unsigned char rev )
{
	EFI_STATUS Status = EFI_UNSUPPORTED;
	local_rsd_p = ((U64)((U32)rsd_p));
    
	if (local_rsd_p) {
		if (rev == 2)
		{
			Status = addConfigurationTable(&gEfiAcpi20TableGuid, &local_rsd_p, "ACPI_20");
		}
		else
		{
			Status = addConfigurationTable(&gEfiAcpiTableGuid, &local_rsd_p, "ACPI");			
		}
	}
	else 
	{
		Status = setupAcpiNoMod();
	}
		
	
	return Status;	
}

static EFI_STATUS EFI_FindAcpiTables(VOID)
{
	EFI_STATUS ret = EFI_UNSUPPORTED;
	
	if (local_rsd_p)
	{
		return EFI_SUCCESS;
	}

	if (!FindAcpiTables(&acpi_tables))
	{
		printf("Failed to detect ACPI tables.\n");
		ret = EFI_NOT_FOUND;
	}
	
	local_rsd_p = ((uint64_t)((uint32_t)acpi_tables.RsdPointer));
	
	if (local_rsd_p)
	{
		ret = EFI_SUCCESS;
	}
	return ret;

}

/* Setup ACPI without any patch. */
static EFI_STATUS setupAcpiNoMod(VOID)
{    	
	EFI_STATUS ret = EFI_UNSUPPORTED;
	
	if (EFI_FindAcpiTables() == EFI_SUCCESS) 
	{
		ACPI_TABLE_RSDP* rsdp = (ACPI_TABLE_RSDP*)((uint32_t)local_rsd_p);
		if(rsdp->Revision > 0 && (GetChecksum(rsdp, sizeof(ACPI_TABLE_RSDP)) == 0))
		{
			ret = addConfigurationTable(&gEfiAcpi20TableGuid, &local_rsd_p, "ACPI_20");		 
		}
		else
		{
			ret = addConfigurationTable(&gEfiAcpiTableGuid, &local_rsd_p, "ACPI");		
		}
	}    
	
	return ret;
}

EFI_STATUS setup_acpi (VOID)
{	
	EFI_STATUS ret = EFI_UNSUPPORTED;	
	
	do {
		
        if ((ret = EFI_FindAcpiTables()) != EFI_SUCCESS)
        {            
            break;
        }        
        
        {
            ACPI_TABLE_FADT *FacpPointer = (acpi_tables.FacpPointer64 != (void*)0ul) ? (ACPI_TABLE_FADT *)acpi_tables.FacpPointer64 : (ACPI_TABLE_FADT *)acpi_tables.FacpPointer;
            
            uint8_t type = FacpPointer->PreferredProfile;
            if (type <= MaxSupportedPMProfile) 
                safe_set_env(envType,type);
        }        
        
        ret = setupAcpiNoMod();
		
	} while (0);	
	
	return ret;	
	
}

/*==========================================================================
 * Fake EFI implementation
 */

/* These should be const but DT__AddProperty takes char* */
static const char const FIRMWARE_REVISION_PROP[] = "firmware-revision";
static const char const FIRMWARE_ABI_PROP[] = "firmware-abi";
static const char const FIRMWARE_VENDOR_PROP[] = "firmware-vendor";
static const char const FIRMWARE_NAME_PROP[] = "firmware-name";
static const char const FIRMWARE_DATE_PROP[] = "firmware-date";
static const char const FIRMWARE_DEV_PROP[] = "firmware-maintener";
static const char const FIRMWARE_PUBLISH_PROP[] = "firmware-publisher";


static const char const FIRMWARE_ABI_32_PROP_VALUE[] = "EFI32";
static const char const FIRMWARE_ABI_64_PROP_VALUE[] = "EFI64";
static const char const SYSTEM_ID_PROP[] = "system-id";
static const char const SYSTEM_SERIAL_PROP[] = "SystemSerialNumber";
static const char const SYSTEM_TYPE_PROP[] = "system-type";
static const char const MODEL_PROP[] = "Model";
static const char const MOTHERBOARD_NAME_PROP[] = "motherboard-name";


/*
 * Get an smbios option string option to convert to EFI_CHAR16 string
 */

static EFI_CHAR16* getSmbiosChar16(const char * key, size_t* len)
{
	if (!GetgPlatformName() && strncmp(key, "SMproductname", sizeof("SMproductname")) == 0)
		readSMBIOS(thePlatformName);
	
	const char	*PlatformName =  GetgPlatformName() ;
	
	const char	*src = (strncmp(key, "SMproductname", sizeof("SMproductname")) == 0) ? PlatformName : getStringForKey(key, DEFAULT_SMBIOS_CONFIG);
	
	EFI_CHAR16*	 dst = 0;	
	
	if (!key || !(*key) || !src) goto error;
	
    int tmp_len = strlen(src);
    
    *len = ((tmp_len)+1) * 2; // return the CHAR16 bufsize in cluding zero terminated CHAR16
    
    if (!(*len > 0)) goto error;
    
	dst = (EFI_CHAR16*) malloc( *len );
    if (!dst) 
    {
        goto error;
    }
    
	{
		size_t		 i = 0;
		for (; i < (tmp_len); i++)	 dst[i] = src[i];
	}
	dst[(tmp_len)] = '\0';
	return dst;
    
error:
    *len = 0;
    return NULL;
}

/*
 * Get the SystemID from the bios dmi info
 */

static EFI_CHAR8* getSmbiosUUID(VOID)
{
	static EFI_CHAR8		 uuid[UUID_LEN];
	int						 i, isZero, isOnes;
	SMBByte					*p;		
	
    p = (SMBByte*)(uint32_t)get_env(envUUID);
    
    if ( p == NULL )
	{
        DBG("No patched UUID found, fallback to original UUID (if exist) \n");
		
        readSMBIOS(theUUID);		
        p = (SMBByte*)(uint32_t)get_env(envUUID);
        
    }
    
	for (i=0, isZero=1, isOnes=1; i<UUID_LEN; i++)
	{
		if (p[i] != 0x00) isZero = 0;
		if (p[i] != 0xff) isOnes = 0;
	}
	
	if (isZero || isOnes) // empty or setable means: no uuid present
	{
		verbose("No UUID present in SMBIOS System Information Table\n");
		return 0;
	} 
#if DEBUG_EFI
	else
		verbose("Found UUID in SMBIOS System Information Table\n");
#endif
	
	memcpy(uuid, p, UUID_LEN);
	return uuid;
}

/*
 * return a binary UUID value from the overriden SystemID and SMUUID if found, 
 * or from the bios if not, or from a fixed value if no bios value is found 
 */

static int8_t *getSystemID(VOID)
{
    static int8_t				sysid[16];
	// unable to determine UUID for host. Error: 35 fix
	// Rek: new SMsystemid option conforming to smbios notation standards, this option should
	// belong to smbios config only ...
	EFI_CHAR8*	ret = getUUIDFromString(getStringForKey(kSystemID, DEFAULT_BOOT_CONFIG));
	
	if (!ret) // try bios dmi info UUID extraction	
		ret = getSmbiosUUID();
	
	if (!ret)
	{
		// no bios dmi UUID available, set a fixed value for system-id
		ret=getUUIDFromString((const char*) SYSTEM_ID);
		verbose("Customizing SystemID with : %s\n", getStringFromUUID(ret)); // apply a nice formatting to the displayed output
	}
	else	
	{
		const char *mac = getStringFromUUID(ret);
		verbose("MAC address : %c%c:%c%c:%c%c:%c%c:%c%c:%c%c\n",mac[24],mac[25],mac[26],mac[27],mac[28],mac[29]
				,mac[30],mac[31],mac[32],mac[33],mac[34],mac[35]);		 
		
	}
	
	if (ret)
	{        
		memcpy(sysid, ret, UUID_LEN);
        set_env_copy(envSysId, sysid, sizeof(sysid));
	}
	
	return sysid;
}

/*
 * Must be called AFTER setup Acpi because we need to take care of correct
 * facp content to reflect in ioregs
 */

static VOID setupSystemType(VOID)
{
	Node *node = DT__FindNode("/", false);
	if (node == 0) stop("Couldn't get root node");
	// we need to write this property after facp parsing
	// Export system-type only if it has been overrriden by the SystemType option
    kType = get_env(envType);
	DT__AddProperty(node, SYSTEM_TYPE_PROP, sizeof(uint8_t), &kType);
}

static VOID setupEfiDeviceTree(VOID)
{	
	Node		*node;
	
	node = DT__FindNode("/", false);
	
	if (node == 0) stop("Couldn't get root node");
    
#ifndef NO_BOOT_IMG
	{
		long           size;
		{
#include "appleClut8.h"
			size = sizeof(appleClut8);
			long clut = AllocateKernelMemory(size);
			bcopy(&appleClut8, (void*)clut, size);
            
            if (((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion[3] == '8') 
            {
                AllocateMemoryRange( "FailedCLUT", clut, size);
            
            } else
                AllocateMemoryRange( "BootCLUT", clut, size);
			
		}
		
		{
#include "failedboot.h"	
			size = 32 + kFailedBootWidth * kFailedBootHeight;
			long bootPict = AllocateKernelMemory(size);
            if (((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion[3] == '8') 
            {
                AllocateMemoryRange( "FailedImage", bootPict, size);    
            
            } else
                AllocateMemoryRange( "Pict-FailedBoot", bootPict, size);    
            
			((boot_progress_element *)bootPict)->width  = kFailedBootWidth;
			((boot_progress_element *)bootPict)->height = kFailedBootHeight;
			((boot_progress_element *)bootPict)->yOffset = kFailedBootOffset;	
			if (((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion[3] == '8') 
            {
                ((boot_progress_element *)bootPict)->data_size = size - 32; 
            }
			bcopy((char *)gFailedBootPict, (char *)(bootPict + 32), size - 32);
		}
	}
#endif
	//Fix an error with the Lion's (DP2+) installer	
	if (execute_hook("getboardproductPatched", NULL, NULL, NULL, NULL, NULL, NULL) != EFI_SUCCESS)
	{
		Setgboardproduct(getStringForKey("SMboardproduct", DEFAULT_SMBIOS_CONFIG));
		
		if (!Getgboardproduct()) readSMBIOS(theProducBoard);
		
	}
	if (Getgboardproduct())
	{
		DT__AddProperty(node, "board-id", strlen(Getgboardproduct())+1, Getgboardproduct());
	}
	
	{
		Node *chosenNode = DT__FindNode("/chosen", true);
		if (chosenNode)
		{			
			DT__AddProperty(chosenNode, "boot-args", strlen(bootArgs->CommandLine)+1, (EFI_CHAR16*)bootArgs->CommandLine);
			
			// "boot-uuid" MAIN GOAL IS SYMPLY TO BOOT FROM THE UUID SET IN THE DT AND DECREASE BOOT TIME, SEE IOKitBSDInit.cpp
			// additionally this value can be used by third-party apps or osx components (ex: pre-10.7 kextcache, ...)
			if (bootInfo->uuidStr[0]) 			
				DT__AddProperty(chosenNode, kBootUUIDKey, strlen(bootInfo->uuidStr)+1, bootInfo->uuidStr);
			
			if (GetgRootDevice())
			{
				
				DT__AddProperty(chosenNode, "boot-device-path", strlen(GetgRootDevice())+1, GetgRootDevice());			
				
			}			
#ifdef rootpath
			else
				if (gRootPath[0])
				{
					
					DT__AddProperty(chosenNode, "rootpath", strlen(gRootPath)+1, gRootPath);			
					
				} 
            
#endif
			
			// "boot-file" is not used by kextcache if there is no "boot-device-path" or if there is a valid "rootpath" ,
			// but i let it by default since it may be used by another service
			DT__AddProperty(chosenNode, "boot-file", strlen(bootInfo->bootFile)+1, (EFI_CHAR16*)bootInfo->bootFile);			
			
			if ((kAdler32 = (uint32_t)get_env(envAdler32))) 
				DT__AddProperty(chosenNode, "boot-kernelcache-adler32", sizeof(unsigned long), &kAdler32);
			
		}
	}	
	
	// We could also just do DT__FindNode("/efi/platform", true)
	// But I think eventually we want to fill stuff in the efi node
	// too so we might as well create it so we have a pointer for it too.
	Node *efiNode  = DT__AddChild(node, "efi");
	
	{
		// Set up the /efi/runtime-services table node similar to the way a child node of configuration-table
		// is set up.  That is, name and table properties
		Node *runtimeServicesNode = DT__AddChild(efiNode, "runtime-services");		
		Node *kernelCompatibilityNode = 0; // ??? not sure that it should be used like that (because it's maybe the kernel capability and not the cpu capability)
		
		if (((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion[3] > '6')
		{
			kernelCompatibilityNode = DT__AddChild(efiNode, "kernel-compatibility");	
			DT__AddProperty(kernelCompatibilityNode, "i386", sizeof(uint32_t), (EFI_UINT32*)&DEVICE_SUPPORTED);
		}
		
		if (get_env(envarchCpuType) == CPU_TYPE_I386)
		{
			// The value of the table property is the 32-bit physical address for the RuntimeServices table.
			// Since the EFI system table already has a pointer to it, we simply use the address of that pointer
			// for the pointer to the property data.  Warning.. DT finalization calls free on that but we're not
			// the only thing to use a non-malloc'd pointer for something in the DT
			
			DT__AddProperty(runtimeServicesNode, "table", sizeof(uint64_t), &gST32->RuntimeServices);
			DT__AddProperty(efiNode, FIRMWARE_ABI_PROP, sizeof(FIRMWARE_ABI_32_PROP_VALUE), (char*)FIRMWARE_ABI_32_PROP_VALUE);
		}
		else
		{
			if (kernelCompatibilityNode) 
				DT__AddProperty(kernelCompatibilityNode, "x86_64", sizeof(uint32_t), (EFI_UINT32*)&DEVICE_SUPPORTED);
			
			DT__AddProperty(runtimeServicesNode, "table", sizeof(uint64_t), &gST64->RuntimeServices);
			DT__AddProperty(efiNode, FIRMWARE_ABI_PROP, sizeof(FIRMWARE_ABI_64_PROP_VALUE), (char*)FIRMWARE_ABI_64_PROP_VALUE);
		}
	}
	
	DT__AddProperty(efiNode, FIRMWARE_REVISION_PROP, sizeof(FIRMWARE_REVISION), (EFI_UINT32*)&FIRMWARE_REVISION);
	DT__AddProperty(efiNode, FIRMWARE_VENDOR_PROP, sizeof(FIRMWARE_VENDOR), (EFI_CHAR16*)FIRMWARE_VENDOR);
	DT__AddProperty(efiNode, FIRMWARE_NAME_PROP, sizeof(FIRMWARE_NAME), (EFI_CHAR16*)FIRMWARE_NAME);
	DT__AddProperty(efiNode, FIRMWARE_DATE_PROP, strlen(I386BOOT_BUILDDATE)+1, I386BOOT_BUILDDATE);	
	DT__AddProperty(efiNode, FIRMWARE_DEV_PROP, strlen(FIRMWARE_MAINTENER)+1, FIRMWARE_MAINTENER);
	DT__AddProperty(efiNode, FIRMWARE_PUBLISH_PROP, strlen(FIRMWARE_PUBLISHER)+1, FIRMWARE_PUBLISHER);
	
	{
		// Export it for amlsgn support
		char * DefaultPlatform = readDefaultPlatformName();
		if (DefaultPlatform)
		{
			DT__AddProperty(efiNode, MOTHERBOARD_NAME_PROP, strlen(DefaultPlatform)+1, DefaultPlatform);
		}
		
	}
	
	// Set up the /efi/configuration-table node which will eventually have several child nodes for
	// all of the configuration tables needed by various kernel extensions.
	gEfiConfigurationTableNode = DT__AddChild(efiNode, "configuration-table");
	
	{
		EFI_CHAR16   *serial = 0, *productname = 0;
        int8_t				*sysid = 0;
		size_t		 len = 0;
		
		// Now fill in the /efi/platform Node
		Node *efiPlatformNode = DT__AddChild(efiNode, "platform");
		
		DT__AddProperty(efiPlatformNode, "DevicePathsSupported", sizeof(uint32_t), (EFI_UINT32*)&DEVICE_SUPPORTED);
		
		// NOTE WELL: If you do add FSB Frequency detection, make sure to store
		// the value in the fsbFrequency global and not an malloc'd pointer
		// because the DT_AddProperty function does not copy its args.
		
        kFSBFrequency = get_env(envFSBFreq);
		if (kFSBFrequency != 0)
			DT__AddProperty(efiPlatformNode, FSB_Frequency_prop, sizeof(uint64_t), &kFSBFrequency);		
		
#if UNUSED
		// Export TSC and CPU frequencies for use by the kernel or KEXTs
		Platform.CPU.TSCFrequency = get_env(envTSCFreq);
        if (Platform.CPU.TSCFrequency != 0)
			DT__AddProperty(efiPlatformNode, TSC_Frequency_prop, sizeof(uint64_t), &Platform.CPU.TSCFrequency);
		
        Platform.CPU.CPUFrequency = get_env(envCPUFreq);
		if (Platform.CPU.CPUFrequency != 0)
			DT__AddProperty(efiPlatformNode, CPU_Frequency_prop, sizeof(uint64_t), &Platform.CPU.CPUFrequency);
#endif
		
		// Export system-id. Can be disabled with SystemId=No in com.apple.Boot.plist		
		if ((sysid = getSystemID()))
			DT__AddProperty(efiPlatformNode, SYSTEM_ID_PROP, UUID_LEN, (EFI_UINT32*) sysid);
		
		// Export SystemSerialNumber if present
		if ((serial=getSmbiosChar16("SMserial", &len)))
			DT__AddProperty(efiPlatformNode, SYSTEM_SERIAL_PROP, len, serial);
		
		// Export Model if present
		if ((productname=getSmbiosChar16("SMproductname", &len)))
			DT__AddProperty(efiPlatformNode, MODEL_PROP, len, productname);
	}		
	
	// Fill /efi/device-properties node.			
	setupDeviceProperties(efiNode);
}

/*
 * Load the smbios.plist override config file if any
 */

void setupSmbiosConfigFile(const char *filename)
{	
	static bool readSmbConfigFile = true;
	
	if (readSmbConfigFile == true)
	{
		char		dirSpecSMBIOS[128] = "";
		const char *override_pathname = NULL;
		int			len = 0, err = 0;
		
		// Take in account user overriding
		if (getValueForKey("SMBIOS", &override_pathname, &len, DEFAULT_BOOT_CONFIG) && len > 0)
		{
			// Specify a path to a file, e.g. SMBIOS=/Extra/macProXY.plist
			snprintf(dirSpecSMBIOS, sizeof(dirSpecSMBIOS),override_pathname);
			err = loadConfigFile(dirSpecSMBIOS, DEFAULT_SMBIOS_CONFIG);
		}
		else
		{
			// Check selected volume's Extra.
			snprintf(dirSpecSMBIOS, sizeof(dirSpecSMBIOS),"/Extra/%s", filename);
			if ((err = loadConfigFile(dirSpecSMBIOS, DEFAULT_SMBIOS_CONFIG)))
			{
				// Check booter volume/rdbt Extra.
				snprintf(dirSpecSMBIOS, sizeof(dirSpecSMBIOS),"bt(0,0)/Extra/%s", filename);
				err = loadConfigFile(dirSpecSMBIOS, DEFAULT_SMBIOS_CONFIG);
			}
		}
		
		if (err)
		{
			verbose("No SMBIOS config file found.\n");
		}
		readSmbConfigFile = false;
	}
}

static VOID setup_Smbios(VOID)
{			
	if (execute_hook("getSmbiosPatched",NULL, NULL, NULL, NULL, NULL, NULL) != EFI_SUCCESS)
	{
		DBG("Using the original SMBIOS !!\n");	
        struct SMBEntryPoint *smbios_o = getSmbiosOriginal();	
        smbios_p = ((uint64_t)((uint32_t)smbios_o));                 
	}	
}

static VOID setup_machine_signature(VOID)
{
	Node *chosenNode = DT__FindNode("/chosen", false);
	if (chosenNode)
	{
		if (get_env(envHardwareSignature) == 0xFFFFFFFF)
		{			
			do {
				if (!local_rsd_p)
				{			
					if ( EFI_FindAcpiTables() != EFI_SUCCESS)
					{
						printf("Failed to detect ACPI tables.\n");
						break;
					}					
				}
				
				ACPI_TABLE_FACS *FacsPointer = (acpi_tables.FacsPointer64 != (void*)0ul) ? (ACPI_TABLE_FACS *)acpi_tables.FacsPointer64:(ACPI_TABLE_FACS *)acpi_tables.FacsPointer;				
                
                safe_set_env(envHardwareSignature , FacsPointer->HardwareSignature);
				
			} while (0);            	
			
			// Verify that we have a valid hardware signature
			if (get_env(envHardwareSignature) == 0xFFFFFFFF) 
			{
				verbose("Warning: hardware_signature is invalid, defaulting to 0 \n");                
                safe_set_env(envHardwareSignature , 0);
			}
		}
		
        kHardware_signature = get_env(envHardwareSignature);        
		DT__AddProperty(chosenNode, "machine-signature", sizeof(uint32_t), &kHardware_signature);
	}
	
}

/*
 * Installs all the needed configuration table entries
 */

static VOID setupEfiConfigurationTable(VOID)
{
    if (smbios_p)
        addConfigurationTable(&gEfiSmbiosTableGuid, &smbios_p, NULL);
#ifndef NO_SMP_SUPPORT
	if (get_env(envVendor) == CPUID_VENDOR_INTEL )
	{
		int num_cpus;
		
		void *mps_p = imps_probe(&num_cpus);
		
		if (mps_p)
		{		
			addConfigurationTable(&gEfiMpsTableGuid, ((uint64_t*)((uint32_t)mps_p)), NULL);
		}
		
#if DEBUG_EFI
        if (num_cpus != get_env(envNoCores))
        {
            printf("Warning: SMP nb of core (%d) mismatch with the value found in cpu.c (%d) \n",num_cpus,get_env(envNoCores));                
        }        
#endif               
	}	
#endif
	// PM_Model
	if (get_env(envIsServer))
    {
        safe_set_env(envType , Workstation);
	}
	else if (get_env(envIsMobile))//Slice
	{	
        safe_set_env(envType , Mobile);
	} 
	else
	{
        safe_set_env(envType , Desktop);
	}
	
	// Invalidate the platform hardware signature (this needs to be verified with acpica, but i guess that 0xFFFFFFFF is an invalid signature)  	
    safe_set_env(envHardwareSignature , 0xFFFFFFFF);
	
	// Setup ACPI (based on the mackerintel's patch)
	(VOID)setup_acpi();
	
	setup_machine_signature();
	
	// We now have to write the system-type in ioregs: we cannot do it before in setupDeviceTree()
	// because we need to take care of facp original content, if it is correct.
	setupSystemType();
	
	// We've obviously changed the count.. so fix up the CRC32	
    EFI_ST_FIX_CRC32();
}

/*
 * Entrypoint from boot.c
 */

void setupFakeEfi(void)
{
	// Collect PCI info &| Generate device prop string
	setup_pci_devs(root_pci_dev);
	
	// load smbios.plist file if any
	setupSmbiosConfigFile("SMBIOS.plist");
	setup_Smbios();
	
	// Initialize the base table
	if (get_env(envarchCpuType) == CPU_TYPE_I386)
	{
		setupEfiTables(32); 
	}
	else
	{
		setupEfiTables(64); 
	}
	
	// Initialize the device tree
	setupEfiDeviceTree();
	
	// Add configuration table entries to both the services table and the device tree
	setupEfiConfigurationTable();		
	
}