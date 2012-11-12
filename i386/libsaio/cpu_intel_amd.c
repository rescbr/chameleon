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

//#define LEGACY_CPU 

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

#ifdef LEGACY_CPU
static uint64_t measure_tsc_frequency(void);
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
#endif


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
 * - FSBFreq = TSCFreq / multi
 * - CPUFreq = FSBFreq * multi
 */

void scan_cpu(void)
{
	uint64_t	msr = 0;        
    
    
    uint64_t	Features = 0;		// CPU Features like MMX, SSE2, VT ...
	uint64_t	ExtFeatures = 0;    // CPU Extended Features like SYSCALL, XD, EM64T, LAHF ...
    uint64_t	TSCFreq = 0 ;
    uint64_t    FSBFreq = 0 ;    
    uint64_t    CPUFreq = 0;
    
    uint32_t	reg[4];
    uint32_t    cores_per_package = 0;
    uint32_t    logical_per_package = 0;
    
    uint32_t	Vendor = 0;			// Vendor
	uint32_t	Signature = 0;		// Signature
	uint8_t     Stepping = 0;		// Stepping
	uint8_t     Model = 0;			// Model
	uint8_t     ExtModel = 0;		// Extended Model
	uint8_t     Family = 0;			// Family
	uint8_t     ExtFamily = 0;		// Extended Family
	uint32_t	NoCores = 0;		// No Cores per Package
	uint32_t	NoThreads = 0;		// Threads per Package
	uint8_t     Brand = 0; 
	uint32_t	MicrocodeVersion = 0;   // The microcode version number a.k.a. signature a.k.a. BIOS ID 
    
	uint8_t     isMobile = 0;        
	
	boolean_t	dynamic_acceleration = 0;
	boolean_t	invariant_APIC_timer = 0;
	boolean_t	fine_grain_clock_mod = 0;
	
	uint32_t    cpuid_max_basic = 0;
	uint32_t    cpuid_max_ext = 0;
	uint32_t	sub_Cstates = 0;
	uint32_t    extensions = 0;    
    
	uint8_t		maxcoef = 0, maxdiv = 0, currcoef = 0, currdiv = 0;
    char		CpuBrandString[48];	// 48 Byte Branding String    
    
	do_cpuid(0, reg);
    Vendor		= reg[ebx];
    cpuid_max_basic     = reg[eax];
	
    if (Vendor == CPUID_VENDOR_INTEL)
    {
        do_cpuid2(0x00000004, 0, reg);
        cores_per_package		= bitfield(reg[eax], 31, 26) + 1;
    } 
    else if (Vendor != CPUID_VENDOR_AMD)
    {
        stop("Error: CPU unsupported\n");
        return;
    }
	
    /* get extended cpuid results */
	do_cpuid(0x80000000, reg);
	cpuid_max_ext = reg[eax];    
    
	/* Begin of Copyright: from Apple's XNU cpuid.c */
	
	/* get brand string (if supported) */
	if (cpuid_max_ext > 0x80000004)
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
		
		strlcpy(CpuBrandString,	s, sizeof(CpuBrandString));
		
		if (!strncmp(CpuBrandString, CPUID_STRING_UNKNOWN, min(sizeof(CpuBrandString), (unsigned)strlen(CPUID_STRING_UNKNOWN) + 1)))
		{
            /*
             * This string means we have a firmware-programmable brand string,
             * and the firmware couldn't figure out what sort of CPU we have.
             */
            CpuBrandString[0] = '\0';
        }
	}  
	
    /*
	 * Get processor signature and decode
	 * and bracket this with the approved procedure for reading the
	 * the microcode version number a.k.a. signature a.k.a. BIOS ID
	 */
    if (Vendor == CPUID_VENDOR_INTEL )
    {
        wrmsr64(MSR_IA32_BIOS_SIGN_ID, 0);
        do_cpuid(1, reg);
        MicrocodeVersion =
        (uint32_t) (rdmsr64(MSR_IA32_BIOS_SIGN_ID) >> 32);	
    }
    else if (Vendor != CPUID_VENDOR_AMD)
        do_cpuid(1, reg);

	Signature        = reg[eax];
	Stepping         = bitfield(reg[eax],  3,  0);
	Model            = bitfield(reg[eax],  7,  4);
	Family           = bitfield(reg[eax], 11,  8);
	ExtModel         = bitfield(reg[eax], 19, 16);
	ExtFamily        = bitfield(reg[eax], 27, 20);
	Brand            = bitfield(reg[ebx],  7,  0);
	Features         = quad(reg[ecx], reg[edx]);
    		
    /* Fold extensions into family/model */
	if (Family == 0x0f)
		Family += ExtFamily;
	if (Family == 0x0f || Family == 0x06)
		Model += (ExtModel << 4);
    
    if (Features & CPUID_FEATURE_HTT)
		logical_per_package =
        bitfield(reg[ebx], 23, 16);
	else
		logical_per_package = 1;
	
	if (cpuid_max_ext >= 0x80000001)
	{
		do_cpuid(0x80000001, reg);
		ExtFeatures =
        quad(reg[ecx], reg[edx]);
		
	}
	
	if (cpuid_max_ext >= 0x80000007)
	{
		do_cpuid(0x80000007, reg);  
		
		/* Fold in the Invariant TSC feature bit, if present */
		ExtFeatures |=
        reg[edx] & (uint32_t)CPUID_EXTFEATURE_TSCI;
		
        if (Vendor == CPUID_VENDOR_AMD)
        {
            /* Fold in the Hardware P-State control feature bit, if present */
           ExtFeatures |=
            reg[edx] & (uint32_t)_Bit(7);
            
            /* Fold in the read-only effective frequency interface feature bit, if present */
            ExtFeatures |=
            reg[edx] & (uint32_t)_Bit(10);
        }

	}    
	
    if (Vendor == CPUID_VENDOR_AMD )
    {
        if (cpuid_max_ext >= 0x80000008)
        {
            if (Features & CPUID_FEATURE_HTT) 
            {
                do_cpuid(0x80000008, reg);
                cores_per_package		= bitfield(reg[ecx], 7 , 0) + 1; // NC + 1
            }
        }		
    }	
    if (cpuid_max_basic >= 0x5) {        
		/*
		 * Extract the Monitor/Mwait Leaf info:
		 */
		do_cpuid(5, reg);
        if (Vendor == CPUID_VENDOR_INTEL )
        {
            sub_Cstates  = reg[edx];
        }
        
        extensions   = reg[ecx];	
	}
	
    if (Vendor == CPUID_VENDOR_INTEL)
    {
        if (cpuid_max_basic >= 0x6)
        {        
            /*
             * The thermal and Power Leaf:
             */
            do_cpuid(6, reg);
            dynamic_acceleration = bitfield(reg[eax], 1, 1); // "Dynamic Acceleration Technology (Turbo Mode)"
            invariant_APIC_timer = bitfield(reg[eax], 2, 2); //  "Invariant APIC Timer"
            fine_grain_clock_mod = bitfield(reg[eax], 4, 4);
        }
        
        if ((Vendor == 0x756E6547 /* Intel */) && 
            (Family == 0x06))
        {
            /*
             * Find the number of enabled cores and threads
             * (which determines whether SMT/Hyperthreading is active).
             */
            switch (Model)
            {
                    
                case CPUID_MODEL_DALES_32NM:
                case CPUID_MODEL_WESTMERE:
                case CPUID_MODEL_WESTMERE_EX:
                {
                    msr = rdmsr64(MSR_CORE_THREAD_COUNT);
                    NoThreads = bitfield((uint32_t)msr, 15,  0);
                    NoCores   = bitfield((uint32_t)msr, 19, 16);            
                    break;
                }
                    
                case CPUID_MODEL_NEHALEM:
                case CPUID_MODEL_FIELDS:
                case CPUID_MODEL_DALES:
                case CPUID_MODEL_NEHALEM_EX:
                case CPUID_MODEL_SANDYBRIDGE:
                case CPUID_MODEL_JAKETOWN:
				case CPUID_MODEL_IVYBRIDGE:
                {
                    msr = rdmsr64(MSR_CORE_THREAD_COUNT);
                    NoThreads = bitfield((uint32_t)msr, 15,  0);
                    NoCores   = bitfield((uint32_t)msr, 31, 16);            
                    break;
                }        
            }
        }
    }
    
    if (NoCores == 0)
	{
        if (Vendor == CPUID_VENDOR_AMD)
        {
            if (!cores_per_package) {
                //legacy method
                if ((ExtFeatures & _HBit(1)/* CmpLegacy */) && ( Features & CPUID_FEATURE_HTT) )
                    cores_per_package = logical_per_package; 
                else 
                    cores_per_package = 1;
            }		
        }
		NoThreads    = logical_per_package;
		NoCores      = cores_per_package ? cores_per_package : 1 ;
	}
	
	/* End of Copyright: from Apple's XNU cpuid.c */
    
	FSBFreq = (uint64_t)(compute_bclk() * 1000000);

#ifdef LEGACY_CPU
	TSCFreq = measure_tsc_frequency();
#endif	
	
    if (Vendor == CPUID_VENDOR_AMD)
    {

#define K8_FIDVID_STATUS		0xC0010042
#define K10_COFVID_STATUS		0xC0010071
        if (ExtFeatures & _Bit(10))
        {		
            CPUFreq = measure_aperf_frequency();
        }
        
        if ((Vendor == CPUID_VENDOR_AMD) && (Family == 0x0f))
        {
            switch(ExtFamily)
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
                     CPUFreq = aperf;
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
            
            if (!FSBFreq)
            {
                if (maxcoef)
                {
                    if (currdiv)
                    {
                        if (!currcoef) currcoef = maxcoef;
                        if (!CPUFreq)
                            FSBFreq = ((TSCFreq * currdiv) / currcoef);
                        else
                            FSBFreq = ((CPUFreq * currdiv) / currcoef);
                        
                        DBG("%d.%d\n", currcoef / currdiv, ((currcoef % currdiv) * 100) / currdiv);
                    } else {
                        if (!CPUFreq)
                            FSBFreq = (TSCFreq / maxcoef);
                        else 
                            FSBFreq = (CPUFreq / maxcoef);
                        DBG("%d\n", currcoef);
                    }
                }
                else if (currcoef)
                {
                    if (currdiv)
                    {
                        FSBFreq = ((TSCFreq * currdiv) / currcoef);
                        DBG("%d.%d\n", currcoef / currdiv, ((currcoef % currdiv) * 100) / currdiv);
                    } else {
                        FSBFreq = (TSCFreq / currcoef);
                        DBG("%d\n", currcoef);
                    }
                }
            }
            
        }
        
        // NOTE: This is not the approved method,
        // the method provided by AMD is: 
        // if ((PowerNow == enabled (cpuid_max_ext >= 0x80000007)) && (StartupFID(??) != MaxFID(??))) then "mobile processor present"
        
        if (strstr(CpuBrandString, "obile")) 
            isMobile = true;
        else 
            isMobile = false;
        
        DBG("%s platform detected.\n", isMobile?"Mobile":"Desktop");
    }
    else if ((Vendor == CPUID_VENDOR_INTEL) && 
		((Family == 0x06) || 
		 (Family == 0x0f)))
	{
		if ((Family == 0x06 && Model >= 0x0c) || 
			(Family == 0x0f && Model >= 0x03))
		{
			/* Nehalem CPU model */
			if (Family == 0x06 && (Model == CPUID_MODEL_NEHALEM || 
                                          Model == CPUID_MODEL_FIELDS || 
                                          Model == CPUID_MODEL_DALES || 
                                          Model == CPUID_MODEL_DALES_32NM || 
                                          Model == CPUID_MODEL_WESTMERE ||
                                          Model == CPUID_MODEL_NEHALEM_EX ||
                                          Model == CPUID_MODEL_WESTMERE_EX ||
                                          Model == CPUID_MODEL_SANDYBRIDGE ||
                                          Model == CPUID_MODEL_JAKETOWN)) 
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
#ifdef LEGACY_CPU
				if (bus_ratio_max)
				{
					FSBFreq = (TSCFreq / bus_ratio_max);
				}
#endif
				//valv: Turbo Ratio Limit
				if ((Model != 0x2e) && (Model != 0x2f))
				{
					//msr = rdmsr64(MSR_TURBO_RATIO_LIMIT);
					CPUFreq = bus_ratio_max * FSBFreq;
					max_ratio = bus_ratio_max * 10;
				}
				else
				{
#ifdef LEGACY_CPU
					CPUFreq = TSCFreq;
#else
					CPUFreq = bus_ratio_max * FSBFreq;
#endif
				}								
#if DEBUG_CPU
				DBG("Sticking with [BCLK: %dMhz, Bus-Ratio: %d]\n", FSBFreq / 1000000, max_ratio);
#endif
				currcoef = bus_ratio_max;
                
                TSCFreq = CPUFreq;
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
                
				if ((Family == 0x06 && Model >= 0x0e) || 
					(Family == 0x0f)) // This will always be model >= 3
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
#ifdef LEGACY_CPU
				if (maxcoef) 
				{					
					
					if (maxdiv)
					{
						FSBFreq = ((TSCFreq * 2) / ((maxcoef * 2) + 1));
					}
					else 
					{
						FSBFreq = (TSCFreq / maxcoef);
					}
					
					if (currdiv) 
					{
						CPUFreq = (FSBFreq * ((currcoef * 2) + 1) / 2);
					}
					else 
					{
						CPUFreq = (FSBFreq * currcoef);
					}
#if DEBUG_CPU
					DBG("max: %d%s current: %d%s\n", maxcoef, maxdiv ? ".5" : "",currcoef, currdiv ? ".5" : "");
#endif
				}
#else
				
				
				if (currdiv) 
				{
					CPUFreq = (FSBFreq * ((currcoef * 2) + 1) / 2);
				}
				else 
				{
					CPUFreq = (FSBFreq * currcoef);
				}
				
				if (maxcoef) 
				{
					if (maxdiv)
					{
						TSCFreq  = (FSBFreq * ((maxcoef * 2) + 1)) / 2;
					}
					else 
					{
						TSCFreq = FSBFreq * maxcoef;
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
	    isMobile = false;
		switch (Model)
		{
			case 0x0D:
				isMobile = true; 
				break;			
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x06:	
				isMobile = (rdmsr64(0x2C) & (1 << 21));
				break;
			default:
				isMobile = (rdmsr64(0x17) & (1 << 28));
				break;
		}

		DBG("%s platform detected.\n",isMobile?"Mobile":"Desktop");
	}

	if (!CPUFreq) CPUFreq = TSCFreq;
    if (!TSCFreq) TSCFreq = CPUFreq;
    
	if (Vendor == CPUID_VENDOR_INTEL) {
		set_env(envDynamicAcceleration,  dynamic_acceleration);    
		set_env(envInvariantAPICTimer,	 invariant_APIC_timer);    
		set_env(envFineGrainClockMod,  fine_grain_clock_mod);
		set_env(envMicrocodeVersion, MicrocodeVersion);  
		set_env(envSubCstates,  sub_Cstates);
	}	 
	set_env(envVendor,          Vendor);
    set_env(envModel,           Model);    
    set_env(envExtModel,        ExtModel);    
    
	set_env(envCPUIDMaxBasic, cpuid_max_basic);
	set_env(envCPUIDMaxBasic, cpuid_max_ext);

    set_env_copy(envBrandString, CpuBrandString, sizeof(CpuBrandString));
	set_env(envSignature, Signature);    
	set_env(envStepping,  Stepping);    
	set_env(envFamily,	 Family);    
	set_env(envExtModel,  ExtModel);    
	set_env(envExtFamily, ExtFamily);    
	set_env(envBrand,	 Brand);    
	set_env(envFeatures,  Features);
    set_env(envExtFeatures,  ExtFeatures);

	set_env(envExtensions,   extensions); 

	set_env(envNoThreads,	 NoThreads);    
	set_env(envNoCores,		 NoCores);
	set_env(envIsMobile,		 isMobile);
	
	set_env(envMaxCoef,		 maxcoef);    
	set_env(envMaxDiv,		 maxdiv);
	set_env(envCurrCoef,		 currcoef);
	set_env(envCurrDiv,	     currdiv);    
	set_env(envTSCFreq,	 TSCFreq);
	set_env(envFSBFreq,	 FSBFreq);
	set_env(envCPUFreq,	 CPUFreq);
	
}
