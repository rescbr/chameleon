
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
#ifndef __915_RESOLUTION_H
#define __915_RESOLUTION_H

#include "edid.h"

#if DEBUG
#ifndef AUTORES_DEBUG
#define AUTORES_DEBUG
#endif
#endif

#ifdef AUTORES_DEBUG
#define PRINT(a, b...) printf(a, ##b);
#else 
#define PRINT(a, b...) verbose(a, ##b);
#endif

#define __packed __attribute__((packed))

#define NEW(a) ((a *)(malloc(sizeof(a))))
#define FREE(a) (free(a))

#define VBIOS_START         0xc0000
#define VBIOS_SIZE          0x10000

#define FALSE 0
#define TRUE 1


bool gAutoResolution;


typedef struct
{
	uint8_t width;
	uint8_t height;
} sAspect;


typedef enum
{
	CT_UNKWN, CT_830, CT_845G, CT_855GM, CT_865G, 
	CT_915G, CT_915GM, CT_945G, CT_945GM, CT_945GME, CT_946GZ, 
	CT_950GM, CT_955X, CT_G965, CT_Q965, CT_965GM, CT_975X, 
	CT_P35, CT_X48, CT_B43, CT_Q45, CT_P45,
	CT_GM45, CT_G41, CT_G31, CT_G45, CT_500, CT_3150
} chipsetType;


typedef enum
{
	BT_UNKWN, BT_1, BT_2, BT_3, BT_ATI_1, BT_ATI_2, BT_NVDA
} biosType;

typedef struct
{
	uint32_t	clock;
	uint16_t	x;
	uint16_t	hSyncStart;
	uint16_t	hSyncEnd;
	uint16_t	hTotal;
	uint16_t	y;
	uint16_t	vSyncStart;
	uint16_t	vSyncEnd;
	uint16_t	vTotal;
} generic_modeline;

typedef struct sModeTable_
{
	uint8_t	*	pointer;
	
	uint8_t		id;
	uint32_t	size;
	uint32_t	modeCount;
	
	uint8_t *	backup;
	
	struct sModeTable_ *prev;
	struct sModeTable_ *next;
} sModeTable;
	


typedef struct
{
	uint32_t chipsetId;
	chipsetType chipset;
	biosType bios;
	
	uint8_t* biosBackupPtr;
	uint8_t* biosPtr;
	
	sModeTable *modeTables;
	
	uint32_t currentX, currentY;
	uint8_t b1, b2;
	
	bool hasSwitched;
	
	bool (*setMode)(sModeTable *,uint8_t,uint32_t*,uint32_t*);
	
	sAspect aspectRatio;
	
	uint8_t unlocked;
} vBiosMap;

vBiosMap *map;

vBiosMap * openVbios(chipsetType type);
void closeVbios (vBiosMap* map);

void unlockVbios(vBiosMap* map);
void relockVbios(vBiosMap* map);

void saveVbios(vBiosMap* map);
void restoreVbios(vBiosMap* map);

void saveTables(sModeTable * table);
void restoreTables(vBiosMap * map, sModeTable * table);

void gtfTimings(uint32_t x, uint32_t y, uint32_t freq,
				 uint32_t *clock,
				 uint16_t *hSyncStart, uint16_t *hSyncEnd, uint16_t *hBlank,
				 uint16_t *vSyncStart, uint16_t *vSyncEnd, uint16_t *vBlank);

sModeTable * intializeTables(vBiosMap * map, int tablesCount);

void patchVbios(vBiosMap* map, uint32_t x, uint32_t y, uint32_t bp, uint32_t hTotal, uint32_t vTotal);

#endif
