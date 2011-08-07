/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the "License").	You may not use this file
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
/* Useful types. */

#ifndef __LIBSAIO_SAIO_TYPES_H
#define __LIBSAIO_SAIO_TYPES_H

#include <sys/reboot.h>
#include <sys/types.h>
#include <sys/param.h>
#include "bios.h"

#if DEBUG
#define DEBUG_DISK(x)    printf x
#else
#define DEBUG_DISK(x)
#endif

typedef unsigned long entry_t;

typedef struct {
	unsigned int sectors:8;
	unsigned int heads:8;
	unsigned int cylinders:16;
} compact_diskinfo_t;

struct driveParameters {
	int cylinders;
	int sectors;
	int heads;
	int totalDrives;
};

struct iob {
	unsigned int   i_flgs;			/* see F_* below */
	unsigned int   i_offset;		/* seek byte offset in file */
	int			   i_filesize;		/* size of file */
	char *		   i_buf;			/* file load address */
};

#define BPS		   512				/* sector size of the device */
#define F_READ	   0x1				/* file opened for reading */
#define F_WRITE	   0x2				/* file opened for writing */
#define F_ALLOC	   0x4				/* buffer allocated */
#define F_FILE	   0x8				/* file instead of device */
#define F_NBSF	   0x10				/* no bad sector forwarding */
#define F_SSI	   0x40				/* set skip sector inhibit */
#define F_MEM	   0x80				/* memory instead of file or device */

#define BVSTRLEN 32

enum {
	kBVFlagPrimary			= 0x01,
	kBVFlagNativeBoot		= 0x02,
	kBVFlagForeignBoot		= 0x04,
	kBVFlagBootable			= 0x08,
	kBVFlagEFISystem		= 0x10,
	kBVFlagBooter			= 0x20,
	kBVFlagSystemVolume		= 0x40
};

enum {
	kBIOSDevTypeFloppy		= 0x00,
	kBIOSDevTypeHardDrive	= 0x80,
	kBIOSDevTypeNetwork		= 0xE0,
	kBIOSDevUnitMask		= 0x0F,
	kBIOSDevTypeMask		= 0xF0,
	kBIOSDevMask			= 0xFF
};

enum {
	kPartitionTypeHFS		= 0xAF,
	kPartitionTypeHPFS		= 0x07,
	kPartitionTypeFAT16		= 0x06,
	kPartitionTypeFAT32		= 0x0c,
	kPartitionTypeEXT3		= 0x83
};

//#define BIOS_DEV_TYPE(d)	((d) & kBIOSDevTypeMask)
#define BIOS_DEV_UNIT(bvr)	((bvr)->biosdev - (bvr)->type)

/*
 * KernBootStruct device types.
 */
enum {
	DEV_SD = 0,
	DEV_HD = 1,
	DEV_FD = 2,
	DEV_EN = 3
};

/*
 * min/max Macros.
 * counting and rounding Macros.
 *
 * Azi: defined on <sys/param.h>,
 *		i386/include/IOKit/IOLib.h (min/max), and others...
 *
#ifndef MIN
#define MIN(a,b) ( ((a) < (b)) ? (a) : (b) )
#endif
#ifndef MAX
#define MAX(a,b) ( ((a) > (b)) ? (a) : (b) )
#endif

#define round2(x, m)	(((x) + (m / 2)) & ~(m - 1))
#define roundup2(x, m)	(((x) + m - 1) & ~(m - 1))*/

enum {
	kNetworkDeviceType = kBIOSDevTypeNetwork,
	kBlockDeviceType   = kBIOSDevTypeHardDrive
}; //gBootFileType_t;

enum {
	kCursorTypeHidden	 = 0x0100,
	kCursorTypeUnderline = 0x0607
};

#endif /* !__LIBSAIO_SAIO_TYPES_H */
