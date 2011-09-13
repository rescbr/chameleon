/*
 * Copyright 2008 Islam Ahmed Zaid. All rights reserved.  <azismed@gmail.com>
 * AsereBLN: 2009: cleanup and bugfix
 */

#include "libsaio.h"
#include "platform.h"
#include "cpu.h"

#ifndef DEBUG_CPU
#define DEBUG_CPU 0
#endif

#if DEBUG_CPU
#define DBG(x...)		printf(x)
#else
#define DBG(x...)		msglog(x)
#endif

//#define AMD_SUPPORT 

#ifndef INTEL_SUPPORT
#define INTEL_SUPPORT 0 //Default (0: nolegacy, 1 : legacy)
#endif

#ifdef AMD_SUPPORT
#ifdef LEGACY_CPU
#undef LEGACY_CPU
#endif
#ifdef INTEL_SUPPORT
#undef INTEL_SUPPORT
#endif
#define LEGACY_CPU 1
#endif

#ifdef INTEL_SUPPORT 
#ifdef LEGACY_CPU
#undef LEGACY_CPU
#endif
#define LEGACY_CPU INTEL_SUPPORT
#endif
// (?) : if AMD_SUPPORT then LEGACY_CPU = 1, INTEL_SUPPORT = disabled
//		   else LEGACY_CPU = INTEL_SUPPORT


#if LEGACY_CPU
static uint64_t measure_tsc_frequency(void);

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
/*
 * DFE: Measures the TSC frequency in Hz (64-bit) using the ACPI PM timer
 */
static uint64_t measure_tsc_frequency(void)
{
    uint64_t tscStart;
    uint64_t tscEnd;
    uint64_t tscDelta = 0xffffffffffffffffULL;
    unsigned long pollCount;
    uint64_t retval = 0;
    int i;
	
    /* Time how many TSC ticks elapse in 30 msec using the 8254 PIT
     * counter 2.  We run this loop 3 times to make sure the cache
     * is hot and we take the minimum delta from all of the runs.
     * That is to say that we're biased towards measuring the minimum
     * number of TSC ticks that occur while waiting for the timer to
     * expire.  That theoretically helps avoid inconsistencies when
     * running under a VM if the TSC is not virtualized and the host
     * steals time.  The TSC is normally virtualized for VMware.
     */
    for(i = 0; i < 10; ++i)
    {
        enable_PIT2();
        set_PIT2_mode0(CALIBRATE_LATCH);
        tscStart = rdtsc64();
        pollCount = poll_PIT2_gate();
        tscEnd = rdtsc64();
        /* The poll loop must have run at least a few times for accuracy */
        if(pollCount <= 1)
            continue;
        /* The TSC must increment at LEAST once every millisecond.  We
         * should have waited exactly 30 msec so the TSC delta should
         * be >= 30.  Anything less and the processor is way too slow.
         */
        if((tscEnd - tscStart) <= CALIBRATE_TIME_MSEC)
            continue;
        // tscDelta = min(tscDelta, (tscEnd - tscStart))
        if( (tscEnd - tscStart) < tscDelta )
            tscDelta = tscEnd - tscStart;
    }
    /* tscDelta is now the least number of TSC ticks the processor made in
     * a timespan of 0.03 s (e.g. 30 milliseconds)
     * Linux thus divides by 30 which gives the answer in kiloHertz because
     * 1 / ms = kHz.  But we're xnu and most of the rest of the code uses
     * Hz so we need to convert our milliseconds to seconds.  Since we're
     * dividing by the milliseconds, we simply multiply by 1000.
     */
	
    /* Unlike linux, we're not limited to 32-bit, but we do need to take care
     * that we're going to multiply by 1000 first so we do need at least some
     * arithmetic headroom.  For now, 32-bit should be enough.
     * Also unlike Linux, our compiler can do 64-bit integer arithmetic.
     */
    if(tscDelta > (1ULL<<32))
        retval = 0;
    else
    {
        retval = tscDelta * 1000 / 30;
    }
    disable_PIT2();
    return retval;
}

#ifdef AMD_SUPPORT
#define MSR_AMD_APERF           0x000000E8
/*
 * Original comment/code:
 *  "DFE: Measures the Max Performance Frequency in Hz (64-bit)"
 *
 * Measures the Actual Performance Frequency in Hz (64-bit)
 *  (just a naming change, mperf --> aperf )
 */
static uint64_t measure_aperf_frequency(void)
{
	uint64_t aperfStart;
	uint64_t aperfEnd;
	uint64_t aperfDelta = 0xffffffffffffffffULL;
	unsigned long pollCount;
	uint64_t retval = 0;
	int i;
	
	/* Time how many APERF ticks elapse in 30 msec using the 8254 PIT
	 * counter 2. We run this loop 3 times to make sure the cache
	 * is hot and we take the minimum delta from all of the runs.
	 * That is to say that we're biased towards measuring the minimum
	 * number of APERF ticks that occur while waiting for the timer to
	 * expire.
	 */
	for(i = 0; i < 10; ++i)
	{
		enable_PIT2();
		set_PIT2_mode0(CALIBRATE_LATCH);
		aperfStart = rdmsr64(MSR_AMD_APERF);
		pollCount = poll_PIT2_gate();
		aperfEnd = rdmsr64(MSR_AMD_APERF);
		/* The poll loop must have run at least a few times for accuracy */
		if (pollCount <= 1)
			continue;
		/* The TSC must increment at LEAST once every millisecond.
		 * We should have waited exactly 30 msec so the APERF delta should
		 * be >= 30. Anything less and the processor is way too slow.
		 */
		if ((aperfEnd - aperfStart) <= CALIBRATE_TIME_MSEC)
			continue;
		// tscDelta = MIN(tscDelta, (tscEnd - tscStart))
		if ( (aperfEnd - aperfStart) < aperfDelta )
			aperfDelta = aperfEnd - aperfStart;
	}
	/* mperfDelta is now the least number of MPERF ticks the processor made in
	 * a timespan of 0.03 s (e.g. 30 milliseconds)
	 */
	
	if (aperfDelta > (1ULL<<32))
		retval = 0;
	else
	{
		retval = aperfDelta * 1000 / 30;
	}
	disable_PIT2();
	return retval;
}
#endif

#endif

/*
 License for x2apic_enabled, get_apicbase, compute_bclk.
 
 Copyright (c) 2010, Intel Corporation
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 * Neither the name of Intel Corporation nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
static inline __attribute__((always_inline)) void rdmsr32(uint32_t msr, uint32_t * lo_data_addr, uint32_t * hi_data_addr);
static inline __attribute__((always_inline)) void wrmsr32(uint32_t msr, uint32_t lo_data, uint32_t hi_data);
static uint32_t x2apic_enabled(void);
static uint32_t get_apicbase(void);
static uint32_t compute_bclk(void);
static inline __attribute__((always_inline)) void rdmsr32(uint32_t msr, uint32_t * lo_data_addr, uint32_t * hi_data_addr)
{    
    __asm__ volatile(
					 "rdmsr"
					 : "=a" (*lo_data_addr), "=d" (*hi_data_addr)
					 : "c" (msr)
					 );    
}	
static inline __attribute__((always_inline)) void wrmsr32(uint32_t msr, uint32_t lo_data, uint32_t hi_data)
{
    __asm__ __volatile__ (
						  "wrmsr"
						  : /* No outputs */
						  : "c" (msr), "a" (lo_data), "d" (hi_data)
						  );
}
#define MSR_APIC_BASE 0x1B
#define APIC_TMR_INITIAL_CNT 0x380
#define APIC_TMR_CURRENT_CNT 0x390
#define APIC_TMR_DIVIDE_CFG 0x3E0
#define MSR_APIC_TMR_INITIAL_CNT 0x838
#define MSR_APIC_TMR_CURRENT_CNT 0x839
#define MSR_APIC_TMR_DIVIDE_CFG 0x83E
static uint32_t x2apic_enabled(void)
{
    uint64_t temp64;
	
    temp64 = rdmsr64(MSR_APIC_BASE);
	
    return (uint32_t) (temp64 & (1 << 10)) ? 1 : 0;
}
static uint32_t get_apicbase(void)
{
    uint64_t temp64;
	
    temp64 = rdmsr64(MSR_APIC_BASE);
	
    return (uint32_t) (temp64 & 0xfffff000);
}
static uint32_t compute_bclk(void)
{
    uint32_t dummy;
    uint32_t start, stop;
    uint8_t temp8;
    uint16_t delay_count;
    uint32_t bclk;
	
#define DELAY_IN_US 1000
	
    // Compute fixed delay as time
    // delay count = desired time * PIT frequency
    // PIT frequency = 1.193182 MHz
    delay_count = 1193182 / DELAY_IN_US;
	
    // PIT channel 2 gate is controlled by IO port 0x61, bit 0
#define PIT_CH2_LATCH_REG 0x61
#define CH2_SPEAKER (1 << 1) // bit 1 -- 1 = speaker enabled 0 = speaker disabled
#define CH2_GATE_IN (1 << 0) // bit 0 -- 1 = gate enabled, 0 = gate disabled
#define CH2_GATE_OUT (1 << 5) // bit 5 -- 1 = gate latched, 0 = gate not latched
	
    // PIT Command register
#define PIT_MODE_COMMAND_REG 0x43
#define SELECT_CH2 (2 << 6)
#define ACCESS_MODE_LOBYTE_HIBYTE (3 << 4)
#define MODE0_INTERRUPT_ON_TERMINAL_COUNT 0 // Despite name, no interrupts on CH2
	
    // PIT Channel 2 data port
#define PIT_CH2_DATA 0x42
	
    // Disable the PIT channel 2 speaker and gate
    temp8 = inb(PIT_CH2_LATCH_REG);
    temp8 &= ~(CH2_SPEAKER | CH2_GATE_IN);
    outb(PIT_CH2_LATCH_REG, temp8);
	
    // Setup command and mode
    outb(PIT_MODE_COMMAND_REG, SELECT_CH2 | ACCESS_MODE_LOBYTE_HIBYTE | MODE0_INTERRUPT_ON_TERMINAL_COUNT);
	
    // Set time for fixed delay
    outb(PIT_CH2_DATA, (uint8_t) (delay_count));
    outb(PIT_CH2_DATA, (uint8_t) (delay_count >> 8));
	
    // Prepare to enable channel 2 gate but leave the speaker disabled
    temp8 = inb(PIT_CH2_LATCH_REG);
    temp8 &= ~CH2_SPEAKER;
    temp8 |= CH2_GATE_IN;
	
    if (x2apic_enabled())
	{
        // Set APIC Timer Divide Value as 2
        wrmsr32(MSR_APIC_TMR_DIVIDE_CFG, 0, 0);
		
        // start APIC timer with a known value
        start = ~0UL;
        wrmsr32(MSR_APIC_TMR_INITIAL_CNT, start, 0);
    }
    else
	{
        // Set APIC Timer Divide Value as 2
        *(volatile uint32_t *)(uint32_t) (get_apicbase() + APIC_TMR_DIVIDE_CFG) = 0UL;
		
        // start APIC timer with a known value
        start = ~0UL;
        *(volatile uint32_t *)(uint32_t) (get_apicbase() + APIC_TMR_INITIAL_CNT) = start;
    }
	
    // Actually start the PIT channel 2
    outb(PIT_CH2_LATCH_REG, temp8);
	
    // Wait for the fixed delay
    while (!(inb(PIT_CH2_LATCH_REG) & CH2_GATE_OUT));
	
    if (x2apic_enabled())
	{
        // read the APIC timer to determine the change that occurred over this fixed delay
        rdmsr32(MSR_APIC_TMR_CURRENT_CNT, &stop, &dummy);
		
        // stop APIC timer
        wrmsr32(MSR_APIC_TMR_INITIAL_CNT, 0, 0);
		
    }
    else
	{
        // read the APIC timer to determine the change that occurred over this fixed delay
        stop = *(volatile uint32_t *)(uint32_t) (get_apicbase() + APIC_TMR_CURRENT_CNT);
		
        // stop APIC timer
        *(volatile uint32_t *)(uint32_t) (get_apicbase() + APIC_TMR_INITIAL_CNT) = 0UL;
    }
	
    // Disable channel 2 speaker and gate input
    temp8 = inb(PIT_CH2_LATCH_REG);
    temp8 &= ~(CH2_SPEAKER | CH2_GATE_IN);
    outb(PIT_CH2_LATCH_REG, temp8);
	
    bclk = (start - stop) * 2 / DELAY_IN_US;
	
    // Round bclk to the nearest 100/12 integer value
    bclk = ((((bclk * 24) + 100) / 200) * 200) / 24;
	
    return bclk;
}


/*
 * Calculates the FSB and CPU frequencies using specific MSRs for each CPU
 * - multi. is read from a specific MSR. In the case of Intel, there is:
 *     a max multi. (used to calculate the FSB freq.),
 *     and a current multi. (used to calculate the CPU freq.)
 * - fsbFrequency = tscFrequency / multi
 * - cpuFrequency = fsbFrequency * multi
 */

void scan_cpu(PlatformInfo_t *p)
{
	uint64_t	tscFrequency = 0, fsbFrequency = 0, cpuFrequency = 0;
	uint64_t	msr;
	uint8_t		maxcoef = 0, maxdiv = 0, currcoef = 0, currdiv = 0;
    uint32_t	reg[4];
    uint32_t        cores_per_package;
    uint32_t        logical_per_package;    
    
	do_cpuid(0, reg);
    p->CPU.Vendor		= reg[ebx];
    p->CPU.cpuid_max_basic     = reg[eax];
    
    do_cpuid2(0x00000004, 0, reg);
    cores_per_package		= bitfield(reg[eax], 31, 26) + 1;
    
    /* get extended cpuid results */
	do_cpuid(0x80000000, reg);
	p->CPU.cpuid_max_ext = reg[eax];
    
    
	/* Begin of Copyright: from Apple's XNU cpuid.c */
	
	/* get brand string (if supported) */
	if (p->CPU.cpuid_max_ext > 0x80000004)
	{		
        char        str[128], *s;
		/*
		 * The brand string 48 bytes (max), guaranteed to
		 * be NUL terminated.
		 */
		do_cpuid(0x80000002, reg);
		bcopy((char *)reg, &str[0], 16);
		do_cpuid(0x80000003, reg);
		bcopy((char *)reg, &str[16], 16);
		do_cpuid(0x80000004, reg);
		bcopy((char *)reg, &str[32], 16);
		for (s = str; *s != '\0'; s++)
		{
			if (*s != ' ') break;
		}
		
		strlcpy(p->CPU.BrandString,	s, sizeof(p->CPU.BrandString));
		
		if (!strncmp(p->CPU.BrandString, CPUID_STRING_UNKNOWN, min(sizeof(p->CPU.BrandString), (unsigned)strlen(CPUID_STRING_UNKNOWN) + 1)))
		{
            /*
             * This string means we have a firmware-programmable brand string,
             * and the firmware couldn't figure out what sort of CPU we have.
             */
            p->CPU.BrandString[0] = '\0';
        }
	}  
	
    /*
	 * Get processor signature and decode
	 * and bracket this with the approved procedure for reading the
	 * the microcode version number a.k.a. signature a.k.a. BIOS ID
	 */
#ifndef AMD_SUPPORT
	wrmsr64(MSR_IA32_BIOS_SIGN_ID, 0);
	do_cpuid(1, reg);
    p->CPU.MicrocodeVersion =
    (uint32_t) (rdmsr64(MSR_IA32_BIOS_SIGN_ID) >> 32);	
#else
	do_cpuid(1, reg);
#endif	
	p->CPU.Signature        = reg[eax];
	p->CPU.Stepping         = bitfield(reg[eax],  3,  0);
	p->CPU.Model            = bitfield(reg[eax],  7,  4);
	p->CPU.Family           = bitfield(reg[eax], 11,  8);
	p->CPU.ExtModel         = bitfield(reg[eax], 19, 16);
	p->CPU.ExtFamily        = bitfield(reg[eax], 27, 20);
	p->CPU.Brand            = bitfield(reg[ebx],  7,  0);
	p->CPU.Features         = quad(reg[ecx], reg[edx]);
    
	if (p->CPU.cpuid_max_ext >= 0x80000001)
	{
		do_cpuid(0x80000001, reg);
		p->CPU.ExtFeatures =
        quad(reg[ecx], reg[edx]);
		
	}
	
    /* Fold extensions into family/model */
	if (p->CPU.Family == 0x0f)
		p->CPU.Family += p->CPU.ExtFamily;
	if (p->CPU.Family == 0x0f || p->CPU.Family == 0x06)
		p->CPU.Model += (p->CPU.ExtModel << 4);
    
    if (p->CPU.Features & CPUID_FEATURE_HTT)
		logical_per_package =
        bitfield(reg[ebx], 23, 16);
	else
		logical_per_package = 1;	
    
	if (p->CPU.cpuid_max_ext >= 0x80000007)
	{
		do_cpuid(0x80000007, reg);  
		
		/* Fold in the Invariant TSC feature bit, if present */
		p->CPU.ExtFeatures |=
        reg[edx] & (uint32_t)CPUID_EXTFEATURE_TSCI;
		
#ifdef AMD_SUPPORT
		/* Fold in the Hardware P-State control feature bit, if present */
		p->CPU.ExtFeatures |=
        reg[edx] & (uint32_t)_Bit(7);
		
		/* Fold in the read-only effective frequency interface feature bit, if present */
		p->CPU.ExtFeatures |=
        reg[edx] & (uint32_t)_Bit(10);
#endif
	}    
	
    if (p->CPU.cpuid_max_basic >= 0x5) {        
		/*
		 * Extract the Monitor/Mwait Leaf info:
		 */
		do_cpuid(5, reg);
#ifndef AMD_SUPPORT
        p->CPU.sub_Cstates  = reg[edx];
#endif
        p->CPU.extensions   = reg[ecx];	
	}
	
#ifndef AMD_SUPPORT    
    if (p->CPU.cpuid_max_basic >= 0x6)
    {        
		/*
		 * The thermal and Power Leaf:
		 */
		do_cpuid(6, reg);
		p->CPU.dynamic_acceleration = bitfield(reg[eax], 1, 1); // "Dynamic Acceleration Technology (Turbo Mode)"
		p->CPU.invariant_APIC_timer = bitfield(reg[eax], 2, 2); //  "Invariant APIC Timer"
        p->CPU.fine_grain_clock_mod = bitfield(reg[eax], 4, 4);
	}
	
    if ((p->CPU.Vendor == 0x756E6547 /* Intel */) && 
		(p->CPU.Family == 0x06))
	{
		/*
		 * Find the number of enabled cores and threads
		 * (which determines whether SMT/Hyperthreading is active).
		 */
		switch (p->CPU.Model)
		{
				
			case CPUID_MODEL_DALES_32NM:
			case CPUID_MODEL_WESTMERE:
			case CPUID_MODEL_WESTMERE_EX:
			{
				msr = rdmsr64(MSR_CORE_THREAD_COUNT);
				p->CPU.NoThreads = bitfield((uint32_t)msr, 15,  0);
				p->CPU.NoCores   = bitfield((uint32_t)msr, 19, 16);            
				break;
			}
				
			case CPUID_MODEL_NEHALEM:
			case CPUID_MODEL_FIELDS:
			case CPUID_MODEL_DALES:
			case CPUID_MODEL_NEHALEM_EX:
			case CPUID_MODEL_SANDYBRIDGE:
			case CPUID_MODEL_JAKETOWN:
			{
				msr = rdmsr64(MSR_CORE_THREAD_COUNT);
				p->CPU.NoThreads = bitfield((uint32_t)msr, 15,  0);
				p->CPU.NoCores   = bitfield((uint32_t)msr, 31, 16);            
				break;
			}        
		}
    }
#endif
    if (p->CPU.NoCores == 0)
	{
		p->CPU.NoThreads    = logical_per_package;
		p->CPU.NoCores      = cores_per_package ? cores_per_package : 1 ;
	}
	
	/* End of Copyright: from Apple's XNU cpuid.c */
    
	fsbFrequency = (uint64_t)(compute_bclk() * 1000000);

#if LEGACY_CPU
	tscFrequency = measure_tsc_frequency();
#endif	
	
#ifdef AMD_SUPPORT
#define K8_FIDVID_STATUS		0xC0010042
#define K10_COFVID_STATUS		0xC0010071
	if (p->CPU.ExtFeatures & _Bit(10))
	{		
		cpuFrequency = measure_aperf_frequency();
	}
	
    if ((p->CPU.Vendor == 0x68747541 /* AMD */) && (p->CPU.Family == 0x0f))
	{
		switch(p->CPU.ExtFamily)
		{
			case 0x00: /* K8 */
				msr = rdmsr64(K8_FIDVID_STATUS);
				maxcoef = bitfield(msr, 21, 16) / 2 + 4;
				currcoef = bitfield(msr, 5, 0) / 2 + 4;
				break;
				
			case 0x01: /* K10 */
            {
                //uint32_t reg[4];
				msr = rdmsr64(K10_COFVID_STATUS);
				/*
				do_cpuid2(0x00000006, 0, reg);
				 EffFreq: effective frequency interface
				if (bitfield(reg[ecx], 0, 0) == 1)
				{
					uint64_t aperf = measure_aperf_frequency();
					cpuFrequency = aperf;
				}
				*/				 
				// NOTE: tsc runs at the maccoeff (non turbo)
				//			*not* at the turbo frequency.
				maxcoef	 = bitfield(msr, 54, 49) / 2 + 4;
				currcoef = bitfield(msr, 5, 0) + 0x10;
				currdiv = 2 << bitfield(msr, 8, 6);
				
				break;
			}	
			case 0x05: /* K14 */
				msr = rdmsr64(K10_COFVID_STATUS);
				currcoef  = (bitfield(msr, 54, 49) + 0x10) << 2;
				currdiv = (bitfield(msr, 8, 4) + 1) << 2;
				currdiv += bitfield(msr, 3, 0);
				
				break;
				
			case 0x02: /* K11 */
				DBG("K11 detected, but not supported !!!\n");
				// not implimented
				break;
		}
		
		if (!fsbFrequency)
		{
			if (maxcoef)
			{
				if (currdiv)
				{
					if (!currcoef) currcoef = maxcoef;
					if (!cpuFrequency)
						fsbFrequency = ((tscFrequency * currdiv) / currcoef);
					else
						fsbFrequency = ((cpuFrequency * currdiv) / currcoef);
					
					DBG("%d.%d\n", currcoef / currdiv, ((currcoef % currdiv) * 100) / currdiv);
				} else {
					if (!cpuFrequency)
						fsbFrequency = (tscFrequency / maxcoef);
					else 
						fsbFrequency = (cpuFrequency / maxcoef);
					DBG("%d\n", currcoef);
				}
			}
			else if (currcoef)
			{
				if (currdiv)
				{
					fsbFrequency = ((tscFrequency * currdiv) / currcoef);
					DBG("%d.%d\n", currcoef / currdiv, ((currcoef % currdiv) * 100) / currdiv);
				} else {
					fsbFrequency = (tscFrequency / currcoef);
					DBG("%d\n", currcoef);
				}
			}
		}
		
	}	
#else
    if ((p->CPU.Vendor == 0x756E6547 /* Intel */) && 
		((p->CPU.Family == 0x06) || 
		 (p->CPU.Family == 0x0f)))
	{
		if ((p->CPU.Family == 0x06 && p->CPU.Model >= 0x0c) || 
			(p->CPU.Family == 0x0f && p->CPU.Model >= 0x03))
		{
			/* Nehalem CPU model */
			if (p->CPU.Family == 0x06 && (p->CPU.Model == CPUID_MODEL_NEHALEM || 
                                          p->CPU.Model == CPUID_MODEL_FIELDS || 
                                          p->CPU.Model == CPUID_MODEL_DALES || 
                                          p->CPU.Model == CPUID_MODEL_DALES_32NM || 
                                          p->CPU.Model == CPUID_MODEL_WESTMERE ||
                                          p->CPU.Model == CPUID_MODEL_NEHALEM_EX ||
                                          p->CPU.Model == CPUID_MODEL_WESTMERE_EX ||
                                          p->CPU.Model == CPUID_MODEL_SANDYBRIDGE ||
                                          p->CPU.Model == CPUID_MODEL_JAKETOWN)) 
			{
				uint8_t		bus_ratio_max = 0, bus_ratio_min = 0;
				uint32_t	max_ratio = 0;
				uint64_t	flex_ratio = 0;
				msr = rdmsr64(MSR_PLATFORM_INFO);
#if DEBUG_CPU
				DBG("msr(%d): platform_info %08x\n", __LINE__, msr & 0xffffffff);
#endif
				bus_ratio_max = (msr >> 8) & 0xff;
				bus_ratio_min = (msr >> 40) & 0xff; 
				msr = rdmsr64(MSR_FLEX_RATIO);
#if DEBUG_CPU
				DBG("msr(%d): flex_ratio %08x\n", __LINE__, msr & 0xffffffff);
#endif
				if ((msr >> 16) & 0x01)
				{
					flex_ratio = (msr >> 8) & 0xff;
					/* bcc9: at least on the gigabyte h67ma-ud2h,
					 where the cpu multipler can't be changed to
					 allow overclocking, the flex_ratio msr has unexpected (to OSX)
					 contents.  These contents cause mach_kernel to
					 fail to compute the bus ratio correctly, instead
					 causing the system to crash since tscGranularity
					 is inadvertently set to 0.
					 */
					if (flex_ratio == 0)
					{
						/* Clear bit 16 (evidently the
						 presence bit) */
						wrmsr64(MSR_FLEX_RATIO, (msr & 0xFFFFFFFFFFFEFFFFULL));
						msr = rdmsr64(MSR_FLEX_RATIO);
#if DEBUG_CPU
						DBG("Unusable flex ratio detected.  MSR Patched to %08x\n", msr & 0xffffffff);
#endif
					}
					else
					{
						if (bus_ratio_max > flex_ratio)
						{
							bus_ratio_max = flex_ratio;
						}
					}
				}
#if LEGACY_CPU
				if (bus_ratio_max)
				{
					fsbFrequency = (tscFrequency / bus_ratio_max);
				}
#endif
				//valv: Turbo Ratio Limit
				if ((p->CPU.Model != 0x2e) && (p->CPU.Model != 0x2f))
				{
					//msr = rdmsr64(MSR_TURBO_RATIO_LIMIT);
					cpuFrequency = bus_ratio_max * fsbFrequency;
					max_ratio = bus_ratio_max * 10;
				}
				else
				{
#if LEGACY_CPU
					cpuFrequency = tscFrequency;
#else
					cpuFrequency = bus_ratio_max * fsbFrequency;
#endif
				}								
#if DEBUG_CPU
				DBG("Sticking with [BCLK: %dMhz, Bus-Ratio: %d]\n", fsbFrequency / 1000000, max_ratio);
#endif
				currcoef = bus_ratio_max;
			} 
			else
			{
				msr = rdmsr64(MSR_IA32_PERF_STATUS);
#if DEBUG_CPU
				DBG("msr(%d): ia32_perf_stat 0x%08x\n", __LINE__, msr & 0xffffffff);
#endif
				currcoef = (msr >> 8) & 0x1f;
				/* Non-integer bus ratio for the max-multi*/
				maxdiv = (msr >> 46) & 0x01;
				/* Non-integer bus ratio for the current-multi (undocumented)*/
				currdiv = (msr >> 14) & 0x01;
                
				if ((p->CPU.Family == 0x06 && p->CPU.Model >= 0x0e) || 
					(p->CPU.Family == 0x0f)) // This will always be model >= 3
				{
					/* On these models, maxcoef defines TSC freq */
					maxcoef = (msr >> 40) & 0x1f;
				} 
				else 
				{
					/* On lower models, currcoef defines TSC freq */
					/* XXX */
					maxcoef = currcoef;
				}
				if (!currcoef) currcoef = maxcoef;
#if LEGACY_CPU
				if (maxcoef) 
				{					
					
					if (maxdiv)
					{
						fsbFrequency = ((tscFrequency * 2) / ((maxcoef * 2) + 1));
					}
					else 
					{
						fsbFrequency = (tscFrequency / maxcoef);
					}
					
					if (currdiv) 
					{
						cpuFrequency = (fsbFrequency * ((currcoef * 2) + 1) / 2);
					}
					else 
					{
						cpuFrequency = (fsbFrequency * currcoef);
					}
#if DEBUG_CPU
					DBG("max: %d%s current: %d%s\n", maxcoef, maxdiv ? ".5" : "",currcoef, currdiv ? ".5" : "");
#endif
				}
#else
				
				
				if (currdiv) 
				{
					cpuFrequency = (fsbFrequency * ((currcoef * 2) + 1) / 2);
				}
				else 
				{
					cpuFrequency = (fsbFrequency * currcoef);
				}
				
				if (maxcoef) 
				{
					if (maxdiv)
					{
						tscFrequency  = (fsbFrequency * ((maxcoef * 2) + 1)) / 2;
					}
					else 
					{
						tscFrequency = fsbFrequency * maxcoef;
					}
				}								
#if DEBUG_CPU
				DBG("max: %d%s current: %d%s\n", maxcoef, maxdiv ? ".5" : "",currcoef, currdiv ? ".5" : "");
#endif
                
#endif // LEGACY_CPU
				
			}
		}
        /* Mobile CPU ? */ 
		//Slice 
	    p->CPU.isMobile = false;
		switch (p->CPU.Model)
		{
			case 0x0D:
				p->CPU.isMobile = true; 
				break;			
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x06:	
				p->CPU.isMobile = (rdmsr64(0x2C) & (1 << 21));
				break;
			default:
				p->CPU.isMobile = (rdmsr64(0x17) & (1 << 28));
				break;
		}
        // TODO: this part of code seems to work very well for the intel platforms, need to find the equivalent for AMD
		DBG("%s platform found.\n", p->CPU.isMobile?"Mobile":"Desktop");
	}
#endif
	if (!cpuFrequency) cpuFrequency = tscFrequency;

	p->CPU.MaxCoef = maxcoef;
	p->CPU.MaxDiv = maxdiv;
	p->CPU.CurrCoef = currcoef;
	p->CPU.CurrDiv = currdiv;
	
    p->CPU.TSCFrequency = tscFrequency ;
	p->CPU.FSBFrequency = fsbFrequency ;
	p->CPU.CPUFrequency = cpuFrequency ;
#ifdef AMD_SUPPORT
    msglog("AMD CPU Detection Enabled\n");
#endif
	DBG("CPU: Vendor/Model/ExtModel: 0x%x/0x%x/0x%x\n", p->CPU.Vendor, p->CPU.Model, p->CPU.ExtModel);
	DBG("CPU: Family/ExtFamily:      0x%x/0x%x\n", p->CPU.Family, p->CPU.ExtFamily);
#ifdef AMD_SUPPORT
	DBG("CPU (AMD): TSCFreq:               %dMHz\n", p->CPU.TSCFrequency / 1000000);
	DBG("CPU (AMD): FSBFreq:               %dMHz\n", p->CPU.FSBFrequency / 1000000);
	DBG("CPU (AMD): CPUFreq:               %dMHz\n", p->CPU.CPUFrequency / 1000000);
	DBG("CPU (AMD): MaxCoef/CurrCoef:      0x%x/0x%x\n", p->CPU.MaxCoef, p->CPU.CurrCoef);
	DBG("CPU (AMD): MaxDiv/CurrDiv:        0x%x/0x%x\n", p->CPU.MaxDiv, p->CPU.CurrDiv);
#else
	DBG("CPU: TSCFreq:               %dMHz\n", p->CPU.TSCFrequency / 1000000);
	DBG("CPU: FSBFreq:               %dMHz\n", p->CPU.FSBFrequency / 1000000);
	DBG("CPU: CPUFreq:               %dMHz\n", p->CPU.CPUFrequency / 1000000);
	DBG("CPU: MaxCoef/CurrCoef:      0x%x/0x%x\n", p->CPU.MaxCoef, p->CPU.CurrCoef);
	DBG("CPU: MaxDiv/CurrDiv:        0x%x/0x%x\n", p->CPU.MaxDiv, p->CPU.CurrDiv);		
#endif
	
	DBG("CPU: NoCores/NoThreads:     %d/%d\n", p->CPU.NoCores, p->CPU.NoThreads);
	DBG("CPU: Features:              0x%08x\n", p->CPU.Features);
    DBG("CPU: ExtFeatures:           0x%08x\n", p->CPU.ExtFeatures);
#ifndef AMD_SUPPORT
    DBG("CPU: MicrocodeVersion:      %d\n", p->CPU.MicrocodeVersion);
#endif
#if DEBUG_CPU
	pause();
#endif
	
}
