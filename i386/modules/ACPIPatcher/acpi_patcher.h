/*
 * Copyright 2008 mackerintel
 */

/*
 * cparm : cleaned, moved into a module
 */

#ifndef __LIBSAIO_ACPI_PATCHER_H
#define __LIBSAIO_ACPI_PATCHER_H

#include "libsaio.h"
#include "efi.h"

#define kAcpiPatcher		"AcpiPatcher"		/* acpi_patcher.c */ 

#define kDSDT				"DSDT"				/* acpi_patcher.c */
#define kDropSSDT			"DropSSDT"			/* acpi_patcher.c */
#define kRestartFix			"RestartFix"        /* acpi_patcher.c */
#define kGeneratePStates	"GeneratePStates"	/* acpi_patcher.c */
#define kGenerateCStates	"GenerateCStates"	/* acpi_patcher.c */
#define kGenerateFACS		"GenerateFACS"		/* acpi_patcher.c */
#define kMaxRatio			"MaxBusRatio"		/* acpi_patcher.c */
#define kMinRatio			"MinBusRatio"		/* acpi_patcher.c */
#define	kQSSDT				"quickSSDTsearch"	/* acpi_patcher.c */
#define	kSpeedstep			"EnableSpeedStep"	/* acpi_patcher.c */
#define kEnableC4State		"EnableC4State"		/* acpi_patcher.c */

#define kFACS				"FACS"				/* acpi_patcher.c */
#define kSSDT				"SSDT"				/* acpi_patcher.c */
#define kHPET				"HPET"				/* acpi_patcher.c */
#define kSBST				"SBST"				/* acpi_patcher.c */
#define kECDT				"ECDT"				/* acpi_patcher.c */
#define kASFT				"ASFT"				/* acpi_patcher.c */
#define kDMAR				"DMAR"				/* acpi_patcher.c */
#define kFADT				"FADT"				/* acpi_patcher.c */
#define kAPIC				"APIC"				/* acpi_patcher.c */
#define kMCFG				"MCFG"				/* acpi_patcher.c */
#define kOEMDSDT			"oemDSDT"			/* acpi_patcher.c */
#define kOEMSSDT			"oemSSDT"			/* acpi_patcher.c */
#define kOEMHPET			"oemHPET"			/* acpi_patcher.c */
#define kOEMSBST			"oemSBST"			/* acpi_patcher.c */
#define kOEMECDT			"oemECDT"			/* acpi_patcher.c */
#define kOEMASFT			"oemASFT"			/* acpi_patcher.c */
#define kOEMDMAR			"oemDMAR"			/* acpi_patcher.c */
#define kOEMFADT			"oemFADT"			/* acpi_patcher.c */
#define kOEMAPIC			"oemAPIC"			/* acpi_patcher.c */
#define kOEMMCFG			"oemMCFG"			/* acpi_patcher.c */
#define kOEMFACS			"oemFACS"			/* acpi_patcher.c */

#define kUpdateACPI			"UpdateACPI"	    /* acpi_patcher.c */

#define kEnableASPM			"EnableASPM"		/* acpi_patcher.c */

extern int setupAcpi();

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
