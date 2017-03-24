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
#include <vers.h>

extern int	vprf(const char * fmt, va_list ap);

bool gVerboseMode = false;
bool gErrors = false;

/*
 *  Azi: Doubled available log size; this seems to fix some hangs and instant reboots caused by
 *  booting with -f (ignore caches). 96kb are enough to hold full log, booting with -f; even so,
 *  this depends on how much we "play" at the boot prompt and with what patches we're playing,
 *  depending on how much they print to the log.
 *	
 *  Kabyl: BooterLog
 */
#define BOOTER_LOG_SIZE	(128 * 1024)
#define SAFE_LOG_SIZE	134

char *msgbuf = 0;
char *cursor = 0;

// Bungo:
typedef struct {
	uint16_t year;
	uint8_t  mon;
	uint8_t  day;
	uint8_t  hour;
	uint8_t  mins;
	uint8_t  secs;
	uint8_t  dlight;
} datetime_t;
static datetime_t datetime;

struct putc_info //Azi: exists on gui.c & printf.c
{
	char * str;
	char * last_str;
};

static int
sputc(int c, struct putc_info * pi) //Azi: same as above
{
	if (pi->last_str)
	if (pi->str == pi->last_str)
	{
		*(pi->str) = '\0';
		return 0;
	}
	*(pi->str)++ = c;
	return c;
}

uint64_t getRTCdatetime() // 0xYYYYMMDDHHMMSS0L in decimal
{
	biosBuf_t bb;

	bb.intno = 0x1a;
	bb.eax.r.h = 0x04; // get RTC date
	//bb.flags.cf = 0;
	bios(&bb);
	if (bb.flags.cf) return 0;
	datetime.year = (bb.ecx.r.h >> 4) * 1000 + (bb.ecx.r.h & 0x0F) * 100 + (bb.ecx.r.l >> 4) * 10 + (bb.ecx.r.l & 0x0F) * 1;
	datetime.mon = (bb.edx.r.h >> 4) * 10 + (bb.edx.r.h & 0x0F) * 1;
	datetime.day = (bb.edx.r.l >> 4) * 10 + (bb.edx.r.l & 0x0F) * 1;

	bb.intno = 0x1a;
	bb.eax.r.h = 0x02; // get RTC time
	//bb.flags.cf = 0;
	bios(&bb);
	if (bb.flags.cf) return 0;
	datetime.dlight = bb.edx.r.l & 0x0F;
	datetime.hour = (bb.ecx.r.h >> 4) * 10 + (bb.ecx.r.h & 0x0F) * 1;
	datetime.mins = (bb.ecx.r.l >> 4) * 10 + (bb.ecx.r.l & 0x0F) * 1;
	datetime.secs = (bb.edx.r.h >> 4) * 10 + (bb.edx.r.h & 0x0F) * 1;
	return *(uint64_t *)&datetime;
}

void initBooterLog(void)
{
	msgbuf = malloc(BOOTER_LOG_SIZE);
	bzero(msgbuf, BOOTER_LOG_SIZE);
	cursor = msgbuf;
	msglog("%s\n", "Chameleon " I386BOOT_CHAMELEONVERSION " (svn-r" I386BOOT_CHAMELEONREVISION ")" " [" I386BOOT_BUILDDATE "]");
	getRTCdatetime();
	msglog("Logging started: %04d/%02d/%02d, %02d:%02d:%02d\n", datetime.year, datetime.mon, datetime.day, datetime.hour, datetime.mins, datetime.secs);
}

int msglog(const char * fmt, ...)
{
	va_list ap;
	struct putc_info pi;

	if (!msgbuf)
	{
		return 0;
	}

	if (((cursor - msgbuf) > (BOOTER_LOG_SIZE - SAFE_LOG_SIZE)))
	{
		return 0;
	}

	va_start(ap, fmt);
	pi.str = cursor;
	pi.last_str = 0;
	prf(fmt, ap, sputc, &pi);
	va_end(ap);
	cursor += strlen((char *)cursor);

	return 0;
}

void setupBooterLog(void)
{
	if (!msgbuf)
	{
		return;
	}

	Node *node = DT__FindNode("/", false);
	if (node)
		DT__AddProperty(node, "boot-log", strlen((char *)msgbuf) + 1, msgbuf);
}
/* Kabyl: !BooterLog */

/*
 * write one character to console
 */
int putchar(int c)
{
	if ( c == '\t' ) {
		for (c = 0; c < 8; c++) bios_putchar(' ');
		return c;
	}

	if ( c == '\n' ) {
		bios_putchar('\r');
	}

	bios_putchar(c);
    
	return c;
}

int getc()
{
	int c = bgetc();

	if ((c & 0xff) == 0) {
		return c;
	} else {
		return (c & 0xff);
	}
}

// Read and echo a character from console.  This doesn't echo backspace
// since that screws up higher level handling

int getchar()
{
	register int c = getc();
	return (c);
}

int printf(const char * fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (bootArgs->Video.v_display == VGA_TEXT_MODE)
	{
		prf(fmt, ap, putchar, 0);
	}
	else
	{
		vprf(fmt, ap);
	}
	va_end(ap);
	return 0;
}

int verbose(const char * fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (gVerboseMode)
	{
		if (bootArgs->Video.v_display == VGA_TEXT_MODE)
		{
			prf(fmt, ap, putchar, 0);
		}
		else
		{
			vprf(fmt, ap);
		}
	}
	va_end(ap);

	{
		// Kabyl: BooterLog
		struct putc_info pi;

		if (!msgbuf) {
			return 0;
		}

		if (((cursor - msgbuf) > (BOOTER_LOG_SIZE - SAFE_LOG_SIZE))) {
			return 0;
		}
		pi.str = cursor;
		pi.last_str = 0;
		va_start(ap, fmt);
		prf(fmt, ap, sputc, &pi);
		va_end(ap);
		cursor +=  strlen((char *)cursor);
	}

	return(0);
}

int error(const char * fmt, ...)
{
	va_list ap;
	gErrors = true;
	va_start(ap, fmt);
	if (bootArgs->Video.v_display == VGA_TEXT_MODE)
	{
		prf(fmt, ap, putchar, 0);
	}
	else
	{
		vprf(fmt, ap);
	}
	va_end(ap);

	// Kabyl: BooterLog
	struct putc_info pi;

	if (!msgbuf)
	{
		return 0;
	}

	if (((cursor - msgbuf) > (BOOTER_LOG_SIZE - SAFE_LOG_SIZE)))
	{
		return 0;
	}

	pi.str = cursor;
	pi.last_str = 0;
	va_start(ap, fmt);
	prf(fmt, ap, sputc, &pi);
	va_end(ap);
	cursor +=  strlen((char *)cursor);

	return(0);
}

void stop(const char * fmt, ...)
{
	va_list ap;

	printf("\n");
	va_start(ap, fmt);
	if (bootArgs->Video.v_display == VGA_TEXT_MODE)
	{
		prf(fmt, ap, putchar, 0);
	}
	else
	{
		vprf(fmt, ap);
	}
	va_end(ap);
	printf("\nThis is a non recoverable error! System HALTED!!!");
	halt();
	while (1);
}

/** Print a "Press a key to continue..." message and wait for a key press. */
void pause()
{
	printf("Press a key to continue...\n");
	getchar(); // replace getchar() by pause() were useful.
}
