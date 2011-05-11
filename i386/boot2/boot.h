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
 * Copyright 1994 NeXT Computer, Inc.
 * All rights reserved.
 */

#ifndef __BOOT2_BOOT_H
#define __BOOT2_BOOT_H

#include "libsaio.h"

/*
 * Keys used in system Boot.plist
 */
#define kGraphicsModeKey	"Graphics Mode"
#define kTextModeKey		"Text Mode"
#define kQuietBootKey		"Quiet Boot"
#define kKernelFlagsKey		"Kernel Flags"
#define kMKextCacheKey		"MKext Cache"
#define kKernelNameKey		"Kernel"
#define kPrelinkKernelKey	"Prelinked Kernel"
#define kBootDeviceKey		"Boot Device"
#define kTimeoutKey			"Timeout"
#define kRootDeviceKey		"rd"
#define kBootUUIDKey		"boot-uuid"
#define kHelperRootUUIDKey	"Root UUID"
#define kPlatformKey		"platform"
#define kCDROMPromptKey		"CD-ROM Prompt"
#define kCDROMOptionKey		"CD-ROM Option Key"
#define kRescanPromptKey	"Rescan Prompt"
#define kRescanKey		    "Rescan"
#define kScanSingleDriveKey	"Scan Single Drive"
#define kInsantMenuKey		"Instant Menu"
#define kDefaultKernel		"mach_kernel"
#define kWaitForKeypressKey	"Wait"
/* AsereBLN: added the other keys */

#define kProductVersion		"ProductVersion"	/* boot.c */
#define karch				"arch"				/* boot.c */
#define kDeviceProperties	"device-properties"	/* device_inject.c */
#define kHidePartition		"Hide Partition"	/* disk.c */
#define kRenamePartition	"Rename Partition"	/* disk.c */
#define kSMBIOSKey			"SMBIOS"			/* fake_efi.c */
#define kSystemID			"SystemId"			/* fake_efi.c */
#define kSystemType			"SystemType"		/* fake_efi.c */
#define kPCIRootUID			"PCIRootUID"		/* pci_root.c */
#define kDefaultPartition	"Default Partition"	/* sys.c */

enum {
	kBackspaceKey	= 0x08,
	kTabKey			= 0x09,
	kReturnKey		= 0x0d,
	kEscapeKey		= 0x1b,
	kUpArrowkey		= 0x4800, 
	kDownArrowkey	= 0x5000,
	kASCIIKeyMask	= 0x7f,
	kF5Key			= 0x3f00,
	kF10Key			= 0x4400
};

#define PLATFORM_NAME_LEN 64
#define ROOT_PATH_LEN 256

/*
 * Flags to the booter or kernel
 */
#define kVerboseModeFlag	"-v"
#define kSafeModeFlag		"-x"
#define kIgnoreCachesFlag	"-f"
#define kSingleUserModeFlag	"-s"
#define kIgnorePrelinkKern  "-F"
#define kIgnoreBootFileFlag	"-B"

/*
 * Booter behavior control
 */
#define kBootTimeout         -1
#define kCDBootTimeout       8

/*
 * A global set by boot() to record the device that the booter
 * was loaded from.
 */
extern int  gBIOSDev;
extern long gBootMode;
extern bool sysConfigValid;
extern char bootBanner[];
extern char bootPrompt[];
extern bool gOverrideKernel;
extern char gMKextName[];
extern bool gEnableCDROMRescan;
extern bool gScanSingleDrive;
extern char *gPlatformName;
//extern char gRootPath[];

extern char *gRootDevice;
extern bool uuidSet;
/*
 * Boot Modes
 */
enum {
    kBootModeNormal = 0,
    kBootModeSafe   = 1,
    kBootModeSecure = 2,
    kBootModeQuiet  = 4
};

extern void initialize_runtime();
extern void common_boot(int biosdev);
extern unsigned long
local_adler32( unsigned char * buffer, long length );


/*
 * graphics.c
 */
extern void printVBEModeInfo();
extern void setVideoMode(int mode, int drawgraphics);
#if TEXT_SPINNER
extern void spinActivityIndicator();
extern void clearActivityIndicator();
#endif
extern void drawColorRectangle( unsigned short x,
                         unsigned short y,
                         unsigned short width,
                         unsigned short height,
                         unsigned char  colorIndex );
extern void drawDataRectangle( unsigned short  x,
                        unsigned short  y,
                        unsigned short  width,
                        unsigned short  height,
                               unsigned char * data );
extern int
convertImage( unsigned short width,
              unsigned short height,
              const unsigned char *imageData,
              unsigned char **newImageData );
extern char * decodeRLE( const void * rleData, int rleBlocks, int outBytes );
extern void drawBootGraphics(void);
extern void drawPreview(void *src, uint8_t * saveunder);
extern int getVideoMode(void);
extern void loadImageScale (void *input, int iw, int ih, int ip, void *output, int ow, int oh, int op, int or);

/*
 * drivers.c
 */
extern long LoadDrivers(char * dirSpec);
extern long DecodeKernel(void *binary, entry_t *rentry, char **raddr, int *rsize);

typedef long (*FileLoadDrivers_t)(char *dirSpec, long plugin);
/*!
    Hookable function pointer called during the driver loading phase that
    allows other code to cause additional drivers to be loaded.
 */
extern struct multiboot_info *gMI;
/*
 * options.c
 */
extern int getBootOptions(bool firstRun);
extern int processBootOptions();
extern bool promptForRescanOption(void);
extern bool copyArgument(const char *argName, const char *val, int cnt, char **argP, int *cntRemainingP);


void showHelp();
void showTextFile();
char *getMemoryInfoString();
void showMessage(char * message);

typedef struct {
    char   name[80];
    void * param;
} MenuItem;

/*
 * lzss.c
 */
extern int decompress_lzss(u_int8_t *dst, u_int8_t *src, u_int32_t srclen);

struct compressed_kernel_header {
  u_int32_t signature;
  u_int32_t compress_type;
  u_int32_t adler32;
  u_int32_t uncompressed_size;
  u_int32_t compressed_size;
  u_int32_t reserved[11];
  char      platform_name[PLATFORM_NAME_LEN];
  char      root_path[ROOT_PATH_LEN];
  u_int8_t  data[0];
};
typedef struct compressed_kernel_header compressed_kernel_header;

#endif /* !__BOOT2_BOOT_H */
