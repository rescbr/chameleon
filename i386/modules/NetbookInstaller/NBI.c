/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include <mach-o/fat.h>
#include <libkern/OSByteOrder.h>
#include <mach/machine.h>

#include "libsaio.h"
#include "boot.h"

#include "sl.h"
#include "bootstruct.h"
#include "platform.h"
#include "xml.h"
#include "drivers.h"
#include "modules.h"

int		runNetbookInstaller = 0;

long NBI_LoadDrivers( char * dirSpec );
void NBI_md0Ramdisk();
void NBI_PreBoot_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
//void NBI_loadBootGraphics(void);
void NBI_md0Ramdisk_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);

//extern long GetDriverGbl(void);

void NetbookInstaller_start(void);
void NetbookInstaller_start(void)
{	 
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
	
	
	
	
	
	if (!runNetbookInstaller && getBoolForKey("recovery", &dummyVal, DEFAULT_BOOT_CONFIG) && dummyVal)
	{
		if(dummyVal) runNetbookInstaller = 2;
	}
	
	if(runNetbookInstaller)
	{
		
		replace_system_function("_LoadDrivers", &NBI_LoadDrivers);
		
		if(runNetbookInstaller == 1 )
		{
			if (execute_hook("isRamDiskRegistred", NULL, NULL, NULL, NULL, NULL, NULL) == EFI_SUCCESS)
			{
				replace_function_any("_md0Ramdisk", &NBI_md0Ramdisk);
			} 
			else
			{
				register_hook_callback("md0Ramdisk", NBI_md0Ramdisk_hook);
			}

		}
		
		// Force arch=i386 + -v
		safe_set_env(envarchCpuType, CPU_TYPE_I386);
		safe_set_env(envgVerboseMode, true);
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
	snprintf(filename, sizeof(filename),"%s", "Extra/NetbookInstaller.img");;
	fh = open(filename);
	
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

                AllocateMemoryRange("RAMDisk", ramdiskPtr.base, ramdiskPtr.size);

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
    if ( get_env(envgBootFileType) == kNetworkDeviceType )
    {
        if (NetLoadDrivers(dirSpec) != 0)
		{
            error("Could not load drivers from the network\n");
            return -1;
        }
    }
    else
#endif
		if ( get_env(envgBootFileType) == kBlockDeviceType )
		{
			verbose("Loading Recovery Extensions\n");
			strlcpy(dirSpecExtra, "/Extra/RecoveryExtensions/", sizeof(dirSpecExtra));
			FileLoadDrivers(dirSpecExtra, sizeof(dirSpecExtra), 0);
			
#ifdef BOOT_HELPER_SUPPORT			
			// TODO: fix this, the order does matter, and it's not correct now.
			// Also try to load Extensions from boot helper partitions.
			if (((BVRef)(uint32_t)get_env(envgBootVolume))->flags & kBVFlagBooter)
			{
				strlcpy(dirSpecExtra, "/com.apple.boot.P/System/Library/", sizeof(dirSpecExtra));
				if (FileLoadDrivers(dirSpecExtra, sizeof(dirSpecExtra), 0) != 0)
				{
					strlcpy(dirSpecExtra, "/com.apple.boot.R/System/Library/", sizeof(dirSpecExtra));
					if (FileLoadDrivers(dirSpecExtra, sizeof(dirSpecExtra), 0) != 0)
					{
						strlcpy(dirSpecExtra, "/com.apple.boot.S/System/Library/", sizeof(dirSpecExtra));
						FileLoadDrivers(dirSpecExtra, sizeof(dirSpecExtra), 0);
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
               char * ExtensionsSpec = (char*)(uint32_t)get_env(envDriverExtSpec);

				strlcpy(ExtensionsSpec, dirSpec, DEFAULT_DRIVER_SPEC_SIZE);
				strlcat(ExtensionsSpec, "System/Library/", DEFAULT_DRIVER_SPEC_SIZE);
				FileLoadDrivers(ExtensionsSpec,DEFAULT_DRIVER_SPEC_SIZE, 0);
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
