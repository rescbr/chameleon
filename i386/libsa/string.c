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
/* string operations */

#include "libsa.h"

static int _mach_strlen(const char *str);
static char *STRDUP(const char *string);

/*
 * Abstract:
 *      strcmp (s1, s2) compares the strings "s1" and "s2".
 *      It returns 0 if the strings are identical. It returns
 *      > 0 if the first character that differs in the two strings
 *      is larger in s1 than in s2 or if s1 is longer than s2 and
 *      the contents are identical up to the length of s2.
 *      It returns < 0 if the first differing character is smaller
 *      in s1 than in s2 or if s1 is shorter than s2 and the
 *      contents are identical upto the length of s1.
 * Deprecation Warning:
 *	strcmp() is being deprecated. Please use strncmp() instead.
 */

int
strcmp(
	   const char *s1,
	   const char *s2)
{
	unsigned int a, b;
	
	do {
		a = *s1++;
		b = *s2++;
		if (a != b)
			return a-b;     /* includes case when
							 'a' is zero and 'b' is not zero
							 or vice versa */
	} while (a != '\0');
	
	return 0;       /* both are zero */
}

/*
 * Abstract:
 *      strncmp (s1, s2, n) compares the strings "s1" and "s2"
 *      in exactly the same way as strcmp does.  Except the
 *      comparison runs for at most "n" characters.
 */

int
strncmp(
        const char *s1,
        const char *s2,
        size_t n)
{
	unsigned int a, b;
	
	while (n != 0) {
		a = *s1++;
		b = *s2++;
		if (a != b)
			return a-b;     /* includes case when
							 'a' is zero and 'b' is not zero
							 or vice versa */
		if (a == '\0')
			return 0;       /* both are zero */
		n--;
	}
	
	return 0;
}

/*
 * Abstract:
 *      strcpy copies the contents of the string "from" including
 *      the null terminator to the string "to". A pointer to "to"
 *      is returned.
 * Deprecation Warning: 
 *	strcpy() is being deprecated. Please use strlcpy() instead.
 */
char *
strcpy(
	   char *to,
	   const char *from)
{
	char *ret = to;
	
	while ((*to++ = *from++) != '\0')
		continue;
	
	return ret;
}

/*
 * Abstract:
 *      strncpy copies "count" characters from the "from" string to
 *      the "to" string. If "from" contains less than "count" characters
 *      "to" will be padded with null characters until exactly "count"
 *      characters have been written. The return value is a pointer
 *      to the "to" string.
 */

char *
strncpy(
		char *s1, 
		const char *s2,
		size_t n)
{
	char *os1 = s1;
	unsigned long i;
	
	for (i = 0; i < n;)
		if ((*s1++ = *s2++) == '\0')
			for (i++; i < n; i++)
				*s1++ = '\0';
		else
			i++;
	return (os1);
}

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t
strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	
	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}
	
	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}
	
	return(s - src - 1);	/* count does not include NUL */
}

/*
 * History:
 *  2002-01-24 	gvdl	Initial implementation of strstr
 */

const char *
strstr(const char *in, const char *str)
{
    char c;
    size_t len;
	
    c = *str++;
    if (!c)
        return (const char *) in;	// Trivial empty string case
	
    len = strlen(str);
    do {
        char sc;
		
        do {
            sc = *in++;
            if (!sc)
                return (char *) 0;
        } while (sc != c);
    } while (strncmp(in, str, len) != 0);
	
    return (const char *) (in - 1);
}

void *
memmove(void *dst, const void *src, size_t ulen)
{
    bcopy(src, dst, ulen);   
    return dst;
}

int
ptol(const char *str)
{
	register int c = *str;
	
	if (c <= '7' && c >= '0')
		c -= '0';
	else if (c <= 'h' && c >= 'a')
		c -= 'a';
	else c = 0;
	return c;
}

/*
 * atoi:
 *
 *      This function converts an ascii string into an integer.
 *
 * input        : string
 * output       : a number
 */

int
atoi(const char *cp)
{
	int     number;
	
	for (number = 0; ('0' <= *cp) && (*cp <= '9'); cp++)
		number = (number * 10) + (*cp - '0');
	
	return( number );
}

/*
 * convert an integer to an ASCII string.
 * inputs:
 *	num	integer to be converted
 *	str	string pointer.
 *
 * outputs:
 *	pointer to string start.
 */

char *
itoa(
	 int	num,
	 char	*str)
{
	char    digits[11];
	char *dp;
	char *cp = str;
	
	if (num == 0) {
		*cp++ = '0';
	}
	else {
		dp = digits;
		while (num) {
			*dp++ = '0' + num % 10;
			num /= 10;
		}
		while (dp != digits) {
			*cp++ = *--dp;
		}
	}
	*cp++ = '\0';
	
	return str;
}
/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t
strlcat(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	size_t dlen;
	
	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;
	
	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';
	
	return(dlen + (s - src));       /* count does not include NUL */
}

/*
 *
 */

char *
strncat(char *s1, const char *s2, unsigned long n)
{
	char *os1;
	int i = n;
	
	os1 = s1;
	while (*s1++)
		;
	--s1;
	while ((*s1++ = *s2++))
		if (--i < 0) {
			*--s1 = '\0';
			break;
		}
	return(os1);
}

static int
_mach_strlen(const char *str)
{
	const char *p;
	for (p = str; p; p++) {
		if (*p == '\0') {
			return (p - str);
		}
	}
	/* NOTREACHED */
	return 0;
}

size_t strlen(const char * str)
{	
	return (size_t)_mach_strlen(str);
}

/*
 * Does the same thing as strlen, except only looks up
 * to max chars inside the buffer. 
 * Taken from archive/kern-stuff/sbf_machine.c in 
 * seatbelt. 
 * inputs:
 * 	s	string whose length is to be measured
 *	max	maximum length of string to search for null
 * outputs:
 *	length of s or max; whichever is smaller
 */
size_t 
strnlen(const char *s, size_t max) {
	const char *es = s + max, *p = s;
	while(*p && p != es) 
		p++;
	
	return p - s;
}

/* 
 * Deprecation Warning:
 *	strcat() is being deprecated. Please use strlcat() instead.
 */
char *
strcat(
	   char *dest,
	   const char *src)
{
	char *old = dest;
	
	while (*dest)
		++dest;
	while ((*dest++ = *src++))
		;
	return (old);
}

/*
 * STRDUP
 *
 * Description: The STRDUP function allocates sufficient memory for a copy
 *              of the string "string", does the copy, and returns a pointer
 *              it. The pointer may subsequently be used as an argument to
 *              the macro FREE().
 *
 * Parameters:  string		String to be duplicated
 *
 * Returns:     char *          A pointer to the newly allocated string with
 *                              duplicated contents in it.
 *
 *              NULL		If MALLOC() fails.
 * 
 */

static char *
STRDUP(const char *string)
{
	size_t len;
	char *copy;   
	
	len = strlen(string) + 1;
	
    copy = calloc(len, sizeof(char));
	
	if (copy == NULL)
		return (NULL);
	bcopy(string, copy, len);
	return (copy); 
}

char *strdup(const char *string)
{
	if (string) {
		return STRDUP(string);
	}
	return (NULL);
}

#if STRNCASECMP

//
// Lame implementation just for use by strcasecmp/strncasecmp
//
static int
tolower(unsigned char ch)
{
    if (ch >= 'A' && ch <= 'Z')
		ch = 'a' + (ch - 'A');
	
    return ch;
}

int
strcasecmp(const char *s1, const char *s2)
{
    const unsigned char *us1 = (const u_char *)s1,
	*us2 = (const u_char *)s2;
	
    while (tolower(*us1) == tolower(*us2++))
		if (*us1++ == '\0')
			return (0);
    return (tolower(*us1) - tolower(*--us2));
}

int
strncasecmp(const char *s1, const char *s2, size_t n)
{
    if (n != 0) {
		const unsigned char *us1 = (const u_char *)s1,
		*us2 = (const u_char *)s2;
		
		do {
			if (tolower(*us1) != tolower(*us2++))
				return (tolower(*us1) - tolower(*--us2));
			if (*us1++ == '\0')
				break;
		} while (--n != 0);
    }
    return (0);
}
#endif
/*
 *
 */

char *strchr(const char *str, int ch)
{
    do {
		if (*str == ch)
			return(__CAST_AWAY_QUALIFIER(str, const, char *));
    } while (*str++);
    return ((char *) 0);
}      

char* strbreak(const char *str, char **next, long *len)
{
    char *start = (char*)str, *end;
    bool quoted = false;
    
    if ( !start || !len )
        return 0;
    
    *len = 0;
    
    while ( isspace(*start) )
        start++;
    
    if (*start == '"')
    {
        start++;
        
        end = strchr(start, '"');
        if(end)
            quoted = true;
        else
            end = strchr(start, '\0');
    }
    else
    {
        for ( end = start; *end && !isspace(*end); end++ )
        {}
    }
    
    *len = end - start;
    
    if(next)
        *next = quoted ? end+1 : end;
    
    return start;
}

unsigned long
local_adler32( unsigned char * buffer, long length )
{
    long          cnt;
    unsigned long result, lowHalf, highHalf;
    
    lowHalf  = 1;
    highHalf = 0;
	
	for ( cnt = 0; cnt < length; cnt++ )
    {
        if ((cnt % 5000) == 0)
        {
            lowHalf  %= 65521L;
            highHalf %= 65521L;
        }
		
        lowHalf  += buffer[cnt];
        highHalf += lowHalf;
    }
	
	lowHalf  %= 65521L;
	highHalf %= 65521L;
	
	result = (highHalf << 16) | lowHalf;
	
	return result;
}

/*-
 * For memcmp, bsearch, memchr , memcpy, memmove, bcopy.
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

/*
 * Compare memory regions.
 */
int
memcmp(const void *s1, const void *s2, size_t n)
{
	if (n != 0) {
		const unsigned char *p1 = s1, *p2 = s2;
		
		do {
			if (*p1++ != *p2++)
				return (*--p1 - *--p2);
		} while (--n != 0);
	}
	return (0);
}

/*
 * Perform a binary search.
 *
 * The code below is a bit sneaky.  After a comparison fails, we
 * divide the work in half by moving either left or right. If lim
 * is odd, moving left simply involves halving lim: e.g., when lim
 * is 5 we look at item 2, so we change lim to 2 so that we will
 * look at items 0 & 1.  If lim is even, the same applies.  If lim
 * is odd, moving right again involes halving lim, this time moving
 * the base up one item past p: e.g., when lim is 5 we change base
 * to item 3 and make lim 2 so that we will look at items 3 and 4.
 * If lim is even, however, we have to shrink it by one before
 * halving: e.g., when lim is 4, we still looked at item 2, so we
 * have to make lim 3, then halve, obtaining 1, so that we will only
 * look at item 3.
 */
void *
bsearch(key, base0, nmemb, size, compar)
register const void *key;
const void *base0;
size_t nmemb;
register size_t size;
register int (*compar)(const void *, const void *);
{
	register const char *base = base0;
	register size_t lim;
	register int cmp;
	register const void *p;
    
	for (lim = nmemb; lim != 0; lim >>= 1) {
		p = base + (lim >> 1) * size;
		cmp = (*compar)(key, p);
		if (cmp == 0)
			return ((void *)(uintptr_t)p);
		if (cmp > 0) {	/* key > p: move right */
			base = (const char *)p + size;
			lim--;
		}		/* else move left */
	}
	return (NULL);
}

void *
memchr(const void *s, int c, size_t n)
{
	if (n != 0) {
		const unsigned char *p = s;
		
		do {
			if (*p++ == (unsigned char)c)
				return ((void *)(p - 1));
		} while (--n != 0);
	}
	return (NULL);
}

#if 1
/*
 * sizeof(word) MUST BE A POWER OF TWO
 * SO THAT wmask BELOW IS ALL ONES
 */
typedef int word;               /* "word" used for optimal copy speed */

#define wsize   sizeof(word)
#define wmask   (wsize - 1)

/*
 * Copy a block of memory, handling overlap.
 * This is the routine that actually implements
 * (the portable versions of) bcopy, memcpy, and memmove.
 */

void
bcopy(const void *src0, void *dst0, size_t length)
{
	memcpy(dst0,src0,length);
}

void *memcpy(void *dst0, const void *src0, size_t length)
{
	char *dst = dst0;
	const char *src = src0;
	size_t t;
	
	if (length == 0 || dst == src)          /* nothing to do */
		goto done;
	
	/*
	 * Macros: loop-t-times; and loop-t-times, t>0
	 */
#define TLOOP(s) if (t) TLOOP1(s)
#define TLOOP1(s) do { s; } while (--t)
	
	if ((unsigned long)dst < (unsigned long)src) {
		/*
		 * Copy forward.
		 */
		t = (uintptr_t)src;     /* only need low bits */
		if ((t | (uintptr_t)dst) & wmask) {
			/*
			 * Try to align operands.  This cannot be done
			 * unless the low bits match.
			 */
			if ((t ^ (uintptr_t)dst) & wmask || length < wsize)
				t = length;
			else
				t = wsize - (t & wmask);
			length -= t;
			TLOOP1(*dst++ = *src++);
		}
		/*
		 * Copy whole words, then mop up any trailing bytes.
		 */
		t = length / wsize;
		TLOOP(*(word *)dst = *(word *)src; src += wsize; dst += wsize);
		t = length & wmask;
		TLOOP(*dst++ = *src++);
	} else {
		/*
		 * Copy backwards.  Otherwise essentially the same.
		 * Alignment works as before, except that it takes
		 * (t&wmask) bytes to align, not wsize-(t&wmask).
		 */
		src += length;
		dst += length;
		t = (uintptr_t)src;
		if ((t | (uintptr_t)dst) & wmask) {
			if ((t ^ (uintptr_t)dst) & wmask || length <= wsize)
				t = length;
			else
				t &= wmask;
			length -= t;
			TLOOP1(*--dst = *--src);
		}
		t = length / wsize;
		TLOOP(src -= wsize; dst -= wsize; *(word *)dst = *(word *)src);
		t = length & wmask;
		TLOOP(*--dst = *--src);
	}
done:
	return (dst0);
}


#ifdef wsize
#undef wsize
#endif
#ifdef wmask
#undef wmask
#endif
#define wsize   sizeof(u_int)
#define wmask   (wsize - 1)


void bzero(void *dst0, size_t length)
{	
#ifdef RETURN
#undef RETURN
#endif
#ifdef VAL
#undef VAL
#endif
#ifdef WIDEVAL
#undef WIDEVAL
#endif
#define RETURN  return
#define VAL     0
#define WIDEVAL 0
	
	size_t t;
	u_char *dst;
	
	dst = dst0;
	/*
	 * If not enough words, just fill bytes.  A length >= 2 words
	 * guarantees that at least one of them is `complete' after
	 * any necessary alignment.  For instance:
	 *
	 *      |-----------|-----------|-----------|
	 *      |00|01|02|03|04|05|06|07|08|09|0A|00|
	 *                ^---------------------^
	 *               dst             dst+length-1
	 *
	 * but we use a minimum of 3 here since the overhead of the code
	 * to do word writes is substantial.
	 */
	if (length < 3 * wsize) {
		while (length != 0) {
			*dst++ = VAL;
			--length;
		}
		RETURN;
	}
	
	/* Align destination by filling in bytes. */
	if ((t = (long)dst & wmask) != 0) {
		t = wsize - t;
		length -= t;
		do {
			*dst++ = VAL;
		} while (--t != 0);
	}
	
	/* Fill words.  Length was >= 2*words so we know t >= 1 here. */
	t = length / wsize;
	do {
		*(u_int *)dst = WIDEVAL;
		dst += wsize;
	} while (--t != 0);
	
	/* Mop up trailing bytes, if any. */
	t = length & wmask;
	if (t != 0)
		do {
			*dst++ = VAL;
		} while (--t != 0);
	RETURN;
}


void *
memset(void *dst0, int c0, size_t length)
{
#ifdef RETURN
#undef RETURN
#endif	
#ifdef VAL
#undef VAL
#endif
#ifdef WIDEVAL
#undef WIDEVAL
#endif
	
#define VAL     c0
#define WIDEVAL c
#define RETURN  return (dst0)
	
	size_t t;
	u_int c;
	u_char *dst;
	
	dst = dst0;
	/*
	 * If not enough words, just fill bytes.  A length >= 2 words
	 * guarantees that at least one of them is `complete' after
	 * any necessary alignment.  For instance:
	 *
	 *      |-----------|-----------|-----------|
	 *      |00|01|02|03|04|05|06|07|08|09|0A|00|
	 *                ^---------------------^
	 *               dst             dst+length-1
	 *
	 * but we use a minimum of 3 here since the overhead of the code
	 * to do word writes is substantial.
	 */
	if (length < 3 * wsize) {
		while (length != 0) {
			*dst++ = VAL;
			--length;
		}
		RETURN;
	}
	
	if ((c = (u_char)c0) != 0) {    /* Fill the word. */
		c = (c << 8) | c;       /* u_int is 16 bits. */
#if UINT_MAX > 0xffff
		c = (c << 16) | c;      /* u_int is 32 bits. */
#endif
#if UINT_MAX > 0xffffffff
		c = (c << 32) | c;      /* u_int is 64 bits. */
#endif
	}
	
	/* Align destination by filling in bytes. */
	if ((t = (long)dst & wmask) != 0) {
		t = wsize - t;
		length -= t;
		do {
			*dst++ = VAL;
		} while (--t != 0);
	}
	
	/* Fill words.  Length was >= 2*words so we know t >= 1 here. */
	t = length / wsize;
	do {
		*(u_int *)dst = WIDEVAL;
		dst += wsize;
	} while (--t != 0);
	
	/* Mop up trailing bytes, if any. */
	t = length & wmask;
	if (t != 0)
		do {
			*dst++ = VAL;
		} while (--t != 0);
	RETURN;
}

#endif