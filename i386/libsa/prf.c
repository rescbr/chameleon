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

#define DIVIDEND_LOW  *(unsigned int*) dividend
#define DIVIDEND_HIGH ((unsigned int*) dividend)[1]

/*
 * Divides 64-bit dividend by 32-bit divisor.
 *   Quotient stored in dividend, remainder returned.
 * Assumes little-endian byte order.
 * Assumes divisor is non-zero.
 */
unsigned int i386_unsigned_div(
	unsigned long long* dividend,
	unsigned int divisor
	)
{
	unsigned int high = DIVIDEND_HIGH;

	if (high >= divisor)
	{
		__asm__ volatile ("xorl %%edx, %%edx; divl %2" : "=a"(DIVIDEND_HIGH), "=d"(high) : "r"(divisor), "a"(high));
	}
	else
	{
		DIVIDEND_HIGH = 0;
	}
	__asm__ volatile("divl %2" : "+a"(DIVIDEND_LOW), "+d"(high) : "r"(divisor));
	return high;
}

#undef DIVIDEND_HIGH
#undef DIVIDEND_LOW

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
	static const char hexdig[] = "0123456789abcdef0123456789ABCDEF";

	if ((flag & SIGNED) && (long long)n < 0)
	{
		neg = 1;
		n = (unsigned long long)(-(long long)n);
	}
	cp = prbuf;
	if ((b & -b) == b)	// b is a power of 2
	{
		unsigned int log2b = (unsigned int) (__builtin_ctz((unsigned int) b) & 31);
		unsigned int mask  = (unsigned int) (b - 1);
		do
		{
			*cp++ = hexdig[(flag & UCASE) + (int) (n & mask)];
			n >>= log2b;
			width++;
		}
		while (n);
	}
	else	// b is not a power of 2
	{
		do
		{
			*cp++ = hexdig[(flag & UCASE) + (int) i386_unsigned_div(&n, (unsigned int) b)];
			width++;
		}
		while (n);
	}

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

/*
 * Printp prints a pointer.
 */
static int printp(
	const void* p,
	int minwidth,
	int (*putfn_p)(),
	void* putfn_arg
	)
{
	int width = 0;

	if (p)
	{
		if (putfn_p)
		{
			(void)(*putfn_p)('0', putfn_arg);
			(void)(*putfn_p)('x', putfn_arg);
		}
		width = 2;
		minwidth = ((minwidth >= 2) ? (minwidth - 2) : 0);
	}
	return width + printn((unsigned long long) p, 16, ZERO, minwidth, putfn_p, putfn_arg);
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
		case 'i':
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
			if (!s)
			{
				s = "(null)";
			}
			width = 0;
			if (!putfn_p)
			{
				while ((c = *s++))
				{
					width++;
				}
				len += ((width < minwidth) ? minwidth : width);
				break;
			}
			while ((c = *s++))
			{
				(void)(*putfn_p)(c, putfn_arg);
				len++;
				width++;
			}
			while (width++ < minwidth)
			{
				(void)(*putfn_p)(' ', putfn_arg);
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
		case '%':
			if (putfn_p)
			{
				(void)(*putfn_p)('%', putfn_arg);
			}
			len++;
			break;
		case 'p':
			len += printp(va_arg(ap, const void*), minwidth, putfn_p, putfn_arg);
			break;
		case 'n':
			s = va_arg(ap, const char*);
			if (s)
			{
				*(int*) s = len;
			}
			break;
		default:
			break;
	}
	goto loop;
}
