/*
 * Supplemental ramdisk functions for the multiboot ramdisk driver
 * Copyright 2009 Tamas Kosarszky. All rights reserved.
 *
 */

#ifndef __BOOT_RAMDISK_H
#define __BOOT_RAMDISK_H

#include "boot.h"
#include "drivers.h"
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

//
extern int (*p_get_ramdisk_info)(int biosdev, struct driveInfo *dip);
extern long (*LoadExtraDrivers_p)(FileLoadDrivers_t FileLoadDrivers_p);
extern int (*p_ramdiskReadBytes)( int biosdev, unsigned int blkno,
								 unsigned int byteoff,
								 unsigned int byteCount, void * buffer );
extern BVRef gRAMDiskVolume;
extern bool gRAMDiskBTAliased;

extern void setRAMDiskBTHook(bool mode);
extern int mountRAMDisk(const char * param);
extern void processRAMDiskCommand(char ** argPtr, const char * cmd);
extern int loadPrebootRAMDisk(void);
extern void showInfoRAMDisk(void);
extern void umountRAMDisk(void);
#endif /* !__BOOT_RAMDISK_H */
