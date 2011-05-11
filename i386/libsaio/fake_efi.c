
/*
 * Copyright 2007 David F. Elliott.	 All rights reserved.
 */

/*
 * Copyright 2010,2011 Cadet-petit Armel <armelcadetpetit@gmail.com>. All rights reserved.
 */

#include "libsaio.h"
#include "boot.h"
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

#define ptov64(addr) (uint64_t)((uint64_t)addr | 0xFFFFFF8000000000ULL)

/*==========================================================================
 * Fake EFI implementation
 */

/* Identify ourselves as the EFI firmware vendor */
static EFI_CHAR16 const FIRMWARE_VENDOR[] = {'J','a','r','o','d','_','1','.','2', 0};
static EFI_UINT32 const FIRMWARE_REVISION = 0x0001000a;
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
		stop("Ran out of space for configuration tables.  Increase the reserved size in the code.\n");
	
	if (pGuid == NULL)
		return EFI_INVALID_PARAMETER;
	
	if (table != NULL)
	{
		// FIXME
		//((EFI_CONFIGURATION_TABLE_64 *)gST->ConfigurationTable)[i].VendorGuid = *pGuid;
		//((EFI_CONFIGURATION_TABLE_64 *)gST->ConfigurationTable)[i].VendorTable = (EFI_PTR64)table;
		
		//++gST->NumberOfTableEntries;
		
		Node *tableNode = DT__AddChild(gEfiConfigurationTableNode, mallocStringForGuid(pGuid));
		
		// Use the pointer to the GUID we just stuffed into the system table
		DT__AddProperty(tableNode, "guid", sizeof(EFI_GUID), (void*)pGuid);
		
		// The "table" property is the 32-bit (in our implementation) physical address of the table
		DT__AddProperty(tableNode, "table", sizeof(void*) * 2, table);
		
		// Assume the alias pointer is a global or static piece of data
		if (alias != NULL)
			DT__AddProperty(tableNode, "alias", strlen(alias)+1, (char*)alias);
		
		return EFI_SUCCESS;
	}
	return EFI_UNSUPPORTED;
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
static const char const TSC_Frequency_prop[] = "TSCFrequency";
static const char const FSB_Frequency_prop[] = "FSBFrequency";
static const char const CPU_Frequency_prop[] = "CPUFrequency";

/*==========================================================================
 * SMBIOS
 */

/* From Foundation/Efi/Guid/Smbios/SmBios.h */
/* Modified to wrap Data4 array init with {} */
#define EFI_SMBIOS_TABLE_GUID {0xeb9d2d31, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d}}

/* From Foundation/Efi/Guid/Smbios/SmBios.c */
EFI_GUID const	gEfiSmbiosTableGuid = EFI_SMBIOS_TABLE_GUID;

#define SMBIOS_RANGE_START		0x000F0000
#define SMBIOS_RANGE_END		0x000FFFFF

/* '_SM_' in little endian: */
#define SMBIOS_ANCHOR_UINT32_LE 0x5f4d535f

#ifndef DEBUG_SMBIOS
#define DEBUG_SMBIOS 0
#endif

#if DEBUG_SMBIOS
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

uint64_t smbios_p;

// getting smbios addr with fast compare ops, late checksum testing ...
#define COMPARE_DWORD(a,b) ( *((u_int32_t *) a) == *((u_int32_t *) b) )
static const char * const SMTAG = "_SM_";
static const char* const DMITAG= "_DMI_";

static struct SMBEntryPoint *getAddressOfSmbiosTable(void)
{
	struct SMBEntryPoint	*smbios;
	/* 
	 * The logic is to start at 0xf0000 and end at 0xfffff iterating 16 bytes at a time looking
	 * for the SMBIOS entry-point structure anchor (literal ASCII "_SM_").
	 */
	smbios = (struct SMBEntryPoint*) SMBIOS_RANGE_START;
	while (smbios <= (struct SMBEntryPoint *)SMBIOS_RANGE_END) {
		if (COMPARE_DWORD(smbios->anchor, SMTAG)  && 
			COMPARE_DWORD(smbios->dmi.anchor, DMITAG) &&
			smbios->dmi.anchor[4]==DMITAG[4] &&
			checksum8(smbios, sizeof(struct SMBEntryPoint)) == 0)
	    {			
			return smbios;
	    }
		smbios = (struct SMBEntryPoint*) ( ((char*) smbios) + 16 );
	}
	printf("Error: Could not find original SMBIOS !!\n");
	pause();
	return NULL;
}

static struct SMBEntryPoint *orig = NULL; // cached

struct SMBEntryPoint *getSmbiosOriginal()
{    	
    if (orig == NULL) {
		orig = getAddressOfSmbiosTable();		
		
		if (orig) {
			verbose("Found System Management BIOS (SMBIOS) table\n");			
		}
        
    }
    return orig;    
}


#define theUUID 0
#define thePlatformName 1

/* get UUID or product Name from original SMBIOS, stripped version of kabyl's readSMBIOSInfo */
void local_readSMBIOS(int value)
{			
	
	SMBEntryPoint *eps = getSmbiosOriginal();
	if (eps == NULL) return;
	
	uint8_t *structPtr = (uint8_t *)eps->dmi.tableAddress;
	SMBStructHeader *structHeader = (SMBStructHeader *)structPtr;
	
	for (;((eps->dmi.tableAddress + eps->dmi.tableLength) > ((uint32_t)(uint8_t *)structHeader + sizeof(SMBStructHeader)));)
	{
		switch (structHeader->type)
		{				
			case 1: //kSMBTypeSystemInformation
			{
				switch (value) {
					case theUUID:
						Platform->UUID = ((SMBSystemInformation *)structHeader)->uuid;
						break;
					case thePlatformName:
					{
						uint8_t *stringPtr = (uint8_t *)structHeader + structHeader->length;
						uint8_t field = ((SMBSystemInformation *)structHeader)->productName;
						
						if (!field)
							return;
						
						for (field--; field != 0 && strlen((char *)stringPtr) > 0; 
							 field--, stringPtr = (uint8_t *)((uint32_t)stringPtr + strlen((char *)stringPtr) + 1));
						
						DBG("original SMBIOS Product name: %s\n",(char *)stringPtr);
						gPlatformName = (char *)stringPtr;
						break;
					}
					default:
						break;
				}
								
				return;
				
				break;	
			}				
				
		}
		
		structPtr = (uint8_t *)((uint32_t)structHeader + structHeader->length);
		for (; ((uint16_t *)structPtr)[0] != 0; structPtr++);
		
		if (((uint16_t *)structPtr)[0] == 0)
			structPtr += 2;
		
		structHeader = (SMBStructHeader *)structPtr;
	}	
}

/*==========================================================================
 * ACPI 
 */

#define EFI_ACPI_TABLE_GUID \
  { \
	0xeb9d2d30, 0x2d88, 0x11d3, { 0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d } \
  }

#define EFI_ACPI_20_TABLE_GUID \
  { \
	0x8868e871, 0xe4f1, 0x11d3, { 0xbc, 0x22, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 } \
  }

EFI_GUID gEfiAcpiTableGuid = EFI_ACPI_TABLE_GUID;
EFI_GUID gEfiAcpi20TableGuid = EFI_ACPI_20_TABLE_GUID;

uint64_t acpi10_p;
uint64_t acpi20_p;

static uint64_t local_acpi10_p;
static uint64_t local_acpi20_p;

#define tableSign(table, sgn) (table[0]==sgn[0] && table[1]==sgn[1] && table[2]==sgn[2] && table[3]==sgn[3])

static struct acpi_common_header * get_ACPI_TABLE(const char * table);
static struct acpi_2_fadt *gFADT = 0;

#define EFI_MPS_TABLE_GUID \
{ \
0xeb9d2d2f,0x2d88,0x11d3,{0x9a,0x16,0x0,0x90,0x27,0x3f,0xc1,0x4d} \
}
EFI_GUID gEfiMpsTableGuid = EFI_MPS_TABLE_GUID;

#define MP_SIGL 0x5f504d5f
#define MP_SIGSTR "_MP_"
struct mp_t {
	uint8_t sig[4];
	uint32_t config_ptr;
	uint8_t len;
	uint8_t ver;
	uint8_t checksum;
	uint8_t f1;
	uint8_t f2;
	uint8_t fr[3];
}__attribute__((packed)) mp_t;

/*
 * THIS FILE USE SOME CODE FROM smp-imps
 *
 *  <Insert copyright here : it must be BSD-like so anyone can use it>
 *
 *  Author:  Erich Boleyn  <erich@uruk.org>   http://www.uruk.org/~erich/
 *
 *  Source file implementing Intel MultiProcessor Specification (MPS)
 *  version 1.1 and 1.4 SMP hardware control for Intel Architecture CPUs,
 *  with hooks for running correctly on a standard PC without the hardware.
 *
 *  This file was created from information in the Intel MPS version 1.4
 *  document, order number 242016-004, which can be ordered from the
 *  Intel literature center.
 *
 *  General limitations of this code:
 *
 *   (1) : This code has never been tested on an MPS-compatible system with
 *           486 CPUs, but is expected to work.
 *   (2) : Presumes "int", "long", and "unsigned" are 32 bits in size, and
 *	     that 32-bit pointers and memory addressing is used uniformly.
 */

/*
 *  MP Configuration Table Header  (cth)
 *
 *  Look at page 4-5 of the MP spec for the starting definitions of
 *  this structure.
 */
struct imps_cth
{
	unsigned sig;
	unsigned short base_length;
	unsigned char spec_rev;
	unsigned char checksum;
	char oem_id[8];
	char prod_id[12];
	unsigned oem_table_ptr;
	unsigned short oem_table_size;
	unsigned short entry_count;
	unsigned lapic_addr;
	unsigned short extended_length;
	unsigned char extended_checksum;
	char reserved[1];
};

static inline void
cmos_write_byte (int loc, int val)
{
	outb (0x70, loc);
	outb (0x71, val);
}

static inline unsigned
cmos_read_byte (int loc)
{
	outb (0x70, loc);
	return inb (0x71);
}

#define LAPIC_ID				0x20

static int lapic_dummy = 0;
unsigned imps_lapic_addr = ((unsigned)(&lapic_dummy)) - LAPIC_ID;

#include "smp.h"

#define CMOS_WRITE_BYTE(x, y)	cmos_write_byte(x, y)
#define CMOS_READ_BYTE(x)	cmos_read_byte(x)

/*
 *  Defines that are here so as not to be in the global header file.
 */
#define EBDA_SEG_ADDR			0x40E
#define EBDA_SEG_LEN			0x400
#define BIOS_RESET_VECTOR		0x467
#define LAPIC_ADDR_DEFAULT		0xFEE00000uL
#define IOAPIC_ADDR_DEFAULT		0xFEC00000uL
#define CMOS_RESET_CODE			0xF
#define		CMOS_RESET_JUMP		0xa
#define CMOS_BASE_MEMORY		0x15

static struct mp_t *
biosacpi_search_mp(char *base, int length);

struct mp_t* getAddressOfMPSTable()
{
	struct mp_t *mp;
    uint16_t		*addr;
	
    /* EBDA is the 1 KB addressed by the 16 bit pointer at 0x40E. */
    addr = (uint16_t *)ptov(EBDA_SEG_ADDR);
    if ((mp = biosacpi_search_mp((char *)(*addr << 4), EBDA_SEG_LEN)) != NULL)
		return (mp);
	
	unsigned mem_lower = ((CMOS_READ_BYTE(CMOS_BASE_MEMORY+1) << 8)
						  | CMOS_READ_BYTE(CMOS_BASE_MEMORY))       << 10;
	
	if ((mp = biosacpi_search_mp((char *)mem_lower, EBDA_SEG_LEN)) != NULL)
		return (mp);
	
    if ((mp = biosacpi_search_mp((char *)0x00F0000, ACPI_RANGE_END)) != NULL)
		return (mp);
	
    return (NULL);
	    
}

static struct mp_t *
biosacpi_search_mp(char *base, int length)
{
	/* TODO: Before searching the BIOS space we are supposed to search the first 1K of the EBDA */
	struct mp_t *mp;
    int			ofs;
	
    /* search on 16-byte boundaries */
    for (ofs = 0; ofs < length; ofs += 16) {
		
			mp = (struct mp_t*)ptov(base + ofs);
			
			/* compare signature, validate checksum */
			if (!strncmp((char*)mp->sig, MP_SIGSTR, strlen(MP_SIGSTR))) {
				uint8_t csum = checksum8(mp, sizeof(struct mp_t));
				if(csum == 0) { 
					return mp;
				}
				
			}				
	}	
    return NULL;
}

#define ACPI_SIG_RSDP           "RSD PTR "      /* Root System Description Pointer */
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
biosacpi_search_rsdp(char *base, int length, int rev);

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

/* Setup ACPI without any patch. */
static int setupAcpiNoMod()
{		
	int ret = 0;
	if(local_acpi20_p) {
		addConfigurationTable(&gEfiAcpi20TableGuid, &local_acpi20_p, "ACPI_20");
		ret = 1;
	} else if (local_acpi10_p) {
		addConfigurationTable(&gEfiAcpiTableGuid, &local_acpi10_p, "ACPI");
		ret = 1;
	}
	
	
	return ret;
}

int setup_acpi (void)
{	
	int ret = 0;
	
	/* XXX aserebln why uint32 cast if pointer is uint64 ? */
	acpi10_p = local_acpi10_p = (uint32_t)biosacpi_find_rsdp(0);
	acpi20_p = local_acpi20_p = (uint32_t)biosacpi_find_rsdp(2);
	
	execute_hook("setupEfiConfigurationTable", &ret, NULL, NULL, NULL, NULL, NULL);
	
	if (!ret) {
		gFADT = (struct acpi_2_fadt *)get_ACPI_TABLE("FACP");
		uint8_t type = gFADT->PM_Profile;
		if (type <= MaxSupportedPMProfile) {
			Platform->Type = type;
		}
		ret = setupAcpiNoMod();			
	}
	
	return ret;	
	
}

/*==========================================================================
 * Fake EFI implementation
 */

/* These should be const but DT__AddProperty takes char* */
static const char const FIRMWARE_REVISION_PROP[] = "firmware-revision";
static const char const FIRMWARE_ABI_PROP[] = "firmware-abi";
static const char const FIRMWARE_VENDOR_PROP[] = "firmware-vendor";
static const char const FIRMWARE_ABI_32_PROP_VALUE[] = "EFI32";
static const char const FIRMWARE_ABI_64_PROP_VALUE[] = "EFI64";
static const char const SYSTEM_ID_PROP[] = "system-id";
static const char const SYSTEM_SERIAL_PROP[] = "SystemSerialNumber";
static const char const SYSTEM_TYPE_PROP[] = "system-type";
static const char const MODEL_PROP[] = "Model";


/*
 * Get an smbios option string option to convert to EFI_CHAR16 string
 */

static EFI_CHAR16* getSmbiosChar16(const char * key, size_t* len)
{
	if (!gPlatformName && strcmp(key, "SMproductname") == 0)
		local_readSMBIOS(thePlatformName);
	
	const char	*src = (strcmp(key, "SMproductname") == 0) ? gPlatformName : getStringForKey(key, &bootInfo->smbiosConfig);
	
	EFI_CHAR16*	 dst = 0;
	size_t		 i = 0;
	
	if (!key || !(*key) || !src) return 0;
	
	*len = strlen(src);
	dst = (EFI_CHAR16*) malloc( ((*len)+1) * 2 );
	for (; i < (*len); i++)	 dst[i] = src[i];
	dst[(*len)] = '\0';
	*len = ((*len)+1)*2; // return the CHAR16 bufsize in cluding zero terminated CHAR16
	return dst;
}

/*
 * Get the SystemID from the bios dmi info
 */

static	EFI_CHAR8* getSmbiosUUID()
{
	static EFI_CHAR8		 uuid[UUID_LEN];
	int						 i, isZero, isOnes;
	SMBByte					*p;		
	
	local_readSMBIOS(theUUID);		
	p = (SMBByte*)Platform->UUID;
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
#if DEBUG_SMBIOS
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

static EFI_CHAR8* getSystemID()
{
	// unable to determine UUID for host. Error: 35 fix
	// Rek: new SMsystemid option conforming to smbios notation standards, this option should
	// belong to smbios config only ...
	EFI_CHAR8*	ret = getUUIDFromString(getStringForKey(kSystemID, &bootInfo->bootConfig));
	
	if (!ret) // try bios dmi info UUID extraction	
		ret = getSmbiosUUID();		 

	
	if (!ret) {
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
	memcpy(bootInfo->sysid, ret, UUID_LEN);
	
	return ret;
}

/*
 * Must be called AFTER setup Acpi because we need to take care of correct
 * facp content to reflect in ioregs
 */

void setupSystemType()
{
	Node *node = DT__FindNode("/", false);
	if (node == 0) stop("Couldn't get root node");
	// we need to write this property after facp parsing
	// Export system-type only if it has been overrriden by the SystemType option
	DT__AddProperty(node, SYSTEM_TYPE_PROP, sizeof(Platform->Type), &Platform->Type);
}

struct boot_progress_element {
	unsigned int	width;
	unsigned int	height;
	int			yOffset;
	unsigned int	res[5];
	unsigned char	data[0];
};
typedef struct boot_progress_element boot_progress_element;

void setupEfiDeviceTree(void)
{
	EFI_CHAR16*	 serial = 0, *productname = 0;
	size_t		 len = 0;
	Node		*node;
	extern char gMacOSVersion[];
	node = DT__FindNode("/", false);
	
	if (node == 0) stop("Couldn't get root node");
	
#include "appleClut8.h"
	long clut = AllocateKernelMemory(sizeof(appleClut8));
	bcopy(&appleClut8, (void*)clut, sizeof(appleClut8));
	AllocateMemoryRange( "BootCLUT", clut, sizeof(appleClut8),-1);
	
#include "failedboot.h"
	
	long bootPict = AllocateKernelMemory(sizeof(boot_progress_element));
	((boot_progress_element *)bootPict)->width  = kFailedBootWidth;
    ((boot_progress_element *)bootPict)->height = kFailedBootHeight;
    ((boot_progress_element *)bootPict)->yOffset = kFailedBootOffset;
	
	long bootFail = AllocateKernelMemory(sizeof(gFailedBootPict));
	bcopy(&gFailedBootPict, (void*)bootFail, sizeof(gFailedBootPict));
    ((boot_progress_element *)bootPict)->data[0]   = (uint8_t)bootFail;// ?? please verify
	
	AllocateMemoryRange( "Pict-FailedBoot", bootPict, sizeof(boot_progress_element),-1);
	
	//Fix error message with Lion DP2+ installer
	const char *boardid = getStringForKey("SMboardproduct", &bootInfo->smbiosConfig);	
	if (boardid)	
		DT__AddProperty(node, "board-id", strlen(boardid)+1, (char*)boardid);
	
	
	Node *chosenNode = DT__FindNode("/chosen", true);
	if (chosenNode) {
				
		DT__AddProperty(chosenNode, "boot-args", strlen(bootArgs->CommandLine)+1, (EFI_CHAR16*)bootArgs->CommandLine);
		
		if (uuidSet &&bootInfo->uuidStr[0]) 			
				DT__AddProperty(chosenNode, kBootUUIDKey, strlen(bootInfo->uuidStr)+1, bootInfo->uuidStr);
		
		/*if (gRootPath[0]) {
			
			DT__AddProperty(chosenNode, "rootpath", 256, gRootPath);			
			
		} else */if (gRootDevice) {
			
			DT__AddProperty(chosenNode, "boot-device-path", strlen(gRootDevice)+1, gRootDevice);			
			
		}			
			
				
		// "boot-file" is not used by kextcache if there is no "boot-device-path" or if there is a valid "rootpath" ,
		// but i let it by default since it may be used by another service
		DT__AddProperty(chosenNode, "boot-file", strlen(bootInfo->bootFile)+1, (EFI_CHAR16*)bootInfo->bootFile);
				
	
		if (bootInfo->adler32) 
		DT__AddProperty(chosenNode, "boot-kernelcache-adler32", sizeof(unsigned long), &bootInfo->adler32);

	}

	// We could also just do DT__FindNode("/efi/platform", true)
	// But I think eventually we want to fill stuff in the efi node
	// too so we might as well create it so we have a pointer for it too.
	node = DT__AddChild(node, "efi");
	// Set up the /efi/runtime-services table node similar to the way a child node of configuration-table
	// is set up.  That is, name and table properties
	Node *runtimeServicesNode = DT__AddChild(node, "runtime-services");
	
	Node *kernelCompatibilityNode = 0; // ??? not sure that it should be used like that
	if (gMacOSVersion[3] == '7'){
		kernelCompatibilityNode = DT__AddChild(node, "kernel-compatibility");	
		DT__AddProperty(kernelCompatibilityNode, "i386", sizeof(uint32_t), (EFI_UINT32*)&DEVICE_SUPPORTED);
	}
	
	if (archCpuType == CPU_TYPE_I386)
	{
		// The value of the table property is the 32-bit physical address for the RuntimeServices table.
		// Since the EFI system table already has a pointer to it, we simply use the address of that pointer
		// for the pointer to the property data.  Warning.. DT finalization calls free on that but we're not
		// the only thing to use a non-malloc'd pointer for something in the DT
		
		DT__AddProperty(runtimeServicesNode, "table", sizeof(uint64_t), &gST32->RuntimeServices);
		DT__AddProperty(node, FIRMWARE_ABI_PROP, sizeof(FIRMWARE_ABI_32_PROP_VALUE), (char*)FIRMWARE_ABI_32_PROP_VALUE);
	}
	else
	{
		if (kernelCompatibilityNode) 
			DT__AddProperty(kernelCompatibilityNode, "x86_64", sizeof(uint32_t), (EFI_UINT32*)&DEVICE_SUPPORTED);
		
		DT__AddProperty(runtimeServicesNode, "table", sizeof(uint64_t), &gST64->RuntimeServices);
		DT__AddProperty(node, FIRMWARE_ABI_PROP, sizeof(FIRMWARE_ABI_64_PROP_VALUE), (char*)FIRMWARE_ABI_64_PROP_VALUE);
	}
	DT__AddProperty(node, FIRMWARE_REVISION_PROP, sizeof(FIRMWARE_REVISION), (EFI_UINT32*)&FIRMWARE_REVISION);
	DT__AddProperty(node, FIRMWARE_VENDOR_PROP, sizeof(FIRMWARE_VENDOR), (EFI_CHAR16*)FIRMWARE_VENDOR);
	
	// Set up the /efi/configuration-table node which will eventually have several child nodes for
	// all of the configuration tables needed by various kernel extensions.
	gEfiConfigurationTableNode = DT__AddChild(node, "configuration-table");
	
	// Now fill in the /efi/platform Node
	Node *efiPlatformNode = DT__AddChild(node, "platform");
	
	DT__AddProperty(efiPlatformNode, "DevicePathsSupported", sizeof(uint32_t), (EFI_UINT32*)&DEVICE_SUPPORTED);
	
	// NOTE WELL: If you do add FSB Frequency detection, make sure to store
	// the value in the fsbFrequency global and not an malloc'd pointer
	// because the DT_AddProperty function does not copy its args.
	
	if (Platform->CPU.FSBFrequency != 0)
		DT__AddProperty(efiPlatformNode, FSB_Frequency_prop, sizeof(uint64_t), &Platform->CPU.FSBFrequency);

	
#if UNUSED
	// Export TSC and CPU frequencies for use by the kernel or KEXTs
	if (Platform->CPU.TSCFrequency != 0)
		DT__AddProperty(efiPlatformNode, TSC_Frequency_prop, sizeof(uint64_t), &Platform->CPU.TSCFrequency);
	
	if (Platform->CPU.CPUFrequency != 0)
		DT__AddProperty(efiPlatformNode, CPU_Frequency_prop, sizeof(uint64_t), &Platform->CPU.CPUFrequency);
#endif
	
	// Export system-id. Can be disabled with SystemId=No in com.apple.Boot.plist
	
	if (getSystemID())
		DT__AddProperty(efiPlatformNode, SYSTEM_ID_PROP, UUID_LEN, (EFI_UINT32*) bootInfo->sysid);

	 // Export SystemSerialNumber if present
	if ((serial=getSmbiosChar16("SMserial", &len)))
		DT__AddProperty(efiPlatformNode, SYSTEM_SERIAL_PROP, len, serial);
	
	// Export Model if present
	if ((productname=getSmbiosChar16("SMproductname", &len)))
		DT__AddProperty(efiPlatformNode, MODEL_PROP, len, productname);

	// Fill /efi/device-properties node.
	setupDeviceProperties(node);
}

/*
 * Load the smbios.plist override config file if any
 */
static bool readSmbConfigFile = true;

void setupSmbiosConfigFile(const char *filename)
{	
	
	if (readSmbConfigFile == true) {
		
		char		dirSpecSMBIOS[128] = "";
		const char *override_pathname = NULL;
		int			len = 0, err = 0;
	
		// Take in account user overriding
		if (getValueForKey("SMBIOS", &override_pathname, &len, &bootInfo->bootConfig) && len > 0)
		{
			// Specify a path to a file, e.g. SMBIOS=/Extra/macProXY.plist
			sprintf(dirSpecSMBIOS, override_pathname);
			err = loadConfigFile(dirSpecSMBIOS, &bootInfo->smbiosConfig);
		}
		else
		{
			// Check selected volume's Extra.
			sprintf(dirSpecSMBIOS, "/Extra/%s", filename);
			if (err = loadConfigFile(dirSpecSMBIOS, &bootInfo->smbiosConfig))
			{
				// Check booter volume/rdbt Extra.
				sprintf(dirSpecSMBIOS, "bt(0,0)/Extra/%s", filename);
				err = loadConfigFile(dirSpecSMBIOS, &bootInfo->smbiosConfig);
			}
		}

		if (err)
		{
			verbose("No SMBIOS config file found.\n");
		}
		readSmbConfigFile = false;
	}
}

static void setup_Smbios()
{	
	struct SMBEntryPoint *smbios_o = getSmbiosOriginal();	
	smbios_p = (EFI_PTR32)smbios_o; 
	if (smbios_o != NULL) {				
		execute_hook("smbios_helper", (void *)smbios_o, NULL, NULL, NULL, NULL, NULL);
		if (execute_hook("getSmbiosPatched",(void *)smbios_o, NULL, NULL, NULL, NULL, NULL) == 0)
			verbose("Using the original SMBIOS !!\n");		
	}
	
}

struct acpi_common_header * get_ACPI_TABLE(const char * table)
{
	int version;
	struct acpi_common_header *header = NULL;
	for (version=0; version<2; version++) {
		struct acpi_2_rsdp *rsdp=(struct acpi_2_rsdp *)(version?((struct acpi_2_rsdp *)(uint32_t)local_acpi20_p):((struct acpi_2_rsdp *)(uint32_t)local_acpi10_p));
		if (!rsdp)
		{
			DBG("No ACPI version %d found. Ignoring\n", version+1);			
			continue;
		}
		struct acpi_2_rsdt *rsdt=(struct acpi_2_rsdt *)(rsdp->RsdtAddress);
		if (rsdt && (uint32_t)rsdt !=0xffffffff && rsdt->Length<0x10000)
		{
			int i;
			int rsdt_entries_num=(rsdt->Length-sizeof(struct acpi_2_rsdt))/4;
			uint32_t *rsdt_entries=(uint32_t *)(rsdt+1);
			for (i=0;i<rsdt_entries_num;i++)
			{
				header=(struct acpi_common_header *)rsdt_entries[i];				
				if (!header)
					continue;
				if (strcmp(header->Signature, table) == 0)
				{					
					if ((uint32_t)header == 0xffffffff )
					{
						printf("ACPI TABLE (%s) incorrect.\n", table);
						header = NULL;
					}					
					break;				
				}
			}
		}
		if (version)
		{
			struct acpi_2_xsdt *xsdt ;
			xsdt=(struct acpi_2_xsdt*) ((uint32_t)rsdp->XsdtAddress);
			if (xsdt && (uint64_t)rsdp->XsdtAddress<0xffffffff && xsdt->Length<0x10000)
			{
				int i;
				int xsdt_entries_num=(xsdt->Length-sizeof(struct acpi_2_xsdt))/8;
				uint64_t *xsdt_entries=(uint64_t *)(xsdt+1);
				for (i=0;i<xsdt_entries_num;i++)
				{
					header=(struct acpi_common_header *)((uint32_t)(xsdt_entries[i]));
					if (!header)
						continue;
					if (strcmp(header->Signature, table))
					{					
						header=(struct acpi_common_header *)(uint32_t)xsdt_entries[i];
						if (!header || (uint64_t)xsdt_entries[i] >= 0xffffffff)
						{
							printf("ACPIv2+ TABLE (%s) incorrect.\n", table);
							header = NULL;
						}
						break;
					}
				}
			}
		}
	}
	return header;
}

static void setup_machine_signature()
{
	Node *chosenNode = DT__FindNode("/chosen", false);
	if (chosenNode) {
		if (Platform->hardware_signature == 0xFFFFFFFF)
		{
			if (!gFADT) 
				gFADT = (struct acpi_2_fadt *)get_ACPI_TABLE("FACP");
			
			struct acpi_2_facs * facs = 0;
			
			facs = (struct acpi_2_facs *)(uint32_t)gFADT->X_FIRMWARE_CTRL;
			
			if (!facs || strcmp(facs->Signature, "FACS") != 0)
				facs = (struct acpi_2_facs *)gFADT->FIRMWARE_CTRL;
			
			if (!facs || strcmp(facs->Signature, "FACS") != 0)
				Platform->hardware_signature = 0;
			else
				Platform->hardware_signature = facs->hardware_signature;			
			
		}
		
		// Verify that we have a valid hardware signature
		if (Platform->hardware_signature == 0xFFFFFFFF) 
		{
			printf("warning hardware_signature is invalid");
			 Platform->hardware_signature = 0;
		}
		
		DT__AddProperty(chosenNode, "machine-signature", sizeof(uint32_t), &Platform->hardware_signature);
	}
	
}

/*
 * Installs all the needed configuration table entries
 */

static void setupEfiConfigurationTable()
{
	addConfigurationTable(&gEfiSmbiosTableGuid, &smbios_p, NULL);
	
	
	
	struct mp_t *mps_p = getAddressOfMPSTable() ;
	uint64_t mps = (uint32_t)mps_p;	
	
	if (mps_p->config_ptr) {
		
		struct imps_cth *local_cth_ptr
		= (struct imps_cth *)ptov(mps_p->config_ptr);
		
		imps_lapic_addr = local_cth_ptr->lapic_addr;
		
	} else {
		imps_lapic_addr = LAPIC_ADDR_DEFAULT;
	}
	
	addConfigurationTable(&gEfiMpsTableGuid, &mps, NULL);	
	
	
	//Slice
	// PM_Model
	if (platformCPUFeature(CPU_FEATURE_MOBILE)) {	
		Platform->Type = Mobile;
	} else {
		Platform->Type = Desktop;
	}

	// Invalid the platform hardware signature (this needs to be verified with acpica, but i guess that 0xFFFFFFFF is an invalid signature)  	
	Platform->hardware_signature = 0xFFFFFFFF; 
	
	// Setup ACPI with DSDT overrides (mackerintel's patch)
	setup_acpi();
	
	setup_machine_signature();
	// We now have to write the systemm-type in ioregs: we cannot do it before in setupDeviceTree()
	// because we need to take care of facp original content, if it is correct.
	setupSystemType();
	
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
}

/*
 * Entrypoint from boot.c
 */

void setupFakeEfi(void)
{
	// Generate efi device strings 
	setup_pci_devs(root_pci_dev);

	// load smbios.plist file if any
	setupSmbiosConfigFile("SMBIOS.plist");
	setup_Smbios();
	
	// Initialize the base table
	if (archCpuType == CPU_TYPE_I386)
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

