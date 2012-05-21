/*
 * Copyright (c) 2002 Hiroaki Etoh, Federico G. Schwindt, and Miodrag Vallat.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include "libsaio.h"

#define __arraycount(__x)       (sizeof(__x) / sizeof(__x[0]))

long __stack_chk_guard[8] = {0, 0, 0, 0, 0, 0, 0, 0};

//__private_extern__ void __guard_setup(void) /*__attribute__ ((visibility ("hidden")))*/;
void __guard_setup(void) __attribute__((constructor));

void __stack_chk_fail(void);


void
__guard_setup(void)
{
	if (__stack_chk_guard[0]!=0) return;
	
	size_t i;
	long guard[__arraycount(__stack_chk_guard)];	
	
	arc4random_buf(guard, sizeof(guard));
	for (i = 0; i < __arraycount(guard); i++)
		__stack_chk_guard[i] = guard[i];
	
	if (__stack_chk_guard[0]!=0 && *__stack_chk_guard != 0) return;
	
	/* If a random generator can't be used, the protector switches the guard
     to the "terminator canary" */
	((char*)__stack_chk_guard)[0] = 0; ((char*)__stack_chk_guard)[1] = 0;
	((char*)__stack_chk_guard)[2] = '\n'; ((char*)__stack_chk_guard)[3] = 255;	
	
}

void
__stack_chk_fail()
{
#ifndef BOOT1
	stop("stack overflow");
#endif
	for(;;);
}
