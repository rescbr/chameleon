/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include <mach-o/loader.h>
#include <mach-o/nlist.h>

#ifndef __BOOT2_KERNEL_PATCHER_H
#define __BOOT2_KERNEL_PATCHER_H

#define CPUID_MODEL_ANY		0x00
#define CPUID_MODEL_UNKNOWN	0x01

#define KERNEL_ANY	0x00
#define KERNEL_64	0x01
#define KERNEL_32	0x02
#define KERNEL_ERR	0xFF

typedef struct patchRoutine_t
{
	void(*patchRoutine)(void*);
	int validArchs;
	int validCpu;
	struct patchRoutine_t* next;
} patchRoutine_t;


typedef struct kernSymbols_t
{
	char* symbol;
	UInt64 addr;
	struct kernSymbols_t* next;
} kernSymbols_t;


#endif /* !__BOOT2_KERNEL_PATCHER_H */
