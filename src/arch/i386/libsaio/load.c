/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 2.0 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 *  load.c - Functions for decoding a Mach-o Kernel.
 *
 *  Copyright (c) 1998-2003 Apple Computer, Inc.
 *
 */

#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach/machine/thread_status.h>


cpu_type_t archCpuType=CPU_TYPE_I386;

// Public Functions

long ThinFatFile(void **binary, unsigned long *length)
{
  unsigned long nfat, swapped, size = 0;
  struct fat_header *fhp = (struct fat_header *)*binary;
  struct fat_arch   *fap =
    (struct fat_arch *)((unsigned long)*binary + sizeof(struct fat_header));
  cpu_type_t fapcputype;
  uint32_t fapoffset;
  uint32_t fapsize;	
  
  if (fhp->magic == FAT_MAGIC) {
    nfat = fhp->nfat_arch;
    swapped = 0;
  } else if (fhp->magic == FAT_CIGAM) {
    nfat = OSSwapInt32(fhp->nfat_arch);
    swapped = 1;
  } else {
    return -1;
  }
  
  for (; nfat > 0; nfat--, fap++) {
    if (swapped) {
      fapcputype = OSSwapInt32(fap->cputype);
      fapoffset = OSSwapInt32(fap->offset);
      fapsize = OSSwapInt32(fap->size);
    }
	else
	{
		fapcputype = fap->cputype;
		fapoffset = fap->offset;
		fapsize = fap->size;
	}
    
    if (fapcputype == archCpuType) {
      *binary = (void *) ((unsigned long)*binary + fapoffset);
	  size = fapsize;
	  break;
    }
  }
  
  if (length != 0) *length = size;
  
  return 0;
}