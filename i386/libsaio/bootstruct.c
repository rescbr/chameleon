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
/*
 * Copyright 1993 NeXT, Inc.
 * All rights reserved.
 */

#include "config.h"
#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"

#if DEBUG_BOOTSTRUCT
	#define DBG(x...)	printf(x)
#else
	#define DBG(x...)	msglog(x)
#endif

#define MEG		(1024*1024)

/*==========================================================================
 * Initialize the structure of parameters passed to
 * the kernel by the booter.
 */

boot_args		*bootArgs	= NULL;
boot_args_legacy	*bootArgsLegacy	= NULL;

PrivateBootInfo_t	*bootInfo	= NULL;
Node			*gMemoryMapNode;

static char platformName[64];

void initKernBootStruct( void )
{
	Node *node;
	int nameLen;

	static int init_done = 0;

	if ( !init_done )
	{
		bootArgs = (boot_args *)malloc(sizeof(boot_args));
		bootArgsLegacy = (boot_args_legacy *)malloc(sizeof(boot_args_legacy));
		bootInfo = (PrivateBootInfo_t *)malloc(sizeof(PrivateBootInfo_t));

		if (bootArgs == 0 || bootArgsLegacy == 0 || bootInfo == 0)
		{
			stop("Couldn't allocate boot info\n");
		}

		bzero(bootArgs, sizeof(boot_args));
		bzero(bootArgsLegacy, sizeof(boot_args_legacy));
		bzero(bootInfo, sizeof(PrivateBootInfo_t));

		// Get system memory map. Also update the size of the
		// conventional/extended memory for backwards compatibility.

		bootInfo->memoryMapCount =
			getMemoryMap( bootInfo->memoryMap, kMemoryMapCountMax,
						  (unsigned long *) &bootInfo->convmem,
						  (unsigned long *) &bootInfo->extmem );

		if ( bootInfo->memoryMapCount == 0 )
		{
			// BIOS did not provide a memory map, systems with
			// discontiguous memory or unusual memory hole locations
			// may have problems.

			bootInfo->convmem	= getConventionalMemorySize();
			bootInfo->extmem	= getExtendedMemorySize();
		}

		bootInfo->configEnd		= bootInfo->config;
		bootArgs->Video.v_display	= VGA_TEXT_MODE;

		DT__Initialize();

		node = DT__FindNode("/", true);
		if (node == 0)
		{
			stop("Couldn't create root node");
		}

		getPlatformName(platformName, sizeof(platformName));

		nameLen = strlen(platformName) + 1;
		DT__AddProperty(node, "compatible", nameLen, platformName);
		DT__AddProperty(node, "model", nameLen, platformName);

		gMemoryMapNode = DT__FindNode("/chosen/memory-map", true);

		bootArgs->Version  = kBootArgsVersion;
		bootArgs->Revision = kBootArgsRevision;

		bootArgsLegacy->Version  = kBootArgsLegacyVersion;
		bootArgsLegacy->Revision = kBootArgsLegacyRevision;

		init_done = 1;
	}
}

/* Copy boot args after kernel and record address. */

void reserveKernBootStruct(void)
{
	if ( MacOSVerCurrent < MacOSVer2Int("10.7") )
	{
		// for 10.4 10.5 10.6
		void *oldAddr = bootArgsLegacy;
		bootArgsLegacy = (boot_args_legacy *)AllocateKernelMemory(sizeof(boot_args_legacy));
		bcopy(oldAddr, bootArgsLegacy, sizeof(boot_args_legacy));
	}
	else
	{
		// for 10.7 10.8 10.9 10.10 10.11 10.12 10.13
		void *oldAddr = bootArgs;
		bootArgs = (boot_args *)AllocateKernelMemory(sizeof(boot_args));
		bcopy(oldAddr, bootArgs, sizeof(boot_args));
	}

	// 10.12 and 10.13 new bootArgs?
}

//==============================================================================

void finalizeBootStruct(void)
{
	uint32_t size;
	void *addr;
	int i;

	/* Memory size to use for defaults calculations (cparm) */
	uint64_t	sane_size = 0;

	EfiMemoryRange	*memoryMap	= NULL;
	MemoryRange	*range		= NULL;
	int memoryMapCount = bootInfo->memoryMapCount;

	if (memoryMapCount == 0)
	{
		// XXX could make a two-part map here
		stop("No memory map found!\n");
		return;
	}

	// convert memory map to boot_args memory map
	memoryMap = (EfiMemoryRange *)AllocateKernelMemory(sizeof(EfiMemoryRange) * memoryMapCount);
	if (memoryMap == NULL)
	{
		stop("Unable to allocate kernel space for the memory map!\n");
		return;
	}

	bootArgs->MemoryMap			= (uint32_t)memoryMap;
	bootArgs->MemoryMapSize			= sizeof(EfiMemoryRange) * memoryMapCount;
	bootArgs->MemoryMapDescriptorSize	= sizeof(EfiMemoryRange);
	bootArgs->MemoryMapDescriptorVersion	= 0;

	for (i = 0; i < memoryMapCount; i++, memoryMap++)
	{
		range = &bootInfo->memoryMap[i];

		if (!range || !memoryMap)
		{
			stop("Error while computing kernel memory map\n");
			return;
		}

		switch(range->type)
		{
			case kMemoryRangeACPI:
				memoryMap->Type = kEfiACPIReclaimMemory;
				break;

			case kMemoryRangeNVS:
				memoryMap->Type = kEfiACPIMemoryNVS;
				break;

			case kMemoryRangeUsable:
				memoryMap->Type = kEfiConventionalMemory;
				break;

			case kMemoryRangeReserved:

			default:
				memoryMap->Type = kEfiReservedMemoryType;
				break;
		}

		memoryMap->PhysicalStart	= range->base;
		memoryMap->VirtualStart		= range->base;
		memoryMap->NumberOfPages	= range->length >> I386_PGSHIFT;
		memoryMap->Attribute		= 0;

		switch (memoryMap->Type)
		{
			case kEfiLoaderCode:
			case kEfiLoaderData:
			case kEfiBootServicesCode:
			case kEfiBootServicesData:
			case kEfiConventionalMemory:
				/*
				 * Consolidate usable memory types into one (cparm).
				 */
				sane_size += (uint64_t)(memoryMap->NumberOfPages << I386_PGSHIFT);
				break;

			case kEfiRuntimeServicesCode:
			case kEfiRuntimeServicesData:
			case kEfiACPIReclaimMemory:
			case kEfiACPIMemoryNVS:
			case kEfiPalCode:
				/*
				 * sane_size should reflect the total amount of physical ram
				 * in the system, not just the amount that is available for
				 * the OS to use (cparm)
				 */
				sane_size += (uint64_t)(memoryMap->NumberOfPages << I386_PGSHIFT);
				break;
			default:
				break;
		}

		if (sane_size == 0)
		{

			// I Guess that if sane_size == 0 we've got a big problem here,
			// and it means that the memory map was not converted properly (cparm)
			stop("Unable to convert memory map into proper format\n");
			return;
		}

		/*
		 * For user visible memory size, round up to 128 Mb - accounting for the various stolen memory
		 * not reported by EFI. (cparm)
		 */
#if DEBUG_BOOTSTRUCT
		sane_size = (sane_size + 128 * MEG - 1) & ~((uint64_t)(128 * MEG - 1));
		DBG( "Total amount of physical RAM in the system : %d\n", ((sane_size + 128 * MEG - 1) & ~((uint64_t)(128 * MEG - 1))) );
#else
		DBG( "Total amount of RAM in the system : %d\n", sane_size );
#endif
		bootArgs->PhysicalMemorySize = sane_size;
	}

	// copy bootFile into device tree
	// XXX

	// add PCI info somehow into device tree
	// XXX

	// Flatten device tree
	DT__FlattenDeviceTree(0, &size);
	addr = (void *)AllocateKernelMemory(size);
	if (addr == 0)
	{
		stop("Couldn't allocate device tree\n");
		return;
	}

	DT__FlattenDeviceTree((void **)&addr, &size);
	if (!size)
	{
		stop("Couldn't get flatten device tree\n");
		return;
	}
	bootArgs->deviceTreeP = (uint32_t)addr;
	bootArgs->deviceTreeLength = size;

	// Copy BootArgs values to older structure

	memcpy(&bootArgsLegacy->CommandLine, &bootArgs->CommandLine, BOOT_LINE_LENGTH);
	memcpy(&bootArgsLegacy->Video, &bootArgs->Video, sizeof(Boot_Video));

	bootArgsLegacy->MemoryMap = bootArgs->MemoryMap;
	bootArgsLegacy->MemoryMapSize = bootArgs->MemoryMapSize;
	bootArgsLegacy->MemoryMapDescriptorSize = bootArgs->MemoryMapDescriptorSize;
	bootArgsLegacy->MemoryMapDescriptorVersion = bootArgs->MemoryMapDescriptorVersion;

	bootArgsLegacy->deviceTreeP = bootArgs->deviceTreeP;
	bootArgsLegacy->deviceTreeLength = bootArgs->deviceTreeLength;

	bootArgsLegacy->kaddr = bootArgs->kaddr;
	bootArgsLegacy->ksize = bootArgs->ksize;

	bootArgsLegacy->efiRuntimeServicesPageStart = bootArgs->efiRuntimeServicesPageStart;
	bootArgsLegacy->efiRuntimeServicesPageCount = bootArgs->efiRuntimeServicesPageCount;
	bootArgsLegacy->efiSystemTable = bootArgs->efiSystemTable;

	bootArgsLegacy->efiMode = bootArgs->efiMode;

	bootArgsLegacy->performanceDataStart = bootArgs->performanceDataStart;
	bootArgsLegacy->performanceDataSize = bootArgs->performanceDataSize;
	bootArgsLegacy->efiRuntimeServicesVirtualPageStart = bootArgs->efiRuntimeServicesVirtualPageStart;
}
