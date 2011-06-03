/*
 *  ati_resolution.c
 *  
 *
 *  Created by Le Bidou on 19/03/10.
 *  Copyright 2010 ---. All rights reserved.
 *
 */

#include "ati_resolution.h"

char detectAtiBiosType(sModeTable * table) {	
	return table->size % sizeof(atomModeTiming) == 0;
}



vBiosMap * openAtiVbios(vBiosMap * map, atiBiosTables atiTables)
{
	/*
	 * Locate the Standard VESA Table
	 */
	
	atiTables.masterDataTables = (uint16_t *) &((atomMasterDataTable *) (map->biosPtr + atiTables.atomRomHeader->masterDataTableOffset))->listOfDataTables;
	uint16_t stdVesaOffset = (uint16_t) ((atomMasterListOfDataTables *)atiTables.masterDataTables)->StandardVESA_Timing;
	atomStandardVesaTiming * stdVesa = (atomStandardVesaTiming *) (map->biosPtr + stdVesaOffset);
	
	// intialize the table chain with one element
	sModeTable * table = intializeTables(map, 1);
		
	table->pointer = (uint8_t *)stdVesa + sizeof(atomCommonTableHeader);
	PRINT("Standard VESA Table at offset * 0x%x\n", (uint8_t)(table->pointer - map->biosPtr));
	if (table->pointer == 0) {
		PRINT("Unable to locate the mode table.\n");
		PRINT("Please run the program 'dump_bios' as root and\n");
		PRINT("email the file 'vbios.dmp' to gaeloulacuisse@yahoo.fr.\n"); //Azi: remove?
		
		closeVbios(map);
		return 0;
	}
	
	//Determine Size of the Table
	table->size = stdVesa->header.structureSize - sizeof(atomCommonTableHeader);
	
	/*
	 * Find out type of table and how many entries it has
	 */
	
	if (!detectAtiBiosType(table)) map->bios = BT_ATI_2;
	if (map->bios == BT_ATI_2) {
		table->modeCount = table->size / sizeof(atomDtdFormat);
		map->setMode = atiSetMode_2;
		PRINT("Using DTD Format modelines\n");
	} else {
		table->modeCount = table->size / sizeof(atomModeTiming);
		map->setMode = atiSetMode_1;
		PRINT("Using Atom Mode Timing modelines\n");
	}
	saveTables(table);
	return map;
}

bool atiSetMode_1(sModeTable * table, uint8_t idx, uint32_t * x, uint32_t * y)
{
	atomModeTiming *modeTiming = (atomModeTiming *) table->pointer;
	
	// patch only if mode differs
	if ((*x != modeTiming[idx].hActive) || (*y != modeTiming[idx].vActive)) {
		PRINT("Mode %dx%d -> ",modeTiming[idx].hActive, modeTiming[idx].vActive);
		
		modeTiming[idx].hActive = *x;
		modeTiming[idx].vActive = *y;
		
		PRINT("%dx%d\n",modeTiming[idx].hActive, modeTiming[idx].vActive);
	}
	
	// since only the first mode in table is patched, this is deprecated
	*x = modeTiming[idx + 1].hActive;
	*y = modeTiming[idx + 1].vActive;
	
	return true;
}

bool atiSetMode_2(sModeTable * table, uint8_t idx, uint32_t* x, uint32_t* y)
{
	atomDtdFormat *modeTiming = (atomDtdFormat *) table->pointer;
	
	// patch only if mode differs
	if ((*x != modeTiming[idx].hActive) || (*y != modeTiming[idx].vActive)) {
		
		PRINT("Mode %dx%d -> ", modeTiming[idx].hActive, modeTiming[idx].vActive);
		
		modeTiming[idx].hActive = *x;
		modeTiming[idx].vActive = *y;
		
		PRINT("%dx%d\n", modeTiming[idx].hActive, modeTiming[idx].vActive);
	}
	
	// since only the first mode in table is patched, this is deprecated
	*x = modeTiming[idx + 1].hActive;
	*y = modeTiming[idx + 1].hActive;
	
	return true;
}
