/*
 *  edid.h
 *  
 *
 *  Created by Evan Lojewski on 12/1/09.
 *  Copyright 2009. All rights reserved.
 *
 */
//#ifndef __EDID_H__
//#define __EDID_H__


#include "libsaio.h"
#define EDID_BLOCK_SIZE	128
#define EDID_V1_BLOCKS_TO_GO_OFFSET 126

char* readEDID();
void getResolution(UInt32* x, UInt32* y, UInt32* bp);

//#endif