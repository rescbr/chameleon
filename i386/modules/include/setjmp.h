/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
#ifndef _BSD_SETJMP_H
#define _BSD_SETJMP_H

#include <sys/cdefs.h>

#if defined(__x86_64__)
/*
 * _JBLEN is number of ints required to save the following:
 * rflags, rip, rbp, rsp, rbx, r12, r13, r14, r15... these are 8 bytes each
 * mxcsr, fp control word, sigmask... these are 4 bytes each
 * add 16 ints for future expansion needs...
 */
#define _JBLEN ((9 * 2) + 3 + 16)
typedef int jmp_buf[_JBLEN];
typedef int sigjmp_buf[_JBLEN + 1];

#elif defined(__i386__)

/*
 * _JBLEN is number of ints required to save the following:
 * eax, ebx, ecx, edx, edi, esi, ebp, esp, ss, eflags, eip,
 * cs, de, es, fs, gs == 16 ints
 * onstack, mask = 2 ints
 */

#define _JBLEN (18)
typedef int jmp_buf[_JBLEN];

#elif defined(__arm__)

#include <machine/signal.h>

/*
 *	_JBLEN is number of ints required to save the following:
 *	r4-r8, r10, fp, sp, lr, sig  == 10 register_t sized
 *	s16-s31 == 16 register_t sized + 1 int for FSTMX
 *	1 extra int for future use
 */
#define _JBLEN		(10 + 16 + 2)
#define _JBLEN_MAX	_JBLEN

typedef int jmp_buf[_JBLEN];
typedef int sigjmp_buf[_JBLEN + 1];

#else
#	error Undefined platform for setjmp
#endif

__BEGIN_DECLS
extern int	setjmp(jmp_buf);
extern void longjmp(jmp_buf, int) ;

#ifndef _ANSI_SOURCE
int	_setjmp(jmp_buf);
void	_longjmp(jmp_buf, int) ;
#endif /* _ANSI_SOURCE  */

__END_DECLS

#endif /* _BSD_SETJMP_H */
