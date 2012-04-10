/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
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
/* 
 * Mach Operating System
 * Copyright (c) 1990 Carnegie-Mellon University
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

/*
 * 			INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license  agreement or 
 *	nondisclosure agreement with Intel Corporation and may not be copied 
 *	nor disclosed except in accordance with the terms of that agreement.
 *
 *	Copyright 1988, 1989 Intel Corporation
 */

/*
 * Copyright 1993 NeXT, Inc.
 * All rights reserved.
 */

#include "libsaio.h"

/*
 * keyboard controller (8042) I/O port addresses
 */
#define PORT_A      0x60    /* port A */
#define PORT_B      0x64    /* port B */

/*
 * keyboard controller command
 */
#define CMD_WOUT    0xd1    /* write controller's output port */

/*
 * keyboard controller status flags
 */
#define KB_INFULL   0x2     /* input buffer full */
#define KB_OUTFULL  0x1     /* output buffer full */

#define KB_A20      0x9f    /* enable A20,
                               enable output buffer full interrupt
                               enable data line
                               disable clock line */

//==========================================================================
// Enable A20 gate to be able to access memory above 1MB
static inline void flushKeyboardInputBuffer();
static inline void flushKeyboardInputBuffer()
{
    unsigned char ret;
    /* Apparently all flags on means that they're invalid and that the code
       should stop trying to check them because they'll never change */
    do
    {
        ret = inb(PORT_B);
    } while( (ret != 0xff) && (ret & KB_INFULL));
}

void enableA20()
{
    /* make sure that the input buffer is empty */
    flushKeyboardInputBuffer();

    /* make sure that the output buffer is empty */
    if (inb(PORT_B) & KB_OUTFULL)
        (void)inb(PORT_A);

    /* make sure that the input buffer is empty */
    flushKeyboardInputBuffer();

    /* write output port */
    outb(PORT_B, CMD_WOUT);
    delay(100);

    /* wait until command is accepted */
    flushKeyboardInputBuffer();

    outb(PORT_A, KB_A20);
    delay(100);

    /* wait until done */
    flushKeyboardInputBuffer();
}
#if UNUSED
void turnOffFloppy(void)
{
	/*
	 * Disable floppy:
	 * Hold controller in reset,
	 * disable DMA and IRQ,
	 * turn off floppy motors.
	 */
	outb(0x3F2, 0x00);
}
#endif

static bool fix_random=true;

struct ran_obj* random_init (int rmin, int rmax)
{
	if (rmin > rmax) 
		return false;
	
    int n = (rmax+1) - rmin;
	
	int tab[rand_tab_len];
	
	struct ran_obj * self = (struct ran_obj * )malloc(sizeof(struct ran_obj));	
	if (!self) {
        return NULL;
    }
    bzero(self,sizeof(struct ran_obj));
    
	self->rmin= rmin;
	
	self->n= n;
	
	int i;
	srand(time18());
	int limit1= 0, limit2= 0, gate1 = 0, gate2 = 0;
	
	if (fix_random) {
		limit1 = (rand() % 20) ;		
		limit2 =  (rand() % 20)  ;
	}
	
	for (i = 0; i < rand_tab_len; i++){
		
		tab[i] = (rand() % n) + rmin;
		
		if (fix_random) {			
			
			if (i > 1 && gate1 < limit1 && tab[i]==tab[i-2]) {
				i--;
				gate1++;
				
				continue;
			}
			
			if (i > 7 && gate2 < limit2 && tab[i]==tab[i-((rand() % 4)+5)]) {
				i--;
				gate2++;
				
				continue;
			}
			
		}
		self->tab[i]= tab[i];		
	}
	
	
	return self;
}

int random (struct ran_obj* self)
{
		
    struct ran_obj * ret = self ;
    
	static int wheel = 0;
	int gate3 = 0;
	int limit3 = 0;	
	static int retlst[rand_tab_len];
	
	
	int index;
	int rn;	
	
    if (!ret) {
		return -1;	// TODO: fix this	
	}
    
	if (fix_random) {
		gate3 = rand() % 2;		
	}
	
	
retry:	
	index = rand() % rand_tab_len;	
	rn = ret->tab[index];	
	ret->tab[index] = (rand() % ret->n) + ret->rmin;
	
	
	if (fix_random) {	
		if ((gate3 && limit3 < 5) && (ret->tab[index] == rn)) {
			limit3++;
			goto retry;
		}	
		retlst[wheel] = rn;
		
		if (wheel > 0 && limit3 < 5) {	
			
			if (gate3 && (rn == retlst[wheel-1])) {			
				limit3++;
				goto retry;
			}
			
			if (gate3 && (wheel > 3 && rn==retlst[wheel-((rand() % 3)+1)]) && (limit3 < 5)) {			
				limit3++;
				goto retry;
			}
		}
		
		wheel++;
	}
	
	self = ret;	
	return rn;	
}

void random_free (struct ran_obj* self)
{
    if (self /* && self->sgn == random_obj_sgn */) {
        free(self);
    }    
    
}

void usefixedrandom (bool opt)
{
	fix_random = opt;
}

//==========================================================================
// Return the platform name for this hardware.
//
#ifndef BOOT1
void
getPlatformName(char *nameBuf)
{
    strcpy(nameBuf, "ACPI");
}
#endif