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
#include "bootstruct.h"
#include "platform.h"

/* Kabyl: BooterLog */
#define BOOTER_LOG_SIZE    (128 * 1024)
#define SAFE_LOG_SIZE    134

#define LOG    1
#define PRINT  2

struct log_t {
    char *buf;
    char *ptr;
};
typedef struct log_t log_t;
static log_t booterlog;

void initBooterLog(void)
{
	booterlog.buf = malloc(BOOTER_LOG_SIZE);
    if (!booterlog.buf) {
        printf("Couldn't allocate buffer for booter log\n");
        booterlog.ptr = 0;
        booterlog.buf = 0;
        return;
    }
	bzero(booterlog.buf, BOOTER_LOG_SIZE);
	booterlog.ptr = booterlog.buf;    
	
}

void
debug_putc(char c)
{
    if (((booterlog.ptr-booterlog.buf) < (BOOTER_LOG_SIZE - SAFE_LOG_SIZE))) {
        *booterlog.ptr=c;
        booterlog.ptr++;
    }
}

void setupBooterLog(void)
{
	if (!booterlog.buf)
		return;	
    
	Node *node = DT__FindNode("/", false);
	if (node)
		DT__AddProperty(node, "boot-log", strlen((char *)booterlog.buf) + 1, booterlog.buf);
}

/* Kabyl: !BooterLog */


/*
 * write one character to console
 */
void putchar(char c)
{
	
	if ( c == '\t' )
	{
		for (c = 0; c < 8; c++) putc(' ');
		return;
	}
	
	if ( c == '\n' )
    {
		putc('\r');
    }
	
	putc(c);
}

int getc()
{	
    int c = bgetc();	
	
    if ((c & 0xff) == 0) 		
        return c;
    else 	
		return (c & 0xff); 
}

// Read and echo a character from console.  This doesn't echo backspace
// since that screws up higher level handling

int getchar()
{
	register int c = getc();
	
	if ( c == '\r' ) c = '\n';
	
	if ( c >= ' ' && c < 0x7f) putchar(c);
	
	return (c);
}

int
reallyVPrint(const char *format, va_list ap, int flag)
{
    if (flag & PRINT) prf(format, ap, putchar);
	
    if (flag & LOG)
	{
		/* Kabyl: BooterLog */		
		prf(format, ap, debug_putc);
	}
    return 0;
}

int localVPrintf(const char *format, va_list ap, int flag)
{
    /**/
    
    reallyVPrint(format, ap, flag);
    return 0;
}

int printf(const char * fmt, ...)
{
    va_list ap;
	va_start(ap, fmt);
	
	localVPrintf(fmt, ap, LOG | PRINT);
	
	va_end(ap);    
    
    return 0;
}

void msglog(const char * fmt, ...)
{
    va_list ap;
	va_start(ap, fmt);	
	
    localVPrintf(fmt, ap, LOG);
	
	va_end(ap);    
}

int verbose(const char * fmt, ...)
{
    va_list ap;
    int flag = 0;
	va_start(ap, fmt);
    if (get_env(envgVerboseMode))
    {
		flag = PRINT;
    }
	
    localVPrintf(fmt, ap, LOG | flag);
	
    va_end(ap);    
    
    return(0);
}

int error(const char * fmt, ...)
{
    va_list ap;
	int len;
	char *str = NULL;
	
    va_start(ap, fmt);
    
    localVPrintf(fmt, ap, 0);
	
    
	len = prf(fmt, ap, 0);
	if (len > 0)
	{
		str = newEmptyStringWithLength(len);
		if (str != NULL) 
		{			
			vsnprintf(str,len,fmt,ap);
		}
		
	}	
    va_end(ap);
	
	set_env_ptr(envConsoleErr, str, len);
	free(str);
	
    return(0);
}

void stop(const char * fmt, ...)
{
	va_list ap;
	
	printf("\n");
	va_start(ap, fmt);
	
    localVPrintf(fmt, ap, PRINT);
	
	va_end(ap);
	printf("\nThis is a non recoverable error! System HALTED!!!");
	halt();
	while (1);
}

/** Print a "Press a key to continue..." message and wait for a key press. */
void pause(void) 
{
    printf("Press a key to continue...");
    getc();
}

char * newStringWithFormat(const char * fmt, ...)
{
    va_list ap;
	int len;
	char *str = NULL;
	
    va_start(ap, fmt);
	len = prf(fmt, ap, 0);
	if (len > 0)
	{
		str = newEmptyStringWithLength(len);
		if (str != NULL) 
		{			
			vsnprintf(str,len,fmt,ap);
		}
		
	}	
    va_end(ap);	
	
	return str;
	
}