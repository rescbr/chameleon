/*
 * Copyright (c) 2004-2006 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#include <sys/cdefs.h>
#include <sys/_types.h>

#ifdef __need_struct_timespec
#undef __need_struct_timespec
#ifndef _STRUCT_TIMESPEC
#define _STRUCT_TIMESPEC	struct timespec
_STRUCT_TIMESPEC
{
	__darwin_time_t	tv_sec;
	long		tv_nsec;
};
#endif /* _STRUCT_TIMESPEC */
#endif /* __need_struct_timespec */

#ifdef __need_struct_timeval
#undef __need_struct_timeval
#ifndef _STRUCT_TIMEVAL
#define _STRUCT_TIMEVAL		struct timeval
_STRUCT_TIMEVAL
{
	__darwin_time_t		tv_sec;		/* seconds */
	__darwin_suseconds_t	tv_usec;	/* and microseconds */
};
#endif /* _STRUCT_TIMEVAL */
#endif /* __need_struct_timeval */

#ifdef __need_struct_timeval32
#undef __need_struct_timeval32
#ifndef _STRUCT_TIMEVAL32
#define _STRUCT_TIMEVAL32	struct timeval32
_STRUCT_TIMEVAL32
{
	__int32_t		tv_sec;		/* seconds */
	__int32_t		tv_usec;	/* and microseconds */
};
#endif /* _STRUCT_TIMEVAL32 */
#endif /* __need_struct_timeval32 */


#ifdef __need_fd_set
#undef __need_fd_set
#ifndef _FD_SET
#define _FD_SET
/*
 * Select uses bit masks of file descriptors in longs.  These macros
 * manipulate such bit fields (the filesystem macros use chars).  The
 * extra protection here is to permit application redefinition above
 * the default size.
 */
#ifdef FD_SETSIZE
#define	__DARWIN_FD_SETSIZE	FD_SETSIZE
#else /* !FD_SETSIZE */
#define	__DARWIN_FD_SETSIZE	1024
#endif /* FD_SETSIZE */
#define	__DARWIN_NBBY		8				/* bits in a byte */
#define __DARWIN_NFDBITS	(sizeof(__int32_t) * __DARWIN_NBBY) /* bits per mask */
#define	__DARWIN_howmany(x, y)	((((x) % (y)) == 0) ? ((x) / (y)) : (((x) / (y)) + 1)) /* # y's == x bits? */

__BEGIN_DECLS
typedef	struct fd_set {
	__int32_t	fds_bits[__DARWIN_howmany(__DARWIN_FD_SETSIZE, __DARWIN_NFDBITS)];
} fd_set;
__END_DECLS

/* This inline avoids argument side-effect issues with FD_ISSET() */
static __inline int
__darwin_fd_isset(int _n, const struct fd_set *_p)
{
	return (_p->fds_bits[_n/__DARWIN_NFDBITS] & (1<<(_n % __DARWIN_NFDBITS)));
}

#define	__DARWIN_FD_SET(n, p)	do { int __fd = (n); ((p)->fds_bits[__fd/__DARWIN_NFDBITS] |= (1<<(__fd % __DARWIN_NFDBITS))); } while(0)
#define	__DARWIN_FD_CLR(n, p)	do { int __fd = (n); ((p)->fds_bits[__fd/__DARWIN_NFDBITS] &= ~(1<<(__fd % __DARWIN_NFDBITS))); } while(0)
#define	__DARWIN_FD_ISSET(n, p)	__darwin_fd_isset((n), (p))

#if __GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ >= 3
/*
 * Use the built-in bzero function instead of the library version so that
 * we do not pollute the namespace or introduce prototype warnings.
 */
#define	__DARWIN_FD_ZERO(p)	__builtin_bzero(p, sizeof(*(p)))
#else
#define	__DARWIN_FD_ZERO(p)	bzero(p, sizeof(*(p)))
#endif

#define	__DARWIN_FD_COPY(f, t)	bcopy(f, t, sizeof(*(f)))
#endif /* _FD_SET */
#endif /* __need_fd_set */
