
#include "libsaio.h"

/*
 * License for L15_Swap, L15_InitState, L_SCHEDULE, L15_KSA, L15_Discard, L15, L15_Byte
 *
 * Copyright (c) 2004, 2005, 2006 Robin J Carey. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification, immediately at the beginning of the file.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $DragonFly: src/sys/kern/kern_nrandom.c,v 1.7 2008/08/01 04:42:30 dillon Exp $
 */

/*
 * IMPORTANT NOTE: LByteType must be exactly 8-bits in size or this software
 * will not function correctly.
 */
typedef unsigned char   LByteType;
#define L15_STATE_SIZE  256
static LByteType        L15_x, L15_y;
static LByteType        L15_start_x;
static LByteType        L15_state[L15_STATE_SIZE];

/*
 * PRIVATE FUNCS:
 */
static void             L15(const LByteType * const key, const size_t keyLen);
static void             L15_Swap(const LByteType pos1, const LByteType pos2);
static void             L15_InitState(void);
static void             L15_KSA(const LByteType * const key,
                                const size_t keyLen);
static void             L15_Discard(const LByteType numCalls);
/*
 * PUBLIC INTERFACE:
 */
static LByteType        L15_Byte(void);
 
static __inline void
L15_Swap(const LByteType pos1, const LByteType pos2)
{
	const LByteType save1 = L15_state[pos1];
	
	L15_state[pos1] = L15_state[pos2];
	L15_state[pos2] = save1;        
}

static void
L15_InitState (void)
{
	size_t i;
	for (i = 0; i < L15_STATE_SIZE; ++i)
		L15_state[i] = i;        
}

#define  L_SCHEDULE(xx)                                         \
                                                                 \
for (i = 0; i < L15_STATE_SIZE; ++i) {                          \
    L15_Swap(i, (stateIndex += (L15_state[i] + (xx))));         \
}

static void
L15_KSA (const LByteType * const key, const size_t keyLen)
{
        size_t  i, keyIndex;
        LByteType stateIndex = 0;

        L_SCHEDULE(keyLen);
        for (keyIndex = 0; keyIndex < keyLen; ++keyIndex) {
                L_SCHEDULE(key[keyIndex]);
        }
}
 
static void
L15_Discard(const LByteType numCalls)
{
        LByteType i;
        for (i = 0; i < numCalls; ++i) {
                (void)L15_Byte();
        }
}
  
/*
 * PUBLIC INTERFACE:
 */
static void
L15(const LByteType * const key, const size_t keyLen)
{
        L15_x = L15_start_x = 0;
        L15_y = L15_STATE_SIZE - 1;
        L15_InitState();
        L15_KSA(key, keyLen);
        L15_Discard(L15_Byte());
}

static LByteType
L15_Byte(void)
{
	LByteType z;
	
	L15_Swap(L15_state[L15_x], L15_y);
	z = (L15_state [L15_x++] + L15_state[L15_y--]);
	if (L15_x == L15_start_x) {
		--L15_y;
	}
	return (L15_state[z]);        
}



/*-
 * THE BEER-WARE LICENSE
 *
 * <dan@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff.  If we meet some day, and you
 * think this stuff is worth it, you can buy me a beer in return.
 *
 * Dan Moschuk
 *
 * $FreeBSD: src/sys/libkern/arc4random.c,v 1.3.2.2 2001/09/17 07:06:50 silby Exp $
 */


#define	ARC4_MAXRUNS 16384
#define	ARC4_RESEED_SECONDS 300 * 18
#define	ARC4_KEYBYTES 32 /* 256 bit key */

static u_int8_t arc4_i, arc4_j;
static int arc4_initialized = 0;
static int arc4_numruns = 0;
static u_int8_t arc4_sbox[256];
static u_int32_t arc4_tv_nextreseed;

static u_int8_t arc4_randbyte(void);

static __inline void
arc4_swap(u_int8_t *a, u_int8_t *b)
{
	u_int8_t c;
	
	c = *a;
	*a = *b;
	*b = c;
}	

/*
 * Stir our S-box.
 */
static void
arc4_randomstir (void)
{
	u_int8_t key[256];
	int r, n;
	
	/*
	 * XXX read_random() returns unsafe numbers if the entropy
	 * device is not loaded -- MarkM.
	 */
	 
	{		
		for (r = 0; r < ARC4_KEYBYTES; ++r)
			((u_char *)key)[r] = L15_Byte();
	}

	/* If r == 0 || -1, just use what was on the stack. */
	if (r > 0)
	{
		for (n = r; n < sizeof(key); n++)
			key[n] = key[n % r];
	}
	
	for (n = 0; n < 256; n++)
	{
		arc4_j = (arc4_j + arc4_sbox[n] + key[n]) % 256;
		arc4_swap(&arc4_sbox[n], &arc4_sbox[arc4_j]);
	}
	arc4_i = arc4_j = 0;
	
	/* Reset for next reseed cycle. */
	arc4_tv_nextreseed = time18();
	arc4_tv_nextreseed += ARC4_RESEED_SECONDS;
	arc4_numruns = 0;
}

/*
 * Initialize our S-box to its beginning defaults.
 */
void
arc4_init(void)
{
	int n;
	
	u_int32_t now = time18();
	L15((const LByteType *)&now, sizeof(u_int32_t));
	
	arc4_i = arc4_j = 0;
	for (n = 0; n < 256; n++)
		arc4_sbox[n] = (u_int8_t) n;
	
	arc4_randomstir();
	arc4_initialized = 1;
	
	/*
	 * Throw away the first N words of output, as suggested in the
	 * paper "Weaknesses in the Key Scheduling Algorithm of RC4"
	 * by Fluher, Mantin, and Shamir.  (N = 256 in our case.)
	 */
	for (n = 0; n < 256*4; n++)
		arc4_randbyte();
}

/*
 * Generate a random byte.
 */
static u_int8_t
arc4_randbyte(void)
{
	u_int8_t arc4_t;
	
	arc4_i = (arc4_i + 1) % 256;
	arc4_j = (arc4_j + arc4_sbox[arc4_i]) % 256;
	
	arc4_swap(&arc4_sbox[arc4_i], &arc4_sbox[arc4_j]);
	
	arc4_t = (arc4_sbox[arc4_i] + arc4_sbox[arc4_j]) % 256;
	return arc4_sbox[arc4_t];
}

void
arc4rand(void *ptr, u_int len, int reseed)
{
	u_char *p;
	u_int32_t tv_now;
	
	/* Initialize array if needed. */
	if (!arc4_initialized)
		arc4_init();
	
	tv_now = time18();

	if (reseed || 
		(arc4_numruns > ARC4_MAXRUNS) ||
		(tv_now > arc4_tv_nextreseed))
		arc4_randomstir();
	
	arc4_numruns += len;
	p = ptr;
	while (len--)
		*p++ = arc4_randbyte();
}

uint32_t
arc4random(void)
{
	uint32_t ret;
	
	arc4rand(&ret, sizeof ret, 0);
	return ret;        
}
