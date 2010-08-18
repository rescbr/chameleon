/*
 * Supplemental ramdisk functions for the multiboot ramdisk driver.
 * Copyright 2009 Tamas Kosarszky. All rights reserved.
 *
 */

//Azi:include
#include "boot.h" // needed for kMD0ImageKey
#include "bootstruct.h"
#include "multiboot.h"
#include "ramdisk.h"

struct multiboot_info * gRAMDiskMI = NULL;

// gRAMDiskVolume holds the bvr for the mounted ramdisk image.
BVRef gRAMDiskVolume = NULL;
bool gRAMDiskBTAliased = false;
char gRAMDiskFile[512];

// Notify OS X that a ramdisk has been setup. XNU will attach this to /dev/md0
void md0Ramdisk()
{
	RAMDiskParam ramdiskPtr;
	char filename[512];
	const char* override_filename = 0;
	int fh = -1;
	int len;
	
	if(getValueForKey(kMD0ImageKey, &override_filename, &len, &bootInfo->bootConfig))
	{
		// Use user specified md0 file
		sprintf(filename, "%s", override_filename);
		fh = open(filename, 0);
		
		if(fh < 0)
		{
			sprintf(filename, "rd(0,0)/Extra/%s", override_filename);
			fh = open(filename, 0);
			
			if(fh < 0)
			{
				sprintf(filename, "/Extra/%s", override_filename);
				fh = open(filename, 0);
			}
		}		
	}
	
	if(fh < 0)
	{
		// Fallback to Postboot.img
		sprintf(filename, "rd(0,0)/Extra/Postboot.img");
		fh = open(filename, 0);
		
		if(fh < 0)
		{
			sprintf(filename, "/Extra/Postboot.img");	// Check /Extra if not in rd(0,0)
			fh = open(filename, 0);
		}
	}		
	
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

void umountRAMDisk()
{
	if (gRAMDiskMI != NULL)
	{
		// Release ramdisk BVRef and DiskBVMap.
		struct DiskBVMap *oldMap = diskResetBootVolumes(0x100);
		CacheReset();
		diskFreeMap(oldMap);
		
		// Free multiboot info and module structures.
		if ((void *)gRAMDiskMI->mi_mods_addr != NULL) free((void *)gRAMDiskMI->mi_mods_addr);
		if (gRAMDiskMI != NULL) free(gRAMDiskMI);
		
		// Reset multiboot structures.
		gMI = gRAMDiskMI = NULL;
		*gRAMDiskFile = '\0';
		
		// Release ramdisk driver hooks.
		p_get_ramdisk_info = NULL;
		p_ramdiskReadBytes = NULL;
		
		//Azi: Avoids "Failed to read boot sector from BIOS device 100h. error=1" message, when booting
		// after unmounting a image and instant reboot if the bt hook was active.
		// (test the latest again!! that's were we started)
		gRAMDiskVolume = NULL; // Reset ramdisk bvr
		printf("\nunmounting: done");
	}
}

int mountRAMDisk(const char * param)
{
	char pathname[128]; //Azi:alloc
	int fh = 0, ramDiskSize;
	int error = 0;
	
	//Azi: new ramdisk dedicated location + type just the name, without .dmg file extension (just an idea).
	sprintf(pathname, "bt(0,0)/Extra/Ramdisks/%s.dmg", param);
	
	// Get file handle for ramdisk file.
	fh = open(pathname, 0);
	if (fh != -1)
	{
		printf("\nreading ramdisk image: %s.dmg", param); //Azi:---
		
		ramDiskSize = file_size(fh);
		if (ramDiskSize > 0)
		{
			// Unmount previously mounted image if exists.
			// some unmounting glitches, when mounting without unmounting...
			umountRAMDisk();
			
			// Read new ramdisk image contents into PREBOOT_DATA area.
			if (read(fh, (char *)PREBOOT_DATA, ramDiskSize) != ramDiskSize) error = -1;
		}
		else error = -1;
		
		close(fh);
	}
	else error = -1;
	
	if (error == 0)
	{
		// Save pathname in gRAMDiskFile to display information.
		strcpy(gRAMDiskFile, pathname); //Azi: was param
		
		// Set gMI as well for the multiboot ramdisk driver hook.
		gMI = gRAMDiskMI = malloc(sizeof(multiboot_info));
		struct multiboot_module * ramdisk_module = malloc(sizeof(multiboot_module));
		
		// Fill in multiboot info and module structures.
		if (gRAMDiskMI != NULL && ramdisk_module != NULL)
		{
			gRAMDiskMI->mi_mods_count = 1;
			gRAMDiskMI->mi_mods_addr = (uint32_t)ramdisk_module;
			ramdisk_module->mm_mod_start = PREBOOT_DATA;
			ramdisk_module->mm_mod_end = PREBOOT_DATA + ramDiskSize;
			
			// Set ramdisk driver hooks.
			p_get_ramdisk_info = &multiboot_get_ramdisk_info;
			p_ramdiskReadBytes = &multibootRamdiskReadBytes;
			
			int partCount; // unused
			// Save bvr of the mounted image.
			gRAMDiskVolume = diskScanBootVolumes(0x100, &partCount);
			if (gRAMDiskVolume == NULL)
			{
				umountRAMDisk();
				printf("\nRamdisk contains no partitions.");
			}
			else
			{
				char dirSpec[128]; //Azi: this seems a nice size, still way over the needed for
				// "rd(0,0)/RAMDisk.plist", 21 char.
				
				// Reading ramdisk configuration.
				strcpy(dirSpec, RAMDISKCONFIG_FILENAME); // ramdisk.h
				
				if (loadConfigFile(dirSpec, &bootInfo->ramdiskConfig) == 0)
				{
					getBoolForKey("BTAlias", &gRAMDiskBTAliased, &bootInfo->ramdiskConfig);
				}
				else
				{
					printf("\nno ramdisk config...\n");
				}
				printf("\nmounting: done");
			}
		}
	}
	return error;
}

void setRAMDiskBTHook(bool mode)
{
	gRAMDiskBTAliased = mode;
	if (mode)
	{
		printf("\nEnabled bt(0,0) alias.");
	}
	else
	{
		printf("\nDisabled bt(0,0) alias.");
	}
}

void showInfoRAMDisk(void)
{
	int len;
	const char *val;
	
	if (gRAMDiskMI != NULL)
	{
		struct multiboot_module * ramdisk_module = (void *)gRAMDiskMI->mi_mods_addr;
		
		printf("\nfile: %s %d", gRAMDiskFile, ramdisk_module->mm_mod_end - ramdisk_module->mm_mod_start);
		printf("\nalias: %s", gRAMDiskBTAliased ? "enabled" : "disabled");
		
		// Display ramdisk information if available.
		if (getValueForKey("Info", &val, &len, &bootInfo->ramdiskConfig))
		{
			printf("\ninfo: %s", val);
		}
		else
		{
			printf("\nramdisk info not available.");
		}
	}
	else
	{
		printf("\nNo ramdisk mounted.");
	}
}

int loadPrebootRAMDisk()
{
	mountRAMDisk("bt(0,0)/Extra/Ramdisks/Preboot.dmg"); //Azi: new location for ramdisks
	
	if (gRAMDiskMI != NULL)
	{
		printf("\n");
		return 0;
	}
	else
	{
		return -1;
	}
}

void processRAMDiskCommand(char ** argPtr, const char * cmd)
{
	char * ptr = *argPtr;
	char param[1024];
	
	getNextArg(&ptr, param);
	
	if (strcmp(cmd, "m") == 0)
	{
		mountRAMDisk(param);
		printf("\n\nPress Enter to boot selected system or continue typing...\n");
		sleep(2);
	}
	else if (strcmp(cmd, "u") == 0)
	{
		umountRAMDisk();
		printf("\n\nPress Enter to boot selected system or continue typing...\n");
		sleep(2);
	}
	else if (strcmp(cmd, "e") == 0)
	{
		setRAMDiskBTHook(true);
		printf("\n\nPress Enter to boot selected system or continue typing...\n");
		sleep(2);
	}
	else if (strcmp(cmd, "d") == 0)
	{
		setRAMDiskBTHook(false);
		printf("\n\nPress Enter to boot selected system or continue typing...\n");
		sleep(2);
	}
	else if (strcmp(cmd, "i") == 0)
	{
		//Azi: "clear screen rows", etc... stuff, doesn't seem to work;
		// displayed messages just keep scrolling...
		setActiveDisplayPage(1);
		clearScreenRows(0, 24);
		setCursorPosition(0, 0, 1);
		showInfoRAMDisk();
		printf("\n\nPress Enter to boot selected system or continue typing...\n");
		sleep(2);
		setActiveDisplayPage(0);
	}
	else
	{
		setActiveDisplayPage(1);
		clearScreenRows(0, 24);
		setCursorPosition(0, 0, 1);
		printf("\nusage:\n");
		printf("\n?rd i - display ramdisk information");
		printf("\n?rd m <filename> - mount ramdisk image - no file extension required\n?rd u - unmount ramdisk image - mounting a ramdisk, unmounts a previous mounted one");
		printf("\n?rd e - enable bt(0,0) alias\n?rd d - disable bt(0,0) alias");
		printf("\n\nPress Enter to boot selected system or continue typing...\n");
		sleep(2); // these should just pause 2 sec, instead they wait for key press??
		setActiveDisplayPage(0);
	}
}
