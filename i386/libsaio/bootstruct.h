/*
 * Copyright (c) 2002-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 2002-2003 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __BOOTSTRUCT_H
#define __BOOTSTRUCT_H

#include "bootXnu.h"
#include "saio_types.h"
#include "bios.h"
#include "device_tree.h"
/*!
    Kernel boot args global also used by booter for its own data.
 */
extern boot_args_common *bootArgs;

/*!
    Boot args passed to the kernel.
 */
extern boot_args_Legacy  *bootArgsLegacy;
extern boot_args_107	 *bootArgs107;

extern Node *gMemoryMapNode;

#define VGA_TEXT_MODE 0

#if 0
/*
 * Maximum number of boot drivers that can be loaded.
 */
#define NDRIVERS  500

#define CONFIG_SIZE (40 * 4096)
#endif

#define kMemoryMapCountMax 40

#if UNUSED

/*
 * PCI bus information.
 */
typedef struct _PCI_bus_info_t {
    union {
        struct {
            unsigned char configMethod1 :1;
            unsigned char configMethod2 :1;
            unsigned char               :2;
            unsigned char specialCycle1 :1;
            unsigned char specialCycle2 :1;
        } s;
        unsigned char d;
    } u_bus;
    unsigned char maxBusNum;
    unsigned char majorVersion;
    unsigned char minorVersion;
    unsigned char BIOSPresent;
} PCI_bus_info_t;

typedef struct {
    unsigned long address;  // address where driver was loaded
    unsigned long size;     // number of bytes
    unsigned long type;     // driver type
} driver_config_t;
#endif

/*
 * INT15, E820h - Query System Address Map.
 *
 * Documented in ACPI Specification Rev 2.0,
 * Chapter 15 (System Address Map Interfaces).
 */

/*
 * ACPI defined memory range types.
 */
enum {
    kMemoryRangeUsable   = 1,    // RAM usable by the OS.
    kMemoryRangeReserved = 2,    // Reserved. (Do not use)
    kMemoryRangeACPI     = 3,    // ACPI tables. Can be reclaimed.
    kMemoryRangeNVS      = 4,    // ACPI NVS memory. (Do not use)

    /* Undefined types should be treated as kMemoryRangeReserved */
};

/*!
    PrivateBootInfo has fields used by the booter that used to be part of
    KernelBootArgs_t *bootArgs.  When the switch was made to EFI the structure
    completely changed to boot_args *bootArgs.  This (new to boot-132) structure
    contains the fields the kernel no longer cares about but the booter still
    uses internally.  Some fields (e.g. the video information) remain interesting
    to the kernel and are thus located in bootArgs although with different field names.
 */
typedef struct PrivateBootInfo {
    int              convmem;                      // conventional memory
    int              extmem;                       // extended memory
#if 0
    int              numBootDrivers;               // number of drivers loaded
#endif
    char             bootFile[128];                // kernel file name		

    unsigned long    memoryMapCount;
    MemoryRange      memoryMap[kMemoryMapCountMax];
    
#if 0
    PCI_bus_info_t   pciInfo;
    driver_config_t  driverConfig[NDRIVERS];
    char *           configEnd;                    // pointer to end of config files
    char             config[CONFIG_SIZE];	
#endif
	
    config_file_t    bootConfig;		               // the booter boot.plist
    config_file_t    overrideConfig;               // additional boot.plist which can override bootConfig keys
	
	config_file_t    SystemConfig;               // system confing found in /Library/Preferences/SystemConfiguration/com.apple.Boot.plist
    

    config_file_t    themeConfig;				           // theme.plist
    config_file_t    smbiosConfig;				         // smbios.plist
    config_file_t    helperConfig;                 // boot helper partition's boot.plist
    config_file_t    ramdiskConfig;                // RAMDisk.plist
	
	unsigned long    adler32;
	
	char uuidStr[64+1];										//boot device  uuid
    
} PrivateBootInfo_t;

extern PrivateBootInfo_t *bootInfo; 

#endif /* __BOOTSTRUCT_H */
