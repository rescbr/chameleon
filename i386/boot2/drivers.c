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
#include "xml.h"
#include "ramdisk.h"
#include "modules.h"

#if DEBUG
	#define DBG(x...)	printf(x)
#else
	#define DBG(x...)	msglog(x)
#endif

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
	unsigned long adler32;
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

long (*LoadExtraDrivers_p)(FileLoadDrivers_t FileLoadDrivers_p);

static inline unsigned long Adler32( unsigned char * buffer, long length );

long FileLoadDrivers(char *dirSpec, long plugin);
long NetLoadDrivers(char *dirSpec);
long LoadDriverMKext(char *fileSpec);
long LoadDriverPList(char *dirSpec, char *name, long bundleType);
long LoadMatchedModules(void);

static long MatchPersonalities(void);
static long MatchLibraries(void);

#ifdef NOTDEF
	static ModulePtr FindModule(char *name);
	static void ThinFatFile(void **loadAddrP, unsigned long *lengthP);
#endif
static long ParseXML(char *buffer, ModulePtr *module, TagPtr *personalities);
static long InitDriverSupport(void);

ModulePtr gModuleHead, gModuleTail;
static TagPtr	gPersonalityHead, gPersonalityTail;
static char	*gExtensionsSpec;
static char	*gDriverSpec;
static char	*gFileSpec;
static char	*gTempSpec;
static char	*gFileName;
// Bungo:
char gDarwinBuildVerStr[256] = "Darwin Kernel Version";

static inline unsigned long Adler32( unsigned char *buffer, long length )
{
	long          cnt;
	unsigned long result, lowHalf, highHalf;

	lowHalf  = 1;
	highHalf = 0;

	for (cnt = 0; cnt < length; cnt++)
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

//==========================================================================
// InitDriverSupport
static long InitDriverSupport( void )
{
	gExtensionsSpec = malloc( 4096 );
	gDriverSpec     = malloc( 4096 );
	gFileSpec       = malloc( 4096 );
	gTempSpec       = malloc( 4096 );
	gFileName       = malloc( 4096 );

	if ( !gExtensionsSpec || !gDriverSpec || !gFileSpec || !gTempSpec || !gFileName )
	{
		stop("InitDriverSupport error");
	}

	return 0;
}

//==========================================================================
// LoadDrivers
long LoadDrivers( char *dirSpec )
{
	char dirSpecExtra[1024];

	if ( InitDriverSupport() != 0 )
	{
		return 0;
	}

	// ========================================

	// Load extra drivers if a hook has been installed.
	if (LoadExtraDrivers_p != NULL)
	{
		(*LoadExtraDrivers_p)(&FileLoadDrivers);
	}

	if ( gBootFileType == kNetworkDeviceType )
	{
		if (NetLoadDrivers(dirSpec) != 0)
		{
			error("LoadDrivers: Could not load drivers from the network\n");
			return -1;
		}
	}
	else if ( gBootFileType == kBlockDeviceType )
	{
		// First try to load Extra extensions from the ramdisk if isn't aliased as bt(0,0).
		if (gRAMDiskVolume && !gRAMDiskBTAliased)
		{
			strcpy(dirSpecExtra, "rd(0,0)/Extra/");
			FileLoadDrivers(dirSpecExtra, 0);
		}
		verbose("Attempting to loading drivers from \"Extra\" repository:\n");

		// =====================================================================
		// Firstly try to load drivers from Common folder
		sprintf(dirSpecExtra, "bt(0,0)/Extra/Common/");
		FileLoadDrivers(dirSpecExtra, 0);
		// =====================================================================

		// Next try to load Extra extensions from the selected root partition.
		strlcpy(dirSpecExtra, "/Extra/", sizeof(dirSpecExtra));
		if (FileLoadDrivers(dirSpecExtra, 0) != 0)
		{
			// If failed, then try to load Extra extensions from the boot partition
			// in case we have a separate booter partition or a bt(0,0) aliased ramdisk.
			if ( !(gBIOSBootVolume->biosdev == gBootVolume->biosdev  && gBIOSBootVolume->part_no == gBootVolume->part_no)
				|| (gRAMDiskVolume && gRAMDiskBTAliased) )
			{
				// Next try a specfic OS version folder ie 10.5
				sprintf(dirSpecExtra, "bt(0,0)/Extra/%s/", &gMacOSVersion[0]);
				if (FileLoadDrivers(dirSpecExtra, 0) != 0)
				{
					// Next we'll try the base
					strlcpy(dirSpecExtra, "bt(0,0)/Extra/", sizeof(dirSpecExtra));
					FileLoadDrivers(dirSpecExtra, 0);
				}
			}
		}

		if(!gHaveKernelCache)
		{
			// Don't load main driver (from /System/Library/Extentions) if gHaveKernelCache is set.
			// since these drivers will already be in the kernel cache.
			// NOTE: when gHaveKernelCache, xnu cannot (by default) load *any* extra kexts from the bootloader.
			// The /Extra code is not disabled in this case due to a kernel patch that allows for this to happen.

			// Also try to load Extensions from boot helper partitions.
			if (gBootVolume->flags & kBVFlagBooter)
			{
				strlcpy(dirSpecExtra, "/com.apple.boot.P/System/Library/", sizeof(dirSpecExtra));
				if (FileLoadDrivers(dirSpecExtra, 0) != 0)
				{
					strlcpy(dirSpecExtra, "/com.apple.boot.R/System/Library/", sizeof(dirSpecExtra));
					if (FileLoadDrivers(dirSpecExtra, 0) != 0)
					{
						strlcpy(dirSpecExtra, "/com.apple.boot.S/System/Library/", sizeof(dirSpecExtra));
						FileLoadDrivers(dirSpecExtra, 0);
					}
				}
			}

			if (gMKextName[0] != '\0')
			{
				verbose("LoadDrivers: Loading from '%s'\n", gMKextName);
				if ( LoadDriverMKext(gMKextName) != 0 )
				{
					error("Could not load %s\n", gMKextName);
					return -1;
				}
			}
			else
			{
				verbose("Attempting to load drivers from standard repositories:\n");

				if ( (gMacOSVersion[3] == '9') || ((gMacOSVersion[3] == '1') && ((gMacOSVersion[4] == '0') || (gMacOSVersion[4] == '1') || (gMacOSVersion[4] == '2') ) )) // issue 352
				{
					verbose("\t- Third party extensions search path: /Library/Extensions\n");
					strlcpy(gExtensionsSpec, dirSpec, 4087); /* 4096 - sizeof("Library/") mean 4096 - 9 = 4087 */
					strcat(gExtensionsSpec, "Library/");
					FileLoadDrivers(gExtensionsSpec, 0);
				}

				verbose("\t- Apple extensions search path: /System/Library/Extensions\n");
				strlcpy(gExtensionsSpec, dirSpec, 4080); /* 4096 - sizeof("System/Library/")  mean 4096 -16 = 4080 */
				strcat(gExtensionsSpec, "System/Library/");
				FileLoadDrivers(gExtensionsSpec, 0);
			}

		}
	}
	else
	{
		return 0;
	}

	MatchPersonalities();

	MatchLibraries();

	LoadMatchedModules();

	return 0;
}

//==========================================================================
// FileLoadMKext
static long FileLoadMKext( const char *dirSpec, const char *extDirSpec )
{
	long		ret, flags;
	u_int32_t	time, time2;
	char		altDirSpec[512];
	
	snprintf(altDirSpec, sizeof(altDirSpec), "%s%s", dirSpec, extDirSpec);
	ret = GetFileInfo(altDirSpec, "Extensions.mkext", &flags, &time);

	if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeFlat))
	{
		ret = GetFileInfo(dirSpec, "Extensions", &flags, &time2);

		if ((ret != 0)
			|| ((flags & kFileTypeMask) != kFileTypeDirectory)
			|| (((gBootMode & kBootModeSafe) == 0) && (time == (time2 + 1))))
		{
			snprintf(gDriverSpec, sizeof(altDirSpec) + 18, "%sExtensions.mkext", altDirSpec);
			verbose("LoadDrivers: Loading from '%s'\n", gDriverSpec);

			if (LoadDriverMKext(gDriverSpec) == 0)
			{
				return 0;
			}
		}
	}
	return -1;
}

//==========================================================================
// FileLoadDrivers
long FileLoadDrivers( char *dirSpec, long plugin )
{
	long long	index;
	long		ret, length, flags, bundleType;
	long		result = -1;
	u_int32_t	time;
	const char	* name;

	if ( !plugin )
	{
		// First try 10.6's path for loading Extensions.mkext.
		if (FileLoadMKext(dirSpec, "Caches/com.apple.kext.caches/Startup/") == 0) {
			return 0;
		}

		// Next try the legacy path.
		else if (FileLoadMKext(dirSpec, "") == 0)
		{
			return 0;
		}

		strcat(dirSpec, "Extensions");
	}

	index = 0;
	while (1)
	{
		ret = GetDirEntry(dirSpec, &index, &name, &flags, &time);
		if (ret == -1) {
			break;
		}

		// Make sure this is a directory.
		if ((flags & kFileTypeMask) != kFileTypeDirectory)
		{
			continue;
		}

		// Make sure this is a kext.
		length = strlen(name);
		if (strncmp(name + length - 5, ".kext", sizeof(".kext") ))
		{
			continue;
		}

		// Save the file name.
		strlcpy(gFileName, name, 4096);

		// Determine the bundle type.
		snprintf(gTempSpec, 4096, "%s/%s", dirSpec, gFileName);
		ret = GetFileInfo(gTempSpec, "Contents", &flags, &time);
		if (ret == 0)
		{
			bundleType = kCFBundleType2;
		}
		else
		{
			bundleType = kCFBundleType3;
		}

		if (!plugin)
		{
			snprintf(gDriverSpec, 4096, "%s/%s/%sPlugIns", dirSpec, gFileName, (bundleType == kCFBundleType2) ? "Contents/" : "");
		}

		ret = LoadDriverPList(dirSpec, gFileName, bundleType);

		if (result != 0)
		{
			result = ret;
		}

		if (!plugin)
		{
			FileLoadDrivers(gDriverSpec, 1);
		}
	}

	return result;
}

//==========================================================================
// 
long NetLoadDrivers( char *dirSpec )
{
	long tries;

#if NODEF
	long cnt;

	// Get the name of the kernel
	cnt = strlen(gBootFile);
	while (cnt--) {
		if ((gBootFile[cnt] == '\\')  || (gBootFile[cnt] == ','))
		{
			cnt++;
			break;
		}
	}
#endif

	// INTEL modification
	snprintf(gDriverSpec, 4096, "%s%s.mkext", dirSpec, bootInfo->bootFile);
    
	verbose("NetLoadDrivers: Loading from [%s]\n", gDriverSpec);
    
	tries = 3;
	while (tries--)
	{
		if (LoadDriverMKext(gDriverSpec) == 0)
		{
			break;
		}
	}
	if (tries == -1)
	{
		return -1;
	}

	return 0;
}

//==========================================================================
// loadDriverMKext
long LoadDriverMKext( char *fileSpec )
{
	unsigned long	driversAddr, driversLength;
	long		length;
	char		segName[32];
	DriversPackage	*package;

#define GetPackageElement(e)     OSSwapBigToHostInt32(package->e)

	// Load the MKext.
	length = LoadThinFatFile(fileSpec, (void **)&package);
	if (length < sizeof (DriversPackage))
	{
		return -1;
	}

	// call hook to notify modules that the mkext has been loaded
	execute_hook("LoadDriverMKext", (void *)fileSpec, (void *)package, (void *) &length, NULL);

	
	// Verify the MKext.
	if (( GetPackageElement(signature1) != kDriverPackageSignature1) ||
		( GetPackageElement(signature2) != kDriverPackageSignature2) ||
		( GetPackageElement(length)      > kLoadSize )               ||
		( GetPackageElement(adler32)    !=
		Adler32((unsigned char *)&package->version, GetPackageElement(length) - 0x10) ) )
	{
		return -1;
	}

	// Make space for the MKext.
	driversLength = GetPackageElement(length);
	driversAddr   = AllocateKernelMemory(driversLength);

	// Copy the MKext.
	memcpy((void *)driversAddr, (void *)package, driversLength);

	// Add the MKext to the memory map.
	snprintf(segName, sizeof(segName), "DriversPackage-%lx", driversAddr);
	AllocateMemoryRange(segName, driversAddr, driversLength, kBootDriverTypeMKEXT);

	return 0;
}

//==========================================================================
// LoadDriverPList
long LoadDriverPList( char *dirSpec, char *name, long bundleType )
{
	long		length, executablePathLength, bundlePathLength;
	ModulePtr	module;
	TagPtr		personalities;
	char		*buffer = 0;
	char		*tmpExecutablePath = 0;
	char		*tmpBundlePath = 0;
	long		ret = -1;

	do {
	// Save the driver path.
        
	if(name)
	{
		snprintf(gFileSpec, 4096, "%s/%s/%s", dirSpec, name, (bundleType == kCFBundleType2) ? "Contents/MacOS/" : "");
	}
	else
	{
		snprintf(gFileSpec, 4096, "%s/%s", dirSpec, (bundleType == kCFBundleType2) ? "Contents/MacOS/" : "");
	}
	executablePathLength = strlen(gFileSpec) + 1;

	tmpExecutablePath = malloc(executablePathLength);
	if (tmpExecutablePath == 0) {
		break;
	}
	strlcpy(tmpExecutablePath, gFileSpec, executablePathLength);

	if(name)
	{
		snprintf(gFileSpec, 4096, "%s/%s", dirSpec, name);
	}
	else
	{
		strncpy(gFileSpec, dirSpec, 4096);
	}
	bundlePathLength = strlen(gFileSpec) + 1;

	tmpBundlePath = malloc(bundlePathLength);
	if (tmpBundlePath == 0)
	{
		break;
	}

	strlcpy(tmpBundlePath, gFileSpec, bundlePathLength);

	// Construct the file spec to the plist, then load it.

	if(name)
	{
		snprintf(gFileSpec, 4096, "%s/%s/%sInfo.plist", dirSpec, name, (bundleType == kCFBundleType2) ? "Contents/" : "");
	}
	else
	{
		snprintf(gFileSpec, 4096, "%s/%sInfo.plist", dirSpec, (bundleType == kCFBundleType2) ? "Contents/" : "");
	}

	length = LoadFile(gFileSpec);

	if (length == -1)
	{
		break;
	}

	length = length + 1;
	buffer = malloc(length);

	if (buffer == 0)
	{
		break;
	}

	strlcpy(buffer, (char *)kLoadAddr, length);

	// Parse the plist.

	ret = ParseXML(buffer, &module, &personalities);

	if (ret != 0) {
		break;
	}

	if (!module) // cparm
	{
		ret = -1;
		break;
	} // Should never happen but it will make the compiler happy

	// Allocate memory for the driver path and the plist.

	module->executablePath = tmpExecutablePath;
	module->bundlePath = tmpBundlePath;
	module->bundlePathLength = bundlePathLength;
	module->plistAddr = malloc(length);

	if ((module->executablePath == 0) || (module->bundlePath == 0) || (module->plistAddr == 0))
	{
		break;
	}

	// Save the driver path in the module.
	//strcpy(module->driverPath, tmpDriverPath);
	tmpExecutablePath = 0;
	tmpBundlePath = 0;

	// Add the plist to the module.

	strlcpy(module->plistAddr, (char *)kLoadAddr, length);
	module->plistLength = length;

	// Add the module to the end of the module list.
        
	if (gModuleHead == 0) {
		gModuleHead = module;
	} else 	{
		gModuleTail->nextModule = module;
	}
	gModuleTail = module;

	// Add the persionalities to the personality list.

	if (personalities) {
		personalities = personalities->tag;
	}
	while (personalities != 0)
	{
		if (gPersonalityHead == 0) {
			gPersonalityHead = personalities->tag;
		} else {
			gPersonalityTail->tagNext = personalities->tag;
		}

		gPersonalityTail = personalities->tag;
		personalities = personalities->tagNext;
	}
        
	ret = 0;
	}
	while (0);
    
	if ( buffer ) {
		free( buffer );
	}
	if ( tmpExecutablePath ) {
		free( tmpExecutablePath );
	}
	if ( tmpBundlePath ) {
		free( tmpBundlePath );
	}
	return ret;
}

//==========================================================================
// LoadMatchedModules
long LoadMatchedModules( void )
{
	TagPtr	prop;
	ModulePtr	module;
	char		*fileName, segName[32];
	DriverInfoPtr	driver;
	long		length, driverAddr, driverLength;
	void		*executableAddr = 0;

	module = gModuleHead;

	while (module != 0)
	{
		if (module->willLoad)
		{
			prop = XMLGetProperty(module->dict, kPropCFBundleExecutable);

			if (prop != 0)
			{
				fileName = prop->string;
				snprintf(gFileSpec, 4096, "%s%s", module->executablePath, fileName);

				length = LoadThinFatFile(gFileSpec, &executableAddr);
				if (length == 0)
				{
					length = LoadFile(gFileSpec);
					executableAddr = (void *)kLoadAddr;
				}
//		printf("%s length = %d addr = 0x%x\n", gFileSpec, length, driverModuleAddr); getchar();
			}
			else
			{
				length = 0;
			}

			if ((length != -1) && executableAddr)
			{
				// Make make in the image area.

				execute_hook("LoadMatchedModules", module, &length, executableAddr, NULL);

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
					driver->executableAddr	 = 0;
					driver->executableLength = 0;
				}

				driver->bundlePathAddr = (void *)(driverAddr + sizeof(DriverInfo) +
									 module->plistLength + driver->executableLength);
				driver->bundlePathLength = module->bundlePathLength;

				// Save the plist, module and bundle.
				strlcpy(driver->plistAddr, module->plistAddr, driver->plistLength);

				if (length != 0)
				{
					memcpy(driver->executableAddr, executableAddr, length);
				}

				strlcpy(driver->bundlePathAddr, module->bundlePath, module->bundlePathLength);

				// Add an entry to the memory map.
				snprintf(segName, sizeof(segName), "Driver-%lx", (unsigned long)driver);
				AllocateMemoryRange(segName, driverAddr, driverLength,
									kBootDriverTypeKEXT);
			}
		}
		module = module->nextModule;
	}

	return 0;
}

//==========================================================================
// MatchPersonalities
static long MatchPersonalities( void )
{
	/* IONameMatch support not implemented */
	return 0;
}

//==========================================================================
// MatchLibraries
static long MatchLibraries( void )
{
	TagPtr     prop;
	TagPtr     prop2;
	ModulePtr  module;
	ModulePtr  module2;
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
								if (module2->willLoad == 0)
								{
									module2->willLoad = 1;
								}
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

#if NOTDEF
static ModulePtr FindModule( char *name )
{
	ModulePtr module;
	TagPtr    prop;

	module = gModuleHead;

	while (module != 0)
	{
		prop = GetProperty(module->dict, kPropCFBundleIdentifier);

		if ((prop != 0) && !strcmp(name, prop->string))
		{
			break;
		}

		module = module->nextModule;
	}
    
	return module;
}
#endif /* NOTDEF */

//==========================================================================
// ParseXML
static long ParseXML( char *buffer, ModulePtr *module, TagPtr *personalities )
{
	long		length;
	long		pos = 0;
	TagPtr		moduleDict;
	TagPtr		required;
	ModulePtr	tmpModule;

	while (1)
	{
		length = XMLParseNextTag(buffer + pos, &moduleDict);
		if (length == -1)
		{
			break;
		}

		pos += length;

		if (moduleDict == 0)
		{
			continue;
		}
		if (moduleDict->type == kTagTypeDict)
		{
			break;
		}
		XMLFreeTag(moduleDict);
	}

	if (length == -1)
	{
		return -1;
	}

	required = XMLGetProperty(moduleDict, kPropOSBundleRequired);

	if ( (required == 0) || (required->type != kTagTypeString) || !strncmp(required->string, "Safe Boot", sizeof("Safe Boot")))
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

#if NOTDEF
static char gPlatformName[64];
#endif

long DecodeKernel(void *binary, entry_t *rentry, char **raddr, int *rsize)
{
	long ret = 0;
	compressed_kernel_header *kernel_header = (compressed_kernel_header *)binary;
	u_int32_t uncompressed_size = 0, size = 0, adler32 = 0;
	void *buffer = NULL;
	unsigned long len = 0;

/*#if 0
	printf("kernel header:\n");
	printf("signature: 0x%x\n", kernel_header->signature);
	printf("compress_type: 0x%x\n", kernel_header->compress_type);
	printf("adler32: 0x%x\n", kernel_header->adler32);
	printf("uncompressed_size: 0x%x\n", kernel_header->uncompressed_size);
	printf("compressed_size: 0x%x\n", kernel_header->compressed_size);
	getchar();
#endif*/

	if (kernel_header->signature == OSSwapBigToHostConstInt32('comp'))
	{
		DBG("Decompressing Kernel Cache:\n");

		if ((kernel_header->compress_type != OSSwapBigToHostConstInt32('lzss')) &&
			(kernel_header->compress_type != OSSwapBigToHostConstInt32('lzvn')))
		{
			error("\tERROR: kernel compression is bad!\n");
			return -1;
		}

		if (kernel_header->compress_type == OSSwapBigToHostConstInt32('lzss'))
		{
			verbose ("\t- Decompressing Kernel Using lzss\n");
		}

		if (kernel_header->compress_type == OSSwapBigToHostConstInt32('lzvn'))
		{
			verbose ("\t- Decompressing Kernel Using lzvn\n");
		}

#if NOTDEF
		if (kernel_header->platform_name[0] && strcmp(gPlatformName, kernel_header->platform_name))
		{
			return -1;
		}

		if (kernel_header->root_path[0] && strcmp(gBootFile, kernel_header->root_path))
		{
			return -1;
		}
#endif
		uncompressed_size = OSSwapBigToHostInt32(kernel_header->uncompressed_size);
		binary = buffer = malloc(uncompressed_size);

		// MinusZwei
		size = 0;
		switch (kernel_header->compress_type)
		{
			case OSSwapBigToHostConstInt32('lzvn'):
				size = lzvn_decode(binary, uncompressed_size, &kernel_header->data[0], OSSwapBigToHostInt32(kernel_header->compressed_size));
				break;

			case OSSwapBigToHostConstInt32('lzss'):
				size = decompress_lzss((u_int8_t *)binary, uncompressed_size, &kernel_header->data[0], OSSwapBigToHostInt32(kernel_header->compressed_size));
				break;

			default:
				break;
		}
		// MinusZwei

		if (uncompressed_size != size)
		{
			if ( kernel_header->compress_type == OSSwapBigToHostConstInt32('lzvn'))
			{
				error("ERROR! Size mismatch from lzvn (found: %x, expected: %x).\n", size, uncompressed_size);
			}

			if ( kernel_header->compress_type == OSSwapBigToHostConstInt32('lzss'))
			{
				error("ERROR! size mismatch from lzss (found: %x, expected: %x).\n", size, uncompressed_size);
			}

			return -1;
		}

		adler32 = Adler32(binary, uncompressed_size);
		if (OSSwapBigToHostInt32(kernel_header->adler32) != adler32)
		{
			error("ERROR! Adler mismatch (found: %X, expected: %X).\n", adler32, OSSwapBigToHostInt32(kernel_header->adler32));
			return -1;
		}

		DBG("\n");
	}
	
	ret = ThinFatFile(&binary, &len);
	if (ret == 0 && len == 0 && archCpuType==CPU_TYPE_X86_64)
	{
		archCpuType=CPU_TYPE_I386;
		ret = ThinFatFile(&binary, &len);
	}

	// Bungo: scan binary for Darwin Kernel Version string
	useDarwinVersion = true;
	uint32_t offset = 0;
	strncpy(gDarwinBuildVerStr, "Darwin Kernel Version", sizeof(gDarwinBuildVerStr));

	while ((offset < 0xFFFFFFFF - (uint32_t)binary - 256) && memcmp(binary + offset, gDarwinBuildVerStr, 21))
	{
		offset++;
	}
	if (offset < 0xFFFFFFFF - (uint32_t)binary - 256)
	{
		strncpy(gDarwinBuildVerStr, (char *)(binary + offset), sizeof(gDarwinBuildVerStr));
	}
	else
	{
		strcat(gDarwinBuildVerStr, ": Unknown");
		useDarwinVersion = false;
	}
	// Micky1979 use Bungo gDarwinBuildVerStr and split into gDarwinMajor, gDarwinMinor and gDarwinRev
	if (useDarwinVersion)
	{
		useDarwinVersion = false;
		const char *pattern = strstr(gDarwinBuildVerStr, "Darwin Kernel Version ")+22;
		const char *until = strstr(pattern, ":");
		size_t vlen = until - pattern;
		char *ver = (char *)malloc(sizeof(char)*(len+1));
		strncpy(ver, pattern, vlen);
		ver[vlen] = '\0';
		char *delim;
		char *temp;
		gDarwinMajor = -1; gDarwinMinor = -1; gDarwinRev = -1;
		if (ver != NULL)
		{
			temp = ver;
			int count = 1;
			while ((delim = strsep_c(&ver, ".")) != NULL)
			{
				switch (count)
				{
					case 1: gDarwinMajor = atoi(delim); break;
					case 2: gDarwinMinor = atoi(delim); break;
					case 3: gDarwinRev   = atoi(delim); break;
					default: break;
				}
				count ++;
			}
			free(temp);
		}

		if (gDarwinMajor >= 0 && gDarwinMinor >= 0 && gDarwinRev >= 0)
		{
			useDarwinVersion = true;
		}

		switch (gDarwinMajor)
		{
			case 10:
				switch (gDarwinMinor)
				{
					case 0: kernelOSVer = 0xA060000; break;
					case 1: kernelOSVer = 0xA060100; break;
					case 2: kernelOSVer = 0xA060200; break;
					case 3: kernelOSVer = 0xA060300; break;
					case 4: kernelOSVer = 0xA060400; break;
					case 5: kernelOSVer = 0xA060500; break;
					case 6: kernelOSVer = 0xA060600; break;
					case 7: kernelOSVer = 0xA060700; break;
					case 8: kernelOSVer = 0xA060800; break;
					default:kernelOSVer = 0xA060800; break; //Last known kernel
				}
				break;
			case 11:
				switch (gDarwinMinor)
				{
					case 0: kernelOSVer = 0xA070000; break;
					case 1: kernelOSVer = 0xA070100; break;
					case 2: kernelOSVer = 0xA070200; break;
					case 3: kernelOSVer = 0xA070300; break;
					case 4:
						switch (gDarwinRev)
						{
							case 0: kernelOSVer = 0xA070400; break;
							case 1: kernelOSVer = 0xA070400; break;
							case 2: kernelOSVer = 0xA070500; break;
							default:kernelOSVer = 0xA070500; break; //Last known kernel
						}
					default:kernelOSVer = 0xA070500; break; //Last known kernel
				}
				break;
			case 12:
				switch (gDarwinMinor)
				{
					case 0: kernelOSVer = 0xA080000; break;
					case 1: kernelOSVer = 0xA080100; break;
					case 2: kernelOSVer = 0xA080200; break;
					case 3: kernelOSVer = 0xA080300; break;
					case 4: kernelOSVer = 0xA080400; break;
					case 5: kernelOSVer = 0xA080500; break; // 10.8.5
					case 6: kernelOSVer = 0xA080500; break; // 10.8.5 update
					default:kernelOSVer = 0xA080500; break; //Last known kernel
				}
				break;
			case 13:
				switch (gDarwinMinor)
				{
					case 0: kernelOSVer = 0xA090000;
					switch (gDarwinRev)
					{
						case 0: kernelOSVer = 0xA090000; break;
						case 1: kernelOSVer = 0xA090000; break; // never exist (or released)
						case 2: kernelOSVer = 0xA090100; break;
						default:kernelOSVer = 0xA090100; break; //Last known kernel
					}
						break;
					case 1: kernelOSVer = 0xA090100; break; // never exist (or released)
					case 2: kernelOSVer = 0xA090200; break;
					case 3: kernelOSVer = 0xA090300; break;
					case 4: kernelOSVer = 0xA090400; break;
					case 5: kernelOSVer = 0xA090500; break;
					default:kernelOSVer = 0xA090500; break; //Last known kernel
				}
				break;
			case 14:
				switch (gDarwinMinor)
				{
					case 0: kernelOSVer = 0xA0A0000; break; // same kernel of 10.10.1
					case 1: kernelOSVer = 0xA0A0100; break; // same kernel of 10.10
					case 2: kernelOSVer = 0xA0A0200; break;
					case 3: kernelOSVer = 0xA0A0300; break;
					case 4: kernelOSVer = 0xA0A0400; break;
					case 5: kernelOSVer = 0xA0A0500; break;
					default:kernelOSVer = 0xA0A0500; break; //Last known kernel
				}
				break;
			case 15: /* El Capitan */
				switch (gDarwinMinor)
				{
					case 0: kernelOSVer = 0xA0B0000; break;
					case 1: kernelOSVer = 0xA0B0100; break;
					case 2: kernelOSVer = 0xA0B0200; break;
					case 3: kernelOSVer = 0xA0B0300; break;
					case 4: kernelOSVer = 0xA0B0400; break;
					case 5: kernelOSVer = 0xA0B0500; break;
					case 6: kernelOSVer = 0xA0B0600; break;
					default:kernelOSVer = 0xA0B0600; break; //Last known kernel (add here updates)
				}
				break;
			case 16:  /* Sierra */
				switch (gDarwinMinor)
				{
					case 0: kernelOSVer = 0xA0C0000; break;
					case 1: kernelOSVer = 0xA0C0100; break;
					case 2: kernelOSVer = 0xA0C0200; break;
					case 3: kernelOSVer = 0xA0C0200; break;
					case 4: kernelOSVer = 0xA0C0300; break;
					case 5: kernelOSVer = 0xA0C0400; break;
					case 6: kernelOSVer = 0xA0C0500; break;
					case 7: kernelOSVer = 0xA0C0600; break;
//					case 8: kernelOSVer = 0xA0C0700; break;
					default:kernelOSVer = 0xA0C0600; break; //Last known kernel (add here updates)
				}
				break;
			case 17:  /* High Sierra */
				switch (gDarwinMinor)
				{
					case 0: kernelOSVer = 0xA0D0000; break;
					case 1: kernelOSVer = 0xA0D0100; break;
//					case 2: kernelOSVer = 0xA0D0200; break; // 17.2.0
//					case 3: kernelOSVer = 0xA0C0200; break;
//					case 4: kernelOSVer = 0xA0C0300; break;
//					case 5: kernelOSVer = 0xA0C0400; break;
//					case 6: kernelOSVer = 0xA0C0500; break;
//					case 7: kernelOSVer = 0xA0C0600; break;
//					case 8: kernelOSVer = 0xA0C0700; break;
					default:kernelOSVer = 0xA0D0100; break; //Last known kernel (add here updates)
				}
				break;
			default:
				kernelOSVer = 0xA0D0100;
				break; //Last known kernel is Sierra 10.13.1
			}
		}
		else
		{
			switch (MacOSVerCurrent)
			{
			// Snow
			case 0xA060000: gDarwinMajor = 10; gDarwinMinor =  0; gDarwinRev = 0; break; // 10.6
			case 0xA060100: gDarwinMajor = 10; gDarwinMinor =  1; gDarwinRev = 0; break; // 10.6.1
			case 0xA060200: gDarwinMajor = 10; gDarwinMinor =  2; gDarwinRev = 0; break; // 10.6.2
			case 0xA060300: gDarwinMajor = 10; gDarwinMinor =  3; gDarwinRev = 0; break; // 10.6.3
			case 0xA060400: gDarwinMajor = 10; gDarwinMinor =  4; gDarwinRev = 0; break; // 10.6.4
			case 0xA060500: gDarwinMajor = 10; gDarwinMinor =  5; gDarwinRev = 0; break; // 10.6.5
			case 0xA060600: gDarwinMajor = 10; gDarwinMinor =  6; gDarwinRev = 0; break; // 10.6.6
			case 0xA060700: gDarwinMajor = 10; gDarwinMinor =  7; gDarwinRev = 0; break; // 10.6.7
			case 0xA060800: gDarwinMajor = 10; gDarwinMinor =  8; gDarwinRev = 0; break; // 10.6.8
			// Lion
			case 0xA070000: gDarwinMajor = 11; gDarwinMinor =  0; gDarwinRev = 0; break; // 10.7
			case 0xA070100: gDarwinMajor = 11; gDarwinMinor =  1; gDarwinRev = 0; break; // 10.7.1
			case 0xA070200: gDarwinMajor = 11; gDarwinMinor =  2; gDarwinRev = 0; break; // 10.7.2
			case 0xA070300: gDarwinMajor = 11; gDarwinMinor =  3; gDarwinRev = 0; break; // 10.7.3
			case 0xA070400: gDarwinMajor = 11; gDarwinMinor =  4; gDarwinRev = 0; break; // 10.7.4
			case 0xA070500: gDarwinMajor = 11; gDarwinMinor =  4; gDarwinRev = 2; break; // 10.7.5
			// ML
			case 0xA080000: gDarwinMajor = 12; gDarwinMinor =  0; gDarwinRev = 0; break; // 10.8
			case 0xA080100: gDarwinMajor = 12; gDarwinMinor =  1; gDarwinRev = 0; break; // 10.8.1
			case 0xA080200: gDarwinMajor = 12; gDarwinMinor =  2; gDarwinRev = 0; break; // 10.8.2
			case 0xA080300: gDarwinMajor = 12; gDarwinMinor =  3; gDarwinRev = 0; break; // 10.8.3
			case 0xA080400: gDarwinMajor = 12; gDarwinMinor =  4; gDarwinRev = 0; break; // 10.8.4
			case 0xA080500: gDarwinMajor = 12; gDarwinMinor =  5; gDarwinRev = 0; break; // 10.8.5
			// Mavericks
			case 0xA090000: gDarwinMajor = 13; gDarwinMinor =  0; gDarwinRev = 0; break; // 10.9
			case 0xA090100: gDarwinMajor = 13; gDarwinMinor =  1; gDarwinRev = 0; break; // 10.9.1
			case 0xA090200: gDarwinMajor = 13; gDarwinMinor =  2; gDarwinRev = 0; break; // 10.9.2
			case 0xA090300: gDarwinMajor = 13; gDarwinMinor =  3; gDarwinRev = 0; break; // 10.9.3
			case 0xA090400: gDarwinMajor = 13; gDarwinMinor =  4; gDarwinRev = 0; break; // 10.9.4
			case 0xA090500: gDarwinMajor = 13; gDarwinMinor =  5; gDarwinRev = 0; break; // 10.9.5
			// Yosemite
			case 0xA0A0000: gDarwinMajor = 14; gDarwinMinor =  0; gDarwinRev = 0; break; // 10.10
			case 0xA0A0100: gDarwinMajor = 14; gDarwinMinor =  0; gDarwinRev = 0; break; // 10.10.1
			case 0xA0A0200: gDarwinMajor = 14; gDarwinMinor =  2; gDarwinRev = 0; break; // 10.10.2
			case 0xA0A0300: gDarwinMajor = 14; gDarwinMinor =  3; gDarwinRev = 0; break; // 10.10.3
			case 0xA0A0400: gDarwinMajor = 14; gDarwinMinor =  4; gDarwinRev = 0; break; // 10.10.4
			case 0xA0A0500: gDarwinMajor = 14; gDarwinMinor =  5; gDarwinRev = 0; break; // 10.10.5
			// El Capitan
			case 0xA0B0000: gDarwinMajor = 15; gDarwinMinor =  0; gDarwinRev = 0; break; // 10.11
			case 0xA0B0100: gDarwinMajor = 15; gDarwinMinor =  1; gDarwinRev = 0; break; // 10.11.1
			case 0xA0B0200: gDarwinMajor = 15; gDarwinMinor =  2; gDarwinRev = 0; break; // 10.11.2
			case 0xA0B0300: gDarwinMajor = 15; gDarwinMinor =  3; gDarwinRev = 0; break; // 10.11.3
			case 0xA0B0400: gDarwinMajor = 15; gDarwinMinor =  4; gDarwinRev = 0; break; // 10.11.4
			case 0xA0B0500: gDarwinMajor = 15; gDarwinMinor =  5; gDarwinRev = 0; break; // 10.11.5
			case 0xA0B0600: gDarwinMajor = 15; gDarwinMinor =  6; gDarwinRev = 0; break; // 10.11.6
			// Sierra
			case 0xA0C0000: gDarwinMajor = 16; gDarwinMinor =  0; gDarwinRev = 0; break; // 10.12
			case 0xA0C0100: gDarwinMajor = 16; gDarwinMinor =  1; gDarwinRev = 0; break; // 10.12.1
			case 0xA0C0200: gDarwinMajor = 16; gDarwinMinor =  3; gDarwinRev = 0; break; // 10.12.2
			case 0xA0C0300: gDarwinMajor = 16; gDarwinMinor =  4; gDarwinRev = 0; break; // 10.12.3
			case 0xA0C0400: gDarwinMajor = 16; gDarwinMinor =  5; gDarwinRev = 0; break; // 10.12.4
			case 0xA0C0500: gDarwinMajor = 16; gDarwinMinor =  6; gDarwinRev = 0; break; // 10.12.5
			case 0xA0C0600: gDarwinMajor = 16; gDarwinMinor =  7; gDarwinRev = 0; break; // 10.12.6
//			case 0xA0C0700: gDarwinMajor = 16; gDarwinMinor =  x; gDarwinRev = x; break; // 10.12.7
			// High Sierra
			case 0xA0D0000: gDarwinMajor = 17; gDarwinMinor =  0; gDarwinRev = 0; break; // 10.13
			case 0xA0D0100: gDarwinMajor = 17; gDarwinMinor =  1; gDarwinRev = 0; break; // 10.13.1

			// default = last known kernel
			default:        gDarwinMajor = 17; gDarwinMinor =  1; gDarwinRev = 0; break; // 10.13.1;
		}
	}

	// Notify modules that the kernel has been decompressed, thinned and is about to be decoded
	execute_hook("DecodeKernel", (void *)binary, NULL, NULL, NULL);

/* ================================================================ */

	ret = DecodeMachO(binary, rentry, raddr, rsize);
	if (ret < 0 && archCpuType == CPU_TYPE_X86_64)
	{
		archCpuType = CPU_TYPE_I386;
		ret = DecodeMachO(binary, rentry, raddr, rsize);
	}

	return ret;
}
