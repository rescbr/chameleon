/*
 * Copyright 2008 mackerintel
 */

#ifndef __LIBSAIO_ACPI_PATCHER_H
#define __LIBSAIO_ACPI_PATCHER_H

//Azi:include
#include "libsaio.h"
#include "ramdisk.h" //Azi:searchalgo - for dsdt_patcher.c & fake_efi.c

/*Azi: "AsereBLN: this is bullsh*t... declaring vars in a header"
uint64_t acpi10_p;
uint64_t acpi20_p;
uint64_t smbios_p;*/

extern int setupAcpi();

extern EFI_STATUS addConfigurationTable();

extern EFI_GUID gEfiAcpiTableGuid;
extern EFI_GUID gEfiAcpi20TableGuid;

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
