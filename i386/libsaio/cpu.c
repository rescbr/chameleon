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
	uint64_t	tscFrequency, fsbFrequency, cpuFrequency;
	uint64_t	msr;
	uint8_t		maxcoef, maxdiv, currcoef, currdiv;
    uint32_t	reg[4];
    uint32_t	CPUID[CPUID_MAX][4];	// CPUID 0..4, 80..81 Raw Values
    uint32_t        cores_per_package;
    uint32_t        logical_per_package;
	maxcoef = maxdiv = currcoef = currdiv = 0;
	    
	do_cpuid(0, CPUID[0]);
    p->CPU.Vendor		= CPUID[CPUID_0][1];
    
    do_cpuid2(0x00000004, 0, CPUID[CPUID_4]);
    cores_per_package		= bitfield(CPUID[CPUID_4][0], 31, 26) + 1;
    
    /* get extended cpuid results */
	do_cpuid(0x80000000, reg);
	uint32_t cpuid_max_ext = reg[eax];
    
    /* get brand string (if supported) */
	/* Copyright: from Apple's XNU cpuid.c */
	if (cpuid_max_ext > 0x80000004) {		
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
		for (s = str; *s != '\0'; s++) {
			if (*s != ' ') break;
		}
		
		strlcpy(p->CPU.BrandString,	s, sizeof(p->CPU.BrandString));
		
		if (!strncmp(p->CPU.BrandString, CPUID_STRING_UNKNOWN, min(sizeof(p->CPU.BrandString), (unsigned)strlen(CPUID_STRING_UNKNOWN) + 1))) {
            /*
             * This string means we have a firmware-programmable brand string,
             * and the firmware couldn't figure out what sort of CPU we have.
             */
            p->CPU.BrandString[0] = '\0';
        }
	}  
       
    /* get processor signature and decode */
	do_cpuid(1, reg);
	p->CPU.Signature        = reg[eax];
	p->CPU.Stepping         = bitfield(reg[eax],  3,  0);
	p->CPU.Model            = bitfield(reg[eax],  7,  4);
	p->CPU.Family           = bitfield(reg[eax], 11,  8);
	p->CPU.ExtModel         = bitfield(reg[eax], 19, 16);
	p->CPU.ExtFamily        = bitfield(reg[eax], 27, 20);
	p->CPU.Brand            = bitfield(reg[ebx],  7,  0);
	p->CPU.Features         = quad(reg[ecx], reg[edx]);
    //p->CPU.Type           = bitfield(reg[eax], 13, 12);
    
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
    
	if (cpuid_max_ext >= 0x80000001) {
		do_cpuid(0x80000001, reg);
		p->CPU.ExtFeatures =
        quad(reg[ecx], reg[edx]);
                
	}
    
	/* Fold in the Invariant TSC feature bit, if present */
	if (cpuid_max_ext >= 0x80000007) {
		do_cpuid(0x80000007, reg);  
		p->CPU.ExtFeatures |=
        reg[edx] & (uint32_t)CPUID_EXTFEATURE_TSCI;
	}
    
    /* Find the microcode version number a.k.a. signature a.k.a. BIOS ID */
    p->CPU.MicrocodeVersion =
    (uint32_t) (rdmsr64(MSR_IA32_BIOS_SIGN_ID) >> 32);
    
    if ((p->CPU.Vendor == 0x756E6547 /* Intel */) && 
		(p->CPU.Family == 0x06)) {
            /*
             * Find the number of enabled cores and threads
             * (which determines whether SMT/Hyperthreading is active).
             */
            switch (p->CPU.Model) {
                    /*
                     * This should be the same as Nehalem but an A0 silicon bug returns
                     * invalid data in the top 12 bits. Hence, we use only bits [19..16]
                     * rather than [31..16] for core count - which actually can't exceed 8. 
                     */
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
        
    if (p->CPU.NoCores == 0) {
		p->CPU.NoThreads    = cores_per_package;
		p->CPU.NoCores      = logical_per_package;
	}
    
	    
	tscFrequency = measure_tsc_frequency();
	fsbFrequency = 0;
	cpuFrequency = 0;
	
		
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
				bus_ratio_min = (msr >> 40) & 0xff; //valv: not sure about this one (Remarq.1)
				msr = rdmsr64(MSR_FLEX_RATIO);
#if DEBUG_CPU
				DBG("msr(%d): flex_ratio %08x\n", __LINE__, msr & 0xffffffff);
#endif
				if ((msr >> 16) & 0x01) {
					flex_ratio = (msr >> 8) & 0xff;
					/* bcc9: at least on the gigabyte h67ma-ud2h,
					 where the cpu multipler can't be changed to
					 allow overclocking, the flex_ratio msr has unexpected (to OSX)
					 contents.  These contents cause mach_kernel to
					 fail to compute the bus ratio correctly, instead
					 causing the system to crash since tscGranularity
					 is inadvertently set to 0.
					 */
					if (flex_ratio == 0) {
						/* Clear bit 16 (evidently the
						 presence bit) */
						wrmsr64(MSR_FLEX_RATIO, (msr & 0xFFFFFFFFFFFEFFFFULL));
						msr = rdmsr64(MSR_FLEX_RATIO);
#if DEBUG_CPU
						DBG("Unusable flex ratio detected.  MSR Patched to %08x\n", msr & 0xffffffff);
#endif
					} else {
						if (bus_ratio_max > flex_ratio) {
							bus_ratio_max = flex_ratio;
						}
					}
				}
				
				if (bus_ratio_max) {
					fsbFrequency = (tscFrequency / bus_ratio_max);
				}
				//valv: Turbo Ratio Limit
				if ((p->CPU.Model != 0x2e) && (p->CPU.Model != 0x2f)) {
					msr = rdmsr64(MSR_TURBO_RATIO_LIMIT);
					cpuFrequency = bus_ratio_max * fsbFrequency;
					max_ratio = bus_ratio_max * 10;
				} else {
					cpuFrequency = tscFrequency;
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
			}
		}
        /* Mobile CPU ? */ 
		//Slice 
	    p->CPU.isMobile = false;
		switch (p->CPU.Model) {
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
        
		DBG("%s platform found.\n", p->CPU.isMobile?"Mobile":"Desktop");
	}
	
	p->CPU.MaxCoef = maxcoef;
	p->CPU.MaxDiv = maxdiv;
	p->CPU.CurrCoef = currcoef;
	p->CPU.CurrDiv = currdiv;
    
	p->CPU.TSCFrequency = (tscFrequency / 1000000) * 1000000;
	p->CPU.FSBFrequency = (fsbFrequency / 1000000) * 1000000;
	p->CPU.CPUFrequency = (cpuFrequency / 1000000) * 1000000;
    
    //p->CPU.TSCFrequency = tscFrequency ;
	//p->CPU.FSBFrequency = fsbFrequency ;
	//p->CPU.CPUFrequency = cpuFrequency ;
    
	DBG("CPU: Vendor/Model/ExtModel: 0x%x/0x%x/0x%x\n", p->CPU.Vendor, p->CPU.Model, p->CPU.ExtModel);
	DBG("CPU: Family/ExtFamily:      0x%x/0x%x\n", p->CPU.Family, p->CPU.ExtFamily);
	DBG("CPU: TSCFreq:               %dMHz\n", p->CPU.TSCFrequency / 1000000);	
	if(p->CPU.Vendor == 0x756E6547 /* Intel */)
	{		
		DBG("CPU: FSBFreq:               %dMHz\n", p->CPU.FSBFrequency / 1000000);
		DBG("CPU: CPUFreq:               %dMHz\n", p->CPU.CPUFrequency / 1000000);
		DBG("CPU: MaxCoef/CurrCoef:      0x%x/0x%x\n", p->CPU.MaxCoef, p->CPU.CurrCoef);
		DBG("CPU: MaxDiv/CurrDiv:        0x%x/0x%x\n", p->CPU.MaxDiv, p->CPU.CurrDiv);		
	}
	
	DBG("CPU: NoCores/NoThreads:     %d/%d\n", p->CPU.NoCores, p->CPU.NoThreads);
	DBG("CPU: Features:              0x%08x\n", p->CPU.Features);
    DBG("CPU: ExtFeatures:           0x%08x\n", p->CPU.ExtFeatures); // where is SYSCALL ??
    DBG("CPU: MicrocodeVersion:      %d\n", p->CPU.MicrocodeVersion);
#if DEBUG_CPU
		pause();
#endif
	
}
