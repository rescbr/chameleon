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
 * structure of parameters passed to
 * the kernel by the booter.
 */
boot_args_legacy  *bootArgsLegacy= NULL;
boot_args         *bootArgs= NULL;

PrivateBootInfo_t *bootInfo  = NULL;

static char platformName[64];
static MemoryRange memoryMap[kMemoryMapCountMax];

void initKernBootStruct( void )
{        
    static int init_done = 0;
    
    if ( !init_done )
    {			
		int              convmem;                      // conventional memory
		int              extmem;                       // extended memory
		
		unsigned long    memoryMapCount = 0;
		
        bootArgs = (boot_args *)malloc(sizeof(boot_args));
        bootInfo = (PrivateBootInfo_t *)malloc(sizeof(PrivateBootInfo_t));
        if (bootArgs == NULL || bootInfo == NULL)
        {
            stop("Couldn't allocate boot info\n");
            return;
        }
        else
        {
            bzero(bootArgs, sizeof(boot_args));
            bzero(bootInfo, sizeof(PrivateBootInfo_t));
            
            // Get system memory map. Also update the size of the
            // conventional/extended memory for backwards compatibility.
            
            
            memoryMapCount =
            getMemoryMap( memoryMap, kMemoryMapCountMax,
                         (unsigned long *) &convmem,
                         (unsigned long *) &extmem );
            
            
            
            if ( memoryMapCount == 0 )
            {
                // BIOS did not provide a memory map, systems with
                // discontiguous memory or unusual memory hole locations
                // may have problems.
                
                convmem = getConventionalMemorySize();
                extmem  = getExtendedMemorySize();			
                
            }
			
            bootArgs->Video.v_display = VGA_TEXT_MODE;
            
            DT__Initialize();
            
            {
                Node *node;
                node = DT__FindNode("/", true);
                if (node == 0) {
                    stop("Couldn't create root node");
                    return;
                }
                getPlatformName(platformName, sizeof(platformName));
                
                {
                    int nameLen;
                    nameLen = strlen(platformName) + 1;
                    DT__AddProperty(node, "compatible", nameLen, platformName);
                    DT__AddProperty(node, "model", nameLen, platformName);
                }  
            }		      
            
            Node *gMemoryMapNode = DT__FindNode("/chosen/memory-map", true);
            
            set_env(envConvMem, convmem);
            set_env(envExtMem,  extmem);
            set_env(envMemoryMap, (uint32_t)memoryMap);
            set_env(envMemoryMapCnt, memoryMapCount);
			set_env(envMemoryMapNode, (uint32_t)gMemoryMapNode);

            
            init_done = 1;
        }
        
    }
    
}

/* boot args getters/setters. */

void setBootArgsVideoMode(int mode)
{
    bootArgs->Video.v_display = mode;
}
//==========================================================================
// Return the current video mode.
uint32_t getVideoMode(void)
{
    return bootArgs->Video.v_display;
}
void setBootArgsVideoStruct(Boot_Video	*Video)
{
    bootArgs->Video.v_display  = Video->v_display;
    bootArgs->Video.v_width    = Video->v_width;
    bootArgs->Video.v_height   = Video->v_height;
    bootArgs->Video.v_depth    = Video->v_depth;
    bootArgs->Video.v_rowBytes = Video->v_rowBytes;
    bootArgs->Video.v_baseAddr = Video->v_baseAddr;
    return;
}
boot_args * getBootArgs(void)
{
    return bootArgs;
}

/* Copy boot args after kernel and record address. */

void
reserveKernBootStruct(void)
{
    void *oldAddr = bootArgs;	
	
    bootArgs = (boot_args *)AllocateKernelMemory(sizeof(boot_args));
    bcopy(oldAddr, bootArgs, sizeof(boot_args));
    
}

void
reserveKernLegacyBootStruct(void)
{    
    bootArgsLegacy = (boot_args_legacy *)AllocateKernelMemory(sizeof(boot_args_legacy));
    
	bootArgsLegacy->Revision = bootArgs->Revision ;
	bootArgsLegacy->Version = bootArgs->Version   ;
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
    
	
}

void
finalizeBootStruct(void)
{    
	{
		int i;
		EfiMemoryRange *kMemoryMap = NULL;
		MemoryRange *range = NULL;
		
		uint64_t	sane_size = 0;  /* Memory size to use for defaults calculations */
		
		int memoryMapCount = (int)get_env(envMemoryMapCnt);
		
		if (memoryMapCount == 0) {
			
			// XXX could make a two-part map here
			stop("No memory map found\n");
            return;
		}
		
		
		
		// convert memory map to boot_args memory map
		kMemoryMap = (EfiMemoryRange *)AllocateKernelMemory(sizeof(EfiMemoryRange) * memoryMapCount);
		if (kMemoryMap == NULL) {
			
			stop("Unable to allocate kernel space for the memory map\n");
            return;
		}
		bootArgs->MemoryMap = (uint32_t)kMemoryMap;
		bootArgs->MemoryMapSize = sizeof(EfiMemoryRange) * memoryMapCount;
		bootArgs->MemoryMapDescriptorSize = sizeof(EfiMemoryRange);
		bootArgs->MemoryMapDescriptorVersion = 0;
		
		for (i=0; i<memoryMapCount; i++, kMemoryMap++) {
			range = &memoryMap[i];
            if (!range || !kMemoryMap) {
                stop("Error while computing kernel memory map\n");
                return;
            }
			switch(range->type) {
				case kMemoryRangeACPI:
					kMemoryMap->Type = kEfiACPIReclaimMemory;
					break;
				case kMemoryRangeNVS:
					kMemoryMap->Type = kEfiACPIMemoryNVS;
					break;
				case kMemoryRangeUsable:
					kMemoryMap->Type = kEfiConventionalMemory;
					break;
				case kMemoryRangeReserved:
				default:
					kMemoryMap->Type = kEfiReservedMemoryType;
					break;
			}
			kMemoryMap->PhysicalStart = range->base;
			kMemoryMap->VirtualStart = range->base;
			kMemoryMap->NumberOfPages = range->length >> I386_PGSHIFT;
			kMemoryMap->Attribute = 0;
			
			switch (kMemoryMap->Type) {
				case kEfiLoaderCode:
				case kEfiLoaderData:
				case kEfiBootServicesCode:
				case kEfiBootServicesData:
				case kEfiConventionalMemory:
					/*
					 * Consolidate usable memory types into one.
					 */
					sane_size += (uint64_t)(kMemoryMap->NumberOfPages << I386_PGSHIFT);
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
					sane_size += (uint64_t)(kMemoryMap->NumberOfPages << I386_PGSHIFT);
					break;
				default:
					break;
					
			}
		}
		
		if (sane_size == 0) {
			
			// I Guess that if sane_size == 0 we've got a big problem here, 
			// and it means that the memory map was not converted properly
			stop("Unable to convert memory map into proper format\n");
            return;
		}
		
#define MEG		(1024*1024)
		
		/*
		 * For user visible memory size, round up to 128 Mb - accounting for the various stolen memory
		 * not reported by EFI.
		 */
		
		sane_size = (sane_size + 128 * MEG - 1) & ~((uint64_t)(128 * MEG - 1));
		bootArgs->PhysicalMemorySize = sane_size;
		bootArgs->FSBFrequency = get_env(envFSBFreq);
        
	}	
    
    
	{
		uint32_t size;
		void *addr;
		// Flatten device tree
		DT__FlattenDeviceTree(0, &size);
		addr = (void *)AllocateKernelMemory(size);
		if (addr == 0) {
			stop("Couldn't allocate device tree\n");
            return;
		}
		
		DT__FlattenDeviceTree((void **)&addr, &size);
        if (!size) {
            stop("Couldn't get flatten device tree\n");
            return;
        }
		bootArgs->deviceTreeP = (uint32_t)addr;
		bootArgs->deviceTreeLength = size;
	}        
    
}
