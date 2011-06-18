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

//#define DEBUG 1

//#include "bootstruct.h"
//#include "libsa.h"
#include "boot.h"
//#include "io_inline.h" // Lion
#include "ramdisk.h" // bootstruct.h: memory.h instead of saio_internal.h
#include "gui.h"
#include "modules.h"
#include "fake_efi.h"
#include "platform.h" // bootstruct.h - device_tree.h
#include "sl.h" // bootstruct.h: memory.h instead of saio_types.h

long gBootMode; /* defaults to 0 == kBootModeNormal */
bool gOverrideKernel;
//--- testing
#define PLATFORM_NAME_LEN 64
#define ROOT_PATH_LEN 256
static char gCacheNameAdler[PLATFORM_NAME_LEN + ROOT_PATH_LEN];
#define BOOT_DEVICE_PATH "\\System\\Library\\CoreServices\\boot.efi"
static char gBootKernelCacheFile[512];
//---
char *gPlatformName = gCacheNameAdler; // disabled ??
char gRootDevice[512];
char gMKextName[512];
bool gEnableCDROMRescan;
bool gScanSingleDrive;

int     bvCount = 0; // global ?? - Slice
//int		menucount = 0;
int     gDeviceCount = 0;

BVRef   bvr;
//BVRef   menuBVR; - doesn't seem used here
BVRef   bvChain;
bool    useGUI;

//static void selectBiosDevice(void);
//Azi: this doesn't match the function; matches the "Alder32" on drivers.c
static unsigned long Adler32(unsigned char *buffer, long length);


static bool gUnloadPXEOnExit = false;

/*
 * How long to wait (in seconds) to load the
 * kernel after displaying the "boot:" prompt.
 */
#define kBootErrorTimeout 5

/*
 * Default path to kernel cache file
 */
// OS X 10.4 & 10.5
#define kCachePathTigerLeopard "/System/Library/Caches/com.apple.kernelcaches/kernelcache"
// OS X 10.6
#define kCachePathSnowLion "/System/Library/Caches/com.apple.kext.caches/Startup/kernelcache"

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
//Initializes the runtime. Right now this means zeroing the BSS and initializing malloc.
//
void initialize_runtime(void)
{
	zeroBSS();
	malloc_init(0, 0, 0, malloc_error);
}

//==========================================================================
// execKernel - Load the kernel image (mach-o) and jump to its entry point.

//Azi:autoresolution
extern void initAutoRes();
extern void finishAutoRes();

static int ExecKernel(void *binary)
{
    entry_t                   kernelEntry;
    int                       ret;

    bootArgs->kaddr = bootArgs->ksize = 0;
	execute_hook("ExecKernel", (void*)binary, NULL, NULL, NULL);

    //Azi: gHaveKernelCache is set here
    ret = DecodeKernel(binary,
                       &kernelEntry,
                       (char **) &bootArgs->kaddr,
                       (int *)&bootArgs->ksize );

    if ( ret != 0 )
        return ret;

    // Reserve space for boot args
    reserveKernBootStruct();

    //Azi: ...
    // Load boot drivers from the specifed root path,
    // if we don't have a prelinked kernel - check load.c 43 & 264
    if (!gHaveKernelCache) {
          LoadDrivers("/");
    }

    clearActivityIndicator();

    if (gErrors) {
        printf("Errors encountered while starting up the computer.\n");
        printf("Pausing %d seconds...\n", kBootErrorTimeout);
        sleep(kBootErrorTimeout);
    }

    setupFakeEfi(); //Azi: check position on Mek (plkernel)

    md0Ramdisk();

    verbose("Starting Darwin %s\n",( archCpuType == CPU_TYPE_I386 ) ? "x86" : "x86_64");

    // Cleanup the PXE base code.

    if ( (gBootFileType == kNetworkDeviceType) && gUnloadPXEOnExit ) {
		if ( (ret = nbpUnloadBaseCode()) != nbpStatusSuccess )
        {
        	printf("nbpUnloadBaseCode error %d\n", (int) ret);
            sleep(2);
        }
    }

    bool dummyVal;

	//Azi: Wait=y is breaking other keys when typed "after them" at boot prompt.
	// Works properly if typed in first place or used on Boot.plist.
	if (getBoolForKey(kWaitForKeypressKey, &dummyVal, &bootInfo->bootConfig) && dummyVal) {
		verbose("(Wait) ");
		pause();
	}

	usb_loop();

//autoresolution - Check if user disabled AutoResolution at the boot prompt.
	// we can't check the plist here if we have AutoResolution=n there and we anabled it
	// at boot prompt...?????
//	getBoolForKey(kAutoResolutionKey, &gAutoResolution, &bootInfo->bootConfig);
	finishAutoRes();
	
	//Azi: closing Vbios after "if (gVerboseMode)" stuff eliminates the need for setting
	// AutoResolution = true above; but creates another bug when booting in TextMode with -v arg.
	// Simptoms include: staring some seconds at a nicely drawn white screen, after boot prompt.
	// Think i'm just going to end up removing setting gAutoResolution = false
	// on closeVbios().. the more i think, the less sense it makes doing it there!!
//autoresolution - end

	// Notify modules that the kernel is about to be started
	// testing...
	
	if (gMacOSVersion[3] <= '6')
		execute_hook("Kernel Start", (void*)kernelEntry, (void*)bootArgsPreLion, NULL, NULL);
	else
		execute_hook("Kernel Start", (void*)kernelEntry, (void*)bootArgs, NULL, NULL);

    // If we were in text mode, switch to graphics mode.
    // This will draw the boot graphics unless we are in
    // verbose mode.

    if(gVerboseMode)
      setVideoMode( GRAPHICS_MODE, 0 );
    else
      drawBootGraphics();

	setupBooterLog();
	
    finalizeBootStruct();

//Azi: see asm.s LABEL(_disableIRQs)
//	outb(0x21, 0xff);
//	outb(0xa1, 0xff);

    // Jump to kernel's entry point. There's no going back now.
	if (gMacOSVersion[3] <= '6')
		startprog( kernelEntry, bootArgsPreLion );
	else
		startprog( kernelEntry, bootArgs );

    // Not reached
    return 0;
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
    int      status;
    char     *bootFile;
    unsigned long adler32;
    bool     quiet;
    bool     firstRun = true;
    bool     instantMenu;
    bool     rescanPrompt = false;
    unsigned int allowBVFlags = kBVFlagSystemVolume|kBVFlagForeignBoot;
    unsigned int denyBVFlags = kBVFlagEFISystem;

    // Set reminder to unload the PXE base code. Neglect to unload
    // the base code will result in a hang or kernel panic.
    gUnloadPXEOnExit = true;

    // Record the device that the booter was loaded from.
    gBIOSDev = biosdev & kBIOSDevMask;

    // Initialize boot info structure.
    initKernBootStruct();

	initBooterLog();
	
	//Azi: log booter version, revision & build date, for bdmesg.
	msglog(bootLogBanner);

    // Setup VGA text mode.
    // Not sure if it is safe to call setVideoMode() before the
    // config table has been loaded. Call video_mode() instead.
#if DEBUG
    printf("before video_mode\n"); //Azi: this one is not printing... i remember it did.. check trunk.
#endif
    video_mode( 2 );  // 80x25 mono text mode.
#if DEBUG
    printf("after video_mode\n");
#endif

    // Scan and record the system's hardware information.
    scan_platform();

    // First get info for boot volume.
    scanBootVolumes(gBIOSDev, 0);
    bvChain = getBVChainForBIOSDev(gBIOSDev);
	//Azi: initialising gBIOSBootVolume & gBootVolume for the first time.. i think!?
	// also, kDefaultPartitionKey is checked here, on selectBootVolume.
    setBootGlobals(bvChain);
	msglog("setBootGlobals:\n Default: %d, ->biosdev: %d, ->part_no: %d ->flags: %d\n", gBootVolume, gBootVolume->biosdev, gBootVolume->part_no, gBootVolume->flags);
    msglog(" bt(0,0): %d, ->biosdev: %d, ->part_no: %d ->flags: %d\n", gBIOSBootVolume, gBIOSBootVolume->biosdev, gBIOSBootVolume->part_no, gBIOSBootVolume->flags);

	// Boot Volume is set as Root at this point so, pointing to Extra, /Extra or bt(0,0)/Extra
	// is exactly the same.	Review bt(0,0)/bla bla paths......			(Reviewing...)
	
	//Azi: works as expected but... trying this because Kernel=mach_kernel doesn't work on a
	// override Boot.plist; this makes it impossible to override e.g. Kernel=bt(0,0)mach_kernel
	// on the main Boot.plist, when loading kernel from ramdisk btAliased.
	loadPrebootRAMDisk();
	
    // Load boot.plist config file
    //Azi: on this first check, boot.plist acts as both "booter config file"
    // and bootargs/options "carrier".*****
    status = loadSystemConfig(&bootInfo->bootConfig);

    if (getBoolForKey(kQuietBootKey, &quiet, &bootInfo->bootConfig) && quiet) {
        gBootMode |= kBootModeQuiet;
    }

    // Override firstRun to get to the boot menu instantly by setting "Instant Menu"=y in system config
    if (getBoolForKey(kInstantMenuKey, &instantMenu, &bootInfo->bootConfig) && instantMenu) {
        firstRun = false;
    }

	// Loading preboot ramdisk if exists.
//	loadPrebootRAMDisk(); //Azi: this needs to be done before load_all_modules()
	// because of btAlias...			(Reviewing...)

	// Intialize module system
	if (init_module_system())
	{
		load_all_modules();
	}

    // Disable rescan option by default
    gEnableCDROMRescan = false;

    // If we're loading the booter from optical media...			(Reviewing...)
	if (biosDevIsCDROM(gBIOSDev))
	{
		// ... ask the user for Rescan option by setting "Rescan Prompt"=y in system config...
		if (getBoolForKey(kRescanPromptKey, &rescanPrompt, &bootInfo->bootConfig) && rescanPrompt)
		{
	        gEnableCDROMRescan = promptForRescanOption();
	    }
		else // ... or enable it with Rescan=y in system config.
	    if (getBoolForKey(kRescanKey, &gEnableCDROMRescan, &bootInfo->bootConfig) && gEnableCDROMRescan)
		{
	        gEnableCDROMRescan = true;
	    }
	}

	//Azi: Is this a cdrom only thing?			(Reviewing...)
    // Enable touching a single BIOS device only if "Scan Single Drive"=y is set in system config.
    if (getBoolForKey(kScanSingleDriveKey, &gScanSingleDrive, &bootInfo->bootConfig) && gScanSingleDrive)
	{
		scanBootVolumes(gBIOSDev, &bvCount);
    }
	else
	{
		//Azi: scanDisks uses scanBootVolumes.
		scanDisks(gBIOSDev, &bvCount);
	}
	
    // Create a separated bvr chain using the specified filters.
    bvChain = newFilteredBVChain(0x80, 0xFF, allowBVFlags, denyBVFlags, &gDeviceCount);

    gBootVolume = selectBootVolume(bvChain);

//#if DEBUG
//printf
    msglog(":something...???\n Default: %d, ->biosdev: %d, ->part_no: %d ->flags: %d\n", gBootVolume, gBootVolume->biosdev, gBootVolume->part_no, gBootVolume->flags);
    msglog(" bt(0,0): %d, ->biosdev: %d, ->part_no: %d ->flags: %d\n", gBIOSBootVolume, gBIOSBootVolume->biosdev, gBIOSBootVolume->part_no, gBIOSBootVolume->flags);
//    getchar(); //getc(); Azi: getc stuff - DON'T FORGET pause() - take modules.c as reference.
//#endif

    useGUI = true;
    // Override useGUI default
    getBoolForKey(kGUIKey, &useGUI, &bootInfo->bootConfig);

	// AutoResolution - Azi: default to false
	// http://forum.voodooprojects.org/index.php/topic,1227.0.html
	gAutoResolution = false;
	
	// Check if user enabled AutoResolution on Boot.plist...
	getBoolForKey(kAutoResolutionKey, &gAutoResolution, &bootInfo->bootConfig);
	
	// Patch the Video Bios with the extracted resolution, before initGui.
	if (gAutoResolution == true)
	{
		initAutoRes();
	}

    if (useGUI && initGUI())
	{
		// initGUI() returned with an error, disabling GUI.
		useGUI = false;
	}

    setBootGlobals(bvChain);
	msglog("setBootGlobals:\n Default: %d, ->biosdev: %d, ->part_no: %d ->flags: %d\n", gBootVolume, gBootVolume->biosdev, gBootVolume->part_no, gBootVolume->flags);
    msglog(" bt(0,0): %d, ->biosdev: %d, ->part_no: %d ->flags: %d\n", gBIOSBootVolume, gBIOSBootVolume->biosdev, gBIOSBootVolume->part_no, gBIOSBootVolume->flags);

    // Parse args, load and start kernel.
    while (1) {
        const char *val;
        int len;
        int trycache;
		long flags, cachetime, kerneltime, exttime, sleeptime, time;
        int ret = -1;
        void *binary = (void *)kLoadAddr;
        bool tryresume;
        bool tryresumedefault;
        bool forceresume;
		bool ignoreKC = false;

        // additional variable for testing alternate kernel image locations on boot helper partitions.
        char     bootFileSpec[512];
		
        // Initialize globals.

        sysConfigValid = false;
        gErrors        = false;

        status = getBootOptions(firstRun);
        firstRun = false;
        if (status == -1) continue;

		//Azi: test (gBootVolume == NULL) - so far Ok!
		// test with optical media again...?
		// Turn off any GUI elements, draw background and update VRAM.
		if ( bootArgs->Video.v_display == GRAPHICS_MODE )
		{
			gui.devicelist.draw = false;
			gui.bootprompt.draw = false;
			gui.menu.draw = false;
			gui.infobox.draw = false;
			gui.logo.draw = false;
			drawBackground();
			updateVRAM();
		}

		status = processBootOptions();

		//Azi: AutoResolution -  closing Vbios here without restoring, causes an allocation error,
		// if the user tries to boot, after a e.g."Can't find bla_kernel" msg.
		// Doing it on execKernel() instead.

		// Status == 1 means to chainboot
		if ( status ==	1 ) break;
		
		// Status == -1 means that gBootVolume is NULL. Config file is not mandatory anymore! 
		if ( status == -1 )
		{
			// gBootVolume == NULL usually means the user hit escape.			(Reviewing...)
			if (gBootVolume == NULL)
			{
				freeFilteredBVChain(bvChain);
				
				if (gEnableCDROMRescan)
					rescanBIOSDevice(gBIOSDev);
				
				bvChain = newFilteredBVChain(0x80, 0xFF, allowBVFlags, denyBVFlags, &gDeviceCount);
				setBootGlobals(bvChain);
				setupDeviceList(&bootInfo->themeConfig);
			}
			continue;
		}
		
        // Other status (e.g. 0) means that we should proceed with boot.

		// If cpu handles 64 bit instructions...
		if (platformCPUFeature(CPU_FEATURE_EM64T))
		{
			// use x86_64 kernel arch,...
			archCpuType = CPU_TYPE_X86_64;
		}
		else
		{
			// else use i386 kernel arch.
			archCpuType = CPU_TYPE_I386;
		}
		// If user override...
		if (getValueForKey(kArchKey, &val, &len, &bootInfo->bootConfig))
		{
			// matches i386...
			if (strncmp(val, "i386", 4) == 0)
			{
				// use i386 kernel arch.
				archCpuType = CPU_TYPE_I386;
			}
		}
		
		if (!getBoolForKey (kWakeKey, &tryresume, &bootInfo->bootConfig)) {
			tryresume = true;
			tryresumedefault = true;
		} else {
			tryresumedefault = false;
		}

		if (!getBoolForKey (kForceWakeKey, &forceresume, &bootInfo->bootConfig)) {
			forceresume = false;
		}
		
		if (forceresume) {
			tryresume = true;
			tryresumedefault = false;
		}
		
		while (tryresume) {
			const char *tmp;
			BVRef bvr;
			if (!getValueForKey(kWakeKeyImageKey, &val, &len, &bootInfo->bootConfig))
				val="/private/var/vm/sleepimage";
			
			// Do this first to be sure that root volume is mounted
			ret = GetFileInfo(0, val, &flags, &sleeptime);

			if ((bvr = getBootVolumeRef(val, &tmp)) == NULL)
				break;
			
			// Can't check if it was hibernation Wake=y is required
			if (bvr->modTime == 0 && tryresumedefault)
				break;
			
			if ((ret != 0) || ((flags & kFileTypeMask) != kFileTypeFlat))
				break;
			
			if (!forceresume && ((sleeptime+3)<bvr->modTime)) {
				//Azi: no need for printf at this point - reminder
				printf ("Hibernate image is too old by %d seconds. Use ForceWake=y to override.\n",bvr->modTime-sleeptime);
				break;
			}
				
			HibernateBoot((char *)val);
			break;
		}

//Azi:kernelcache stuff
		bool patchKernel = false;
		getBoolForKey(kKPatcherKey, &patchKernel, &bootInfo->bootConfig);
		//Azi: avoiding having to use -f to ignore kernel cache
		//Azi: ignore kernel cache but still use kext cache (E/E.mkext & S/L/E.mkext). - explain...
		getBoolForKey(kUseKCKey, &ignoreKC, &bootInfo->bootConfig); // equivalent to UseKernelCache
		if (ignoreKC)
		{
			verbose("KC: cache ignored by user.\n");
			// make sure the damn thing get's zeroed, just in case... :)*
			bzero(gBootKernelCacheFile, sizeof(gBootKernelCacheFile));
		}
		else if (patchKernel) // to be moved..?
		{
			verbose("KC: kernel patcher enabled, ignore cache.\n");
			bzero(gBootKernelCacheFile, sizeof(gBootKernelCacheFile));
		}
		else if (getValueForKey(kKernelCacheKey, &val, &len, &bootInfo->bootConfig))
		{
            strlcpy(gBootKernelCacheFile, val, len + 1);
			verbose("KC: path set by user = %s\n", gBootKernelCacheFile);
			//Azi: bypass time check when user sets path ???
			// cache is still ignored if time doesn't match... (e.g. booter on usb stick)
        }
		else
		{
			// Reset cache name.
			bzero(gCacheNameAdler + 64, sizeof(gCacheNameAdler) - 64);
			
			// kextcache_main.c: Construct entry from UUID of boot volume...(reminder)
			// assemble ?string? to generate adler from...
//			sprintf(gCacheNameAdler + 64, "%s,%s", gRootDevice, bootInfo->bootFile);
			const char *ProductName = getStringForKey("SMproductname", &bootInfo->smbiosConfig);
			sprintf(gCacheNameAdler, ProductName); // well, at least the smbios.plist can be loaded this early...
			// to set/get "ProductName" this early, booter needs complete rewrite!!
			// see DHP's Revolution booter rework example!
			verbose("KC: gCacheNameAdler 1 = %s\n", gCacheNameAdler);
			//Azi: check the validity of this, e.g. on Helper Partitions
			sprintf(gCacheNameAdler + 64, "%s", "\\System\\Library\\CoreServices\\boot.efi");
			verbose("KC: gCacheNameAdler 2 = %s\n", gCacheNameAdler + 64);
			sprintf(gCacheNameAdler + (64 + 38), "%s", bootInfo->bootFile);
			verbose("KC: gCacheNameAdler 3 = %s\n", gCacheNameAdler + (64 + 38));

			// generate adler
			adler32 = Adler32((unsigned char *)gCacheNameAdler, sizeof(gCacheNameAdler));
			verbose("KC: Adler32 = %08X\n", adler32);
//Azi: no check for OS version here ?? - yes there is :)
			// append arch and/or adler (checksum) to kc path...
			if (gMacOSVersion[3] <= '5')
			{
				sprintf(gBootKernelCacheFile, "%s.%08lX", kCachePathTigerLeopard, adler32);
				verbose("KC: adler added to path = %s\n", gBootKernelCacheFile);
			}
			else
			{
				sprintf(gBootKernelCacheFile, "%s_%s.%08X", kCachePathSnowLion,
						(archCpuType == CPU_TYPE_I386) ? "i386" : "x86_64", adler32);
				verbose("KC: arch & adler added to path = %s\n", gBootKernelCacheFile);
			}
        }

        // Check for cache file.
		//Azi: trycache is done if...
        trycache = ( ( (gBootMode & kBootModeSafe) == 0) //... we're not booting in safe mode (-x arg),
					&& !gOverrideKernel // we're not overriding default kernel "name",
					&& (gBootFileType == kBlockDeviceType) // we're booting from local storage device,
					&& (gMKextName[0] == '\0') // "MKext Cache" key IS NOT in use, and
					&& (gBootKernelCacheFile[0] != '\0') ); // gBootKernelCacheFile is populated.
					// we could add the use of "kernelpatcher" to this bunch..??

//		verbose("Loading Darwin %s\n", gMacOSVersion); //Azi: move?? to getOSVersion? :)
		
        if (trycache) do
		{
			verbose("KC: checking kernel cache (system prelinked kernel)...\n");
            // if we haven't found the kernel yet, don't use the cache
            ret = GetFileInfo(NULL, bootInfo->bootFile, &flags, &kerneltime);
            if ((ret != 0) || ((flags & kFileTypeMask) != kFileTypeFlat))
            {
				verbose("KC: no kernel found (shouldn't happen?!?)\n");
                trycache = 0; // ignore kernel cache...
                break;
            }
			verbose("KC: kerneltime = %d\n", kerneltime);
			
            ret = GetFileInfo(NULL, gBootKernelCacheFile, &flags, &cachetime);
            if ((ret != 0) || ((flags & kFileTypeMask) != kFileTypeFlat)
                || (cachetime < kerneltime))
            {
				if (cachetime <= 100) // confirm: 100 = inexistent path, -xxxxxxxxx = wrong name
				// not confirming... i also get -xxxxxxxxx with inexisting prelinked kernel
					verbose("KC: cachetime  = %d, kernel cache path/adler is incorrect, ignoring it. ??? \n",
							cachetime);
				else
					verbose("KC: cachetime  = %d, kernel cache is older than the kernel, ignoring it.\n",
							cachetime);
                trycache = 0;
                break;
            }
			verbose("KC: cachetime  = %d\n", cachetime);
			
            ret = GetFileInfo("/System/Library/", "Extensions", &flags, &exttime);
            if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeDirectory)
                && (cachetime < exttime))
            {
				verbose("KC: exttime    = %d, kernel cache is older than S/L/E, ignoring it.\n", exttime);
                trycache = 0;
                break;
            }
			verbose("KC: exttime    = %d\n", exttime);
			
            if (kerneltime > exttime) // if S/L/E is older than the kernel...
            {
				verbose("KC: S/L/E is older than the kernel, matching exttime with kerneltime...\n");
                exttime = kerneltime;
            }
			verbose("KC: exttime +1 = %d\n", exttime + 1);
			
            if (cachetime != (exttime + 1))
            {
				verbose("KC: kernel cache time is diff from S/L/E time, ignoring it.\n");
                trycache = 0;
                break;
            }
			verbose("KC: kernel cache found and up to date, will be used.\n");

        } while (0);

        do
        {
			// Load kernel cache if not ignored.
            if (trycache)
            {
                bootFile = gBootKernelCacheFile;
                verbose("Loading kernel cache %s\n", bootFile);

                ret = LoadFile(bootFile);
                binary = (void *)kLoadAddr;

                if (ret >= 0)
                {
                    break;
                }
            }

            bootFile = bootInfo->bootFile;

            // Try to load kernel image from alternate locations on boot helper partitions.
            sprintf(bootFileSpec, "com.apple.boot.P/%s", bootFile);
            ret = GetFileInfo(NULL, bootFileSpec, &flags, &time); 
  	  	    if (ret == -1)
  	  	    {
				sprintf(bootFileSpec, "com.apple.boot.R/%s", bootFile);
				ret = GetFileInfo(NULL, bootFileSpec, &flags, &time); 
				if (ret == -1)
				{
					sprintf(bootFileSpec, "com.apple.boot.S/%s", bootFile);
					ret = GetFileInfo(NULL, bootFileSpec, &flags, &time); 
					if (ret == -1)
					{
						// No alternate location found, using the original kernel image path.
						strcpy(bootFileSpec, bootFile);
					}
				}
            }
            			
            verbose("Loading kernel %s\n", bootFileSpec);
            ret = LoadThinFatFile(bootFileSpec, &binary);
            if (ret <= 0 && archCpuType == CPU_TYPE_X86_64)
            {
				archCpuType = CPU_TYPE_I386;
				ret = LoadThinFatFile(bootFileSpec, &binary);				
            }
			
        } while (0);

        clearActivityIndicator();
/*#if DEBUG
        printf("Pausing...");
        sleep(8);
#endif
Azi: annoying stuff :P */
        if (ret <= 0)
		{
			printf("Can't find %s\n", bootFile);

			sleep(1);

            if (gBootFileType == kNetworkDeviceType) {
                // Return control back to PXE. Don't unload PXE base code.
                gUnloadPXEOnExit = false;
                break;
            }
        }
		else
		{
            // Won't return if successful.
            ret = ExecKernel(binary);
        }
    } // while (1)
    
    // chainboot
    if (status==1) {
		if (getVideoMode() == GRAPHICS_MODE) {	// if we are already in graphics-mode,
			setVideoMode(VGA_TEXT_MODE, 0);	// switch back to text mode
		}
    }
	
    if ((gBootFileType == kNetworkDeviceType) && gUnloadPXEOnExit)
	{
		nbpUnloadBaseCode();
    }
}

/*!
    Selects a new BIOS device, taking care to update the global state appropriately.
 */
/*
static void selectBiosDevice(void)
{
    struct DiskBVMap *oldMap = diskResetBootVolumes(gBIOSDev);
    CacheReset();
    diskFreeMap(oldMap);
    oldMap = NULL;

    int dev = selectAlternateBootDevice(gBIOSDev);

    BVRef bvchain = scanBootVolumes(dev, 0);
    BVRef bootVol = selectBootVolume(bvchain);
    gBootVolume = bootVol;
    setRootVolume(bootVol);
    gBIOSDev = dev;
}
*/

#define BASE 65521L /* largest prime smaller than 65536 */
#define NMAX 5000
// NMAX (was 5521) the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1

#define DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

unsigned long Adler32(unsigned char *buf, long len)
{
    unsigned long s1 = 1; // adler & 0xffff;
    unsigned long s2 = 0; // (adler >> 16) & 0xffff;
    unsigned long result;
    int k;

    while (len > 0) {
        k = len < NMAX ? len : NMAX;
        len -= k;
        while (k >= 16) {
            DO16(buf);
            buf += 16;
            k -= 16;
        }
        if (k != 0) do {
            s1 += *buf++;
            s2 += s1;
        } while (--k);
        s1 %= BASE;
        s2 %= BASE;
    }
    result = (s2 << 16) | s1;
    return OSSwapHostToBigInt32(result);
}
