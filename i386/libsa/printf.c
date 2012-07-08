/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*-
 * Copyright (c) 1986, 1988, 1991, 1993
 *      The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      @(#)subr_prf.c  8.4 (Berkeley) 5/4/95
 */
/*
 * @OSF_COPYRIGHT@
 */

/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */


#include "libsa.h"

struct snprintf_arg {
    char *str;
    size_t remain;
};

static void
dummy_putc(int ch, void *arg)
{
    void (*real_putc)() = arg;
    
    if (real_putc) real_putc(ch);
}

#if 0
void 
_doprnt(
        register const char     *fmt,
        va_list           argp,
        /* character output routine */
        void                    (*putc)(),
        int                     radix)          /* default radix - for '%r' */
{
    __doprnt(fmt, argp, dummy_putc, putc, radix);
}
#endif
#if 1
#define Ctod(c) ((c) - '0')

#define MAXBUF (sizeof(long long int) * 8)	/* enough for binary */
static char digs[] = "0123456789abcdef";
static int
printnum(
         unsigned long long int	u,	/* number to print */
         int		base,
         void			(*putc)(int, void *),
         void                    *arg)
{
	char	buf[MAXBUF];	/* build number here */
	char *	p = &buf[MAXBUF-1];
	int nprinted = 0;
    
	do {
	    *p-- = digs[u % base];
	    u /= base;
	} while (u != 0);
    
	while (++p != &buf[MAXBUF]) {
	    if (putc) (*putc)(*p, arg);
	    nprinted++;
	}
    
	return nprinted;
}

bool	_doprnt_truncates = false;

int
__doprnt(
         const char	*fmt,
         va_list			argp,
         /* character output routine */
         void			(*putc)(int ch, void *arg),
         void                    *arg,
         int			radix)		/* default radix - for '%r' */
{
	int		length;
	int		prec;
	bool	ladjust;
	char		padc;
	long long		n;
	unsigned long long	u;
	int		plus_sign;
	int		sign_char;
	bool	altfmt, truncate;
	int		base;
	char	c;
	int		capitals;
	int		long_long;
	int             nprinted = 0;
    
	while ((c = *fmt) != '\0') {
	    if (c != '%') {
            if (putc) {
                (*putc)(c, arg);
            }
            nprinted++;
            fmt++;
            continue;
	    }
        
	    fmt++;
        
	    long_long = 0;
	    length = 0;
	    prec = -1;
	    ladjust = false;
	    padc = ' ';
	    plus_sign = 0;
	    sign_char = 0;
	    altfmt = false;
        
	    while (true) {
            c = *fmt;
            if (c == '#') {
                altfmt = true;
            }
            else if (c == '-') {
                ladjust = true;
            }
            else if (c == '+') {
                plus_sign = '+';
            }
            else if (c == ' ') {
                if (plus_sign == 0)
                    plus_sign = ' ';
            }
            else
                break;
            fmt++;
	    }
        
	    if (c == '0') {
            padc = '0';
            c = *++fmt;
	    }
        
	    if (isdigit(c)) {
            while(isdigit(c)) {
                length = 10 * length + Ctod(c);
                c = *++fmt;
            }
	    }
	    else if (c == '*') {
            length = va_arg(argp, int);
            c = *++fmt;
            if (length < 0) {
                ladjust = !ladjust;
                length = -length;
            }
	    }
        
	    if (c == '.') {
            c = *++fmt;
            if (isdigit(c)) {
                prec = 0;
                while(isdigit(c)) {
                    prec = 10 * prec + Ctod(c);
                    c = *++fmt;
                }
            }
            else if (c == '*') {
                prec = va_arg(argp, int);
                c = *++fmt;
            }
	    }
        
	    if (c == 'l') {
            c = *++fmt;	/* need it if sizeof(int) < sizeof(long) */
            if (sizeof(int)<sizeof(long))
                long_long = 1;
            if (c == 'l') {
                long_long = 1;
                c = *++fmt;
            }	
	    } else if (c == 'q' || c == 'L') {
	    	long_long = 1;
            c = *++fmt;
	    } 
        
	    truncate = false;
	    capitals=0;		/* Assume lower case printing */
        
	    switch(c) {
            case 'b':
            case 'B':
            {
                register char *p;
                boolean_t	  any;
                register int  i;
                
                if (long_long) {
                    u = va_arg(argp, unsigned long long);
                } else {
                    u = va_arg(argp, unsigned int);
                }
                p = va_arg(argp, char *);
                base = *p++;
                nprinted += printnum(u, base, putc, arg);
                
                if (u == 0)
                    break;
                
                any = false;
                while ((i = *p++) != '\0') {
                    if (*fmt == 'B')
                        i = 33 - i;
                    if (*p <= 32) {
                        /*
                         * Bit field
                         */
                        register int j;
                        if (any)
                            if (putc) (*putc)(',', arg);
                            else {
                                if (putc) (*putc)('<', arg);
                                any = true;
                            }
                        nprinted++;
                        j = *p++;
                        if (*fmt == 'B')
                            j = 32 - j;
                        for (; (c = *p) > 32; p++) {
                            if (putc) (*putc)(c, arg);
                            nprinted++;
                        }
                        nprinted += printnum((unsigned)( (u>>(j-1)) & ((2<<(i-j))-1)),
                                             base, putc, arg);
                    }
                    else if (u & (1<<(i-1))) {
                        if (any)
                            if (putc) (*putc)(',', arg);
                            else {
                                if (putc) (*putc)('<', arg);
                                any = true;
                            }
                        nprinted++;
                        for (; (c = *p) > 32; p++) {
                            if (putc) (*putc)(c, arg);
                            nprinted++;
                        }
                    }
                    else {
                        for (; *p > 32; p++)
                            continue;
                    }
                }
                if (any) {
                    if (putc) (*putc)('>', arg);
                    nprinted++;
                }
                break;
            }
                
            case 'c':
                c = va_arg(argp, int);
                if (putc) (*putc)(c, arg);
                nprinted++;
                break;
                
            case 's':
            {
                register const char *p;
                register const char *p2;
                
                if (prec == -1)
                    prec = 0x7fffffff;	/* MAXINT */
                
                p = va_arg(argp, char *);
                
                if (p == NULL)
                    p = "";
                
                if (length > 0 && !ladjust) {
                    n = 0;
                    p2 = p;
                    
                    for (; *p != '\0' && n < prec; p++)
                        n++;
                    
                    p = p2;
                    
                    while (n < length) {
                        if (putc) (*putc)(' ', arg);
                        n++;
                        nprinted++;
                    }
                }
                
                n = 0;
                
                while ((n < prec) && (!(length > 0 && n >= length))) {
                    if (*p == '\0') {
                        break;
                    }
                    if (putc) (*putc)(*p++, arg);
                    nprinted++;
                    n++;
                }
                
                if (n < length && ladjust) {
                    while (n < length) {
                        if (putc) (*putc)(' ', arg);
                        n++;
                        nprinted++;
                    }
                }
                
                break;
            }
                
            case 'o':
                truncate = _doprnt_truncates;
            case 'O':
                base = 8;
                goto print_unsigned;
                
            case 'D': {
                unsigned char *up;
                char *q, *p;
                
                up = (unsigned char *)va_arg(argp, unsigned char *);
                p = (char *)va_arg(argp, char *);
                if (length == -1)
                    length = 16;
                while(length--) {
                    if (putc) (*putc)(digs[(*up >> 4)], arg);
                    if (putc) (*putc)(digs[(*up & 0x0f)], arg);
                    nprinted += 2;
                    up++;
                    if (length) {
                        for (q=p;*q;q++) {
                            if (putc) (*putc)(*q, arg);
                            nprinted++;
                        }
                    }
                }
                break;
            }
                
            case 'd':
                truncate = _doprnt_truncates;
                base = 10;
                goto print_signed;
                
            case 'u':
                truncate = _doprnt_truncates;
            case 'U':
                base = 10;
                goto print_unsigned;
                
            case 'p':
                altfmt = true;
                if (sizeof(int)<sizeof(void *)) {
                    long_long = 1;
                }
            case 'x':
                truncate = _doprnt_truncates;
                base = 16;
                goto print_unsigned;
                
            case 'X':
                base = 16;
                capitals=16;	/* Print in upper case */
                goto print_unsigned;
                
            case 'z':
                truncate = _doprnt_truncates;
                base = 16;
                goto print_signed;
                
            case 'Z':
                base = 16;
                capitals=16;	/* Print in upper case */
                goto print_signed;
                
            case 'r':
                truncate = _doprnt_truncates;
            case 'R':
                base = radix;
                goto print_signed;
                
            case 'n':
                truncate = _doprnt_truncates;
            case 'N':
                base = radix;
                goto print_unsigned;
                
            print_signed:
                if (long_long) {
                    n = va_arg(argp, long long);
                } else {
                    n = va_arg(argp, int);
                }
                if (n >= 0) {
                    u = n;
                    sign_char = plus_sign;
                }
                else {
                    u = -n;
                    sign_char = '-';
                }
                goto print_num;
                
            print_unsigned:
                if (long_long) {
                    u = va_arg(argp, unsigned long long);
                } else { 
                    u = va_arg(argp, unsigned int);
                }
                goto print_num;
                
            print_num:
            {
                char	buf[MAXBUF];	/* build number here */
                register char *	p = &buf[MAXBUF-1];
                static char digits[] = "0123456789abcdef0123456789ABCDEF";
                const char *prefix = NULL;
                
                if (truncate) u = (long long)((int)(u));
                
                if (u != 0 && altfmt) {
                    if (base == 8)
                        prefix = "0";
                    else if (base == 16)
                        prefix = "0x";
                }
                
                do {
                    /* Print in the correct case */
                    *p-- = digits[(u % base)+capitals];
                    u /= base;
                } while (u != 0);
                
                length -= (int)(&buf[MAXBUF-1] - p);
                if (sign_char)
                    length--;
                if (prefix)
                    length -= (int)strlen(prefix);
                
                if (padc == ' ' && !ladjust) {
                    /* blank padding goes before prefix */
                    while (--length >= 0) {
                        if (putc)  (*putc)(' ', arg);
                        nprinted++;
                    }			    
                }
                if (sign_char) {
                    if (putc) (*putc)(sign_char, arg);
                    nprinted++;
                }
                if (prefix) {
                    while (*prefix) {
                        if (putc) (*putc)(*prefix++, arg);
                        nprinted++;
                    }
                }
                if (padc == '0') {
                    /* zero padding goes after sign and prefix */
                    while (--length >= 0) {
                        if (putc) (*putc)('0', arg);
                        nprinted++;
                    }			    
                }
                while (++p != &buf[MAXBUF]) {
                    if (putc) (*putc)(*p, arg);
                    nprinted++;
                }
                
                if (ladjust) {
                    while (--length >= 0) {
                        if (putc) (*putc)(' ', arg);
                        nprinted++;
                    }
                }
                break;
            }
                
            case '\0':
                fmt--;
                break;
                
            default:
                if (putc) (*putc)(c, arg);
                nprinted++;
	    }
        fmt++;
	}
    
	return nprinted;
}
#endif

int
prf(
         const char	*fmt,
         va_list			ap,
         /* character output routine */
         void			(*putc)(char))		
{
    return __doprnt(fmt, ap, dummy_putc, putc, 10);                
}


static char *copybyte_str;

static void
copybyte(
         char byte)
{
    *copybyte_str++ = byte;
    *copybyte_str = '\0';
}

int
sprintf(char *buf, const char *fmt, ...)
{
    va_list listp;
    
    va_start(listp, fmt);
    copybyte_str = buf;
    prf(fmt, listp, copybyte);
    va_end(listp);
    return strlen(buf);
}

static void
snprintf_func(int ch, void *arg)
{
    struct snprintf_arg *const info = arg;
    
    if (info->remain >= 2) {
        *info->str++ = ch;
        info->remain--;
    }
}

/*
 * Scaled down version of vsnprintf(3).
 */
int
vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    struct snprintf_arg info;
    int retval;
    
    info.str = str;
    info.remain = size;
    retval = __doprnt(format, ap, snprintf_func, &info, 10);
    if (info.remain >= 1)
        *info.str++ = '\0';
    return retval;
}

/*
 * Scaled down version of snprintf(3).
 */
int
snprintf(char *str, size_t size, const char *format, ...)
{
    int retval;
    va_list ap;
    
    va_start(ap, format);
    retval = vsnprintf(str, size, format, ap);
    va_end(ap);
    return(retval);
}

#if 0
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
#include <sys/param.h>

#define SPACE	1
#define ZERO	2
#define UCASE   16

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
static int
printn(u_long n, int b, int flag, int minwidth, void (*putfn_p)(int ch, void *arg), void *putfn_arg)
{
	char prbuf[11];
	register char *cp;
	int width = 0, neg = 0, len = 0;
	
	if (b == 10 && (int)n < 0) {
		neg = 1;
		n = (unsigned)(-(int)n);
	}
	cp = prbuf;
	do {
		*cp++ = "0123456789abcdef0123456789ABCDEF"[(flag & UCASE) + n%b];
		n /= b;
		width++;
	} while (n);
	
	if (neg) {
		(*putfn_p)('-', putfn_arg);
		width++;
        len++;
	}
	while (width++ < minwidth)
    {
		(*putfn_p)( (flag & ZERO) ? '0' : ' ', putfn_arg);
        len++;
    }
	
	do
    {
		(*putfn_p)(*--cp, putfn_arg);
        len++;
        
    } while (cp > prbuf);
    
    return len;
}

int __doprnt(
             const char *fmt,
             va_list argp,
             void (*putfn_p)(int ch, void *arg),
             void *putfn_arg,
             int   radix
             )
{
	int b, c, len =0;
	char *s;
	int flag = 0, width = 0;
	int minwidth;
    unsigned int *adx = (unsigned int*)argp;
loop:
	while ((c = *fmt++) != '%') {
		if(c == '\0')
			return len;
		if (putfn_p) {
			(*putfn_p)(c, putfn_arg);
		}
		len++;
	}
	minwidth = 0;
again:
	c = *fmt++;
	switch (c) {
		case 'l':
			goto again;
		case ' ':
			flag |= SPACE;
			goto again;
		case '0':
			if (minwidth == 0) {
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
			b = 10;
			goto number;
		case 'o': case 'O':
			b = 8;
		number:
			len += printn((u_long)*adx, b, flag, minwidth, putfn_p, putfn_arg);			
			break;
		case 's':
			s = (char *)*adx;
			while ((c = *s++)) {
				if (putfn_p) {
					(*putfn_p)(c, putfn_arg);
				}
				len++;
				width++;
			}
			while (width++ < minwidth) {
				if (putfn_p) {
					(*putfn_p)(' ', putfn_arg);
				}
				len++;
			}
			break;
		case 'c':
			if (putfn_p) {
				(*putfn_p)((char)*adx, putfn_arg);
			}
			len++;
			break;
		default:
			break;
	}
	adx++;
	goto loop;
}
#endif