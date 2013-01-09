/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
/*
 *	File:	sbrk.c
 *
 *	Unix compatibility for sbrk system call.
 *
 * HISTORY
 * 09-Mar-90  Gregg Kellogg (gk) at NeXT.
 *	include <kern/mach_interface.h> instead of <kern/mach.h>
 *
 * 14-Feb-89  Avadis Tevanian (avie) at NeXT.
 *	Total rewrite using a fixed area of VM from break region.
 */

#include "libsa.h"
#include "memory.h"

/*
 * natural_t and integer_t are Mach's legacy types for machine-
 * independent integer types (unsigned, and signed, respectively).
 * Their original purpose was to define other types in a machine/
 * compiler independent way.
 *
 * They also had an implicit "same size as pointer" characteristic
 * to them (i.e. Mach's traditional types are very ILP32 or ILP64
 * centric).  We support x86 ABIs that do not follow either of
 * these models (specifically LP64).  Therefore, we had to make a
 * choice between making these types scale with pointers or stay
 * tied to integers.  Because their use is predominantly tied to
 * to the size of an integer, we are keeping that association and
 * breaking free from pointer size guarantees.
 *
 * New use of these types is discouraged.
 */
typedef __darwin_natural_t	natural_t;

/*
 * A vm_offset_t is a type-neutral pointer,
 * e.g. an offset into a virtual memory space.
 */
#ifdef __LP64__
typedef uintptr_t		vm_offset_t;
#else	/* __LP64__ */
typedef	natural_t		vm_offset_t;
#endif	/* __LP64__ */

/*
 * A vm_size_t is the proper type for e.g.
 * expressing the difference between two
 * vm_offset_t entities.
 */
#ifdef __LP64__
typedef uintptr_t		vm_size_t;
#else	/* __LP64__ */
typedef	natural_t		vm_size_t;
#endif	/* __LP64__ */

typedef vm_offset_t     	vm_address_t;

static vm_size_t sbrk_region_size = (vm_size_t)MALLOC_LEN; /* Well, what should it be? */
static vm_address_t sbrk_curbrk = (vm_address_t)MALLOC_ADDR;

caddr_t sbrk(size)
int	size;
{		
	if (size <= 0)
		return((caddr_t)sbrk_curbrk);
    
    if (size > sbrk_region_size)
        return((caddr_t)-1);
    
	sbrk_curbrk += size;
	sbrk_region_size -= size;
	
	return((caddr_t)(sbrk_curbrk - size));
}

caddr_t brk(x)
caddr_t x;
{
	return((caddr_t)-1);
}
