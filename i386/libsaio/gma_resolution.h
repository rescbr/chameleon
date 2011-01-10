/*
 *  gma_resolution.h
 *  
 *
 *  Created by Le Bidou on 19/03/10.
 *  Copyright 2010 ---. All rights reserved.
 *
 */

#ifndef _GMA_RESOLUTION_H_
#define _GMA_RESOLUTION_H_

#include "libsaio.h"
#include "autoresolution.h"

#define MODE_TABLE_OFFSET_845G 617
#define INTEL_SIGNATURE "Intel Corp"

typedef struct {
	uint8_t mode;
	uint8_t bitsPerPixel;
	uint16_t resolution;
	uint8_t unknown;
} __packed vbiosMode;

typedef struct {
	uint8_t unknow1[2];
	uint8_t hActive1;
	uint8_t xTotal;
	uint8_t hActive2;
	uint8_t vActive1;
	uint8_t yTotal;
	uint8_t vActive2;
} __packed vbiosResolutionType1;

typedef struct {
	uint32_t clock;
	
	uint16_t hActive1;
	uint16_t hTotal;
	uint16_t hActive2;
	uint16_t hBlank;
	uint16_t hSyncStart;
	uint16_t hSyncEnd;
	uint16_t vActive1;
    uint16_t vTotal;
    uint16_t vActive2;
	uint16_t vBlank;
	uint16_t vSyncStart;
	uint16_t vSyncEnd;
} __packed vbiosModelineType2;

typedef struct {
	uint8_t xChars;
	uint8_t yChars;
	uint8_t unknown[4];
	
	vbiosModelineType2 modelines[];
} __packed vbiosResolutionType2;

typedef struct {
	uint32_t clock;
	
	uint16_t hActive1;
	uint16_t hTotal;
	uint16_t hActive2;
	uint16_t hBlank;
	uint16_t hSyncStart;
	uint16_t hSyncEnd;
	
	uint16_t vActive1;
	uint16_t vTotal;
	uint16_t vActive2;
	uint16_t vBlank;
	uint16_t vSyncStart;
	uint16_t vSyncEnd;
	
	uint16_t timingH;
	uint16_t timingV;
	
	uint8_t unknown[6];
} __packed vbiosModelineType3;

typedef struct {
	unsigned char unknown[6];
	
    vbiosModelineType3 modelines[];
} __packed vbiosResolutionType3;


vbiosResolutionType1 * mapType1Resolution(vBiosMap * map, uint16_t res);
vbiosResolutionType2 * mapType2Resolution(vBiosMap * map, uint16_t res);
vbiosResolutionType3 * mapType3Resolution(vBiosMap * map, uint16_t res);

char detectBiosType(vBiosMap * map, char modeline, int entrySize);

vBiosMap * openIntelVbios(vBiosMap *);

bool intelSetMode_1(sModeTable* table, uint8_t idx, uint32_t* x, uint32_t* y);
bool intelSetMode_2(sModeTable* table, uint8_t idx, uint32_t* x, uint32_t* y);
bool intelSetMode_3(sModeTable* table, uint8_t idx, uint32_t* x, uint32_t* y);

#endif
