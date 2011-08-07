/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
/*
 *  sl.h - Headers for configuring the Secondary Loader
 *
 *  Copyright (c) 1998-2005 Apple Computer, Inc.
 *
 *  DRI: Josh de Cesare
 */

#ifndef _BOOTX_SL_H_
#define _BOOTX_SL_H_

// i.e. should we put up our failure screen; else back to OF
#define kFailToBoot (1)

/*

Memory Map:  assumed 96 MB (temporarily bumping to 112 MB for 4359362)

Physical Address

Open Firmware Version    3x, 4x, ...
00000000 - 00003FFF  :   Exception Vectors
00004000 - 057FFFFF  :   Free Memory
// 05800000 - 05FFFFFF  :   OF Image (top 8 MB reserved) [96 MB map]
06800000 - 06FFFFFF  :   OF Image (top 8 MB reserved) [112 MB map]


Logical Address

// 96 MB map (currently unused - 4363357 tracks re-adoption)
00000000 - 00003FFF  : Exception Vectors
00004000 - 03FFFFFF  : Kernel Image, Boot Struct and Drivers (~64 MB)
04000000 - 04FFFFFF  : File Load Area (16 MB) 	[80 MB]
05000000 - 053FFFFF  : FS Cache    (4 MB)  	[84 MB]
05400000 - 055FFFFF  : Malloc Zone (2 MB)	[86 MB]
05600000 - 057FFFFF  : BootX Image (2 MB)	[88 MB]
05800000 - 05FFFFFF  : Unused/OF   (8 MB)	[96 MB]

// 112 MB map (per 4359362)
00000000 - 00003FFF  : Exception Vectors
00004000 - 03FFFFFF  : Kernel Image, Boot Struct and Drivers (~64 MB)
04000000 - 05FFFFFF  : File Load Area (32 MB) 	[96 MB]
06000000 - 063FFFFF  : FS Cache    (4 MB)  	[100 MB]
06400000 - 065FFFFF  : Malloc Zone (2 MB)	[102 MB]
06600000 - 067FFFFF  : BootX Image (2 MB)	[104 MB]
06800000 - 06FFFFFF  : Unused/OF   (8 MB)	[112 MB]
*/

#define kVectorAddr     (0x00000000)
#define kVectorSize     (0x00004000)

// OF 3.x
#define kImageAddr      (0x00004000)
#define kImageSize      (0x03FFC000)

// OF 1.x 2.x
#define kImageAddr0     (0x00004000)
#define kImageSize0     (0x002FC000)
#define kImageAddr1     (0x00300000)
#define kImageSize1     (0x00200000)
#define kImageAddr1Phys (0x05800000)
#define kImageAddr2     (0x00500000)
#define kImageSize2     (0x03B00000)

#define kLoadAddr       (0x04000000)
#define kLoadSize       (0x02000000)	// 32 MB @ 64
#define kMaxMKextSize   (0x01000000)	// only allow 16 MB of drivers

#define kFSCacheAddr    (0x06000000)
#define kFSCacheSize    (0x00400000)	// 4 MB @ 96

#define kMallocAddr     (0x06400000)
#define kMallocSize     (0x00200000)	// 2 MB @ 100

#define kMallocAddr_H   (0x06400000)	// ditto for hibernate
#define kMallocSize_H   (0x00200000)
#define kImageAddr_H    (0x07000000)	// fallback for hiberate image buffer

// Default Output Level
#define kOutputLevelOff  (0)
#define kOutputLevelFull (16)

// OF versions
#define kOFVersion1x    (0x01000000)
#define kOFVersion2x    (0x02000000)
#define kOFVersion3x    (0x03000000)
#define kOFVersion4x    (0x04000000)

// Device Types
enum {
  kUnknownDeviceType = 0,
  kNetworkDeviceType,
  kBlockDeviceType
};

// File Permissions and Types
enum {
  kPermOtherExecute  = 1 << 0,
  kPermOtherWrite    = 1 << 1,
  kPermOtherRead     = 1 << 2,
  kPermGroupExecute  = 1 << 3,
  kPermGroupWrite    = 1 << 4,
  kPermGroupRead     = 1 << 5,
  kPermOwnerExecute  = 1 << 6,
  kPermOwnerWrite    = 1 << 7,
  kPermOwnerRead     = 1 << 8,
  kPermMask          = 0x1FF,
  kOwnerNotRoot      = 1 << 9,
  kFileTypeUnknown   = 0x0 << 16,
  kFileTypeFlat      = 0x1 << 16,
  kFileTypeDirectory = 0x2 << 16,
  kFileTypeLink      = 0x3 << 16,
  kFileTypeMask      = 0x3 << 16
};

// Key Numbers
#define kCommandKey (0x200)
#define kOptKey     (0x201)
#define kShiftKey   (0x202)
#define kControlKey (0x203)
#define kDeleteKey  (0x204)

// Mac OS X Booter Signature 'MOSX'
#define kMacOSXSignature (0x4D4F5358)

// Boot Modes
enum {
  kBootModeNormal = 0,
  kBootModeSafe,
  kBootModeSecure
};

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

#include <ci.h>
#include <sl_words.h>
#include <libclite.h>

// Externs for main.c
extern char *gVectorSaveAddr;
extern long gKernelEntryPoint;
extern long gDeviceTreeAddr;
extern long gDeviceTreeSize;
extern long gBootArgsAddr;
extern long gBootArgsSize;
extern long gSymbolTableAddr;
extern long gSymbolTableSize;

extern long gBootMode;
extern long gBootDeviceType;
extern long gBootFileType;
extern char gHaveKernelCache;
extern char gBootDevice[256];
extern char gBootFile[256];

extern char gTempStr[4096];

extern long *gDeviceTreeMMTmp;

extern long gOFVersion;

extern char *gKeyMap;

extern long gRootAddrCells;
extern long gRootSizeCells;

extern CICell gChosenPH;
extern CICell gOptionsPH;
extern CICell gScreenPH;
extern CICell gMemoryMapPH;
extern CICell gStdOutPH;

extern CICell gMMUIH;
extern CICell gMemoryIH;
extern CICell gStdOutIH;
extern CICell gKeyboardIH;

#endif /* ! _BOOTX_SL_H_ */
