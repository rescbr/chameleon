/*
 * Copyright (c) 1999-2004 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2004 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __BOOT2_OPTIONS_H
#define __BOOT2_OPTIONS_H

//#include "boot.h"
//#include "bootstruct.h"
//#include "graphics.h"

//AZI: KEEP THIS FILE ???

//extern int	gDeviceCount;
void clearBootArgs(void);

typedef struct {
    int x;
    int y;
    int type;
} CursorState;

enum {
    kMenuTopRow    = 5,
    kMenuMaxItems  = 10,
    kScreenLastRow = 24
};

/*
extern const MenuItem * gMenuItems;



void addBootArg(const char * argStr);
void changeCursor( int col, int row, int type, CursorState * cs );
void moveCursor( int col, int row );
void restoreCursor( const CursorState * cs );
void printMemoryInfo(void);
void lspci(void);
void printMenuItem( const MenuItem * item, int highlight );
bool flushKeyboardBuffer(void);


extern  bool shouldboot;

#ifdef UNUSED
extern int multiboot_timeout;
extern int multiboot_timeout_set;
#endif

extern BVRef    bvChain;
//extern int		menucount;




//==========================================================================

extern  char   gBootArgs[BOOT_STRING_LEN];
extern  char * gBootArgsPtr;
extern  char * gBootArgsEnd;
extern  char   booterCommand[BOOT_STRING_LEN];
extern  char   booterParam[BOOT_STRING_LEN];


//==========================================================================

extern  int   gMenuItemCount;
extern  int   gMenuRow;
extern  int   gMenuHeight;
extern  int   gMenuTop;
extern  int   gMenuBottom;
extern  int   gMenuSelection;

extern  int	 gMenuStart;
extern  int	 gMenuEnd;

extern unsigned char chainbootdev;
extern unsigned char chainbootflag;


// Maximum config table value size
#define VALUE_SIZE 2048
*/
#endif /* __BOOT2_OPTIONS_H */