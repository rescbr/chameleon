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

#define CPUID_MODEL_YONAH	14
#define CPUID_MODEL_MEROM	15
#define CPUID_MODEL_PENRYN	23
#define CPUID_MODEL_NEHALEM	26
#define CPUID_MODEL_ATOM	28
#define CPUID_MODEL_FIELDS	30	/* Lynnfield, Clarksfield, Jasper */
#define CPUID_MODEL_DALES	31	/* Havendale, Auburndale */
#define CPUID_MODEL_NEHALEM_EX	46

#define KERNEL_ANY	0x00
#define KERNEL_64	0x01
#define KERNEL_32	0x02


typedef struct patchRoutine_t
{
	void* patchRoutine;
	int validArchs;
	int validCpu;
	struct patchRoutine_t* next;
} patchRoutine_t;


typedef struct kernSymbols_t
{
	char* symbol;
	void* symbolAddress;
	struct kernSymbols_t* next;
} kernSymbols_t;

void* lookup_kernel_symbol(const char* name);

void patch_kernel(void* kernelData);
void register_kernel_patch(void* patch, int arch, int cpus);

int locate_symbols(void* kernelData);

void patch_kernel_32(void* kernelData);



void patch_cpuid_set_info(void* kernelData, UInt32 impersonateFamily, UInt8 inpersonateModel);
void patch_pmCPUExitHaltToOff(void* kernelData);
void patch_lapic_init(void* kernelData);
void patch_commpage_stuff_routine(void* kernelData);
#endif /* !__BOOT2_KERNEL_PATCHER_H */
