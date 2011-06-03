/*
 *  gmaResolution.c
 *  
 *
 *  Created by Le Bidou on 19/03/10.
 *  Copyright 2010 ---. All rights reserved.
 *
 */

#include "gma_resolution.h"

char * biosTypeNames[] = {"UNKNOWN", "TYPE 1", "TYPE 2", "TYPE 3"};
int freqs[] = { 60, 75, 85 };


vbiosResolutionType1 * mapType1Resolution(vBiosMap * map, uint16_t res)
{
	vbiosResolutionType1 * ptr = ((vbiosResolutionType1*)(map->biosPtr + res)); 
	return ptr;
}

vbiosResolutionType2 * mapType2Resolution(vBiosMap * map, uint16_t res)
{
	vbiosResolutionType2 * ptr = ((vbiosResolutionType2*)(map->biosPtr + res)); 
	return ptr;
}

vbiosResolutionType3 * mapType3Resolution(vBiosMap * map, uint16_t res)
{
	vbiosResolutionType3 * ptr = ((vbiosResolutionType3*)(map->biosPtr + res)); 
	return ptr;
}

char detectBiosType(vBiosMap * map, char modeline, int entrySize)
{
	uint32_t i;
	uint16_t r1, r2;
	
	vbiosMode * modeTable = (vbiosMode *)map->modeTables->pointer;
	
	r1 = r2 = 32000;
	
	for (i=0; i < map->modeTables->size; i++)
	{
		if (modeTable[i].resolution <= r1)
			r1 = modeTable[i].resolution;
		else if (modeTable[i].resolution <= r2)
			r2 = modeTable[i].resolution;
	}
	
	return (r2-r1-6) % entrySize == 0;
}

vBiosMap * openIntelVbios(vBiosMap *map)
{
	/*
	 * Find the location of the Mode Table
	 */
	unsigned char* p = map->biosPtr + 16;
	unsigned char* limit = map->biosPtr + VBIOS_SIZE - (3 * sizeof(vbiosMode));
	
	// initialize the table chain with one element
	sModeTable * table = intializeTables(map, 1);
	
	while (p < limit && table->pointer == 0)
	{
		vbiosMode* modePtr = (vbiosMode*) p;
		
		if (((modePtr[0].mode & 0xf0) == 0x30) && ((modePtr[1].mode & 0xf0) == 0x30) &&
			((modePtr[2].mode & 0xf0) == 0x30) && ((modePtr[3].mode & 0xf0) == 0x30))
		{
			
			table->pointer = (uint8_t *)modePtr;
		}
		
		p++;
	}
	
	if (table->pointer == 0)
	{
		PRINT("Unable to locate the mode table.\n");
		PRINT("Please run the program 'dump_bios' as root and\n");
		PRINT("email the file 'vbios.dmp' to gaeloulacuisse@yahoo.fr.\n");
		
		closeVbios(map);
		return 0;
	}
	
	PRINT("Mode Table at offset: 0x%x\n", (table->pointer) - map->biosPtr);
	
	/*
	 * Determine size of mode table
	 */
	
	vbiosMode * modePtr = (vbiosMode *)table->pointer;
	
	while (modePtr->mode != 0xff)
	{
		table->size++;
		modePtr++;
	}
	
	table->modeCount = table->size;
	PRINT("Mode Table size: %d\n", table->modeCount);
	
	/*
	 * Figure out what type of bios we have
	 *  order of detection is important
	 */
	
	if (detectBiosType(map, true, sizeof(vbiosModelineType3)))
	{
		map->bios = BT_3;
		map->setMode = intelSetMode_3;
		PRINT("Bios Type: BT_3\n");
	}
	else if (detectBiosType(map, true, sizeof(vbiosModelineType2)))
	{
		map->bios = BT_2;
		map->setMode = intelSetMode_2;
		PRINT("Bios Type: BT_2\n");
	}
	else if (detectBiosType(map, false, sizeof(vbiosResolutionType1)))
	{
		map->bios = BT_1;
		map->setMode = intelSetMode_1;
		PRINT("Bios Type: BT_1\n");
	}
	else
	{
		PRINT("Unable to determine bios type.\n");
		PRINT("Please run the program 'dump_bios' as root and\n");
		PRINT("email the file 'vbios.dmp' to gaeloulacuisse@yahoo.fr.\n"); //Azi: remove?
		
		return 0;
	}
	
	return map;
}
	


bool intelSetMode_1(sModeTable * table, uint8_t idx, uint32_t* x, uint32_t* y)
{
	vbiosMode *modeTiming = (vbiosMode *) table->pointer;
	vbiosResolutionType1 * res = mapType1Resolution(map, modeTiming[idx].resolution);
	
	//retreive the original mode's dimensions
	uint32_t actualX = ((res->hActive2 & 0xf0) << 4) | (res->hActive1 & 0xff);
	uint32_t actualY = ((res->vActive2 & 0xf0) << 4) | (res->vActive1 & 0xff);
	
	// patch only if mode differs
	if ((*x != actualX) || (*y != actualY))
	{	
		PRINT("Mode %dx%d -> ", actualX, actualY);
		
		res->hActive2 = (res->hActive2 & 0x0f) | ((*x >> 4) & 0xf0);
		res->hActive1 = (*x & 0xff);
		
		res->vActive2 = (res->hActive2 & 0x0f) | ((*y >> 4) & 0xf0);
		res->vActive1 = (*y & 0xff);
		
		uint32_t actualX = ((res->hActive2 & 0xf0) << 4) | (res->hActive1 & 0xff);
		uint32_t actualY = ((res->vActive2 & 0xf0) << 4) | (res->vActive1 & 0xff);
		
		PRINT("%dx%d \n", actualX, actualY);
	}
	
	//since only the first mode is patched, this is deprecated
	res = mapType1Resolution(map, modeTiming[idx + 1].resolution);
	
	actualX = ((res->hActive2 & 0xf0) << 4) | (res->hActive1 & 0xff);
	actualY = ((res->vActive2 & 0xf0) << 4) | (res->vActive1 & 0xff);
	
	*x = actualX;
	*y = actualY;
	
	return true;
}

bool intelSetMode_2(sModeTable * table, uint8_t idx, uint32_t* x, uint32_t* y)
{
	uint32_t xprev, yprev, j = 0;
	
	vbiosMode *modeTiming = (vbiosMode *) table->pointer;
	vbiosResolutionType2 * res = mapType2Resolution(map, modeTiming[idx].resolution);
	
	//patch only if mode differs
	if (	(*x != (res->modelines[0].hActive1 + 1)) 
		||	(*y != (res->modelines[0].vActive1 + 1)) )
	{
		
		PRINT("Mode %dx%d -> ", res->modelines[0].hActive1 + 1, res->modelines[0].vActive1 + 1);
		
		res->xChars = *x / 8;
		res->yChars = *y / 16 - 1;
		xprev = res->modelines[0].hActive1;
		yprev = res->modelines[0].vActive1;
		
		// recalculate timigs for all frequencies and patch
		for(j = 0; j < 3; j++)
		{
			vbiosModelineType2 * mode = &res->modelines[j];
			
			if (mode->hActive1 == xprev && mode->vActive1 == yprev)
			{
				mode->hActive1 = mode->hActive2 = *x - 1;
				mode->vActive1 = mode->vActive2 = *y - 1;
				
				gtfTimings(*x, *y, freqs[j], &mode->clock,
							&mode->hSyncStart, &mode->hSyncEnd,
							&mode->hBlank, &mode->vSyncStart,
							&mode->vSyncEnd, &mode->vBlank);
				
				mode->hTotal = mode->hBlank;
				mode->vTotal = mode->vBlank;
			}
		}
		PRINT("%dx%d\n", res->modelines[0].hActive1 + 1, res->modelines[0].vActive1 + 1);
		
	}
	//since only the first mode is patched, this is deprecated
	res = mapType2Resolution(map, modeTiming[idx + 1].resolution);
	
	*x = res->modelines[0].hActive1 + 1;
	*y = res->modelines[0].vActive1 + 1;
	
	return true;
}

bool intelSetMode_3(sModeTable * table, uint8_t idx, uint32_t* x, uint32_t* y)
{
	uint32_t xprev, yprev, j = 0;
	
	vbiosMode *modeTiming = (vbiosMode *) table->pointer;
	vbiosResolutionType3 * res = mapType3Resolution(map, modeTiming[idx].resolution);
	
	// patch only if mode differs
	if (	(*x != (res->modelines[0].hActive1 + 1)) 
		||	(*y != (res->modelines[0].vActive1 + 1)) )
	{
		
		PRINT("Mode %dx%d -> ", res->modelines[0].hActive1 + 1, res->modelines[0].vActive1 + 1);
		
		xprev = res->modelines[0].hActive1;
		yprev = res->modelines[0].vActive1;
		
		// recalculate timings for all frequencies and patch
		for(j = 0; j < 3; j++) {
			vbiosModelineType3 * mode = &res->modelines[j];
			
			if (mode->hActive1 == xprev && mode->vActive1 == yprev)
			{
				mode->hActive1 = mode->hActive2 = *x - 1;
				mode->vActive1 = mode->vActive2 = *y - 1;
				
				gtfTimings(*x, *y, freqs[j], &mode->clock,
							&mode->hSyncStart, &mode->hSyncEnd,
							&mode->hBlank, &mode->vSyncStart,
							&mode->vSyncEnd, &mode->vBlank);
				
				mode->hTotal = mode->hBlank;
				mode->vTotal = mode->vBlank;
				
				mode->timingH   = *y-1;
				mode->timingV   = *x-1;
			}
		}
		
		PRINT("%dx%d\n", res->modelines[0].hActive1 + 1, res->modelines[0].vActive1 + 1);
		
	}
	//since only the first mode is patched, this is deprecated
	res = mapType3Resolution(map, modeTiming[idx + 1].resolution);
	
	*x = res->modelines[0].hActive1 + 1;
	*y = res->modelines[0].vActive1 + 1;
	
	return true;
}