/*
 * Copyright (c) 1998-2006 Apple Computer, Inc. All rights reserved.
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

/* This file is a stripped-down version of the one found in the AppleSMBIOS project.
 * Changes:
 * - Don't use pragma pack but instead use GCC's packed attribute
 */

#ifndef _LIBSAIO_SMBIOS_H
#define _LIBSAIO_SMBIOS_H

#include "libsaio.h"

#define kIsServer			"IsServer"	
extern struct SMBEntryPoint *getSmbiosOriginal(void);

#define theUUID 0
#define thePlatformName 1
#define theProducBoard 2
extern int readSMBIOS(int value); // value copied into the platform structure
extern char* readDefaultPlatformName(void);

#define SMBIOS_RANGE_START		0x000F0000
#define SMBIOS_RANGE_END		0x000FFFFF

/* '_SM_' in little endian: */
//#define SMBIOS_ANCHOR_UINT32_LE 0x5f4d535f

// getting smbios addr with fast compare ops, late checksum testing ...
#define COMPARE_DWORD(a,b) ( *((u_int32_t *) a) == *((u_int32_t *) b) )


//
// SMBIOS structure types.
//

enum {
    kSMBTypeBIOSInformation             =  0,
    kSMBTypeSystemInformation           =  1,
    kSMBTypeBaseBoard					=  2,
    kSMBTypeSystemEnclosure             =  3,
    kSMBTypeProcessorInformation        =  4,
    kSMBTypeMemoryModule                =  6,
    kSMBTypeCacheInformation            =  7,
    kSMBTypeSystemSlot                  =  9,
    kSMBTypePhysicalMemoryArray         = 16,
    kSMBTypeMemoryDevice                = 17,
    kSMBType32BitMemoryErrorInfo        = 18,
    kSMBType64BitMemoryErrorInfo        = 33,
	
    kSMBTypeEndOfTable                  = 127,
	
    /* Apple Specific Structures */
    kSMBTypeFirmwareVolume              = 128,
    kSMBTypeMemorySPD                   = 130,
    kSMBTypeOemProcessorType            = 131,
    kSMBTypeOemProcessorBusSpeed        = 132
};

/*
 * Based on System Management BIOS Reference Specification v2.5
 */

typedef uint8_t		SMBString;
typedef uint8_t		SMBByte;
typedef uint16_t	SMBWord;
typedef uint32_t	SMBDWord;
typedef uint64_t	SMBQWord;

typedef struct DMIHeader {
	SMBByte			type;
	SMBByte			length;
	SMBWord			handle;
} __attribute__((packed)) DMIHeader;

#define DMI_STRUCT_HEADER  DMIHeader dmiHeader;

typedef struct DMIEntryPoint {
	SMBByte			anchor[5];
	SMBByte			checksum;
	SMBWord			tableLength;
	SMBDWord		tableAddress;
	SMBWord			structureCount;
	SMBByte			bcdRevision;
} __attribute__((packed)) DMIEntryPoint;

typedef struct SMBEntryPoint {
	SMBByte			anchor[4];
	SMBByte			checksum;
	SMBByte			entryPointLength;
	SMBByte			majorVersion;
	SMBByte			minorVersion;
	SMBWord			maxStructureSize;
	SMBByte			entryPointRevision;
	SMBByte			formattedArea[5];
	struct DMIEntryPoint	dmi;
} __attribute__((packed)) SMBEntryPoint;

typedef struct DMIMemoryControllerInfo {/* 3.3.6 Memory Controller Information (Type 5) */
	DMI_STRUCT_HEADER
	SMBByte			errorDetectingMethod;
	SMBByte			errorCorrectingCapability;
	SMBByte			supportedInterleave;
	SMBByte			currentInterleave;
	SMBByte			maxMemoryModuleSize;
	SMBWord			supportedSpeeds;
	SMBWord			supportedMemoryTypes;
	SMBByte			memoryModuleVoltage;
	SMBByte			numberOfMemorySlots;
} __attribute__((packed)) DMIMemoryControllerInfo;

typedef struct DMIMemoryModuleInfo {	/* 3.3.7 Memory Module Information (Type 6) */
	DMI_STRUCT_HEADER
	SMBByte			socketDesignation;
	SMBByte			bankConnections;
	SMBByte			currentSpeed;
	SMBWord			currentMemoryType;
	SMBByte			installedSize;
	SMBByte			enabledSize;
	SMBByte			errorStatus;
} __attribute__((packed)) DMIMemoryModuleInfo;

typedef struct DMIPhysicalMemoryArray {	/* 3.3.17 Physical Memory Array (Type 16) */
	DMI_STRUCT_HEADER
	SMBByte			location;
	SMBByte			use;
	SMBByte			memoryCorrectionError;
	SMBDWord		maximumCapacity;
	SMBWord			memoryErrorInformationHandle;
	SMBWord			numberOfMemoryDevices;
} __attribute__((packed)) DMIPhysicalMemoryArray;

typedef struct DMIMemoryDevice {	/* 3.3.18 Memory Device (Type 17) */
	DMI_STRUCT_HEADER
	SMBWord			physicalMemoryArrayHandle;
	SMBWord			memoryErrorInformationHandle;
	SMBWord			totalWidth;
	SMBWord			dataWidth;
	SMBWord			size;
	SMBByte			formFactor;
	SMBByte			deviceSet;
	SMBByte			deviceLocator;
	SMBByte			bankLocator;
	SMBByte			memoryType;
	SMBWord			typeDetail;
    SMBWord         speed;
} __attribute__((packed)) DMIMemoryDevice;

typedef struct SMBStructHeader {
    SMBByte    type;
    SMBByte    length;
    SMBWord    handle;
}__attribute__((packed)) SMBStructHeader;

#define SMB_STRUCT_HEADER  SMBStructHeader header;

typedef struct SMBSystemInformation {
	// 2.0+ spec (8 bytes)
	SMB_STRUCT_HEADER               // Type 1
	SMBString  manufacturer;
	SMBString  productName;
	SMBString  version;
	SMBString  serialNumber;
	// 2.1+ spec (25 bytes)
	SMBByte    uuid[UUID_LEN];            // can be all 0 or all 1's
	SMBByte    wakeupReason;        // reason for system wakeup
	// 2.4+ spec (27 bytes)
    SMBString  skuNumber;
    SMBString  family;
}__attribute__((packed)) SMBSystemInformation;

//
// Base Board (Type 2)
//
typedef struct SMBBaseBoard {
    SMB_STRUCT_HEADER               // Type 2
    SMBString	manufacturer;
    SMBString	product;
    SMBString	version;
    SMBString	serialNumber;
    SMBString	assetTagNumber;
    SMBByte		featureFlags;
    SMBString	locationInChassis;
    SMBWord		chassisHandle;
    SMBByte		boardType;
    SMBByte		numberOfContainedHandles;
	// 0 - 255 contained handles go here but we do not include
	// them in our structure. Be careful to use numberOfContainedHandles
	// times sizeof(SMBWord) when computing the actual record size,
	// if you need it.
} __attribute__((packed)) SMBBaseBoard;

#endif /* !_LIBSAIO_SMBIOS_H */
