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

#define kScreenWidth		"ScreenWidth"		
#define kScreenHeight		"ScreenHeight"		

char* readEDID(void);
void getResolution(UInt32* x, UInt32* y, UInt32* bp);
void getResolutionHook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);