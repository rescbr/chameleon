/*
 *  nvidia_resolution.c
 *  
 *
 *  Created by Le Bidou on 19/03/10.
 *  Copyright 2010 ---. All rights reserved.
 *
 */

#include "nvidia_resolution.h"

vBiosMap * openNvidiaVbios(vBiosMap *map)
{
	unsigned short nvDataTableOffset = 0;
	unsigned short nvModeline_2_Offset = 0;
	unsigned short * nvDataTable = NULL;
	nvVesaTable * stdVesa;
	
	// initialize the table chain with two elements
	sModeTable * table = intializeTables(map, 2);
	/*
	 * Locate the VESA Tables
	 */
	
	int i = 0;
	//First VESA Table
	while (i < 0x300) //We don't need to look for the table in the whole bios, the 768 first bytes only
	{ 
		if ((map->biosPtr[i] == 0x44) 
			&& (map->biosPtr[i+1] == 0x01) 
			&& (map->biosPtr[i+2] == 0x04) 
			&& (map->biosPtr[i+3] == 0x00)) {
			nvDataTableOffset = (unsigned short) (map->biosPtr[i+4] | (map->biosPtr[i+5] << 8));
			break;
		}
		i++;
	}
	
	nvDataTable = (unsigned short *) (map->biosPtr + (nvDataTableOffset + OFFSET_TO_VESA_TABLE_INDEX));
	stdVesa = (nvVesaTable *) (map->biosPtr + *nvDataTable);
	table->pointer = (uint8_t *) stdVesa + sizeof(nvCommonTableHeader);
	verbose("First Standard VESA Table at offset 0x%x\n",(unsigned int) (table->pointer - map->biosPtr));
	
	//Second VESA Table
	while (i < VBIOS_SIZE) //We don't know how to locate it other way
	{
		if ((map->biosPtr[i] == 0x40) && (map->biosPtr[i+1] == 0x01) //this is the first 320x200 modeline.
			&& (map->biosPtr[i+2] == 0xC8) && (map->biosPtr[i+3] == 0x00)
			&& (map->biosPtr[i+4] == 0x28)
			&& (map->biosPtr[i+5] == 0x18)
			&& (map->biosPtr[i+6] == 0x08)
			&& (map->biosPtr[i+7] == 0x08))
		{
			nvModeline_2_Offset = (unsigned short) i;
			break;
		}
		i++;
	}
	
	
	
	if (nvModeline_2_Offset == (VBIOS_SIZE-1) || nvModeline_2_Offset == 0)
	{
		//If no second vesa table is available, free the corresponding table in chain
		free(table->next);
		table->next = NULL;
		PRINT("There is no Second Standard VESA Table to patch\n");
	} 
	else
	{
		table = table->next;
		table->pointer = map->biosPtr + nvModeline_2_Offset;
		PRINT("Second Standard VESA Table at offset 0x%x\n", (unsigned int)(table->pointer - map->biosPtr));
	}
	
	if (table->prev->pointer == NULL)
	{
		PRINT("Unable to locate the mode table.\n");
		PRINT("Please run the program 'dumpBios' as root and\n");
		PRINT("email the file 'vbios.dmp' to gaeloulacuisse@yahoo.fr.\n");
		
		closeVbios(map);
		return 0;
	}
	
	// reset the table pointer to the first in chain
	table = map->modeTables;
	
	/*
	 * for each table in chain, figure out how many modes are available
	 *   and what is the size of the table
	 */
	while (table != NULL)
	{
		table->modeCount = 0;
		PRINT("Table #%d has ", table->id);
		
		if (table->id == MAIN_VESA_TABLE)
		{
			table->modeCount = stdVesa->header.tableSize & 0xff;
			if (table->modeCount == 0) table->modeCount = 16;
			table->size = table->modeCount * sizeof(nvModeline);
		}
		
		if (table->id == SECOND_VESA_TABLE)
		{
			nvModeline_2 * modePtr = (nvModeline_2 *)table->pointer;
			while (1)
			{
				if (modePtr[table->modeCount].hActive > 2048) break;
				table->modeCount++;		
			}
			if (table->modeCount == 0) table->modeCount = 31;
			table->size = table->modeCount * sizeof(nvModeline_2);
		}
		
		PRINT("%d modes\n", table->modeCount);
		table = table->next;
	}
	
	map->setMode = nvidiaSetMode;
	
	saveTables(map->modeTables);
	
#if AUTORES_DEBUG
	PRINT("Press Any Key...\n");
	getc();
#endif
	
	return map;
}

bool nvidiaSetMode(sModeTable * table, uint8_t idx, uint32_t* x, uint32_t* y)
{
	//In First VESA table, only first mode in table is patched
	if (table->id == MAIN_VESA_TABLE)
	{
		nvModeline * modeTiming = (nvModeline *)table->pointer;
		
		// patch only if mode differs and if the mode to patch isn't a console mode
		if (((*x != modeTiming[idx].hActive) || (*y != modeTiming[idx].vActive))
			&& (modeTiming[idx].hActive > 100))
		{
			
			PRINT("Mode %dx%d -> ", modeTiming[idx].hActive, modeTiming[idx].vActive);
			
			if (modeTiming[idx].hActive != *x)
			{
				modeTiming[idx].hActive = *x;
				modeTiming[idx].hActiveMinusOne = *x - 1;
				modeTiming[idx].hActiveMinusOne_ = *x - 1;
			}
			
			modeTiming[idx].vActive = *y;
			modeTiming[idx].vActiveMinusOne = *y - 1;
			modeTiming[idx].vActiveMinusOne_ = *y - 1;
			
			PRINT("%dx%d (%d %d %d %d %d %d)\n",
				  modeTiming[idx].hActive,
				  modeTiming[idx].vActive,
				  modeTiming[idx].hSyncStart,
				  modeTiming[idx].hSyncEnd,
				  modeTiming[idx].hTotal,
				  modeTiming[idx].vSyncStart,
				  modeTiming[idx].vSyncEnd,
				  modeTiming[idx].vTotal);
		}
		
		//Since we are only patching the first mode, this deprecated
		*x = modeTiming[idx + 1].hActive;
		*y = modeTiming[idx + 1].vActive;
	}
	
	if (table->id == SECOND_VESA_TABLE)
	{
		nvModeline_2 * modeTiming = (nvModeline_2 *) table->pointer;
		int h = *y;
		int w = *x;
		
		while (idx < table->modeCount)
		{
			*y = *x * h / w;
			//patch only different mode in table except console modes and 320 wide modes
			if (((*x != modeTiming[idx].hActive) || (*y != modeTiming[idx].vActive))
				&& (modeTiming[idx].hActive > 400))
			{
			
				PRINT("Mode %dx%d -> ", modeTiming[idx].hActive, modeTiming[idx].vActive);
			
				if (modeTiming[idx].hActive != *x)
					modeTiming[idx].hActive = *x;
			
				modeTiming[idx].vActive = *y;
			
				PRINT("%dx%d (%d %d %d %d ",
					  modeTiming[idx].hActive,
					  modeTiming[idx].vActive,
					  modeTiming[idx].hActive + modeTiming[idx].hSyncOffset,
					  modeTiming[idx].hActive + modeTiming[idx].hSyncOffset + modeTiming[idx].hSyncWidth,
					  modeTiming[idx].hActive + modeTiming[idx].hBlank,
					  modeTiming[idx].vActive + modeTiming[idx].vBlank);
			
				if ((modeTiming[idx].flags & HSyncPolarityMask) == HSyncPolarityMask)
				{	PRINT("H- ");	}
				else
				{	PRINT("H+ ");	}

				if ((modeTiming[idx].flags & VSyncPolarityMask) == VSyncPolarityMask)
				{	PRINT("V-)\n");	}
				else
				{	PRINT("V+)\n");	}
			}
			//returns the next mode in table
			*x = modeTiming[idx + 1].hActive;
			*y = modeTiming[idx + 1].vActive;
			
			idx++;
		}
		
		
		
	}
	return TRUE;
}