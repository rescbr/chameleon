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

//#include "libsaio.h"
//#include "autoresolution.h" - included on *_resolution.h
#include "nvidia_resolution.h"
#include "ati_resolution.h"
#include "gma_resolution.h"
#include "../boot2/graphics.h" //Azi:reminder

char * chipsetTypeNames[] = {
	"UNKNOWN", "830",  "845G", "855GM", "865G", "915G", "915GM", "945G", "945GM", "945GME",
	"946GZ",   "955X", "G965", "Q965", "965GM", "975X",
	"P35", "X48", "B43", "Q45", "P45", "GM45", "G41", "G31", "G45", "500"
};

uint32_t getChipsetId(void)
{
	outl(0xcf8, 0x80000000);
	return inl(0xcfc);
}

chipsetType getChipset(uint32_t id)
{
	chipsetType type;
	
	switch (id)
	{
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
			
		case 0x27748086:
			type = CT_955X;
			break;
			
		case 0x277c8086:
			type = CT_975X;
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
			
		case 0x29e08086:
			type = CT_X48;
			break;			
			
		case 0x2a408086:
			type = CT_GM45;
			break;
			
		case 0x2e108086:
		case 0X2e908086:
			type = CT_B43;
			break;
			
		case 0x2e208086:
			type = CT_P45;
			break;
			
		case 0x2e308086:
			type = CT_G41;
			break;
			
		case 0x29c08086:
			type = CT_G31;
			break;
			
		case 0x29208086:
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


void gtfTimings(uint32_t x, uint32_t y, uint32_t freq,
				 uint32_t *clock,
				 uint16_t *hSyncStart, uint16_t *hSyncEnd, uint16_t *hBlank,
				 uint16_t *vSyncStart, uint16_t *vSyncEnd, uint16_t *vBlank)
{
	uint32_t hbl, vbl, vfreq;
	
	vbl = y + (y+1)/(20000/(11*freq) - 1) + 1;
	
	vfreq = vbl * freq;
	hbl = 16 * (int)(x * (30 - 300000 / vfreq) /
					 +            (70 + 300000 / vfreq) / 16 + 0);
	
	*vSyncStart = y;
	*vSyncEnd = y + 3;
	*vBlank = vbl;	
	*hSyncStart = x + hbl / 2 - (x + hbl + 50) / 100 * 8 ;	
	*hSyncEnd = x + hbl / 2;	
	*hBlank = x + hbl;	
	*clock = (x + hbl) * vfreq / 1000;
}


void getAspectRatio(sAspect* aspect, uint32_t x, uint32_t y)
{
	if ((y * 16 / 9) == x)
	{
		aspect->width  = 16;
		aspect->height = 9;
	}
	else if ((y * 16 / 10) == x)
	{
		aspect->width  = 16;
		aspect->height = 10;
	}
	else if ((y * 5 / 4) == x)
	{
		aspect->width  = 5;
		aspect->height = 4;
	}
	else if ((y * 15 / 9) == x)
	{
		aspect->width  = 15;
		aspect->height = 9;
	}
	else
	{
		aspect->width  = 4;
		aspect->height = 3;
	}
	PRINT("Aspect Ratio is %d/%d\n", aspect->width, aspect->height);
}


/*
 * initialize the mode tables chain
 *  tablesCount represents the number of VESA tables in the vBios
 *  return value is a pointer to the first table
 */
sModeTable * intializeTables(vBiosMap * map, int tablesCount) {
	map->modeTables = (sModeTable *)malloc(sizeof(sModeTable));
	sModeTable * table = map->modeTables;
	
	PRINT("Creating %d Mode Tables\n", tablesCount);
	
	int i = tablesCount;
	while ( i != 0 )
	{		
		table->id = tablesCount - i;
		PRINT("New table with id : %d\n", table->id);
		
		// opening the chain if it's the first table
		if (i == tablesCount)
			table->prev = NULL;
		else // attache the table to the chain
			table->prev = table; 
		
		//if there is more than one table, alloc the next one
		if (i > 1)
		{
			table->next = (sModeTable *)malloc(sizeof(sModeTable));
			table = table->next;
		}
		else // no more table, close the chain
			table->next = NULL;
		
		i--;
	}
	
	return map->modeTables;
}


//void closeVbios(vBiosMap * map); azi: dup - declared on header

vBiosMap * openVbios(chipsetType forcedChipset)
{
	uint32_t z;
	vBiosMap * map = NEW(vBiosMap);
	
	for(z = 0; z < sizeof(vBiosMap); z++)
		((char*)map)[z] = 0;
	
	/*
	 * Determine chipset
	 */
	
	if (forcedChipset == CT_UNKWN)
	{
		map->chipsetId = getChipsetId();
		map->chipset = getChipset(map->chipsetId);
		PRINT("Chipset is %s (pci id 0x%x)\n",chipsetTypeNames[map->chipset], map->chipsetId);
	}
	else if (forcedChipset != CT_UNKWN)
	{
		map->chipset = forcedChipset;
	}
	else
	{
		map->chipset = CT_915GM;
	}
	    
	/*
	 *  Map the video bios to memory
	 */
	
	map->biosPtr=(uint8_t*)VBIOS_START;
	
	/*
	 *  Common initialisation
	 */
	
	map->hasSwitched = false;
	
	/*
	 * check if we have ATI Radeon and open atombios
	 */
	atiBiosTables atiTables;
	
	atiTables.base = map->biosPtr;
	atiTables.atomRomHeader = (atomRomHeader *) (map->biosPtr + *(uint16_t *) (map->biosPtr + OFFSET_TO_POINTER_TO_ATOM_ROM_HEADER)); 
	
	if (strcmp ((char *) atiTables.atomRomHeader->firmWareSignature, "ATOM") == 0)
	{
		map->bios = BT_ATI_1;
		PRINT("We have an AtomBios Card\n");
		return openAtiVbios(map, atiTables);
	}


	/*
	 * check if we have NVidia
	 */
	if (map->bios != BT_ATI_1)
	{
		int i = 0;
		while (i < 512) // we don't need to look through the whole bios, just the first 512 bytes
		{
			if ((map->biosPtr[i] == 'N') 
				&& (map->biosPtr[i+1] == 'V') 
				&& (map->biosPtr[i+2] == 'I') 
				&& (map->biosPtr[i+3] == 'D')) 
			{
				map->bios = BT_NVDA;
				PRINT("We have an NVIDIA Card\n");
				return openNvidiaVbios(map);
				break;
			}
			i++;
		}
	}
	
	/*
	 * check if we have Intel
	 */
	    
	if ((map->bios != BT_ATI_1) && (map->bios != BT_NVDA))
	{
		int i = 0;
		while (i < VBIOS_SIZE)
		{
			if ((map->biosPtr[i] == 'I') 
				&& (map->biosPtr[i+1] == 'n') 
				&& (map->biosPtr[i+2] == 't') 
				&& (map->biosPtr[i+3] == 'e') 
				&& (map->biosPtr[i+4] == 'l')) 
			{
				map->bios = BT_1;
				PRINT("We have an Intel Card\n");
				saveVbios(map);
				return openIntelVbios(map);
				break;
			}
			i++;
		}
	}
	
	/*
	 * Unidentified Chipset
	 */
	
	if ( (map->chipset == CT_UNKWN) || ((map->bios != BT_ATI_1) && (map->bios != BT_NVDA) && (map->bios != BT_1)) )
	{
		PRINT("Unknown chipset type and unrecognized bios.\n");
		
		PRINT("autoresolution only works with Intel 800/900 series graphic chipsets.\n");
		
		PRINT("Chipset Id: %x\n", map->chipsetId);
		closeVbios(map);
		return 0;
	}

	/*
	 * Should never get there 
	 */
	return 0;
}

void closeVbios(vBiosMap * map)
{
	PRINT("Closing VBios\n");
	//make sure to turn autoResolution off
	if (gAutoResolution == TRUE)
		gAutoResolution = FALSE;
	
	// if we saved the vBios, free the copy
	if (map->biosBackupPtr != NULL)
	{
		PRINT("Freeing biosBackupPtr\t");
		FREE(map->biosBackupPtr);
		PRINT("[OK]\n");
	}
	
	// free table backups if any
	sModeTable * table = map->modeTables;
	while (table != NULL)
	{		
		if (table->backup != NULL)
		{
			PRINT("Table #%d : Freeing backup\t", table->id);
			FREE(table->backup);
			PRINT("[OK]\n");
		}
		
		if (table != NULL)
		{ 
			PRINT("Table #%d : Freeing\t\t", table->id);
			FREE(table);
			PRINT("[OK]\n");
		}
		
		if (table->next == NULL)
			break;
		
		table = table->next;
	}
	
	PRINT("Freeing map\t\t\t");
	FREE(map);
	PRINT("[OK]\n");
}

void unlockVbios(vBiosMap * map)
{

	map->unlocked = TRUE;
	    
	switch (map->chipset)
	{
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
		case CT_955X:
		case CT_G965:
		case CT_Q965:
		case CT_965GM:
		case CT_975X:
		case CT_P35:
		case CT_X48:
		case CT_B43:
		case CT_Q45:
		case CT_P45:
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
		uint32_t t = inl(0xcfc);
		PRINT("unlock PAM: (0x%08x)\n", t);
	}
#endif
}

void relockVbios(vBiosMap * map)
{

	map->unlocked = FALSE;
	
	switch (map->chipset)
	{
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
		case CT_955X:
		case CT_G965:
		case CT_Q965:
		case CT_965GM:
		case CT_975X:
		case CT_P35:
		case CT_X48:
		case CT_B43:
		case CT_Q45:
		case CT_P45:
		case CT_GM45:
		case CT_G41:
		case CT_G31:
		case CT_G45:
		case CT_500:
			
			outl(0xcf8, 0x80000090);
			outb(0xcfd, map->b1);
			outb(0xcfe, map->b2);
			break;
	}
	
	#if DEBUG
	{
        uint32_t t = inl(0xcfc);
		PRINT("relock PAM: (0x%08x)\n", t);
	}
	#endif
}

/*
 * saveVbios - save the entire vBios in case the patch has to be removed
 */
void saveVbios(vBiosMap * map)
{
	map->biosBackupPtr = malloc(VBIOS_SIZE);
	bcopy((const uint8_t *)0xC0000, map->biosBackupPtr, VBIOS_SIZE);
}

/*
 * restoreVbios - restore the vBios backup or table backups if any
 */
void restoreVbios(vBiosMap * map)
{
	if ((map->bios == BT_ATI_1) || (map->bios == BT_ATI_2) || (map->bios == BT_NVDA))
	{
		restoreTables(map, map->modeTables);
	}
	else
	{
		unlockVbios(map);
		bcopy(map->biosBackupPtr,(uint8_t *)0xC0000, VBIOS_SIZE);
		relockVbios(map);
	}
}

/*
 * saveTables - save the tables in case the patch has to be removed
 */
void saveTables(sModeTable * table)
{
	while (table != NULL)
	{
		table->backup = (uint8_t *)malloc(table->size);
		bcopy((const uint8_t *)table->pointer, table->backup, table->size);
		table = table->next;
	}
}

/*
 * restoreTables - restore tables backup 
 */
void restoreTables(vBiosMap * map, sModeTable * table)
{
	unlockVbios(map);
	while (table != NULL)
	{
		bcopy(table->backup, (uint8_t *)table->pointer, table->size);
		table = table->next;
	}
	relockVbios(map);
}

/*
 * patchVbios - call the vendor specific function to patch the VESA tables
 *   x & y are horizontal and vertical dimensions respectively, in pixels
 *   for GMA and ATI, only first mode in Table is patched
 *
 * each vendor specific function have the same form :
 *   bool vendorSetMode_type(sModeTable * table, uint8_t index, uint32_t * x, uint32_t * y);
 * where 'table' is the VESA table to patch, 'index' is the index of the mode in table,
 *       'x' & 'y' are pointer to the target resolution and return the next resolution in table.
 */
void patchVbios(vBiosMap * map, uint32_t x, uint32_t y, uint32_t bp, uint32_t hTotal, uint32_t vTotal)
{
	uint32_t i = 0;
	
	sModeTable * table = map->modeTables;
	
	// save the target resolution for future use
	map->currentX = x;
	map->currentY = y;
	
	unlockVbios(map);
	
	// Get the aspect ratio for the requested mode
	getAspectRatio(&map->aspectRatio, x, y);
	
	i = 0;
	
	// Call the vendor specific function for each available VESA Table
	table = map->modeTables;
	while (table != NULL)
	{
		//reset resolution value before treating a new table
		x = map->currentX;
		y = map->currentY;
		
		PRINT("Patching Table #%d : \n", table->id);
		
		map->setMode(table, i, &x, &y);
#ifdef AUTORES_DEBUG
		getc();
#endif

		table = table->next;
	}
	
	relockVbios(map);
	return;
}  