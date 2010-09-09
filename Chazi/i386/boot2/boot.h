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

//Azi: keep it minimal for now.
// Todo: check bootstruct.h, sl.h, platform.h, next; include bootstruct here??
// don't include headers like libsaio.h on other headers?!
// move keys to the headers of the files they're in, if they have it!?
#include "saio_internal.h" // the only one needed by boot.h it self, afaics.

/*
 * Default names - these end with "Name" (easier to sort them).
 */
#define kDefaultKernelName	"mach_kernel"
#define kDefaultThemeName	"Default"
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
 * Undeclared (or undeclared here) keys.
 */
/*
"-checkers" gui.c, getValFK;
"biosdev" mboot.c, getValFBK
"timeout" mboot.c, getValFBK
"partno" mboot.c, getValFBK
"DumpSPD" spd.c, getBollFK

kNVCAP nvidia.c, getValFK - key is declared as a "variable", char kNVCAP[12], on setup_nvidia_devprop().
Check: http://efixusers.com/showthread.php?t=644, Update Version 1.1.9 (Asere's booter).
"If you want to override the NVCAP value, you must determine the PCI DeviceID of your graphic card.
For instance: my new GTX260 has the DeviceID 0×05e2. Knowing the DeviceID add this to your com.apple.Boot.plist:
<key>NVCAP_05e2</key>
<string>0000000000000000000000000000000000000000</string>
The NVCAP value is exactly 20 bytes long. You have to specify it using ASCII-HEX (0-9,a-f)."

ramdisk specific:
"Info" // ramdisk.c, getValFK
"BTAlias" // ramdisk.c, getBollFK
*/

/*
 * Internal or "default" Boot.plist only keys (firstrun) - these end with "Key".
 *
 * keys that make no sense on "override" Boot.plist or at boot prompt;
 * like so, they won't be present on BootHelp.txt.
 *
 */
//		identifier			 string				 location
#define kTextModeKey		 "Text Mode"		 // graphics.c
#define kProductVersionKey	 "ProductVersion"	 // options.c (boot.c on trunk)
#define kDefaultPartitionKey "Default Partition" // sys.c
#define kHidePartitionKey	 "Hide Partition"	 // disk.c
#define kRenamePartitionKey	 "Rename Partition"	 // disk.c
#define kInstantMenuKey		 "Instant Menu"		 // boot.c
#define kQuietBootKey		 "Quiet Boot"		 // boot.c
#define kTimeoutKey			 "Timeout"			 // options.c
#define kThemeNameKey		 "Theme"			 // gui.c
#define kGUIKey				 "GUI"				 // boot.c
#define kBootBannerKey		 "Boot Banner"		 // options.c
#define kDebugInfoKey		 "DebugInfo"		 // gui.c, graphics.c
#define kCDROMPromptKey		 "CD-ROM Prompt"	 // options.c
#define kCDROMOptionKey		 "CD-ROM Option Key" // options.c
#define kRescanPromptKey	 "Rescan Prompt"	 // boot.c
#define kRescanKey		     "Rescan"			 // boot.c
#define kScanSingleDriveKey	 "Scan Single Drive" // boot.c
#define kDevicePropertiesKey "device-properties" // device_inject.c


/*
 * Prompt or Boot.plist keys (secondrun) - these end with "Key".
 *
 * Keys that make some/all sense at boot prompt or any Boot.plist.
 *
 */
#define kWaitForKeypressKey	 "Wait"				 // boot.c
#define kTestConfigKey		 "config"			 // stringTable.c
#define kCanOverrideKey		 "CanOverride"		 // stringTable.c
#define kRootDeviceKey		 "rd"				 // options.c
#define kBootDeviceKey		 "Boot Device"		 // options.c - ????? internal
#define kBootUUIDKey		 "boot-uuid"		 // options.c
#define kHelperRootUUIDKey	 "Root UUID"		 // options.c
#define kKernelNameKey		 "Kernel"			 // options.c
#define kKernelCacheKey		 "Kernel Cache"		 // boot.c
#define kKernelFlagsKey		 "Kernel Flags"		 // options.c
#define kKPatcherKey		 "PatchKernel"		 // kernel_patcher.c
#define kExtensionsKey		 "kext"				 // drivers.c
#define kMKextCacheKey		 "MKext Cache"		 // options.c
#define kMD0ImageKey		 "md0"				 // ramdisk.c
#define kWakeKey			 "Wake"				 // boot.c
#define kForceWakeKey		 "ForceWake"		 // boot.c
#define kWakeKeyImageKey	 "WakeImage"		 // boot.c
#define kUseAtiROMKey		 "UseAtiROM"		 // ati.c
#define kUseNvidiaROMKey	 "UseNvidiaROM"		 // nvidia.c
#define kVBIOSKey			 "VBIOS"			 // nvidia.c
#define kGraphicsModeKey	 "Graphics Mode"	 // graphics.c - here because of AutoResolution patch, which uses it + F2!!
#define kAutoResolutionKey	 "AutoResolution"	 // boot.c
#define kGraphicsEnablerKey	 "GraphicsEnabler"	 // pci_setup.c
#define kLegacyLogoKey		 "Legacy Logo"		 // gui.c
#define kDSDTKey			 "DSDT"				 // acpi_patcher.c
#define kDropSSDTKey		 "DropSSDT"			 // acpi_patcher.c
#define kRestartFixKey		 "RestartFix"        // acpi_patcher.c
#define kGeneratePStatesKey	 "GeneratePStates"	 // acpi_patcher.c
#define kGenerateCStatesKey	 "GenerateCStates"	 // acpi_patcher.c
#define kEnableC4StatesKey	 "EnableC4State"	 // acpi_patcher.c
#define kUseMemDetectKey	 "UseMemDetect"	     // platform.c
#define kSMBIOSdefaultsKey	 "SMBIOSdefaults"	 // smbios_patcher.c
#define kSMBIOSKey			 "SMBIOS"			 // fake_efi.c
#define kSystemIDKey		 "SystemId"			 // fake_efi.c
#define kSystemTypeKey		 "SystemType"		 // fake_efi.c
#define kPCIRootUIDKey		 "PciRoot"			 // pci_root.c
#define kEthernetBuiltInKey	 "EthernetBuiltIn"	 // pci_setup.c
#define kForceHPETKey		 "ForceHPET"		 // pci_setup.c
#define kUSBBusFixKey		 "USBBusFix"		 // usb.c
#define kEHCIacquireKey		 "EHCIacquire"		 // usb.c
#define kEHCIhardKey		 "EHCIhard"			 // usb.c - ????? internal
#define kUHCIresetKey		 "UHCIreset"		 // usb.c
#define kLegacyOffKey		 "USBLegacyOff"		 // usb.c

/*
 * Flags to the booter and/or kernel - these end with "Flag".
 */
#define kVerboseModeFlag	 "-v"				 // options.c
#define kSafeModeFlag		 "-x"				 // options.c
#define kIgnoreCachesFlag	 "-f"				 // options.c
#define kIgnoreBootFileFlag	 "-F"				 // options.c
#define kSingleUserModeFlag	 "-s"				 // options.c
#define kLegacyModeFlag		 "-legacy"			 // boot.c
#define kArchI386Flag		 "32"				 // boot.c - to be reverted!?
#define kArchX86_64Flag		 "64"				 // boot.c - to be reverted!?

/*
 * Booter behavior control
 */
#define kBootTimeout          -1
#define kCDBootTimeout        8

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
