/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the 'License').  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 *  drivers.c - Driver Loading Functions.
 *
 *  Copyright (c) 2000 Apple Computer, Inc.
 *
 *  DRI: Josh de Cesare
 */

#include <mach-o/fat.h>
#include <libkern/OSByteOrder.h>
#include <mach/machine.h>

#include "sl.h"
#include "boot.h"
#include "bootstruct.h"
#include "platform.h"
#include "xml.h"
#include "drivers.h"
#include "modules.h"

struct Module {  
	struct Module *nextModule;
	long          willLoad;
	TagPtr        dict;
	char          *plistAddr;
	long          plistLength;
	char          *executablePath;
	char          *bundlePath;
	long          bundlePathLength;
};
typedef struct Module Module, *ModulePtr;

struct DriverInfo {
	char *plistAddr;
	long plistLength;
	void *executableAddr;
	long executableLength;
	void *bundlePathAddr;
	long bundlePathLength;
};
typedef struct DriverInfo DriverInfo, *DriverInfoPtr;

#define kDriverPackageSignature1 'MKXT'
#define kDriverPackageSignature2 'MOSX'

struct DriversPackage {
	unsigned long signature1;
	unsigned long signature2;
	unsigned long length;
	unsigned long alder32;
	unsigned long version;
	unsigned long numDrivers;
	unsigned long reserved1;
	unsigned long reserved2;
};
typedef struct DriversPackage DriversPackage;

enum {
	kCFBundleType2,
	kCFBundleType3
};

static ModulePtr gModuleHead, gModuleTail;
static TagPtr    gPersonalityHead, gPersonalityTail;
static char *    gExtensionsSpec;
static char *    gDriverSpec;
static char *    gFileSpec;
static char *    gTempSpec;
static char *    gFileName;

long LoadDrivers( char * dirSpec );
long DecodeKernel(void *binary, entry_t *rentry, char **raddr, int *rsize);
long InitDriverSupport(void);
long FileLoadDrivers(char *dirSpec, long size,long plugin);
long LoadDriverMKext(char *fileSpec);
long LoadDriverPList(char *dirSpec, char *name, long bundleType);
long LoadMatchedModules(void);
long MatchLibraries(void);
#if UNUSED
long MatchPersonalities(void);
#endif
#ifdef NBP_SUPPORT
long NetLoadDrivers(char *dirSpec);
#endif

static long FileLoadMKext( const char * dirSpec, const char * extDirSpec );
static long ParseXML(char *buffer, ModulePtr *module, TagPtr *personalities);
#if UNUSED
static ModulePtr FindModule( char * name );
#endif

//==========================================================================
// InitDriverSupport

long
InitDriverSupport( void )
{
    static bool DriverSet = false;
    
    if (DriverSet == true)  return 0;    
    
    gExtensionsSpec = malloc( DEFAULT_DRIVER_SPEC_SIZE );
    gDriverSpec     = malloc( DEFAULT_DRIVER_SPEC_SIZE );
    gFileSpec       = malloc( DEFAULT_DRIVER_SPEC_SIZE );
    gTempSpec       = malloc( DEFAULT_DRIVER_SPEC_SIZE );
    gFileName       = malloc( DEFAULT_DRIVER_SPEC_SIZE );
    
    if ( !gExtensionsSpec || !gDriverSpec || !gFileSpec || !gTempSpec || !gFileName )
    {
        stop("InitDriverSupport error");
        return -1;
    }
    
    DriverSet = true;
    set_env(envDriverExtSpec,(uint32_t)gExtensionsSpec);
    set_env(envDriverSpec,(uint32_t)gDriverSpec);
    set_env(envDriverFileSpec,(uint32_t)gFileSpec);
    set_env(envDriverTempSpec,(uint32_t)gTempSpec);
    set_env(envDriverFileName,(uint32_t)gFileName);
    
    return 0;
}

//==========================================================================
// LoadDrivers

long LoadDrivers( char * dirSpec )
{	
    char dirSpecExtra[1024];
	
    if ( InitDriverSupport() != 0 )
        return 0;
	
	
	
    // Load extra drivers if a hook has been installed.
    
	int step = 0;
	execute_hook("ramDiskLoadDrivers", &step, NULL, NULL, NULL, NULL, NULL);
#ifdef NBP_SUPPORT	
    if ( get_env(envgBootFileType) == kNetworkDeviceType )
    {
        if (NetLoadDrivers(dirSpec) != 0) {
            error("Could not load drivers from the network\n");
            return -1;
        }
    }
    else
#endif
		if ( get_env(envgBootFileType) == kBlockDeviceType )
		{
			// First try to load Extra extensions from the ramdisk if isn't aliased as bt(0,0).
            
			// First try a specfic OS version folder ie 10.5
			
			step = 1;
			execute_hook("ramDiskLoadDrivers", &step, NULL, NULL, NULL, NULL, NULL);
			
			
			// First try a specfic OS version folder ie 10.5
			snprintf(dirSpecExtra, sizeof(dirSpecExtra) ,"/Extra/%s/", (char*)((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion);
			if (FileLoadDrivers(dirSpecExtra, DEFAULT_DRIVER_SPEC_SIZE, 0) != 0)
			{	
				// Next try to load Extra extensions from the selected root partition.
				strlcpy(dirSpecExtra, "/Extra/", sizeof(dirSpecExtra));
				if (FileLoadDrivers(dirSpecExtra, DEFAULT_DRIVER_SPEC_SIZE, 0) != 0)
				{
					// If failed, then try to load Extra extensions from the boot partition
					// in case we have a separate booter partition or a bt(0,0) aliased ramdisk.
					//if (!(gBIOSBootVolume->biosdev == gBootVolume->biosdev  && gBIOSBootVolume->part_no == gBootVolume->part_no))
					if (!((((BVRef)(uint32_t)get_env(envgBIOSBootVolume))->biosdev == ((BVRef)(uint32_t)get_env(envgBootVolume))->biosdev)  && (((BVRef)(uint32_t)get_env(envgBIOSBootVolume))->part_no == ((BVRef)(uint32_t)get_env(envgBootVolume))->part_no)))
					{
						// First try a specfic OS version folder ie 10.5
						snprintf(dirSpecExtra, sizeof(dirSpecExtra),"bt(0,0)/Extra/%s/", (char*)((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion);
						if (FileLoadDrivers(dirSpecExtra, DEFAULT_DRIVER_SPEC_SIZE, 0) != 0)
						{	
							// Next we'll try the base
							strlcpy(dirSpecExtra, "bt(0,0)/Extra/", sizeof(dirSpecExtra));
							FileLoadDrivers(dirSpecExtra, DEFAULT_DRIVER_SPEC_SIZE, 0);
						}
					}
                    
					step = 2;
					execute_hook("ramDiskLoadDrivers", &step, NULL, NULL, NULL, NULL, NULL);
                    
				}
				
			}	
#ifdef BOOT_HELPER_SUPPORT
			// TODO: fix this, the order does matter, and it's not correct now.
			// Also try to load Extensions from boot helper partitions.
			if (((BVRef)(uint32_t)get_env(envgBootVolume))->flags & kBVFlagBooter)
			{
				strlcpy(dirSpecExtra, "/com.apple.boot.P/System/Library/", sizeof(dirSpecExtra));
				if (FileLoadDrivers(dirSpecExtra, DEFAULT_DRIVER_SPEC_SIZE, 0) != 0)
				{
					strlcpy(dirSpecExtra, "/com.apple.boot.R/System/Library/", sizeof(dirSpecExtra));
					if (FileLoadDrivers(dirSpecExtra, DEFAULT_DRIVER_SPEC_SIZE, 0) != 0)
					{
						strlcpy(dirSpecExtra, "/com.apple.boot.S/System/Library/", sizeof(dirSpecExtra));
						FileLoadDrivers(dirSpecExtra, DEFAULT_DRIVER_SPEC_SIZE, 0);
					}
				}
			}
#endif
			char * MKextName = (char*)(uint32_t)get_env(envMKextName);
			if (MKextName[0] != '\0')
			{
				verbose("LoadDrivers: Loading from [%s]\n", MKextName);
				if ( LoadDriverMKext(MKextName) != 0 )
				{
					error("Could not load %s\n", MKextName);
					return -1;
				}
			}
			else
			{
				strlcpy(gExtensionsSpec, dirSpec, DEFAULT_DRIVER_SPEC_SIZE);
				strlcat(gExtensionsSpec, "System/Library/", DEFAULT_DRIVER_SPEC_SIZE);
				FileLoadDrivers(gExtensionsSpec, DEFAULT_DRIVER_SPEC_SIZE, 0);
			}
		}
		else
		{
			return 0;
		}
#if UNUSED
    MatchPersonalities();
#endif
	
    MatchLibraries();
	
    LoadMatchedModules();
	
    return 0;
}

//==========================================================================
// FileLoadMKext

static long
FileLoadMKext( const char * dirSpec, const char * extDirSpec )
{
	long  ret, flags, time, time2;
	char altDirSpec[512];
	
	snprintf (altDirSpec, sizeof(altDirSpec),"%s%s", dirSpec, extDirSpec);
	ret = GetFileInfo(altDirSpec, "Extensions.mkext", &flags, &time);
	if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeFlat))
	{
		ret = GetFileInfo(dirSpec, "Extensions", &flags, &time2);
		if ((ret != 0) || ((flags & kFileTypeMask) != kFileTypeDirectory) ||
			(((get_env(envgBootMode) & kBootModeSafe) == 0) && (time == (time2 + 1))))
		{
			snprintf(gDriverSpec, DEFAULT_DRIVER_SPEC_SIZE,"%sExtensions.mkext", altDirSpec);
			verbose("LoadDrivers: Loading from [%s]\n", gDriverSpec);
			if (LoadDriverMKext(gDriverSpec) == 0) return 0;
		}
	}
	return -1;
}

//==========================================================================
// FileLoadDrivers

long
FileLoadDrivers( char * dirSpec, long size, long plugin )
{
    long         ret, length, flags, time, bundleType;
    long long	 index;
    long         result = -1;
    const char * name;
	
    if ( !plugin )
    {
        // First try 10.6's path for loading Extensions.mkext.
        if (FileLoadMKext(dirSpec, "Caches/com.apple.kext.caches/Startup/") == 0)
			return 0;
		
        // Next try the legacy path.
        else if (FileLoadMKext(dirSpec, "") == 0)
			return 0;
		
        strlcat(dirSpec, "Extensions", size);
        
        // here we are clearely in a situation where we'll have to load all drivers as with the option -f, in my experience, sometime it can help to add it explicitly in the bootargs
        extern void addBootArg(const char * );
        addBootArg("-f");
        
    }
	
    index = 0;
    while (1) {
        ret = GetDirEntry(dirSpec, &index, &name, &flags, &time);
        if (ret == -1) break;
		
        // Make sure this is a directory.
        if ((flags & kFileTypeMask) != kFileTypeDirectory) continue;
        
        // Make sure this is a kext.
        length = strlen(name);
        if (strncmp(name + length - 5, ".kext", 5)) continue;
		
        // Save the file name.
        strlcpy(gFileName, name, DEFAULT_DRIVER_SPEC_SIZE);
		
        // Determine the bundle type.
        snprintf(gTempSpec, DEFAULT_DRIVER_SPEC_SIZE,"%s/%s", dirSpec, gFileName);
        ret = GetFileInfo(gTempSpec, "Contents", &flags, &time);
        if (ret == 0) bundleType = kCFBundleType2;
        else bundleType = kCFBundleType3;
		
        if (!plugin)
            snprintf(gDriverSpec, DEFAULT_DRIVER_SPEC_SIZE,"%s/%s/%sPlugIns", dirSpec, gFileName,
                    (bundleType == kCFBundleType2) ? "Contents/" : "");
		
        ret = LoadDriverPList(dirSpec, gFileName, bundleType);
		
        if (result != 0)
			result = ret;
		
        if (!plugin)
			FileLoadDrivers(gDriverSpec, 0, 1);
    }
	
    return result;
}

//==========================================================================
// 
#ifdef NBP_SUPPORT
long
NetLoadDrivers( char * dirSpec )
{
    long tries;
	
#if NODEF
    long cnt;
	
    // Get the name of the kernel
    cnt = strlen(gBootFile);
    while (cnt--) {
        if ((gBootFile[cnt] == '\\')  || (gBootFile[cnt] == ',')) {
			cnt++;
			break;
        }
    }
#endif
	
    // INTEL modification
    snprintf(gDriverSpec, DEFAULT_DRIVER_SPEC_SIZE,"%s%s.mkext", dirSpec, bootInfo->bootFile);
    
    verbose("NetLoadDrivers: Loading from [%s]\n", gDriverSpec);
    
    tries = 3;
    while (tries--)
    {
        if (LoadDriverMKext(gDriverSpec) == 0) break;
    }
    if (tries == -1) return -1;
	
    return 0;
}
#endif
//==========================================================================
// loadDriverMKext

long
LoadDriverMKext( char * fileSpec )
{
    unsigned long    driversAddr, driversLength;
    long             length;
    char             segName[32];
    DriversPackage * package;
	
#define GetPackageElement(e)     OSSwapBigToHostInt32(package->e)
	
    // Load the MKext.
    length = LoadThinFatFile(fileSpec, (void **)&package);
    if (!length || (unsigned)length < sizeof (DriversPackage)) return -1;
	
	// call hook to notify modules that the mkext has been loaded
	execute_hook("LoadDriverMKext", (void*)fileSpec, (void*)package, &length, NULL, NULL, NULL);
	
	
    // Verify the MKext.
    if (( GetPackageElement(signature1) != kDriverPackageSignature1) ||
        ( GetPackageElement(signature2) != kDriverPackageSignature2) ||
        ( GetPackageElement(length)      > kLoadSize )               ||
        ( GetPackageElement(alder32)    !=
		 adler32((unsigned char *)&package->version, GetPackageElement(length) - 0x10) ) )
    {
        return -1;
    }
	
	
    // Make space for the MKext.
    driversLength = GetPackageElement(length);
    driversAddr   = AllocateKernelMemory(driversLength);
	
    // Copy the MKext.
    memcpy((void *)driversAddr, (void *)package, driversLength);
	
    // Add the MKext to the memory map.
    snprintf(segName, sizeof(segName),"DriversPackage-%lx", driversAddr);
    
    AllocateMemoryRange(segName, driversAddr, driversLength);
	
    return 0;
}

//==========================================================================
// LoadDriverPList

long
LoadDriverPList( char * dirSpec, char * name, long bundleType )
{
    long      length, executablePathLength, bundlePathLength;
    ModulePtr module = 0;
    TagPtr    personalities;
    char *    buffer = 0;
    char *    tmpExecutablePath = 0;
    char *    tmpBundlePath = 0;
    long      ret = -1;
	
    do {
        // Save the driver path.
        
        snprintf(gFileSpec, DEFAULT_DRIVER_SPEC_SIZE,"%s/%s/%s", dirSpec, name,
                (bundleType == kCFBundleType2) ? "Contents/MacOS/" : "");
        executablePathLength = strlen(gFileSpec) + 1;
		
        tmpExecutablePath = malloc(executablePathLength);
        if (tmpExecutablePath == 0) break;
		
        strlcpy(tmpExecutablePath, gFileSpec, executablePathLength);
        
        snprintf(gFileSpec, DEFAULT_DRIVER_SPEC_SIZE,"%s/%s", dirSpec, name);
        bundlePathLength = strlen(gFileSpec) + 1;
		
        tmpBundlePath = malloc(bundlePathLength);
        if (tmpBundlePath == 0) break;
		
        strlcpy(tmpBundlePath, gFileSpec, bundlePathLength);
		
        // Construct the file spec to the plist, then load it.
		
        snprintf(gFileSpec, DEFAULT_DRIVER_SPEC_SIZE,"%s/%s/%sInfo.plist", dirSpec, name,
                (bundleType == kCFBundleType2) ? "Contents/" : "");
		
        length = LoadFile(gFileSpec);
        if (length == -1) break;
		
        length = length + 1;
        buffer = malloc(length);
        if (buffer == 0) break;
		
        strlcpy(buffer, (char *)kLoadAddr, length);
		
        // Parse the plist.
		
        ret = ParseXML(buffer, &module, &personalities);
        if (ret != 0) { break; }
		
		if (!module) {ret = -1;break;} // Should never happen but it will make the compiler happy

        // Allocate memory for the driver path and the plist.
		
        module->executablePath = tmpExecutablePath;
        module->bundlePath = tmpBundlePath;
        module->bundlePathLength = bundlePathLength;
        module->plistAddr = malloc(length);
		
        if ((module->executablePath == 0) || (module->bundlePath == 0) || (module->plistAddr == 0))
        {
            if ( module->plistAddr ) free(module->plistAddr);
			ret = -1;
            break;
        }
		
        // Save the driver path in the module.
        //strcpy(module->driverPath, tmpDriverPath);
        //tmpExecutablePath = 0;
        //tmpBundlePath = 0;
		
        // Add the plist to the module.
		
        strlcpy(module->plistAddr, (char *)kLoadAddr, length);
        module->plistLength = length;
		
        // Add the module to the end of the module list.
        
        if (gModuleHead == 0)
            gModuleHead = module;
        else
            gModuleTail->nextModule = module;
        gModuleTail = module;
		
        // Add the persionalities to the personality list.
		
        if (personalities) personalities = personalities->tag;
        while (personalities != 0)
        {
            if (gPersonalityHead == 0)
                gPersonalityHead = personalities->tag;
            else
                gPersonalityTail->tagNext = personalities->tag;
            
            gPersonalityTail = personalities->tag;
            personalities = personalities->tagNext;
        }
        
        ret = 0;
    }
    while (0);
    
    if ( buffer )        free( buffer );
    if (ret != 0)
    {
        if ( tmpExecutablePath ) free( tmpExecutablePath );
        if ( tmpBundlePath ) free( tmpBundlePath );
        if ( module )        free( module );
    }	
    return ret;
}

//==========================================================================
// LoadMatchedModules

long LoadMatchedModules( void )
{
    TagPtr        prop;
    ModulePtr     module;
    char          *fileName, segName[32];
    DriverInfoPtr driver;
    long          length, driverAddr, driverLength;
    void          *executableAddr = 0;
	
	
    module = gModuleHead;
	
    while (module != 0)
    {
        if (module->willLoad)
        {
            prop = XMLGetProperty(module->dict, kPropCFBundleExecutable);
			
            if (prop != 0)
            {
                fileName = prop->string;
                snprintf(gFileSpec, DEFAULT_DRIVER_SPEC_SIZE,"%s%s", module->executablePath, fileName);
                length = LoadThinFatFile(gFileSpec, &executableAddr);
				if (length == 0)
				{
					length = LoadFile(gFileSpec);
					executableAddr = (void *)kLoadAddr;
				}
                //printf("%s length = %d addr = 0x%x\n", gFileSpec, length, driverModuleAddr); getc();
                
            }
            else
                length = 0;
			
            if ((length != -1) && executableAddr)
            {
                
                // Make make in the image area.
				
				execute_hook("LoadMatchedModules", module, &length, executableAddr, NULL, NULL, NULL);
				
                driverLength = sizeof(DriverInfo) + module->plistLength + length + module->bundlePathLength;
                driverAddr = AllocateKernelMemory(driverLength);
				
                // Set up the DriverInfo.
                driver = (DriverInfoPtr)driverAddr;
                driver->plistAddr = (char *)(driverAddr + sizeof(DriverInfo));
                driver->plistLength = module->plistLength;
                if (length != 0)
                {
                    driver->executableAddr = (void *)(driverAddr + sizeof(DriverInfo) +
													  module->plistLength);
                    driver->executableLength = length;
                }
                else
                {
                    driver->executableAddr   = 0;
                    driver->executableLength = 0;
                }
                driver->bundlePathAddr = (void *)(driverAddr + sizeof(DriverInfo) +
												  module->plistLength + driver->executableLength);
                driver->bundlePathLength = module->bundlePathLength;
				
                // Save the plist, module and bundle.
                strlcpy(driver->plistAddr, module->plistAddr,driver->plistLength);
                if (length != 0)
                {
                    memcpy(driver->executableAddr, executableAddr, length);
                }
                strlcpy(driver->bundlePathAddr, module->bundlePath, module->bundlePathLength);
				
                // Add an entry to the memory map.
                snprintf(segName, sizeof(segName),"Driver-%lx", (unsigned long)driver);
                
                AllocateMemoryRange(segName, driverAddr, driverLength);
                
            }
        }
        module = module->nextModule;
    }
	
    return 0;
}

#if UNUSED
//==========================================================================
// MatchPersonalities

long MatchPersonalities( void )
{
    /* IONameMatch support not implemented */
    return 0;
}
#endif

//==========================================================================
// MatchLibraries

long MatchLibraries( void )
{
    TagPtr     prop, prop2;
    ModulePtr  module, module2;
    long       done;
	
    do {
        done = 1;
        module = gModuleHead;
        
        while (module != 0)
        {
            if (module->willLoad == 1)
            {
                prop = XMLGetProperty(module->dict, kPropOSBundleLibraries);
                if (prop != 0)
                {
                    prop = prop->tag;
                    while (prop != 0)
                    {
                        module2 = gModuleHead;
                        while (module2 != 0)
                        {
                            prop2 = XMLGetProperty(module2->dict, kPropCFBundleIdentifier);
                            if ((prop2 != 0) && (!strcmp(prop->string, prop2->string)))
                            {
                                if (module2->willLoad == 0) module2->willLoad = 1;
                                break;
                            }
                            module2 = module2->nextModule;
                        }
                        prop = prop->tagNext;
                    }
                }
                module->willLoad = 2;
                done = 0;
            }
            module = module->nextModule;
        }
    }
    while (!done);
	
    return 0;
}


//==========================================================================
// FindModule

#if UNUSED
static ModulePtr
FindModule( char * name )
{
    ModulePtr module;
    TagPtr    prop;
    
    module = gModuleHead;
    
    while (module != 0)
    {
        prop = GetProperty(module->dict, kPropCFBundleIdentifier);
        if ((prop != 0) && !strcmp(name, prop->string)) break;
        module = module->nextModule;
    }
    
    return module;
}
#endif

//==========================================================================
// ParseXML

static long
ParseXML( char * buffer, ModulePtr * module, TagPtr * personalities )
{
	long       length, pos;
	TagPtr     moduleDict, required;
	ModulePtr  tmpModule;
	
    pos = 0;
	
    while (1)
    {
        length = XMLParseNextTag(buffer + pos, &moduleDict);
        if (length == -1) break;
		
        pos += length;
		
        if (moduleDict == 0) continue;
        if (moduleDict->type == kTagTypeDict) break;
		
        XMLFreeTag(moduleDict);
    }
	
    if (length == -1) return -1;
	
    required = XMLGetProperty(moduleDict, kPropOSBundleRequired);
    if ( (required == 0) ||
		(required->type != kTagTypeString) ||
		!strncmp(required->string, "Safe Boot", sizeof("Safe Boot")))
    {
        XMLFreeTag(moduleDict);
        return -2;
    }
	
    tmpModule = malloc(sizeof(Module));
    if (tmpModule == 0)
    {
        XMLFreeTag(moduleDict);
        return -1;
    }
    tmpModule->dict = moduleDict;
	
    // For now, load any module that has OSBundleRequired != "Safe Boot".
	
    tmpModule->willLoad = 1;
	
    *module = tmpModule;
	
    // Get the personalities.
	
    *personalities = XMLGetProperty(moduleDict, kPropIOKitPersonalities);
	
    return 0;
}


long 
DecodeKernel(void *binary, entry_t *rentry, char **raddr, int *rsize)
{
    long ret;
    compressed_kernel_header * kernel_header = (compressed_kernel_header *) binary;
	
#if 0
    printf("kernel header:\n");
    printf("signature: 0x%x\n", kernel_header->signature);
    printf("compress_type: 0x%x\n", kernel_header->compress_type);
    printf("adler32: 0x%x\n", kernel_header->adler32);
    printf("uncompressed_size: 0x%x\n", kernel_header->uncompressed_size);
    printf("compressed_size: 0x%x\n", kernel_header->compressed_size);
    getc();
#endif
	
    if (kernel_header->signature == OSSwapBigToHostConstInt32('comp'))
	{
        if (kernel_header->compress_type != OSSwapBigToHostConstInt32('lzss'))
		{
            error("kernel compression is bad\n");
            return -1;
        }
		
		u_int32_t uncompressed_size, size;
		
        uncompressed_size = OSSwapBigToHostInt32(kernel_header->uncompressed_size);
        binary = malloc(uncompressed_size);
		if (!binary) {
			printf("Unable to allocate memory for uncompressed kernel\n");
            return -1;
		}
		
        size = decompress_lzss((u_int8_t *) binary, &kernel_header->data[0],
                               OSSwapBigToHostInt32(kernel_header->compressed_size));
        if (uncompressed_size != size)
		{
            error("size mismatch from lzss: %x\n", size);
            return -1;
        }
        if (OSSwapBigToHostInt32(kernel_header->adler32) !=
            adler32(binary, uncompressed_size))
		{
            printf("adler mismatch\n");
            return -1;
        }
        char* BootKernelCacheFile = (char*)(uint32_t)get_env(envkCacheFile);
		if (((get_env(envgBootMode) & kBootModeSafe) == 0) && (BootKernelCacheFile[0] != '\0') && ((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion[3] > '6') 
			safe_set_env(envAdler32, kernel_header->adler32);
    }
	
	{
		unsigned long len;
		ret = ThinFatFile(&binary, &len);
		if ((ret == 0) && (len == 0) && (get_env(envarchCpuType)==CPU_TYPE_X86_64))
		{
			safe_set_env(envarchCpuType, CPU_TYPE_I386);
			ThinFatFile(&binary, &len);
		}
		
		ret = DecodeMachO(binary, rentry, raddr, rsize);
		
		if (ret<0 && get_env(envarchCpuType)==CPU_TYPE_X86_64)
		{
			safe_set_env(envarchCpuType, CPU_TYPE_I386);
			ret = DecodeMachO(binary, rentry, raddr, rsize);
		}
	}
    
	return ret;
}