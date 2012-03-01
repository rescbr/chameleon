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
#ifndef OPTION_ROM
#include "appleboot.h"
#endif
#include "modules.h"
#include "xml.h"
#include "options.h"

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

static char gBootKernelCacheFile[Cache_len_name];
static char gMKextName[Cache_len_name];

static BVRef   bvChain;
static bool forcecache = false;

static void zeroBSS(void);
#ifdef SAFE_MALLOC
static inline void malloc_error(char *addr, size_t size, const char *file, int line);
#else
static inline void malloc_error(char *addr, size_t size);
#endif
static int ExecKernel(void *binary);
static void getRootDevice();
#ifdef NBP_SUPPORT
static bool gUnloadPXEOnExit = false;
#endif
static bool find_file_with_ext(const char* dir, const char *ext, const char * name_compare, size_t ext_size);
static bool found_extra_kext(void);
static void determineCpuArch(void);
static void init_pic(void);
void getKernelCachePath(void);


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
	extern char _DATA__bss__begin, _DATA__bss__end;
	extern char _DATA__common__begin, _DATA__common__end;
	
	bzero(&_DATA__bss__begin, (&_DATA__bss__end - &_DATA__bss__begin));
	bzero(&_DATA__common__begin, (&_DATA__common__end - &_DATA__common__begin));
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

#if 0
static inline void exception_error(char *msg, int nb)
{
    printf("\r\nException number = %d\r\nmessage = %s\r\n\r\n", nb, msg);
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
#if 0
	exception_init(exception_error);
#endif
}

static void init_pic(void)
{
    /* Remap IRQ's */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    outb(0x70,0x80); /* Disable NMI */
    
	outb(0x21, 0xff);   /* Maskout all interrupts Pic1 */
	outb(0xa1, 0xff);   /* Maskout all interrupts Pic2 */
}

//==========================================================================
// execKernel - Load the kernel image (mach-o) and jump to its entry point.

static int ExecKernel(void *binary)
{
    entry_t                   kernelEntry;
    int                       ret;
				
    bootArgs->kaddr = bootArgs->ksize = 0;
	
	if(gBootVolume->OSVersion[3] <= '6')
	{
		bootArgs->Header.Version  = kBootArgsVersion1;		
		bootArgs->Header.Revision = gBootVolume->OSVersion[3];	
	}
	else 
	{		
#if kBootArgsVersion > 1
		
		bootArgs->Header.Version  = kBootArgsVersion;		
		bootArgs->Header.Revision = kBootArgsRevision;
#else
		if(gBootVolume->OSVersion[3] >= '7')
		{
			bootArgs->Header.Version  = 2;
			bootArgs->Header.Revision = 0;
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
	
    // Load boot drivers from the specifed root path.
	
    if (!gHaveKernelCache)
	{
		LoadDrivers("/");
    }

    if (gErrors)
	{
        printf("Errors encountered while starting up the computer.\n");
#if DEBUG_BOOT
        printf("Pausing %d seconds...\n", kBootErrorTimeout);
        sleep(kBootErrorTimeout);
#endif
    }
	
	execute_hook("md0Ramdisk", NULL, NULL, NULL, NULL, NULL, NULL);

    setupFakeEfi();
		
    verbose("Starting Darwin %s\n",( archCpuType == CPU_TYPE_I386 ) ? "x86" : "x86_64");
#ifdef NBP_SUPPORT
    // Cleanup the PXE base code.
	
    if ( (gBootFileType == kNetworkDeviceType) && gUnloadPXEOnExit )
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
			
			if (strval && ((strcmp(strval, "no") == 0) || (strcmp(strval, "No") == 0)))
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
#if UNUSED
		__setVideoMode( GRAPHICS_MODE, 0 );
#else
		__setVideoMode( GRAPHICS_MODE );
#endif
		
#ifndef OPTION_ROM
        
		if(!gVerboseMode)
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
				__convertImage(bootImageWidth, bootImageHeight, appleBootPict, &bootImageData); 
				if (bootImageData)
				{	
					x = (screen_params[0] - MIN(kAppleBootWidth, screen_params[0])) / 2; 
					y = (screen_params[1] - MIN(kAppleBootHeight, screen_params[1])) / 2; 
					__drawDataRectangle(x, y, kAppleBootWidth, kAppleBootHeight, bootImageData);
					free(bootImageData);
				}
				free(appleBootPict); 
			}
            
		}
#endif
	}

    finalizeEFIConfigTable();
    
	setupBooterLog();
	
    finalizeBootStruct(); 
    
	execute_hook("Kernel Start", (void*)kernelEntry, (void*)bootArgs, NULL, NULL, NULL, NULL);	// Notify modules that the kernel is about to be started
	
    switch (gBootVolume->OSVersion[3]) {
		case '4':
		case '5':
		case '6':
			reserveKernLegacyBootStruct();
			break;		
		case '7':
			reserveKern107BootStruct();
			break;
		case '8':
			reserveKern108BootStruct();
			break;
		default:
			break;
	}	
	
#if UNUSED
	turnOffFloppy();
#endif
#if BETA
#include "smp-imps.h"
#include "apic.h"
	IMPS_LAPIC_WRITE(LAPIC_LVT1, LAPIC_ICR_DM_NMI);
#endif

	switch (gBootVolume->OSVersion[3]) {
		case '4':						
		case '5':
		case '6':
			// Jump to kernel's entry point. There's no going back now. XXX LEGACY OS XXX
			startprog( kernelEntry, bootArgsLegacy );
			break;		
		case '7':
			init_pic();
			// Jump to kernel's entry point. There's no going back now.  XXX LION XXX
			startprog( kernelEntry, bootArgs107 );
			break;
		case '8':
			init_pic();
			// Jump to kernel's entry point. There's no going back now.  XXX MOUNTAIN LION XXX
			startprog( kernelEntry, bootArgs108 );
			break;
		default:
			printf("Error: Unsupported Darwin version\n");
			getc();
			break;
	}	

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

    
    unsigned int allowBVFlags = kBVFlagSystemVolume|kBVFlagForeignBoot;
    unsigned int denyBVFlags = kBVFlagEFISystem;
	
#ifdef NBP_SUPPORT
    // Set reminder to unload the PXE base code. Neglect to unload
    // the base code will result in a hang or kernel panic.
    gUnloadPXEOnExit = true;
#endif
    // Record the device that the booter was loaded from.
    safe_set_env(envgBIOSDev,biosdev & kBIOSDevMask);
	
	
    // Setup VGA text mode.
    // Not sure if it is safe to call setVideoMode() before the
    // config table has been loaded. Call video_mode() instead.
#if DEBUG
    printf("before video_mode\n");
#endif
    video_mode( 2 );  // 80x25 mono text mode.
#if DEBUG
    printf("after video_mode\n");
#endif
	printf("Starting Chameleon ...\n");
    
	init_ut_fnc();
    
    arc4_init();

	initBooterLog();    
    
	// Initialize boot info structure.
    initKernBootStruct();
	
	
    // Scan and record the system's hardware information.
    scan_platform();
	
    safe_set_env(envgBootMode, kBootModeNormal); /* defaults to 0 == kBootModeNormal */
    safe_set_env(envShouldboot, false);
    safe_set_env(envkCache, (uint32_t)gBootKernelCacheFile);
    safe_set_env(envMKextName, (uint32_t)gMKextName);
    InitBootPrompt();
    
    // First get info for boot volume.
    scanBootVolumes((int)get_env(envgBIOSDev), 0);
    bvChain = getBVChainForBIOSDev((int)get_env(envgBIOSDev));
    setBootGlobals(bvChain);
	
    // Load Booter boot.plist config file
    status = loadBooterConfig();
	
    {
        bool isServer = get_env(envIsServer);    
        getBoolForKey(kIsServer, &isServer, DEFAULT_BOOT_CONFIG); // set this as soon as possible    
        safe_set_env(envIsServer , isServer);
    }
	

	{
		bool     quiet = false;
		if (getBoolForKey(kQuietBootKey, &quiet, DEFAULT_BOOT_CONFIG) && quiet)
		{
            long gBootMode = kBootModeNormal;
			gBootMode |= kBootModeQuiet;
            safe_set_env(envgBootMode, gBootMode);
		}
	}	
	
	{
		bool     instantMenu = false;
		// Override firstRun to get to the boot menu instantly by setting "Instant Menu"=y in system config
		if (getBoolForKey(kInsantMenuKey, &instantMenu, DEFAULT_BOOT_CONFIG) && instantMenu)
		{
			firstRun = false;
		}
	}	

    {	
#ifndef OPTION_ROM
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
            scanBootVolumes((int)get_env(envgBIOSDev), &bvCount);
        } 
        else 
#endif
        {
#if UNUSED
            scanDisks((int)get_env(envgBIOSDev), &bvCount);
#else
            scanDisks();
#endif
        }    
    
	}
    
    // Create a separated bvr chain using the specified filters.
    bvChain = newFilteredBVChain(0x80, 0xFF, allowBVFlags, denyBVFlags, &devcnt);
    safe_set_env(envgDeviceCount,devcnt);
       
    
	gBootVolume = selectBootVolume(bvChain);
	
	{
		// Intialize module system
		EFI_STATUS sysinit = init_module_system();
		if((sysinit == EFI_SUCCESS) || (sysinit == EFI_ALREADY_STARTED) /*should never happen*/ ) 
		{
			load_all_modules();
		}
	}	
	
    load_all_internal_modules();
    
    // Loading preboot ramdisk if exists.
	execute_hook("loadPrebootRAMDisk", NULL, NULL, NULL, NULL, NULL, NULL);		

#ifndef OPTION_ROM

    
	
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
    
#endif	
	
#if DEBUG
    printf(" Default: %d, ->biosdev: %d, ->part_no: %d ->flags: %d\n", gBootVolume, gBootVolume->biosdev, gBootVolume->part_no, gBootVolume->flags);
    printf(" bt(0,0): %d, ->biosdev: %d, ->part_no: %d ->flags: %d\n", gBIOSBootVolume, gBIOSBootVolume->biosdev, gBIOSBootVolume->part_no, gBIOSBootVolume->flags);
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
        gErrors        = false;
		
        status = getBootOptions(firstRun);
        firstRun = false;
        if (status == -1) continue;
		
        status = processBootOptions();
        // Status==1 means to chainboot
        if ( status ==  1 ) break;
        // Status==-1 means that the config file couldn't be loaded or that gBootVolume is NULL
        if ( status == -1 )
        {
			// gBootVolume == NULL usually means the user hit escape.
			if(gBootVolume == NULL)
			{
				freeFilteredBVChain(bvChain);
#ifndef OPTION_ROM
				if (get_env(envgEnableCDROMRescan))
					rescanBIOSDevice((int)get_env(envgBIOSDev));
#endif
				
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
				archCpuType = CPU_TYPE_X86_64;
			} 
			else if (strncmp(val, "i386", 4) == 0)
			{
				archCpuType = CPU_TYPE_I386;
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
			(gBootFileType == kBlockDeviceType) &&
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
						
		verbose("Loading Darwin %s\n", gBootVolume->OSVersion);
		{
			long cachetime, kerneltime, exttime;
			if (trycache && !forcecache) do {
				
                if (strcmp(bootInfo->bootFile, kDefaultKernel) != 0) {
                    // if we haven't found the kernel yet, don't use the cache
                    ret = GetFileInfo(NULL, bootInfo->bootFile, &flags, &kerneltime);
                    if ((ret != 0) || ((flags & kFileTypeMask) != kFileTypeFlat))
                    {
                        trycache = 0;
                        bootInfo->adler32  = 0;
                        DBG("No kernel found, kernelcache disabled !!!\n");
                        break;
                    }
                }
                else if (gBootVolume->kernelfound != true) // Should never happen.
                {
                    bootFile = kDefaultKernel;
                    goto out;
                } 
                    
				ret = GetFileInfo(NULL, gBootKernelCacheFile, &flags, &cachetime);
				if ((ret != 0) || ((flags & kFileTypeMask) != kFileTypeFlat)
					|| (cachetime < kerneltime))
				{
					trycache = 0;
					bootInfo->adler32  = 0;
					DBG("Warning: No kernelcache found or kernelcache too old (timestamp of the kernel > timestamp of the cache), kernelcache disabled !!!\n");

					break;				                
				} 
				ret = GetFileInfo("/System/Library/", "Extensions", &flags, &exttime);
				if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeDirectory)
					&& (cachetime < exttime))
				{
					trycache = 0;
					bootInfo->adler32  = 0;
					DBG("Warning: kernelcache too old, timestamp of S/L/E > timestamp of the cache, kernelcache disabled !!! \n");

					break;
				}
				if (kerneltime > exttime)
				{
					exttime = kerneltime;
				}
				if (cachetime != (exttime + 1))
				{
					trycache = 0;
					bootInfo->adler32  = 0;
					DBG("Warning: invalid timestamp, kernelcache disabled !!!\n");

					break;
				}
			} while (0);
		}		

        do {
            if (trycache)
			{
                bootFile = gBootKernelCacheFile;
                verbose("Loading kernel cache %s\n", bootFile);
				if (gBootVolume->OSVersion[3] > '6')
				{					
					ret = LoadThinFatFile(bootFile, &binary);
					if (ret <= 0 && archCpuType == CPU_TYPE_X86_64)
					{
						archCpuType = CPU_TYPE_I386;
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
			bootInfo->adler32  = 0;
            bootFile = bootInfo->bootFile;
#ifdef BOOT_HELPER_SUPPORT
			
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
            if (ret <= 0 && archCpuType == CPU_TYPE_X86_64)
            {
				archCpuType = CPU_TYPE_I386;
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
            if (gBootFileType == kNetworkDeviceType)
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
            ret = ExecKernel(binary);
        }
    }
    
    // chainboot
    if (status==1)
	{
		if (__getVideoMode() == GRAPHICS_MODE)
		{	// if we are already in graphics-mode,
#if UNUSED
			__setVideoMode(VGA_TEXT_MODE, 0);	// switch back to text mode
#else
			__setVideoMode(VGA_TEXT_MODE);	// switch back to text mode
#endif
		}
    }
#ifdef NBP_SUPPORT
    if ((gBootFileType == kNetworkDeviceType) && gUnloadPXEOnExit)
	{
		nbpUnloadBaseCode();
    }
#endif
}

static void determineCpuArch(void)
{
	if (cpu_mode_is64bit())
	{
		archCpuType = CPU_TYPE_X86_64;
	}
	else
	{
		archCpuType = CPU_TYPE_I386;
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
			if(gBootVolume->OSVersion[3] > '6')
			{					
				sprintf(gBootKernelCacheFile, "%s", kDefaultCachePath);
			}
			else if(gBootVolume->OSVersion[3] <= '6')
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
					
					bootInfo->adler32 = OSSwapHostToBigInt32(adler32((unsigned char *)platformInfo, sizeof(*platformInfo)));
					
					free(platformInfo);	
				}
				
				DBG("Adler32: %08lX\n",bootInfo->adler32);
				
				if (gBootVolume->OSVersion[3] < '6')
				{
					long flags, cachetime;
					int ret = -1;
					sprintf(gBootKernelCacheFile, "%s.%08lX", "/System/Library/Caches/com.apple.kernelcaches/kernelcache",bootInfo->adler32);
					ret = GetFileInfo(NULL, gBootKernelCacheFile, &flags, &cachetime);
					if ((ret != 0) || ((flags & kFileTypeMask) != kFileTypeFlat))
					{
						bootInfo->adler32 = 0;
						sprintf(gBootKernelCacheFile, "%s", "/System/Library/Caches/com.apple.kernelcaches/kernelcache"); 
					}
					
				} else
					sprintf(gBootKernelCacheFile, "%s_%s.%08lX", kDefaultCachePath, (archCpuType == CPU_TYPE_I386) ? "i386" : "x86_64", bootInfo->adler32); //Snow Leopard
				
			}	
		}
	}	
}

static void getRootDevice()
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
			if (gBootVolume->flags & kBVFlagBooter)
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
			
			if (gBootVolume->fs_getuuid && gBootVolume->fs_getuuid (gBootVolume, bootInfo->uuidStr) == 0)
			{
				verbose("Setting boot-uuid to: %s\n", bootInfo->uuidStr);
				uuidSet = true;                
				SetgRootDevice(bootInfo->uuidStr);
				return;
			}
		
		}	
	}
		
out:	
	verbose("Setting %s to: %s\n", uuidSet ? kBootUUIDKey : "root device", (char* )val); 
    SetgRootDevice(val);
}

static bool find_file_with_ext(const char* dir, const char *ext, const char * name_compare, size_t ext_size)
{
	char* name;
	long flags;
	long time;
	struct dirstuff* moduleDir = opendir(dir);
	while(readdir(moduleDir, (const char**)&name, &flags, &time) >= 0)
	{		
		int len = strlen(name);
		
		if (len >= ext_size)
		{
			if(strcmp(&name[len - ext_size], ext) == 0)
			{	
				if (name_compare)
				{
					if (strcmp(name, name_compare) == 0)
					{
						DBG("found : %s\n", name);
						closedir(moduleDir);
						return true;
					}
				}
				else
				{
					DBG("found : %s\n", name);
					closedir(moduleDir);
					return true;
				}			
			}
#if DEBUG_BOOT
			else 
			{
				DBG("Ignoring %s\n", name);
			}
#endif
		}		
#if DEBUG_BOOT
		else 
		{
			DBG("Ignoring %s\n", name);
		}
#endif		
	}
	
	if (moduleDir) {
		closedir(moduleDir);
		
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
#define MKEXT_EXT_SIZE sizeof("mkext")
#define KEXT_EXT  ".kext"	
#define KEXT_EXT_SIZE sizeof("kext")
	
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
		sprintf(bootFile, "/%s", str); // append a leading /
	else
		strlcpy(bootFile, bootInfo->bootFile, sizeof(bootInfo->bootFile));
	
	return bootfile;
}
#endif
