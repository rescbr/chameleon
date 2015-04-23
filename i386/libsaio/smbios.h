/*
 * Copyright (c) 1998-2009 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __LIBSAIO_SMBIOS_H
#define __LIBSAIO_SMBIOS_H


/* Based on System Management BIOS Reference Specification v2.4 */
// http://dmtf.org/standards/smbios

typedef uint8_t  SMBString;
typedef uint8_t  SMBByte;
typedef uint16_t SMBWord;
typedef uint32_t SMBDWord;
typedef uint64_t SMBQWord;


typedef struct DMIEntryPoint
{
    SMBByte    anchor[5];
    SMBByte    checksum;
    SMBWord    tableLength;
    SMBDWord   tableAddress;
    SMBWord    structureCount;
    SMBByte    bcdRevision;
} __attribute__((packed)) DMIEntryPoint;

typedef struct SMBEntryPoint
{
    SMBByte    anchor[4];
    SMBByte    checksum;
    SMBByte    entryPointLength;
    SMBByte    majorVersion;
    SMBByte    minorVersion;
    SMBWord    maxStructureSize;
    SMBByte    entryPointRevision;
    SMBByte    formattedArea[5];
    DMIEntryPoint dmi;
} __attribute__((packed)) SMBEntryPoint;

/* Header common to all SMBIOS structures */
typedef struct SMBStructHeader
{
	SMBByte	type;
	SMBByte	length;
	SMBWord	handle;
//	SMBByte	*data;
} __attribute__((packed)) SMBStructHeader;

#define SMB_STRUCT_HEADER  SMBStructHeader header;

typedef struct SMBAnchor
{
	const SMBStructHeader *	header;
	const uint8_t *			next;
	const uint8_t *			end;
} SMBAnchor;

#define SMB_ANCHOR_IS_VALID(x)	\
	((x) && ((x)->header) && ((x)->next) && ((x)->end))

#define SMB_ANCHOR_RESET(x)		\
	bzero(x, sizeof(typedef struct SMBAnchor));

/* ======================= SMBIOS structure types. ======================= */
enum
{
	kSMBTypeBIOSInformation			=  0, // BIOS information (Type 0)
	kSMBTypeSystemInformation		=  1, // System Information (Type 1)
	kSMBTypeBaseBoard               =  2, // BaseBoard Information (Type 2)
	kSMBTypeSystemEnclosure			=  3, // System Chassis Information (Type 3)
	kSMBTypeProcessorInformation	=  4, // Processor Information (Type 4)
	// Memory Controller Information (Type 5) Obsolete
	kSMBTypeMemoryModule			=  6, // Memory Module Information (Type 6) Obsolete
	kSMBTypeCacheInformation		=  7, // Cache Information (Type 7)
	// Port Connector Information (Type 8)
	kSMBTypeSystemSlot              =  9, // System Slots (Type 9)
	// On Board Devices Information (Type 10) Obsolete
	kSMBOEMStrings                  =  11 ,// OEM Strings (Type 11)
	// System Configuration Options (Type 12)
	// BIOS Language Information (Type 13)
	// Group Associations (Type 14)
	// System Event Log (Type 15)
	kSMBTypePhysicalMemoryArray		=  16, // Physical Memory Array (Type 16)
	kSMBTypeMemoryDevice			=  17, // Memory Device (Type 17)
	kSMBType32BitMemoryErrorInfo		=  18, // 32-Bit Memory Error Information (Type 18)
	// Memory Array Mapped Address (Type 19)
	// Memory Device Mapped Address (Type 20)
	// Built-in Pointing Device (Type 21)
	// Portable Battery (Type 22)
	// System Reset (Type 23)
	// Hardware Security (Type 24)
	// System Power Controls (Type 25)
	// Voltage Probe (Type 26)
	// Cooling Device (Type 27)
	// Temperature Probe (Type 28)
	// Electrical Current Probe (Type 29)
	// Out-of-Band Remote Access (Type 30)
	// Boot Integrity Service (BIS) Entry Point (Type 31)
	// System Boot Information (Type 32)
	kSMBType64BitMemoryErrorInfo       =  33, // 64-Bit Memory Error Information (Type 33)
	// Managment Device (Type 34)
	// Managment Device Component (Type 35)
	// Management Device Threshold Data (Type 36)
	// Memory Channel (Type 37)
	// IPMI Device Information (Type 38)
	// System Power Supply (Type 39)
	// Additional Information (Type 40)
	// Onboard Devices Extended Information (Type 41)
	// Management Controlle Host Interface (Type 42)

	// Inactive (Type 126)
	kSMBTypeEndOfTable              =  127, // End-of-Table (Type 127)

	// Apple Specific Structures
	kSMBTypeFirmwareVolume			=  128, // FirmwareVolume (TYPE 128)
	kSMBTypeMemorySPD               =  130, // MemorySPD (TYPE 130)
	kSMBTypeOemProcessorType		=  131, // Processor Type (Type 131)
	kSMBTypeOemProcessorBusSpeed    =  132, // Processor Bus Speed (Type 132)
	kSMBTypeOemPlatformFeature		=  133 // Platform Feature (Type 133)
};

//----------------------------------------------------------------------------------------------------------
// Struct - BIOS Information (Type 0)
typedef struct SMBBIOSInformation
{
    SMB_STRUCT_HEADER
    SMBString  vendor;              // BIOS vendor name
    SMBString  version;             // BIOS version
    SMBWord    startSegment;        // BIOS segment start
    SMBString  releaseDate;         // BIOS release date
    SMBByte    romSize;             // BIOS ROM Size (n); 64K * (n+1) bytes
    SMBQWord   characteristics;     // supported BIOS functions
    // Bungo
    SMBByte    characteristicsExt1; // BIOS characteristics extension byte 1
    SMBByte    characteristicsExt2; // BIOS characteristics extension byte 2
    SMBByte    releaseMajor;        // BIOS release (major)
    SMBByte    releaseMinor;        // BIOS release (minor)
    SMBByte    ECreleaseMajor;      // Embedded Controller firmware release (major)
    SMBByte    ECreleaseMinor;      // Embedded Controller firmware release (minor)
} __attribute__((packed)) SMBBIOSInformation;

//----------------------------------------------------------------------------------------------------------
// Struct - System Information (Type 1)
typedef struct SMBSystemInformation
{
    // 2.0+ spec (8 bytes)
    SMB_STRUCT_HEADER
    SMBString  manufacturer;
    SMBString  productName;
    SMBString  version;
    SMBString  serialNumber;
    // 2.1+ spec (25 bytes)
    SMBByte    uuid[16];            // can be all 0 or all 1's
    SMBByte    wakeupReason;        // reason for system wakeup
    // 2.4+ spec (27 bytes)
    SMBString  skuNumber;
    SMBString  family;
} __attribute__((packed)) SMBSystemInformation;

//----------------------------------------------------------------------------------------------------------
// Base Board (or Module) Information (Type 2)

// Base Board - Board Type.
// Values for SMBBaseBoard.boardType
typedef enum
{
    kSMBBaseBoardUnknown               = 0x01,	// Unknow
    kSMBBaseBoardOther                 = 0x02,	// Other
    kSMBBaseBoardServerBlade           = 0x03,	// Server Blade
    kSMBBaseBoardConnectivitySwitch    = 0x04,	// Connectivity Switch
    kSMBBaseBoardSystemMgmtModule      = 0x05,	// System Management Module
    kSMBBaseBoardProcessorModule       = 0x06,	// Processor Module
    kSMBBaseBoardIOModule              = 0x07,	// I/O Module
    kSMBBaseBoardMemoryModule          = 0x08,	// Memory Module
    kSMBBaseBoardDaughter              = 0x09,	// Daughter Board
    kSMBBaseBoardMotherboard           = 0x0A,	// Motherboard (includes processor, memory, and I/O)
    kSMBBaseBoardProcessorMemoryModule = 0x0B,	// Processor/Memory Module
    kSMBBaseBoardProcessorIOModule     = 0x0C,	// Processor/IO Module
    kSMBBaseBoardInterconnect          = 0x0D	// Interconnect board
} BASE_BOARD_TYPE;

// Struct - Base Board (or Module) Information (Type 2)
typedef struct SMBBaseBoard
{
	SMB_STRUCT_HEADER               // Type 2
	SMBString	manufacturer;
	SMBString	product;
	SMBString	version;
	SMBString	serialNumber;
	SMBString	assetTag;			// Bungo: renamed from assetTagNumber folowing convention
	SMBByte     featureFlags;			// Collection of flag that identify features of this baseboard
	SMBString	locationInChassis;
	SMBWord     chassisHandle;
	SMBByte     boardType;				// Type of board, numeration value from BASE_BOARD_TYPE.
	SMBByte     numberOfContainedHandles;
	// 0 - 255 contained handles go here but we do not include
	// them in our structure. Be careful to use numberOfContainedHandles
	// times sizeof(SMBWord) when computing the actual record size,
	// if you need it.
    SMBByte     containedObjectHandles;
} __attribute__((packed)) SMBBaseBoard;

//----------------------------------------------------------------------------------------------------------
// System Enclosure (Type 3)

// Bungo: values for SMBSystemEnclosure.chassisType
typedef enum {
    kSMBchassisOther                    = 0x01,
    kSMBchassisUnknown                  = 0x02,
    kSMBchassisDesktop                  = 0x03,
    kSMBchassisLPDesktop                = 0x04,
    kSMBchassisPizzaBox                 = 0x05,
    kSMBchassisMiniTower                = 0x06,
    kSMBchassisTower                    = 0x07,
    kSMBchassisPortable                 = 0x08,
    kSMBchassisLaptop                   = 0x09,
    kSMBchassisNotebook                 = 0x0A,
    kSMBchassisHandHeld                 = 0x0B,
    kSMBchassisDockingStation           = 0x0C,
    kSMBchassisAllInOne                 = 0x0D,
    kSMBchassisSubNotebook              = 0x0E,
    // ... fill up if needed ;-)
    kSMBchassisLunchBox                 = 0x10,
    // ... fill up if needed ;-)
    kSMBchassisBladeEnclosing           = 0x1D
} MISC_CHASSIS_TYPE;

// System Enclosure or Chassis States.
// values for SMBSystemEnclosure.bootupState
// values for SMBSystemEnclosure.powerSupplyState
// values for SMBSystemEnclosure.thermalState
typedef enum {
    kSMBChassisStateOther           = 0x01,
    kSMBChassisStateUnknown         = 0x02,
    kSMBChassisStateSafe            = 0x03,
    kSMBChassisStateWarning         = 0x04,
    kSMBChassisStateCritical        = 0x05,
    kSMBChassisStateNonRecoverable  = 0x06
} MISC_CHASSIS_STATE;

// System Enclosure or Chassis Security Status.
// values for SMBSystemEnclosure.securityStatus
typedef enum {
    kSMBChassisSecurityStatusOther                          = 0x01,
    kSMBChassisSecurityStatusUnknown                        = 0x02,
    kSMBChassisSecurityStatusNone                           = 0x03,
    kSMBChassisSecurityStatusExternalInterfaceLockedOut     = 0x04,
    kSMBChassisSecurityStatusExternalInterfaceLockedEnabled = 0x05
} MISC_CHASSIS_SECURITY_STATE;

// Struct - System Enclosure (Type 3)
typedef struct SMBSystemEnclosure
{
    SMB_STRUCT_HEADER               // Type 3
    SMBString  manufacturer;
    SMBByte    chassisType;		// System Enclosure Indicator
    SMBString  version;		// Board Number?
    SMBString  serialNumber;
    SMBString  assetTag;		// Bungo: renamed from assetTagNumber folowing convention
    SMBByte    bootupState;		// State of enclosure when when it was last booted
    SMBByte    powerSupplyState;	// State of enclosure's power supply when last booted
    SMBByte    thermalState;	// Thermal state of the enclosure when last booted
    SMBByte    securityStatus;	// Physical security status of the enclosure when last booted
    SMBDWord   oemDefined;		// OEM- or BIOS vendor-specific information
    SMBByte    height;		// Height of the enclosure, in 'U's
    SMBByte    numberOfPowerCords;	// Number of power cords associated with the enclosure or chassis
    SMBByte    containedElementCount;	// Number of Contained Element record that follow, in the range 0 to 255
    //	SMBByte    containedElementRecord;	// Byte leght of each Contained Element record that follow, in the range 0 to 255
    //	SMBByte    containedElements;	// Elements, possibly defined by other SMBIOS structures present in chassis
    //	SMBString  skuNumber;		// Number of null-terminated string describing the chassis or enclosure SKU number (2.7+)
} __attribute__((packed)) SMBSystemEnclosure;

//----------------------------------------------------------------------------------------------------------
// Processor Information (Type 4)
#define kSMBProcessorInformationMinSize     26

// Processor Information - Processor Type.
// Values for SMBProcessorInformation.processorType
typedef enum
{
    kSMBprocessorTypeOther          = 0x01,
    kSMBprocessorTypeUnknown        = 0x02,
    kSMBprocessorTypeCPU            = 0x03,
    kSMBprocessorTypeMPU            = 0x04,
    kSMBprocessorTypeDSP            = 0x05,
    kSMBprocessorTypeGPU            = 0x06
} PROCESSOR_TYPE_DATA;

// Processor Information - Processor Family.
// Values for SMBProcessorInformation.processorFamily
typedef enum {
    kSMBprocessorFamilyOther                  = 0x01,
    kSMBprocessorFamilyUnknown                = 0x02,
    kSMBprocessorFamily8086                   = 0x03,
    kSMBprocessorFamily80286                  = 0x04,
    kSMBprocessorFamilyIntel386               = 0x05,
    kSMBprocessorFamilyIntel486               = 0x06,
    kSMBprocessorFamily8087                   = 0x07,
    kSMBprocessorFamily80287                  = 0x08,
    kSMBprocessorFamily80387                  = 0x09,
    kSMBprocessorFamily80487                  = 0x0A,
    kSMBprocessorFamilyPentium                = 0x0B,
    kSMBprocessorFamilyPentiumPro             = 0x0C,
    kSMBprocessorFamilyPentiumII              = 0x0D,
    kSMBprocessorFamilyPentiumMMX             = 0x0E,
    kSMBprocessorFamilyCeleron                = 0x0F,
    kSMBprocessorFamilyPentiumIIXeon          = 0x10,
    kSMBprocessorFamilyPentiumIII             = 0x11,
    kSMBprocessorFamilyM1                     = 0x12,
    kSMBprocessorFamilyM2                     = 0x13,
    kSMBprocessorFamilyIntelCeleronM          = 0x14,
    kSMBprocessorFamilyIntelPentium4Ht        = 0x15,
    kSMBprocessorFamilyM1Reserved4            = 0x16,
    kSMBprocessorFamilyM1Reserved5            = 0x17,
    kSMBprocessorFamilyAmdDuron               = 0x18,
    kSMBprocessorFamilyK5                     = 0x19,
    kSMBprocessorFamilyK6                     = 0x1A,
    kSMBprocessorFamilyK6_2                   = 0x1B,
    kSMBprocessorFamilyK6_3                   = 0x1C,
    kSMBprocessorFamilyAmdAthlon              = 0x1D,
    kSMBprocessorFamilyAmd29000               = 0x1E,
    kSMBprocessorFamilyK6_2Plus               = 0x1F,
    kSMBprocessorFamilyPowerPC                = 0x20,
    kSMBprocessorFamilyPowerPC601             = 0x21,
    kSMBprocessorFamilyPowerPC603             = 0x22,
    kSMBprocessorFamilyPowerPC603Plus         = 0x23,
    kSMBprocessorFamilyPowerPC604             = 0x24,
    kSMBprocessorFamilyPowerPC620             = 0x25,
    kSMBprocessorFamilyPowerPCx704            = 0x26,
    kSMBprocessorFamilyPowerPC750             = 0x27,
    kSMBprocessorFamilyIntelCoreDuo           = 0x28,
    kSMBprocessorFamilyIntelCoreDuoMobile     = 0x29,
    kSMBprocessorFamilyIntelCoreSoloMobile    = 0x2A,
    kSMBprocessorFamilyIntelAtom              = 0x2B,
    kSMBprocessorFamilyAlpha3                 = 0x30,
    kSMBprocessorFamilyAlpha21064             = 0x31,
    kSMBprocessorFamilyAlpha21066             = 0x32,
    kSMBprocessorFamilyAlpha21164             = 0x33,
    kSMBprocessorFamilyAlpha21164PC           = 0x34,
    kSMBprocessorFamilyAlpha21164a            = 0x35,
    kSMBprocessorFamilyAlpha21264             = 0x36,
    kSMBprocessorFamilyAlpha21364             = 0x37,
    kSMBprocessorFamilyAmdTurionIIUltraDualCoreMobileM    = 0x38,
    kSMBprocessorFamilyAmdTurionIIDualCoreMobileM         = 0x39,
    kSMBprocessorFamilyAmdAthlonIIDualCoreM   = 0x3A,
    kSMBprocessorFamilyAmdOpteron6100Series   = 0x3B,
    kSMBprocessorFamilyAmdOpteron4100Series   = 0x3C,
    kSMBprocessorFamilyAmdOpteron6200Series   = 0x3D,
    kSMBprocessorFamilyAmdOpteron4200Series   = 0x3E,
    kSMBprocessorFamilyMips                   = 0x40,
    kSMBprocessorFamilyMIPSR4000              = 0x41,
    kSMBprocessorFamilyMIPSR4200              = 0x42,
    kSMBprocessorFamilyMIPSR4400              = 0x43,
    kSMBprocessorFamilyMIPSR4600              = 0x44,
    kSMBprocessorFamilyMIPSR10000             = 0x45,
    kSMBprocessorFamilyAmdCSeries             = 0x46,
    kSMBprocessorFamilyAmdESeries             = 0x47,
    kSMBprocessorFamilyAmdSSeries             = 0x48,
    kSMBprocessorFamilyAmdGSeries             = 0x49,
    kSMBprocessorFamilySparc                  = 0x50,
    kSMBprocessorFamilySuperSparc             = 0x51,
    kSMBprocessorFamilymicroSparcII           = 0x52,
    kSMBprocessorFamilymicroSparcIIep         = 0x53,
    kSMBprocessorFamilyUltraSparc             = 0x54,
    kSMBprocessorFamilyUltraSparcII           = 0x55,
    kSMBprocessorFamilyUltraSparcIIi          = 0x56,
    kSMBprocessorFamilyUltraSparcIII          = 0x57,
    kSMBprocessorFamilyUltraSparcIIIi         = 0x58,
    kSMBprocessorFamily68040                  = 0x60,
    kSMBprocessorFamily68xxx                  = 0x61,
    kSMBprocessorFamily68000                  = 0x62,
    kSMBprocessorFamily68010                  = 0x63,
    kSMBprocessorFamily68020                  = 0x64,
    kSMBprocessorFamily68030                  = 0x65,
    kSMBprocessorFamilyHobbit                 = 0x70,
    kSMBprocessorFamilyCrusoeTM5000           = 0x78,
    kSMBprocessorFamilyCrusoeTM3000           = 0x79,
    kSMBprocessorFamilyEfficeonTM8000         = 0x7A,
    kSMBprocessorFamilyWeitek                 = 0x80,
    kSMBprocessorFamilyItanium                = 0x82,
    kSMBprocessorFamilyAmdAthlon64            = 0x83,
    kSMBprocessorFamilyAmdOpteron             = 0x84,
    kSMBprocessorFamilyAmdSempron             = 0x85,
    kSMBprocessorFamilyAmdTurion64Mobile      = 0x86,
    kSMBprocessorFamilyDualCoreAmdOpteron     = 0x87,
    kSMBprocessorFamilyAmdAthlon64X2DualCore  = 0x88,
    kSMBprocessorFamilyAmdTurion64X2Mobile    = 0x89,
    kSMBprocessorFamilyQuadCoreAmdOpteron     = 0x8A,
    kSMBprocessorFamilyThirdGenerationAmdOpteron = 0x8B,
    kSMBprocessorFamilyAmdPhenomFxQuadCore    = 0x8C,
    kSMBprocessorFamilyAmdPhenomX4QuadCore    = 0x8D,
    kSMBprocessorFamilyAmdPhenomX2DualCore    = 0x8E,
    kSMBprocessorFamilyAmdAthlonX2DualCore    = 0x8F,
    kSMBprocessorFamilyPARISC                 = 0x90,
    kSMBprocessorFamilyPaRisc8500             = 0x91,
    kSMBprocessorFamilyPaRisc8000             = 0x92,
    kSMBprocessorFamilyPaRisc7300LC           = 0x93,
    kSMBprocessorFamilyPaRisc7200             = 0x94,
    kSMBprocessorFamilyPaRisc7100LC           = 0x95,
    kSMBprocessorFamilyPaRisc7100             = 0x96,
    kSMBprocessorFamilyV30                    = 0xA0,
    kSMBprocessorFamilyQuadCoreIntelXeon3200Series  = 0xA1,
    kSMBprocessorFamilyDualCoreIntelXeon3000Series  = 0xA2,
    kSMBprocessorFamilyQuadCoreIntelXeon5300Series  = 0xA3,
    kSMBprocessorFamilyDualCoreIntelXeon5100Series  = 0xA4,
    kSMBprocessorFamilyDualCoreIntelXeon5000Series  = 0xA5,
    kSMBprocessorFamilyDualCoreIntelXeonLV          = 0xA6,
    kSMBprocessorFamilyDualCoreIntelXeonULV         = 0xA7,
    kSMBprocessorFamilyDualCoreIntelXeon7100Series  = 0xA8,
    kSMBprocessorFamilyQuadCoreIntelXeon5400Series  = 0xA9,
    kSMBprocessorFamilyQuadCoreIntelXeon            = 0xAA,
    kSMBprocessorFamilyDualCoreIntelXeon5200Series  = 0xAB,
    kSMBprocessorFamilyDualCoreIntelXeon7200Series  = 0xAC,
    kSMBprocessorFamilyQuadCoreIntelXeon7300Series  = 0xAD,
    kSMBprocessorFamilyQuadCoreIntelXeon7400Series  = 0xAE,
    kSMBprocessorFamilyMultiCoreIntelXeon7400Series = 0xAF,
    kSMBprocessorFamilyPentiumIIIXeon         = 0xB0,
    kSMBprocessorFamilyPentiumIIISpeedStep    = 0xB1,
    kSMBprocessorFamilyPentium4               = 0xB2,
    kSMBprocessorFamilyIntelXeon              = 0xB3,
    kSMBprocessorFamilyAS400                  = 0xB4,
    kSMBprocessorFamilyIntelXeonMP            = 0xB5,
    kSMBprocessorFamilyAMDAthlonXP            = 0xB6,
    kSMBprocessorFamilyAMDAthlonMP            = 0xB7,
    kSMBprocessorFamilyIntelItanium2          = 0xB8,
    kSMBprocessorFamilyIntelPentiumM          = 0xB9,
    kSMBprocessorFamilyIntelCeleronD          = 0xBA,
    kSMBprocessorFamilyIntelPentiumD          = 0xBB,
    kSMBprocessorFamilyIntelPentiumEx         = 0xBC,
    kSMBprocessorFamilyIntelCoreSolo          = 0xBD,  ///< SMBIOS spec 2.6 correct this value
    kSMBprocessorFamilyReserved               = 0xBE,
    kSMBprocessorFamilyIntelCore2             = 0xBF,
    kSMBprocessorFamilyIntelCore2Solo         = 0xC0,
    kSMBprocessorFamilyIntelCore2Extreme      = 0xC1,
    kSMBprocessorFamilyIntelCore2Quad         = 0xC2,
    kSMBprocessorFamilyIntelCore2ExtremeMobile = 0xC3,
    kSMBprocessorFamilyIntelCore2DuoMobile    = 0xC4,
    kSMBprocessorFamilyIntelCore2SoloMobile   = 0xC5,
    kSMBprocessorFamilyIntelCoreI7            = 0xC6,
    kSMBprocessorFamilyDualCoreIntelCeleron   = 0xC7,
    kSMBprocessorFamilyIBM390                 = 0xC8,
    kSMBprocessorFamilyG4                     = 0xC9,
    kSMBprocessorFamilyG5                     = 0xCA,
    kSMBprocessorFamilyG6                     = 0xCB,
    kSMBprocessorFamilyzArchitectur           = 0xCC,
    kSMBprocessorFamilyIntelCoreI5            = 0xCD,
    kSMBprocessorFamilyIntelCoreI3            = 0xCE,
    kSMBprocessorFamilyViaC7M                 = 0xD2,
    kSMBprocessorFamilyViaC7D                 = 0xD3,
    kSMBprocessorFamilyViaC7                  = 0xD4,
    kSMBprocessorFamilyViaEden                = 0xD5,
    kSMBprocessorFamilyMultiCoreIntelXeon           = 0xD6,
    kSMBprocessorFamilyDualCoreIntelXeon3Series     = 0xD7,
    kSMBprocessorFamilyQuadCoreIntelXeon3Series     = 0xD8,
    kSMBprocessorFamilyViaNano                      = 0xD9,
    kSMBprocessorFamilyDualCoreIntelXeon5Series     = 0xDA,
    kSMBprocessorFamilyQuadCoreIntelXeon5Series     = 0xDB,
    kSMBprocessorFamilyDualCoreIntelXeon7Series     = 0xDD,
    kSMBprocessorFamilyQuadCoreIntelXeon7Series     = 0xDE,
    kSMBprocessorFamilyMultiCoreIntelXeon7Series    = 0xDF,
    kSMBprocessorFamilyMultiCoreIntelXeon3400Series = 0xE0,
    kSMBprocessorFamilyEmbeddedAmdOpteronQuadCore   = 0xE6,
    kSMBprocessorFamilyAmdPhenomTripleCore          = 0xE7,
    kSMBprocessorFamilyAmdTurionUltraDualCoreMobile = 0xE8,
    kSMBprocessorFamilyAmdTurionDualCoreMobile      = 0xE9,
    kSMBprocessorFamilyAmdAthlonDualCore            = 0xEA,
    kSMBprocessorFamilyAmdSempronSI                 = 0xEB,
    kSMBprocessorFamilyAmdPhenomII                  = 0xEC,
    kSMBprocessorFamilyAmdAthlonII                  = 0xED,
    kSMBprocessorFamilySixCoreAmdOpteron            = 0xEE,
    kSMBprocessorFamilyAmdSempronM                  = 0xEF,
    kSMBprocessorFamilyi860                   = 0xFA,
    kSMBprocessorFamilyi960                   = 0xFB,
    kSMBprocessorFamilyIndicatorFamily2       = 0xFE,
    kSMBprocessorFamilyReserved1              = 0xFF
} PROCESSOR_FAMILY_DATA;

// Processor Information - Processor Upgrade.
// Values for SMBProcessorInformation.processorUpgrade
typedef enum {
    kSMBprocessorUpgradeOther         = 0x01,
    kSMBprocessorUpgradeUnknown       = 0x02,
    kSMBprocessorUpgradeDaughterBoard = 0x03,
    kSMBprocessorUpgradeZIFSocket     = 0x04,
    kSMBprocessorUpgradePiggyBack     = 0x05, ///< Replaceable.
    kSMBprocessorUpgradeNone          = 0x06,
    kSMBprocessorUpgradeLIFSocket     = 0x07,
    kSMBprocessorUpgradeSlot1         = 0x08,
    kSMBprocessorUpgradeSlot2         = 0x09,
    kSMBprocessorUpgrade370PinSocket  = 0x0A,
    kSMBprocessorUpgradeSlotA         = 0x0B,
    kSMBprocessorUpgradeSlotM         = 0x0C,
    kSMBprocessorUpgradeSocket423     = 0x0D,
    kSMBprocessorUpgradeSocketA       = 0x0E, ///< Socket 462.
    kSMBprocessorUpgradeSocket478     = 0x0F,
    kSMBprocessorUpgradeSocket754     = 0x10,
    kSMBprocessorUpgradeSocket940     = 0x11,
    kSMBprocessorUpgradeSocket939     = 0x12,
    kSMBprocessorUpgradeSocketmPGA604 = 0x13,
    kSMBprocessorUpgradeSocketLGA771  = 0x14,
    kSMBprocessorUpgradeSocketLGA775  = 0x15,
    kSMBprocessorUpgradeSocketS1      = 0x16,
    kSMBprocessorUpgradeAM2           = 0x17,
    kSMBprocessorUpgradeF1207         = 0x18,
    kSMBprocessorUpgradeSocketLGA1366 = 0x19,
    kSMBprocessorUpgradeSocketG34     = 0x1A,
    kSMBprocessorUpgradeSocketAM3     = 0x1B,
    kSMBprocessorUpgradeSocketC32     = 0x1C,
    kSMBprocessorUpgradeSocketLGA1156 = 0x1D,
    kSMBprocessorUpgradeSocketLGA1567 = 0x1E,
    kSMBprocessorUpgradeSocketPGA988A = 0x1F,
    kSMBprocessorUpgradeSocketBGA1288 = 0x20,
    kSMBprocessorUpgradeSocketrPGA988B = 0x21,
    kSMBprocessorUpgradeSocketBGA1023 = 0x22,
    kSMBprocessorUpgradeSocketBGA1224 = 0x23,
    kSMBprocessorUpgradeSocketBGA1155 = 0x24,
    kSMBprocessorUpgradeSocketLGA1356 = 0x25,
    kSMBprocessorUpgradeSocketLGA2011 = 0x26,
    kSMBprocessorUpgradeSocketFS1     = 0x27,
    kSMBprocessorUpgradeSocketFS2     = 0x28,
    kSMBprocessorUpgradeSocketFM1     = 0x29,
    kSMBprocessorUpgradeSocketFM2     = 0x2A
} PROCESSOR_UPGRADE;

// Struct - Processor Information (Type 4).
typedef struct SMBProcessorInformation
{
	// 2.0+ spec (26 bytes)
	SMB_STRUCT_HEADER               // Type 4
	SMBString  socketDesignation;
	SMBByte    processorType;       // The enumeration value from PROCESSOR_TYPE_DATA.
	SMBByte    processorFamily;     // The enumeration value from PROCESSOR_FAMILY_DATA.
	SMBString  manufacturer;
	SMBQWord   processorID;         // based on CPUID
	SMBString  processorVersion;
	SMBByte    voltage;             // bit7 cleared indicate legacy mode
	SMBWord    externalClock;       // external clock in MHz
	SMBWord    maximumClock;        // max internal clock in MHz
	SMBWord    currentClock;        // current internal clock in MHz
	SMBByte    status;
	SMBByte    processorUpgrade;    // The enumeration value from PROCESSOR_UPGRADE.
	// 2.1+ spec (32 bytes)
	SMBWord    L1CacheHandle;
	SMBWord    L2CacheHandle;
	SMBWord    L3CacheHandle;
	// 2.3+ spec (35 bytes)
	SMBString  serialNumber;
	SMBString  assetTag;
	SMBString  partNumber;
	// 2.5+ spec (40 bytes)
	SMBByte    coreCount;
	SMBByte    coreEnabled;
	SMBByte    threadCount;
//	SMBWord    processorFuncSupport;
	// 2.6+ spec (42 bytes)
//	SMBWord    processorFamily2;
} __attribute__((packed)) SMBProcessorInformation;

//----------------------------------------------------------------------------------------------------------
// Struct - Memory Controller Information (Type 5) Obsolete since SMBIOS version 2.1
typedef struct SMBMemoryControllerInfo {
	SMB_STRUCT_HEADER 
	SMBByte			errorDetectingMethod;
	SMBByte			errorCorrectingCapability;
	SMBByte			supportedInterleave;
	SMBByte			currentInterleave;
	SMBByte			maxMemoryModuleSize;
	SMBWord			supportedSpeeds;
	SMBWord			supportedMemoryTypes;
	SMBByte			memoryModuleVoltage;
	SMBByte			numberOfMemorySlots;
} __attribute__((packed)) SMBMemoryControllerInfo;

//----------------------------------------------------------------------------------------------------------
// Struct - Memory Module Information (Type 6) Obsolete since SMBIOS version 2.1
typedef struct SMBMemoryModule
{
    SMB_STRUCT_HEADER               // Type 6
    SMBString  socketDesignation;
    SMBByte    bankConnections;
    SMBByte    currentSpeed;
    SMBWord    currentMemoryType;
    SMBByte    installedSize;
    SMBByte    enabledSize;
    SMBByte    errorStatus;
} __attribute__((packed)) SMBMemoryModule;

#define kSMBMemoryModuleSizeNotDeterminable 0x7D
#define kSMBMemoryModuleSizeNotEnabled      0x7E
#define kSMBMemoryModuleSizeNotInstalled    0x7F

//----------------------------------------------------------------------------------------------------------
// Struct - Cache Information (Type 7)
typedef struct SMBCacheInformation
{
    SMB_STRUCT_HEADER               // Type 7
    SMBString  socketDesignation;
    SMBWord    cacheConfiguration;
    SMBWord    maximumCacheSize;
    SMBWord    installedSize;
    SMBWord    supportedSRAMType;
    SMBWord    currentSRAMType;
    SMBByte    cacheSpeed;
    SMBByte    errorCorrectionType;
    SMBByte    systemCacheType;
    SMBByte    associativity;
} __attribute__((packed)) SMBCacheInformation;

//----------------------------------------------------------------------------------------------------------
// Struct - System Slots (Type 9)
typedef struct SMBSystemSlot
{
    // 2.0+ spec (12 bytes)
	SMB_STRUCT_HEADER
	SMBString   slotDesignation;
	SMBByte     slotType;
	SMBByte     slotDataBusWidth;
	SMBByte     currentUsage;
	SMBByte     slotLength;
	SMBWord     slotID;
	SMBByte     slotCharacteristics1;
	// 2.1+ spec (13 bytes)
	SMBByte     slotCharacteristics2;
	// 2.6+ spec (17 bytes)
//	SMBWord		segmentGroupNumber;
//	SMBByte		busNumber;
//	SMBByte		deviceFunctionNumber;
} __attribute__((packed)) SMBSystemSlot;

//----------------------------------------------------------------------------------------------------------
// Struct - OEM Strings (Type 11)
typedef struct SMBOEMStrings
{
	SMB_STRUCT_HEADER               // Type 11
	SMBByte		count;		// number of strings
} __attribute__((packed)) SMBOEMStrings;

//----------------------------------------------------------------------------------------------------------
// Physical Memory Array (Type 16)

// Physical Memory Array - Use.
// Values for SMBPhysicalMemoryArray.arrayUse
typedef enum
{
    kSMBMemoryArrayUseOther             = 0x01,
    kSMBMemoryArrayUseUnknown           = 0x02,
    kSMBMemoryArrayUseSystemMemory      = 0x03,
    kSMBMemoryArrayUseVideoMemory       = 0x04,
    kSMBMemoryArrayUseFlashMemory       = 0x05,
    kSMBMemoryArrayUseNonVolatileMemory = 0x06,
    kSMBMemoryArrayUseCacheMemory       = 0x07
} MEMORY_ARRAY_USE;

// Physical Memory Array - Error Correction Types.
// Values for SMBPhysicalMemoryArray.errorCorrection
typedef enum
{
    kSMBMemoryArrayErrorCorrectionTypeOther         = 0x01,
    kSMBMemoryArrayErrorCorrectionTypeUnknown       = 0x02,
    kSMBMemoryArrayErrorCorrectionTypeNone          = 0x03,
    kSMBMemoryArrayErrorCorrectionTypeParity        = 0x04,
    kSMBMemoryArrayErrorCorrectionTypeSingleBitECC  = 0x05,
    kSMBMemoryArrayErrorCorrectionTypeMultiBitECC   = 0x06,
    kSMBMemoryArrayErrorCorrectionTypeCRC           = 0x07
} MEMORY_ERROR_CORRECTION;

// Struct - Physical Memory Array (Type 16)
typedef struct SMBPhysicalMemoryArray
{
    // 2.1+ spec (15 bytes)
    SMB_STRUCT_HEADER               // Type 16
    SMBByte    physicalLocation;    // physical location
    SMBByte    arrayUse;            // the use for the memory array, The enumeration value from MEMORY_ARRAY_USE.
    SMBByte    errorCorrection;     // error correction/detection method, The enumeration value from MEMORY_ERROR_CORRECTION.
    SMBDWord   maximumCapacity;     // maximum memory capacity in kilobytes
    SMBWord    errorHandle;         // handle of a previously detected error
    SMBWord    numMemoryDevices;    // number of memory slots or sockets
    // 2.7+ spec
    //	SMBQWord   extMaximumCapacity;	// maximum memory capacity in bytes
} __attribute__((packed)) SMBPhysicalMemoryArray;

//----------------------------------------------------------------------------------------------------------
// Struct - Memory Device (Type 17)
typedef struct SMBMemoryDevice
{
	// 2.1+ spec (21 bytes)
	SMB_STRUCT_HEADER               // Type 17
	SMBWord    arrayHandle;         // handle of the parent memory array
	SMBWord    errorHandle;         // handle of a previously detected error
	SMBWord    totalWidth;          // total width in bits; including ECC bits
	SMBWord    dataWidth;           // data width in bits
	SMBWord    memorySize;          // bit15 is scale, 0 = MB, 1 = KB
	SMBByte    formFactor;          // memory device form factor
	SMBByte    deviceSet;           // parent set of identical memory devices
	SMBString  deviceLocator;       // labeled socket; e.g. "SIMM 3"
	SMBString  bankLocator;         // labeled bank; e.g. "Bank 0" or "A"
	SMBByte    memoryType;          // type of memory
	SMBWord    memoryTypeDetail;    // additional detail on memory type
	// 2.3+ spec (27 bytes)
	SMBWord    memorySpeed;         // speed of device in MHz (0 for unknown)
	SMBString  manufacturer;
	SMBString  serialNumber;
	SMBString  assetTag;
	SMBString  partNumber;
	// 2.6+ spec (28 bytes)
//	SMBByte    attributes;
	// 2.7+ spec
//	SMBDWord   memoryExtSize;
//	SMBWord    confMemClkSpeed;
	// 2.8+ spec
//	SMBWord    minimumVolt;
//	SMBWord    maximumVolt;
//	SMBWord    configuredVolt;
} __attribute__((packed)) SMBMemoryDevice;

//----------------------------------------------------------------------------------------------------------

// Struct - Memory Array Mapped Address (Type 19)
//typedef struct SMBMemoryArrayMappedAddress
//{
    // 2.1+ spec
//	SMB_STRUCT_HEADER               // Type 19
//	SMBDWord   startingAddress;
//	SMBDWord   endingAddress;
//	SMBWord    arrayHandle;
//	SMBByte    partitionWidth;
	// 2.7+ spec
//	SMBQWord   extStartAddress;
//	SMBQWord   extEndAddress;
//} __attribute__((packed)) SMBMemoryArrayMappedAddress;

//----------------------------------------------------------------------------------------------------------

// Struct - Memory Device Mapped Address (Type 20)
//typedef struct SMBMemoryDeviceMappedAddress
//{
	// 2.1+ spec
//	SMB_STRUCT_HEADER               // Type 20
//	SMBDWord   startingAddress;
//	SMBDWord   endingAddress;
//	SMBWord    arrayHandle;
//	SMBByte    partitionRowPosition;
//	SMBByte    interleavePosition;
//	SMBByte    interleaveDataDepth;
	// 2.7+ spec
//	SMBQWord   extStartAddress;
//	SMBQWord   extEndAddress;
//} __attribute__((packed)) SMBMemoryDeviceMappedAddress;

//----------------------------------------------------------------------------------------------------------
// Firmware Volume Description (Apple Specific - Type 128)
enum
{
	FW_REGION_RESERVED   = 0,
	FW_REGION_RECOVERY   = 1,
	FW_REGION_MAIN       = 2,
	FW_REGION_NVRAM      = 3,
	FW_REGION_CONFIG     = 4,
	FW_REGION_DIAGVAULT  = 5,

	NUM_FLASHMAP_ENTRIES = 8
};

typedef struct FW_REGION_INFO
{
	SMBDWord   StartAddress;
	SMBDWord   EndAddress;
} __attribute__((packed)) FW_REGION_INFO;

// Struct - Firmware Volume Description (Apple Specific - Type 128)
typedef struct SMBFirmwareVolume
{
	SMB_STRUCT_HEADER			// Type 128
	SMBByte           RegionCount;
	SMBByte           Reserved[3];
	SMBDWord          FirmwareFeatures;
	SMBDWord          FirmwareFeaturesMask;
	SMBByte           RegionType[ NUM_FLASHMAP_ENTRIES ];
	FW_REGION_INFO    FlashMap[   NUM_FLASHMAP_ENTRIES ];
} __attribute__((packed)) SMBFirmwareVolume;

//----------------------------------------------------------------------------------------------------------

/* ===========================================
 Memory SPD Data   (Apple Specific - Type 130)
 ============================================= */
typedef struct SMBMemorySPD
{
	SMB_STRUCT_HEADER			// Type 130
	SMBWord           Type17Handle;
	SMBWord           Offset;
	SMBWord           Size;
	SMBWord           Data[1];
} __attribute__((packed)) SMBMemorySPD;

//----------------------------------------------------------------------------------------------------------

/* ============================================
 OEM Processor Type (Apple Specific - Type 131)
 ============================================== */
typedef struct SMBOemProcessorType
{
	SMB_STRUCT_HEADER			// Type131
	SMBWord    ProcessorType;
} __attribute__((packed)) SMBOemProcessorType;

//----------------------------------------------------------------------------------------------------------

/* =================================================
 OEM Processor Bus Speed (Apple Specific - Type 132)
 =================================================== */
typedef struct SMBOemProcessorBusSpeed
{
	SMB_STRUCT_HEADER			// Type 132
	SMBWord    ProcessorBusSpeed;   // MT/s unit
} __attribute__((packed)) SMBOemProcessorBusSpeed;

//----------------------------------------------------------------------------------------------------------

/* ==============================================
 OEM Platform Feature (Apple Specific - Type 133)
 ================================================ */
struct SMBOemPlatformFeature
{
	SMB_STRUCT_HEADER			// Type 133
	SMBWord    PlatformFeature;
} __attribute__((packed)) SMBOemPlatformFeature;

//----------------------------------------------------------------------------------------------------------

/* From Foundation/Efi/Guid/Smbios/SmBios.h */
/* Modified to wrap Data4 array init with {} */
#define EFI_SMBIOS_TABLE_GUID {0xeb9d2d31, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d}}

#define SMBIOS_ORIGINAL		0
#define SMBIOS_PATCHED		1

extern void *getSmbios(int which);
extern void readSMBIOSInfo(SMBEntryPoint *eps);
extern void setupSMBIOSTable(void);

extern void decodeSMBIOSTable(SMBEntryPoint *eps);


#endif /* !__LIBSAIO_SMBIOS_H */
