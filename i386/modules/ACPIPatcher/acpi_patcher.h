/*
 * Copyright 2008 mackerintel
 */

/*
 * Copyright (c) 2010 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#ifndef __LIBSAIO_ACPI_PATCHER_H
#define __LIBSAIO_ACPI_PATCHER_H

#include "libsaio.h"
#include "efi.h"

#define kDSDT				"DSDT"				/* acpi_patcher.c */
#define kRestartFix			"RestartFix"        /* acpi_patcher.c */
#define kGeneratePStates	"GeneratePStates"	/* acpi_patcher.c */
#define kGenerateCStates	"GenerateCStates"	/* acpi_patcher.c */
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

#define kUpdateACPI			"UpdateACPI"	    /* acpi_patcher.c */

#define kDisableASPM		"DisableASPM"		/* acpi_patcher.c */
#define kDisableMSI			"DisableMSI"		/* acpi_patcher.c */

#if UNUSED
#define kGenerateFACS		"GenerateFACS"		/* acpi_patcher.c */
#define kOEMFACS			"oemFACS"			/* acpi_patcher.c */
#endif

extern EFI_STATUS setupAcpi(void);

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

/*
 * SIGNATURE_16, SIGNATURE_32, SIGNATURE_64 are extracted from the edk2 project (Base.h), and are under the following license:
 *
 * Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.
 * Portions copyright (c) 2008 - 2009, Apple Inc. All rights reserved.
 * This program and the accompanying materials are licensed and made available under the terms and conditions of the BSD License which accompanies this distribution. The full text of the license may be found at http://opensource.org/licenses/bsd-license.php.
 * THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
 */

#define SIGNATURE_16(A, B)        ((A) | (B << 8))

#define SIGNATURE_32(A, B, C, D)  (SIGNATURE_16 (A, B) | (SIGNATURE_16 (C, D) << 16))

#define SIGNATURE_64(A, B, C, D, E, F, G, H) \
(SIGNATURE_32 (A, B, C, D) | ((uint64_t) (SIGNATURE_32 (E, F, G, H)) << 32))

#define ACPI_SIGNATURE_UINT64_LE SIGNATURE_64('R','S','D',' ','P','T','R',' ')


/* Per ACPI 3.0a spec */

// TODO Migrate
struct acpi_2_rsdp {
    char            Signature[8];
    uint8_t         Checksum;
    char            OEMID[6];
    uint8_t         Revision;
    uint32_t        RsdtAddress;
    uint32_t        Length;
    uint64_t        XsdtAddress;
    uint8_t         ExtendedChecksum;
    char            Reserved[3];
} __attribute__((packed));


#define ACPI_HEADER_CORE			\
char            Signature[4];	\
uint32_t        Length;			\
uint8_t         Revision;		\
uint8_t         Checksum;		\
char            OEMID[6];		\
char            OEMTableId[8];	\
uint32_t        OEMRevision;	\
uint32_t        CreatorId;		\
uint32_t        CreatorRevision;

struct acpi_common_header {
	ACPI_HEADER_CORE	
} __attribute__((packed));

// TODO Migrate
struct acpi_2_rsdt {
	ACPI_HEADER_CORE	
} __attribute__((packed));

// TODO Migrate
struct acpi_2_xsdt {
	ACPI_HEADER_CORE	
} __attribute__((packed));

// TODO Migrate
struct acpi_2_gas {
	uint8_t				Address_Space_ID;
	uint8_t				Register_Bit_Width;
	uint8_t				Register_Bit_Offset;
	uint8_t				Access_Size;
	uint64_t			Address;
} __attribute__((packed));

// TODO Migrate
struct acpi_2_ssdt {
	ACPI_HEADER_CORE	
} __attribute__((packed));

// TODO Migrate
struct acpi_2_dsdt {
	ACPI_HEADER_CORE	
} __attribute__((packed));

// TODO Migrate
struct acpi_2_fadt {
	ACPI_HEADER_CORE	
	uint32_t        FIRMWARE_CTRL;
	uint32_t        DSDT;
	uint8_t         Model;			// JrCs
	uint8_t         PM_Profile;		// JrCs
	uint16_t		SCI_Interrupt;
	uint32_t		SMI_Command_Port;
	uint8_t			ACPI_Enable;
	uint8_t			ACPI_Disable;
	uint8_t			S4BIOS_Command;
	uint8_t			PState_Control;
	uint32_t		PM1A_Event_Block_Address;
	uint32_t		PM1B_Event_Block_Address;
	uint32_t		PM1A_Control_Block_Address;
	uint32_t		PM1B_Control_Block_Address;
	uint32_t		PM2_Control_Block_Address;
	uint32_t		PM_Timer_Block_Address;
	uint32_t		GPE0_Block_Address;
	uint32_t		GPE1_Block_Address;
	uint8_t			PM1_Event_Block_Length;
	uint8_t			PM1_Control_Block_Length;
	uint8_t			PM2_Control_Block_Length;
	uint8_t			PM_Timer_Block_Length;
	uint8_t			GPE0_Block_Length;
	uint8_t			GPE1_Block_Length;
	uint8_t			GPE1_Base_Offset;
	uint8_t			CST_Support;
	uint16_t		C2_Latency;
	uint16_t		C3_Latency;
	uint16_t		CPU_Cache_Size;
	uint16_t		Cache_Flush_Stride;
	uint8_t			Duty_Cycle_Offset;
	uint8_t			Duty_Cycle_Width;
	uint8_t			RTC_Day_Alarm_Index;
	uint8_t			RTC_Month_Alarm_Index;
	uint8_t			RTC_Century_Index;
	uint16_t		Boot_Flags;
	uint8_t			Reserved0;
    /* Begin Asere */
	//Reset Fix
	uint32_t			Flags;
	struct acpi_2_gas	RESET_REG;	
	uint8_t				Reset_Value;
	uint8_t				Reserved[3];
    
	uint64_t			X_FIRMWARE_CTRL;
	uint64_t			X_DSDT;
    
#if UNUSED
	/* End Asere */
	/*We absolutely don't care about theese fields*/
	uint8_t		notimp2[96];
#else
	struct acpi_2_gas	X_PM1a_EVT_BLK;
	struct acpi_2_gas	X_PM1b_EVT_BLK;
	struct acpi_2_gas	X_PM1a_CNT_BLK;
	struct acpi_2_gas	X_PM1b_CNT_BLK;
	struct acpi_2_gas	X_PM2_CNT_BLK;
	struct acpi_2_gas	X_PM_TMR_BLK;
	struct acpi_2_gas	X_GPE0_BLK;
	struct acpi_2_gas	X_GPE1_BLK;
#endif
	
} __attribute__((packed));

struct acpi_2_facs {
	char			Signature[4];	
	uint32_t        Length;			
	uint32_t		hardware_signature;
	uint32_t		firmware_waking_vector;
	uint32_t		global_lock;
	uint32_t		flags;
	uint64_t		x_firmware_waking_vector;	
	uint8_t			version;
	uint8_t			Reserved[3];
	uint32_t		ospm_flags;
	uint8_t			Reserved1[24];
} __attribute__ ((packed));

#endif /* !__LIBSAIO_ACPI_PATCHER_H */
