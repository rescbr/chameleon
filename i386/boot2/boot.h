/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the "License").  You may not use this file
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
/*
 * Copyright 1994 NeXT Computer, Inc.
 * All rights reserved.
 */

#ifndef __BOOT2_BOOT_H
#define __BOOT2_BOOT_H

#include "libsaio.h"

void boot(int biosdev);

enum {
	kBackspaceKey	= 0x08,
	kTabKey			= 0x09,
	kReturnKey		= 0x0d,
	kEscapeKey		= 0x1b,
	kUpArrowkey		= 0x4800, 
	kDownArrowkey	= 0x5000,
	kASCIIKeyMask	= 0x7f,
	kF5Key			= 0x3f00,
	kF10Key			= 0x4400
};



/*
 * Flags to the booter or kernel
 */
#define kVerboseModeFlag	"-v"
#define kSafeModeFlag		"-x"
#define kIgnoreCachesFlag	"-f"
#define kSingleUserModeFlag	"-s"
#define kIgnorePrelinkKern  "-F"
#define kIgnoreBootFileFlag	"-B"

/*
 * Booter behavior control
 */
#define kBootTimeout         -1
#define kCDBootTimeout       8

/*
 * A global set by boot() to record the device that the booter
 * was loaded from.
 */
#define Cache_len_name 512

/*
 * Boot Modes
 */
enum {
    kBootModeNormal = 0,
    kBootModeSafe   = 1,
    kBootModeSecure = 2,
    kBootModeQuiet  = 4
};

extern void initialize_runtime(void);
extern void common_boot(int biosdev);
extern BVRef getBvChain(void);

/*
 * drivers.c
 */
extern long LoadDrivers(char * dirSpec);
extern long DecodeKernel(void *binary, entry_t *rentry, char **raddr, int *rsize);

typedef long (*FileLoadDrivers_t)(char *dirSpec, long plugin);
/*!
 Hookable function pointer called during the driver loading phase that
 allows other code to cause additional drivers to be loaded.
 */
//extern struct multiboot_info *gMI;


struct compressed_kernel_header {
    u_int32_t signature;
    u_int32_t compress_type;
    u_int32_t adler32;
    u_int32_t uncompressed_size;
    u_int32_t compressed_size;
    u_int32_t reserved[11];
    char      platform_name[PLATFORM_NAME_LEN];
    char      root_path[ROOT_PATH_LEN];
    u_int8_t  data[0];
};
typedef struct compressed_kernel_header compressed_kernel_header;

/*
 * prompt.c
 */
extern void InitBootPrompt(void);

#endif /* !__BOOT2_BOOT_H */
