/*
 * Copyright 2008 Islam Ahmed Zaid. All rights reserved.  <azismed@gmail.com>
 * AsereBLN: 2009: cleanup and bugfix
 */

#ifndef __LIBSAIO_CPU_H
#define __LIBSAIO_CPU_H

#include "platform.h"

extern void scan_cpu(PlatformInfo_t *);

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

/* From Apple's cpuid.h */
typedef enum { eax, ebx, ecx, edx } cpuid_register_t;

static inline void cpuid(uint32_t *data)
{
	asm(
		"cpuid" : "=a" (data[eax]),
		"=b" (data[ebx]),
		"=c" (data[ecx]),
		"=d" (data[edx]) : "a"  (data[eax]),
		"b"  (data[ebx]),
		"c"  (data[ecx]),
		"d"  (data[edx]));
}

static inline void do_cpuid(uint32_t selector, uint32_t *data)
{
	asm(
		"cpuid" : "=a" (data[eax]),
		"=b" (data[ebx]),
		"=c" (data[ecx]),
		"=d" (data[edx]) : "a"(selector),
		"b" (0),
		"c" (0),
		"d" (0));
}

static inline void do_cpuid2(uint32_t selector, uint32_t selector2, uint32_t *data)
{
	asm volatile (
		"cpuid" : "=a" (data[eax]),
		"=b" (data[ebx]),
		"=c" (data[ecx]),
		"=d" (data[edx]) : "a" (selector),
		"b" (0),
		"c" (selector2),
		"d" (0));
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
						 : "=a"(nmi_sc_val) /*:*/ /* no input */ /*:*/ /* no clobber */);
    } while( (nmi_sc_val & 0x20) == 0);
    return count;
}

inline static void
set_PIT2(int value)
{
/*
 * First, tell the clock we are going to write 16 bits to the counter
 * and enable one-shot mode (command 0xB8 to port 0x43)
 * Then write the two bytes into the PIT2 clock register (port 0x42).
 * Loop until the value is "realized" in the clock,
 * this happens on the next tick.
 */
    asm volatile(
        " movb  $0xB8,%%al      \n\t"
        " outb  %%al,$0x43      \n\t"
        " movb  %%dl,%%al       \n\t"
        " outb  %%al,$0x42      \n\t"
        " movb  %%dh,%%al       \n\t"
        " outb  %%al,$0x42      \n"
"1:       inb   $0x42,%%al      \n\t" 
        " inb   $0x42,%%al      \n\t"
        " cmp   %%al,%%dh       \n\t"
        " jne   1b"
        : : "d"(value) : "%al");
}


inline static uint64_t
get_PIT2(unsigned int *value)
{
    register uint64_t   result;
/*
 * This routine first latches the time (command 0x80 to port 0x43),
 * then gets the time stamp so we know how long the read will take later.
 * Read (from port 0x42) and return the current value of the timer.
 */
#ifdef __i386__
    asm volatile(
        " xorl  %%ecx,%%ecx     \n\t"
        " movb  $0x80,%%al      \n\t"
        " outb  %%al,$0x43      \n\t"
        " rdtsc                 \n\t"
        " pushl %%eax           \n\t"
        " inb   $0x42,%%al      \n\t"
        " movb  %%al,%%cl       \n\t"
        " inb   $0x42,%%al      \n\t"
        " movb  %%al,%%ch       \n\t"
        " popl  %%eax   "
        : "=A"(result), "=c"(*value));
#else /* __x86_64__ */
    asm volatile(
		" xorq  %%rcx,%%rcx     \n\t"
		" movb  $0x80,%%al      \n\t"
		" outb  %%al,$0x43      \n\t"
		" rdtsc                 \n\t"
		" pushq  %%rax          \n\t"
		" inb   $0x42,%%al      \n\t"
		" movb  %%al,%%cl       \n\t"
		" inb   $0x42,%%al      \n\t"
		" movb  %%al,%%ch       \n\t"
		" popq  %%rax   "
		: "=A"(result), "=c"(*value));
#endif

    return result;
}

#endif /* !__LIBSAIO_CPU_H */
