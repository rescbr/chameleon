/*
 *  nvidia_resolution.c
 *  
 *
 *  Created by Le Bidou on 19/03/10.
 *  Copyright 2010 ---. All rights reserved.
 *
 */

#include "nvidia_resolution.h"

vbios_map * open_nvidia_vbios(vbios_map *map)
{
	unsigned short nv_data_table_offset = 0;
	unsigned short nv_modeline_2_offset = 0;
	unsigned short * nv_data_table = NULL;
	NV_VESA_TABLE * std_vesa;
	
	/*
	 * Locate the VESA Tables
	 */
	
	int i = 0;
	
	while (i < 0x300) { //We don't need to look for the table in the whole bios, the 768 first bytes only
		if ((map->bios_ptr[i] == 0x44) 
			&& (map->bios_ptr[i+1] == 0x01) 
			&& (map->bios_ptr[i+2] == 0x04) 
			&& (map->bios_ptr[i+3] == 0x00)) {
			nv_data_table_offset = (unsigned short) (map->bios_ptr[i+4] | (map->bios_ptr[i+5] << 8));
			break;
		}
		i++;
	}
	//Second VESA Table on some nVidia 8xxx 9xxx and GT
	while (i < VBIOS_SIZE) { //We don't know how to locate it other way
		if ((map->bios_ptr[i] == 0x40) && (map->bios_ptr[i+1] == 0x01) //this is the first 320x200 modeline.
			&& (map->bios_ptr[i+2] == 0xC8) && (map->bios_ptr[i+3] == 0x00)
			&& (map->bios_ptr[i+4] == 0x28)
			&& (map->bios_ptr[i+5] == 0x18)
			&& (map->bios_ptr[i+6] == 0x08)
			&& (map->bios_ptr[i+7] == 0x08)) {
			nv_modeline_2_offset = (unsigned short) i;
			break;
		}
		i++;
	}
	
	nv_data_table = (unsigned short *) (map->bios_ptr + (nv_data_table_offset + OFFSET_TO_VESA_TABLE_INDEX));
	std_vesa = (NV_VESA_TABLE *) (map->bios_ptr + *nv_data_table);
	map->mode_table = (char *) std_vesa->sModelines;
	verbose("First Standard VESA Table at offset 0x%x\n", *nv_data_table);
	
	if (nv_modeline_2_offset == (VBIOS_SIZE-1) || nv_modeline_2_offset == 0) {
		map->nv_mode_table_2 = NULL;
		verbose("There is no Second Standard VESA Table to patch\n");
	} else {
		map->nv_mode_table_2 = (char*) map->bios_ptr + nv_modeline_2_offset;
		verbose("Second Standard VESA Table at offset 0x%x\n", nv_modeline_2_offset);
	}
	
	if (map->mode_table == NULL) {
		verbose("Unable to locate the mode table.\n");
		verbose("Please run the program 'dump_bios' as root and\n");
		verbose("email the file 'vbios.dmp' to gaeloulacuisse@yahoo.fr.\n");
		
		close_vbios(map);
		return 0;
	}
	
	//This won't be used as there is no garanty this is right
	map->mode_table_size = std_vesa->sHeader.usTable_Size;

	return map;
}

bool nvidia_set_mode(vbios_map* map, UInt8 idx, UInt32* x, UInt32* y, char Type)
{
	if (Type == MAIN_VESA_TABLE) {
		NV_MODELINE * mode_timing = (NV_MODELINE *) map->mode_table;
		
		if ((mode_timing[idx].reserved3 & 0xff) != 0xff) return FALSE;
		
		if ((*x != 0) && (*y != 0) && ( mode_timing[idx].usH_Active >= 640 )) {
			
			verbose("Mode %dx%d -> %dx%d ", mode_timing[idx].usH_Active, mode_timing[idx].usV_Active,
				   *x, *y);
			
			mode_timing[idx].usH_Active = *x;
			mode_timing[idx].usH_Active_minus_One = *x - 1;
			mode_timing[idx].usH_Active_minus_One_ = *x - 1;
			mode_timing[idx].usV_Active = *y;
			mode_timing[idx].usV_Active_minus_One = *y - 1;
			mode_timing[idx].usV_Active_minus_One_ = *y - 1;
		}
		
		*x = mode_timing[idx + 1].usH_Active;
		*y = mode_timing[idx + 1].usV_Active;
	}
	
	if (Type == SECOND_VESA_TABLE) {
		NV_MODELINE_2 * mode_timing = (NV_MODELINE_2 *) map->nv_mode_table_2;
		
		if (mode_timing[idx].h_disp > 0x800) return FALSE;
		
		if ((*x != 0) && (*y != 0) && ( mode_timing[idx].h_disp >= 640 )) {
			
			verbose("Mode %dx%d -> %dx%d ", mode_timing[idx].h_disp, mode_timing[idx].v_disp,
				   *x, *y);
			
			mode_timing[idx].h_disp = *x;
			mode_timing[idx].v_disp = *y;
		}
		
		*x = mode_timing[idx + 1].h_disp;
		*y = mode_timing[idx + 1].v_disp;
	}
	return TRUE;
}
