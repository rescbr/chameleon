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
#include "bootstruct.h"
#include "fake_efi.h"
#include "sl.h"
#include "libsa.h"
#include "platform.h"
#include "graphics.h"
#include "appleboot.h"
#include "modules.h"
#include "xml.h"
#include "options.h"
#include "drivers.h"

#ifndef DEBUG_BOOT
#define DEBUG_BOOT 0
#endif

#if DEBUG_BOOT
#define DBG(x...)		printf(x)
#else
#define DBG(x...)
#endif


typedef struct platform_info {
	char platformName[PLATFORM_NAME_LEN];
	char rootPath[ROOT_PATH_LEN];
} PlatformInfo;

static BVRef   bvChain;
static bool forcecache = false;

static char gBootKernelCacheFile[Cache_len_name];
static char gMKextName[Cache_len_name];
static void zeroBSS(void);
static int ExecKernel(void *binary);
static void getRootDevice();
static bool find_file_with_ext(const char* dir, const char *ext, const char * name_compare, size_t ext_size);
static bool found_extra_kext(void);
static void determineCpuArch(void);
void getKernelCachePath(void);
#ifdef NBP_SUPPORT
static bool gUnloadPXEOnExit = false;
#endif
static void getRootDevice(void);

/*
 * How long to wait (in seconds) to load the
 * kernel after displaying the "boot:" prompt.
 */
#define kBootErrorTimeout 5

/*
 * Default path to kernel cache file
 */
#define kDefaultCachePath "/System/Library/Caches/com.apple.kext.caches/Startup/kernelcache"

//==========================================================================
// Zero the BSS.

static void zeroBSS(void)
{
#if (defined(__clang__)) && (defined(__XCODE45_HACK__))/* WARNING : must be first, __GNUC__ seems to be also defined */
	
	extern int  bss_start  __asm("section$start$__DATA$__bss");
    extern int  bss_end    __asm("section$end$__DATA$__bss");
    extern int  common_start  __asm("section$start$__DATA$__common");
    extern int  common_end    __asm("section$end$__DATA$__common");
	
	bzero(&bss_start, (&bss_end - &bss_start));
	bzero(&common_start, (&common_end - &common_start));
	
#elif (defined(__GNUC__)) || (defined(__llvm__))
	
	extern char _DATA__bss__begin, _DATA__bss__end;
	extern char _DATA__common__begin, _DATA__common__end;
	
	bzero(&_DATA__bss__begin, (&_DATA__bss__end - &_DATA__bss__begin));
	bzero(&_DATA__common__begin, (&_DATA__common__end - &_DATA__common__begin));
	
#endif
}

//==========================================================================
// Malloc error function

#ifdef SAFE_MALLOC
static inline void malloc_error(char *addr, size_t size, const char *file, int line)
{
    stop("\nMemory allocation error! Addr=0x%x, Size=0x%x, File=%s, Line=%d\n", (unsigned)addr, (unsigned)size, file, line);
}
#else
static inline void malloc_error(char *addr, size_t size)
{
    printf("\nMemory allocation error (0x%x, 0x%x)\n", (unsigned)addr, (unsigned)size);
    asm volatile ("hlt");
}
#endif

BVRef getBvChain(void)
{
	return bvChain;
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
// execKernel - Load the kernel image (mach-o) and jump to its entry point.

static int ExecKernel(void *binary)
{
    entry_t                   kernelEntry;
    int                       ret;
    
    bootArgs->kaddr = bootArgs->ksize = 0;
		
	{
		bool KPRebootOption = false;
		bool HiDPIOption = false;

		getBoolForKey(kRebootOnPanic, &KPRebootOption, DEFAULT_BOOT_CONFIG);
		if (KPRebootOption == true) bootArgs->flags |= kBootArgsFlagRebootOnPanic;
		
		getBoolForKey(kEnableHiDPI, &HiDPIOption, DEFAULT_BOOT_CONFIG);
		if (HiDPIOption == true) bootArgs->flags |= kBootArgsFlagHiDPI;
	}
	
	if(((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion[3] <= '6')
	{
		bootArgs->Version  = kBootArgsVersion1;
		bootArgs->Revision = ((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion[3];
	}
	else
	{
#if kBootArgsVersion > 1
		
		bootArgs->Version  = kBootArgsVersion;
		bootArgs->Revision = kBootArgsRevision;
#else
		if(((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion[3] >= '7')
		{
			bootArgs->Version  = 2;
			bootArgs->Revision = 0;
		}
#endif
	}
	
	execute_hook("ExecKernel", (void*)binary, NULL, NULL, NULL, NULL, NULL);
    
    ret = DecodeKernel(binary,
                       &kernelEntry,
                       (char **) &bootArgs->kaddr,
                       (int *)&bootArgs->ksize );
    
    if ( ret != 0 )
        return ret;
    
    // Reserve space for boot args for 10.7 only (for 10.6 and earlier, we will convert (to legacy) the structure and reserve kernel memory for it later.)
	if(((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion[3] >= '7')
        reserveKernBootStruct();
	
    // Load boot drivers from the specifed root path.
	
    if (!get_env(envgHaveKernelCache))
	{
		LoadDrivers("/");
    }
    
    showError();
	
	execute_hook("md0Ramdisk", NULL, NULL, NULL, NULL, NULL, NULL);
    
    setupFakeEfi();
    
    verbose("Starting Darwin %s\n",( get_env(envarchCpuType) == CPU_TYPE_I386 ) ? "x86" : "x86_64");
#ifdef NBP_SUPPORT
    // Cleanup the PXE base code.
	
    if ( (get_env(envgBootFileType) == kNetworkDeviceType) && gUnloadPXEOnExit )
	{
		if ( (ret = nbpUnloadBaseCode()) != nbpStatusSuccess )
        {
        	printf("nbpUnloadBaseCode error %d\n", (int) ret);
            sleep(2);
        }
    }
#endif
	{
		bool wait = false;
		const char *strval = 0;
		int dummysize /*= 0*/;
		
		getBoolForKey(kWaitForKeypressKey, &wait, DEFAULT_BOOT_CONFIG);
		
		if (getValueForBootKey(bootArgs->CommandLine, "-wait", &strval, &dummysize))
		{
			wait = true;
			
			if (strval && ((strncmp(strval, "no", sizeof("no")) == 0) || (strncmp(strval, "No", sizeof("No")) == 0)))
			{
				wait = false;
			}
		}
		
		if (wait == true)
		{
			pause();
		}
	}
    
    //debug_platform_env();
    
	if ((execute_hook("GUI_ExecKernel", NULL, NULL, NULL, NULL, NULL, NULL) != EFI_SUCCESS)) // (bootArgs->Video.v_display == VGA_TEXT_MODE)
	{
        __setVideoMode( GRAPHICS_MODE );
		
        
		if(!get_env(envgVerboseMode))
		{
			__drawColorRectangle(0, 0, DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, 0x01);
            
			uint8_t *appleBootPict;
			uint16_t bootImageWidth = kAppleBootWidth;
			uint16_t bootImageHeight = kAppleBootHeight;
			uint8_t *bootImageData = NULL;
			uint16_t x, y;
			
			unsigned long screen_params[4] = {DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, 32, 0};	// here we store the used screen resolution
			// Prepare the data for the default Apple boot image.
			appleBootPict = (uint8_t *) __decodeRLE(gAppleBootPictRLE, kAppleBootRLEBlocks, bootImageWidth * bootImageHeight);
			if (appleBootPict)
			{
				if(__convertImage(bootImageWidth, bootImageHeight, appleBootPict, &bootImageData) == 0)
                {
                    if (bootImageData)
                    {
                        x = (screen_params[0] - MIN(kAppleBootWidth, screen_params[0])) / 2;
                        y = (screen_params[1] - MIN(kAppleBootHeight, screen_params[1])) / 2;
                        __drawDataRectangle(x, y, kAppleBootWidth, kAppleBootHeight, bootImageData);
                        free(bootImageData);
                    }
                }
				
				free(appleBootPict);
			}
            
		}
	}
    
    finalizeEFIConfigTable();
    
	setupBooterLog();
	
    finalizeBootStruct();
    
	execute_hook("Kernel Start", (void*)kernelEntry, (void*)bootArgs, NULL, NULL, NULL, NULL);	// Notify modules that the kernel is about to be started
	
    if (((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion[3] <= '6')
		reserveKernLegacyBootStruct();
	
#if UNUSED
	turnOffFloppy();
#endif
#if BETA
#include "smp-imps.h"
#include "apic.h"
	IMPS_LAPIC_WRITE(LAPIC_LVT1, LAPIC_ICR_DM_NMI);
#endif
    
	if (((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion[3] <= '6') {
		
		// Jump to kernel's entry point. There's no going back now. XXX LEGACY OS XXX
		startprog( kernelEntry, bootArgsLegacy );
	}
    
	outb(0x21, 0xff);   /* Maskout all interrupts Pic1 */
	outb(0xa1, 0xff);   /* Maskout all interrupts Pic2 */
    
	// Jump to kernel's entry point. There's no going back now. XXX LION XXX
    startprog( kernelEntry, bootArgs );
    
    // Should not be reached
	
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
    bool     firstRun = true;
    int      devcnt = 0;
    int      bvCount = 0;
	int      BIOSDev = 0;
    long     gBootMode = kBootModeNormal; /* defaults to 0 == kBootModeNormal */
    
    unsigned int allowBVFlags = kBVFlagSystemVolume|kBVFlagForeignBoot;
    unsigned int denyBVFlags = kBVFlagEFISystem;
	
#ifdef NBP_SUPPORT
    // Set reminder to unload the PXE base code. Neglect to unload
    // the base code will result in a hang or kernel panic.
    gUnloadPXEOnExit = true;
#endif
    
    // Setup VGA text mode.
    // It's unsafe to call setVideoMode() before the
    // bootargs is initialized, we call video_mode() instead.
#if DEBUG
    printf("before video_mode\n");
#endif
    video_mode( 2 );  // 80x25 mono text mode.
#if DEBUG
    printf("after video_mode\n");
#endif
	printf("Starting Chameleon ...\n");
    
	init_ut_fnc();
    
	initBooterLog();
    
	// Initialize boot info structure.
    initKernBootStruct();
	
    // Scan and record the system's hardware information.
    scan_platform();
	
  
	// Pseudo-random generator initialization.
    // arc4_init();
	
    set_env(envgBIOSDev, (BIOSDev = biosdev & kBIOSDevMask));
    set_env(envShouldboot, false);
    set_env(envkCacheFile, (uint32_t)gBootKernelCacheFile);
    set_env(envMKextName, (uint32_t)gMKextName);
	set_env(envHFSLoadVerbose, 1);
	set_env(envarchCpuType, CPU_TYPE_I386);
	set_env(envgHaveKernelCache, false);
    
    InitBootPrompt();
  
    // First get info for boot volume.
    scanBootVolumes(BIOSDev, 0);
    
    bvChain = getBVChainForBIOSDev(BIOSDev);

    setBootGlobals(bvChain);
    
    // Load Booter boot.plist config file
    loadBooterConfig();
	
    {
        bool isServer = false;
        getBoolForKey(kIsServer, &isServer, DEFAULT_BOOT_CONFIG); // set this as soon as possible
        set_env(envIsServer , isServer);
    }
	
    
	{
		bool     quiet = false;
		if (getBoolForKey(kQuietBootKey, &quiet, DEFAULT_BOOT_CONFIG) && quiet)
		{
			gBootMode |= kBootModeQuiet;
		}
	}
    
    set_env(envgBootMode, gBootMode);
	
	{
		bool     instantMenu = false;
		// Override firstRun to get to the boot menu instantly by setting "Instant Menu"=y in system config
		if (getBoolForKey(kInsantMenuKey, &instantMenu, DEFAULT_BOOT_CONFIG) && instantMenu)
		{
			firstRun = false;
		}
	}
    
    {
        bool ScanSingleDrive = false;
        // Enable touching a single BIOS device only if "Scan Single Drive"=y is set in system config.
        if (getBoolForKey(kScanSingleDriveKey, &ScanSingleDrive, DEFAULT_BOOT_CONFIG) && ScanSingleDrive)
        {
            ScanSingleDrive = true;
        }
        safe_set_env(envgScanSingleDrive, ScanSingleDrive);
        // Create a list of partitions on device(s).
        if (ScanSingleDrive)
        {
            scanBootVolumes(BIOSDev, &bvCount);
        }
        else
        {
            scanDisks();
        }
        
	}
    
    // Create a separated bvr chain using the specified filters.
    bvChain = newFilteredBVChain(0x80, 0xFF, allowBVFlags, denyBVFlags, &devcnt);
    safe_set_env(envgDeviceCount,devcnt);
    
	safe_set_env(envgBootVolume, (uint32_t)selectBootVolume(bvChain));
    
	
	LoadBundles("/Extra/");
    
    // Loading preboot ramdisk if exists.
	execute_hook("loadPrebootRAMDisk", NULL, NULL, NULL, NULL, NULL, NULL);
    
    {
        // Disable rescan option by default
        bool CDROMRescan = false;
        
        // Enable it with Rescan=y in system config
        if (getBoolForKey(kRescanKey, &CDROMRescan, DEFAULT_BOOT_CONFIG) && CDROMRescan)
        {
            CDROMRescan = true;
            
        }
        safe_set_env(envgEnableCDROMRescan, CDROMRescan);
        
    }
    
	
	{
		bool     rescanPrompt = false;
		// Ask the user for Rescan option by setting "Rescan Prompt"=y in system config.
		if (getBoolForKey(kRescanPromptKey, &rescanPrompt , DEFAULT_BOOT_CONFIG) && rescanPrompt && biosDevIsCDROM((int)get_env(envgBIOSDev)))
		{
            safe_set_env(envgEnableCDROMRescan, promptForRescanOption());
		}
	}
	
#if DEBUG
    printf(" Default: %p, ->biosdev: %d, ->part_no: %d ->flags: %d\n", ((BVRef)(uint32_t)get_env(envgBootVolume)), ((BVRef)(uint32_t)get_env(envgBootVolume))->biosdev, ((BVRef)(uint32_t)get_env(envgBootVolume))->part_no, ((BVRef)(uint32_t)get_env(envgBootVolume))->flags);
    printf(" bt(0,0): %p, ->biosdev: %d, ->part_no: %d ->flags: %d\n", ((BVRef)(uint32_t)get_env(envgBIOSBootVolume)), ((BVRef)(uint32_t)get_env(envgBIOSBootVolume))->biosdev, ((BVRef)(uint32_t)get_env(envgBIOSBootVolume))->part_no, ((BVRef)(uint32_t)get_env(envgBIOSBootVolume))->flags);
    getc();
#endif
    
    setBootGlobals(bvChain);
	
	// Display the GUI
	execute_hook("GUI_Display", NULL, NULL, NULL, NULL, NULL, NULL);
    
    // Parse args, load and start kernel.
    while (1) {
        const char *val;
        int len;
		char     *bootFile;
		bool trycache = true; // Always try to catch the kernelcache first
		
        long flags;
#ifdef BOOT_HELPER_SUPPORT
		long time;
#endif
        int ret = -1;
        void *binary = (void *)kLoadAddr;
        
        // additional variable for testing alternate kernel image locations on boot helper partitions.
        char     bootFileSpec[512];
		
        // Initialize globals.
        safe_set_env(envSysConfigValid, false);
		
        status = getBootOptions(firstRun);
        firstRun = false;
        if (status == -1) continue;
		
        status = processBootOptions();
#ifndef NO_MULTIBOOT_SUPPORT
        // Status==1 means to chainboot
        if ( status ==  1 ) break;
#endif
        // Status==-1 means that the config file couldn't be loaded or that gBootVolume is NULL
        if ( status == -1 )
        {
			// gBootVolume == NULL usually means the user hit escape.
			if(((BVRef)(uint32_t)get_env(envgBootVolume)) == NULL)
			{
				freeFilteredBVChain(bvChain);
                
				if (get_env(envgEnableCDROMRescan))
					rescanBIOSDevice((int)get_env(envgBIOSDev));
				
				bvChain = newFilteredBVChain(0x80, 0xFF, allowBVFlags, denyBVFlags, &devcnt);
                safe_set_env(envgDeviceCount,devcnt);
                
				setBootGlobals(bvChain);
			}
			continue;
        }
        
        // Other status (e.g. 0) means that we should proceed with boot.
		execute_hook("GUI_PreBoot", NULL, NULL, NULL, NULL, NULL, NULL);
        
		if (getValueForKey(karch, &val, &len, DEFAULT_BOOT_CONFIG) && val)
		{
			if (strncmp(val, "x86_64", 4) == 0)
			{
				safe_set_env(envarchCpuType, CPU_TYPE_X86_64);
                
			}
			else if (strncmp(val, "i386", 4) == 0)
			{
				safe_set_env(envarchCpuType, CPU_TYPE_I386);
                
			}
			else
			{
				DBG("Incorrect parameter for option 'arch =' , please use x86_64 or i386\n");
				determineCpuArch();
			}
            
		}
		else determineCpuArch();
        
        
		getRootDevice();
        
		// Notify to all modules that we are attempting to boot
		execute_hook("PreBoot", NULL, NULL, NULL, NULL, NULL, NULL);
		
		if (execute_hook("getProductNamePatched", NULL, NULL, NULL, NULL, NULL, NULL) != EFI_SUCCESS)
			readSMBIOS(thePlatformName); // read smbios Platform Name
		
		
		if (((get_env(envgBootMode) & kBootModeSafe) == 0) &&
			!get_env(envgOverrideKernel) &&
			(get_env(envgBootFileType) == kBlockDeviceType) &&
			(gMKextName[0] == '\0') &&
			!getValueForBootKey(bootArgs->CommandLine, kIgnorePrelinkKern, &val, &len))
		{
			getBoolForKey(kUseKernelCache, &trycache, DEFAULT_BOOT_CONFIG);
			if (trycache == true)
			{
				// try to find the cache and fill the gBootKernelCacheFile string
				getKernelCachePath();
				
				// Check for cache file
				trycache = (gBootKernelCacheFile[0] != '\0') ? true : false; // if gBootKernelCacheFile is filled et bla bla bla.... :-)
			}
			
		}
		else
		{
			trycache = false;
		}
        
		verbose("Loading Darwin %s\n", ((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion);
		{
			long cachetime, kerneltime = 0, exttime;
			if (trycache && !forcecache) do {
				
                if (strncmp(bootInfo->bootFile, kDefaultKernel,sizeof(kDefaultKernel)) != 0) {
                    // if we haven't found the kernel yet, don't use the cache
                    ret = GetFileInfo(NULL, bootInfo->bootFile, &flags, &kerneltime);
                    if ((ret != 0) || ((flags & kFileTypeMask) != kFileTypeFlat))
                    {
                        trycache = false;
                        safe_set_env(envAdler32, 0);
                        DBG("No kernel found, kernelcache disabled !!!\n");
                        break;
                    }
                }
                else if (((BVRef)(uint32_t)get_env(envgBootVolume))->kernelfound != true) // Should never happen.
                {
                    bootFile = kDefaultKernel;
                    goto out;
                }
                
				ret = GetFileInfo(NULL, gBootKernelCacheFile, &flags, &cachetime);
				if ((ret != 0) || ((flags & kFileTypeMask) != kFileTypeFlat)
					|| (cachetime < kerneltime))
				{
					trycache = false;
					safe_set_env(envAdler32, 0);
					DBG("Warning: No kernelcache found or kernelcache too old (timestamp of the kernel > timestamp of the cache), kernelcache disabled !!!\n");
                    
					break;
				}
				ret = GetFileInfo("/System/Library/", "Extensions", &flags, &exttime);
				if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeDirectory)
					&& (cachetime < exttime))
				{
					trycache = false;
					safe_set_env(envAdler32, 0);
					DBG("Warning: kernelcache too old, timestamp of S/L/E > timestamp of the cache, kernelcache disabled !!! \n");
                    
					break;
				}
				if (kerneltime > exttime)
				{
					exttime = kerneltime;
				}
				if (cachetime != (exttime + 1))
				{
					trycache = false;
					safe_set_env(envAdler32, 0);
					DBG("Warning: invalid timestamp, kernelcache disabled !!!\n");
                    
					break;
				}
			} while (0);
		}

        do {
            if (trycache == true || forcecache == true)
			{
                bootFile = gBootKernelCacheFile;
                verbose("Loading kernel cache %s\n", bootFile);
				if (((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion[3] > '6')
				{
					ret = LoadThinFatFile(bootFile, &binary);
					if ((ret <= 0) && (get_env(envarchCpuType) == CPU_TYPE_X86_64))
					{
						safe_set_env(envarchCpuType, CPU_TYPE_I386);
						ret = LoadThinFatFile(bootFile, &binary);
					}
				}
				else
				{
					ret = LoadFile(bootFile);
					binary = (void *)kLoadAddr;
				}
				
                if (ret >= 0)
				{
                    break;
                }
				
            }
			safe_set_env(envAdler32, 0);
            bootFile = bootInfo->bootFile;
#ifdef BOOT_HELPER_SUPPORT
			
            // Try to load kernel image from alternate locations on boot helper partitions.
            snprintf(bootFileSpec, sizeof(bootFileSpec),"com.apple.boot.P/%s", bootFile);
            ret = GetFileInfo(NULL, bootFileSpec, &flags, &time);
  	  	    if (ret == -1)
  	  	    {
				snprintf(bootFileSpec, sizeof(bootFileSpec), "com.apple.boot.R/%s", bootFile);
				ret = GetFileInfo(NULL, bootFileSpec, &flags, &time);
				if (ret == -1)
				{
					snprintf(bootFileSpec, sizeof(bootFileSpec), "com.apple.boot.S/%s", bootFile);
					ret = GetFileInfo(NULL, bootFileSpec, &flags, &time);
					if (ret == -1)
					{
						// Not found any alternate locations, using the original kernel image path.
						strlcpy(bootFileSpec, bootFile,sizeof(bootFileSpec));
					}
				}
            }
#else
			strlcpy(bootFileSpec, bootFile,sizeof(bootFileSpec));
#endif
			
            verbose("Loading kernel %s\n", bootFileSpec);
            ret = LoadThinFatFile(bootFileSpec, &binary);
            if ((ret <= 0) && (get_env(envarchCpuType) == CPU_TYPE_X86_64))
            {
				safe_set_env(envarchCpuType, CPU_TYPE_I386);
				ret = LoadThinFatFile(bootFileSpec, &binary);
            }
			
        } while (0);
		
#if DEBUG
        printf("Pausing...");
        sleep(8);
#endif
		
        if (ret <= 0)
		{
        out:
			printf("Can't find %s\n", bootFile);
			
			sleep(1);
#ifdef NBP_SUPPORT
            if (get_env(envgBootFileType) == kNetworkDeviceType)
			{
                // Return control back to PXE. Don't unload PXE base code.
                gUnloadPXEOnExit = false;
                break;
            }
#endif
        }
		else
		{
            /* Won't return if successful. */
            if ( ExecKernel(binary))
            {
                firstRun = true;
                continue;
            }
        }
    }
	
#ifndef NO_MULTIBOOT_SUPPORT
    // chainboot
    if (status==1)
	{
		if (getVideoMode() == GRAPHICS_MODE)
		{	// if we are already in graphics-mode,
            
			__setVideoMode(VGA_TEXT_MODE);	// switch back to text mode
            
		}
    }
#else
	printf("No proper Darwin Partition found, reseting ... \n");
	pause();
	common_boot(biosdev);
#endif
	
#ifdef NBP_SUPPORT
    if ((get_env(envgBootFileType) == kNetworkDeviceType) && gUnloadPXEOnExit)
	{
		nbpUnloadBaseCode();
    }
#endif
}

static void determineCpuArch(void)
{
	if (cpu_mode_is64bit())
	{
		safe_set_env(envarchCpuType, CPU_TYPE_X86_64);
        
	}
	else
	{
		safe_set_env(envarchCpuType, CPU_TYPE_I386);
	}
}

void getKernelCachePath(void)
{
	{
		// If there is an extra kext/mkext, we return immediatly and we skip the kernelCache
		// since kexts/mkexts are not loaded properly when the kernelCache is used.
		// Another method would be to re-build the kernelCache one the fly
		if (found_extra_kext() == true) return;
	}
	
	{
		const char    *val;
		int            len;
		unsigned long    Adler32 = 0;
		
		if (getValueForKey(kKernelCacheKey, &val, &len, DEFAULT_BOOT_CONFIG))
		{
            char * buffer = newString(val);
            
			if (val[0] == '\\')
			{
				// Flip the back slash's to slash's .
                len = 0;
                while (buffer[len] != '\0') {
                    if (buffer[len] == '\\')
                    {
                        buffer[len] = '/';
                    }
                    len++;
                }
			}
			strlcpy(gBootKernelCacheFile, buffer, sizeof(gBootKernelCacheFile));
            forcecache = true;
		}
		else
		{
			if(((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion[3] > '6')
			{
				snprintf(gBootKernelCacheFile, sizeof(gBootKernelCacheFile), "%s", kDefaultCachePath);
			}
			else if(((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion[3] <= '6')
			{
				
				PlatformInfo    *platformInfo = malloc(sizeof(PlatformInfo));
				if (platformInfo)
				{
					
					bzero(platformInfo, sizeof(PlatformInfo));
					
					if (GetgPlatformName())
						strlcpy(platformInfo->platformName,GetgPlatformName(), sizeof(platformInfo->platformName)+1);
					
					if (GetgRootDevice())
					{
						char *rootPath_p = platformInfo->rootPath;
						len = strlen(GetgRootDevice()) + 1;
						if ((unsigned)len > sizeof(platformInfo->rootPath))
						{
							len = sizeof(platformInfo->rootPath);
						}
						memcpy(rootPath_p, GetgRootDevice(),len);
						
						rootPath_p += len;
						
						len = strlen(bootInfo->bootFile);
						
						if ((unsigned)(rootPath_p - platformInfo->rootPath + len) >=
							sizeof(platformInfo->rootPath))
						{
							
							len = sizeof(platformInfo->rootPath) -
							(rootPath_p - platformInfo->rootPath);
						}
						memcpy(rootPath_p, bootInfo->bootFile, len);
						
					}
					
					if (!platformInfo->platformName[0] || !platformInfo->rootPath[0])
					{
						platformInfo->platformName[0] = platformInfo->rootPath[0] = 0;
					}
#ifdef rootpath
                    SetgRootPath(platformInfo->rootPath);
#endif
					
					Adler32 = OSSwapHostToBigInt32(adler32((unsigned char *)platformInfo, sizeof(PlatformInfo)));
					safe_set_env(envAdler32, Adler32);
					
					free(platformInfo);
				}
				
				DBG("Adler32: %08lX\n",Adler32);
				
				if (((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion[3] < '6')
				{
					long flags, cachetime;
					int ret = -1;
					
					if (Adler32) {
						snprintf(gBootKernelCacheFile, sizeof(gBootKernelCacheFile), "%s.%08lX", "/System/Library/Caches/com.apple.kernelcaches/kernelcache",Adler32);
						ret = GetFileInfo(NULL, gBootKernelCacheFile, &flags, &cachetime);
					}
					
					if ((ret != 0) || ((flags & kFileTypeMask) != kFileTypeFlat))
					{
						safe_set_env(envAdler32, 0);
						snprintf(gBootKernelCacheFile, sizeof(gBootKernelCacheFile), "%s", "/System/Library/Caches/com.apple.kernelcaches/kernelcache");
					}
					
				} else if (Adler32)
					snprintf(gBootKernelCacheFile, sizeof(gBootKernelCacheFile), "%s_%s.%08lX", kDefaultCachePath, (get_env(envarchCpuType) == CPU_TYPE_I386) ? "i386" : "x86_64", Adler32); //Snow Leopard
				
			}
		}
	}
}

static void getRootDevice(void)
{
	// Maximum config table value size
#define VALUE_SIZE 2048
	bool uuidSet = false;
	const char *val = 0;
    int cnt = 0;
	
	if (getValueForKey(kBootUUIDKey, &val, &cnt, DEFAULT_BOOT_CONFIG))
	{
		uuidSet = true;
	}
	else
	{
		if (getValueForBootKey(bootArgs->CommandLine, kRootDeviceKey, &val, &cnt))
		{
			if (*val == '*' && *(val + 1) != '/' && *(val + 1) != 'u')
			{
				val += 1; //skip the *
				uuidSet = true;
				
			}
			else if (*val == '*' && *(val + 1) == 'u')
			{
				
				if ( getValueForKey( kBootDeviceKey, &val, &cnt, DEFAULT_BOOT_CONFIG))
					uuidSet = true;
				
			}
		}
		else
		{
#ifdef BOOT_HELPER_SUPPORT
			//
			// Try an alternate method for getting the root UUID on boot helper partitions.
			//
			if (((BVRef)(uint32_t)get_env(envgBootVolume))->flags & kBVFlagBooter)
			{
				if((loadHelperConfig() == 0)
				   && getValueForKey(kHelperRootUUIDKey, &val, &cnt, &bootInfo->helperConfig) )
				{
					getValueForKey(kHelperRootUUIDKey, &val, &cnt, &bootInfo->helperConfig);
					uuidSet = true;
					goto out;
				}
			}
#endif
			if ( getValueForKey( kBootDeviceKey, &val, &cnt, DEFAULT_BOOT_CONFIG))
			{
				int ArgCntRemaining = (int)get_env(envArgCntRemaining);
				uuidSet = false;
				char *           valueBuffer;
				valueBuffer = malloc(VALUE_SIZE);
                if (!valueBuffer) {
                    return;
                }
				char *           argP = bootArgs->CommandLine;
				valueBuffer[0] = '*';
				if (cnt > VALUE_SIZE)
				{
					cnt = VALUE_SIZE;
				}
				strlcpy(valueBuffer + 1, val, cnt+1);
				if (!copyArgument( kRootDeviceKey, valueBuffer, cnt, &argP, &ArgCntRemaining))
				{
					free(valueBuffer);
					printf("Error: boot arguments too long, unable to set root device !!");
					getc();
					return;
				}
                safe_set_env(envArgCntRemaining,ArgCntRemaining);
				free(valueBuffer);
				goto out;
			}
			
			if (((BVRef)(uint32_t)get_env(envgBootVolume))->fs_getuuid && (((BVRef)(uint32_t)get_env(envgBootVolume))->fs_getuuid (((BVRef)(uint32_t)get_env(envgBootVolume)), bootInfo->uuidStr, sizeof(bootInfo->uuidStr)) == 0))
			{
				verbose("Setting boot-uuid to: %s\n", bootInfo->uuidStr);
				//uuidSet = true;
				SetgRootDevice(bootInfo->uuidStr);
				return;
			}
            
		}
	}
    
out:
	verbose("Setting %s to: %s\n", uuidSet ? kBootUUIDKey : "root device", (char* )val);
    SetgRootDevice(val);
}

static bool find_file_with_ext(const char* dir, const char *ext, const char * name_to_compare, size_t ext_size)
{
    long         ret, length, flags, time;
    long long	 index;
    const char * name;
    
	DBG("FileLoadBundles in %s\n",dirSpec);
    
    index = 0;
    while (1) {
        ret = GetDirEntry(dir, &index, &name, &flags, &time);
        if (ret == -1) break;
		
        // Make sure this is not a directory.
        if ((flags & kFileTypeMask) != kFileTypeFlat) continue;
        
        // Make sure this is a kext or mkext.
        length = strlen(name);
        if (strncmp(name + length - ext_size, ext, ext_size)) continue;
		
        if (name_to_compare)
        {
            if (strcmp(name, name_to_compare) == 0)
            {
                DBG("found : %s\n", name);
                return true;
            }
        }
        else
        {
            DBG("found : %s\n", name);
            return true;
        }
		
    }
	return false;
}

// If a kext is found in /Extra/Extentions return true
// If a mkext is found in /Extra return true
// Otherwise return false
// Tips (if you still want to use extra kext(s)/mkext(s) ):
// With Lion and earlier, the default boot cache is a kernelcache, no mkext is created anymore by kextd automaticaly (kextd still update the existing mkexts),
// so it's recommended to create a system mkext by yourself to decrease boot time (see the kextcache commandline)
static bool found_extra_kext(void)
{
#define EXTENSIONS "Extensions"
#define MKEXT_EXT  ".mkext"
#define MKEXT_EXT_SIZE strlen(MKEXT_EXT)
#define KEXT_EXT  ".kext"
#define KEXT_EXT_SIZE strlen(KEXT_EXT)
	
	long flags;
	long exttime;
	int ret = -1;
	
	ret = GetFileInfo("rd(0,0)/Extra/", EXTENSIONS, &flags, &exttime);
	if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeFlat))
	{
		if (((flags & kFileTypeMask) == kFileTypeFlat))
		{
			if (find_file_with_ext("rd(0,0)/Extra/", MKEXT_EXT, EXTENSIONS, MKEXT_EXT_SIZE))
			{
				return true;
			}
		}
		else if (((flags & kFileTypeMask) == kFileTypeDirectory))
		{
			if (find_file_with_ext("rd(0,0)/Extra/Extensions/", KEXT_EXT, NULL, KEXT_EXT_SIZE))
			{
				return true;
			}
		}
	}
	ret = GetFileInfo("/Extra/", EXTENSIONS, &flags, &exttime);
	if (ret == 0)
	{
		if (((flags & kFileTypeMask) == kFileTypeFlat))
		{
			if (find_file_with_ext("/Extra/", MKEXT_EXT, EXTENSIONS, MKEXT_EXT_SIZE))
			{
				return true;
			}
		}
		else if (((flags & kFileTypeMask) == kFileTypeDirectory))
		{
			if (find_file_with_ext("/Extra/Extensions/", KEXT_EXT, NULL, KEXT_EXT_SIZE))
			{
				return true;
			}
		}
	}
	ret = GetFileInfo("bt(0,0)/Extra/", EXTENSIONS, &flags, &exttime);
	if (ret == 0)
	{
		if (((flags & kFileTypeMask) == kFileTypeFlat))
		{
			if (find_file_with_ext("bt(0,0)/Extra/", MKEXT_EXT, EXTENSIONS, MKEXT_EXT_SIZE))
			{
				return true;
			}
		}
		else if (((flags & kFileTypeMask) == kFileTypeDirectory))
		{
			if (find_file_with_ext("bt(0,0)/Extra/Extensions/", KEXT_EXT, NULL, KEXT_EXT_SIZE))
			{
				return true;
			}
		}
	}
	DBG("NO Extra Mkext/Kext found\n");
	
	// nothing found
	return false;
}

#if 0
static char *FIXED_BOOTFILE_PATH(char * str)
{
	char bootfile[128];
	
	bool bootFileWithDevice = false;
	// Check if bootFile start with a device ex: bt(0,0)/Extra/mach_kernel
	if (strncmp(str,"bt(",3) == 0 ||
		strncmp(str,"hd(",3) == 0 ||
		strncmp(str,"rd(",3) == 0)
	{
		bootFileWithDevice = true;
	}
    
	// bootFile must start with a / if it not start with a device name
	if (!bootFileWithDevice && (str)[0] != '/')
		snprintf(bootFile, sizeof(bootfile), "/%s", str); // append a leading /
	else
		strlcpy(bootFile, bootInfo->bootFile, sizeof(bootFile));
	
	return bootfile;
}
#endif
