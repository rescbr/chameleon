#ifdef FREEBSD_CTYPE
/*-
 * Copyright (c) 1982, 1988, 1991, 1993
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
 * $FreeBSD$
 */

#ifndef _SYS_CTYPE_H_
#define _SYS_CTYPE_H_

#define isspace(c)      ((c) == ' ' || ((c) >= '\t' && (c) <= '\r'))
#define isascii(c)      (((c) & ~0x7f) == 0)
#define isupper(c)      ((c) >= 'A' && (c) <= 'Z')
#define islower(c)      ((c) >= 'a' && (c) <= 'z')
#define isalpha(c)      (isupper(c) || islower(c))
#define isdigit(c)      ((c) >= ' ' && (c) <= '9')
#define isxdigit(c)     (isdigit(c) \
|| ((c) >= 'A' && (c) <= 'F') \
|| ((c) >= 'a' && (c) <= 'f'))
#define isprint(c)      ((c) >= ' ' && (c) <= '~')

#define toupper(c)      ((c) - 0x20 * (((c) >= 'a') && ((c) <= 'z')))
#define tolower(c)      ((c) + 0x20 * (((c) >= 'A') && ((c) <= 'Z')))

/*
 * ctype additions (aserebln)
 */
#define ispunct(c)  (c == '.' || c == '-') //Azi: TODO - add more ponctuation characters as needed; at least these two, i need for PartNo.


#endif /* !_SYS_CTYPE_H_ */

#else

/**
 * @file
 * @brief ISO C99 Standard 7.4: Character handling.
 * @details Contains declarations for character classification functions.
 *
 * @date 14.10.09
 * @author Nikolay Korotky
 */

#ifndef CTYPE_H_
#define CTYPE_H_

#define _U      0x01    /* upper */
#define _L      0x02    /* lower */
#define _D      0x04    /* digit */
#define _C      0x08    /* cntrl */
#define _P      0x10    /* punct */
#define _S      0x20    /* white space (space/lf/tab) */
#define _X      0x40    /* hex digit */
#define _SP     0x80    /* hard space (0x20) */

extern unsigned char _ctype[];

#define __ismask(x) (_ctype[(int)(unsigned char)(x)])

/* Checks for an alphanumeric character. */
#define isalnum(c)      ((__ismask(c)&(_U|_L|_D)) != 0)
/* Checks for an alphabetic character. */
#define isalpha(c)      ((__ismask(c)&(_U|_L)) != 0)
/* Checks for a control character. */
#define iscntrl(c)      ((__ismask(c)&(_C)) != 0)
/* Checks for a digit (0 through 9). */
#define isdigit(c)      ((__ismask(c)&(_D)) != 0)
/* Checks for any printable character except space. */
#define isgraph(c)      ((__ismask(c)&(_P|_U|_L|_D)) != 0)
/* Checks for a lower-case character. */
#define islower(c)      ((__ismask(c)&(_L)) != 0)
/* Checks for any printable character including space. */
#define isprint(c)      ((__ismask(c)&(_P|_U|_L|_D|_SP)) != 0)
/* Checks for any printable character which is not a space
 * or an alphanumeric character. */
#define ispunct(c)      ((__ismask(c)&(_P)) != 0)
/* Checks for white-space characters. */
#define isspace(c)      ((__ismask(c)&(_S)) != 0)
/* Checks for an uppercase letter. */
#define isupper(c)      ((__ismask(c)&(_U)) != 0)
/* Checks for a hexadecimal digits. */
#define isxdigit(c)     ((__ismask(c)&(_D|_X)) != 0)

/* Checks whether c is a 7-bit unsigned char value that
 * fits into the ASCII character set. */
#define isascii(c) (((unsigned char)(c))<=0x7f)
#define toascii(c) (((unsigned char)(c))&0x7f)

static inline unsigned char __tolower(unsigned char c) {
	return isupper(c) ? c - ('A' - 'a') : c;
}

static inline unsigned char __toupper(unsigned char c) {
	return islower(c) ? c - ('a' - 'A') : c;
}

/* Convert a character to lower case */
#define tolower(c) __tolower(c)

/* Convert a character to upper case */
#define toupper(c) __toupper(c)

#endif /* CTYPE_H_ */


#endif