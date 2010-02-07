/*
 *  edid.h
 *  
 *
 *  Created by Evan Lojewski on 12/1/09.
 *  Copyright 2009. All rights reserved.
 *
 */


#define EDID_BLOCK_SIZE	128
#define EDID_V1_BLOCKS_TO_GO_OFFSET 126

#define SERVICE_REPORT_DDC	0
#define SERVICE_READ_EDID	1
#define SERVICE_LAST		1  // Read VDIF has been removed from the spec.

#define FUNC_GET_EDID		0x4F15


char* readEDID();
void getResolution(UInt32* x, UInt32* y, UInt32* bp);