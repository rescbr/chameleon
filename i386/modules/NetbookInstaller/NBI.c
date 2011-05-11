/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include <mach-o/fat.h>
#include <libkern/OSByteOrder.h>
#include <mach/machine.h>

#include "libsaio.h"

#include "sl.h"
#include "boot.h"
#include "bootstruct.h"
#include "xml.h"
#include "drivers.h"
#include "modules.h"

int		runNetbookInstaller = 0;

long NBI_LoadDrivers( char * dirSpec );
void NBI_md0Ramdisk();
void NBI_PreBoot_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void NBI_loadBootGraphics(void);
void NBI_md0Ramdisk_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);

#ifdef NBP_SUPPORT
extern long NetLoadDrivers(char *dirSpec);
#endif
extern long FileLoadDrivers(char *dirSpec, long plugin);

extern char gMacOSVersion;


extern long LoadDriverMKext(char *fileSpec);
extern long LoadDriverPList(char *dirSpec, char *name, long bundleType);

extern long MatchLibraries( void );
extern long MatchPersonalities( void );
extern long LoadMatchedModules( void );
extern long InitDriverSupport(void);
extern char *    gExtensionsSpec;
extern char *    gDriverSpec;
extern char *    gFileSpec;
extern char *    gTempSpec;
extern char *    gFileName;

void NetbookInstaller_start()
{
	//else printf("Unable to locate Extra/SystemVersion.LastPatched.plist\n");
	
	register_hook_callback("PreBoot", &NBI_PreBoot_hook);		
	
}

void NBI_PreBoot_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	
	bool dummyVal = 0;
	config_file_t systemVersion;
	
	char valid = false;
	const char *val;
	int len;
	const char* gPrevMacOSBuildVersion;
	
	
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
		if  (getValueForKey("ProductBuildVersion", &val, &len, &systemVersion))
		{
			
			if (!loadConfigFile("Extra/SystemVersion.LastPatched.plist", &systemVersion))
			{
				if(getValueForKey("ProductBuildVersion", &gPrevMacOSBuildVersion, &len, &systemVersion))
				{
					if(strlen(val) != strlen(gPrevMacOSBuildVersion) ||
					   strcmp(val, gPrevMacOSBuildVersion) != 0
					   )
					{
						runNetbookInstaller = 1;
					}
					else 
					{
						// Only allow restore from hibernation if the system hasn't changed, probably a bad idea though
						//char* val="/private/var/vm/sleepimage";
						
						// Do this first to be sure that root volume is mounted
						//ret = GetFileInfo(0, val, &flags, &sleeptime);
						
						//printf("System version has not changed\n");
						//runNetbookInstaller = 0;
						
					}
					
				}
			}
		}
	}
	
	
	
	
	
	if (!runNetbookInstaller && getBoolForKey("recovery", &dummyVal, &bootInfo->bootConfig) && dummyVal)
	{
		if(dummyVal) runNetbookInstaller = 2;
	}
	
	if(runNetbookInstaller)
	{
		
		replace_function("_LoadDrivers", &NBI_LoadDrivers);
		
		if(runNetbookInstaller == 1 )
		{
			if (is_module_loaded("RamDiskLoader")) {
				replace_function("_md0Ramdisk", &NBI_md0Ramdisk);
			} else {
				register_hook_callback("md0Ramdisk", NBI_md0Ramdisk_hook);
			}

		}
		
		// Force arch=i386 + -v
		archCpuType = CPU_TYPE_I386;
		gVerboseMode = true;
	}
}

void NBI_md0Ramdisk_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	NBI_md0Ramdisk();	
}

typedef struct RAMDiskParam
{
	ppnum_t base;
	unsigned int size;
} RAMDiskParam;

void NBI_md0Ramdisk()
{
	RAMDiskParam ramdiskPtr;
	char filename[512];
	int fh = -1;
	
	// TODO: embed NBI.img in this file
	// If runNetbookInstaller is true, then the system has changed states, patch it 
	sprintf(filename, "%s", "Extra/NetbookInstaller.img");;
	fh = open(filename, 0);
	
	if (fh >= 0)
	{
		verbose("Enabling ramdisk %s\n", filename);
		
		ramdiskPtr.size  = file_size(fh);
		ramdiskPtr.base = AllocateKernelMemory(ramdiskPtr.size);
		
		if(ramdiskPtr.size && ramdiskPtr.base)
		{
			// Read new ramdisk image contents in kernel memory.
			if (read(fh, (char*) ramdiskPtr.base, ramdiskPtr.size) == ramdiskPtr.size)
			{
				AllocateMemoryRange("RAMDisk", ramdiskPtr.base, ramdiskPtr.size, kBootDriverTypeInvalid);
				Node* node = DT__FindNode("/chosen/memory-map", false);
				if(node != NULL)
				{
					DT__AddProperty(node, "RAMDisk", sizeof(RAMDiskParam),  (void*)&ramdiskPtr);		
				}
				else
				{
					verbose("Unable to notify Mac OS X of the ramdisk %s.\n", filename);
				}
			}
			else
			{
				verbose("Unable to read md0 image %s.\n", filename);
			}			
		}
		else
		{
			verbose("md0 image %s is empty.\n", filename);
		}
		
		close(fh);
		
	}
}


long NBI_LoadDrivers( char * dirSpec )
{
	
    char dirSpecExtra[1024];
	
    if ( InitDriverSupport() != 0 )
        return 0;
   
	int step = 0;
	execute_hook("ramDiskLoadDrivers", &step, NULL, NULL, NULL, NULL, NULL);
#ifdef NBP_SUPPORT	
    if ( gBootFileType == kNetworkDeviceType )
    {
        if (NetLoadDrivers(dirSpec) != 0) {
            error("Could not load drivers from the network\n");
            return -1;
        }
    }
    else
#endif
		if ( gBootFileType == kBlockDeviceType )
		{
			verbose("Loading Recovery Extensions\n");
			strcpy(dirSpecExtra, "/Extra/RecoveryExtensions/");
			FileLoadDrivers(dirSpecExtra, 0);
			
#ifdef BOOT_HELPER_SUPPORT			
			// TODO: fix this, the order does matter, and it's not correct now.
			// Also try to load Extensions from boot helper partitions.
			if (gBootVolume->flags & kBVFlagBooter)
			{
				strcpy(dirSpecExtra, "/com.apple.boot.P/System/Library/");
				if (FileLoadDrivers(dirSpecExtra, 0) != 0)
				{
					strcpy(dirSpecExtra, "/com.apple.boot.R/System/Library/");
					if (FileLoadDrivers(dirSpecExtra, 0) != 0)
					{
						strcpy(dirSpecExtra, "/com.apple.boot.S/System/Library/");
						FileLoadDrivers(dirSpecExtra, 0);
					}
				}
			}
#endif
			
			if (gMKextName[0] != '\0')
			{
				verbose("LoadDrivers: Loading from [%s]\n", gMKextName);
				if ( LoadDriverMKext(gMKextName) != 0 )
				{
					error("Could not load %s\n", gMKextName);
					return -1;
				}
			}
			else
			{
				strcpy(gExtensionsSpec, dirSpec);
				strcat(gExtensionsSpec, "System/Library/");
				FileLoadDrivers(gExtensionsSpec, 0);
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
