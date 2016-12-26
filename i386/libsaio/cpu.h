/*
 * Copyright (c) 2000-2006 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*
 * @OSF_COPYRIGHT@
 */
/*
 * Copyright 2008 Islam Ahmed Zaid. All rights reserved.  <azismed@gmail.com>
 * AsereBLN: 2009: cleanup and bugfix
 */

#ifndef __LIBSAIO_CPU_H
#define __LIBSAIO_CPU_H

#include "platform.h"

extern void scan_cpu(PlatformInfo_t *);

#define bit(n)			(1ULL << (n))
#define bitmask(h,l)		((bit(h) | (bit(h)-1)) & ~(bit(l)-1))
#define bitfield(x,h,l)		(((x) & bitmask(h,l)) >> l)
#define hbit(n)			(1ULL << ((n)+32))
#define min(a,b)		((a) < (b) ? (a) : (b))
#define quad32(hi,lo)		((((uint32_t)(hi)) << 16) | (((uint32_t)(lo)) & 0xFFFF))
#define quad64(hi,lo)		((((uint64_t)(hi)) << 32) | (((uint64_t)(lo)) & 0xFFFFFFFFUL))

#define	CPUID_VID_INTEL         "GenuineIntel"
#define	CPUID_VID_AMD           "AuthenticAMD"
#define CPU_STRING_UNKNOWN		"Unknown CPU Typ"

/*
 * The CPUID_FEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 1: 
 */
#define CPUID_FEATURE_FPU       bit(0)   /* Floating point unit on-chip */
#define CPUID_FEATURE_VME       bit(1)   /* Virtual Mode Extension */
#define CPUID_FEATURE_DE        bit(2)   /* Debugging Extension */
#define CPUID_FEATURE_PSE       bit(3)   /* Page Size Extension */
#define CPUID_FEATURE_TSC       bit(4)   /* Time Stamp Counter */
#define CPUID_FEATURE_MSR       bit(5)   /* Model Specific Registers */
#define CPUID_FEATURE_PAE       bit(6)   /* Physical Address Extension */
#define CPUID_FEATURE_MCE       bit(7)   /* Machine Check Exception */
#define CPUID_FEATURE_CX8       bit(8)   /* CMPXCHG8B */
#define CPUID_FEATURE_APIC      bit(9)   /* On-chip APIC */
#define CPUID_FEATURE_SEP       bit(11)  /* Fast System Call */
#define CPUID_FEATURE_MTRR      bit(12)  /* Memory Type Range Register */
#define CPUID_FEATURE_PGE       bit(13)  /* Page Global Enable */
#define CPUID_FEATURE_MCA       bit(14)  /* Machine Check Architecture */
#define CPUID_FEATURE_CMOV      bit(15)  /* Conditional Move Instruction */
#define CPUID_FEATURE_PAT       bit(16)  /* Page Attribute Table */
#define CPUID_FEATURE_PSE36     bit(17)  /* 36-bit Page Size Extension */
#define CPUID_FEATURE_PSN       bit(18)  /* Processor Serial Number */
#define CPUID_FEATURE_CLFSH     bit(19)  /* CLFLUSH Instruction supported */
#define CPUID_FEATURE_DS        bit(21)  /* Debug Store */
#define CPUID_FEATURE_ACPI      bit(22)  /* Thermal monitor and Clock Ctrl */
#define CPUID_FEATURE_MMX       bit(23)  /* MMX supported */
#define CPUID_FEATURE_FXSR      bit(24)  /* Fast floating pt save/restore */
#define CPUID_FEATURE_SSE       bit(25)  /* Streaming SIMD extensions */
#define CPUID_FEATURE_SSE2      bit(26)  /* Streaming SIMD extensions 2 */
#define CPUID_FEATURE_SS        bit(27)  /* Self-Snoop */
#define CPUID_FEATURE_HTT       bit(28)  /* Hyper-Threading Technology */
#define CPUID_FEATURE_TM        bit(29)  /* Thermal Monitor (TM1) */
#define CPUID_FEATURE_PBE       bit(31)  /* Pend Break Enable */
 
#define CPUID_FEATURE_SSE3      hbit(0)  /* Streaming SIMD extensions 3 */
#define CPUID_FEATURE_PCLMULQDQ hbit(1)  /* PCLMULQDQ instruction */
#define CPUID_FEATURE_DTES64    hbit(2)  /* 64-bit DS layout */
#define CPUID_FEATURE_MONITOR   hbit(3)  /* Monitor/mwait */
#define CPUID_FEATURE_DSCPL     hbit(4)  /* Debug Store CPL */
#define CPUID_FEATURE_VMX       hbit(5)  /* VMX */
#define CPUID_FEATURE_SMX       hbit(6)  /* SMX */
#define CPUID_FEATURE_EST       hbit(7)  /* Enhanced SpeedsTep (GV3) */
#define CPUID_FEATURE_TM2       hbit(8)  /* Thermal Monitor 2 */
#define CPUID_FEATURE_SSSE3     hbit(9)  /* Supplemental SSE3 instructions */
#define CPUID_FEATURE_CID       hbit(10) /* L1 Context ID */
#define CPUID_FEATURE_SEGLIM64  hbit(11) /* 64-bit segment limit checking */
#define CPUID_FEATURE_FMA       hbit(12) /* Fused-Multiply-Add support */
#define CPUID_FEATURE_CX16      hbit(13) /* CmpXchg16b instruction */
#define CPUID_FEATURE_xTPR      hbit(14) /* Send Task PRiority msgs */
#define CPUID_FEATURE_PDCM      hbit(15) /* Perf/Debug Capability MSR */

#define CPUID_FEATURE_PCID      hbit(17) /* ASID-PCID support */
#define CPUID_FEATURE_DCA       hbit(18) /* Direct Cache Access */
#define CPUID_FEATURE_SSE4_1    hbit(19) /* Streaming SIMD extensions 4.1 */
#define CPUID_FEATURE_SSE4_2    hbit(20) /* Streaming SIMD extensions 4.2 */
#define CPUID_FEATURE_x2APIC    hbit(21) /* Extended APIC Mode */
#define CPUID_FEATURE_MOVBE     hbit(22) /* MOVBE instruction */
#define CPUID_FEATURE_POPCNT    hbit(23) /* POPCNT instruction */
#define CPUID_FEATURE_TSCTMR    hbit(24) /* TSC deadline timer */
#define CPUID_FEATURE_AES       hbit(25) /* AES instructions */
#define CPUID_FEATURE_XSAVE     hbit(26) /* XSAVE instructions */
#define CPUID_FEATURE_OSXSAVE   hbit(27) /* XGETBV/XSETBV instructions */
#define CPUID_FEATURE_AVX1_0	hbit(28) /* AVX 1.0 instructions */
#define CPUID_FEATURE_F16C      hbit(29) /* Float16 convert instructions */
#define CPUID_FEATURE_RDRAND	hbit(30) /* RDRAND instruction */
#define CPUID_FEATURE_VMM       hbit(31) /* VMM (Hypervisor) present */

/*
 * Leaf 7, subleaf 0 additional features.
 * Bits returned in %ebx to a CPUID request with {%eax,%ecx} of (0x7,0x0}:
 */
#define CPUID_LEAF7_FEATURE_RDWRFSGS bit(0)	/* FS/GS base read/write */
#define CPUID_LEAF7_FEATURE_TSCOFF   bit(1)	/* TSC thread offset */
#define CPUID_LEAF7_FEATURE_BMI1     bit(3)	/* Bit Manipulation Instrs, set 1 */
#define CPUID_LEAF7_FEATURE_HLE      bit(4)	/* Hardware Lock Elision*/
#define CPUID_LEAF7_FEATURE_AVX2     bit(5)	/* AVX2 Instructions */
#define CPUID_LEAF7_FEATURE_SMEP     bit(7)	/* Supervisor Mode Execute Protect */
#define CPUID_LEAF7_FEATURE_BMI2     bit(8)	/* Bit Manipulation Instrs, set 2 */
#define CPUID_LEAF7_FEATURE_ENFSTRG  bit(9)	/* ENhanced Fast STRinG copy */
#define CPUID_LEAF7_FEATURE_INVPCID  bit(10)	/* INVPCID intruction, TDB */
#define CPUID_LEAF7_FEATURE_RTM      bit(11)	/* TBD */

/*
 * The CPUID_EXTFEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 0x80000001: 
 */
#define CPUID_EXTFEATURE_SYSCALL   bit(11)	/* SYSCALL/sysret */
#define CPUID_EXTFEATURE_XD        bit(20)	/* eXecute Disable */

#define CPUID_EXTFEATURE_1GBPAGE   bit(26)	/* 1GB pages */
#define CPUID_EXTFEATURE_RDTSCP	   bit(27)	/* RDTSCP */
#define CPUID_EXTFEATURE_EM64T	   bit(29)	/* Extended Mem 64 Technology */

#define CPUID_EXTFEATURE_LAHF	   hbit(0)	/* LAFH/SAHF instructions */

/*
 * The CPUID_EXTFEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 0x80000007: 
 */
#define CPUID_EXTFEATURE_TSCI      bit(8)	/* TSC Invariant */

#define	CPUID_CACHE_SIZE	16	/* Number of descriptor values */

#define CPUID_MWAIT_EXTENSION	bit(0)	/* enumeration of WMAIT extensions */
#define CPUID_MWAIT_BREAK       bit(1)	/* interrupts are break events	   */

//-- processor type -> p_type:
#define PT_OEM	0x00	// Intel Original OEM Processor;
#define PT_OD	0x01 	// Intel Over Drive Processor;
#define PT_DUAL	0x02	// Intel Dual Processor;
#define PT_RES	0x03	// Intel Reserved;

/* Known MSR registers */
#define MSR_IA32_PLATFORM_ID        0x0017
#define MSR_CORE_THREAD_COUNT       0x0035	/* limited use - not for Penryn or older */
#define IA32_TSC_ADJUST             0x003B
#define MSR_IA32_BIOS_SIGN_ID       0x008B	/* microcode version */
#define MSR_FSB_FREQ                0x00CD	/* limited use - not for i7 */
#define	MSR_PLATFORM_INFO           0x00CE	/* limited use - MinRatio for i7 but Max for Yonah	*/
/* turbo for penryn */
#define MSR_PKG_CST_CONFIG_CONTROL  0x00E2	/* sandy and ivy */
#define MSR_PMG_IO_CAPTURE_BASE     0x00E4
#define IA32_MPERF                  0x00E7	/* TSC in C0 only */
#define IA32_APERF                  0x00E8	/* actual clocks in C0 */
#define MSR_IA32_EXT_CONFIG         0x00EE	/* limited use - not for i7 */
#define MSR_FLEX_RATIO              0x0194	/* limited use - not for Penryn or older */
						//see no value on most CPUs
#define	MSR_IA32_PERF_STATUS        0x0198
#define MSR_IA32_PERF_CONTROL       0x0199
#define MSR_IA32_CLOCK_MODULATION   0x019A
#define MSR_THERMAL_STATUS          0x019C
#define MSR_IA32_MISC_ENABLE        0x01A0
#define MSR_THERMAL_TARGET          0x01A2	 /* TjMax limited use - not for Penryn or older	*/
#define MSR_MISC_PWR_MGMT           0x01AA
#define MSR_TURBO_RATIO_LIMIT       0x01AD	 /* limited use - not for Penryn or older */

#define IA32_ENERGY_PERF_BIAS		0x01B0
#define MSR_PACKAGE_THERM_STATUS	0x01B1
#define IA32_PLATFORM_DCA_CAP		0x01F8
#define MSR_POWER_CTL			0x01FC   // MSR 000001FC  0000-0000-0004-005F

// Sandy Bridge & JakeTown specific 'Running Average Power Limit' MSR's.
#define MSR_RAPL_POWER_UNIT			0x606     /* R/O */
//MSR 00000606                                      0000-0000-000A-1003
#define MSR_PKGC3_IRTL              0x60A    /* RW time limit to go C3 */
// bit 15 = 1 -- the value valid for C-state PM
#define MSR_PKGC6_IRTL              0x60B    /* RW time limit to go C6 */
//MSR 0000060B                                      0000-0000-0000-8854
//Valid + 010=1024ns + 0x54=84mks
#define MSR_PKGC7_IRTL              0x60C    /* RW time limit to go C7 */
//MSR 0000060C                                      0000-0000-0000-8854
#define MSR_PKG_C2_RESIDENCY        0x60D   /* same as TSC but in C2 only */

#define MSR_PKG_RAPL_POWER_LIMIT	0x610 //MSR 00000610  0000-A580-0000-8960
#define MSR_PKG_ENERGY_STATUS		0x611 //MSR 00000611  0000-0000-3212-A857
#define MSR_PKG_POWER_INFO			0x614 //MSR 00000614  0000-0000-01E0-02F8

//AMD
#define K8_FIDVID_STATUS        0xC0010042
#define K10_COFVID_LIMIT        0xC0010061
#define K10_PSTATE_STATUS       0xC0010064
#define K10_COFVID_STATUS       0xC0010071

#define MSR_AMD_MPERF           0x000000E7
#define MSR_AMD_APERF           0x000000E8

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
/*
static inline void do_cpuid(uint32_t selector, uint32_t *data)
{
	asm volatile ("cpuid"
				  : "=a" (data[0]),
				    "=b" (data[1]),
				    "=c" (data[2]),
				    "=d" (data[3])
				  : "a" (selector));
}
*/
/* From Apple's cpuid.h */
typedef enum { eax, ebx, ecx, edx } cpuid_register_t;

static inline void cpuid(uint32_t *data)
{
	asm("cpuid"
		: "=a" (data[eax]),
          "=b" (data[ebx]),
          "=c" (data[ecx]),
          "=d" (data[edx])
		: "a"  (data[eax]),
          "b"  (data[ebx]),
          "c"  (data[ecx]),
          "d"  (data[edx]));
}

static inline void do_cpuid(uint32_t selector, uint32_t *data)
{
	asm("cpuid"
		: "=a" (data[eax]),
          "=b" (data[ebx]),
          "=c" (data[ecx]),
          "=d" (data[edx])
		: "a"(selector),
          "b" (0),
          "c" (0),
          "d" (0));
}
/***/
static inline void do_cpuid2(uint32_t selector, uint32_t selector2, uint32_t *data)
{
	asm volatile ("cpuid"
				  : "=a" (data[eax]),
                    "=b" (data[ebx]),
                    "=c" (data[ecx]),
                    "=d" (data[edx])
				  : "a" (selector),
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
