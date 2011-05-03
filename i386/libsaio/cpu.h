/*
 * Copyright 2008 Islam Ahmed Zaid. All rights reserved.  <azismed@gmail.com>
 * AsereBLN: 2009: cleanup and bugfix
 */

#ifndef __LIBSAIO_CPU_H
#define __LIBSAIO_CPU_H

#include "libsaio.h"

extern void scan_cpu(PlatformInfo_t *);

#define bit(n)				(1UL << (n))
#define bitmask(h,l)		((bit(h)|(bit(h)-1)) & ~(bit(l)-1))
#define bitfield(x,h,l)		(((x) & bitmask(h,l)) >> l)

#define CPU_STRING_UNKNOWN				"Unknown CPU Typ"

#define MSR_CORE_THREAD_COUNT			0x35			// Undocumented
#define MSR_EBC_FREQUENCY_ID			0x2C			// valv
#define MSR_EBL_CR_POWERON				0x2A			// valv
#define MSR_FLEX_RATIO					0x194
#define MSR_FSB_FREQ					0xCD			// valv
#define MSR_IA32_CLOCK_MODULATION		0x19A			// valv
#define MSR_IA32_EXT_CONFIG				0xEE
#define MSR_IA32_MISC_ENABLE			0x1A0			// valv
#define MSR_IA32_PERF_CONTROL			0x199
#define	MSR_IA32_PERF_STATUS			0x198
#define MSR_IA32_PLATFORM_ID			0x17			// valv
#define	MSR_PLATFORM_INFO				0xCE
#define MSR_THERMAL_STATUS				0x19C			// valv
#define MSR_THERMAL_TARGET				0x1A2			// valv
#define MSR_TURBO_RATIO_LIMIT			0x1AD			// valv
#define MSR_MISC_PWR_MGMT				0x1AA			// valv

#define K8_FIDVID_STATUS				0xC0010042
#define K10_COFVID_STATUS				0xC0010071
#define	MSR_AMD_10H_11H_LIMIT			0xc0010061		// valv
#define	AMD_10H_11H_CONFIG				0xc0010064		// valv

#define DEFAULT_FSB						100000		/* for now, hardcoding 100MHz for old CPUs */

// DFE: This constant comes from older xnu:
#define CLKNUM							1193182		/* formerly 1193167 */

// DFE: These two constants come from Linux except CLOCK_TICK_RATE replaced with CLKNUM
#define CALIBRATE_TIME_MSEC				30		/* 30 msecs */
#define CALIBRATE_LATCH					((CLKNUM * CALIBRATE_TIME_MSEC + 1000/2)/1000)

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

typedef struct msr_struct
{
	unsigned lo;
	unsigned hi;
} msr_t;

static inline __attribute__((always_inline)) msr_t rdmsr(unsigned val)
{
	msr_t ret;
	__asm__ volatile(
					 "rdmsr"
					 : "=a" (ret.lo), "=d" (ret.hi)
					 : "c" (val)
					 );
	return ret;
}

static inline __attribute__((always_inline)) void wrmsr(unsigned val, msr_t msr)
{
	__asm__ __volatile__ (
						  "wrmsr"
						  : /* No outputs */
						  : "c" (val), "a" (msr.lo), "d" (msr.hi)
						  );
}

static inline void intel_waitforsts(void) {
	uint32_t inline_timeout = 100000;
	while (rdmsr64(MSR_IA32_PERF_STATUS) & (1 << 21)) { if (!inline_timeout--) break; }
}

static inline void do_cpuid(uint32_t selector, uint32_t *data)
{
	asm volatile ("cpuid"
				  : "=a" (data[0]),
				  "=b" (data[1]),
				  "=c" (data[2]),
				  "=d" (data[3])
				  : "a" (selector));
}

static inline void do_cpuid2(uint32_t selector, uint32_t selector2, uint32_t *data)
{
	asm volatile ("cpuid"
				  : "=a" (data[0]),
				  "=b" (data[1]),
				  "=c" (data[2]),
				  "=d" (data[3])
				  : "a" (selector), "c" (selector2));
}

// DFE: enable_PIT2 and disable_PIT2 come from older xnu

/*
 * Enable or disable timer 2.
 * Port 0x61 controls timer 2:
 *   bit 0 gates the clock,
 *   bit 1 gates output to speaker.
 */
static inline void enable_PIT2(void)
{
    /* Enable gate, disable speaker */
    __asm__ volatile(
					 " inb   $0x61,%%al \n\t"
					 " and   $0xFC,%%al \n\t"  /* & ~0x03 */
					 " or    $1,%%al    \n\t"
					 " outb  %%al,$0x61 \n\t"
					 : : : "%al" );
}

static inline void disable_PIT2(void)
{
    /* Disable gate and output to speaker */
    __asm__ volatile(
					 " inb   $0x61,%%al  \n\t"
					 " and   $0xFC,%%al  \n\t"	/* & ~0x03 */
					 " outb  %%al,$0x61  \n\t"
					 : : : "%al" );
}

// DFE: set_PIT2_mode0, poll_PIT2_gate, and measure_tsc_frequency are
// roughly based on Linux code

/* Set the 8254 channel 2 to mode 0 with the specified value.
 In mode 0, the counter will initially set its gate low when the
 timer expires.  For this to be useful, you ought to set it high
 before calling this function.  The enable_PIT2 function does this.
 */
static inline void set_PIT2_mode0(uint16_t value)
{
    __asm__ volatile(
					 " movb  $0xB0,%%al   \n\t"
					 " outb	%%al,$0x43    \n\t"
					 " movb	%%dl,%%al     \n\t"
					 " outb	%%al,$0x42    \n\t"
					 " movb	%%dh,%%al     \n\t"
					 " outb	%%al,$0x42"
					 : : "d"(value) /*: no clobber */ );
}

/* Returns the number of times the loop ran before the PIT2 signaled */
static inline unsigned long poll_PIT2_gate(void)
{
    unsigned long count = 0;
    unsigned char nmi_sc_val;
    do {
        ++count;
        __asm__ volatile(
						 "inb	$0x61,%0"
						 : "=q"(nmi_sc_val) /*:*/ /* no input */ /*:*/ /* no clobber */);
    } while( (nmi_sc_val & 0x20) == 0);
    return count;
}

#endif /* !__LIBSAIO_CPU_H */
