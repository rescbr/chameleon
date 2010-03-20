/*
 *  ati_resolution.h
 *  
 *
 *  Created by Le Bidou on 19/03/10.
 *  Copyright 2010 ---. All rights reserved.
 *
 */

#ifndef _ATI_RESOLUTION_H_
#define _ATI_RESOLUTION_H_

#include "libsaio.h"
#include "autoresolution.h"
#include "shortatombios.h"

#define ATI_SIGNATURE1 "ATI MOBILITY RADEON"
#define ATI_SIGNATURE2 "ATI Technologies Inc"

typedef struct {
    unsigned char         *base;
    ATOM_ROM_HEADER  *AtomRomHeader;
    unsigned short         *MasterCommandTables;
    unsigned short         *MasterDataTables;
} bios_tables_t;

char detect_ati_bios_type(vbios_map * map);

vbios_map * open_ati_vbios(vbios_map * map, bios_tables_t ati_tables);

bool ati_set_mode_1(vbios_map* map, UInt8 idx, UInt32* x, UInt32* y);
bool ati_set_mode_2(vbios_map* map, UInt8 idx, UInt32* x, UInt32* y);

#endif