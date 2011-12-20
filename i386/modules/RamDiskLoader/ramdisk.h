/*
 * Supplemental ramdisk functions for the multiboot ramdisk driver
 * Copyright 2009 Tamas Kosarszky. All rights reserved.
 *
 */

#ifndef __BOOT_RAMDISK_H
#define __BOOT_RAMDISK_H

#include "drivers.h"
#include "boot.h"
//#include "mboot.h"

#define RAMDISKCONFIG_FILENAME "rd(0,0)/RAMDisk.plist"
#define kMD0Image			   "md0"				/* ramdisk.h */

//#define kPostbootRamdisk
extern void md0Ramdisk();

typedef struct RAMDiskParam
{
	ppnum_t base;
	unsigned int size;
} RAMDiskParam;

/* mboot.c */
extern struct multiboot_info *gMI;

//
extern int (*p_get_ramdisk_info)(int biosdev, struct driveInfo *dip);
extern long (*LoadExtraDrivers_p)(FileLoadDrivers_t FileLoadDrivers_p);
extern int (*p_ramdiskReadBytes)( int biosdev, unsigned int blkno,
								 unsigned int byteoff,
								 unsigned int byteCount, void * buffer );
extern BVRef gRAMDiskVolume;
extern bool gRAMDiskBTAliased;

extern long FileLoadDrivers(char *dirSpec, long plugin);

extern void setRAMDiskBTHook(bool mode);
extern int mountRAMDisk(const char * param);
extern void processRAMDiskCommand(char ** argPtr, const char * cmd);
extern int loadPrebootRAMDisk();

#endif /* !__BOOT_RAMDISK_H */
