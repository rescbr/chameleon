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
 * Default names
 */
#define kDefaultKernel		"mach_kernel"

/*
 * Keys used in system Boot.plist
 */
#define kGraphicsModeKey	"Graphics Mode"		// graphics.c
#define kTextModeKey		"Text Mode"			// graphics.c
#define kQuietBootKey		"Quiet Boot"		// boot.c
#define kKernelFlagsKey		"Kernel Flags"		// options.c
#define kMKextCacheKey		"MKext Cache"		// options.c
#define kKernelNameKey		"Kernel"			// options.c
#define kKernelCacheKey		"Kernel Cache"		// boot.c
#define kBootDeviceKey		"Boot Device"		// options.c
#define kTimeoutKey			"Timeout"			// options.c
#define kRootDeviceKey		"rd"				// options.c
#define kBootUUIDKey		"boot-uuid"			// options.c
#define kHelperRootUUIDKey	"Root UUID"			// options.c
#define kCDROMPromptKey		"CD-ROM Prompt"		// options.c
#define kCDROMOptionKey		"CD-ROM Option Key"	// options.c
#define kRescanPromptKey	"Rescan Prompt"		// boot.c
#define kRescanKey		    "Rescan"			// boot.c
#define kScanSingleDriveKey	"Scan Single Drive"	// boot.c
#define kInstantMenuKey		"Instant Menu"		// boot.c
#define kGUIKey				"GUI"				// boot.c
#define kBootBannerKey		"Boot Banner"		// options.c
#define kDebugInfoKey		"DebugInfo"			// gui.c, graphics.c
#define kWaitForKeypressKey	"Wait"				// boot.c
#define kExtensionsKey		"kext"				// drivers.c
#define kUseAtiROM			"UseAtiROM"			// ati.c
#define kWake				"Wake"				// boot.c
#define kForceWake			"ForceWake"			// boot.c
#define kWakeImage			"WakeImage"			// boot.c
#define kProductVersion		"ProductVersion"	// boot.c
#define kDSDTKey			"DSDT"				// acpi_patcher.c
#define kDropSSDT			"DropSSDT"			// acpi_patcher.c
#define kRestartFix			"RestartFix"        // acpi_patcher.c
#define kGeneratePStates	"GeneratePStates"	// acpi_patcher.c
#define kGenerateCStates	"GenerateCStates"	// acpi_patcher.c
#define kEnableC4States		"EnableC4State"		// acpi_patcher.c
#define kDeviceProperties	"device-properties"	// device_inject.c
#define kHidePartition		"Hide Partition"	// disk.c
#define kRenamePartition	"Rename Partition"	// disk.c
#define kSMBIOSKey			"SMBIOS"			// fake_efi.c
#define kSystemID			"SystemId"			// fake_efi.c
#define kSystemType			"SystemType"		// fake_efi.c
#define kUseNvidiaROM		"UseNvidiaROM"		// nvidia.c
#define kVBIOS				"VBIOS"				// nvidia.c
#define kPCIRootUID			"PciRoot"			// pci_root.c
#define kEthernetBuiltIn	"EthernetBuiltIn"	// pci_setup.c
#define kGraphicsEnabler	"GraphicsEnabler"	// pci_setup.c
#define kForceHPET			"ForceHPET"			// pci_setup.c
#define kUseMemDetect		"UseMemDetect"	    // platform.c
#define kSMBIOSdefaults		"SMBIOSdefaults"	// smbios_patcher.c
#define kUSBBusFix			"USBBusFix"			// usb.c
#define kEHCIacquire		"EHCIacquire"		// usb.c
#define kUHCIreset			"UHCIreset"			// usb.c
#define kLegacyOff			"USBLegacyOff"		// usb.c
#define kEHCIhard			"EHCIhard"			// usb.c
#define kDefaultPartition	"Default Partition"	// sys.c
#define kMD0Image			"md0"				// ramdisk.h
#define kTestConfigKey		"config"			// stringTable.c
#define kCanOverrideKey		"CanOverride"		// stringTable.c
#define kAutoResolutionKey	"AutoResolution"	// boot.c

/*
 * Flags to the booter and/or kernel - these end with "Flag".
 */
#define kVerboseModeFlag	"-v"				// options.c
#define kSafeModeFlag		"-x"				// options.c
#define kIgnoreCachesFlag	"-f"				// options.c
#define kIgnoreBootFileFlag	"-F"				// options.c
#define kSingleUserModeFlag	"-s"				// options.c
#define kArchI386Flag		"32"				// boot.c
#define kLegacyModeFlag		"-legacy"			// boot.c
#define kArchX86_64Flag		"64"				// boot.c

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
//extern char *gPlatformName; disabled
extern char gMKextName[];
extern char gMacOSVersion[];
extern char gRootDevice[];
extern bool gEnableCDROMRescan;
extern bool gScanSingleDrive;
extern bool useGUI;

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

/*
 * usb.c
 */
extern int usb_loop();

/*
 * graphics.c
 */
extern void printVBEModeInfo();
extern void setVideoMode(int mode, int drawgraphics);
extern int  getVideoMode();
extern void spinActivityIndicator();
extern void clearActivityIndicator();
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
extern long (*LoadExtraDrivers_p)(FileLoadDrivers_t FileLoadDrivers_p);

/*
 * options.c
 */
extern int getBootOptions(bool firstRun);
extern int processBootOptions();
extern int selectAlternateBootDevice(int bootdevice);
extern bool promptForRescanOption(void);

void showHelp();
void showTextFile();
char *getMemoryInfoString();

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
  char      platform_name[64];
  char      root_path[256];
  u_int8_t  data[0];
};
typedef struct compressed_kernel_header compressed_kernel_header;

/* resume.c */
void HibernateBoot(char *boot_device);

/* bmdecompress.c */
void * DecompressData(void *srcbase, int *dw, int *dh, int *bytesPerPixel);

#endif /* !__BOOT2_BOOT_H */
