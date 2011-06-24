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

//#include "saio_internal.h" // the only one needed by boot.h it self.
#include "bootstruct.h"

/*
 * Default names - these end with "Name" (easier to sort them).
 */
#define kDefaultKernelName	"mach_kernel"
#define kDefaultThemeName	"Default" // revert?
/*
#define kDefaultBootPlistName	"com.apple.Boot.plist"
#define kDefaultDSDTName		"dsdt.aml"
#define kDefaultSMBIOSName		"smbios.plist"
#define kDefaultRamdiskName		"Preboot.dmg"
???*/

/*
 * Default paths?????
 */

/*
 * Undeclared (or undeclared here)

"ProductVersion" - options.c (boot.c on trunk) - getValFK

"biosdev" mboot.c, getValFBK
"timeout" mboot.c, getValFBK
"partno" mboot.c, getValFBK

"-checkers" gui.c, getValFK;

"DumpSPD" spd.c, getBollFK - seems unsed***
*/

/*
 * gone to modules:

//#define kGraphicsEnablerKey	"GraphicsEnabler"	// GraphicsEnabler.c	getBoolFK
//#define kUseAtiROMKey			"UseAtiROM"			// ati.c				getBoolFK ?? modules
//#define kAtiConfigKey			"AtiConfig"			// ati.c				getStringFK
//#define kATYbinimageKey		"ATYbinimage"		// ati.c				getBoolFK
//#define kUseNvidiaROMKey		"UseNvidiaROM"		// nvidia.c				getBoolFK
//#define kVBIOSKey				"VBIOS"				// nvidia.c				getBoolFK

//#define kUseMemDetectKey		"UseMemDetect"		// platform.c			getBoolFK
*/

/*
separate into:
- internal use (change these to "blablabla")

- kernel & booter flags/arguments
	- no good at boot prompt
	- good at boot prompt
	
- booter options/patches
*/

/*
 * Keys used in system Boot.plist - these end with "Key".
 */
//		identifier			 string				 location				type					comment
#define kTextModeKey		 "Text Mode"		 // graphics.c			getNumberArraiFP		- default 80x25 - kFlag - does it work??

#define kDefaultPartitionKey "Default Partition" // sys.c				getStringFK
#define kHidePartitionKey	 "Hide Partition"	 // disk.c				getValFK
#define kRenamePartitionKey	 "Rename Partition"	 // disk.c				getStringFK

#define kInstantMenuKey		 "Instant Menu"		 // boot.c				getBoolFK
#define kQuietBootKey		 "Quiet Boot"		 // boot.c				getBoolFK				- kFlag
#define kTimeoutKey			 "Timeout"			 // options.c			getIntFK
#define kThemeNameKey		 "Theme"			 // gui.c				getValFK
#define kGUIKey				 "GUI"				 // boot.c				getBoolFK
#define kBootBannerKey		 "Boot Banner"		 // options.c			getBoolFK
#define kLegacyLogoKey		 "Legacy Logo"		 // gui.c				getBoolFK				- revert?
#define kDebugInfoKey		 "DebugInfo"		 // gui.c, graphics.c	getBoolFK
#define kRescanPromptKey	 "Rescan Prompt"	 // boot.c				getBoolFK				- cdrom only - firstrun
#define kRescanKey		     "Rescan"			 // boot.c				getBoolFK				- cdrom only?? - firstrun
#define kCDROMPromptKey		 "CD-ROM Prompt"	 // options.c			getValFK				- internal??
#define kCDROMOptionKey		 "CD-ROM Option Key" // options.c			getIntFK				- internal?? (F8)
#define kScanSingleDriveKey	 "Scan Single Drive" // boot.c				getBoolFK
#define kDevicePropertiesKey "device-properties" // device_inject.c		getValFK
#define kWaitForKeypressKey	 "Wait"				 // boot.c				getBoolFK
#define kAltConfigKey		 "config"			 // stringTable.c		getValFK				- kFlag - hum.. handle like kFlag??
#define kCanOverrideKey		 "CanOverride"		 // stringTable.c		getBoolFK				- remember -F ***
#define kRootDeviceKey		 "rd"				 // options.c			processBootArg			- kFlag
#define kBootDeviceKey		 "Boot Device"		 // options.c			getValFK				- kFlag/option??????
#define kBootUUIDKey		 "boot-uuid"		 // options.c			processBootArg			- kFlag
#define kHelperRootUUIDKey	 "Root UUID"		 // options.c			getValFK				- kFlag
#define kArchKey			 "arch"				 // boot.c				getValFK				- kFlag - after all,
// it's to be passed via "kernel Flags" ?? - "man com.apple.Boot.plist"
#define kKernelNameKey		 "Kernel"			 // options.c			getValFK				- kFlag*** bFlag ?
#define kKernelCacheKey		 "Kernel Cache"		 // boot.c				getValFK				- kFlag
#define kKernelFlagsKey		 "Kernel Flags"		 // options.c			getValFK				- kFlags***
#define kUseKCKey			 "UseKC"			 // boot.c				getBoolFK				- testing***

#define kKPatcherKey		 "PatchKernel"		 // kernel_patcher.c	getBoolFK
#define kAltExtensionsKey	 "kext"				 // drivers.c			getValFK
#define kMKextCacheKey		 "MKext Cache"		 // options.c			getValFK				- kFlag
#define kMD0ImageKey		 "md0"				 // ramdisk.c			getValFK
#define kWakeKey			 "Wake"				 // boot.c				getBoolFK
#define kForceWakeKey		 "ForceWake"		 // boot.c				getBoolFK
#define kWakeKeyImageKey	 "WakeImage"		 // boot.c				getValFK				- location???
#define kGraphicsModeKey	 "Graphics Mode"	 // graphics.c			getNumberArraiFP		- kFlag
#define kAutoResolutionKey	 "AutoResolution"	 // boot.c				getBoolFK

#define kDSDTKey			 "DSDT"				 // acpi_patcher.c		getValFK
#define kDropSSDTKey		 "DropSSDT"			 // acpi_patcher.c		getBoolFK
#define kRestartFixKey		 "RestartFix"        // acpi_patcher.c		getBoolFK				- revert to true?
#define kGeneratePStatesKey	 "GeneratePStates"	 // acpi_patcher.c		getBoolFK
#define kGenerateCStatesKey	 "GenerateCStates"	 // acpi_patcher.c		getBoolFK
#define kEnableC2StatesKey	 "EnableC2State"	 // acpi_patcher.c		getBoolFK
#define kEnableC3StatesKey	 "EnableC3State"	 // acpi_patcher.c		getBoolFK
#define kEnableC4StatesKey	 "EnableC4State"	 // acpi_patcher.c		getBoolFK

#define kSMBIOSdefaultsKey	 "SMBIOSdefaults"	 // smbios_patcher.c	getBoolFK
#define kSMBIOSKey			 "SMBIOS"			 // fake_efi.c			getValFK
//Azi: should this be changed to "SystemID"? BootHelp.txt matches SystemId
// cleaned obsolete comments on fake_efi.c, lines 500/508 & 608.
// fixed CHANGES file, lines 39/40.
#define kSystemIDKey		 "SystemId"			 // fake_efi.c			getStringFK
#define kSystemTypeKey		 "SystemType"		 // fake_efi.c			getStringFK
#define kPCIRootUIDKey		 "PciRoot"			 // pci_root.c			getValFK
#define kEthernetBuiltInKey	 "EthernetBuiltIn"	 // pci_setup.c			getBoolFK
#define kForceHPETKey		 "ForceHPET"		 // pci_setup.c			getBoolFK
#define kUSBBusFixKey		 "USBBusFix"		 // usb.c				getBoolFK				- trouble! - USBLegacyOff + the other = hang
#define kEHCIacquireKey		 "EHCIacquire"		 // usb.c				getBoolFK
#define kEHCIhardKey		 "EHCIhard"			 // usb.c				getBoolFK				- ??
#define kUHCIresetKey		 "UHCIreset"		 // usb.c				getBoolFK
#define kLegacyOffKey		 "USBLegacyOff"		 // usb.c				getBoolFK
#define kBusRatioKey		 "busratio"			 // cpu.c				getValFK

/*
 * Flags to the booter and/or kernel - these end with "Flag".
 */
//		identifier			 string				 location				type					comment
#define kVerboseModeFlag	 "-v"				 // options.c			getValFK				- kFlag
#define kSafeModeFlag		 "-x"				 // options.c			getValFBootK & getValFK - ?? - kFlag
#define kIgnoreCachesFlag	 "-f"				 // options.c			getValFK				- kFlag
#define kIgnoreBootFileFlag	 "-F"				 // options.c			getValFBootK			- kFlag
#define kSingleUserModeFlag	 "-s"				 // options.c			getValFK				- kFlag

/*
 * Booter behavior control
 */
#define kBootTimeout		 -1
#define kCDBootTimeout		 8

/*
 * A global set by boot() to record the device that the booter
 * was loaded from.
 */
extern int  gBIOSDev;
extern long gBootMode;
extern bool sysConfigValid;
extern char bootBanner[];
extern char bootLogBanner[];
extern char bootPrompt[];
extern bool gOverrideKernel;
extern char *gPlatformName; // disabled ??
extern char gMKextName[];
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
