/*
 *  nviviaresolution.h
 *  
 *
 *  Created by Le Bidou on 19/03/10.
 *  Copyright 2010 ---. All rights reserved.
 *
 */

#ifndef _NVDA_RESOLUTION_HEADER_
#define _NVDA_RESOLUTION_HEADER_

#include "libsaio.h" // not worth it; 4 out of 5 are needed.
#include "autoresolution.h"

#define NVIDIA_SIGNATURE "NVIDIA Corp"

#define OFFSET_TO_VESA_TABLE_INDEX 2
#define MAIN_VESA_TABLE 0
#define SECOND_VESA_TABLE 1

#define HSyncPolarityMask		0x4
#define VSyncPolarityMask		0x2
#define isGraphicsModeMask		0x1

typedef struct {
	unsigned char	tableMajor; //These names are probably wrong
	unsigned char	tableMinor;
	unsigned char	tableRev;
	unsigned short	tableSize;
}__packed nvCommonTableHeader;

typedef struct {
	unsigned short	clock;
	unsigned short	hActive;
	unsigned short  hActiveMinusOne;
	unsigned short	reserved1;
	unsigned short  hActiveMinusOne_;
	unsigned short	hSyncStart;
	unsigned short	hSyncEnd;
	unsigned short	hTotal;
	unsigned short	vActive;
	unsigned short  vActiveMinusOne;
	unsigned short	reserved2;
	unsigned short  vActiveMinusOne_;
	unsigned short	vSyncStart;
	unsigned short	vSyncEnd;
	unsigned short	vTotal;
	unsigned short	reserved3;
}__packed nvModeline;

typedef struct {
	unsigned short hActive;
	unsigned short vActive;
	unsigned char  hBlank;
	unsigned char  hSyncOffset;
	unsigned char  hSyncWidth;
	unsigned char  vBlank;
	//unsigned char  vSyncwidth;
	unsigned char  flags; //looks like flags & 1 means "Graphics Mode", to oppose to "Console Mode"
	//on 7xxx the high four bits look like a mode id number.
	//on 8xxx only the low four bits are used, standard graphics mode are always 5.
	//		it can be 1 (1400x1050 and 2048x1536) (HSync High, VSync High ?)
	//				  3 (1440x900, 1680x1050 and 1920x1200) (HSync High, VSync Low ?)
	//				  5 (Standard Timings) (HSync Low, VSync High ?)
	//			   or 7 (1280x800 and 768x340) (HSync Low, VSync Low ?)
}__packed nvModeline_2;

typedef struct {
	nvCommonTableHeader	header;
	nvModeline	*			modelines;
}__packed nvVesaTable;

vBiosMap * openNvidiaVbios(vBiosMap *map);

bool nvidiaSetMode(sModeTable* table, uint8_t idx, uint32_t* x, uint32_t* y);

#endif