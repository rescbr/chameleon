/*
 * Copyright 2008 mackerintel
 * 2010 mojodojo
 */

#ifndef __LIBSAIO_ACPI_PATCHER_H
#define __LIBSAIO_ACPI_PATCHER_H

#include "libsaio.h"

/* From Foundation/Efi/Guid/Acpi/Acpi.h */
/* Modified to wrap Data4 array init with {} */
#define EFI_ACPI_TABLE_GUID     {0xeb9d2d30, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d}}
#define EFI_ACPI_20_TABLE_GUID  {0x8868e871, 0xe4f1, 0x11d3, {0xbc, 0x22, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81}}
extern EFI_GUID const gEfiAcpiTableGuid;
extern EFI_GUID const gEfiAcpi20TableGuid;

extern uint64_t acpi10_p;
extern uint64_t acpi20_p;
// extern uint64_t smbios_p; - moved to smbios.h

boolean_t tableSign(void *table, const char *sgn);
struct acpi_2_rsdp *getRSDPaddress();

extern int setupAcpi();
// extern EFI_STATUS addConfigurationTable();

struct p_state 
{
	union 
	{
		uint16_t Control;
		struct 
		{
			uint8_t VID;	// Voltage ID
			uint8_t FID;	// Frequency ID
		};
	};
	
	uint8_t		CID;		// Compare ID
	uint32_t	Frequency;
};

#endif /* !__LIBSAIO_ACPI_PATCHER_H */
