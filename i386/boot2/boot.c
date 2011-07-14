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


char *gboardproduct = NULL;
char *gPlatformName = NULL;
//char gRootPath[256];
long gBootMode; /* defaults to 0 == kBootModeNormal */
bool gOverrideKernel;
char gBootKernelCacheFile[512];
char gMKextName[512];
char gMacOSVersion[8];
char *gRootDevice = NULL;
bool uuidSet = false;
#ifndef OPTION_ROM
bool gEnableCDROMRescan;
bool gScanSingleDrive;
#endif

int     bvCount = 0;
//int	menucount = 0;
int     gDeviceCount = 0; 

BVRef   bvr;
BVRef   menuBVR;
BVRef   bvChain;

//static void selectBiosDevice(void);

static bool getOSVersion(char *str);
static void getRootDevice();
void getKernelCachePath();

#ifdef NBP_SUPPORT
static bool gUnloadPXEOnExit = false;
#endif

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
	
	if(gMacOSVersion[3] <= '6')
	{
		bootArgs->Version  = kBootArgsVersion1;		
		bootArgs->Revision = gMacOSVersion[3];	
	}
	else 
	{		
#if kBootArgsVersion > 1
		
		bootArgs->Version  = kBootArgsVersion;		
		bootArgs->Revision = kBootArgsRevision;
#else
		bootArgs->Version  = 2;
        bootArgs->Revision = 0;
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
	if(gMacOSVersion[3] == '7')
    reserveKernBootStruct();
	
    // Load boot drivers from the specifed root path.
	
    if (!gHaveKernelCache) {
		LoadDrivers("/");
    }
	
#if TEXT_SPINNER
	clearActivityIndicator();
#endif
	
    if (gErrors) {
        printf("Errors encountered while starting up the computer.\n");
        printf("Pausing %d seconds...\n", kBootErrorTimeout);
        sleep(kBootErrorTimeout);
    }
	
	execute_hook("md0Ramdisk", NULL, NULL, NULL, NULL, NULL, NULL);

    setupFakeEfi();
		
    verbose("Starting Darwin %s\n",( archCpuType == CPU_TYPE_I386 ) ? "x86" : "x86_64");
#ifdef NBP_SUPPORT
    // Cleanup the PXE base code.
	
    if ( (gBootFileType == kNetworkDeviceType) && gUnloadPXEOnExit ) {
		if ( (ret = nbpUnloadBaseCode()) != nbpStatusSuccess )
        {
        	printf("nbpUnloadBaseCode error %d\n", (int) ret);
            sleep(2);
        }
    }
#endif 
    bool wait = false;
	const char *strval = 0;
	int dummysize /*= 0*/;	
	
	getBoolForKey(kWaitForKeypressKey, &wait, &bootInfo->bootConfig);
	
	if (getValueForBootKey(bootArgs->CommandLine, "-wait", &strval, &dummysize))
	{
		wait = true;
		
		if (strval && ((strcmp(strval, "no") == 0) || (strcmp(strval, "No") == 0))) {
			wait = false;
		} 		
	}
		
	if (wait == true) {
		pause();
	}
			
	if ((execute_hook("GUI_ExecKernel", NULL, NULL, NULL, NULL, NULL, NULL) != EFI_SUCCESS)) // (bootArgs->Video.v_display == VGA_TEXT_MODE)	
	{
#if UNUSED
		setVideoMode( GRAPHICS_MODE, 0 );
#else
		setVideoMode( GRAPHICS_MODE );
#endif
		
#ifndef OPTION_ROM
		 
		if(!gVerboseMode)
		{			
			drawColorRectangle(0, 0, DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, 0x01); 
		
			uint8_t *appleBootPict; 
			uint16_t bootImageWidth = kAppleBootWidth; 
			uint16_t bootImageHeight = kAppleBootHeight; 
			uint8_t *bootImageData = NULL; 		 
			uint16_t x, y;
			
			unsigned long screen_params[4] = {DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, 32, 0};	// here we store the used screen resolution
			// Prepare the data for the default Apple boot image. 
			appleBootPict = (uint8_t *) decodeRLE(gAppleBootPictRLE, kAppleBootRLEBlocks, bootImageWidth * bootImageHeight); 
			if (appleBootPict) { 
				convertImage(bootImageWidth, bootImageHeight, appleBootPict, &bootImageData); 
				if (bootImageData) {	
					x = (screen_params[0] - MIN(kAppleBootWidth, screen_params[0])) / 2; 
					y = (screen_params[1] - MIN(kAppleBootHeight, screen_params[1])) / 2; 
					drawDataRectangle(x, y, kAppleBootWidth, kAppleBootHeight, bootImageData);
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
    
	
	if (gMacOSVersion[3] <= '6') 
		reserveKernLegacyBootStruct();   		
	
	execute_hook("Kernel Start", (void*)kernelEntry, (void*)bootArgs, NULL, NULL, NULL, NULL);	// Notify modules that the kernel is about to be started
#if UNUSED
	turnOffFloppy();
#endif
#if BETA
#include "smp.h"
#include "apic.h"
	IMPS_LAPIC_WRITE(LAPIC_LVT1, LAPIC_ICR_DM_NMI);
#endif
	
	if (gMacOSVersion[3] <= '6') {		
		
		// Jump to kernel's entry point. There's no going back now. XXX LEGACY OS XXX
		startprog( kernelEntry, bootArgsLegacy );
	}	
			
	outb(0x21, 0xff);   /* Maskout all interrupts Pic1 */
	outb(0xa1, 0xff);   /* Maskout all interrupts Pic2 */
		
	// Jump to kernel's entry point. There's no going back now. XXX LION XXX
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
    bool     quiet;
    bool     firstRun = true;
    bool     instantMenu;
#ifndef OPTION_ROM
    bool     rescanPrompt;
#endif
    unsigned int allowBVFlags = kBVFlagSystemVolume|kBVFlagForeignBoot;
    unsigned int denyBVFlags = kBVFlagEFISystem;
	
#ifdef NBP_SUPPORT
    // Set reminder to unload the PXE base code. Neglect to unload
    // the base code will result in a hang or kernel panic.
    gUnloadPXEOnExit = true;
#endif
    // Record the device that the booter was loaded from.
    gBIOSDev = biosdev & kBIOSDevMask;
	
	
	
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
#if !TEXT_SPINNER
	printf("Starting Chameleon ...\n");
#endif
	
	initBooterLog();
	
	// Initialize boot info structure.
    initKernBootStruct();
	
	
    // Scan and record the system's hardware information.
    scan_platform();
	
    // First get info for boot volume.
    scanBootVolumes(gBIOSDev, 0);
    bvChain = getBVChainForBIOSDev(gBIOSDev);
    setBootGlobals(bvChain);
	
    // Load boot.plist config file
    status = loadSystemConfig(&bootInfo->bootConfig);
	
	Platform->CPU.isServer = false;    
    getBoolForKey(kIsServer, &Platform->CPU.isServer, &bootInfo->bootConfig); // set this as soon as possible
	
    if (getBoolForKey(kQuietBootKey, &quiet, &bootInfo->bootConfig) && quiet) {
        gBootMode |= kBootModeQuiet;
    }
	
    // Override firstRun to get to the boot menu instantly by setting "Instant Menu"=y in system config
    if (getBoolForKey(kInsantMenuKey, &instantMenu, &bootInfo->bootConfig) && instantMenu) {
        firstRun = false;
    }
	
#ifndef OPTION_ROM
	// Enable touching a single BIOS device only if "Scan Single Drive"=y is set in system config.
    if (getBoolForKey(kScanSingleDriveKey, &gScanSingleDrive, &bootInfo->bootConfig) && gScanSingleDrive) {
        gScanSingleDrive = true;
    }
	
	// Create a list of partitions on device(s).
    if (gScanSingleDrive)
	{
		scanBootVolumes(gBIOSDev, &bvCount);
    } 
	else 
#endif
	{
#if UNUSED
        scanDisks(gBIOSDev, &bvCount);
#else
        scanDisks();
#endif
    }
	
	// Create a separated bvr chain using the specified filters.
    bvChain = newFilteredBVChain(0x80, 0xFF, allowBVFlags, denyBVFlags, &gDeviceCount);
	gBootVolume = selectBootVolume(bvChain);
	
	// Intialize module system
	EFI_STATUS sysinit = init_module_system();
	if((sysinit == EFI_SUCCESS) || (sysinit == EFI_ALREADY_STARTED)/*should never happen*/ ) 
	{
		load_all_modules();
	}
			
    // Loading preboot ramdisk if exists.
	execute_hook("loadPrebootRAMDisk", NULL, NULL, NULL, NULL, NULL, NULL);		

#ifndef OPTION_ROM

    // Disable rescan option by default
    gEnableCDROMRescan = false;
	
    // Enable it with Rescan=y in system config
    if (getBoolForKey(kRescanKey, &gEnableCDROMRescan, &bootInfo->bootConfig) && gEnableCDROMRescan) {
        gEnableCDROMRescan = true;
    }
	
    // Ask the user for Rescan option by setting "Rescan Prompt"=y in system config.
    rescanPrompt = false;
    if (getBoolForKey(kRescanPromptKey, &rescanPrompt , &bootInfo->bootConfig) && rescanPrompt && biosDevIsCDROM(gBIOSDev)) {
        gEnableCDROMRescan = promptForRescanOption();
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
        int trycache = 0;
        long flags, cachetime, kerneltime, exttime;
#ifdef BOOT_HELPER_SUPPORT		
		long time;
#endif
        int ret = -1;
        void *binary = (void *)kLoadAddr;
       		
        // additional variable for testing alternate kernel image locations on boot helper partitions.
        char     bootFileSpec[512];
		
        // Initialize globals.
		
        sysConfigValid = false;
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
				if (gEnableCDROMRescan)
					rescanBIOSDevice(gBIOSDev);
#endif
				
				bvChain = newFilteredBVChain(0x80, 0xFF, allowBVFlags, denyBVFlags, &gDeviceCount);
				setBootGlobals(bvChain);
			}
			continue;
        }
						
        // Other status (e.g. 0) means that we should proceed with boot.
		execute_hook("GUI_PreBoot", NULL, NULL, NULL, NULL, NULL, NULL);
		
		// Find out which version mac os we're booting.
		getOSVersion(gMacOSVersion);
		
		//if (platformCPUFeature(CPU_FEATURE_EM64T)) {        
        if (cpu_mode_is64bit()) {
			archCpuType = CPU_TYPE_X86_64;
		} else {
			archCpuType = CPU_TYPE_I386;
		}
		if (getValueForKey(karch, &val, &len, &bootInfo->bootConfig)) {
			if (strncmp(val, "i386", 4) == 0) {
				archCpuType = CPU_TYPE_I386;
			}
		}
		
		getRootDevice();

		// Notify to all modules that we are attempting to boot
		execute_hook("PreBoot", NULL, NULL, NULL, NULL, NULL, NULL);  
		
		if (execute_hook("getProductNamePatched", NULL, NULL, NULL, NULL, NULL, NULL) != EFI_SUCCESS)					
			readSMBIOS(thePlatformName); // read smbios Platform Name
		
		
		if (!getValueForBootKey(bootArgs->CommandLine, kIgnorePrelinkKern, &val, &len)) {    		
				
			getKernelCachePath(); 
			
		}
		
		// Check for cache file.
		trycache = (((gBootMode & kBootModeSafe) == 0) &&
					!gOverrideKernel &&
					(gBootFileType == kBlockDeviceType) &&
					(gMKextName[0] == '\0') &&
					(gBootKernelCacheFile[0] != '\0'));
		
		verbose("Loading Darwin %s\n", gMacOSVersion);
		
        if (trycache ) do {
			
            // if we haven't found the kernel yet, don't use the cache
            ret = GetFileInfo(NULL, bootInfo->bootFile, &flags, &kerneltime);
            if ((ret != 0) || ((flags & kFileTypeMask) != kFileTypeFlat)) {
                trycache = 0;
				bootInfo->adler32  = 0;
                break;
            }
            ret = GetFileInfo(NULL, gBootKernelCacheFile, &flags, &cachetime);
            if ((ret != 0) || ((flags & kFileTypeMask) != kFileTypeFlat)
                || (cachetime < kerneltime)) {
				trycache = 0;
				bootInfo->adler32  = 0;
				break;				                
            } 
            ret = GetFileInfo("/System/Library/", "Extensions", &flags, &exttime);
            if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeDirectory)
                && (cachetime < exttime)) {
                trycache = 0;
				bootInfo->adler32  = 0;
                break;
            }
            if (kerneltime > exttime) {
                exttime = kerneltime;
            }
            if (cachetime != (exttime + 1)) {
                trycache = 0;
				bootInfo->adler32  = 0;
                break;
            }
        } while (0);
		
        do {
            if (trycache) {
                bootFile = gBootKernelCacheFile;
                verbose("Loading kernel cache %s\n", bootFile);
				if (gMacOSVersion[3] == '7') {					
					ret = LoadThinFatFile(bootFile, &binary);
					if (ret <= 0 && archCpuType == CPU_TYPE_X86_64)
					{
						archCpuType = CPU_TYPE_I386;
						ret = LoadThinFatFile(bootFile, &binary);				
					}
				} else {
					ret = LoadFile(bootFile);
					binary = (void *)kLoadAddr;
				}
				
                if (ret >= 0) {
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
						strcpy(bootFileSpec, bootFile);
					}
				}
            }
#else
			strcpy(bootFileSpec, bootFile);
#endif
			
            verbose("Loading kernel %s\n", bootFileSpec);
            ret = LoadThinFatFile(bootFileSpec, &binary);
            if (ret <= 0 && archCpuType == CPU_TYPE_X86_64)
            {
				archCpuType = CPU_TYPE_I386;
				ret = LoadThinFatFile(bootFileSpec, &binary);				
            }
			
        } while (0);
#if TEXT_SPINNER
        clearActivityIndicator();
#endif
		
#if DEBUG
        printf("Pausing...");
        sleep(8);
#endif
		
        if (ret <= 0) {
			printf("Can't find %s\n", bootFile);
			
			sleep(1);
#ifdef NBP_SUPPORT
            if (gBootFileType == kNetworkDeviceType) {
                // Return control back to PXE. Don't unload PXE base code.
                gUnloadPXEOnExit = false;
                break;
            }
#endif
        } else {
            /* Won't return if successful. */
            ret = ExecKernel(binary);
        }
    }
    
    // chainboot
    if (status==1) {
		if (getVideoMode() == GRAPHICS_MODE) {	// if we are already in graphics-mode,
#if UNUSED
			setVideoMode(VGA_TEXT_MODE, 0);	// switch back to text mode
#else
			setVideoMode(VGA_TEXT_MODE);	// switch back to text mode
#endif
		}
    }
#ifdef NBP_SUPPORT
    if ((gBootFileType == kNetworkDeviceType) && gUnloadPXEOnExit) {
		nbpUnloadBaseCode();
    }
#endif
}

void getKernelCachePath()
{
	long flags, cachetime;
	int ret = -1;
	
	if(gMacOSVersion[3] == '7'){					
		sprintf(gBootKernelCacheFile, "%s", kDefaultCachePath);
	}
	else if(gMacOSVersion[3] <= '6')
	{			 
		
		PlatformInfo    *platformInfo = malloc(sizeof(PlatformInfo));
		if (platformInfo) {
			
			bzero(platformInfo, sizeof(PlatformInfo));
			
			if (gPlatformName)
				strlcpy(platformInfo->platformName,gPlatformName, sizeof(platformInfo->platformName)+1);
			
			if (gRootDevice) {
				char *rootPath_p = platformInfo->rootPath;
				int len = strlen(gRootDevice) + 1;
				if ((unsigned)len > sizeof(platformInfo->rootPath)) {
					len = sizeof(platformInfo->rootPath);
				}
				memcpy(rootPath_p, gRootDevice,len);
				
				rootPath_p += len;
				
				len = strlen(bootInfo->bootFile);
				
				if ((unsigned)(rootPath_p - platformInfo->rootPath + len) >=
					sizeof(platformInfo->rootPath)) {
					
					len = sizeof(platformInfo->rootPath) -
					(rootPath_p - platformInfo->rootPath);
				}                                
				memcpy(rootPath_p, bootInfo->bootFile, len);							
				
			}	
			
			if (!platformInfo->platformName[0] || !platformInfo->rootPath[0]) {
				platformInfo->platformName[0] = platformInfo->rootPath[0] = 0;
			}
			//memcpy(gRootPath,platformInfo->rootPath, sizeof(platformInfo->rootPath));
			
			
			bootInfo->adler32 = OSSwapHostToBigInt32(local_adler32((unsigned char *)platformInfo, sizeof(*platformInfo)));
			
			free(platformInfo);	
		}
		
		DBG("Adler32: %08lX\n",bootInfo->adler32);
		
		if (gMacOSVersion[3] < '6') {
			sprintf(gBootKernelCacheFile, "%s.%08lX", "/System/Library/Caches/com.apple.kernelcaches/kernelcache",bootInfo->adler32);
			ret = GetFileInfo(NULL, gBootKernelCacheFile, &flags, &cachetime);
			if ((ret != 0) || ((flags & kFileTypeMask) != kFileTypeFlat)) {
				bootInfo->adler32 = 0;
				sprintf(gBootKernelCacheFile, "%s", "/System/Library/Caches/com.apple.kernelcaches/kernelcache"); 
			}						
		} else
			sprintf(gBootKernelCacheFile, "%s_%s.%08lX", kDefaultCachePath, (archCpuType == CPU_TYPE_I386) ? "i386" : "x86_64", bootInfo->adler32); //Snow Leopard
		
		
	}
}

// Maximum config table value size
#define VALUE_SIZE 2048
static void getRootDevice()
{
	const char *val = 0;
    int cnt = 0;
	
	if (getValueForKey(kBootUUIDKey, &val, &cnt, &bootInfo->bootConfig)){
		uuidSet = true;		
		
    } else  {
		if (getValueForBootKey(bootArgs->CommandLine, kRootDeviceKey, &val, &cnt)) {
			if (*val == '*' && *(val + 1) != '/' && *(val + 1) != 'u') { 
				val += 1; //skip the *
				uuidSet = true;				
				
			} else if (*val == '*' && *(val + 1) == 'u') {
				
				if ( getValueForKey( kBootDeviceKey, &val, &cnt, &bootInfo->bootConfig)) 
					uuidSet = true;				
				
			} 			
		} else {
#ifdef BOOT_HELPER_SUPPORT
			//
			// Try an alternate method for getting the root UUID on boot helper partitions.
			//
			if (gBootVolume->flags & kBVFlagBooter)
			{
				if((loadHelperConfig(&bootInfo->helperConfig) == 0)
				   && getValueForKey(kHelperRootUUIDKey, &val, &cnt, &bootInfo->helperConfig) )
				{
					getValueForKey(kHelperRootUUIDKey, &val, &cnt, &bootInfo->helperConfig);
					uuidSet = true;
					goto out;
				}
			}
#endif
			
			if ( getValueForKey( kBootDeviceKey, &val, &cnt, &bootInfo->bootConfig)) {
				extern int ArgCntRemaining;
				uuidSet = false;
				char *           valueBuffer;
				valueBuffer = malloc(VALUE_SIZE);
				char *           argP = bootArgs->CommandLine;
				valueBuffer[0] = '*';
				cnt++;
				strlcpy(valueBuffer + 1, val, cnt);				
				if (!copyArgument( kRootDeviceKey, valueBuffer, cnt, &argP, &ArgCntRemaining))
				{
					free(valueBuffer);
					printf("Error: boot arguments too long, unable to set root device !!");
					getc();
					return;
				}
				free(valueBuffer);
				goto out;
			}
			
			if (gBootVolume->fs_getuuid && gBootVolume->fs_getuuid (gBootVolume, bootInfo->uuidStr) == 0) {
				verbose("Setting boot-uuid to: %s\n", bootInfo->uuidStr);
				uuidSet = true;
				gRootDevice = bootInfo->uuidStr;
				return;
			}
		
		}	
	}
		
out:	
	verbose("Setting %s to: %s\n", uuidSet ? kBootUUIDKey : "root device", (char* )val);  
	gRootDevice = (char* )val;
}

static bool getOSVersion(char *str)
{
	bool valid = false;
	config_file_t systemVersion;
	const char *val;
	int len;
	
	if (!loadConfigFile("System/Library/CoreServices/SystemVersion.plist", &systemVersion))
	{
		valid = true;
	}
	else if (!loadConfigFile("System/Library/CoreServices/ServerVersion.plist", &systemVersion))
	{
		valid = true;
	}
	
	if (valid)
	{
		if  (getValueForKey(kProductVersion, &val, &len, &systemVersion))
		{
			// getValueForKey uses const char for val
			// so copy it and trim
			*str = '\0';
			strncat(str, val, MIN(len, 4));
		}
		else
			valid = false;
	}
	
	return valid;
}

unsigned long
local_adler32( unsigned char * buffer, long length )
{
    long          cnt;
    unsigned long result, lowHalf, highHalf;
    
    lowHalf  = 1;
    highHalf = 0;
	
	for ( cnt = 0; cnt < length; cnt++ )
    {
        if ((cnt % 5000) == 0)
        {
            lowHalf  %= 65521L;
            highHalf %= 65521L;
        }
		
        lowHalf  += buffer[cnt];
        highHalf += lowHalf;
    }
	
	lowHalf  %= 65521L;
	highHalf %= 65521L;
	
	result = (highHalf << 16) | lowHalf;
	
	return result;
}
