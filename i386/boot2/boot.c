/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
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
/* 
 * Mach Operating System
 * Copyright (c) 1990 Carnegie-Mellon University
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

/*
 *          INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *  This software is supplied under the terms of a license  agreement or 
 *  nondisclosure agreement with Intel Corporation and may not be copied 
 *  nor disclosed except in accordance with the terms of that agreement.
 *
 *  Copyright 1988, 1989 by Intel Corporation
 */

/*
 * Copyright 1993 NeXT Computer, Inc.
 * All rights reserved.
 */
 
/*
 * Completely reworked by Sam Streeper (sam_s@NeXT.com)
 * Reworked again by Curtis Galloway (galloway@NeXT.com)
 */


#include "boot.h"
#include "sl.h"
#include "libsa.h"
#include "modules.h"

long gBootMode; /* defaults to 0 == kBootModeNormal */
bool gOverrideKernel;
char *gPlatformName;
char gRootDevice[512];
char gMKextName[512];
char gMacOSVersion[8];
bool gEnableCDROMRescan;
bool gScanSingleDrive;

int     bvCount = 0;
int     gDeviceCount = 0; 

BVRef   bvr;
BVRef   menuBVR;
BVRef   bvChain;
bool    useGUI;

/*
 * How long to wait (in seconds) to load the
 * kernel after displaying the "boot:" prompt.
 */
#define kBootErrorTimeout 5

/*
 * Default path to kernel cache file
 */
//Slice - first one for Leopard
#define kDefaultCachePathLeo "/System/Library/Caches/com.apple.kernelcaches/"
#define kDefaultCachePathSnow "/System/Library/Caches/com.apple.kext.caches/Startup/"

//==========================================================================
// Zero the BSS.

static void zeroBSS(void)
{
	extern char _DATA__bss__begin, _DATA__bss__end;
	extern char _DATA__common__begin, _DATA__common__end;

	bzero(&_DATA__bss__begin, (&_DATA__bss__end - &_DATA__bss__begin));
	bzero(&_DATA__common__begin, (&_DATA__common__end - &_DATA__common__begin));
}

//==========================================================================
// Malloc error function

static void malloc_error(char *addr, size_t size, const char *file, int line)
{
	stop("\nMemory allocation error! Addr=0x%x, Size=0x%x, File=%s, Line=%d\n", (unsigned)addr, (unsigned)size, file, line);
}

//==========================================================================
//Initializes the runtime.  Right now this means zeroing the BSS and initializing malloc.
//
void initialize_runtime(void)
{
	zeroBSS();
	malloc_init(0, 0, 0, malloc_error);
}

//==========================================================================
// This is the entrypoint from real-mode which functions exactly as it did
// before. Multiboot does its own runtime initialization, does some of its
// own things, and then calls common_boot.
void boot(int biosdev)
{
	initialize_runtime();
	// Enable A20 gate before accessing memory above 1Mb.
	enableA20();
	common_boot(biosdev);
}

//==========================================================================
// The 'main' function for the booter. Called by boot0 when booting
// from a block device, or by the network booter.
//
// arguments:
//   biosdev - Value passed from boot1/NBP to specify the device
//             that the booter was loaded from.
//
// If biosdev is kBIOSDevNetwork, then this function will return if
// booting was unsuccessful. This allows the PXE firmware to try the
// next boot device on its list.
void common_boot(int biosdev)
{
    //unsigned int allowBVFlags = kBVFlagSystemVolume|kBVFlagForeignBoot;
//    unsigned int denyBVFlags = kBVFlagEFISystem;

    // Setup VGA text mode.
    // Not sure if it is safe to call setVideoMode() before the
    // config table has been loaded. Call video_mode() instead.
    video_mode( 2 );  // 80x25 mono text mode.

    // TOOD: move to a module
    /*
    // Scan and record the system's hardware information.
    scan_platform();

    // First get info for boot volume.
    scanBootVolumes(gBIOSDev, 0);
    
    bvChain = getBVChainForBIOSDev(gBIOSDev);
    
    setBootGlobals(bvChain);
    
    scanDisks(gBIOSDev, &bvCount);

    // Create a separated bvr chain using the specified filters.
    bvChain = newFilteredBVChain(0x80, 0xFF, allowBVFlags, denyBVFlags, &gDeviceCount);

    gBootVolume = selectBootVolume(bvChain);
     */
	// Intialize module system 
	init_module_system();
    
    UInt32 loopCount = 0;
    while(1)
    {
        execute_hook("WorkLoop", (void*)loopCount++, NULL, NULL, NULL);	// Main work loop
    }
}
