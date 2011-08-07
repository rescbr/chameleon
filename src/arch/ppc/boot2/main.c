/*
 * Copyright (c) 2000-2007 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

/*
 *  main.c - Main functions for BootX.
 *
 *  Copyright (c) 1998-2004 Apple Computer, Inc.
 *
 *  DRI: Josh de Cesare
 */


#include <sl.h>
#include <stdio.h>
#include <string.h>

extern void	malloc_init(char * start, int size, int nodes, void (*malloc_error)(char *, size_t, const char *, int));
extern void init_module_system();
extern int	execute_hook(const char* name, void*, void*, void*, void*);

static void Start(void *unused1, void *unused2, ClientInterfacePtr ciPtr);
static void Main(ClientInterfacePtr ciPtr);
static long InitEverything(ClientInterfacePtr ciPtr);
static long InitMemoryMap(void);
static long GetOFVersion(void);
static void malloc_error(char *addr, size_t size, const char *file, int line);

const unsigned long StartTVector[2] = {(unsigned long)Start, 0};


char gStackBaseAddr[0x8000];

long gOFVersion = 0;

char *gKeyMap;

long gRootAddrCells;
long gRootSizeCells;

CICell gChosenPH;
CICell gMemoryMapPH;
CICell gStdOutPH;

CICell gMMUIH;
CICell gMemoryIH;
CICell gStdOutIH;
CICell gKeyboardIH;


// Private Functions

static void malloc_error(char *addr, size_t size, const char *file, int line)
{
	printf("\nMemory allocation error! Addr=0x%x, Size=0x%x, File=%s, Line=%d\n", (unsigned)addr, (unsigned)size, file, line);
	while(1);
}


static void Start(void *unused1, void *unused2, ClientInterfacePtr ciPtr)
{
	long newSP;
	
	// Move the Stack to a chunk of the BSS
	newSP = (long)gStackBaseAddr + sizeof(gStackBaseAddr) - 0x100;
	__asm__ volatile("mr r1, %0" : : "r" (newSP));
	
	Main(ciPtr);
}

static void Main(ClientInterfacePtr ciPtr)
{
	int loopCount = 0;
	long ret;
	
	ret = InitEverything(ciPtr);
	if (ret != 0) Exit();
	
	// Intialize module system 
	init_module_system();
    
	execute_hook("Initialize", (void*)loopCount++, (void*)ciPtr, 0, 0);	// Main work loop

	while(1)
    {
        execute_hook("WorkLoop", (void*)loopCount++, (void*)ciPtr, 0, 0);	// Main work loop
    }	
	
}
static long InitEverything(ClientInterfacePtr ciPtr)
{
	long   ret, size;
	CICell keyboardPH;
	char   name[32];
	
	// Init the OF Client Interface.
	ret = InitCI(ciPtr);
	if (ret != 0) return -1;
	
	// Get the OF Version
	gOFVersion = GetOFVersion();
	if (gOFVersion == 0) return -1;
	
	// Get the address and size cells for the root.
	GetProp(Peer(0), "#address-cells", (char *)&gRootAddrCells, 4);
	GetProp(Peer(0), "#size-cells", (char *)&gRootSizeCells, 4);
	if ((gRootAddrCells > 2) || (gRootAddrCells > 2)) return -1;
	
	// Init the SL Words package.
	ret = InitSLWords();
	if (ret != 0) return -1;
	
	
	// Get the phandle for /chosen
	gChosenPH = FindDevice("/chosen");
	if (gChosenPH == -1) return -1;
	
	// Init the Memory Map.
	ret = InitMemoryMap();
	if (ret != 0) return -1;
	
	// Get IHandles for the MMU and Memory
	size = GetProp(gChosenPH, "mmu", (char *)&gMMUIH, 4);
	if (size != 4) {
		printf("Failed to get the IH for the MMU.\n");
		return -1;
	}
	size = GetProp(gChosenPH, "memory", (char *)&gMemoryIH, 4);
	if (size != 4) {
		printf("Failed to get the IH for the Memory.\n");
		return -1;
	}
		
	// Get stdout's IH, so that the boot display can be found.
	ret = GetProp(gChosenPH, "stdout", (char *)&gStdOutIH, 4);
	if (ret == 4) gStdOutPH = InstanceToPackage(gStdOutIH);
	else gStdOutPH = gStdOutIH = 0;
	
	// Try to find the keyboard using chosen
	ret = GetProp(gChosenPH, "stdin", (char *)&gKeyboardIH, 4);
	if (ret != 4) gKeyboardIH = 0;
	else {
		keyboardPH = InstanceToPackage(gKeyboardIH);
		ret = GetProp(keyboardPH, "name", name, 31);
		if (ret != -1) {
			name[ret] = '\0';
			if (strcmp(name, "keyboard") && strcmp(name, "kbd")) gKeyboardIH = 0;
		} else gKeyboardIH = 0;
	}
	
	// Try to the find the keyboard using open if chosen did not work.
	if (gKeyboardIH == 0) gKeyboardIH = Open("keyboard");
	if (gKeyboardIH == 0) gKeyboardIH = Open("kbd");
	
	// Get the key map set up, and make it up to date.
	gKeyMap = InitKeyMap(gKeyboardIH);
	if (gKeyMap == NULL) return -1;
	UpdateKeyMap();
	
	SetOutputLevel(kOutputLevelFull);
	
	// printf now works.
	printf("\n\nMac OS X Loader\n");
	
	// Claim memory for malloc.
	if (Claim(kMallocAddr, kMallocSize, 0) == 0) {
		printf("Claim for malloc failed.\n");
		return -1;
	}
	malloc_init((char *)kMallocAddr, kMallocSize, 768, malloc_error);
	
	
	return 0;
}

static long InitMemoryMap(void)
{
	long result;
	
	result = Interpret(0, 1,
					   " dev /chosen"
					   " new-device"
					   " \" memory-map\" device-name"
					   " active-package"
					   " device-end"
					   , &gMemoryMapPH);
	
	return result;
}


static long GetOFVersion(void)
{
	CICell ph;
	char   versStr[256], *tmpStr;
	long   vers, size;
	
	// Get the openprom package
	ph = FindDevice("/openprom");
	if (ph == -1) return 0;
	
	// Get it's model property
	size = GetProp(ph, "model", versStr, 255);
	if (size == -1) return -1;
	versStr[size] = '\0';
	
	// Find the start of the number.
	tmpStr = NULL;
	if (!strncmp(versStr, "Open Firmware, ", 15)) {
		tmpStr = versStr + 15;
	} else if (!strncmp(versStr, "OpenFirmware ", 13)) {
		tmpStr = versStr + 13;
	} else return -1;
	
	// Clasify by each instance as needed...
	switch (*tmpStr) {
		case '1' :
			vers = kOFVersion1x;
			break;
			
		case '2' :
			vers = kOFVersion2x;
			break;
			
		case '3' :
			vers = kOFVersion3x;
			break;
			
		case '4' :
			vers = kOFVersion4x;
			break;
			
		default :
			vers = 0;
			break;
	}
	
	return vers;
}