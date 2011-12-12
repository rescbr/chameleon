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
 * Copyright 1993 NeXT, Inc.
 * All rights reserved.
 */

#include "libsaio.h"
#include "bootstruct.h"
#include "platform.h"

/*==========================================================================
 * Initialize the structure of parameters passed to
 * the kernel by the booter.
 */
boot_args_Legacy  *bootArgsLegacy;
boot_args_common  *bootArgs;
boot_args_107     *bootArgs107;
/* ... */

PrivateBootInfo_t *bootInfo;
Node              *gMemoryMapNode;

static char platformName[64];

void initKernBootStruct( void )
{        
    static int init_done = 0;

    if ( !init_done )
    {			
        bootArgs = (boot_args_common *)malloc(sizeof(boot_args_common));
        bootInfo = (PrivateBootInfo_t *)malloc(sizeof(PrivateBootInfo_t));
        if (bootArgs == 0 || bootInfo == 0)
            stop("Couldn't allocate boot info\n");

        bzero(bootArgs, sizeof(boot_args_common));
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

            bootInfo->convmem = getConventionalMemorySize();
            bootInfo->extmem  = getExtendedMemorySize();			
			
        }
#if 0		
        bootInfo->configEnd    = bootInfo->config;
#endif
        bootArgs->Video.v_display = VGA_TEXT_MODE;
        
        DT__Initialize();

		{
			Node *node;
			node = DT__FindNode("/", true);
			if (node == 0) {
				stop("Couldn't create root node");
			}
			getPlatformName(platformName);
			
			{
				int nameLen;
				nameLen = strlen(platformName) + 1;
				DT__AddProperty(node, "compatible", nameLen, platformName);
				DT__AddProperty(node, "model", nameLen, platformName);
			}  
		}		      

        gMemoryMapNode = DT__FindNode("/chosen/memory-map", true);
        
        init_done = 1;
    }

}

#define AllocateKernelMemoryForBootArgs(Ver)                           \
{ \
	bootArgs##Ver = (boot_args_##Ver *)AllocateKernelMemory(sizeof(boot_args_##Ver));\
}

#define CopyCommonBootArgsHeader(Ver)                           \
{ \
	bootArgs##Ver->Revision = bootArgs->Header.Revision ;\
	bootArgs##Ver->Version = bootArgs->Header.Version   ;\
}

// For 10.6, 10.5 and 10.4 please use :Legacy:, for 10.7 use :107:
#define CopyCommonBootArgs(Ver)                           \
{ \
	bcopy(bootArgs->CommandLine, bootArgs##Ver->CommandLine, BOOT_LINE_LENGTH);\
	bootArgs##Ver->MemoryMap = bootArgs->MemoryMap ;\
	bootArgs##Ver->MemoryMapSize = bootArgs->MemoryMapSize ;\
	bootArgs##Ver->MemoryMapDescriptorSize = bootArgs->MemoryMapDescriptorSize ;\
	bootArgs##Ver->MemoryMapDescriptorVersion = bootArgs->MemoryMapDescriptorVersion ;\
	bootArgs##Ver->Video = bootArgs->Video ;\
	bootArgs##Ver->deviceTreeP = bootArgs->deviceTreeP ;\
	bootArgs##Ver->deviceTreeLength = bootArgs->deviceTreeLength ;\
	bootArgs##Ver->kaddr = bootArgs->kaddr ;\
	bootArgs##Ver->ksize = bootArgs->ksize ;\
	bootArgs##Ver->efiRuntimeServicesPageStart = bootArgs->efiRuntimeServicesPageStart ;\
	bootArgs##Ver->efiRuntimeServicesPageCount = bootArgs->efiRuntimeServicesPageCount ;\
	bootArgs##Ver->efiSystemTable = bootArgs->efiSystemTable ;\
	bootArgs##Ver->efiMode = bootArgs->efiMode ;\
	bootArgs##Ver->performanceDataStart = bootArgs->performanceDataStart ;\
	bootArgs##Ver->performanceDataSize = bootArgs->performanceDataSize ;\
	bootArgs##Ver->efiRuntimeServicesVirtualPageStart = bootArgs->efiRuntimeServicesVirtualPageStart ;\
}

#define init_boot_args(Ver)                           \
{ \
	AllocateKernelMemoryForBootArgs(Ver);\
	CopyCommonBootArgsHeader(Ver);\
	CopyCommonBootArgs(Ver);\
}

/* Copy boot args after kernel and record address. */

void
reserveKern107BootStruct(void)
{
    //bootArgs107 = (boot_args_107 *)AllocateKernelMemory(sizeof(boot_args_107));
	
	/* Common Darwin boot arguments */
	/*
	bootArgs107->Revision = bootArgs->Header.Revision ;
	bootArgs107->Version = bootArgs->Header.Version   ;
	bcopy(bootArgs->CommandLine, bootArgs107->CommandLine, BOOT_LINE_LENGTH);
	bootArgs107->MemoryMap = bootArgs->MemoryMap ;
	bootArgs107->MemoryMapSize = bootArgs->MemoryMapSize ;
	bootArgs107->MemoryMapDescriptorSize = bootArgs->MemoryMapDescriptorSize ;
	bootArgs107->MemoryMapDescriptorVersion = bootArgs->MemoryMapDescriptorVersion ;
	bootArgs107->Video = bootArgs->Video ;
	bootArgs107->deviceTreeP = bootArgs->deviceTreeP ;
	bootArgs107->deviceTreeLength = bootArgs->deviceTreeLength ;
	bootArgs107->kaddr = bootArgs->kaddr ;
	bootArgs107->ksize = bootArgs->ksize ;
	bootArgs107->efiRuntimeServicesPageStart = bootArgs->efiRuntimeServicesPageStart ;
	bootArgs107->efiRuntimeServicesPageCount = bootArgs->efiRuntimeServicesPageCount ;
	bootArgs107->efiSystemTable = bootArgs->efiSystemTable ;
	bootArgs107->efiMode = bootArgs->efiMode ;
	bootArgs107->performanceDataStart = bootArgs->performanceDataStart ;
	bootArgs107->performanceDataSize = bootArgs->performanceDataSize ;
	bootArgs107->efiRuntimeServicesVirtualPageStart = bootArgs->efiRuntimeServicesVirtualPageStart ;
	*/
	
	init_boot_args(107);
	
	/* Darwin 10.7 specific boot arguments */
	bootArgs107->keyStoreDataStart = bootArgs->keyStoreDataStart ;
	bootArgs107->keyStoreDataSize = bootArgs->keyStoreDataSize ;
	bootArgs107->bootMemStart = bootArgs->bootMemStart ;
	bootArgs107->bootMemSize = bootArgs->bootMemSize ;
	bootArgs107->PhysicalMemorySize = bootArgs->PhysicalMemorySize ;
	bootArgs107->FSBFrequency = bootArgs->FSBFrequency ;	
	bootArgs107->debugMode = bootArgs->debugMode; 

}

void
reserveKernLegacyBootStruct(void)
{    
    //bootArgsLegacy = (boot_args_legacy *)AllocateKernelMemory(sizeof(boot_args_legacy));
  /*  
	bootArgsLegacy->Revision = bootArgs->Header.Revision ;
	bootArgsLegacy->Version = bootArgs->Header.Version   ;
	bcopy(bootArgs->CommandLine, bootArgsLegacy->CommandLine, BOOT_LINE_LENGTH);
	bootArgsLegacy->MemoryMap = bootArgs->MemoryMap ;
	bootArgsLegacy->MemoryMapSize = bootArgs->MemoryMapSize ;
	bootArgsLegacy->MemoryMapDescriptorSize = bootArgs->MemoryMapDescriptorSize ;
	bootArgsLegacy->MemoryMapDescriptorVersion = bootArgs->MemoryMapDescriptorVersion ;
	bootArgsLegacy->Video = bootArgs->Video ;
	bootArgsLegacy->deviceTreeP = bootArgs->deviceTreeP ;
	bootArgsLegacy->deviceTreeLength = bootArgs->deviceTreeLength ;
	bootArgsLegacy->kaddr = bootArgs->kaddr ;
	bootArgsLegacy->ksize = bootArgs->ksize ;
	bootArgsLegacy->efiRuntimeServicesPageStart = bootArgs->efiRuntimeServicesPageStart ;
	bootArgsLegacy->efiRuntimeServicesPageCount = bootArgs->efiRuntimeServicesPageCount ;
	bootArgsLegacy->efiSystemTable = bootArgs->efiSystemTable ;
	bootArgsLegacy->efiMode = bootArgs->efiMode ;
	bootArgsLegacy->performanceDataStart = bootArgs->performanceDataStart ;
	bootArgsLegacy->performanceDataSize = bootArgs->performanceDataSize ;
	bootArgsLegacy->efiRuntimeServicesVirtualPageStart = bootArgs->efiRuntimeServicesVirtualPageStart ;
*/	
	
	init_boot_args(Legacy);
}

void
finalizeBootStruct(void)
{    
	{
		int i;
		EfiMemoryRange *memoryMap;
		MemoryRange *range;
		uint64_t	sane_size = 0;  /* Memory size to use for defaults calculations */
		
		int memoryMapCount = bootInfo->memoryMapCount;
		
		if (memoryMapCount == 0) {
			
			// XXX could make a two-part map here
			stop("No memory map found\n");
		}
		
		
		
		// convert memory map to boot_args memory map
		memoryMap = (EfiMemoryRange *)AllocateKernelMemory(sizeof(EfiMemoryRange) * memoryMapCount);
		if (memoryMap == NULL) {
			
			stop("Unable to allocate kernel space for the memory map\n");
		}
		bootArgs->MemoryMap = (uint32_t)memoryMap;
		bootArgs->MemoryMapSize = sizeof(EfiMemoryRange) * memoryMapCount;
		bootArgs->MemoryMapDescriptorSize = sizeof(EfiMemoryRange);
		bootArgs->MemoryMapDescriptorVersion = 0;
		
		for (i=0; i<memoryMapCount; i++, memoryMap++) {
			range = &bootInfo->memoryMap[i];
			switch(range->type) {
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
			memoryMap->PhysicalStart = range->base;
			memoryMap->VirtualStart = range->base;
			memoryMap->NumberOfPages = range->length >> I386_PGSHIFT;
			memoryMap->Attribute = 0;
			
			switch (memoryMap->Type) {
				case kEfiLoaderCode:
				case kEfiLoaderData:
				case kEfiBootServicesCode:
				case kEfiBootServicesData:
				case kEfiConventionalMemory:
					/*
					 * Consolidate usable memory types into one.
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
					 * the OS to use
					 */
					sane_size += (uint64_t)(memoryMap->NumberOfPages << I386_PGSHIFT);
					break;
				default:
					break;
					
			}
		}
		
		if (sane_size == 0) {
			
			// I Guess that if sane_size == 0 we've got a big problem here, 
			// and it means that the memory map was not converted properly
			stop("Unable to convert memory map into proper format\n");
		}
		
#define MEG		(1024*1024)
		
		/*
		 * For user visible memory size, round up to 128 Mb - accounting for the various stolen memory
		 * not reported by EFI.
		 */
		
		sane_size = (sane_size + 128 * MEG - 1) & ~((uint64_t)(128 * MEG - 1));
		bootArgs->PhysicalMemorySize = sane_size;
		bootArgs->FSBFrequency = Platform->CPU.FSBFrequency;
	
	}
	
    // add PCI info somehow into device tree
    // XXX
		
	{
		uint32_t size;
		void *addr;
		// Flatten device tree
		DT__FlattenDeviceTree(0, &size);
		addr = (void *)AllocateKernelMemory(size);
		if (addr == 0) {
			stop("Couldn't allocate device tree\n");
		}
		
		DT__FlattenDeviceTree((void **)&addr, &size);
		bootArgs->deviceTreeP = (uint32_t)addr;
		bootArgs->deviceTreeLength = size;
	}        
    
}
