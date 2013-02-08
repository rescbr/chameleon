/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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
 */

#if defined(LIBC_SCCS) && !defined(lint)
#if 0
static char sccsid[] = "@(#)vfprintf.c	8.1 (Berkeley) 6/4/93";
#endif
static const char rcsid[] =
"$FreeBSD: src/lib/libc/stdio/vfprintf.c,v 1.22.2.6 2012/11/17 07:23:37 svnexp Exp $";
#endif /* LIBC_SCCS and not lint */

/*
 * Actual printf innards.
 *
 * This code is large and complicated...
 */

#include "namespace.h"
#include <sys/types.h>

#include <ctype.h>
#include <limits.h>
#include <locale.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <errno.h>
#include "un-namespace.h"
#include "local.h"
#include "fvwrite.h"
#include "printflocal.h"

static int	__sprint __P((FILE *, struct __suio *));
static int	__sbprintf __P((FILE *, const char *, va_list));
static char *	__ultoa __P((u_long, char *, int, int, char *));
static char *	__uqtoa __P((u_quad_t, char *, int, int, char *));

/*
 * Flush out all the vectors defined by the given uio,
 * then reset it so that it can be reused.
 */
static int
__sprint(FILE *fp, struct __suio *uio)
{
	int err;
    
	if (uio->uio_resid == 0) {
		uio->uio_iovcnt = 0;
		return (0);
	}
	err = __sfvwrite(fp, uio);
	uio->uio_resid = 0;
	uio->uio_iovcnt = 0;
	return (err);
}

/*
 * Helper function for `fprintf to unbuffered unix file': creates a
 * temporary buffer.  We only work on write-only files; this avoids
 * worries about ungetc buffers and so forth.
 */
static int
__sbprintf(FILE *fp, const char *fmt, va_list ap)
{
	int ret;
	FILE fake;
	unsigned char buf[BUFSIZ];
    
	/* copy the important variables */
	fake._flags = fp->_flags & ~__SNBF;
	fake._file = fp->_file;
	fake._cookie = fp->_cookie;
	fake._write = fp->_write;
    
	/* set up the buffer */
	fake._bf._base = fake._p = buf;
	fake._bf._size = fake._w = sizeof(buf);
	fake._lbfsize = 0;	/* not actually used, but Just In Case */
    
	/* do the work, then copy any error status */
	ret = vfprintf(&fake, fmt, ap);
	if (ret >= 0 && fflush(&fake))
		ret = EOF;
	if (fake._flags & __SERR)
		fp->_flags |= __SERR;
	return (ret);
}

/*
 * Macros for converting digits to letters and vice versa
 */
#define	to_digit(c)	((c) - '0')
#define is_digit(c)	((unsigned)to_digit(c) <= 9)
#define	to_char(n)	((n) + '0')

/*
 * Convert an unsigned long to ASCII for printf purposes, returning
 * a pointer to the first character of the string representation.
 * Octal numbers can be forced to have a leading zero; hex numbers
 * use the given digits.
 */
static char *
__ultoa(u_long val, char *endp, int base, int octzero, char *xdigs)
{
	register char *cp = endp;
	register long sval;
    
	/*
	 * Handle the three cases separately, in the hope of getting
	 * better/faster code.
	 */
	switch (base) {
        case 10:
            if (val < 10) {	/* many numbers are 1 digit */
                *--cp = to_char(val);
                return (cp);
            }
            /*
             * On many machines, unsigned arithmetic is harder than
             * signed arithmetic, so we do at most one unsigned mod and
             * divide; this is sufficient to reduce the range of
             * the incoming value to where signed arithmetic works.
             */
            if (val > LONG_MAX) {
                *--cp = to_char(val % 10);
                sval = val / 10;
            } else
                sval = val;
            do {
                *--cp = to_char(sval % 10);
                sval /= 10;
            } while (sval != 0);
            break;
            
        case 8:
            do {
                *--cp = to_char(val & 7);
                val >>= 3;
            } while (val);
            if (octzero && *cp != '0')
                *--cp = '0';
            break;
            
        case 16:
            do {
                *--cp = xdigs[val & 15];
                val >>= 4;
            } while (val);
            break;
            
        default:			/* oops */
            abort();
	}
	return (cp);
}

/* Identical to __ultoa, but for quads. */
static char *
__uqtoa(u_quad_t val, char *endp, int base, int octzero, char *xdigs)
{
	char *cp = endp;
	quad_t sval;
    
	/* quick test for small values; __ultoa is typically much faster */
	/* (perhaps instead we should run until small, then call __ultoa?) */
	if (val <= ULONG_MAX)
		return (__ultoa((u_long)val, endp, base, octzero, xdigs));
	switch (base) {
        case 10:
            if (val < 10) {
                *--cp = to_char(val % 10);
                return (cp);
            }
            if (val > QUAD_MAX) {
                *--cp = to_char(val % 10);
                sval = val / 10;
            } else
                sval = val;
            do {
                *--cp = to_char(sval % 10);
                sval /= 10;
            } while (sval != 0);
            break;
            
        case 8:
            do {
                *--cp = to_char(val & 7);
                val >>= 3;
            } while (val);
            if (octzero && *cp != '0')
                *--cp = '0';
            break;
            
        case 16:
            do {
                *--cp = xdigs[val & 15];
                val >>= 4;
            } while (val);
            break;
            
        default:
            abort();
	}
	return (cp);
}

#ifndef NO_FLOATING_POINT


#else /* no FLOATING_POINT */

#define	BUF		68

#endif /* FLOATING_POINT */

#define STATIC_ARG_TBL_SIZE 8           /* Size of static argument table. */

/*
 * Flags used during conversion.
 */
#define	ALT		0x001		/* alternate form */
#define	HEXPREFIX	0x002		/* add 0x or 0X prefix */
#define	LADJUST		0x004		/* left adjustment */
#define	LONGDBL		0x008		/* long double */
#define	LONGINT		0x010		/* long integer */
#define	QUADINT		0x020		/* quad integer */
#define	SHORTINT	0x040		/* short integer */
#define	ZEROPAD		0x080		/* zero (as opposed to blank) pad */
#define FPT		0x100		/* Floating point number */
int
vfprintf(FILE *fp, const char *fmt0, va_list ap)
{
	char *fmt;		/* format string */
	int ch;			/* character from fmt */
	int n, n2;		/* handy integer (short term usage) */
	char *cp;		/* handy char pointer (short term usage) */
	struct __siov *iovp;	/* for PRINT macro */
	int flags;		/* flags as above */
	int ret;		/* return value accumulator */
	int width;		/* width from format (%8d), or 0 */
	int prec;		/* precision from format (%.3d), or -1 */
	char sign;		/* sign prefix (' ', '+', '-', or \0) */
#ifndef NO_FLOATING_POINT
	char *decimal_point = localeconv()->decimal_point;
	char softsign;		/* temporary negative sign for floats */
	double _double;		/* double precision arguments %[eEfgG] */
	int expt;		/* integer value of exponent */
	int expsize;		/* character count for expstr */
	int ndig;		/* actual number of digits returned by cvt */
	char expstr[7];		/* buffer for exponent string */
	char *dtoaresult;	/* buffer allocated by dtoa */
#endif
	u_long	ulval = 0;		/* integer arguments %[diouxX] */
	u_quad_t uqval = 0;		/* %q integers */
	int base;		/* base for [diouxX] conversion */
	int dprec;		/* a copy of prec if [diouxX], 0 otherwise */
	int realsz;		/* field size expanded by dprec, sign, etc */
	int size;		/* size of converted field or string */
	int prsize;             /* max size of printed field */
	char *xdigs = NULL;		/* digits for [xX] conversion */
#define NIOV 8
	struct __suio uio;	/* output information: summary */
	struct __siov iov[NIOV];/* ... and individual io vectors */
	char buf[BUF];		/* space for %c, %[diouxX], %[eEfgG] */
	char ox[2];		/* space for 0x hex-prefix */
    union arg *argtable;    /* args, built due to positional arg */
	union arg statargtable [STATIC_ARG_TBL_SIZE];
    int nextarg;            /* 1-based argument index */
    va_list orgap;          /* original argument pointer */
    
	/*
	 * Choose PADSIZE to trade efficiency vs. size.  If larger printf
	 * fields occur frequently, increase PADSIZE and make the initialisers
	 * below longer.
	 */
#define	PADSIZE	16		/* pad chunk size */
	static char blanks[PADSIZE] =
    {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
	static char zeroes[PADSIZE] =
    {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};
    
	/*
	 * BEWARE, these `goto error' on error, and PAD uses `n'.
	 */
#define	PRINT(ptr, len) { \
iovp->iov_base = (ptr); \
iovp->iov_len = (len); \
uio.uio_resid += (len); \
iovp++; \
if (++uio.uio_iovcnt >= NIOV) { \
if (__sprint(fp, &uio)) \
goto error; \
iovp = iov; \
} \
}
#define	PAD(howmany, with) { \
if ((n = (howmany)) > 0) { \
while (n > PADSIZE) { \
PRINT(with, PADSIZE); \
n -= PADSIZE; \
} \
PRINT(with, n); \
} \
}
#define	FLUSH() { \
if (uio.uio_resid && __sprint(fp, &uio)) \
goto error; \
uio.uio_iovcnt = 0; \
iovp = iov; \
}
    
	/*
	 * Get the argument indexed by nextarg.   If the argument table is
	 * built, use it to get the argument.  If its not, get the next
	 * argument (and arguments must be gotten sequentially).
	 */
#define GETARG(type) \
((argtable != NULL) ? *((type*)(&argtable[nextarg++])) : \
(nextarg++, va_arg(ap, type)))
    
	/*
	 * To extend shorts properly, we need both signed and unsigned
	 * argument extraction methods.
	 */
#define	SARG() \
(flags&LONGINT ? GETARG(long) : \
flags&SHORTINT ? (long)(short)GETARG(int) : \
flags&CHARINT ? (long)(signed char)GETARG(int) : \
(long)GETARG(int))
#define	UARG() \
(flags&LONGINT ? GETARG(u_long) : \
flags&SHORTINT ? (u_long)(u_short)GETARG(int) : \
flags&CHARINT ? (u_long)(u_char)GETARG(int) : \
(u_long)GETARG(u_int))
#define	INTMAX_SIZE	(INTMAXT|SIZET|PTRDIFFT|LLONGINT)
#define SJARG() \
(flags&INTMAXT ? GETARG(intmax_t) : \
flags&SIZET ? (intmax_t)GETARG(ssize_t) : \
flags&PTRDIFFT ? (intmax_t)GETARG(ptrdiff_t) : \
(intmax_t)GETARG(long long))
#define	UJARG() \
(flags&INTMAXT ? GETARG(uintmax_t) : \
flags&SIZET ? (uintmax_t)GETARG(size_t) : \
flags&PTRDIFFT ? (uintmax_t)GETARG(ptrdiff_t) : \
(uintmax_t)GETARG(unsigned long long))
    
	/*
	 * Get * arguments, including the form *nn$.  Preserve the nextarg
	 * that the argument can be gotten once the type is determined.
	 */
#define GETASTER(val) \
n2 = 0; \
cp = fmt; \
while (is_digit(*cp)) { \
n2 = 10 * n2 + to_digit(*cp); \
cp++; \
} \
if (*cp == '$') { \
int hold = nextarg; \
if (argtable == NULL) { \
argtable = statargtable; \
if (__find_arguments (fmt0, orgap, &argtable)) { \
ret = EOF; \
goto error; \
} \
} \
nextarg = n2; \
val = GETARG (int); \
nextarg = hold; \
fmt = ++cp; \
} else { \
val = GETARG (int); \
}
    
    
#ifndef NO_FLOATING_POINT
	dtoaresult = NULL;
#endif
	/* sorry, fprintf(read_only_file, "") returns EOF, not 0 */
	if (prepwrite(fp) != 0)
		return (EOF);
    
	/* optimise fprintf(stderr) (and other unbuffered Unix files) */
	if ((fp->_flags & (__SNBF|__SWR|__SRW)) == (__SNBF|__SWR) &&
	    fp->_file >= 0) {
		return (__sbprintf(fp, fmt0, ap));
	}
    
	fmt = (char *)fmt0;
    argtable = NULL;
    nextarg = 1;
    orgap = ap;
	uio.uio_iov = iovp = iov;
	uio.uio_resid = 0;
	uio.uio_iovcnt = 0;
	ret = 0;
    
	/*
	 * Scan the format for conversions (`%' character).
	 */
	for (;;) {
		for (cp = fmt; (ch = *fmt) != '\0' && ch != '%'; fmt++)
        /* void */;
		if ((n = fmt - cp) != 0) {
			if ((unsigned)ret + n > INT_MAX) {
				ret = EOF;
				goto error;
			}
			PRINT(cp, n);
			ret += n;
		}
		if (ch == '\0')
			goto done;
		fmt++;		/* skip over '%' */
        
		flags = 0;
		dprec = 0;
		width = 0;
		prec = -1;
		sign = '\0';
        
    rflag:		ch = *fmt++;
    reswitch:	switch (ch) {
		case ' ':
			/*
			 * ``If the space and + flags both appear, the space
			 * flag will be ignored.''
			 *	-- ANSI X3J11
			 */
			if (!sign)
				sign = ' ';
			goto rflag;
		case '#':
			flags |= ALT;
			goto rflag;
		case '*':
			/*
			 * ``A negative field width argument is taken as a
			 * - flag followed by a positive field width.''
			 *	-- ANSI X3J11
			 * They don't exclude field widths read from args.
			 */
			GETASTER (width);
			if (width >= 0)
				goto rflag;
			width = -width;
			/* FALLTHROUGH */
		case '-':
			flags |= LADJUST;
			goto rflag;
		case '+':
			sign = '+';
			goto rflag;
		case '.':
			if ((ch = *fmt++) == '*') {
				GETASTER (n);
				prec = n < 0 ? -1 : n;
				goto rflag;
			}
			n = 0;
			while (is_digit(ch)) {
				n = 10 * n + to_digit(ch);
				ch = *fmt++;
			}
			prec = n < 0 ? -1 : n;
			goto reswitch;
		case '0':
			/*
			 * ``Note that 0 is taken as a flag, not as the
			 * beginning of a field width.''
			 *	-- ANSI X3J11
			 */
			flags |= ZEROPAD;
			goto rflag;
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			n = 0;
			do {
				n = 10 * n + to_digit(ch);
				ch = *fmt++;
			} while (is_digit(ch));
			if (ch == '$') {
				nextarg = n;
                if (argtable == NULL) {
                    argtable = statargtable;
                    __find_arguments (fmt0, orgap,
                                      &argtable);
				}
				goto rflag;
            }
			width = n;
			goto reswitch;
#ifndef NO_FLOATING_POINT
		case 'L':
			flags |= LONGDBL;
			goto rflag;
#endif
		case 'h':
			flags |= SHORTINT;
			goto rflag;
		case 'l':
			if (flags & LONGINT)
				flags |= QUADINT;
			else
				flags |= LONGINT;
			goto rflag;
		case 'q':
			flags |= QUADINT;
			goto rflag;
		case 'c':
			*(cp = buf) = GETARG(int);
			size = 1;
			sign = '\0';
			break;
		case 'D':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'd':
		case 'i':
			if (flags & QUADINT) {
				uqval = GETARG(quad_t);
				if ((quad_t)uqval < 0) {
					uqval = -uqval;
					sign = '-';
				}
			} else {
				ulval = SARG();
				if ((long)ulval < 0) {
					ulval = -ulval;
					sign = '-';
				}
			}
			base = 10;
			goto number;
#ifndef NO_FLOATING_POINT
		case 'e':
		case 'E':
		case 'f':
			goto fp_begin;
		case 'g':
		case 'G':
			if (prec == 0)
				prec = 1;
        fp_begin:		if (prec == -1)
            prec = DEFPREC;
			if (flags & LONGDBL)
            /* XXX this loses precision. */
				_double = (double)GETARG(long double);
			else
				_double = GETARG(double);
			/* do this before tricky precision changes */
			if (isinf(_double)) {
				if (_double < 0)
					sign = '-';
				cp = "Inf";
				size = 3;
				break;
			}
			if (isnan(_double)) {
				cp = "NaN";
				size = 3;
				break;
			}
			flags |= FPT;
			if (dtoaresult != NULL) {
				free(dtoaresult);
				dtoaresult = NULL;
			}
			cp = cvt(_double, prec, flags, &softsign,
                     &expt, ch, &ndig, &dtoaresult);
			if (ch == 'g' || ch == 'G') {
				if (expt <= -4 || expt > prec)
					ch = (ch == 'g') ? 'e' : 'E';
				else
					ch = 'g';
			}
			if (ch <= 'e') {	/* 'e' or 'E' fmt */
				--expt;
				expsize = exponent(expstr, expt, ch);
				size = expsize + ndig;
				if (ndig > 1 || flags & ALT)
					++size;
			} else if (ch == 'f') {		/* f fmt */
				if (expt > 0) {
					size = expt;
					if (prec || flags & ALT)
						size += prec + 1;
				} else	/* "0.X" */
					size = prec + 2;
			} else if (expt >= ndig) {	/* fixed g fmt */
				size = expt;
				if (flags & ALT)
					++size;
			} else
				size = ndig + (expt > 0 ?
                               1 : 2 - expt);
            
			if (softsign)
				sign = '-';
			break;
#endif /* FLOATING_POINT */
		case 'n':
			if (flags & QUADINT)
				*GETARG(quad_t *) = ret;
			else if (flags & LONGINT)
				*GETARG(long *) = ret;
			else if (flags & SHORTINT)
				*GETARG(short *) = ret;
			else
				*GETARG(int *) = ret;
			continue;	/* no output */
		case 'O':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'o':
			if (flags & QUADINT)
				uqval = GETARG(u_quad_t);
			else
				ulval = UARG();
			base = 8;
			goto nosign;
		case 'p':
			/*
			 * ``The argument shall be a pointer to void.  The
			 * value of the pointer is converted to a sequence
			 * of printable characters, in an implementation-
			 * defined manner.''
			 *	-- ANSI X3J11
			 */
			ulval = (u_long)GETARG(void *);
			base = 16;
			xdigs = "0123456789abcdef";
			flags = (flags & ~QUADINT) | HEXPREFIX;
			ch = 'x';
			goto nosign;
		case 's':
			if ((cp = GETARG(char *)) == NULL)
				cp = "(null)";
			if (prec >= 0) {
				/*
				 * can't use strlen; can only look for the
				 * NUL in the first `prec' characters, and
				 * strlen() will go further.
				 */
				char *p = memchr(cp, 0, (size_t)prec);
                
				if (p != NULL) {
					size = p - cp;
					if (size > prec)
						size = prec;
				} else
					size = prec;
			} else
				size = strlen(cp);
			sign = '\0';
			break;
		case 'U':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'u':
			if (flags & QUADINT)
				uqval = GETARG(u_quad_t);
			else
				ulval = UARG();
			base = 10;
			goto nosign;
		case 'X':
			xdigs = "0123456789ABCDEF";
			goto hex;
		case 'x':
			xdigs = "0123456789abcdef";
        hex:			if (flags & QUADINT)
            uqval = GETARG(u_quad_t);
        else
            ulval = UARG();
			base = 16;
			/* leading 0x/X only if non-zero */
			if (flags & ALT &&
			    (flags & QUADINT ? uqval != 0 : ulval != 0))
				flags |= HEXPREFIX;
            
			/* unsigned conversions */
        nosign:			sign = '\0';
			/*
			 * ``... diouXx conversions ... if a precision is
			 * specified, the 0 flag will be ignored.''
			 *	-- ANSI X3J11
			 */
        number:			if ((dprec = prec) >= 0)
            flags &= ~ZEROPAD;
            
			/*
			 * ``The result of converting a zero value with an
			 * explicit precision of zero is no characters.''
			 *	-- ANSI X3J11
			 */
			cp = buf + BUF;
			if (flags & QUADINT) {
				if (uqval != 0 || prec != 0)
					cp = __uqtoa(uqval, cp, base,
                                 flags & ALT, xdigs);
			} else {
				if (ulval != 0 || prec != 0)
					cp = __ultoa(ulval, cp, base,
                                 flags & ALT, xdigs);
			}
			size = buf + BUF - cp;
			break;
		default:	/* "%?" prints ?, unless ? is NUL */
			if (ch == '\0')
				goto done;
			/* pretend it was %c with argument ch */
			cp = buf;
			*cp = ch;
			size = 1;
			sign = '\0';
			break;
    }
        
		/*
		 * All reasonable formats wind up here.  At this point, `cp'
		 * points to a string which (if not flags&LADJUST) should be
		 * padded out to `width' places.  If flags&ZEROPAD, it should
		 * first be prefixed by any sign or other prefix; otherwise,
		 * it should be blank padded before the prefix is emitted.
		 * After any left-hand padding and prefixing, emit zeroes
		 * required by a decimal [diouxX] precision, then print the
		 * string proper, then emit zeroes required by any leftover
		 * floating precision; finally, if LADJUST, pad with blanks.
		 *
		 * Compute actual size, so we know how much to pad.
		 * size excludes decimal prec; realsz includes it.
		 */
		realsz = dprec > size ? dprec : size;
		if (sign)
			realsz++;
		else if (flags & HEXPREFIX)
			realsz += 2;
        
		prsize = width > realsz ? width : realsz;
		if ((unsigned)ret + prsize > INT_MAX) {
			ret = EOF;
			goto error;
		}
        
		/* right-adjusting blank padding */
		if ((flags & (LADJUST|ZEROPAD)) == 0)
			PAD(width - realsz, blanks);
        
		/* prefix */
		if (sign) {
			PRINT(&sign, 1);
		} else if (flags & HEXPREFIX) {
			ox[0] = '0';
			ox[1] = ch;
			PRINT(ox, 2);
		}
        
		/* right-adjusting zero padding */
		if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD)
			PAD(width - realsz, zeroes);
        
		/* leading zeroes from decimal precision */
		PAD(dprec - size, zeroes);
        
		/* the string or number proper */
#ifndef NO_FLOATING_POINT
		if ((flags & FPT) == 0) {
			PRINT(cp, size);
		} else {	/* glue together f_p fragments */
			if (ch >= 'f') {	/* 'f' or 'g' */
				if (_double == 0) {
					/* kludge for __dtoa irregularity */
					PRINT("0", 1);
					if (expt < ndig || (flags & ALT) != 0) {
						PRINT(decimal_point, 1);
						PAD(ndig - 1, zeroes);
					}
				} else if (expt <= 0) {
					PRINT("0", 1);
					PRINT(decimal_point, 1);
					PAD(-expt, zeroes);
					PRINT(cp, ndig);
				} else if (expt >= ndig) {
					PRINT(cp, ndig);
					PAD(expt - ndig, zeroes);
					if (flags & ALT)
						PRINT(decimal_point, 1);
				} else {
					PRINT(cp, expt);
					cp += expt;
					PRINT(decimal_point, 1);
					PRINT(cp, ndig-expt);
				}
			} else {	/* 'e' or 'E' */
				if (ndig > 1 || flags & ALT) {
					ox[0] = *cp++;
					ox[1] = *decimal_point;
					PRINT(ox, 2);
					if (_double) {
						PRINT(cp, ndig-1);
					} else	/* 0.[0..] */
                    /* __dtoa irregularity */
						PAD(ndig - 1, zeroes);
				} else	/* XeYYY */
					PRINT(cp, 1);
				PRINT(expstr, expsize);
			}
		}
#else
		PRINT(cp, size);
#endif
		/* left-adjusting padding (always blank) */
		if (flags & LADJUST)
			PAD(width - realsz, blanks);
        
		/* finally, adjust ret */
		ret += prsize;
        
		FLUSH();	/* copy out the I/O vectors */
	}
done:
	FLUSH();
error:
#ifndef NO_FLOATING_POINT
	if (dtoaresult != NULL)
		free(dtoaresult);
#endif
	if (__sferror(fp))
		ret = EOF;
    if ((argtable != NULL) && (argtable != statargtable))
        free (argtable);
	return (ret);
	/* NOTREACHED */
}
