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
 *  drivers.h - Driver Loading Functions.
 *
 *  Copyright (c) 2000 Apple Computer, Inc.
 *
 *  DRI: Josh de Cesare
 */
#ifndef _BOOT2_DRIVERS_H
#define _BOOT2_DRIVERS_H

#include <mach-o/fat.h>
#include <libkern/OSByteOrder.h>
#include <mach/machine.h>

#include "sl.h"
#include "boot.h"
#include "bootstruct.h"
#include "xml.h"
#include "ramdisk.h"
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


#ifndef OPTION_ROM
extern long (*LoadExtraDrivers_p)(FileLoadDrivers_t FileLoadDrivers_p);
#endif

long LoadDrivers( char * dirSpec );
long DecodeKernel(void *binary, entry_t *rentry, char **raddr, int *rsize);

#endif /* _BOOT2_DRIVERS_H */