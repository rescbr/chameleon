/*
 * A very simple SMBIOS Table decoder, part of the Chameleon Boot Loader Project
 *
 * Copyright 2010 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */

#include "libsaio.h"
#include "mysmbios.h"

#ifndef DEBUG_SMBIOS
#define DEBUG_SMBIOS 0
#endif

#if DEBUG_SMBIOS
#define DBG(x...)	printf(x)
#else
#define DBG(x...)	msglog(x)
#endif


static SMBWord minorVersion;

static void decodeBIOSInformation(SMBBIOSInformation *structHeader);
static void decodeSystemInformation(SMBSystemInformation *structHeader);
static void decodeBaseBoard(SMBBaseBoard *structHeader);
static void decodeSystemEnclosure(SMBSystemEnclosure *structHeader);
static void decodeProcessorInformation(SMBProcessorInformation *structHeader);
static void decodeMemoryDevice(SMBMemoryDevice *structHeader);
static void decodeOemProcessorType(SMBOemProcessorType *structHeader);
static void decodeOemProcessorBusSpeed(SMBOemProcessorBusSpeed *structHeader);

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
};

//-------------------------------------------------------------------------------------------------------------------------
// BIOSInformation
//-------------------------------------------------------------------------------------------------------------------------
static void decodeBIOSInformation(SMBBIOSInformation *structHeader)
{
	DBG("BIOSInformation:\n");
	DBG("\tvendor: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->vendor));
	DBG("\tversion: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->version));
	DBG("\treleaseDate: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->releaseDate));
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// SystemInformation
//-------------------------------------------------------------------------------------------------------------------------
static void decodeSystemInformation(SMBSystemInformation *structHeader)
{
	DBG("SystemInformation:\n");
	DBG("\tmanufacturer: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer));
	DBG("\tproductName: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->productName));
	DBG("\tversion: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->version));
	DBG("\tserialNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber));

	if (minorVersion < 1 || structHeader->header.length < 25)
		return;
	uint8_t *uuid = structHeader->uuid;
	DBG("\tuuid: %02X%02X%02X%02X-%02X%02X-%02X%02X-%02x%02X-%02X%02X%02X%02X%02X%02X\n",
		uuid[0], uuid[1], uuid[2], uuid[3],  
		uuid[4], uuid[5], 
		uuid[6], uuid[7], 
		uuid[8], uuid[9], 
		uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
	DBG("\twakeupReason: 0x%x\n", structHeader->wakeupReason);

	if (minorVersion < 4 || structHeader->header.length < 27)
		return;
	DBG("\tskuNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->skuNumber));
	DBG("\tfamily: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->family));
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// BaseBoard
//-------------------------------------------------------------------------------------------------------------------------
static void decodeBaseBoard(SMBBaseBoard *structHeader)
{
	DBG("BaseBoard:\n");
	DBG("\tmanufacturer: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer));
	DBG("\tproduct: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->product));
	DBG("\tversion: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->version));
	DBG("\tserialNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber));
	DBG("\tassetTagNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->assetTagNumber));
	DBG("\tlocationInChassis: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->locationInChassis));
	DBG("\tboardType: 0x%X\n", structHeader->boardType);
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// SystemEnclosure
//-------------------------------------------------------------------------------------------------------------------------
static void decodeSystemEnclosure(SMBSystemEnclosure *structHeader)
{
	DBG("SystemEnclosure:\n");
	DBG("\tmanufacturer: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer));
	DBG("\ttype: %d\n", structHeader->type);
	DBG("\tversion: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->version));
	DBG("\tserialNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber));
	DBG("\tassetTagNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->assetTagNumber));
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// ProcessorInformation
//-------------------------------------------------------------------------------------------------------------------------
static void decodeProcessorInformation(SMBProcessorInformation *structHeader)
{
	DBG("ProcessorInformation:\n");
	DBG("\tsocketDesignation: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->socketDesignation));
	DBG("\tprocessorType: %d\n", structHeader->processorType);
	DBG("\tprocessorFamily: 0x%X\n", structHeader->processorFamily);
	DBG("\tmanufacturer: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer));
	DBG("\tprocessorID: 0x%llX\n", structHeader->processorID);
	DBG("\tprocessorVersion: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->processorVersion));
	DBG("\texternalClock: %dMHz\n", structHeader->externalClock);
	DBG("\tmaximumClock: %dMHz\n", structHeader->maximumClock);
	DBG("\tcurrentClock: %dMHz\n", structHeader->currentClock);

	if (minorVersion < 3 || structHeader->header.length < 35)
		return;
	DBG("\tserialNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber));
	DBG("\tassetTag: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->assetTag));
	DBG("\tpartNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->partNumber));
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// MemoryDevice
//-------------------------------------------------------------------------------------------------------------------------
static void decodeMemoryDevice(SMBMemoryDevice *structHeader)
{
	DBG("MemoryDevice:\n");
	DBG("\tdeviceLocator: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->deviceLocator));
	DBG("\tbankLocator: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->bankLocator));
	DBG("\tmemoryType: %s\n", SMBMemoryDeviceTypes[structHeader->memoryType]);

	if (minorVersion < 3 || structHeader->header.length < 27)
		return;
	DBG("\tmemorySpeed: %dMHz\n", structHeader->memorySpeed);
	DBG("\tmanufacturer: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->manufacturer));
	DBG("\tserialNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->serialNumber));
	DBG("\tassetTag: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->assetTag));
	DBG("\tpartNumber: %s\n", getSMBStringForField((SMBStructHeader *)structHeader, structHeader->partNumber));
	DBG("\n");
}

//-------------------------------------------------------------------------------------------------------------------------
// Apple Specific
//-------------------------------------------------------------------------------------------------------------------------
static void decodeOemProcessorType(SMBOemProcessorType *structHeader)
{
	DBG("AppleProcessorType:\n");
	DBG("\tProcessorType: 0x%x\n", ((SMBOemProcessorType *)structHeader)->ProcessorType);
	DBG("\n");
}

static void decodeOemProcessorBusSpeed(SMBOemProcessorBusSpeed *structHeader)
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
#if DEBUG_SMBIOS
		DBG("Type: %d, Length: %d, Handle: 0x%x\n", 
				structHeader->type, structHeader->length, structHeader->handle);
#endif
		switch (structHeader->type)
		{
			case kSMBTypeBIOSInformation:
            {
#if !DEBUG_SMBIOS
                DBG("Type: %d, Length: %d, Handle: 0x%x\n", 
                    structHeader->type, structHeader->length, structHeader->handle);
#endif
				decodeBIOSInformation((SMBBIOSInformation *)structHeader);
				break;
            }
			case kSMBTypeSystemInformation:
            {
#if !DEBUG_SMBIOS
                DBG("Type: %d, Length: %d, Handle: 0x%x\n", 
                    structHeader->type, structHeader->length, structHeader->handle);
#endif
				decodeSystemInformation((SMBSystemInformation *)structHeader);
				break;
            }
			case kSMBTypeBaseBoard:
            {
#if !DEBUG_SMBIOS
                DBG("Type: %d, Length: %d, Handle: 0x%x\n", 
                    structHeader->type, structHeader->length, structHeader->handle);
#endif
				decodeBaseBoard((SMBBaseBoard *)structHeader);
				break;
            }
			case kSMBTypeSystemEnclosure:
            {
#if !DEBUG_SMBIOS
                DBG("Type: %d, Length: %d, Handle: 0x%x\n", 
                    structHeader->type, structHeader->length, structHeader->handle);
#endif
				decodeSystemEnclosure((SMBSystemEnclosure *)structHeader);
				break;
            }
			case kSMBTypeProcessorInformation:
            {
#if !DEBUG_SMBIOS
                DBG("Type: %d, Length: %d, Handle: 0x%x\n", 
                    structHeader->type, structHeader->length, structHeader->handle);
#endif
				decodeProcessorInformation((SMBProcessorInformation *)structHeader);
				break;
            }
			case kSMBTypeMemoryDevice:
            {
#if !DEBUG_SMBIOS
                DBG("Type: %d, Length: %d, Handle: 0x%x\n", 
                    structHeader->type, structHeader->length, structHeader->handle);
#endif
				decodeMemoryDevice((SMBMemoryDevice *)structHeader);
				break;

			/* Skip all Apple Specific Structures */
			case kSMBTypeFirmwareVolume:
			case kSMBTypeMemorySPD:
				break;
            }
			case kSMBTypeOemProcessorType:
            {
#if !DEBUG_SMBIOS
                DBG("Type: %d, Length: %d, Handle: 0x%x\n", 
                    structHeader->type, structHeader->length, structHeader->handle);
#endif
				decodeOemProcessorType((SMBOemProcessorType *)structHeader);
				break;
            }
			case kSMBTypeOemProcessorBusSpeed:
            {
#if !DEBUG_SMBIOS 
                DBG("Type: %d, Length: %d, Handle: 0x%x\n", 
                    structHeader->type, structHeader->length, structHeader->handle);
#endif
				if (((SMBOemProcessorBusSpeed *)structHeader)->ProcessorBusSpeed)
				decodeOemProcessorBusSpeed((SMBOemProcessorBusSpeed *)structHeader);
				break;
            }
			case kSMBTypeEndOfTable:
				/* Skip, to be added at the end */
				break;

			default:
				break;
		}

		ptr = (uint8_t *)((uint32_t)structHeader + structHeader->length);
		for (; ((uint16_t *)ptr)[0] != 0; ptr++);

		if (((uint16_t *)ptr)[0] == 0)
			ptr += 2;

		structHeader = (SMBStructHeader *)ptr;
	}
	DBG("\n");
}

