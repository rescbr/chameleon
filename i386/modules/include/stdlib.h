/*
 * Copyright (c) 2000, 2002 - 2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
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
/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *
 *	@(#)stdlib.h	8.5 (Berkeley) 5/19/95
 */

#ifndef _STDLIB_H_
#define _STDLIB_H_

#include <Availability.h>

#include <_types.h>

#ifndef	_SIZE_T
#define	_SIZE_T
/* DO NOT REMOVE THIS COMMENT: fixincludes needs to see:
 * _GCC_SIZE_T */
typedef	__darwin_size_t		size_t;
#endif

typedef struct {
	int quot;		/* quotient */
	int rem;		/* remainder */
} div_t;

typedef struct {
	long quot;		/* quotient */
	long rem;		/* remainder */
} ldiv_t;

#if !__DARWIN_NO_LONG_LONG
typedef struct {
	long long quot;
	long long rem;
} lldiv_t;
#endif /* !__DARWIN_NO_LONG_LONG */

#ifndef NULL
#define NULL __DARWIN_NULL
#endif /* ! NULL */

#define	EXIT_FAILURE	1
#define	EXIT_SUCCESS	0

#define	RAND_MAX	0x7fffffff

__BEGIN_DECLS
void	 abort(void) ;
int		 abs(int);
int	 atoi(const char *);
long	 atol(const char *);
void	*bsearch(const void *, const void *, size_t,
	    size_t, int (*)(const void *, const void *));
void	*calloc(size_t, size_t);
#if 1
void	 exit(int) ;
#else
void Throw(int);
#ifdef DEBUG_ALL_EXIT
#define exit(e)  \
((void) (__exit (#e,e, __FILE__, __LINE__)))
#define __exit(estr, e, file, line) \
((void)printf ("%s:%u: exit status `%s'\n", file, line, estr), Throw(e))
#else
#define exit(e)  \
((void) ((e == 0) ? Throw(EXIT_SUCCESS) : __exit (#e, e, __FILE__, __LINE__)))
#define __exit(estr, e, file, line) \
((void)printf ("%s:%u: exit status `%s'\n", file, line, estr), Throw(e))
#endif
#endif
void	 free(void *);
void	*malloc(size_t);
int 	 posix_memalign(void **, size_t, size_t);
void	 qsort(void *, size_t, size_t,
	    int (*)(const void *, const void *));
int	 rand(void);
void	*realloc(void *, size_t);
void	 srand(unsigned);
long	 strtol(const char *, char **, int);

extern void * get_env_ptr(const char *name);
extern int safe_set_env_ptr(const char *name , void * ptr, size_t size );

#define	getenv(x) (char *)get_env_ptr(x)  
#define	setenv(x,y,z) (char *)safe_set_env_ptr(x,(void*)y,(size_t)z)  

unsigned long
	 strtoul(const char *, char **, int);

#include <machine/types.h>

u_int32_t
	 arc4random(void);
void	 arc4random_addrandom(unsigned char * /*dat*/, int /*datlen*/);
void	 arc4random_buf(void * /*buf*/, size_t /*nbytes*/);
void	 arc4random_stir(void);
u_int32_t
	 arc4random_uniform(u_int32_t /*upper_bound*/);

const char
	*getprogname(void);
void	 setprogname(const char *);

void	*reallocf(void *, size_t);

unsigned long long
	 strtouq(const char *, char **, int);
void	*valloc(size_t);

__END_DECLS


#endif /* _STDLIB_H_ */
