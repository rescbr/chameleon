/*
 * Copyright 2008 mackerintel
 * 2010 mojodojo
 */

#ifndef __LIBSAIO_ACPI_PATCHER_H
#define __LIBSAIO_ACPI_PATCHER_H

#include "libsaio.h"

// extern uint64_t smbios_p; - moved to smbios.h

boolean_t tableSign(void *table, const char *sgn);
extern struct acpi_2_rsdp *getRSDPaddress();

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
