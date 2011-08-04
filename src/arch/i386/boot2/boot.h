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

#include <sys/types.h>

/*
 * Keys used in system Boot.plist
 */
#define kGraphicsModeKey	"Graphics Mode"
#define kTextModeKey		"Text Mode"
#define kQuietBootKey		"Quiet Boot"
#define kKernelFlagsKey		"Kernel Flags"
#define karch				"arch"				/* boot.c */
#define kProductVersion		"ProductVersion"	/* boot.c */
#define kMKextCacheKey		"MKext Cache"
#define kKernelNameKey		"Kernel"
#define kKernelCacheKey		"Kernel Cache"
#define kUseKernelCache		"UseKernelCache"	/* boot.c */
#define kBootDeviceKey		"Boot Device"
#define kTimeoutKey			"Timeout"
#define kRootDeviceKey		"rd"
#define kBootUUIDKey		"boot-uuid"
#define kHelperRootUUIDKey	"Root UUID"
#define kPlatformKey		"platform"
#define kACPIKey			"acpi"
#define kCDROMPromptKey		"CD-ROM Prompt"
#define kCDROMOptionKey		"CD-ROM Option Key"
#define kRescanPromptKey	"Rescan Prompt"
#define kRescanKey		    "Rescan"
#define kScanSingleDriveKey	"Scan Single Drive"
#define kInsantMenuKey		"Instant Menu"
#define kDefaultKernel		"mach_kernel"
#define kGUIKey				"GUI"
#define kBootBannerKey		"Boot Banner"
#define kWaitForKeypressKey	"Wait"

#define kDSDT				"DSDT"				/* acpi_patcher.c */
#define kDropSSDT			"DropSSDT"			/* acpi_patcher.c */
#define kRestartFix			"RestartFix"		/* acpi_patcher.c */
#define kGeneratePStates	"GeneratePStates"	/* acpi_patcher.c */
#define kGenerateCStates	"GenerateCStates"	/* acpi_patcher.c */
#define kEnableC2States		"EnableC2State"		/* acpi_patcher.c */
#define kEnableC3States		"EnableC3State"		/* acpi_patcher.c */
#define kEnableC4States		"EnableC4State"		/* acpi_patcher.c */

#define kWake				"Wake"				/* boot.c */
#define kForceWake			"ForceWake"			/* boot.c */
#define kWakeImage			"WakeImage"			/* boot.c */

#define kbusratio			"busratio"			/* cpu.c */

#define kDeviceProperties	"device-properties"	/* device_inject.c */

#define kHidePartition		"Hide Partition"	/* disk.c */
#define kRenamePartition	"Rename Partition"	/* disk.c */
#define kDefaultPartition	"Default Partition"	/* sys.c */

#define kSMBIOSKey			"SMBIOS"			/* fake_efi.c */
#define kSMBIOSdefaults		"SMBIOSdefaults"	/* smbios_patcher.c */
#define kSystemID			"SystemId"			/* fake_efi.c */
#define kSystemType			"SystemType"		/* fake_efi.c */

#define kUseMemDetect		"UseMemDetect"	    /* platform.c */

#define kPCIRootUID			"PCIRootUID"		/* pci_root.c */

#define kUseAtiROM			"UseAtiROM"			/* ati.c */
#define kAtiConfig			"AtiConfig"			/* ati.c */
#define kATYbinimage		"ATYbinimage"		/* ati.c */

#define kUseNvidiaROM		"UseNvidiaROM"		/* nvidia.c */
#define kVBIOS				"VBIOS"				/* nvidia.c */
#define kdcfg0				"display_0"			/* nvidia.c */
#define kdcfg1				"display_1"			/* nvidia.c */

#define kEthernetBuiltIn	"EthernetBuiltIn"	/* pci_setup.c */
#define kGraphicsEnabler	"GraphicsEnabler"	/* pci_setup.c */
#define kForceHPET			"ForceHPET"			/* pci_setup.c */

#define kMD0Image			"md0"				/* ramdisk.h */

#define kUSBBusFix			"USBBusFix"			/* usb.c */
#define kEHCIacquire		"EHCIacquire"		/* usb.c */
#define kUHCIreset			"UHCIreset"			/* usb.c */
#define kLegacyOff			"USBLegacyOff"		/* usb.c */
#define kEHCIhard			"EHCIhard"			/* usb.c */

/*
 * Flags to the booter or kernel
 */
#define kVerboseModeFlag	"-v"
#define kSafeModeFlag		"-x"
#define kIgnoreCachesFlag	"-f"
#define kIgnoreBootFileFlag	"-F"
#define kSingleUserModeFlag	"-s"

/*
 * Booter behavior control
 */
#define kBootTimeout         -1
#define kCDBootTimeout       8

extern void initialize_runtime();
extern void common_boot(int biosdev);


#endif /* !__BOOT2_BOOT_H */
