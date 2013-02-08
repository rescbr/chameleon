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

#ifndef __BOOT_LIBSA_H
#define __BOOT_LIBSA_H

/* Exported API for standalone library */

#include <mach-o/loader.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <setjmp.h>
#include "ctype.h"

typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;

#include "quad.h"
/*
 * This macro casts away the qualifier from the variable
 *
 * Note: use at your own risk, removing qualifiers can result in
 * catastrophic run-time failures.
 */
#ifndef __CAST_AWAY_QUALIFIER
#define __CAST_AWAY_QUALIFIER(variable, qualifier, type)  (type) (long)(variable)
#endif

typedef char* caddr_t;


/*
 * iloop
 */
#define ever ;;

__attribute__ ((__unused__)) static void iloop (void)
{
	for (ever) {}
}

/*
 * string.c
 */

extern char * strbreak(const char *str, char **next, long *len);
extern int    ptol(const char * str);

extern void	bcopy(const void *, void *, size_t);
extern void	bzero(void *, size_t);

extern void	*memcpy(void *, const void *, size_t);
extern int	memcmp(const void *, const void *, size_t);
extern void	*memmove(void *, const void *, size_t);
extern void	*memset(void *, int, size_t);

extern size_t	strlen(const char *);
extern size_t	strnlen(const char *, size_t);

/* strcpy() is being deprecated. Please use strlcpy() instead. */
extern char	*strcpy(char *, const char *);
extern char	*strncpy(char *, const char *, size_t);

extern size_t	strlcat(char *, const char *, size_t);
extern size_t	strlcpy(char *, const char *, size_t);

/* strcat() is being deprecated. Please use strlcat() instead. */
extern char	*strcat(char *, const char *);
extern char * strncat(char * s1, const char * s2, size_t n);

/* strcmp() is being deprecated. Please use strncmp() instead. */
extern int	strcmp(const char *, const char *);
extern int	strncmp(const char *,const char *, size_t);

#if STRNCASECMP
extern int	strcasecmp(const char *s1, const char *s2);
extern int	strncasecmp(const char *s1, const char *s2, size_t n);
#endif
extern char	*strdup(const char *);
extern char	*strchr(const char *s, int c);
extern int atoi(const char *);
extern char *itoa(int	,char	*);
extern const char *strstr(const char *, const char *);
extern void * memchr(const void *, int , size_t );
extern unsigned long
local_adler32( unsigned char * buffer, long length );
extern void * bsearch(register const void *key,const void *base0,size_t nmemb,register size_t size,register int (*compar)(const void *, const void *));
/*
 * strtol.c
 */
extern long strtol(const char * nptr, char ** endptr, int base);
extern unsigned long strtoul(const char * nptr, char ** endptr, int base);
extern unsigned long long strtouq(const char *nptr, char ** endptr, int base);

/*
 * printf.c
 */
extern int sprintf(char *s, const char * format, ...);
extern int snprintf(char *str, size_t size, const char *format, ...);
extern int vsnprintf(char *str, size_t size, const char *format, va_list ap);
extern void
_doprnt(
        register const char     *fmt,
        va_list	                 argp,
        /* character output routine */
        void                    (*putc)(char),
        int                     radix);          /* default radix - for '%r' */

extern int
prf(
    const char	*fmt,
    va_list				ap,
    /* character output routine */
    void			(*putc)(char));

extern int
__doprnt(
         const char	*fmt,
         va_list		argp,
         /* character output routine */
         void			(*putc)(int ch, void *arg),
         void                    *arg,
         int			radix);


/*
 * __printf.c
 */
void __printf_init(void (*print_fn)(char*));
void __printf(const char * format, ...);

/*
 * qsort.c
 */
extern void qsort(void *a,size_t n,size_t es, int (*cmp)());


/*
 * sbrk.c
 */
extern caddr_t sbrk(int size);
extern caddr_t brk(caddr_t x);

/*
 * jemalloc.c
 */
extern void   free(void * start);
extern void * realloc(void * ptr, size_t size);
#if 0
extern void * reallocf(void * ptr, size_t size);
#endif
extern void * malloc(size_t size);
extern void * calloc(size_t number, size_t size);

extern size_t malloc_usable_size(const void *ptr);
extern int posix_memalign(void **memptr, size_t alignment, size_t size);
extern void	malloc_print_stats(void);

extern int	 ffs(int);

#endif /* !__BOOT_LIBSA_H */
