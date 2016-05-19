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
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)prf.c	7.1 (Berkeley) 6/5/86
 */

#include <stdarg.h>

#define SPACE	1
#define ZERO	2
#define UCASE	16
#define SIGNED	32

/*
 * Scaled down version of C Library printf.
 * Used to print diagnostic information directly on console tty.
 * Since it is not interrupt driven, all system activities are
 * suspended.
 *
 */

/*
 * Printn prints a number n in base b.
 * We don't use recursion to avoid deep kernel stacks.
 */
static int printn(
	unsigned long long n,
	int b,
	int flag,
	int minwidth,
	int (*putfn_p)(),
	void* putfn_arg
)
{
	char prbuf[22];
	register char *cp;
	int width = 0, neg = 0;

	if ((flag & SIGNED) && (long long)n < 0)
	{
		neg = 1;
		n = (unsigned long long)(-(long long)n);
	}
	cp = prbuf;
	do
	{
		*cp++ = "0123456789abcdef0123456789ABCDEF"[(flag & UCASE) + (int) (n%b)];
		n /= b;
		width++;
	} while (n);

	if (neg)
	{
		if (putfn_p)
		{
			(void)(*putfn_p)('-', putfn_arg);
		}
		width++;
	}
	if (!putfn_p)
	{
		return (width < minwidth) ? minwidth : width;
	}
	for (;width < minwidth; width++)
		(void)(*putfn_p)( (flag & ZERO) ? '0' : ' ', putfn_arg);

	do
		(void)(*putfn_p)(*--cp, putfn_arg);
	while (cp > prbuf);
	return width;
}

int prf(
	const char *fmt,
	va_list ap,
	int (*putfn_p)(),
	void *putfn_arg
)
{
	int b, c, len = 0;
	const char *s;
	int flag, width, ells;
	int minwidth;

loop:
	while ((c = *fmt++) != '%')
	{
		if(c == '\0')
		{
			return len;
		}

		if (putfn_p)
		{
			(void)(*putfn_p)(c, putfn_arg);
		}
		len++;
	}
	minwidth = 0;
	flag = 0;
	ells = 0;
again:
	c = *fmt++;
	switch (c)
	{
		case 'l':
			if (ells < 2)
			{
				++ells;
			}
			goto again;
		case ' ':
			flag |= SPACE;
			goto again;
		case '0':
			if (minwidth == 0)
			{
				/* this is a flag */
				flag |= ZERO;
				goto again;
			} /* fall through */
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			minwidth *= 10;
			minwidth += c - '0';
			goto again;
		case 'X':
			flag |= UCASE;
			/* fall through */
		case 'x':
			b = 16;
			goto number;
		case 'd':
			flag |= SIGNED;
			/* fall through */
		case 'u':
			b = 10;
			goto number;
		case 'o': case 'O':
			b = 8;
		number:
			switch (ells)
			{
			case 2:
				len += printn(va_arg(ap, unsigned long long), b, flag, minwidth, putfn_p, putfn_arg);
				break;
			case 1:
				len += printn(va_arg(ap, unsigned long), b, flag, minwidth, putfn_p, putfn_arg);
				break;
			default:
				len += printn(va_arg(ap, unsigned int), b, flag, minwidth, putfn_p, putfn_arg);
				break;
			}
			break;
		case 's':
			s = va_arg(ap, const char*);
			width = 0;
			while (s && (c = *s++))
			{
				if (putfn_p)
				{
					(void)(*putfn_p)(c, putfn_arg);
				}
				len++;
				width++;
			}
			while (width++ < minwidth)
			{
				if (putfn_p)
				{
					(void)(*putfn_p)(' ', putfn_arg);
				}
				len++;
			}
			break;
		case 'c':
			if (putfn_p)
			{
				(void)(*putfn_p)((char) va_arg(ap, int), putfn_arg);
			}
			len++;
			break;
		default:
			break;
	}
	goto loop;
}
