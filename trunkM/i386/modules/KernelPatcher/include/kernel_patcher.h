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

#define CPU_MODEL_PENTIUM_M		0x0D
#define CPU_MODEL_YONAH			0x0E			// Sossaman, Yonah
#define CPU_MODEL_MEROM			0x0F			// Allendale, Conroe, Kentsfield, Woodcrest, Clovertown, Tigerton, Merom
#define CPU_MODEL_PENRYN		0x17			// Wolfdale, Yorkfield, Harpertown, Penryn
#define CPU_MODEL_NEHALEM		0x1A			// Bloomfield. Nehalem-EP, Nehalem-WS, Gainestown
#define CPU_MODEL_ATOM			0x1C			// Atom
#define CPU_MODEL_FIELDS		0x1E			// Lynnfield, Clarksfield, Jasper Forest
#define CPU_MODEL_DALES			0x1F			// Havendale, Auburndale
#define CPU_MODEL_DALES_32NM	0x25			// Clarkdale, Arrandale
#define CPU_MODEL_SANDY			0x2A			// Sandy Bridge
#define CPU_MODEL_WESTMERE		0x2C			// Gulftown, Westmere-EP, Westmere-WS
#define CPU_MODEL_SANDY_XEON	0x2D			// Sandy Bridge Xeon
#define CPU_MODEL_NEHALEM_EX	0x2E			// Beckton
#define CPU_MODEL_WESTMERE_EX	0x2F

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

kernSymbols_t* lookup_kernel_symbol(const char* name);
void register_kernel_symbol(int kernelType, const char* name);

long long symbol_handler(char* symbolName, long long addr, char is64);
void patch_kernel(void* kernelData, void* arg2, void* arg3, void *arg4);
void register_kernel_patch(void* patch, int arch, int cpus);

int locate_symbols(void* kernelData);

int determineKernelArchitecture(void* kernelData);

/*
 * Internal patches provided by this module.
 */
void patch_cpuid_set_info_all(void* kernelData);
void patch_cpuid_set_info_32(void* kernelData, UInt32 impersonateFamily, UInt8 impersonateModel);
void patch_cpuid_set_info_64(void* kernelData, UInt32 impersonateFamily, UInt8 impersonateModel);

void patch_pmCPUExitHaltToOff(void* kernelData);
void patch_lapic_init(void* kernelData);
void patch_commpage_stuff_routine(void* kernelData);
void patch_lapic_configure(void* kernelData);
void patch_lapic_interrupt(void* kernelData);

#endif /* !__BOOT2_KERNEL_PATCHER_H */
