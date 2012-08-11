/*
 *  RamDiskLoader.c
 *  Chameleon
 *
 *  Created by cparm on 05/12/10. <armelcadetpetit@gmail.com>
 *  Copyright 2010,2012. All rights reserved.
 *
 */

#include "libsaio.h"
#include "bootstruct.h"
#include "modules.h"
#include "ramdisk.h"
#include "drivers.h"
#include "disk.h"
#include "platform.h"


void loadPrebootRAMDisk_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void md0Ramdisk_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void processRAMDiskCommand_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void ramDiskLoadDrivers_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void newRamDisk_BVR_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void p_get_ramdisk_info_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void p_ramdiskReadBytes_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void is_Ram_Disk_Registred_Hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);


enum {
	kPseudoBIOSDevRAMDisk = 0x100,
	kPseudoBIOSDevBooter = 0x101
};

void loadPrebootRAMDisk_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	loadPrebootRAMDisk();	
}

void md0Ramdisk_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	md0Ramdisk();	
}

void processRAMDiskCommand_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	char * argPtr = (char *)arg1;
	const char*bp = (const char *)arg2;
	processRAMDiskCommand(&argPtr, bp);
}

void ramDiskLoadDrivers_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	char dirSpecExtra[1024];
	int step = *(int*)arg1;	
	switch (step) {
		case 0:
			if (LoadExtraDrivers_p != NULL)
			{
				(*LoadExtraDrivers_p)(&FileLoadDrivers);
			}
			break;
		case 1:
			// First try a specfic OS version folder ie 10.5
			snprintf(dirSpecExtra, sizeof(dirSpecExtra),"rd(0,0)/Extra/%s/", (char*)((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion);
			if (FileLoadDrivers(dirSpecExtra, sizeof(dirSpecExtra),0) != 0)
			{	
				// Next we'll try the base
				strlcpy(dirSpecExtra, "rd(0,0)/Extra/",sizeof(dirSpecExtra));
				FileLoadDrivers(dirSpecExtra,sizeof(dirSpecExtra), 0);
			}
			break;
		case 2:
			// First try a specfic OS version folder ie 10.5
			snprintf(dirSpecExtra, sizeof(dirSpecExtra),"bt(0,0)/Extra/%s/", (char*)((BVRef)(uint32_t)get_env(envgBootVolume))->OSVersion);
			if (FileLoadDrivers(dirSpecExtra,sizeof(dirSpecExtra), 0) != 0)
			{	
				// Next we'll try the base
				strlcpy(dirSpecExtra, "bt(0,0)/Extra/",sizeof(dirSpecExtra));
				FileLoadDrivers(dirSpecExtra,sizeof(dirSpecExtra), 0);
			}
			break;
		default:
			break;
	}	
}

void newRamDisk_BVR_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	int biosdev = *(int*)arg1;
	BVRef *bvr1 = (BVRef*)arg2;
	if (biosdev == kPseudoBIOSDevRAMDisk)
	{		
		if (gRAMDiskVolume)
		    *bvr1 = gRAMDiskVolume;
	} 
	else if (biosdev == kPseudoBIOSDevBooter)
	{
		if (gRAMDiskVolume != NULL && gRAMDiskBTAliased)
			*bvr1 = gRAMDiskVolume;
	}
}

void p_get_ramdisk_info_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	int cc;
	int biosdev = *(int *)arg1;
	struct driveInfo *dip = (struct driveInfo *)arg2;	
	int *ret = (int *)arg3;
	if(p_get_ramdisk_info != NULL)
		cc = (*p_get_ramdisk_info)(biosdev, dip);
	else
		cc = -1;
	if(cc < 0)
	{
		dip->valid = 0;
		*ret = -1;
	}	
}

void p_ramdiskReadBytes_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	int biosdev					= *(int*)arg1;
	unsigned long long blkno	= *(unsigned long long*)arg2;
	unsigned int byteoff		= *(unsigned int*)arg3;
	unsigned int byteCount		= *(unsigned int*)arg4;
	void * buffer				= (void*)arg5;
	int *ret					= (int*)arg6;
	
	if(p_ramdiskReadBytes != NULL && biosdev >= 0x100)
        *ret = (*p_ramdiskReadBytes)(biosdev, blkno, byteoff, byteCount, buffer);	
}

void is_Ram_Disk_Registred_Hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6){}

void RamDiskLoader_start(void);
void RamDiskLoader_start(void)
{
    register_hook_callback("loadPrebootRAMDisk", &loadPrebootRAMDisk_hook);
    register_hook_callback("md0Ramdisk", &md0Ramdisk_hook);
    register_hook_callback("processRAMDiskCommand", &processRAMDiskCommand_hook);
    register_hook_callback("ramDiskLoadDrivers", &ramDiskLoadDrivers_hook);	
    register_hook_callback("newRamDisk_BVR", &newRamDisk_BVR_hook);
    register_hook_callback("p_get_ramdisk_info", &p_get_ramdisk_info_hook);
    register_hook_callback("p_ramdiskReadBytes", &p_ramdiskReadBytes_hook);
    register_hook_callback("isRamDiskRegistred", &is_Ram_Disk_Registred_Hook);

}