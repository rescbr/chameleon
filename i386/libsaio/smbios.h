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

//
// Based on System Management BIOS Reference Specification v3.1.1
// https://www.dmtf.org/standards/smbios
//

typedef uint8_t  SMBString;
typedef uint8_t  SMBByte;
typedef uint16_t SMBWord;
typedef uint32_t SMBDWord;
typedef uint64_t SMBQWord;



//
// SMBIOS 2.1 (32-bit) Entry Point
//
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



//
// Structure header format
// Each SMBIOS structure begins with a four-byte header
//
typedef struct SMBStructHeader
{
	SMBByte	type;
	SMBByte	length;
	SMBWord	handle;
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



//
// SMBIOS structure types.
// The following structures are requiered:
//   - BIOS Information (Type 0)
//   - System Information (Type 1)
//   - System Enclosure (Type 3)
//   - Processor Information (Type 4)
//   - Cache Information (Type 7)
//   - System Slots (Type 9)
//   - Physical Memory Array (Type 16)
//   - Memory Device (Type 17)
//   - Memory Array Mapped Address (Type 19)
//   - System Boot Information (Type 32)
//

enum
{
	kSMBTypeBIOSInformation         =  0, // BIOS information (Type 0)
	kSMBTypeSystemInformation       =  1, // System Information (Type 1)
	kSMBTypeBaseBoard               =  2, // BaseBoard Information (Type 2)
	kSMBTypeSystemEnclosure         =  3, // System Chassis Information (Type 3)
	kSMBTypeProcessorInformation    =  4, // Processor Information (Type 4)
	// Memory Controller Information (Type 5) Obsolete
	// Memory Module Information (Type 6) Obsolete
	kSMBTypeCacheInformation        =  7, // Cache Information (Type 7)
	// Port Connector Information (Type 8)
	kSMBTypeSystemSlot              =  9, // System Slots (Type 9)
	// On Board Devices Information (Type 10) Obsolete
	kSMBOEMStrings                  =  11,// OEM Strings (Type 11)
	// System Configuration Options (Type 12)
	// BIOS Language Information (Type 13)
	// Group Associations (Type 14)
	// System Event Log (Type 15)
	kSMBTypePhysicalMemoryArray     =  16, // Physical Memory Array (Type 16)
	kSMBTypeMemoryDevice            =  17, // Memory Device (Type 17)
	kSMBType32BitMemoryErrorInfo    =  18, // 32-Bit Memory Error Information (Type 18)
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
	kSMBType64BitMemoryErrorInfo    =  33, // 64-Bit Memory Error Information (Type 33)
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
	kSMBTypeFirmwareVolume          =  128, // FirmwareVolume (TYPE 128)
	kSMBTypeMemorySPD               =  130, // MemorySPD (TYPE 130)
	kSMBTypeOemProcessorType        =  131, // Processor Type (Type 131)
	kSMBTypeOemProcessorBusSpeed    =  132, // Processor Bus Speed (Type 132)
	kSMBTypeOemPlatformFeature      =  133, // Platform Feature (Type 133)
	kSMBTypeOemSMCVersion           =  134  // SMC Version (Type 134)
};



//----------------------------------------------------------------------------------------------------------
// Struct - BIOS Information (Type 0), Apple uses 24 bytes length
//----------------------------------------------------------------------------------------------------------

typedef struct SMBBIOSInformation
{
    SMB_STRUCT_HEADER               // Type 0
    SMBString  vendor;              // BIOS vendor name
    SMBString  version;             // BIOS version
    SMBWord    startSegment;        // BIOS segment start
    SMBString  releaseDate;         // BIOS release date (mm/dd/yy or mm/dd/yyyy)
    SMBByte    romSize;             // BIOS ROM Size (n); 64K * (n+1) bytes
    SMBQWord   characteristics;     // supported BIOS functions
    
    // Bungo - 2.4+ spec (6 bytes)
    SMBByte    characteristicsExt1; // BIOS characteristics extension byte 1
    SMBByte    characteristicsExt2; // BIOS characteristics extension byte 2
    SMBByte    releaseMajor;        // BIOS release (major)
    SMBByte    releaseMinor;        // BIOS release (minor)
    SMBByte    ECreleaseMajor;      // Embedded Controller firmware release (major)
    SMBByte    ECreleaseMinor;      // Embedded Controller firmware release (minor)
} __attribute__((packed)) SMBBIOSInformation;

// TODO: Add constants for BIOS characteristics bits (Section 7.1.1, table 7)
// TODO: Add constants for BIOS characteristics extenstion bytes (Section 7.1.2, table 8+9)


//----------------------------------------------------------------------------------------------------------
// Struct - System Information (Type 1), Apple uses 27 bytes length
//----------------------------------------------------------------------------------------------------------

typedef struct SMBSystemInformation
{
    // 2.0+ spec (8 bytes)
    SMB_STRUCT_HEADER               // Type 1
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

// TODO: Add enum for wake-up type field (Section 7.2.2, table 12)


//----------------------------------------------------------------------------------------------------------
// Base Board (or Module) Information (Type 2), Apple uses 16 bytes length
//----------------------------------------------------------------------------------------------------------

typedef struct SMBBaseBoard
{
    SMB_STRUCT_HEADER                     // Type 2
    SMBString	manufacturer;
    SMBString	product;
    SMBString	version;
    SMBString	serialNumber;
    SMBString	assetTag;                 // Bungo: renamed from assetTagNumber folowing convention
    SMBByte     featureFlags;             // Collection of flag that identify features of this baseboard
    SMBString	locationInChassis;
    SMBWord     chassisHandle;
    SMBByte     boardType;			      // Type of board, numeration value from BASE_BOARD_TYPE.
    SMBByte     numberOfContainedHandles;
    // 0 - 255 contained handles go here but we do not include
    // them in our structure. Be careful to use numberOfContainedHandles
    // times sizeof(SMBWord) when computing the actual record size,
    // if you need it.
    SMBByte     containedObjectHandles;
} __attribute__((packed)) SMBBaseBoard;

// Base Board - Board Type.
// Values for SMBBaseBoard.boardType
typedef enum
{
    kSMBBaseBoardUnknown               = 0x01,	// Unknown
    kSMBBaseBoardOther                 = 0x02,	// Other
    kSMBBaseBoardServerBlade           = 0x03,	// Server Blade
    kSMBBaseBoardConnectivitySwitch    = 0x04,	// Connectivity Switch
    kSMBBaseBoardSystemMgmtModule      = 0x05,	// System Management Module
    kSMBBaseBoardProcessorModule       = 0x06,	// Processor Module
    kSMBBaseBoardIOModule              = 0x07,	// I/O Module
    kSMBBaseBoardMemoryModule          = 0x08,	// Memory Module
    kSMBBaseBoardDaughter              = 0x09,	// Daughter board
    kSMBBaseBoardMotherboard           = 0x0A,	// Motherboard (includes processor, memory, and I/O)
    kSMBBaseBoardProcessorMemoryModule = 0x0B,	// Processor/Memory Module
    kSMBBaseBoardProcessorIOModule     = 0x0C,	// Processor/IO Module
    kSMBBaseBoardInterconnect          = 0x0D	// Interconnect board
} BASE_BOARD_TYPE;

// TODO: Add constants to identify the baseboard feature flags


//----------------------------------------------------------------------------------------------------------
// System Enclosure or Chassis (Type 3), Apple uses 21 bytes length
//----------------------------------------------------------------------------------------------------------

typedef struct SMBSystemEnclosure
{
	SMB_STRUCT_HEADER                   // Type 3
	SMBString  manufacturer;
	SMBByte    chassisType;             // System Enclosure Indicator
	SMBString  version;                 // Board Number?
	SMBString  serialNumber;
	SMBString  assetTag;                // Bungo: renamed from assetTagNumber folowing convention
	SMBByte    bootupState;             // State of enclosure when when it was last booted
	SMBByte    powerSupplyState;        // State of enclosure's power supply when last booted
	SMBByte    thermalState;            // Thermal state of the enclosure when last booted
	SMBByte    securityStatus;          // Physical security status of the enclosure when last booted
	SMBDWord   oemDefined;              // OEM- or BIOS vendor-specific information
	SMBByte    height;                  // Height of the enclosure, in 'U's
	SMBByte    numberOfPowerCords;      // Number of power cords associated with the enclosure or chassis
	SMBByte    containedElementCount;   // Number of Contained Element record that follow, in the range 0 to 255
	SMBByte    containedElementRecord;  // Byte leght of each Contained Element record that follow, in the range 0 to 255
    //	SMBByte    containedElements;   // Elements, possibly defined by other SMBIOS structures present in chassis
    // 2.7+
    //	SMBString  skuNumber;		// Number of null-terminated string describing the chassis or enclosure SKU number
} __attribute__((packed)) SMBSystemEnclosure;


// Bungo: values for SMBSystemEnclosure.chassisType
// MinusZwei: renamed enum values and added new values from later specs
typedef enum {
    kSMBChassisTypeOther             = 0x01,  // Other
    kSMBChassisTypeUnknown           = 0x02,  // Unknown
    kSMBChassisTypeDesktop           = 0x03,  // Desktop
    kSMBChassisTypeLowProfileDesktop = 0x04,  // Low Profile Desktop
    kSMBChassisTypePizzaBox          = 0x05,  // Pizza Box
    kSMBChassisTypeMiniTower         = 0x06,  // Mini Tower
    kSMBChassisTypeTower             = 0x07,  // Tower
    kSMBChassisTypePortable          = 0x08,  // Portable
    kSMBChassisTypeLaptop            = 0x09,  // Laptop
    kSMBChassisTypeNotebook          = 0x0A,  // Notebook
    kSMBChassisTypeHandHeld          = 0x0B,  // Hand Held
    kSMBChassisTypeDockingStation    = 0x0C,  // Docking Station
    kSMBChassisTypeAllInOne          = 0x0D,  // All in One
    kSMBChassisTypeSubNotebook       = 0x0E,  // Sub Notebook
    kSMBChassisTypeSpaceSaving       = 0x0F,  // Space-saving
    kSMBChassisTypeLunchBox          = 0x10,  // Lunch Box
    kSMBChassisTypeMainServer        = 0x11,  // Main Server Chassis
    kSMBChassisTypeExpansion         = 0x12,  // Expansion Chassis
    kSMBChassisTypeSubChassis        = 0x13,  // SubChassis
    kSMBChassisTypeBusExpansion      = 0x14,  // Bus Expansion Chassis
    kSMBChassisTypePeripheral        = 0x15,  // Peripheral Chassis
    kSMBChassisTypeRAID              = 0x16,  // RAID Chassis
    kSMBChassisTypeRackMount         = 0x17,  // Rack Mount Chassis
    kSMBChassisTypeSealedCase        = 0x18,  // Sealed-case PC
    kSMBChassisTypeMultiSystem       = 0x19,  // Multi-system chassis
    kSMBChassisTypeCompactPCI        = 0x1A,  // Compact PCI
    kSMBChassisTypeAdvancedTCA       = 0x1B,  // Advanced TCA
    kSMBChassisTypeBlade             = 0x1C,  // Blade
    kSMBChassisTypeBladeEnclosing    = 0x1D,  // Blade Enclosure
    kSMBChassisTypeTablet            = 0x1E,  // Tablet
    kSMBChassisTypeConvertible       = 0x1F,  // Convertible
    kSMBChassisTypeDetachable        = 0x20,  // Detachable
    kSMBChassisTypeIoTGateway        = 0x21,  // IoT Gateway
    kSMBChassisTypeEmbeddedPC        = 0x22,  // Embedded PC
    kSMBChassisTypeMiniPC            = 0x23,  // Mini PC
    kSMBChassisTypeStickPC           = 0x24   // Stick PC
} SYSTEM_ENCLOSURE_CHASSIS_TYPE;

// System Enclosure or Chassis States.
// values for SMBSystemEnclosure.bootupState
// values for SMBSystemEnclosure.powerSupplyState
// values for SMBSystemEnclosure.thermalState
typedef enum {
    kSMBChassisStateOther           = 0x01,  // Other
    kSMBChassisStateUnknown         = 0x02,  // Unknown
    kSMBChassisStateSafe            = 0x03,  // Safe
    kSMBChassisStateWarning         = 0x04,  // Warning
    kSMBChassisStateCritical        = 0x05,  // Critical
    kSMBChassisStateNonRecoverable  = 0x06   // Non-recoverable
} SYSTEM_ENCLOSURE_CHASSIS_STATE;

// System Enclosure or Chassis Security Status.
// values for SMBSystemEnclosure.securityStatus
typedef enum {
    kSMBChassisSecurityStatusOther                          = 0x01,  // Other
    kSMBChassisSecurityStatusUnknown                        = 0x02,  // Unknown
    kSMBChassisSecurityStatusNone                           = 0x03,  // None
    kSMBChassisSecurityStatusExternalInterfaceLockedOut     = 0x04,  // External interface locked out
    kSMBChassisSecurityStatusExternalInterfaceLockedEnabled = 0x05   // External interface enabled
} SYSTEM_ENCLOSURE_CHASSIS_SECURITY_STATE;


//----------------------------------------------------------------------------------------------------------
// Processor Information (Type 4), Apple uses 35 bytes length
//----------------------------------------------------------------------------------------------------------

typedef struct SMBProcessorInformation
{
    // 2.0+ spec (26 bytes)
    SMB_STRUCT_HEADER               // Type 4
    SMBString  socketDesignation;
    SMBByte    processorType;       // The enumeration value from PROCESSOR_TYPE.
    SMBByte    processorFamily;     // The enumeration value from PROCESSOR_FAMILY.
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
    // 2.5+ spec (40 bytes)  Apple still uses 2.4 spec
    //  SMBByte    coreCount;
    //  SMBByte    coreEnabled;
    //  SMBByte    threadCount;
    //	SMBWord    processorCharacteristics;
    // 2.6+ spec (42 bytes)
    //	SMBWord    processorFamily2;
} __attribute__((packed)) SMBProcessorInformation;

#define kSMBProcessorInformationMinSize     26

// Processor Information - Processor Type.
// Values for SMBProcessorInformation.processorType
typedef enum
{
	kSMBProcessorTypeOther   = 0x01,  // Other
	kSMBProcessorTypeUnknown = 0x02,  // Unknown
	kSMBProcessorTypeCPU     = 0x03,  // Central Processor
    kSMBProcessorTypeMPU     = 0x04,  // Math Processor
	kSMBProcessorTypeDSP     = 0x05,  // DSP Processor
	kSMBProcessorTypeGPU     = 0x06   // Video Processor
} PROCESSOR_TYPE;

// Processor Information - Processor Family.
// Values for SMBProcessorInformation.processorFamily
typedef enum {
    kSMBProcessorFamilyOther                           = 0x01,
    kSMBProcessorFamilyUnknown                         = 0x02,
    kSMBProcessorFamily8086                            = 0x03,
    kSMBProcessorFamily80286                           = 0x04,
    kSMBProcessorFamilyIntel386                        = 0x05,
    kSMBProcessorFamilyIntel486                        = 0x06,
    kSMBProcessorFamily8087                            = 0x07,
    kSMBProcessorFamily80287                           = 0x08,
    kSMBProcessorFamily80387                           = 0x09,
    kSMBProcessorFamily80487                           = 0x0A,
    kSMBProcessorFamilyPentium                         = 0x0B,
    kSMBProcessorFamilyPentiumPro                      = 0x0C,
    kSMBProcessorFamilyPentiumII                       = 0x0D,
    kSMBProcessorFamilyPentiumMMX                      = 0x0E,
    kSMBProcessorFamilyCeleron                         = 0x0F,
    kSMBProcessorFamilyPentiumIIXeon                   = 0x10,
    kSMBProcessorFamilyPentiumIII                      = 0x11,
    kSMBProcessorFamilyM1                              = 0x12,
    kSMBProcessorFamilyM2                              = 0x13,
    kSMBProcessorFamilyIntelCeleronM                   = 0x14,
    kSMBProcessorFamilyIntelPentium4Ht                 = 0x15,
    kSMBProcessorFamilyM1Reserved4                     = 0x16,
    kSMBProcessorFamilyM1Reserved5                     = 0x17,
    kSMBProcessorFamilyAmdDuron                        = 0x18,
    kSMBProcessorFamilyK5                              = 0x19,
    kSMBProcessorFamilyK6                              = 0x1A,
    kSMBProcessorFamilyK6_2                            = 0x1B,
    kSMBProcessorFamilyK6_3                            = 0x1C,
    kSMBProcessorFamilyAmdAthlon                       = 0x1D,
    kSMBProcessorFamilyAmd29000                        = 0x1E,
    kSMBProcessorFamilyK6_2Plus                        = 0x1F,
    kSMBProcessorFamilyPowerPC                         = 0x20,
    kSMBProcessorFamilyPowerPC601                      = 0x21,
    kSMBProcessorFamilyPowerPC603                      = 0x22,
    kSMBProcessorFamilyPowerPC603Plus                  = 0x23,
    kSMBProcessorFamilyPowerPC604                      = 0x24,
    kSMBProcessorFamilyPowerPC620                      = 0x25,
    kSMBProcessorFamilyPowerPCx704                     = 0x26,
    kSMBProcessorFamilyPowerPC750                      = 0x27,
    kSMBProcessorFamilyIntelCoreDuo                    = 0x28,
    kSMBProcessorFamilyIntelCoreDuoMobile              = 0x29,
    kSMBProcessorFamilyIntelCoreSoloMobile             = 0x2A,
    kSMBProcessorFamilyIntelAtom                       = 0x2B,
    kSMBProcessorFamilyAlpha3                          = 0x30,
    kSMBProcessorFamilyAlpha21064                      = 0x31,
    kSMBProcessorFamilyAlpha21066                      = 0x32,
    kSMBProcessorFamilyAlpha21164                      = 0x33,
    kSMBProcessorFamilyAlpha21164PC                    = 0x34,
    kSMBProcessorFamilyAlpha21164a                     = 0x35,
    kSMBProcessorFamilyAlpha21264                      = 0x36,
    kSMBProcessorFamilyAlpha21364                      = 0x37,
    kSMBProcessorFamilyAmdTurionIIUltraDualCoreMobileM = 0x38,
    kSMBProcessorFamilyAmdTurionIIDualCoreMobileM      = 0x39,
    kSMBProcessorFamilyAmdAthlonIIDualCoreM            = 0x3A,
    kSMBProcessorFamilyAmdOpteron6100Series            = 0x3B,
    kSMBProcessorFamilyAmdOpteron4100Series            = 0x3C,
    kSMBProcessorFamilyAmdOpteron6200Series            = 0x3D,
    kSMBProcessorFamilyAmdOpteron4200Series            = 0x3E,
    kSMBProcessorFamilyMips                            = 0x40,
    kSMBProcessorFamilyMIPSR4000                       = 0x41,
    kSMBProcessorFamilyMIPSR4200                       = 0x42,
    kSMBProcessorFamilyMIPSR4400                       = 0x43,
    kSMBProcessorFamilyMIPSR4600                       = 0x44,
    kSMBProcessorFamilyMIPSR10000                      = 0x45,
    kSMBProcessorFamilyAmdCSeries                      = 0x46,
    kSMBProcessorFamilyAmdESeries                      = 0x47,
    kSMBProcessorFamilyAmdSSeries                      = 0x48,
    kSMBProcessorFamilyAmdGSeries                      = 0x49,
    kSMBProcessorFamilySparc                           = 0x50,
    kSMBProcessorFamilySuperSparc                      = 0x51,
    kSMBProcessorFamilymicroSparcII                    = 0x52,
    kSMBProcessorFamilymicroSparcIIep                  = 0x53,
    kSMBProcessorFamilyUltraSparc                      = 0x54,
    kSMBProcessorFamilyUltraSparcII                    = 0x55,
    kSMBProcessorFamilyUltraSparcIIi                   = 0x56,
    kSMBProcessorFamilyUltraSparcIII                   = 0x57,
    kSMBProcessorFamilyUltraSparcIIIi                  = 0x58,
    kSMBProcessorFamily68040                           = 0x60,
    kSMBProcessorFamily68xxx                           = 0x61,
    kSMBProcessorFamily68000                           = 0x62,
    kSMBProcessorFamily68010                           = 0x63,
    kSMBProcessorFamily68020                           = 0x64,
    kSMBProcessorFamily68030                           = 0x65,
    kSMBProcessorFamilyHobbit                          = 0x70,
    kSMBProcessorFamilyCrusoeTM5000                    = 0x78,
    kSMBProcessorFamilyCrusoeTM3000                    = 0x79,
    kSMBProcessorFamilyEfficeonTM8000                  = 0x7A,
    kSMBProcessorFamilyWeitek                          = 0x80,
    kSMBProcessorFamilyItanium                         = 0x82,
    kSMBProcessorFamilyAmdAthlon64                     = 0x83,
    kSMBProcessorFamilyAmdOpteron                      = 0x84,
    kSMBProcessorFamilyAmdSempron                      = 0x85,
    kSMBProcessorFamilyAmdTurion64Mobile               = 0x86,
    kSMBProcessorFamilyDualCoreAmdOpteron              = 0x87,
    kSMBProcessorFamilyAmdAthlon64X2DualCore           = 0x88,
    kSMBProcessorFamilyAmdTurion64X2Mobile             = 0x89,
    kSMBProcessorFamilyQuadCoreAmdOpteron              = 0x8A,
    kSMBProcessorFamilyThirdGenerationAmdOpteron       = 0x8B,
    kSMBProcessorFamilyAmdPhenomFxQuadCore             = 0x8C,
    kSMBProcessorFamilyAmdPhenomX4QuadCore             = 0x8D,
    kSMBProcessorFamilyAmdPhenomX2DualCore             = 0x8E,
    kSMBProcessorFamilyAmdAthlonX2DualCore             = 0x8F,
    kSMBProcessorFamilyPARISC                          = 0x90,
    kSMBProcessorFamilyPaRisc8500                      = 0x91,
    kSMBProcessorFamilyPaRisc8000                      = 0x92,
    kSMBProcessorFamilyPaRisc7300LC                    = 0x93,
    kSMBProcessorFamilyPaRisc7200                      = 0x94,
    kSMBProcessorFamilyPaRisc7100LC                    = 0x95,
    kSMBProcessorFamilyPaRisc7100                      = 0x96,
    kSMBProcessorFamilyV30                             = 0xA0,
    kSMBProcessorFamilyQuadCoreIntelXeon3200Series     = 0xA1,
    kSMBProcessorFamilyDualCoreIntelXeon3000Series     = 0xA2,
    kSMBProcessorFamilyQuadCoreIntelXeon5300Series     = 0xA3,
    kSMBProcessorFamilyDualCoreIntelXeon5100Series     = 0xA4,
    kSMBProcessorFamilyDualCoreIntelXeon5000Series     = 0xA5,
    kSMBProcessorFamilyDualCoreIntelXeonLV             = 0xA6,
    kSMBProcessorFamilyDualCoreIntelXeonULV            = 0xA7,
    kSMBProcessorFamilyDualCoreIntelXeon7100Series     = 0xA8,
    kSMBProcessorFamilyQuadCoreIntelXeon5400Series     = 0xA9,
    kSMBProcessorFamilyQuadCoreIntelXeon               = 0xAA,
    kSMBProcessorFamilyDualCoreIntelXeon5200Series     = 0xAB,
    kSMBProcessorFamilyDualCoreIntelXeon7200Series     = 0xAC,
    kSMBProcessorFamilyQuadCoreIntelXeon7300Series     = 0xAD,
    kSMBProcessorFamilyQuadCoreIntelXeon7400Series     = 0xAE,
    kSMBProcessorFamilyMultiCoreIntelXeon7400Series    = 0xAF,
    kSMBProcessorFamilyPentiumIIIXeon                  = 0xB0,
    kSMBProcessorFamilyPentiumIIISpeedStep             = 0xB1,
    kSMBProcessorFamilyPentium4                        = 0xB2,
    kSMBProcessorFamilyIntelXeon                       = 0xB3,
    kSMBProcessorFamilyAS400                           = 0xB4,
    kSMBProcessorFamilyIntelXeonMP                     = 0xB5,
    kSMBProcessorFamilyAMDAthlonXP                     = 0xB6,
    kSMBProcessorFamilyAMDAthlonMP                     = 0xB7,
    kSMBProcessorFamilyIntelItanium2                   = 0xB8,
    kSMBProcessorFamilyIntelPentiumM                   = 0xB9,
    kSMBProcessorFamilyIntelCeleronD                   = 0xBA,
    kSMBProcessorFamilyIntelPentiumD                   = 0xBB,
    kSMBProcessorFamilyIntelPentiumEx                  = 0xBC,
    kSMBProcessorFamilyIntelCoreSolo                   = 0xBD,  ///< SMBIOS spec 2.6 correct this value
    kSMBProcessorFamilyReserved                        = 0xBE,
    kSMBProcessorFamilyIntelCore2                      = 0xBF,
    kSMBProcessorFamilyIntelCore2Solo                  = 0xC0,
    kSMBProcessorFamilyIntelCore2Extreme               = 0xC1,
    kSMBProcessorFamilyIntelCore2Quad                  = 0xC2,
    kSMBProcessorFamilyIntelCore2ExtremeMobile         = 0xC3,
    kSMBProcessorFamilyIntelCore2DuoMobile             = 0xC4,
    kSMBProcessorFamilyIntelCore2SoloMobile            = 0xC5,
    kSMBProcessorFamilyIntelCoreI7                     = 0xC6,
    kSMBProcessorFamilyDualCoreIntelCeleron            = 0xC7,
    kSMBProcessorFamilyIBM390                          = 0xC8,
    kSMBProcessorFamilyG4                              = 0xC9,
    kSMBProcessorFamilyG5                              = 0xCA,
    kSMBProcessorFamilyG6                              = 0xCB,
    kSMBProcessorFamilyzArchitectur                    = 0xCC,
    kSMBProcessorFamilyIntelCoreI5                     = 0xCD,
    kSMBProcessorFamilyIntelCoreI3                     = 0xCE,
    kSMBProcessorFamilyViaC7M                          = 0xD2,
    kSMBProcessorFamilyViaC7D                          = 0xD3,
    kSMBProcessorFamilyViaC7                           = 0xD4,
    kSMBProcessorFamilyViaEden                         = 0xD5,
    kSMBProcessorFamilyMultiCoreIntelXeon              = 0xD6,
    kSMBProcessorFamilyDualCoreIntelXeon3Series        = 0xD7,
    kSMBProcessorFamilyQuadCoreIntelXeon3Series        = 0xD8,
    kSMBProcessorFamilyViaNano                         = 0xD9,
    kSMBProcessorFamilyDualCoreIntelXeon5Series        = 0xDA,
    kSMBProcessorFamilyQuadCoreIntelXeon5Series        = 0xDB,
    kSMBProcessorFamilyDualCoreIntelXeon7Series        = 0xDD,
    kSMBProcessorFamilyQuadCoreIntelXeon7Series        = 0xDE,
    kSMBProcessorFamilyMultiCoreIntelXeon7Series       = 0xDF,
    kSMBProcessorFamilyMultiCoreIntelXeon3400Series    = 0xE0,
    kSMBProcessorFamilyEmbeddedAmdOpteronQuadCore      = 0xE6,
    kSMBProcessorFamilyAmdPhenomTripleCore             = 0xE7,
    kSMBProcessorFamilyAmdTurionUltraDualCoreMobile    = 0xE8,
    kSMBProcessorFamilyAmdTurionDualCoreMobile         = 0xE9,
    kSMBProcessorFamilyAmdAthlonDualCore               = 0xEA,
    kSMBProcessorFamilyAmdSempronSI                    = 0xEB,
    kSMBProcessorFamilyAmdPhenomII                     = 0xEC,
    kSMBProcessorFamilyAmdAthlonII                     = 0xED,
    kSMBProcessorFamilySixCoreAmdOpteron               = 0xEE,
    kSMBProcessorFamilyAmdSempronM                     = 0xEF,
    kSMBProcessorFamilyi860                            = 0xFA,
    kSMBProcessorFamilyi960                            = 0xFB,
    kSMBProcessorFamilyIndicatorFamily2                = 0xFE,
    kSMBProcessorFamilyReserved1                       = 0xFF
} PROCESSOR_FAMILY;

// Processor Information - Processor Upgrade.
// Values for SMBProcessorInformation.processorUpgrade
typedef enum {
    kSMBProcessorUpgradeOther           = 0x01,  // Other
    kSMBProcessorUpgradeUnknown         = 0x02,  // Unknown
    kSMBProcessorUpgradeDaughterBoard   = 0x03,  // Daughter Board
    kSMBProcessorUpgradeZIFSocket       = 0x04,  // ZIF Socket
    kSMBProcessorUpgradePiggyBack       = 0x05,  // Replaceable Piggy Back
    kSMBProcessorUpgradeNone            = 0x06,  // None
    kSMBProcessorUpgradeLIFSocket       = 0x07,  // LIF Socket
    kSMBProcessorUpgradeSlot1           = 0x08,  // Slot 1
    kSMBProcessorUpgradeSlot2           = 0x09,  // Slot 2
    kSMBProcessorUpgrade370PinSocket    = 0x0A,  // 370-pin socket
    kSMBProcessorUpgradeSlotA           = 0x0B,  // Slot A
    kSMBProcessorUpgradeSlotM           = 0x0C,  // Slot M
    kSMBProcessorUpgradeSocket423       = 0x0D,  // Socket 423
    kSMBProcessorUpgradeSocketA         = 0x0E,  // Socket A (Socket 462)
    kSMBProcessorUpgradeSocket478       = 0x0F,  // Socket 478
    kSMBProcessorUpgradeSocket754       = 0x10,  // Socket 754
    kSMBProcessorUpgradeSocket940       = 0x11,  // Socket 940
    kSMBProcessorUpgradeSocket939       = 0x12,  // Socket 939
    kSMBProcessorUpgradeSocketmPGA604   = 0x13,  // Socket mPGA604
    kSMBProcessorUpgradeSocketLGA771    = 0x14,  // Socket LGA771
    kSMBProcessorUpgradeSocketLGA775    = 0x15,  // Socket LGA775
    kSMBProcessorUpgradeSocketS1        = 0x16,  // Socket S1
    kSMBProcessorUpgradeAM2             = 0x17,  // Socket AM2
    kSMBProcessorUpgradeF1207           = 0x18,  // Socket F (1207)
    kSMBProcessorUpgradeSocketLGA1366   = 0x19,  // Socket LGA1366
    kSMBProcessorUpgradeSocketG34       = 0x1A,  // Socket G34
    kSMBProcessorUpgradeSocketAM3       = 0x1B,  // Socket AM3
    kSMBProcessorUpgradeSocketC32       = 0x1C,  // Socket C32
    kSMBProcessorUpgradeSocketLGA1156   = 0x1D,  // Socket LGA1156
    kSMBProcessorUpgradeSocketLGA1567   = 0x1E,  // Socket LGA1567
    kSMBProcessorUpgradeSocketPGA988A   = 0x1F,  // Socket PGA988A
    kSMBProcessorUpgradeSocketBGA1288   = 0x20,  // Socket BGA1288
    kSMBProcessorUpgradeSocketrPGA988B  = 0x21,  // Socket rPGA988B
    kSMBProcessorUpgradeSocketBGA1023   = 0x22,  // Socket BGA1023
    kSMBProcessorUpgradeSocketBGA1224   = 0x23,  // Socket BGA1224
    kSMBProcessorUpgradeSocketBGA1155   = 0x24,  // Socket LGA1155
    kSMBProcessorUpgradeSocketLGA1356   = 0x25,  // Socket LGA1356
    kSMBProcessorUpgradeSocketLGA2011   = 0x26,  // Socket LGA2011
    kSMBProcessorUpgradeSocketFS1       = 0x27,  // Socket FS1
    kSMBProcessorUpgradeSocketFS2       = 0x28,  // Socket FS2
    kSMBProcessorUpgradeSocketFM1       = 0x29,  // Socket FM1
    kSMBProcessorUpgradeSocketFM2       = 0x2A,  // Socket FM2
    kSMBProcessorUpgradeSocketLGA2011_3 = 0x2B,  // Socket LGA2011-3
    kSMBProcessorUpgradeSocketLGA1356_3 = 0x2C,  // Socket LGA1356-3
    kSMBProcessorUpgradeSocketLGA1150   = 0x2D,  // Socket LGA1150
    kSMBProcessorUpgradeSocketBGA1168   = 0x2E,  // Socket BGA1168
    kSMBProcessorUpgradeSocketBGA1234   = 0x2F,  // Socket BGA1234
    kSMBProcessorUpgradeSocketBGA1364   = 0x30,  // Socket BGA1364
    kSMBProcessorUpgradeSocketAM4       = 0x31,  // Socket AM4
    kSMBProcessorUpgradeSocketLGA1151   = 0x32,  // Socket LGA1151
    kSMBProcessorUpgradeSocketBGA1356   = 0x33,  // Socket BGA1356
    kSMBProcessorUpgradeSocketBGA1440   = 0x34,  // Socket BGA1440
    kSMBProcessorUpgradeSocketBGA1515   = 0x35,  // Socket BGA1515
    kSMBProcessorUpgradeSocketLGA3647_1 = 0x36   // Socket LGA3647-1
} PROCESSOR_UPGRADE;


//----------------------------------------------------------------------------------------------------------
// Struct - Cache Information (Type 7), Apple uses 19 bytes length
//----------------------------------------------------------------------------------------------------------

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
// Struct - System Slots (Type 9), Apple uses 13 bytes length
//----------------------------------------------------------------------------------------------------------

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
// Struct - OEM Strings (Type 11), Apple uses 5 bytes length
//----------------------------------------------------------------------------------------------------------

typedef struct SMBOEMStrings
{
	SMB_STRUCT_HEADER               // Type 11
	SMBByte		count;		// number of strings
} __attribute__((packed)) SMBOEMStrings;


//----------------------------------------------------------------------------------------------------------
// Physical Memory Array (Type 16), Apple uses 15 bytes length
//----------------------------------------------------------------------------------------------------------

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


//----------------------------------------------------------------------------------------------------------
// Struct - Memory Device (Type 17), Apple uses 27 bytes length
//----------------------------------------------------------------------------------------------------------

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
// Struct - Memory Array Mapped Address (Type 19), Apple uses 15 bytes length
//----------------------------------------------------------------------------------------------------------

#if 0
typedef struct SMBMemoryArrayMappedAddress
{
    // 2.1+ spec
	SMB_STRUCT_HEADER               // Type 19
	SMBDWord   startingAddress;
	SMBDWord   endingAddress;
	SMBWord    arrayHandle;
	SMBByte    partitionWidth;
    // 2.7+ spec
	SMBQWord   extStartAddress;
	SMBQWord   extEndAddress;
} __attribute__((packed)) SMBMemoryArrayMappedAddress;
#endif


//----------------------------------------------------------------------------------------------------------
// Struct - Memory Device Mapped Address (Type 20)
//----------------------------------------------------------------------------------------------------------

#if 0
typedef struct SMBMemoryDeviceMappedAddress
{
    // 2.1+ spec
	SMB_STRUCT_HEADER               // Type 20
	SMBDWord   startingAddress;
	SMBDWord   endingAddress;
	SMBWord    arrayHandle;
	SMBByte    partitionRowPosition;
	SMBByte    interleavePosition;
	SMBByte    interleaveDataDepth;
    // 2.7+ spec
	SMBQWord   extStartAddress;
	SMBQWord   extEndAddress;
} __attribute__((packed)) SMBMemoryDeviceMappedAddress;
#endif



//
// Apple Specific structures
//

//----------------------------------------------------------------------------------------------------------
// Firmware Volume Description (Apple Specific - Type 128), Apple uses 88 bytes length
//----------------------------------------------------------------------------------------------------------

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
// Memory SPD Data (Apple Specific - Type 130)
//----------------------------------------------------------------------------------------------------------

typedef struct SMBMemorySPD
{
	SMB_STRUCT_HEADER			// Type 130
	SMBWord           Type17Handle;
	SMBWord           Offset;
	SMBWord           Size;
	SMBWord           Data[1];
} __attribute__((packed)) SMBMemorySPD;


//----------------------------------------------------------------------------------------------------------
// OEM Processor Type (Apple Specific - Type 131)
//----------------------------------------------------------------------------------------------------------

typedef struct SMBOemProcessorType
{
	SMB_STRUCT_HEADER			// Type131
	SMBWord    ProcessorType;
} __attribute__((packed)) SMBOemProcessorType;


//----------------------------------------------------------------------------------------------------------
// OEM Processor Bus Speed (Apple Specific - Type 132)
//----------------------------------------------------------------------------------------------------------

typedef struct SMBOemProcessorBusSpeed
{
	SMB_STRUCT_HEADER			// Type 132
	SMBWord    ProcessorBusSpeed;		// MT/s unit
} __attribute__((packed)) SMBOemProcessorBusSpeed;


//----------------------------------------------------------------------------------------------------------
// OEM Platform Feature (Apple Specific - Type 133)
//----------------------------------------------------------------------------------------------------------

typedef struct SMBOemPlatformFeature
{
	SMB_STRUCT_HEADER			// Type 133
	SMBWord    PlatformFeature;
} __attribute__((packed)) SMBOemPlatformFeature;


//----------------------------------------------------------------------------------------------------------
// OEM SMC Version (Apple Specific - Type 134)
//----------------------------------------------------------------------------------------------------------

typedef struct SMBOemSMCVersion
{
	SMB_STRUCT_HEADER			// Type 134
	SMBWord    SMCVersion;
} __attribute__((packed)) SMBOemSMCVersion;



//
// Obsolete SMBIOS structures
//

//----------------------------------------------------------------------------------------------------------
// Struct - Memory Controller Information (Type 5) Obsolete since SMBIOS version 2.1
//----------------------------------------------------------------------------------------------------------

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
//----------------------------------------------------------------------------------------------------------

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

#define SMBIOS_ORIGINAL		0
#define SMBIOS_PATCHED		1


extern void *getSmbios(int which);

extern void readSMBIOSInfo(SMBEntryPoint *eps);

extern void setupSMBIOSTable(void);

extern void decodeSMBIOSTable(SMBEntryPoint *eps);

#endif /* !__LIBSAIO_SMBIOS_H */
