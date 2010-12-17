/*
 * Copyright 2008 Islam Ahmed Zaid. All rights reserved.  <azismed@gmail.com>
 * AsereBLN: 2009: cleanup and bugfix
 */

#ifndef __LIBSAIO_CPU_H
#define __LIBSAIO_CPU_H

#include "libsaio.h"

#define CPU_VIA_C3			0x07
#define CPU_VIA_C3_Ezra_T	0x08

#define CPU_VIA_NANO		0x0F



extern void scan_cpu(); //PlatformInfo_t *);

#define bit(n)			(1UL << (n))
#define bitmask(h,l)		((bit(h)|(bit(h)-1)) & ~(bit(l)-1))
#define bitfield(x,h,l)		(((x) & bitmask(h,l)) >> l)

#define CPU_STRING_UNKNOWN		"Unknown CPU Typ"

#define MSR_IA32_PLATFORM_ID	0x0017
#define MSR_P4_EBC_FREQUENCY_ID	0x002C
#define	MSR_IA32_PERF_STATUS	0x0198
#define MSR_IA32_PERF_CONTROL	0x0199
#define MSR_IA32_EXT_CONFIG		0x00EE
#define MSR_FLEX_RATIO			0x0194  //Slice - not used
#define MSR_FSB_FREQ			0x00CD  //Slice - not used
#define	MSR_PLATFORM_INFO		0x00CE	// for i7 it gives min and max
#define MSR_TURBO_RATIO			0x01AD  //Slice - for i7
#define MSR_IA32_EBL_CR_POWERON	0x002A  //Meklort - for VIA
#define K8_FIDVID_STATUS		0xC0010042
#define K10_COFVID_STATUS		0xC0010071

/* Centaur-Hauls/IDT defined MSRs. */
#define MSR_IDT_FCR1			0x00000107
#define MSR_IDT_FCR2			0x00000108
#define MSR_IDT_FCR3			0x00000109
#define MSR_IDT_FCR4			0x0000010a

#define MSR_IDT_MCR0			0x00000110
#define MSR_IDT_MCR1			0x00000111
#define MSR_IDT_MCR2			0x00000112
#define MSR_IDT_MCR3			0x00000113
#define MSR_IDT_MCR4			0x00000114
#define MSR_IDT_MCR5			0x00000115
#define MSR_IDT_MCR6			0x00000116
#define MSR_IDT_MCR7			0x00000117
#define MSR_IDT_MCR_CTRL		0x00000120

/* VIA Nano defined MSTs */
#define MSR_NANO_FCR1 0x1204        // This MSR contains control bits and family, model, etc.
#define MSR_NANO_FCR2 0x1206        // This MSR contains part of the vendor string
#define MSR_NANO_FCR3 0x1207        // This MSR contains part of the vendor string
#define VIA_ALTERNATIVE_VENDOR_BIT (1 << 8)  // This bit in MSR_IDT_FCR1 enables alternative vendor string

/* VIA Cyrix defined MSRs */
#define MSR_VIA_FCR				0x00001107
#define MSR_VIA_LONGHAUL		0x0000110a
#define MSR_VIA_RNG				0x0000110b
#define MSR_VIA_BCR2			0x00001147



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
					 " inb   $0x61,%%al      \n\t"
					 " and   $0xFC,%%al       \n\t"  /* & ~0x03 */
					 " or    $1,%%al         \n\t"
					 " outb  %%al,$0x61      \n\t"
					 : : : "%al" );
}

static inline void disable_PIT2(void)
{
    /* Disable gate and output to speaker */
    __asm__ volatile(
					 " inb   $0x61,%%al      \n\t"
					 " and   $0xFC,%%al      \n\t"	/* & ~0x03 */
					 " outb  %%al,$0x61      \n\t"
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
					 " movb  $0xB0,%%al      \n\t"
					 " outb	%%al,$0x43	\n\t"
					 " movb	%%dl,%%al	\n\t"
					 " outb	%%al,$0x42	\n\t"
					 " movb	%%dh,%%al	\n\t"
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
