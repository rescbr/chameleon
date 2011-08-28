/*
 * Copyright 2008 Islam Ahmed Zaid. All rights reserved.  <azismed@gmail.com>
 * AsereBLN: 2009: cleanup and bugfix
 */

#ifndef __LIBSAIO_CPU_H
#define __LIBSAIO_CPU_H

#include "libsaio.h"

extern void scan_cpu(PlatformInfo_t *);

#define	MSR_IA32_PERF_STATUS	0x198
#define MSR_IA32_PERF_CONTROL	0x199
#define MSR_IA32_EXT_CONFIG		0x00EE
#define MSR_FLEX_RATIO			0x194
#define	MSR_PLATFORM_INFO		0xCE
#define MSR_TURBO_RATIO_LIMIT	0x1AD
#define MSR_IA32_BIOS_SIGN_ID   0x08B
#define MSR_CORE_THREAD_COUNT   0x035
#define K8_FIDVID_STATUS		0xC0010042
#define K10_COFVID_STATUS		0xC0010071

#define DEFAULT_FSB		100000          /* for now, hardcoding 100MHz for old CPUs */

// DFE: This constant comes from older xnu:
#define CLKNUM			1193182		/* formerly 1193167 */

// DFE: These two constants come from Linux except CLOCK_TICK_RATE replaced with CLKNUM
#define CALIBRATE_TIME_MSEC	30		/* 30 msecs */
#define CALIBRATE_LATCH		((CLKNUM * CALIBRATE_TIME_MSEC + 1000/2)/1000)

static inline uint64_t rdtsc64(void)
{
	uint64_t ret;
	__asm__ volatile("rdtsc" : "=A" (ret));
	return ret;
}

static inline uint64_t rdmsr64(uint32_t msr)
{
    uint64_t ret;
    __asm__ volatile("rdmsr" : "=A" (ret) : "c" (msr));
    return ret;
}

static inline void wrmsr64(uint32_t msr, uint64_t val)
{
	__asm__ volatile("wrmsr" : : "c" (msr), "A" (val));
}

static inline void intel_waitforsts(void) {
	uint32_t inline_timeout = 100000;
	while (rdmsr64(MSR_IA32_PERF_STATUS) & (1 << 21)) { if (!inline_timeout--) break; }
}

static inline void do_cpuid2(uint32_t selector, uint32_t selector2, uint32_t *data)
{
	asm volatile ("cpuid"
				  : "=a" (data[0]),
				  "=b" (data[1]),
				  "=c" (data[2]),
				  "=d" (data[3])
				  : "a" (selector), "c" (selector2),
				  "b" (0),
				  "d" (0));
}

#endif /* !__LIBSAIO_CPU_H */
