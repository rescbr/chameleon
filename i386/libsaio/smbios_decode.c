/*
 * A very simple SMBIOS Table decoder, part of the Chameleon Boot Loader Project
 *
 * Copyright 2010 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */

#include "libsaio.h"
#include "smbios.h"
// Bungo:
#include "boot.h"
#include "bootstruct.h"

#ifndef DEBUG_SMBIOS
#define DEBUG_SMBIOS 0
#endif

#if DEBUG_SMBIOS
#define DBG(x...)	printf(x)
#else
#define DBG(x...)	msglog(x)
#endif

extern char *getSMBStringForField(SMBStructHeader *structHeader, uint8_t field);
// Bungo:
#define             NotSpecifiedStr     "Not Specified"  // no string
#define             OutOfSpecStr        "<OUT OF SPEC>"  // value out of smbios spec. range
#define             PrivateStr          "** PRIVATE **"  // masking private data
#define             neverMask           false

static bool         privateData         = true;
static SMBByte      minorVersion;   // SMBIOS rev. minor
static SMBByte      majorVersion;   // SMBIOS rev. major
static SMBByte      bcdRevisionLo;  // DMI rev. minor
static SMBByte      bcdRevisionHi;  // DMI rev. major

/*====
 7.2.2
 ===*/
static const char *SMBWakeUpTypes[] =  // Bungo: strings for wake-up type (Table Type 1 - System Information)
{
	"Reserved",          /* 00h */
	"Other",             /* 01h */
	"Unknown",           /* 02h */
	"APM Timer",         /* 03h */
	"Modem Ring",        /* 04h */
	"LAN Remote",        /* 05h */
	"Power Switch",      /* 06h */
	"PCI PME#",          /* 07h */
	"AC Power Restored"  /* 08h */
};

/*====
 7.3.2
 ===*/
static const char *SMBBaseBoardTypes[] =  // Bungo: strings for base board type (Table Type 2 - Base Board Information)
{
	"Unknown",                  /* 01h */
	"Other",                    /* 02h */
	"Server Blade",             /* 03h */
	"Connectivity Switch",      /* 04h */
	"System Management Module", /* 05h */
	"Processor Module",         /* 06h */
	"I/O Module",               /* 07h */
	"Memory Module",            /* 08h */
	"Daughter Board",           /* 09h */
	"Motherboard",              /* 0Ah */
	"Processor+Memory Module",  /* 0Bh */
	"Processor+I/O Module",     /* 0Ch */
	"Interconnect Board"        /* 0Dh */
};

 /*===
 7.4.1
 ===*/
static const char *SMBChassisTypes[] =  // Bungo: strings for chassis type (Table Type 3 - Chassis Information)
{
	"Other",                /* 01h */
	"Unknown",              /* 02h */
	"Desktop",              /* 03h */
	"Low Profile Desktop",  /* 04h */
	"Pizza Box",            /* 05h */
	"Mini Tower",           /* 06h */
	"Tower",                /* 07h */
	"Portable",             /* 08h */
	"Laptop",               /* 09h */
	"Notebook",             /* 0Ah */
	"Hand Held",            /* 0Bh */
	"Docking Station",      /* 0Ch */
	"All in One",           /* 0Dh */
	"Sub Notebook",         /* 0Eh */
	"Space-saving",         /* 0Fh */
	"Lunch Box",		/* 10h */
	"Main Server Chassis",	/* 11h */
	"Expansion Chassis",	/* 12h */
	"SubChassis",		/* 13h */
	"Bus Expansion Chassis",/* 14h */
	"Peripheral Chassis",	/* 15h */
	"RAID Chassis",		/* 16h */
	"Rack Mount Chassis",   /* 17h */
	"Sealed-case PC",	/* 18h */
	"Multi-system Chassis", /* 19h */
	"Compact PCI",		/* 1Ah */
	"Advanced TCA",		/* 1Bh */
	"Blade",		/* 1Ch */ // An SMBIOS implementation for a Blade would contain a Type 3 Chassis structure
	"Blade Enclosing"	/* 1Dh */ // A Blade Enclosure is a specialized chassis that contains a set of Blades.
};

/*====
 7.5.1
 ===*/
static const char *SMBProcessorTypes[] =  // Bungo: strings for processor type (Table Type 4 - Processor Information)
{
	"Other",                /* 01h */
	"Unknown",              /* 02h */
	"Central Processor",    /* 03h */
	"Math Processor",       /* 04h */
	"DSP Processor",        /* 05h */
	"Video Processor"       /* 06h */
};

/*====
 7.5.5
 ===*/
static const char *SMBProcessorUpgrades[] =  // ErmaC: strings for processor upgrade (Table Type 4 - Processor Information)
{
	"Other",                /* 01h */
	"Unknown",              /* 02h */
	"Daughter Board",
	"ZIF Socket",
	"Replaceable Piggy Back",
	"None",
	"LIF Socket",
	"Slot 1",
	"Slot 2",
	"370-pin Socket",
	"Slot A",
	"Slot M",
	"Socket 423",
	"Socket A (Socket 462)",
	"Socket 478",
	"Socket 754",
	"Socket 940",
	"Socket 939",
	"Socket mPGA604",
	"Socket LGA771",
	"Socket LGA775",
	"Socket S1",
	"Socket AM2",
	"Socket F (1207)",
	"Socket LGA1366",
	"Socket G34",
	"Socket AM3",
	"Socket C32",
	"Socket LGA1156",
	"Socket LGA1567",
	"Socket PGA988A",
	"Socket BGA1288",
	"Socket rPGA988B",
	"Socket BGA1023",
	"Socket BGA1224",
	"Socket BGA1155",
	"Socket LGA1356",
	"Socket LGA2011",
	"Socket FS1",
	"Socket FS2",
	"Socket FM1",
	"Socket FM2",
	"Socket LGA2011-3",
	"Socket LGA1356-3"              /* 2Ch */
};

static const char *SMBMemoryDeviceFormFactors[] =    // Bungo: strings for form factor (Table Type 17 - Memory Device)
{
	"Other",                    /* 01h */
	"Unknown",                  /* 02h */
	"SIMM",                     /* 03h */
	"SIP",                      /* 04h */
	"Chip",                     /* 05h */
	"DIP",                      /* 06h */
	"ZIP",                      /* 07h */
	"Proprietary Card",         /* 08h */
	"DIMM",                     /* 09h */
	"TSOP",                     /* 0Ah */
	"Row of chips",             /* 0Bh */
	"RIMM",                     /* 0Ch */
	"SODIMM",                   /* 0Dh */
	"SRIMM",                    /* 0Eh */
	"FB-DIMM"                   /* 0Fh */
};

/*=====
 7.18.2
 ====*/
static const char *
SMBMemoryDeviceTypes[] =
{
	"RAM",          /* 00h  Undefined */
	"RAM",          /* 01h  Other */
	"RAM",          /* 02h  Unknown */
	"DRAM",         /* 03h  DRAM */
	"EDRAM",        /* 04h  EDRAM */
	"VRAM",         /* 05h  VRAM */
	"SRAM",         /* 06h  SRAM */
	"RAM",          /* 07h  RAM */
	"ROM",          /* 08h  ROM */
	"FLASH",        /* 09h  FLASH */
	"EEPROM",       /* 0Ah  EEPROM */
	"FEPROM",       /* 0Bh  FEPROM */
	"EPROM",        /* 0Ch  EPROM */
	"CDRAM",        /* 0Dh  CDRAM */
	"3DRAM",        /* 0Eh  3DRAM */
	"SDRAM",        /* 0Fh  SDRAM */
	"SGRAM",        /* 10h  SGRAM */
	"RDRAM",        /* 11h  RDRAM */
	"DDR SDRAM",    /* 12h  DDR */
	"DDR2 SDRAM",   /* 13h  DDR2 */
	"DDR2 FB-DIMM", /* 14h  DDR2 FB-DIMM */
	"RAM",		/* 15h  unused */
	"RAM",		/* 16h  unused */
	"RAM",		/* 17h  unused */
	"DDR3",		/* 18h  DDR3, chosen in [5776134] */
	"FBD2"		/* 19h  FBD2 */
};

static const int kSMBMemoryDeviceTypeCount = sizeof(SMBMemoryDeviceTypes) / sizeof(SMBMemoryDeviceTypes[0]);

// Bungo: fixes random string readout if null in smbios to "Not Specified" as dmidecode displays
char *SMBStringForField(SMBStructHeader *structHeader, uint8_t field, const bool mask)
{
	char  *str = getSMBStringForField(structHeader, field);
    if (mask)
		str = PrivateStr;
    else if (!field)
        str = NotSpecifiedStr;

	return str;
};

void printHeader(SMBStructHeader *structHeader)
{
	verbose("Handle: 0x%04X, DMI type %d, %d bytes\n", structHeader->handle, structHeader->type, structHeader->length);
}

//-------------------------------------------------------------------------------------------------------------------------
// BIOS Information (Type 0)
//-------------------------------------------------------------------------------------------------------------------------
void decodeBIOSInformation(SMBStructHeader *structHeader)
{
	printHeader(structHeader);
	verbose("BIOS Information\n");
	verbose("\tVendor: %s\n", SMBStringForField(structHeader, ((SMBBIOSInformation *)structHeader)->vendor, neverMask));
	verbose("\tVersion: %s\n", SMBStringForField(structHeader, ((SMBBIOSInformation *)structHeader)->version, neverMask));
	verbose("\tRelease Date: %s\n", SMBStringForField(structHeader, ((SMBBIOSInformation *)structHeader)->releaseDate, neverMask));
// Address:
// Runtime Size:
// ROM Size:
// verbose("\tSupported BIOS functions: (0x%llX) %s\n", ((SMBBIOSInformation *)structHeader)->characteristics, SMBBIOSInfoChar0[((SMBBIOSInformation *)structHeader)->characteristics]);
	verbose("\tBIOS Revision: %d.%d\n", ((SMBBIOSInformation *)structHeader)->releaseMajor, ((SMBBIOSInformation *)structHeader)->releaseMinor);
// Firmware Major Release
// Firmware Minor Release
// SMBByte    characteristicsExt1;
// SMBByte    characteristicsExt2;
	verbose("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// System Information (Type 1)
//-------------------------------------------------------------------------------------------------------------------------
void decodeSystemInformation(SMBStructHeader *structHeader)
{
	printHeader(structHeader);
	verbose("System Information\n");
	verbose("\tManufacturer: %s\n", SMBStringForField(structHeader, ((SMBSystemInformation *)structHeader)->manufacturer, neverMask));
	verbose("\tProduct Name: %s\n", SMBStringForField(structHeader, ((SMBSystemInformation *)structHeader)->productName, neverMask));
	verbose("\tVersion: %s\n", SMBStringForField(structHeader, ((SMBSystemInformation *)structHeader)->version, neverMask));
	verbose("\tSerial Number: %s\n", SMBStringForField(structHeader, ((SMBSystemInformation *)structHeader)->serialNumber, privateData));
	uint8_t *uuid = ((SMBSystemInformation *)structHeader)->uuid;
	if (privateData) {
		verbose("\tUUID: %s\n", PrivateStr);
	} else {
		verbose("\tUUID: %02X%02X%02X%02X-%02X%02X-%02X%02X-%02x%02X-%02X%02X%02X%02X%02X%02X\n",
			uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
			uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
	}
	if (((SMBSystemInformation *)structHeader)->wakeupReason > 8) {
		verbose("\tWake-up Type: %s\n", OutOfSpecStr);
	} else {
		verbose("\tWake-up Type: %s\n", SMBWakeUpTypes[((SMBSystemInformation *)structHeader)->wakeupReason]);
	}
	verbose("\tSKU Number: %s\n", SMBStringForField(structHeader, ((SMBSystemInformation *)structHeader)->skuNumber, neverMask)); // System SKU#
	verbose("\tFamily: %s\n", SMBStringForField(structHeader, ((SMBSystemInformation *)structHeader)->family, neverMask));
	verbose("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// Base Board (or Module) Information (Type 2)
//-------------------------------------------------------------------------------------------------------------------------
void decodeBaseBoard(SMBStructHeader *structHeader)
{
	printHeader(structHeader);
	verbose("Base Board Information\n");
	verbose("\tManufacturer: %s\n", SMBStringForField(structHeader, ((SMBBaseBoard *)structHeader)->manufacturer, neverMask));
	verbose("\tProduct Name: %s\n", SMBStringForField(structHeader, ((SMBBaseBoard *)structHeader)->product, neverMask));
	verbose("\tVersion: %s\n", SMBStringForField(structHeader, ((SMBBaseBoard *)structHeader)->version, neverMask));
	verbose("\tSerial Number: %s\n", SMBStringForField(structHeader, ((SMBBaseBoard *)structHeader)->serialNumber, privateData));
	verbose("\tAsset Tag: %s\n", SMBStringForField(structHeader, ((SMBBaseBoard *)structHeader)->assetTag, neverMask));
// Feature Flags (BYTE)
	verbose("\tLocation In Chassis: %s\n", SMBStringForField(structHeader, ((SMBBaseBoard *)structHeader)->locationInChassis, neverMask)); // Part Component
// Chassis Handle (WORD)
	if ((((SMBBaseBoard *)structHeader)->boardType < kSMBBaseBoardUnknown) || (((SMBBaseBoard *)structHeader)->boardType > kSMBBaseBoardInterconnect)) {
		verbose("\tType: %s\n", OutOfSpecStr);
	} else {
		verbose("\tType: %s\n", SMBBaseBoardTypes[(((SMBBaseBoard *)structHeader)->boardType - 1)]);
	}
// Number of Contained Object Handles (n) (BYTE)
// Contained Object Handles n(WORDs)
	verbose("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// System Enclosure or Chassis (Type 3)
//-------------------------------------------------------------------------------------------------------------------------
void decodeSystemEnclosure(SMBStructHeader *structHeader)
{
	printHeader(structHeader);
	verbose("Chassis Information\n");
	verbose("\tManufacturer: %s\n", SMBStringForField(structHeader, ((SMBSystemEnclosure *)structHeader)->manufacturer, neverMask));
	if ((((SMBSystemEnclosure *)structHeader)->chassisType < kSMBchassisOther) || (((SMBSystemEnclosure *)structHeader)->chassisType > kSMBchassisBladeEnclosing)) {
		verbose("\tType: %s\n", OutOfSpecStr);
	} else {
		verbose("\tType: %s\n", SMBChassisTypes[(((SMBSystemEnclosure *)structHeader)->chassisType - 1)]);
	}
// Lock:
	verbose("\tVersion: %s\n", SMBStringForField(structHeader, ((SMBSystemEnclosure *)structHeader)->version, neverMask));
	verbose("\tSerial Number: %s\n", SMBStringForField(structHeader, ((SMBSystemEnclosure *)structHeader)->serialNumber, privateData));
	verbose("\tAsset Tag: %s\n", SMBStringForField(structHeader, ((SMBSystemEnclosure *)structHeader)->assetTag, neverMask));
// Boot-up State:
// Power Supply State
// Thermal State
// Security Status:
// OEM Information:
// Height;
// Number Of Power Cords: Cords;
// Contained Elements: ElementsCount;
// SKU Number:
// ElementLen;
// Elements[1];         // open array of ElementsCount*ElementLen BYTEs
	verbose("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// Processor Information (Type 4)
//-------------------------------------------------------------------------------------------------------------------------
void decodeProcessorInformation(SMBStructHeader *structHeader)
{
	printHeader(structHeader);
	verbose("Processor Information\n");
	verbose("\tSocket Designation: %s\n", SMBStringForField(structHeader, ((SMBProcessorInformation *)structHeader)->socketDesignation, neverMask));
	if ((((SMBProcessorInformation *)structHeader)->processorType < kSMBprocessorTypeOther) || (((SMBProcessorInformation *)structHeader)->processorType > kSMBprocessorTypeGPU)) {
		verbose("\tType: %s\n", OutOfSpecStr);
	} else {
		verbose("\tType: %s\n", SMBProcessorTypes[((SMBProcessorInformation *)structHeader)->processorType - 1]);
	}
	verbose("\tFamily: 0x%X\n", ((SMBProcessorInformation *)structHeader)->processorFamily);
	verbose("\tManufacturer: %s\n", SMBStringForField(structHeader, ((SMBProcessorInformation *)structHeader)->manufacturer, neverMask));
    uint64_t procID = ((SMBProcessorInformation *)structHeader)->processorID;
//    verbose("\tID: %02X %02X %02X %02X %02X %02X %02X %02X\n", bitfield(procID, 63, 56), bitfield(procID, 55, 48), bitfield(procID, 47, 40), bitfield(procID, 39, 32), bitfield(procID, 31, 24), bitfield(procID, 23, 16), bitfield(procID, 15, 8), bitfield(procID, 7, 0));
    verbose("\tID: %02X %02X %02X %02X %02X %02X %02X %02X\n", (procID >> 56) & 0xFF, (procID >> 48) & 0xFF, (procID >> 40) & 0xFF, (procID >> 32) & 0xFF, (procID >> 24) & 0xFF, (procID >> 16) & 0xFF, (procID >> 8) & 0xFF, (procID >> 0) & 0xFF);
//	verbose("\tSignature: Type=%02X, Family=%02X, Model=%02X, Stepping=%02X\n", (eax >> 12) & 0x3, ((eax >> 20) & 0xFF) + ((eax >> 8) & 0x0F), ((eax >> 12) & 0xF0) + ((eax >> 4) & 0x0F), eax & 0xF);
// Flags:
	verbose("\tVersion: %s\n", SMBStringForField(structHeader, ((SMBProcessorInformation *)structHeader)->processorVersion, neverMask));
//	verbose("\tVoltage: 0.%dV\n", ((SMBProcessorInformation *)structHeader)->voltage);
	verbose("\tExternal Clock: %d MHz\n", ((SMBProcessorInformation *)structHeader)->externalClock);
	verbose("\tMax Speed: %d MHz\n", ((SMBProcessorInformation *)structHeader)->maximumClock);
	verbose("\tCurrent Speed: %d MHz\n", ((SMBProcessorInformation *)structHeader)->currentClock);
// Status: Populated/Unpopulated
	if ((((SMBProcessorInformation *)structHeader)->processorUpgrade < 1) || (((SMBProcessorInformation *)structHeader)->processorUpgrade > 0x2C)) {
		verbose("\tUpgrade: %s\n", OutOfSpecStr);
	} else {
		verbose("\tUpgrade: %s\n", SMBProcessorUpgrades[((SMBProcessorInformation *)structHeader)->processorUpgrade - 1]);
	}
// L1 Cache Handle:
// L2 Cache Handle:
// L3 Cache Handle:
	verbose("\tSerial Number: %s\n", SMBStringForField(structHeader, ((SMBProcessorInformation *)structHeader)->serialNumber, privateData));
	verbose("\tAsset Tag: %s\n", SMBStringForField(structHeader, ((SMBProcessorInformation *)structHeader)->assetTag, neverMask));
	verbose("\tPart Number: %s\n", SMBStringForField(structHeader, ((SMBProcessorInformation *)structHeader)->partNumber, neverMask));
//	if(((SMBProcessorInformation *)structHeader)->coreCount != 0) verbose("\tCore Count: %d\n", ((SMBProcessorInformation *)structHeader)->coreCount);
//	if(((SMBProcessorInformation *)structHeader)->coreEnabled != 0) verbose("\tCore Enabled: %d\n", ((SMBProcessorInformation *)structHeader)->coreEnabled);
//	if(((SMBProcessorInformation *)structHeader)->threadCount != 0) verbose("\tThread Count: %d\n", ((SMBProcessorInformation *)structHeader)->threadCount);
// Characteristics:
//	verbose("\tProcessor Family 2: %d\n", ((SMBProcessorInformation *)structHeader)->processorFamily2);
	verbose("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// Memory Controller Information (Type 5)
//-------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------
// Memory Module Information (Type 6)
//-------------------------------------------------------------------------------------------------------------------------
//void decodeMemoryModule(SMBStructHeader *structHeader)
//{
//	verbose("Memory Module Information\n");
//	verbose("\tSocket Designation: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->socketDesignation));
//	verbose("\tBank Connections: Type: %d\n", structHeader->bankConnections);
//	verbose("\tCurrent Speed: %X\n", structHeader->currentSpeed);
//	verbose("\tType: %llX\n", structHeader->currentMemoryType);
//	verbose("\tInstalled Size: %d\n", structHeader->installedSize);
//	verbose("\tEnabled Size: %d\n", structHeader->enabledSize);
//	verbose("\tError Status: %x\n", structHeader->errorStatus);
//	verbose("\n");
//}

//-------------------------------------------------------------------------------------------------------------------------
// Cache Information (Type 7)
//-------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------
// Port Connector Information (Type 8)
//-------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------
// System Slot Information (Type 9)
//-------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------
// On Board Device Information (Type 10)
//-------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------
// OEM Strings (Type 11)
//-------------------------------------------------------------------------------------------------------------------------
void decodeSMBOEMStrings(SMBStructHeader *structHeader)
{
	char *stringPtr = (char *)structHeader + structHeader->length;
	printHeader(structHeader);
	verbose("OEM Strings\n");
	SMBByte i;
	for (i = 1; i <= ((SMBOEMStrings *)structHeader)->count; i++) {
		verbose("\tString %d: %s\n", i, stringPtr);
		stringPtr = stringPtr + strlen(stringPtr) + 1;
	}
	verbose("\n");
}
//-------------------------------------------------------------------------------------------------------------------------
// System Configuration Options (Type 12)
//-------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------
// BIOS Language Information (Type 13)
//-------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------
// Physical Memory Array (Type 16)
//-------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------
// MemoryDevice (Type 17)
//-------------------------------------------------------------------------------------------------------------------------
void decodeMemoryDevice(SMBStructHeader *structHeader)
{
	printHeader(structHeader);
	verbose("Memory Device\n");
// Aray Handle
	if (((SMBMemoryDevice *)structHeader)->errorHandle == 0xFFFF) {
		verbose("\tError Information Handle: No Error\n");
	} else {
		verbose("\tError Information Handle: 0x%x\n", ((SMBMemoryDevice *)structHeader)->errorHandle);
	}
// Total Width:
// Data Width:
    switch (((SMBMemoryDevice *)structHeader)->memorySize) {
        case 0:
            verbose("\tSize: No Module Installed\n");
            break;
        case 0x7FFF:
            verbose("\tSize: 32GB or more\n");
            break;
        case 0xFFFF:
            verbose("\tSize: Unknown\n");
            break;
        default:
            verbose("\tSize: %d %s\n", ((SMBMemoryDevice *)structHeader)->memorySize & 0x7FFF, ((((SMBMemoryDevice *)structHeader)->memorySize & 0x8000) == 0x8000) ? "kB" : "MB");
            break;
    }
    if ((((SMBMemoryDevice *)structHeader)->formFactor < 0x01) || (((SMBMemoryDevice *)structHeader)->formFactor > 0x0F)) {
 		verbose("\tForm Factor: %s\n", OutOfSpecStr);
 	} else {
        verbose("\tForm Factor: %s\n", SMBMemoryDeviceFormFactors[((SMBMemoryDevice *)structHeader)->formFactor - 1]);
      }
// Set:
	verbose("\tLocator: %s\n", SMBStringForField(structHeader, ((SMBMemoryDevice *)structHeader)->deviceLocator, neverMask));
	verbose("\tBank Locator: %s\n", SMBStringForField(structHeader, ((SMBMemoryDevice *)structHeader)->bankLocator, neverMask));
	if (((SMBMemoryDevice *)structHeader)->memoryType > kSMBMemoryDeviceTypeCount) {
		verbose("\tMemory Type: %s\n", OutOfSpecStr);
	} else {
		verbose("\tMemory Type: %s\n", SMBMemoryDeviceTypes[((SMBMemoryDevice *)structHeader)->memoryType]);
	}
// Type Detail:
	verbose("\tSpeed: %d MHz\n", ((SMBMemoryDevice *)structHeader)->memorySpeed);
	verbose("\tManufacturer: %s\n", SMBStringForField(structHeader, ((SMBMemoryDevice *)structHeader)->manufacturer, neverMask));
	verbose("\tSerial Number: %s\n", SMBStringForField(structHeader, ((SMBMemoryDevice *)structHeader)->serialNumber, privateData));
	verbose("\tAsset Tag: %s\n", SMBStringForField(structHeader, ((SMBMemoryDevice *)structHeader)->assetTag, neverMask));
	verbose("\tPart Number: %s\n", SMBStringForField(structHeader, ((SMBMemoryDevice *)structHeader)->partNumber, neverMask));
// Rank:
// Configured Clock Speed:
	verbose("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// Apple Specific  (Type 131)
//-------------------------------------------------------------------------------------------------------------------------
void decodeOemProcessorType(SMBStructHeader *structHeader)
{
	printHeader(structHeader);
	verbose("Apple specific Processor Type\n");
	verbose("\tcpu-type = 0x%04X\n", ((SMBOemProcessorType *)structHeader)->ProcessorType);
	verbose("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// Apple Specific  (Type 132)
//-------------------------------------------------------------------------------------------------------------------------
void decodeOemProcessorBusSpeed(SMBStructHeader *structHeader)
{
	printHeader(structHeader);
	verbose("Apple specific Processor Interconnect Speed\n");
	verbose("\tqpi = %d MT/s\n", ((SMBOemProcessorBusSpeed *)structHeader)->ProcessorBusSpeed);
	verbose("\n");
}

// Info for the Table Above: dmi 2.7+ https://wiki.debian.org/InstallingDebianOn/Thinkpad/T42/lenny?action=AttachFile&do=get&target=dmidecode.Lenny_Thinkpad_T42_2373.txt
//-------------------------------------------------------------------------------------------------------------------------
// Apple Specific  (Type 133)
//-------------------------------------------------------------------------------------------------------------------------
//void decodeOemPlatformFeature(SMBStructHeader *structHeader)
//{
//	printHeader(structHeader);
//	verbose("Apple specific Platform Feature\n");
//	verbose("\t%s\n", ((SMBOemPlatformFeature *)structHeader)->PlatformFeature);
//	verbose("\n");
//}

//-------------------------------------------------------------------------------------------------------------------------
// Specific (Type 134)
//-------------------------------------------------------------------------------------------------------------------------
//void decodeOem(SMBStructHeader *structHeader)
//{
//	printHeader(structHeader);
//	verbose("Apple specific Feature\n");
//	verbose("\t%s\n", ((SMBOemPlatformFeature *)structHeader)->Feature);
//	verbose("\n");
//}

//-------------------------------------------------------------------------------------------------------------------------


void decodeSMBIOSTable(SMBEntryPoint *eps)
{
	uint8_t *ptr = (uint8_t *)eps->dmi.tableAddress;
	SMBStructHeader *structHeader = (SMBStructHeader *)ptr;

	minorVersion = eps->minorVersion;
	majorVersion = eps->majorVersion;
	bcdRevisionHi = eps->dmi.bcdRevision >> 4;
	bcdRevisionLo = eps->dmi.bcdRevision & 0x0F;

	getBoolForKey(kPrivateData, &privateData, &bootInfo->chameleonConfig);  // Bungo: chek if mask some data

    verbose("\n");
	for (;((eps->dmi.tableAddress + eps->dmi.tableLength) > ((uint32_t)(uint8_t *)structHeader + sizeof(SMBStructHeader)));)
	{
		switch (structHeader->type)
		{
			case kSMBTypeBIOSInformation: // Type 0
				decodeBIOSInformation(structHeader);
				break;

			case kSMBTypeSystemInformation: // Type 1
				decodeSystemInformation(structHeader);
				break;

			case kSMBTypeBaseBoard: // Type 2
				decodeBaseBoard(structHeader);
				break;

			case kSMBTypeSystemEnclosure: // Type 3
				decodeSystemEnclosure(structHeader);
				break;

			case kSMBTypeProcessorInformation: // Type 4
				decodeProcessorInformation(structHeader);
				break;

			//case kSMBTypeMemoryModule: // Type 6
			//	decodeMemoryModule(structHeader);
			//	break;

			//case kSMBTypeSystemSlot: // Type 9
			//	decodeSMBTypeSystemSlot(structHeader);
			//	break;

			case kSMBOEMStrings: // Type 11
				decodeSMBOEMStrings(structHeader);
				break;

			case kSMBTypeMemoryDevice: // Type 17
				decodeMemoryDevice(structHeader);
				break;

			//kSMBTypeMemoryArrayMappedAddress: // Type 19
			//	break;

			/* Skip all Apple Specific Structures */
			// case kSMBTypeFirmwareVolume: // Type 128
			// case kSMBTypeMemorySPD: // Type 130
			//	break;

			case kSMBTypeOemProcessorType: // Type 131
				decodeOemProcessorType(structHeader);
				break;

			case kSMBTypeOemProcessorBusSpeed: // Type 132
				decodeOemProcessorBusSpeed(structHeader);
				break;

			//kSMBTypeOemPlatformFeature: // Type 133
			//	decodeOemPlatformFeature(structHeader);
			//	break;

			case kSMBTypeEndOfTable: // Type 127
                printHeader(structHeader);
				//verbose("Handle 0x%04x, DMI type %d, %d  bytes\n", structHeader->handle, structHeader->type, structHeader->length);
				verbose("End of Table\n");
				break;

			default:
				break;
		}

		ptr = (uint8_t *)((uint32_t)structHeader + structHeader->length);
		for (; ((uint16_t *)ptr)[0] != 0; ptr++);

		if (((uint16_t *)ptr)[0] == 0) {
			ptr += 2;
		}

		structHeader = (SMBStructHeader *)ptr;
	}
    if (gVerboseMode) pause("");
	verbose("\n");
}

