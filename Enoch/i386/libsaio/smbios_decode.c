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

static SMBByte      minorVersion;   // SMBIOS rev. minor
// Bungo:
static SMBByte      majorVersion;   // SMBIOS rev. major
static SMBByte      bcdRevisionLo;  // DMI rev. minor
static SMBByte      bcdRevisionHi;  // DMI rev. major

extern char *getSMBStringForField(SMBStructHeader *structHeader, uint8_t field);

// Bungo:
#define         NotSpecifiedStr     "Not Specified"  // no string
#define         OutOfSpecStr        "<OUT OF SPEC>"  // value out of smbios spec. range
#define         PrivateStr          "** PRIVATE **"  // masking private data
#define         alwaysMask          true
#define         neverMask           false
static bool     privateData         = true;

char *SMBStringForField(SMBStructHeader *structHeader, uint8_t field, const bool mask)  // Bungo: fixes random string readout if null in smbios to "Not Specified" as dmidecode displays
{
	char  *str = NULL;
	str = getSMBStringForField(structHeader, field);
	if (!field) {
		str = NotSpecifiedStr;
	} else if (mask) {
		str = PrivateStr;
	}

    return str;
};

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
 7.5.5
 ===*/
/*static const char *SMBCpuSocket[] =  // ErmaC: strings for (Table Type 4 - Processor Information )
{
	"Other",                // 01h
    "Unknown",
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
    "Socket LGA1356-3"	// 2Ch
};*/

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

static const int kSMBMemoryDeviceTypeCount = sizeof(SMBMemoryDeviceTypes)   /
                            sizeof(SMBMemoryDeviceTypes[0]);

//-------------------------------------------------------------------------------------------------------------------------
// BIOS Information (Type 0)
//-------------------------------------------------------------------------------------------------------------------------
void decodeBIOSInformation(SMBBIOSInformation *structHeader)
{
	DBG("BIOS Information:\n");
	DBG("\tVendor: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->vendor, neverMask));
	DBG("\tVersion: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->version, neverMask));
	// Address Segment
	DBG("\tRelease Date: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->releaseDate, neverMask));
	DBG("\tBIOS Revision: %d.%d\n", structHeader->releaseMajor, structHeader->releaseMinor);
	// ROM Size
	//DBG("\tSupported BIOS functions: (0x%llX) %s\n", structHeader->characteristics, SMBBIOSInfoChar0[structHeader->characteristics]);
	// Major Release
	// Minor Release
	// Firmware Major Release
	// Firmware Minor Release
	//SMBByte    characteristicsExt1;
	//SMBByte    characteristicsExt2;
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// System Information (Type 1)
//-------------------------------------------------------------------------------------------------------------------------
void decodeSystemInformation(SMBSystemInformation *structHeader)
{
	DBG("System Information:\n");
	DBG("\tManufacturer: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer, neverMask));
	DBG("\tProduct Name: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->productName, neverMask));
	DBG("\tVersion: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->version, neverMask));
	DBG("\tSerial Number: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber, privateData));

	if (minorVersion < 1 || structHeader->header.length < 25) {
		return;
	}

	uint8_t *uuid = structHeader->uuid;
	if (uuid) {
		if (privateData) {
			DBG("\tUUID: %s\n", PrivateStr);
		} else {
			DBG("\tUUID: %02X%02X%02X%02X-%02X%02X-%02X%02X-%02x%02X-%02X%02X%02X%02X%02X%02X\n",
				uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
				uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
		}
	}

	if (structHeader->wakeupReason > 8) {
		DBG("\tWake-up Type: %s\n", OutOfSpecStr);
	} else {
		DBG("\tWake-up Type: %s\n", SMBWakeUpTypes[structHeader->wakeupReason]);
	}
	if (minorVersion < 4 || structHeader->header.length < 27) {
		return;
	}

	DBG("\tSKU Number: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->skuNumber, neverMask)); // System SKU#
	DBG("\tFamily: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->family, neverMask));
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// Base Board (or Module) Information (Type 2)
//-------------------------------------------------------------------------------------------------------------------------
void decodeBaseBoard(SMBBaseBoard *structHeader)
{
	DBG("Base Board Information:\n");
	DBG("\tManufacturer: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer, neverMask));
	DBG("\tProduct Name: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->product, neverMask));
	DBG("\tVersion: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->version, neverMask));
	DBG("\tSerial Number: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber, privateData));
	DBG("\tAsset Tag: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->assetTag, neverMask));
	// Feature Flags (BYTE)
	DBG("\tLocation In Chassis: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->locationInChassis, neverMask)); // Part Component
	// Chassis Handle (WORD)
	if ((structHeader->boardType < kSMBBaseBoardUnknown) || (structHeader->boardType > kSMBBaseBoardInterconnect)) {
		DBG("\tType: %s\n", OutOfSpecStr);
	} else {
		DBG("\tType: %s\n", SMBBaseBoardTypes[(structHeader->boardType - 1)]);
	}
	// Number of Contained Object Handles (n) (BYTE)
	// Contained Object Handles n(WORDs)
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// System Enclosure or Chassis (Type 3)
//-------------------------------------------------------------------------------------------------------------------------
void decodeSystemEnclosure(SMBSystemEnclosure *structHeader)
{
	DBG("Chassis Information:\n");
	DBG("\tManufacturer: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer, neverMask));
	// DBG("\tType: 0x%X\n", structHeader->chassisType);
	if ((structHeader->chassisType < kSMBchassisOther) || (structHeader->chassisType > kSMBchassisBladeEnclosing)) {
		DBG("\tType: %s\n", OutOfSpecStr);
	} else {
		DBG("\tType: %s\n", SMBChassisTypes[(structHeader->chassisType - 1)]);
	}
	DBG("\tVersion: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->version, neverMask));
	DBG("\tSerial Number: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber, privateData));
	DBG("\tAsset Tag: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->assetTag, neverMask));
	// Boot-up State:
	// Power Supply State
	// Thermal State
	// Security Status:
	// OEM Information:
	// Height;
	// Cords;
	// ElementsCount;
	// ElementLen;
	// Elements[1];         // open array of ElementsCount*ElementLen BYTEs
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// Processor Information (Type 4)
//-------------------------------------------------------------------------------------------------------------------------
void decodeProcessorInformation(SMBProcessorInformation *structHeader)
{
	DBG("Processor Information:\n");
	DBG("\tSocket Designation: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->socketDesignation, neverMask));
	DBG("\tType: %d\n", structHeader->processorType);
	DBG("\tFamily: 0x%X\n", structHeader->processorFamily);
	DBG("\tManufacturer: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer, neverMask));
	DBG("\tID: 0x%llX\n", structHeader->processorID);
	DBG("\tProcessor Version: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->processorVersion, neverMask));
//	DBG("\tVoltage: 0.%xV\n", structHeader->voltage);
	DBG("\tExternal Clock: %dMHz\n", structHeader->externalClock);
	DBG("\tMaximum Clock: %dMHz\n", structHeader->maximumClock);
	DBG("\tCurrent Clock: %dMHz\n", structHeader->currentClock);

	if (minorVersion < 3 || structHeader->header.length < 35) {
		return;
	}

	DBG("\tSerial Number: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber, privateData));
	DBG("\tAsset Tag: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->assetTag, neverMask));
	DBG("\tPart Number: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->partNumber, neverMask));
//	DBG("\tProcessor Family 2: %d\n", structHeader->processorFamily2);
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// Memory Module Information (Type 6)
//-------------------------------------------------------------------------------------------------------------------------
//void decodeMemoryModule(SMBMemoryModule *structHeader)
//{
//	DBG("Memory Module Information:\n");
//	DBG("\tSocket Designation: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->socketDesignation));
//	DBG("\tBank Connections: Type: %d\n", structHeader->bankConnections);
//	DBG("\tCurrent Speed: %X\n", structHeader->currentSpeed);
//	DBG("\tCurrent Memory Type: %llX\n", structHeader->currentMemoryType);
//	DBG("\tInstalled Size: %d\n", structHeader->installedSize);
//	DBG("\tEnabled Size: %d\n", structHeader->enabledSize);
//	DBG("\tError Status: %x\n", structHeader->errorStatus);
//	DBG("\n");
//}

//-------------------------------------------------------------------------------------------------------------------------
// OEM Strings (Type 11)
//-------------------------------------------------------------------------------------------------------------------------
//void decodeSMBOEMStrings(SMBOEMStrings *structHeader)
//{
//	DBG("OEM Strings:\n");
//	DBG("\tString 1: %d\n"); //, structHeader->string1);
//	DBG("\tString 2: %d\n"); //, structHeader->string1);
//	DBG("\tString 3: %d\n"); //, structHeader->string1);
//	DBG("\n");
//}

//-------------------------------------------------------------------------------------------------------------------------
// MemoryDevice (Type 17)
//-------------------------------------------------------------------------------------------------------------------------
void decodeMemoryDevice(SMBMemoryDevice *structHeader)
{
	DBG("Memory Device:\n");
	DBG("\tDevice Locator: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->deviceLocator, neverMask));
	DBG("\tBank Locator: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->bankLocator, neverMask));
	DBG("\tMemory Type: %s\n", SMBMemoryDeviceTypes[structHeader->memoryType]);

	if (minorVersion < 3 || structHeader->header.length < 27) {
		return;
	}
	DBG("\tSpeed: %d MHz\n", structHeader->memorySpeed);
	DBG("\tError Handle: %x\n", structHeader->errorHandle);
	DBG("\tManufacturer: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer, neverMask));
	DBG("\tSerial Number: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber, privateData));
	DBG("\tAsset Tag: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->assetTag, neverMask));
	DBG("\tPart Number: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->partNumber, neverMask));
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// Apple Specific
//-------------------------------------------------------------------------------------------------------------------------
void decodeOemProcessorType(SMBOemProcessorType *structHeader)
{
	DBG("Apple specific Processor Type:\n");
	DBG("\tCpu-type: 0x%x\n", ((SMBOemProcessorType *)structHeader)->ProcessorType);
	DBG("\n");
}

void decodeOemProcessorBusSpeed(SMBOemProcessorBusSpeed *structHeader)
{
	DBG("Apple specific Processor Interconnect Speed:\n");
	DBG("\tQPI = %d.%dGT/s\n",
			((SMBOemProcessorBusSpeed *)structHeader)->ProcessorBusSpeed / 1000, 
			(((SMBOemProcessorBusSpeed *)structHeader)->ProcessorBusSpeed / 100) % 10);
	DBG("\n");
}
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

	DBG("\n");
	DBG("SMBIOS rev. %d.%d, DMI rev. %d.%d\n", majorVersion, minorVersion, bcdRevisionHi, bcdRevisionLo);
	DBG("\n");
	for (;((eps->dmi.tableAddress + eps->dmi.tableLength) > ((uint32_t)(uint8_t *)structHeader + sizeof(SMBStructHeader)));)
	{
		DBG("Type: %d, Length: %d, Handle: 0x%04x\n", 
				structHeader->type, structHeader->length, structHeader->handle);

		switch (structHeader->type)
		{
			case kSMBTypeBIOSInformation: // Type 0
				decodeBIOSInformation((SMBBIOSInformation *)structHeader);
				break;

			case kSMBTypeSystemInformation: // Type 1
				decodeSystemInformation((SMBSystemInformation *)structHeader);
				break;

			case kSMBTypeBaseBoard: // Type 2
				decodeBaseBoard((SMBBaseBoard *)structHeader);
				break;

			case kSMBTypeSystemEnclosure: // Type 3
				decodeSystemEnclosure((SMBSystemEnclosure *)structHeader);
				break;

			case kSMBTypeProcessorInformation: // Type 4
				decodeProcessorInformation((SMBProcessorInformation *)structHeader);
				break;

			//case 6: // kSMBTypeMemoryModule: // Type 6
			//	decodeMemoryModule((SMBMemoryModule *)structHeader);
			//	break;

			//case 11: // kSMBOEMStrings: // Type 11
			//	decodeSMBOEMStrings((SMBOEMStrings *)structHeader);
			//	break;

			case kSMBTypeMemoryDevice: // Type 17
				decodeMemoryDevice((SMBMemoryDevice *)structHeader);
				break;

			//kSMBTypeMemoryArrayMappedAddress: // Type 19

			/* Skip all Apple Specific Structures */
			case kSMBTypeFirmwareVolume: // Type 128
			case kSMBTypeMemorySPD: // Type 130
				break;

			case kSMBTypeOemProcessorType: // Type 131
				decodeOemProcessorType((SMBOemProcessorType *)structHeader);
				break;

			case kSMBTypeOemProcessorBusSpeed: // Type 132
				decodeOemProcessorBusSpeed((SMBOemProcessorBusSpeed *)structHeader);
				break;

			//kSMBTypeOemPlatformFeature: // Type 133

			case kSMBTypeEndOfTable: // Type 127
				/* Skip, to be added at the end */
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
	DBG("\n");
}

