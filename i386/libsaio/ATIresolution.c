/* Copied from 915 resolution created by steve tomljenovic
 * This source code is into the public domain.
 *
 * Included to Chameleon RC3 by meklort
 *
 * Included to RC4 and edited by deviato to match more intel chipsets
 *
 */

/* Copied from 915 resolution created by steve tomljenovic
 *
 * This code is based on the techniques used in :
 *
 *   - 855patch.  Many thanks to Christian Zietz (czietz gmx net)
 *     for demonstrating how to shadow the VBIOS into system RAM
 *     and then modify it.
 *
 *   - 1280patch by Andrew Tipton (andrewtipton null li).
 *
 *   - 855resolution by Alain Poirier
 *
 * This source code is into the public domain.
 */

#include "libsaio.h"
#include "ATIresolution.h"



char * chipset_type_names[] = {
	"UNKNOWN", "830",  "845G", "855GM", "865G", "915G", "915GM", "945G", "945GM", "945GME",
	"946GZ",   "G965", "Q965", "965GM", "G41", "G31", "G45", "GM45", "500"
};

char * bios_type_names[] = {"UNKNOWN", "TYPE 1", "TYPE 2", "TYPE 3"};

int freqs[] = { 60, 75, 85 };

UInt32 get_chipset_id(void) {
	outl(0xcf8, 0x80000000);
	return inl(0xcfc);
}

chipset_type get_chipset(UInt32 id) {
	chipset_type type;
	
	switch (id) {
		case 0x35758086:
			type = CT_830;
			break;
		
		case 0x25608086:
			type = CT_845G;
			break;
				
		case 0x35808086:
			type = CT_855GM;
			break;
				
		case 0x25708086:
			type = CT_865G;
			break;
		
		case 0x25808086:
			type = CT_915G;
			break;
			
		case 0x25908086:
			type = CT_915GM;
			break;
			
		case 0x27708086:
			type = CT_945G;
			break;
		
		case 0x27a08086:
			type = CT_945GM;
			break;
			
		case 0x27ac8086:
			type = CT_945GME;
			break;
			
		case 0x29708086:
			type = CT_946GZ;
			break;
			
		case 0x29a08086:
			type = CT_G965;
			break;
			
		case 0x29908086:
			type = CT_Q965;
			break;
			
		case 0x2a008086:
			type = CT_965GM;
			break;
			
		case 0x2a408086:
			type = CT_GM45;
			break;

		case 0x2e308086:
			type = CT_G41;
			break;

		case 0x29c08086:
			type = CT_G31;
			break;

		case 0x2e208086:
			type = CT_G45;
			break;

		case 0x81008086:
			type = CT_500;
			break;
			
			
		default:
			type = CT_UNKWN;
			break;
	}
	return type;
}

vbios_resolution_type1 * map_type1_resolution(vbios_map * map, UInt16 res) {
	vbios_resolution_type1 * ptr = ((vbios_resolution_type1*)(map->bios_ptr + res)); 
	return ptr;
}

vbios_resolution_type2 * map_type2_resolution(vbios_map * map, UInt16 res) {
	vbios_resolution_type2 * ptr = ((vbios_resolution_type2*)(map->bios_ptr + res)); 
	return ptr;
}

vbios_resolution_type3 * map_type3_resolution(vbios_map * map, UInt16 res) {
	vbios_resolution_type3 * ptr = ((vbios_resolution_type3*)(map->bios_ptr + res)); 
	return ptr;
}

char detect_bios_type(vbios_map * map, char modeline, int entry_size) {
	UInt32 i;
	UInt16 r1, r2;
	    
	r1 = r2 = 32000;
	
	for (i=0; i < map->mode_table_size; i++) {
		if (map->mode_table[i].resolution <= r1) {
			r1 = map->mode_table[i].resolution;
		}
		else {
			if (map->mode_table[i].resolution <= r2) {
				r2 = map->mode_table[i].resolution;
			}
		}
		
		/*printf("r1 = %d  r2 = %d\n", r1, r2);*/
	}

	return (r2-r1-6) % entry_size == 0;
}

void close_vbios(vbios_map * map);







vbios_map * open_vbios(chipset_type forced_chipset) {
	UInt32 z;
	vbios_map * map = NEW(vbios_map);
	for(z=0; z<sizeof(vbios_map); z++) ((char*)map)[z]=0;
	
	/*
	 * Determine chipset
	 */
	
	if (forced_chipset == CT_UNKWN) {
		map->chipset_id = get_chipset_id();
		map->chipset = get_chipset(map->chipset_id);
	}
	else if (forced_chipset != CT_UNKWN) {
		map->chipset = forced_chipset;
	}
	else {
		map->chipset = CT_915GM;
	}
	    
	/*
	 *  Map the video bios to memory
	 */
	
	map->bios_ptr=(char*)VBIOS_START;
	
	/*
	 * check if we have ATI Radeon
	 
	
	if (memmem(map->bios_ptr, VBIOS_SIZE, ATI_SIGNATURE1, strlen(ATI_SIGNATURE1)) ||
		memmem(map->bios_ptr, VBIOS_SIZE, ATI_SIGNATURE2, strlen(ATI_SIGNATURE2)) ) {
		printf("ATI chipset detected. \n");
		printf("Chipset Id: %x\n", map->chipset_id);
		return 0;
	
	}*/
	/*
	 * check if we have NVIDIA
	 */
	    
	/*
	 * check if we have Intel
	 */
	    
	/*
	 * check for others
	 */
	
	if (map->chipset == CT_UNKWN) {
		printf("Unknown chipset type and unrecognized bios.\n");
		printf("915resolution only works with Intel 800/900 series graphic chipsets.\n");
	
		printf("Chipset Id: %x\n", map->chipset_id);
		close_vbios(map);
		return 0;
	}

	/*
	 * Figure out where the mode table is 
	 */
	
	{
		char* p = map->bios_ptr + 16;
		char* limit = map->bios_ptr + VBIOS_SIZE;
			
		while (p < limit && map->mode_table == 0) {
			vbios_mode * mode_ptr = (vbios_mode *) p;
			            
			if ((mode_ptr[0].mode == 0x20) && (mode_ptr[1].mode == 0x03) &&
				(mode_ptr[4].mode == 0x58) && (mode_ptr[5].mode == 0x02)) {
				printf("We are onto something.\n");
				map->mode_table = mode_ptr;
			}
			            
			p++;
		}
		
		if (map->mode_table == 0) {
			printf("Unable to locate the mode table.\n");
			printf("Please run the program 'dump_bios' as root and\n");
			printf("email the file 'vbios.dmp' to stomljen@yahoo.com.\n");
			printf("Chipset: %s\n", chipset_type_names[map->chipset]);
			printf("Size: %d\n", sizeof(vbios_map));
			
			close_vbios(map);
				   getc();
			return 0;
		}
	}
	
	/*
	 * Determine size of mode table
	 */
	    
	{
		vbios_mode * mode_ptr = map->mode_table;
			
		while (mode_ptr->mode != 0xff) {
			map->mode_table_size++;
			mode_ptr++;
		}
	}
	
	/*
	 * Figure out what type of bios we have
	 *  order of detection is important
	 */
	
	if (detect_bios_type(map, TRUE, sizeof(vbios_modeline_type3))) {
		map->bios = BT_3;
	}
	else if (detect_bios_type(map, TRUE, sizeof(vbios_modeline_type2))) {
		map->bios = BT_2;
	}
	else if (detect_bios_type(map, FALSE, sizeof(vbios_resolution_type1))) {
		map->bios = BT_1;
	}
	else {
		printf("Unable to determine bios type.\n");
		printf("Please run the program 'dump_bios' as root and\n");
		printf("email the file 'vbios.dmp' to stomljen@yahoo.com.\n");
	
		printf("Chipset: %s\n", chipset_type_names[map->chipset]);
		printf("Mode Table Offset: $C0000 + $%x\n", ((UInt32)map->mode_table) - ((UInt32)map->bios_ptr));
		printf("Mode Table Entries: %u\n", map->mode_table_size);
		getc();
		//THis is now
		
		return 0;
	}
	
	return map;
}

void close_vbios(vbios_map * map) {
	FREE(map);
}

void unlock_vbios(vbios_map * map) {

	map->unlocked = TRUE;
	    
	switch (map->chipset) {
		case CT_UNKWN:
			break;
		case CT_830:
		case CT_855GM:
			outl(0xcf8, 0x8000005a);
			map->b1 = inb(0xcfe);
				
			outl(0xcf8, 0x8000005a);
			outb(0xcfe, 0x33);
			break;
		case CT_845G:
		case CT_865G:
		case CT_915G:
		case CT_915GM:
		case CT_945G:
		case CT_945GM:
		case CT_945GME:
		case CT_946GZ:
		case CT_G965:
		case CT_Q965:
		case CT_965GM:
		case CT_GM45:
		case CT_G41:
		case CT_G31:
		case CT_G45:
		case CT_500:
			outl(0xcf8, 0x80000090);
			map->b1 = inb(0xcfd);
			map->b2 = inb(0xcfe);
			
			outl(0xcf8, 0x80000090);
			outb(0xcfd, 0x33);
			outb(0xcfe, 0x33);
		break;
	}
	
	#if DEBUG
	{
		UInt32 t = inl(0xcfc);
		printf("unlock PAM: (0x%08x)\n", t);
	}
#endif
}

void relock_vbios(vbios_map * map) {

	map->unlocked = FALSE;
	
	switch (map->chipset) {
		case CT_UNKWN:
			break;
		case CT_830:
		case CT_855GM:
			outl(0xcf8, 0x8000005a);
			outb(0xcfe, map->b1);
			break;
		case CT_845G:
		case CT_865G:
		case CT_915G:
		case CT_915GM:
		case CT_945G:
		case CT_945GM:
		case CT_945GME:
		case CT_946GZ:
		case CT_G965:
		case CT_Q965:
		case CT_965GM:
		case CT_GM45:
		case CT_G41:
		case CT_G31:
		case CT_G45:
		case CT_500:
			outl(0xcf8, 0x8000005a);
			outb(0xcfd, map->b1);
			outb(0xcfe, map->b2);
			break;
	}
	
	#if DEBUG
	{
        UInt32 t = inl(0xcfc);
		printf("relock PAM: (0x%08x)\n", t);
	}
	#endif
}
 

void list_modes(vbios_map *map, UInt32 raw) {
    UInt32 i, x, y;
	
    for (i=0; i < map->mode_table_size; i++) {
        switch(map->bios) {
			case BT_1:
            {
                vbios_resolution_type1 * res = map_type1_resolution(map, map->mode_table[i].resolution);
                
                x = ((((UInt32) res->x2) & 0xf0) << 4) | res->x1;
                y = ((((UInt32) res->y2) & 0xf0) << 4) | res->y1;
                
                if (x != 0 && y != 0) {
                    printf("Mode %02x : %dx%d, %d bits/pixel\n", map->mode_table[i].mode, x, y, map->mode_table[i].bits_per_pixel);
                }
				
				if (raw)
				{
                    printf("Mode %02x (raw) :\n\t%02x %02x\n\t%02x\n\t%02x\n\t%02x\n\t%02x\n\t%02x\n\t%02x\n", map->mode_table[i].mode, res->unknow1[0],res->unknow1[1], res->x1,res->x_total,res->x2,res->y1,res->y_total,res->y2);
				}
				
            }
				break;
			case BT_2:
            {
                vbios_resolution_type2 * res = map_type2_resolution(map, map->mode_table[i].resolution);
                
                x = res->modelines[0].x1+1;
                y = res->modelines[0].y1+1;
				
                if (x != 0 && y != 0) {
                    printf("Mode %02x : %dx%d, %d bits/pixel\n", map->mode_table[i].mode, x, y, map->mode_table[i].bits_per_pixel);
                }
            }
				break;
			case BT_3:
            {
                vbios_resolution_type3 * res = map_type3_resolution(map, map->mode_table[i].resolution);
                
                x = res->modelines[0].x1+1;
                y = res->modelines[0].y1+1;
                
                if (x != 0 && y != 0) {
                    printf("Mode %02x : %dx%d, %d bits/pixel\n", map->mode_table[i].mode, x, y, map->mode_table[i].bits_per_pixel);
                }
            }
				break;
			case BT_UNKWN:
				break;
        }
    }
}

static void gtf_timings(UInt32 x, UInt32 y, UInt32 freq,
						unsigned long *clock,
						UInt16 *hsyncstart, UInt16 *hsyncend, UInt16 *hblank,
						UInt16 *vsyncstart, UInt16 *vsyncend, UInt16 *vblank)
{
	UInt32 hbl, vbl, vfreq;

	vbl = y + (y+1)/(20000.0/(11*freq) - 1) + 1.5;
	vfreq = vbl * freq;
	hbl = 16 * (int)(x * (30.0 - 300000.0 / vfreq) /
						  +            (70.0 + 300000.0 / vfreq) / 16.0 + 0.5);

	*vsyncstart = y;
	*vsyncend = y + 3;
	*vblank = vbl - 1;
	*hsyncstart = x + hbl / 2 - (x + hbl + 50) / 100 * 8 - 1;
	*hsyncend = x + hbl / 2 - 1;
	*hblank = x + hbl - 1;
	*clock = (x + hbl) * vfreq / 1000;
}

void set_mode(vbios_map * map, /*UInt32 mode,*/ UInt32 x, UInt32 y, UInt32 bp, UInt32 htotal, UInt32 vtotal) {
	UInt32 xprev, yprev;
	UInt32 i = 0, j;	// patch first available mode

//	for (i=0; i < map->mode_table_size; i++) {
//		if (map->mode_table[0].mode == mode) {
			switch(map->bios) {
				case BT_1:
					{
						vbios_resolution_type1 * res = map_type1_resolution(map, map->mode_table[i].resolution);
						
						if (bp) {
							map->mode_table[i].bits_per_pixel = bp;
						}
						
						res->x2 = (htotal?(((htotal-x) >> 8) & 0x0f) : (res->x2 & 0x0f)) | ((x >> 4) & 0xf0);
						res->x1 = (x & 0xff);
						
						res->y2 = (vtotal?(((vtotal-y) >> 8) & 0x0f) : (res->y2 & 0x0f)) | ((y >> 4) & 0xf0);
						res->y1 = (y & 0xff);
						if (htotal)
							res->x_total = ((htotal-x) & 0xff);
						
						if (vtotal)
							res->y_total = ((vtotal-y) & 0xff);
					}
					break;
				case BT_2:
					{
						vbios_resolution_type2 * res = map_type2_resolution(map, map->mode_table[i].resolution);
						
						res->xchars = x / 8;
						res->ychars = y / 16 - 1;
						xprev = res->modelines[0].x1;
						yprev = res->modelines[0].y1;
						
						for(j=0; j < 3; j++) {
							vbios_modeline_type2 * modeline = &res->modelines[j];
							
							if (modeline->x1 == xprev && modeline->y1 == yprev) {
								modeline->x1 = modeline->x2 = x-1;
								modeline->y1 = modeline->y2 = y-1;
				
								gtf_timings(x, y, freqs[j], &modeline->clock,
											&modeline->hsyncstart, &modeline->hsyncend,
											&modeline->hblank, &modeline->vsyncstart,
											&modeline->vsyncend, &modeline->vblank);
								
								if (htotal)
									modeline->htotal = htotal;
								else
									modeline->htotal = modeline->hblank;
								
								if (vtotal)
									modeline->vtotal = vtotal;
								else
									modeline->vtotal = modeline->vblank;
							}
						}
					}
					break;
				case BT_3:
					{
						vbios_resolution_type3 * res = map_type3_resolution(map, map->mode_table[i].resolution);
						
						xprev = res->modelines[0].x1;
						yprev = res->modelines[0].y1;
				
						for (j=0; j < 3; j++) {
							vbios_modeline_type3 * modeline = &res->modelines[j];
							                        
							if (modeline->x1 == xprev && modeline->y1 == yprev) {
								modeline->x1 = modeline->x2 = x-1;
								modeline->y1 = modeline->y2 = y-1;
								                            
								gtf_timings(x, y, freqs[j], &modeline->clock,
											&modeline->hsyncstart, &modeline->hsyncend,
											&modeline->hblank, &modeline->vsyncstart,
											&modeline->vsyncend, &modeline->vblank);
								if (htotal)
									modeline->htotal = htotal;
								else
									modeline->htotal = modeline->hblank;
								if (vtotal)
									modeline->vtotal = vtotal;
								else
									modeline->vtotal = modeline->vblank;
						
								modeline->timing_h   = y-1;
								modeline->timing_v   = x-1;
							}
						}
					}
					break;
				case BT_UNKWN:
					break;
			}
//		}
//	}
}   

void display_map_info(vbios_map * map) {
	printf("Chipset: %s\n", chipset_type_names[map->chipset]);
	printf("BIOS: %s\n", bios_type_names[map->bios]);
	
	printf("Mode Table Offset: $C0000 + $%x\n", ((UInt32)map->mode_table) - ((UInt32)map->bios_ptr));
	printf("Mode Table Entries: %u\n", map->mode_table_size);
}
