/*
 * Copyright 2008 Islam Ahmed Zaid. All rights reserved.  <azismed@gmail.com>
 * AsereBLN: 2009: cleanup and bugfix
 */

#ifndef __LIBSAIO_CPU_H
#define __LIBSAIO_CPU_H

#include "platform.h"

extern void scan_cpu(PlatformInfo_t *);

struct clock_frequency_info_t
{
	unsigned long bus_clock_rate_hz;
	unsigned long cpu_clock_rate_hz;
	unsigned long dec_clock_rate_hz;
	unsigned long bus_clock_rate_num;
	unsigned long bus_clock_rate_den;
	unsigned long bus_to_cpu_rate_num;
	unsigned long bus_to_cpu_rate_den;
	unsigned long bus_to_dec_rate_num;
	unsigned long bus_to_dec_rate_den;
	unsigned long timebase_frequency_hz;
	unsigned long timebase_frequency_num;
	unsigned long timebase_frequency_den;
	unsigned long long bus_frequency_hz;
	unsigned long long bus_frequency_min_hz;
	unsigned long long bus_frequency_max_hz;
	unsigned long long cpu_frequency_hz;
	unsigned long long cpu_frequency_min_hz;
	unsigned long long cpu_frequency_max_hz;
	unsigned long long prf_frequency_hz;
	unsigned long long prf_frequency_min_hz;
	unsigned long long prf_frequency_max_hz;
	unsigned long long mem_frequency_hz;
	unsigned long long mem_frequency_min_hz;
	unsigned long long mem_frequency_max_hz;
	unsigned long long fix_frequency_hz;
};

typedef struct clock_frequency_info_t clock_frequency_info_t;

extern clock_frequency_info_t gPEClockFrequencyInfo;


struct mach_timebase_info
{
	uint32_t	numer;
	uint32_t	denom;
};

struct hslock
{
	int		lock_data;
};
typedef struct hslock hw_lock_data_t, *hw_lock_t;

#define hw_lock_addr(hwl)	(&((hwl).lock_data))

typedef struct uslock_debug
{
	void		*lock_pc;	/* pc where lock operation began    */
	void		*lock_thread;	/* thread that acquired lock */
	unsigned long	duration[2];
	unsigned short	state;
	unsigned char	lock_cpu;
	void		*unlock_thread;	/* last thread to release lock */
	unsigned char	unlock_cpu;
	void		*unlock_pc;	/* pc where lock operation ended    */
} uslock_debug;

typedef struct slock
{
	hw_lock_data_t	interlock;	/* must be first... see lock.c */
	unsigned short	lock_type;	/* must be second... see lock.c */
#define USLOCK_TAG	0x5353
	uslock_debug	debug;
} usimple_lock_data_t, *usimple_lock_t;

#if !defined(decl_simple_lock_data)
typedef usimple_lock_data_t	*simple_lock_t;
typedef usimple_lock_data_t	simple_lock_data_t;

#define	decl_simple_lock_data(class,name) \
class	simple_lock_data_t	name;
#endif	/* !defined(decl_simple_lock_data) */

typedef struct mach_timebase_info	*mach_timebase_info_t;
typedef struct mach_timebase_info	mach_timebase_info_data_t;

// DFE: These two constants come from Linux except CLOCK_TICK_RATE replaced with CLKNUM
#define CALIBRATE_TIME_MSEC	30		/* 30 msecs */
#define CALIBRATE_LATCH		((CLKNUM * CALIBRATE_TIME_MSEC + 1000/2)/1000)

#define MSR_AMD_INT_PENDING_CMP_HALT 0xC0010055
#define AMD_ACTONCMPHALT_SHIFT 27
#define AMD_ACTONCMPHALT_MASK 3

/*
 * Control register 0
 */

typedef struct _cr0 {
    unsigned int	pe	:1,
    			mp	:1,
			em	:1,
			ts	:1,
				:1,
			ne	:1,
				:10,
			wp	:1,
				:1,
			am	:1,
				:10,
			nw	:1,
			cd	:1,
			pg	:1;
} cr0_t;

/*
 * Debugging register 6
 */

typedef struct _dr6 {
    unsigned int	b0	:1,
    			b1	:1,
			b2	:1,
			b3	:1,
				:9,
			bd	:1,
			bs	:1,
			bt	:1,
				:16;
} dr6_t;

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

/*
 * Timing Functions
 */

/* REP NOP (PAUSE) is a good thing to insert into busy-wait loops. */
static inline void CpuPause(void)
{
	__asm__ volatile ("rep; nop");
}

static inline uint32_t DivU64x32(uint64_t dividend, uint32_t divisor)
{
	__asm__ volatile ("divl %1" : "+A"(dividend) : "r"(divisor));
	return (uint32_t) dividend;
}

static inline uint64_t MultU32x32(uint32_t multiplicand, uint32_t multiplier)
{
	uint64_t result;
	__asm__ volatile ("mull %2" : "=A"(result) : "a"(multiplicand), "r"(multiplier));
	return result;
}

#endif /* !__LIBSAIO_CPU_H */
