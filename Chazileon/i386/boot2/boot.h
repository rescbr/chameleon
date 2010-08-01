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

//Azi:include - keep boot.h free of includes as possible, for now.
#include "saio_internal.h"
//#include "libsaio.h"
//#include "autoresolution.h" // not needed; it's included were it's needed.

#if DEBUG
#ifndef AUTORES_DEBUG
#define AUTORES_DEBUG
#endif
#endif

/*
 * temporary "keys related" stuff
 *
- deactivated:
#define kPlatformKey		"platform"				// options.c, processBootArgument			removed, says on comment
#define kACPIKey			"acpi"					//											not in use, afaics
#define kPCIRootUIDKey		"PCIRootUID"			// pci_root.c, getValFK						removed by me
#define k32BitModeFlag		"-x32" 					// boot.c, getValFK							removed by me
"-pci0"												// pci_root.c, getValFK						removed by me
"-pci1"												// pci_root.c, getValFK						removed by me

- lost & found:
"-checkers" gui.c, getValFK; booter flag
"biosdev" mboot.c, getValFBK
"timeout" mboot.c, getValFBK
"partno" mboot.c, getValFBK
"DumpSPD" spd.c, getBollFK

- ramdisk specific:
"Info" // ramdisk.c, getValFK
"BTAlias" // ramdisk.c, getBollFK

kNVCAP		nvidia.c, getValFK
This introduces a new way (to me) to implement key/value;
key is declared as a "variable", char kNVCAP[12]; on nvidia.c, setup_nvidia_devprop().
Check: http://efixusers.com/showthread.php?t=644, Update Version 1.1.9 (Asere's booter).

- Keep just most "useful at boot prompt" stuff on BootHelp; make it a real "boot help", not a user guide.
- Check who is set to "Yes" by default and if it's really needed! (wake)
- move keys to the headers of the files they're in, if they have it!?
*/

/*
 * Default names - these end with "Name" (easier to sort them).
 */

#define kDefaultKernelName		"mach_kernel"
#define kDefaultThemeName		"Default"
/*#define kDefaultBootPlistName	"com.apple.Boot.plist"
#define kDefaultDSDTName		"dsdt.aml"
#define kDefaultSMBIOSName		"smbios.plist"
#define kDefaultRamdiskName		"Preboot.dmg" ???*/

/*
 * Keys used in system Boot.plist - these end with "Key".
 *
 * Keys marked with * on the "location" area, are present on BootHelp file.
 * New or temporary keys, since RC4, are marked with # on the "location" area.
 */
//definition					name				location/function							description/comment

// Internal use: remove/move, if possible/benefitial?? leave for later...
#define kBootDeviceKey			"Boot Device"		// options.c, getValFK						and this??
#define kTextModeKey			"Text Mode"			// graphics.c, getNumberArrayFromProperty	textmode resolution??
#define kProductVersion			"ProductVersion"	// boot.c, getValFK, internal				key on SystemVersion.plist

// First run: keys that make no sense on "override" Boot.plist or typed at boot prompt.
#define kDefaultPartitionKey	"Default Partition"	// sys.c, getValFK
#define kHidePartitionKey		"Hide Partition"	// disk.c, getValFK
#define kRenamePartitionKey		"Rename Partition"	// disk.c, getStringFK
#define kTimeoutKey				"Timeout"			// * options.c, getIntFK
#define kInstantMenuKey			"Instant Menu"		// * boot.c, getBollFK
#define kQuietBootKey			"Quiet Boot"		// * boot.c, getBollFK
#define kThemeNameKey			"Theme"				// gui.c, getValFK							override default name
#define kGUIKey					"GUI"				// * boot.c, getBollFK
#define kBootBannerKey			"Boot Banner"		// * options.c, getBollFK
#define kDisplayInfoKey			"DisplayInfo"		// #gui.c, graphics.c, getBollFK
#define kGraphicsModeKey		"Graphics Mode"		// * graphics.c, getNumberArrayFromProperty
#define kCDROMPromptKey			"CD-ROM Prompt"		// options.c, getValFK
#define kCDROMOptionKey			"CD-ROM Option Key"	// options.c, getIntFK & getValFK
#define kRescanPromptKey		"Rescan Prompt"		// boot.c, getBollFK						test this
#define kRescanKey				"Rescan"			// * boot.c, getBollFK
#define kScanSingleDriveKey		"Scan Single Drive"	// * boot.c, getBollFK
#define kDevicePropertiesKey	"device-properties"	// device_inject.c, getValFK

// Common: firstrun or secondrun
#define kWaitForKeypressKey		"Wait"				// * boot.c, getBollFK						turn this into flag?
#define kTestConfigFileKey		"config"			// # stringTable.c, getValFK
#define kKernelNameKey			"Kernel"			// * options.c, getValFK,					override default name; at boot prompt typing the name is enough.
#define kKernelCacheKey			"Kernel Cache"		// boot.c, getValFK							test this
#define kKernelFlagsKey			"Kernel Flags"		// * options.c, getValFK
#define kPatchKernelKey			"PatchKernel"		// # stringTable.c, getBollFK				Meklort

#define kRootDeviceKey			"rd"				// * options.c, processBootArgument			test this
#define kBootUUIDKey			"boot-uuid"			// options.c, processBootArgument			test this
#define kHelperRootUUIDKey		"Root UUID"			// options.c, getValFK						can't test this

#define kExtensionsKey			"kext"				// # drivers.c, getValFK
#define kMKextCacheKey			"MKext Cache"		// options.c, getValFK						test this
#define kDSDTKey				"DSDT"				// * acpi_patcher.c, getValFK				override default path/name!?
#define kDropSSDTKey			"DropSSDT"			// * acpi_patcher.c, getBollFK
#define kSMBIOSKey				"SMBIOS"			// * fake_efi.c; getValFK
#define kSMBIOSdefaultsKey		"SMBIOSdefaults"	// * smbios_patcher.c, getBollFK
#define kWakeKey				"Wake"				// * boot.c, getBollFK						firstrun?
#define kForceWakeKey			"ForceWake"			// * boot.c, getBollFK						firstrun?
#define kWakeImageKey			"WakeImage"			// * boot.c, getValFK						firstrun?
#define kPciRootKey				"PciRoot"			// * pci_root.c, getValFK,
#define kUseAtiROMKey			"UseAtiROM"			// # ati.c, getBollFK						firstrun?
#define kUseNvidiaROMKey		"UseNvidiaROM"		// # nvidia.c, getBollFK					firstrun?
#define kVBIOSKey				"VBIOS"				// nvidia.c, getBollFK						firstrun?
#define kSystemIDKey			"SystemId"			// * fake_efi.c, getStringFK
#define kSystemTypeKey			"SystemType"		// * acpi_patcher.c, getStringFK
#define kEthernetBuiltInKey		"EthernetBuiltIn"	// * pci_setup.c, getBollFK
#define kGraphicsEnablerKey		"GraphicsEnabler"	// * pci_setup.c, getBollFK
#define kLegacyLogoKey			"Legacy Logo"		// * gui.c, getBollFK
#define kEHCIhardKey			"EHCIhard"			// usb.c, getBollFK, - rc4					Azi: wtf is this?? internal?
#define kUSBBusFixKey			"USBBusFix"			// * pci_setup.c, getBollFK
#define kEHCIacquireKey			"EHCIacquire"		// * pci_setup.c, getBollFK
#define kUHCIresetKey			"UHCIreset"			// * pci_setup.c, getBollFK
#define kForceHPETKey			"ForceHPET"			// * pci_setup.c, getBollFK
#define kLegacyOffKey			"USBLegacyOff"		// # usb.c, getBoolFK						Meklort
#define kMD0ImageKey			"md0"				// # ramdisk.c, getValFK						||
#define kRestartFixKey			"RestartFix"		// # acpi_patcher.c, getBollFK
#define kUseMemDetectKey		"UseMemDetect"		// # platform.c, getBollFK
#define kAutoResolutionKey		"AutoResolution"	// # boot.c, getBollFK						don't forget F2 key, to change resolution!
#define kCanOverrideKey			"CanOverride"		// # stringTable.c, getBollFK				Azi: most probably will be gone
#define kGeneratePStatesKey		"GeneratePStates"	// # acpi_patcher.c, getBoolFK				Mozodojo
#define kGenerateCStatesKey		"GenerateCStates"	// # acpi_patcher.c, getBoolFK					||
#define kEnableC4States			"EnableC4State"		// # acpi_patcher.c,							||

/*
 * Flags to the booter and/or kernel - these end with "Flag".
 */
// A summary on next 3 flags:
// These have no use on machines with cpu "without" 64 bit intructions.
// Those machines boot i386 arch by "default", Legacy mode!
// Reworked "arch" key; kind of a fusion between "arch" & "k32BitModeFlag";
// If this (or something similar) doesn't stick, i vote for arch=i386, it's more educational!
// 2 keys doing the same thing, just adds confusion!!
#define kArchI386Flag			"32"				// # boot.c, getValFK							force i386 ("64 bit Mode" available)
// Used by the booter to pass i386 arch; used by the kernel to disable "64 bit Mode".
#define kLegacyModeFlag			"-legacy"			// # boot.c, fake_efi.c, getValFK;			force i386 + disable "64 bit Mode"
// Just to override i386/-legacy if flagged on Boot.plist.
#define kArchX86_64Flag			"64"				// # boot.c, getValFK
//----------
#define kVerboseModeFlag		"-v"				// * options.c, getValFK
#define kOldSafeModeFlag		"-f"				// kIgnoreCacheModeFlag*, options.c, getValFK
#define kSafeModeFlag			"-x"				// * options.c, getValueFBK (5) & getValFK
#define kSingleUserModeFlag		"-s"				// * options.c, getValFK
#define kIgnoreBootFileFlag		"-F"				// * options.c, getValFBK
// Can't remember if i ever tried -F; had an idea.. make this point to kTestConfigFileKey,
// kind of a -config key.. like if "if you have a messed up config file" use -F to ignore it
// and use a "safe default one".. investigate

/*
 * Booter behavior control
 */
#define kBootTimeout			-1
#define kCDBootTimeout			8

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
//extern char *gPlatformName; //Azi: not in use
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
