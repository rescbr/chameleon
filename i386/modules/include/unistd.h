/*
 * Copyright (c) 2000, 2002-2006, 2008-2010, 2012 Apple Inc. All rights reserved.
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
 * Copyright (c) 1998-1999 Apple Computer, Inc. All Rights Reserved
 * Copyright (c) 1991, 1993, 1994
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
 *	@(#)unistd.h	8.12 (Berkeley) 4/27/95
 *
 *  Copyright (c)  1998 Apple Compter, Inc.
 *  All Rights Reserved
 */

/* History:
        7/14/99 EKN at Apple fixed getdirentriesattr from getdirentryattr
        3/26/98 CHW at Apple added real interface to searchfs call
  	3/5/98  CHW at Apple added hfs semantic system calls headers
*/

#ifndef _UNISTD_H_
#define	_UNISTD_H_

#include <_types.h>
#include <sys/unistd.h>
#include <Availability.h>

#ifndef _GID_T
#define	_GID_T
typedef __darwin_gid_t		gid_t;
#endif

#ifndef _INTPTR_T
#define	_INTPTR_T
typedef __darwin_intptr_t	intptr_t;
#endif

#ifndef _OFF_T
#define	_OFF_T
typedef __darwin_off_t		off_t;
#endif

#ifndef _SIZE_T
#define	_SIZE_T
/* DO NOT REMOVE THIS COMMENT: fixincludes needs to see:
 * _GCC_SIZE_T */
typedef __darwin_size_t		size_t;
#endif

#ifndef	_SSIZE_T
#define	_SSIZE_T
typedef	__darwin_ssize_t	ssize_t;
#endif

#ifndef _UID_T
#define	_UID_T
typedef __darwin_uid_t		uid_t;	/* user id 	*/
#endif

#ifndef _USECONDS_T
#define	_USECONDS_T
typedef __darwin_useconds_t	useconds_t;
#endif

#ifndef NULL
#define	NULL __DARWIN_NULL
#endif /* ! NULL */

#define	 STDIN_FILENO	0	/* standard input file descriptor */
#define	STDOUT_FILENO	1	/* standard output file descriptor */
#define	STDERR_FILENO	2	/* standard error file descriptor */


/* Version test macros */
/* _POSIX_VERSION and _POSIX2_VERSION from sys/unistd.h */
#define	_XOPEN_VERSION			600		/* [XSI] */
#define	_XOPEN_XCU_VERSION		4		/* Older standard */


/* Please keep this list in the same order as the applicable standard */
#define	_POSIX_ADVISORY_INFO		(-1)		/* [ADV] */
#define	_POSIX_ASYNCHRONOUS_IO		(-1)		/* [AIO] */
#define	_POSIX_BARRIERS			(-1)		/* [BAR] */
#define	_POSIX_CHOWN_RESTRICTED		200112L
#define	_POSIX_CLOCK_SELECTION		(-1)		/* [CS] */
#define	_POSIX_CPUTIME			(-1)		/* [CPT] */
#define	_POSIX_FSYNC			200112L		/* [FSC] */
#define	_POSIX_IPV6			200112L
#define	_POSIX_JOB_CONTROL		200112L
#define	_POSIX_MAPPED_FILES		200112L		/* [MF] */
#define	_POSIX_MEMLOCK			(-1)		/* [ML] */
#define	_POSIX_MEMLOCK_RANGE		(-1)		/* [MR] */
#define	_POSIX_MEMORY_PROTECTION	200112L		/* [MPR] */
#define	_POSIX_MESSAGE_PASSING		(-1)		/* [MSG] */
#define	_POSIX_MONOTONIC_CLOCK		(-1)		/* [MON] */
#define	_POSIX_NO_TRUNC			200112L
#define	_POSIX_PRIORITIZED_IO		(-1)		/* [PIO] */
#define	_POSIX_PRIORITY_SCHEDULING	(-1)		/* [PS] */
#define	_POSIX_RAW_SOCKETS		(-1)		/* [RS] */
#define	_POSIX_READER_WRITER_LOCKS	200112L		/* [THR] */
#define	_POSIX_REALTIME_SIGNALS		(-1)		/* [RTS] */
#define	_POSIX_REGEXP			200112L
#define	_POSIX_SAVED_IDS		200112L		/* XXX required */
#define	_POSIX_SEMAPHORES		(-1)		/* [SEM] */
#define	_POSIX_SHARED_MEMORY_OBJECTS	(-1)		/* [SHM] */
#define	_POSIX_SHELL			200112L
#define	_POSIX_SPAWN			(-1)		/* [SPN] */
#define	_POSIX_SPIN_LOCKS		(-1)		/* [SPI] */
#define	_POSIX_SPORADIC_SERVER		(-1)		/* [SS] */
#define	_POSIX_SYNCHRONIZED_IO		(-1)		/* [SIO] */
#define	_POSIX_THREAD_ATTR_STACKADDR	200112L		/* [TSA] */
#define	_POSIX_THREAD_ATTR_STACKSIZE	200112L		/* [TSS] */
#define	_POSIX_THREAD_CPUTIME		(-1)		/* [TCT] */
#define	_POSIX_THREAD_PRIO_INHERIT	(-1)		/* [TPI] */
#define	_POSIX_THREAD_PRIO_PROTECT	(-1)		/* [TPP] */
#define	_POSIX_THREAD_PRIORITY_SCHEDULING	(-1)	/* [TPS] */
#define	_POSIX_THREAD_PROCESS_SHARED	200112L		/* [TSH] */
#define	_POSIX_THREAD_SAFE_FUNCTIONS	200112L		/* [TSF] */
#define	_POSIX_THREAD_SPORADIC_SERVER	(-1)		/* [TSP] */
#define	_POSIX_THREADS			200112L		/* [THR] */
#define	_POSIX_TIMEOUTS			(-1)		/* [TMO] */
#define	_POSIX_TIMERS			(-1)		/* [TMR] */
#define	_POSIX_TRACE			(-1)		/* [TRC] */
#define	_POSIX_TRACE_EVENT_FILTER	(-1)		/* [TEF] */
#define	_POSIX_TRACE_INHERIT		(-1)		/* [TRI] */
#define	_POSIX_TRACE_LOG		(-1)		/* [TRL] */
#define	_POSIX_TYPED_MEMORY_OBJECTS	(-1)		/* [TYM] */
#ifndef _POSIX_VDISABLE
#define	_POSIX_VDISABLE			0xff		/* same as sys/termios.h */
#endif /* _POSIX_VDISABLE */

#if __DARWIN_C_LEVEL >= 199209L
#define	_POSIX2_C_BIND			200112L
#define	_POSIX2_C_DEV			200112L		/* c99 command */
#define	_POSIX2_CHAR_TERM		200112L
#define	_POSIX2_FORT_DEV		(-1)		/* fort77 command */
#define	_POSIX2_FORT_RUN		200112L
#define	_POSIX2_LOCALEDEF		200112L		/* localedef command */
#define	_POSIX2_PBS			(-1)
#define	_POSIX2_PBS_ACCOUNTING		(-1)
#define	_POSIX2_PBS_CHECKPOINT		(-1)
#define	_POSIX2_PBS_LOCATE		(-1)
#define	_POSIX2_PBS_MESSAGE		(-1)
#define	_POSIX2_PBS_TRACK		(-1)
#define	_POSIX2_SW_DEV			200112L
#define	_POSIX2_UPE			200112L	/* XXXX no fc, newgrp, tabs */
#endif /* __DARWIN_C_LEVEL */

#define	__ILP32_OFF32          (-1)
#define	__ILP32_OFFBIG         (1)
#define	__LP64_OFF64           (1)
#define	__LPBIG_OFFBIG         (1)

#if __DARWIN_C_LEVEL >= 200112L
#define	_POSIX_V6_ILP32_OFF32		__ILP32_OFF32
#define	_POSIX_V6_ILP32_OFFBIG		__ILP32_OFFBIG
#define	_POSIX_V6_LP64_OFF64		__LP64_OFF64
#define	_POSIX_V6_LPBIG_OFFBIG		__LPBIG_OFFBIG
#endif /* __DARWIN_C_LEVEL >= 200112L */

#if __DARWIN_C_LEVEL >= 200809L
#define	_POSIX_V7_ILP32_OFF32		__ILP32_OFF32
#define	_POSIX_V7_ILP32_OFFBIG		__ILP32_OFFBIG
#define	_POSIX_V7_LP64_OFF64		__LP64_OFF64
#define	_POSIX_V7_LPBIG_OFFBIG		__LPBIG_OFFBIG
#endif /* __DARWIN_C_LEVEL >= 200809L */

#if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
#define	_V6_ILP32_OFF32             __ILP32_OFF32
#define	_V6_ILP32_OFFBIG            __ILP32_OFFBIG
#define	_V6_LP64_OFF64              __LP64_OFF64
#define	_V6_LPBIG_OFFBIG            __LPBIG_OFFBIG
#endif /* __DARWIN_C_LEVEL >= __DARWIN_C_FULL */

#if (__DARWIN_C_LEVEL >= 199506L && __DARWIN_C_LEVEL < 200809L) || __DARWIN_C_LEVEL >= __DARWIN_C_FULL
/* Removed in Issue 7 */
#define	_XBS5_ILP32_OFF32		    __ILP32_OFF32
#define	_XBS5_ILP32_OFFBIG		    __ILP32_OFFBIG
#define	_XBS5_LP64_OFF64		    __LP64_OFF64
#define	_XBS5_LPBIG_OFFBIG		    __LPBIG_OFFBIG
#endif /* __DARWIN_C_LEVEL < 200809L */

#if __DARWIN_C_LEVEL >= 199506L /* This really should be XSI */ 
#define	_XOPEN_CRYPT			(1)
#define	_XOPEN_ENH_I18N			(1)		/* XXX required */
#define	_XOPEN_LEGACY			(-1)	/* no ftime gcvt, wcswcs */
#define	_XOPEN_REALTIME			(-1)	/* no q'ed signals, mq_* */
#define	_XOPEN_REALTIME_THREADS		(-1)	/* no posix_spawn, et. al. */
#define	_XOPEN_SHM			(1)
#define	_XOPEN_STREAMS			(-1)   /* Issue 6 */
#define	_XOPEN_UNIX			(1)
#endif /* XSI */

/* configurable system variables */
#define	_SC_ARG_MAX			 1
#define	_SC_CHILD_MAX			 2
#define	_SC_CLK_TCK			 3
#define	_SC_NGROUPS_MAX			 4
#define	_SC_OPEN_MAX			 5
#define	_SC_JOB_CONTROL			 6
#define	_SC_SAVED_IDS			 7
#define	_SC_VERSION			 8
#define	_SC_BC_BASE_MAX			 9
#define	_SC_BC_DIM_MAX			10
#define	_SC_BC_SCALE_MAX		11
#define	_SC_BC_STRING_MAX		12
#define	_SC_COLL_WEIGHTS_MAX		13
#define	_SC_EXPR_NEST_MAX		14
#define	_SC_LINE_MAX			15
#define	_SC_RE_DUP_MAX			16
#define	_SC_2_VERSION			17
#define	_SC_2_C_BIND			18
#define	_SC_2_C_DEV			19
#define	_SC_2_CHAR_TERM			20
#define	_SC_2_FORT_DEV			21
#define	_SC_2_FORT_RUN			22
#define	_SC_2_LOCALEDEF			23
#define	_SC_2_SW_DEV			24
#define	_SC_2_UPE			25
#define	_SC_STREAM_MAX			26
#define	_SC_TZNAME_MAX			27

#if __DARWIN_C_LEVEL >= 199309L
#define	_SC_ASYNCHRONOUS_IO		28
#define	_SC_PAGESIZE			29
#define	_SC_MEMLOCK			30
#define	_SC_MEMLOCK_RANGE		31
#define	_SC_MEMORY_PROTECTION		32
#define	_SC_MESSAGE_PASSING		33
#define	_SC_PRIORITIZED_IO		34
#define	_SC_PRIORITY_SCHEDULING		35
#define	_SC_REALTIME_SIGNALS		36
#define	_SC_SEMAPHORES			37
#define	_SC_FSYNC			38
#define	_SC_SHARED_MEMORY_OBJECTS 	39
#define	_SC_SYNCHRONIZED_IO		40
#define	_SC_TIMERS			41
#define	_SC_AIO_LISTIO_MAX		42
#define	_SC_AIO_MAX			43
#define	_SC_AIO_PRIO_DELTA_MAX		44
#define	_SC_DELAYTIMER_MAX		45
#define	_SC_MQ_OPEN_MAX			46
#define	_SC_MAPPED_FILES		47	/* swap _SC_PAGESIZE vs. BSD */
#define	_SC_RTSIG_MAX			48
#define	_SC_SEM_NSEMS_MAX		49
#define	_SC_SEM_VALUE_MAX		50
#define	_SC_SIGQUEUE_MAX		51
#define	_SC_TIMER_MAX			52
#endif /* __DARWIN_C_LEVEL >= 199309L */

#if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
#define	_SC_NPROCESSORS_CONF		57
#define	_SC_NPROCESSORS_ONLN		58
#endif /* __DARWIN_C_LEVEL >= __DARWIN_C_FULL */

#if __DARWIN_C_LEVEL >= 200112L
#define	_SC_2_PBS			59
#define	_SC_2_PBS_ACCOUNTING		60
#define	_SC_2_PBS_CHECKPOINT		61
#define	_SC_2_PBS_LOCATE		62
#define	_SC_2_PBS_MESSAGE		63
#define	_SC_2_PBS_TRACK			64
#define	_SC_ADVISORY_INFO		65
#define	_SC_BARRIERS			66
#define	_SC_CLOCK_SELECTION		67
#define	_SC_CPUTIME			68
#define	_SC_FILE_LOCKING		69
#define	_SC_GETGR_R_SIZE_MAX		70
#define	_SC_GETPW_R_SIZE_MAX		71
#define	_SC_HOST_NAME_MAX		72
#define	_SC_LOGIN_NAME_MAX		73
#define	_SC_MONOTONIC_CLOCK		74
#define	_SC_MQ_PRIO_MAX			75
#define	_SC_READER_WRITER_LOCKS		76
#define	_SC_REGEXP			77
#define	_SC_SHELL			78
#define	_SC_SPAWN			79
#define	_SC_SPIN_LOCKS			80
#define	_SC_SPORADIC_SERVER		81
#define	_SC_THREAD_ATTR_STACKADDR	82
#define	_SC_THREAD_ATTR_STACKSIZE	83
#define	_SC_THREAD_CPUTIME		84
#define	_SC_THREAD_DESTRUCTOR_ITERATIONS 85
#define	_SC_THREAD_KEYS_MAX		86
#define	_SC_THREAD_PRIO_INHERIT		87
#define	_SC_THREAD_PRIO_PROTECT		88
#define	_SC_THREAD_PRIORITY_SCHEDULING	89
#define	_SC_THREAD_PROCESS_SHARED	90
#define	_SC_THREAD_SAFE_FUNCTIONS	91
#define	_SC_THREAD_SPORADIC_SERVER	92
#define	_SC_THREAD_STACK_MIN		93
#define	_SC_THREAD_THREADS_MAX		94
#define	_SC_TIMEOUTS			95
#define	_SC_THREADS			96
#define	_SC_TRACE			97
#define	_SC_TRACE_EVENT_FILTER		98
#define	_SC_TRACE_INHERIT		99
#define	_SC_TRACE_LOG			100
#define	_SC_TTY_NAME_MAX		101
#define	_SC_TYPED_MEMORY_OBJECTS	102
#define	_SC_V6_ILP32_OFF32		103
#define	_SC_V6_ILP32_OFFBIG		104
#define	_SC_V6_LP64_OFF64		105
#define	_SC_V6_LPBIG_OFFBIG		106
#define	_SC_IPV6			118
#define	_SC_RAW_SOCKETS			119
#define	_SC_SYMLOOP_MAX			120
#endif /* __DARWIN_C_LEVEL >= 200112L */

#if __DARWIN_C_LEVEL >= 199506L /* Really XSI */
#define	_SC_ATEXIT_MAX			107
#define	_SC_IOV_MAX			56
#define	_SC_PAGE_SIZE			_SC_PAGESIZE
#define	_SC_XOPEN_CRYPT			108
#define	_SC_XOPEN_ENH_I18N		109
#define	_SC_XOPEN_LEGACY		110      /* Issue 6 */
#define	_SC_XOPEN_REALTIME		111      /* Issue 6 */
#define	_SC_XOPEN_REALTIME_THREADS	112  /* Issue 6 */
#define	_SC_XOPEN_SHM			113
#define	_SC_XOPEN_STREAMS		114      /* Issue 6 */
#define	_SC_XOPEN_UNIX			115
#define	_SC_XOPEN_VERSION		116
#define	_SC_XOPEN_XCU_VERSION		121
#endif /* XSI */

#if (__DARWIN_C_LEVEL >= 199506L && __DARWIN_C_LEVEL < 200809L) || __DARWIN_C_LEVEL >= __DARWIN_C_FULL
/* Removed in Issue 7 */
#define	_SC_XBS5_ILP32_OFF32		122
#define	_SC_XBS5_ILP32_OFFBIG		123
#define	_SC_XBS5_LP64_OFF64		124
#define	_SC_XBS5_LPBIG_OFFBIG		125
#endif /* __DARWIN_C_LEVEL <= 200809L */

#if __DARWIN_C_LEVEL >= 200112L
#define	_SC_SS_REPL_MAX			126
#define	_SC_TRACE_EVENT_NAME_MAX	127
#define	_SC_TRACE_NAME_MAX		128
#define	_SC_TRACE_SYS_MAX		129
#define	_SC_TRACE_USER_EVENT_MAX	130
#endif

#if __DARWIN_C_LEVEL < 200112L || __DARWIN_C_LEVEL >= __DARWIN_C_FULL
/* Removed in Issue 6 */
#define	_SC_PASS_MAX			131
#endif

#if __DARWIN_C_LEVEL >= 199209L
#ifndef _CS_PATH /* Defined in <sys/unistd.h> */
#define	_CS_PATH				1
#endif
#endif

#if __DARWIN_C_LEVEL >= 200112
#define	_CS_POSIX_V6_ILP32_OFF32_CFLAGS		2
#define	_CS_POSIX_V6_ILP32_OFF32_LDFLAGS	3
#define	_CS_POSIX_V6_ILP32_OFF32_LIBS		4
#define	_CS_POSIX_V6_ILP32_OFFBIG_CFLAGS	5
#define	_CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS	6
#define	_CS_POSIX_V6_ILP32_OFFBIG_LIBS		7
#define	_CS_POSIX_V6_LP64_OFF64_CFLAGS		8
#define	_CS_POSIX_V6_LP64_OFF64_LDFLAGS		9
#define	_CS_POSIX_V6_LP64_OFF64_LIBS		10
#define	_CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS	11
#define	_CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS	12
#define	_CS_POSIX_V6_LPBIG_OFFBIG_LIBS		13
#define	_CS_POSIX_V6_WIDTH_RESTRICTED_ENVS	14
#endif

#if (__DARWIN_C_LEVEL >= 199506L && __DARWIN_C_LEVEL < 200809L) || __DARWIN_C_LEVEL >= __DARWIN_C_FULL
/* Removed in Issue 7 */
#define	_CS_XBS5_ILP32_OFF32_CFLAGS		20
#define	_CS_XBS5_ILP32_OFF32_LDFLAGS		21
#define	_CS_XBS5_ILP32_OFF32_LIBS		22
#define	_CS_XBS5_ILP32_OFF32_LINTFLAGS		23
#define	_CS_XBS5_ILP32_OFFBIG_CFLAGS		24
#define	_CS_XBS5_ILP32_OFFBIG_LDFLAGS		25
#define	_CS_XBS5_ILP32_OFFBIG_LIBS		26
#define	_CS_XBS5_ILP32_OFFBIG_LINTFLAGS		27
#define	_CS_XBS5_LP64_OFF64_CFLAGS		28
#define	_CS_XBS5_LP64_OFF64_LDFLAGS		29
#define	_CS_XBS5_LP64_OFF64_LIBS		30
#define	_CS_XBS5_LP64_OFF64_LINTFLAGS		31
#define	_CS_XBS5_LPBIG_OFFBIG_CFLAGS		32
#define	_CS_XBS5_LPBIG_OFFBIG_LDFLAGS		33
#define	_CS_XBS5_LPBIG_OFFBIG_LIBS		34
#define	_CS_XBS5_LPBIG_OFFBIG_LINTFLAGS		35
#endif

#if __DARWIN_C_LEVEL >= __DARWIN_C_FULL
#define	_CS_DARWIN_USER_DIR			65536
#define	_CS_DARWIN_USER_TEMP_DIR		65537
#define	_CS_DARWIN_USER_CACHE_DIR		65538
#endif /* __DARWIN_C_LEVEL >= __DARWIN_C_FULL */


#ifdef _DARWIN_UNLIMITED_GETGROUPS
#if defined(__IPHONE_OS_VERSION_MIN_REQUIRED) && __IPHONE_OS_VERSION_MIN_REQUIRED < __IPHONE_3_2
#error "_DARWIN_UNLIMITED_GETGROUPS specified, but -miphoneos-version-min version does not support it."
#elif defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_6
#error "_DARWIN_UNLIMITED_GETGROUPS specified, but -mmacosx-version-min version does not support it."
#endif
#endif

/* POSIX.1-1990 */

__BEGIN_DECLS
void	 _exit(int) ;
int	 close(int);
off_t	 lseek(int, off_t, int);
int	 pause(void) ;
ssize_t	 read(int, void *, size_t) ;
unsigned int
	 sleep(unsigned int) ;
ssize_t	 write(int, const void *, size_t);
__END_DECLS

/* Additional functionality provided by:
 * POSIX.1c-1995,
 * POSIX.1i-1995,
 * and the omnibus ISO/IEC 9945-1: 1996
 */

#if __DARWIN_C_LEVEL >= 199506L
                               /* These F_* are really XSI or Issue 6 */
#define F_ULOCK         0      /* unlock locked section */
#define	F_LOCK          1      /* lock a section for exclusive use */
#define	F_TLOCK         2      /* test and lock a section for exclusive use */
#define	F_TEST          3      /* test a section for locks by other procs */

 __BEGIN_DECLS

/* Begin XSI */
void	*brk(const void *);
int	 getpagesize(void) __pure2 /*__POSIX_C_DEPRECATED*/;
void	*sbrk(int);
/* End XSI */

__END_DECLS
#endif /* __DARWIN_C_LEVEL >= 199506L */

__BEGIN_DECLS
void delay(int ms);
__END_DECLS

#endif /* _UNISTD_H_ */
