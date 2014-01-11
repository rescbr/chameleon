/*
 * A very simple SMBIOS Table decoder, part of the Chameleon Boot Loader Project
 *
 * Copyright 2010 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */

#include "libsaio.h"
#include "smbios.h"

#ifndef DEBUG_SMBIOS
#define DEBUG_SMBIOS 0
#endif

#if DEBUG_SMBIOS
#define DBG(x...)	printf(x)
#else
#define DBG(x...)	msglog(x)
#endif

static SMBWord minorVersion;

extern char *getSMBStringForField(SMBStructHeader *structHeader, uint8_t field);

// Bungo: fixes random string readout if null in smbios to "Not Specified" as dmidecode dispays
#define NotSpecifiedStr      "Not Specified"

char *SMBStringForField(SMBStructHeader *structHeader, uint8_t field)
{
    char  *str;
    str = getSMBStringForField(structHeader, field);
    if (str == 0)
        str = NotSpecifiedStr;

    return str;
};
//
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
    "RAM",			/* 15h  unused */
    "RAM",			/* 16h  unused */
    "RAM",			/* 17h  unused */
    "DDR3",			/* 18h  DDR3, chosen in [5776134] */
    "FBD2"			/* 19h  FBD2 */
};

static const int kSMBMemoryDeviceTypeCount = sizeof(SMBMemoryDeviceTypes)   /
                            sizeof(SMBMemoryDeviceTypes[0]);

//-------------------------------------------------------------------------------------------------------------------------
// BIOS Information (Type 0)
//-------------------------------------------------------------------------------------------------------------------------
void decodeBIOSInformation(SMBBIOSInformation *structHeader)
{
	DBG("BIOS Information:\n");
	DBG("\tVendor: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->vendor));
	DBG("\tVersion: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->version));
	// Address Segment
	DBG("\tRelease Date: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->releaseDate));
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
	DBG("\tManufacturer: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer));
	DBG("\tProduct Name: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->productName));
	DBG("\tVersion: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->version));
	DBG("\tSerial Number:  ** PRIVATE **\n"); //%s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber));

	if (minorVersion < 1 || structHeader->header.length < 25)
		return;
	uint8_t *uuid = structHeader->uuid;
	if (uuid) {
		DBG("\tuuid: %02X%02X%02X%02X-%02X%02X-%02X%02X-%02x%02X-%02X%02X%02X%02X%02X%02X\n",
			uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
			uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
	}
	DBG("\tWake-up Type: 0x%x\n", structHeader->wakeupReason);

	if (minorVersion < 4 || structHeader->header.length < 27)
		return;
	DBG("\tSKU Number: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->skuNumber)); // System SKU#
	DBG("\tFamily: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->family));
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// Base Board (or Module) Information (Type 2)
//-------------------------------------------------------------------------------------------------------------------------
void decodeBaseBoard(SMBBaseBoard *structHeader)
{
	DBG("Base Board Information:\n");
	DBG("\tManufacturer: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer));
	DBG("\tProduct Name: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->product));
	DBG("\tVersion: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->version));
	DBG("\tSerial Number: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber));
	DBG("\tAsset Tag: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->assetTagNumber));
	// Feature Flags (BYTE)
	DBG("\tLocation In Chassis: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->locationInChassis)); // Part Component
	// Chassis Handle (WORD)
	DBG("\tType: 0x%X\n", structHeader->boardType);
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
	DBG("\tManufacturer: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer));
	DBG("\tType: 0x%X\n", structHeader->chassisType);
	DBG("\tVersion: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->version));
	DBG("\tSerial Number:  ** PRIVATE **\n"); //%s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber));
	DBG("\tAsset Tag Number: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->assetTagNumber));
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
	DBG("\tSocket Designation: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->socketDesignation));
	DBG("\tType: %d\n", structHeader->processorType);
	DBG("\tFamily: 0x%X\n", structHeader->processorFamily);
	DBG("\tManufacturer: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer));
	DBG("\tID: 0x%llX\n", structHeader->processorID);
	DBG("\tProcessor Version: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->processorVersion));
//	DBG("\tVoltage: 0.%xV\n", structHeader->voltage);
	DBG("\tExternal Clock: %dMHz\n", structHeader->externalClock);
	DBG("\tMaximum Clock: %dMHz\n", structHeader->maximumClock);
	DBG("\tCurrent Clock: %dMHz\n", structHeader->currentClock);

	if (minorVersion < 3 || structHeader->header.length < 35)
		return;
	DBG("\tSerial Number: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber));
	DBG("\tAsset Tag: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->assetTag));
	DBG("\tPart Number: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->partNumber));
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
	DBG("\tDevice Locator: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->deviceLocator));
	DBG("\tBank Locator: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->bankLocator));
	DBG("\tMemory Type: %s\n", SMBMemoryDeviceTypes[structHeader->memoryType]);

	if (minorVersion < 3 || structHeader->header.length < 27)
		return;
	DBG("\tSpeed: %d MHz\n", structHeader->memorySpeed);
	DBG("\tError Handle: %x\n", structHeader->errorHandle);
	DBG("\tManufacturer: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer));
	DBG("\tSerial Number: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber));
	DBG("\tAsset Tag: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->assetTag));
	DBG("\tPart Number: %s\n", SMBStringForField((SMBStructHeader *)structHeader, structHeader->partNumber));
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// Apple Specific
//-------------------------------------------------------------------------------------------------------------------------
void decodeOemProcessorType(SMBOemProcessorType *structHeader)
{
	DBG("AppleProcessorType:\n");
	DBG("\tProcessorType: 0x%x\n", ((SMBOemProcessorType *)structHeader)->ProcessorType);
	DBG("\n");
}

void decodeOemProcessorBusSpeed(SMBOemProcessorBusSpeed *structHeader)
{
	DBG("AppleProcessorBusSpeed:\n");
	DBG("\tProcessorBusSpeed (QPI): %d.%dGT/s\n", 
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

	DBG("\n");
	for (;((eps->dmi.tableAddress + eps->dmi.tableLength) > ((uint32_t)(uint8_t *)structHeader + sizeof(SMBStructHeader)));)
	{
		DBG("Type: %d, Length: %d, Handle: 0x%x\n", 
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

