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
/*
 * ctype stuff (aserebln)
 */
#define isupper(c)  (c >= 'A' && c <= 'Z')
#define islower(c)  (c >= 'a' && c <= 'z')
#define isalpha(c)  ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
#define isascii(c)  ((c >= 0x20) && (c < 0x7f))
#define isspace(c)  (c == ' ' || c == '\t' || c == '\n' || c == '\12')
#define isdigit(c)  (c >= '0' && c <= '9')
#define isxdigit(c) ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))
#define ispunct(c)  (c == '.' || c == '-') //Azi: TODO - add more ponctuation characters as needed; at least these two, i need for PartNo.

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

extern uint8_t checksum8( void * start, unsigned int length );
extern unsigned long
adler32( unsigned char * buffer, long length );
extern void * bsearch(register const void *key,const void *base0,size_t nmemb,register size_t size,register int (*compar)(const void *, const void *));
/*
 * strtol.c
 */
extern long strtol(const char * nptr, char ** endptr, int base);
extern unsigned long strtoul(const char * nptr, char ** endptr, int base);
extern unsigned long long strtouq(const char *nptr, char ** endptr, int base);

/*
 * prf.c
 */
//extern int prf(const char * fmt, va_list ap, void (*putfn_p)(),
//			   void * putfn_arg);

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
 * zalloc.c
 */

extern void   free(void * start);
extern void * realloc(void * ptr, size_t size);

#ifdef SAFE_MALLOC
extern size_t zalloced_size;
extern void malloc_init(char * start, int size, int nodes, void (*malloc_err_fn)(char *, size_t, const char *, int));
#else
extern void   malloc_init(char * start, int size, int nodes, void (*malloc_error)(char *, size_t));
#endif
extern void * malloc(size_t size);

#endif /* !__BOOT_LIBSA_H */
