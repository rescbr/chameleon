/*
 * Copyright 2008 Islam Ahmed Zaid. All rights reserved.  <azismed@gmail.com>
 * AsereBLN: 2009: cleanup and bugfix
 */

#include "libsaio.h"
#include "platform.h"
//#include "mem.h"
#include "smbios_patcher.h"
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

void scan_cpu() //PlatformInfo_t *p)
{
	PlatformInfo_t *p = Platform;
	int i = 0;
	uint8_t turbo = 0;
	uint64_t	tscFrequency, fsbFrequency, cpuFrequency;
	uint64_t	msr; //, flex_ratio;
	uint8_t		maxcoef, maxdiv, currcoef, currdiv, mindiv;

	maxcoef = maxdiv = currcoef = currdiv = mindiv = 0;
	
#if DEBUG_CPU
	printf("Enter cpuid_info\n");
	pause();
#endif
	cpuid_update_generic_info();
	
#if DEBUG_CPU
	printf("...OK\n");
	pause();
#endif
	
#if OLDMETHOD	
	/* get cpuid values */
	for( ; i <= 3; i++)
	{
		do_cpuid(i, p->CPU.CPUID[i]);
	}
	
	do_cpuid2(0x00000004, 0, p->CPU.CPUID[CPUID_4]);
	do_cpuid(0x80000000, p->CPU.CPUID[CPUID_80]);
	if ((p->CPU.CPUID[CPUID_80][0] & 0x0000000f) >= 1) {
		do_cpuid(0x80000001, p->CPU.CPUID[CPUID_81]);
	}
#if DEBUG_CPU
	{
		int		i;
		DBG("CPUID Raw Values:\n");
		for (i=0; i<CPUID_MAX; i++) {
			DBG("%02d: %08x-%08x-%08x-%08x\n", i,
				p->CPU.CPUID[i][0], p->CPU.CPUID[i][1],
				p->CPU.CPUID[i][2], p->CPU.CPUID[i][3]);
		}
	}
#endif
	p->CPU.Vendor		= p->CPU.CPUID[CPUID_0][1];
	p->CPU.Signature	= p->CPU.CPUID[CPUID_1][0];
	p->CPU.Stepping		= bitfield(p->CPU.CPUID[CPUID_1][0], 3, 0);
	p->CPU.Model		= bitfield(p->CPU.CPUID[CPUID_1][0], 7, 4);
	p->CPU.Family		= bitfield(p->CPU.CPUID[CPUID_1][0], 11, 8);
	p->CPU.ExtModel		= bitfield(p->CPU.CPUID[CPUID_1][0], 19, 16);
	p->CPU.ExtFamily	= bitfield(p->CPU.CPUID[CPUID_1][0], 27, 20);
	p->CPU.NoThreads	= bitfield(p->CPU.CPUID[CPUID_1][1], 23, 16);
	p->CPU.NoCores		= bitfield(p->CPU.CPUID[CPUID_4][0], 31, 26) + 1;

	p->CPU.Model += (p->CPU.ExtModel << 4);
	
	/* get brand string (if supported) */
	/* Copyright: from Apple's XNU cpuid.c */
	if (p->CPU.CPUID[CPUID_80][0] > 0x80000004) {
		uint32_t	reg[4];
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
		
		if (!strncmp(p->CPU.BrandString, CPU_STRING_UNKNOWN, min(sizeof(p->CPU.BrandString), strlen(CPU_STRING_UNKNOWN) + 1))) {
			 /*
			  * This string means we have a firmware-programmable brand string,
			  * and the firmware couldn't figure out what sort of CPU we have.
			  */
			 p->CPU.BrandString[0] = '\0';
		 }
	}
	
	/* setup features */
	p->CPU.Features |= (CPU_FEATURE_MMX | CPU_FEATURE_SSE | CPU_FEATURE_SSE2 | CPU_FEATURE_MSR) & p->CPU.CPUID[CPUID_1][3];
	p->CPU.Features |= (CPU_FEATURE_SSE3 | CPU_FEATURE_SSE41 | CPU_FEATURE_SSE42) & p->CPU.CPUID[CPUID_1][2];	
	p->CPU.Features |= (CPU_FEATURE_EM64T) & p->CPU.CPUID[CPUID_81][3];


	//if ((CPU_FEATURE_HTT & p->CPU.CPUID[CPUID_1][3]) != 0) {
	if (p->CPU.NoThreads > p->CPU.NoCores) {
		p->CPU.Features |= CPU_FEATURE_HTT;
	}
#else //Slice
	p->CPU.Vendor		= *(UInt32*)&cpuid_info()->cpuid_vendor;
	p->CPU.Signature	= cpuid_info()->cpuid_signature;
	p->CPU.Stepping		= cpuid_info()->cpuid_stepping;
	p->CPU.Model		= cpuid_info()->cpuid_model;
	p->CPU.Family		= cpuid_info()->cpuid_family;
	p->CPU.ExtModel		= cpuid_info()->cpuid_extmodel;
	p->CPU.ExtFamily	= cpuid_info()->cpuid_extfamily;
//	DBG("CPU: Vendor/Model/ExtModel: 0x%x/0x%x/0x%x\n", p->CPU.Vendor, p->CPU.Model, p->CPU.ExtModel);
//	DBG("CPU: Family/ExtFamily:      0x%x/0x%x\n", p->CPU.Family, p->CPU.ExtFamily);
	
	strlcpy(p->CPU.BrandString, cpuid_info()->cpuid_brand_string, sizeof(p->CPU.BrandString));
	DBG("CPU: BrandString %s\n", p->CPU.BrandString);
	p->CPU.Features = cpuid_info()->cpuid_features;
	p->CPU.NoCores  = cpuid_info()->core_count;
	p->CPU.NoThreads = cpuid_info()->thread_count;
//	DBG("CPU: MaxCoef/CurrCoef:      0x%x/0x%x\n", p->CPU.MaxCoef, p->CPU.CurrCoef);
//	DBG("CPU: MaxDiv/CurrDiv:        0x%x/0x%x\n", p->CPU.MaxDiv?2:1, p->CPU.CurrDiv?2:1);
//	DBG("CPU: TSCFreq:               %dMHz\n", p->CPU.TSCFrequency / 1000000);
//	DBG("CPU: FSBFreq:               %dMHz\n", p->CPU.FSBFrequency / 1000000);
//	DBG("CPU: CPUFreq:               %dMHz\n", p->CPU.CPUFrequency / 1000000);
//	DBG("CPU: NoCores/NoThreads:     %d/%d\n", p->CPU.NoCores, p->CPU.NoThreads);
//	DBG("CPU: Features:              0x%08x\n", p->CPU.Features);
#if DEBUG_CPU
	pause();
#endif
	
#endif

	tscFrequency = measure_tsc_frequency();
	DBG("measure_tsc_frequency = %dMHz\n", tscFrequency / MEGA);
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
			if (p->CPU.Family == 0x06 && (p->CPU.Model == 0x1a || p->CPU.Model == 0x1e ||
										  p->CPU.Model == 0x1f || p->CPU.Model == 0x25 ||
										  p->CPU.Model == 0x19 || p->CPU.Model == 0x2c)) 
			{
				msr = rdmsr64(MSR_PLATFORM_INFO);
				DBG("msr(0x%04x): platform_info %08x-%08x\n", MSR_PLATFORM_INFO,
				(msr >> 32) & 0xffffffff, msr & 0xffffffff);
				mindiv = (msr >> 40) & 0xff;
				maxcoef = (msr >> 8) & 0xff;  
				
				msr = rdmsr64(MSR_TURBO_RATIO);
				turbo = msr & 0x7f;
				//Slice - doesn't work
				/*
				msr = rdmsr64(MSR_FLEX_RATIO);
				DBG("msr(0x%04x): flex_ratio %08x\n", MSR_FLEX_RATIO, msr & 0xffffffff);
				if ((msr >> 16) & 0x01) {
					flex_ratio = (msr >> 8) & 0xff;
					if (currcoef > flex_ratio) {
						currcoef = flex_ratio;
					}
				}*/
				msr = rdmsr64(MSR_IA32_PERF_STATUS);
				if (msr) {
					currcoef = msr & 0x1f;
				}
				
				if (!currcoef) {
					currcoef = maxcoef;
				}
				
				if (currcoef < mindiv) {
					currcoef = mindiv;
				}
				
				if (currcoef) {
					fsbFrequency = (tscFrequency / currcoef);
				}
				cpuFrequency = tscFrequency;
			} 
			else //not nehalem
			{
				//Slice - it is not FSB frequency. It is System Bus Speed: FSB = SBS * 4;	
				if (p->CPU.Family != 0x0d){
					msr = rdmsr64(MSR_FSB_FREQ);
					switch (msr & 7) {
						case 0:
							fsbFrequency = 266670 * 1000;
							break;
						case 1:
							fsbFrequency = 133330 * 1000;
							break;
						case 2:
							fsbFrequency = 200000 * 1000;
							break;
						case 3:
							fsbFrequency = 166670 * 1000;
							break;
						case 4:
							fsbFrequency = 333330 * 1000;
							break;
						case 5:
							fsbFrequency = 200000 * 1000;
							break;
						case 6:
							fsbFrequency = 400000 * 1000;
							break;
						default:
							fsbFrequency = 0;
							break;
					}
					DBG("msr(0x%04x): MSR_FSB_FREQ %d.%dMHz\n", MSR_FSB_FREQ,
						fsbFrequency/MEGA, (fsbFrequency%MEGA)/1000);
				}
				
				msr = rdmsr64(MSR_PLATFORM_INFO);  //info only?
				uint32_t m2 = msr >> 32;
				DBG("msr(0x%04x): platform_info %08x-%08x\n", MSR_PLATFORM_INFO,
					m2 & 0xffffffff, msr & 0xffffffff);
				turbo = (m2 >> 8) & 0x1f;
				
				msr = rdmsr64(MSR_IA32_PERF_STATUS);
				m2 = msr >> 32;
				DBG("msr(0x%04x): MSR_IA32_PERF_STATUS %08x-%08x\n", MSR_IA32_PERF_STATUS,
					m2 & 0xffffffff, msr & 0xffffffff);
				
				currcoef = (msr >> 8) & 0x1f;
				mindiv = (msr >> 24) & 0xf;
				if (currcoef < mindiv) {
					currcoef = mindiv;
				}
				
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
					if (!fsbFrequency) {
						if (maxdiv)
						{
							fsbFrequency = ((tscFrequency * 2) / ((maxcoef * 2) + 1));
						}
						else 
						{
							fsbFrequency = (tscFrequency / maxcoef);
						}
						
					}
					
					if (currdiv) 
					{
						cpuFrequency = (fsbFrequency * ((currcoef * 2) + 1) / 2);
					}
					else 
					{
						cpuFrequency = (fsbFrequency * currcoef);
					}
					DBG("max: %d%s current: %d%s\n", maxcoef, maxdiv ? ".5" : "",currcoef, currdiv ? ".5" : "");
				}
			}
		}
		/* Mobile CPU ? */
//Slice 
		msr = rdmsr64(MSR_IA32_PLATFORM_ID);
		DBG("msr(0x%04x): MSR_IA32_PLATFORM_ID 0x%08x\n", MSR_IA32_PLATFORM_ID, msr & 0xffffffff); //__LINE__ - source line number :)
		if (!scanDMI() && msr) {
			p->CPU.Mobile = FALSE;
			switch (p->CPU.Model) {
				case 0x0D:
					p->CPU.Mobile = TRUE; // CPU_FEATURE_MOBILE;
					break;
				case 0x0F:
					p->CPU.Mobile = FALSE; // CPU_FEATURE_MOBILE;
					break;
				case 0x02:
				case 0x03:
				case 0x04:
				case 0x06:	
					p->CPU.Mobile = (rdmsr64(MSR_P4_EBC_FREQUENCY_ID) && (1 << 21));
					break;
				default:
					p->CPU.Mobile = (rdmsr64(MSR_IA32_PLATFORM_ID) && (1<<28));
					break;
			}
			if (p->CPU.Mobile) {
				p->CPU.Features |= CPU_FEATURE_MOBILE;
			}
		}
		DBG("CPU is %s\n", p->CPU.Mobile?"Mobile":"Desktop");
			
	}
#if 0
	else if((p->CPU.Vendor == 0x68747541 /* AMD */) && (p->CPU.Family == 0x0f))
	{
		if(p->CPU.ExtFamily == 0x00 /* K8 */)
		{
			msr = rdmsr64(K8_FIDVID_STATUS);
			currcoef = (msr & 0x3f) / 2 + 4;
			currdiv = (msr & 0x01) * 2;
		} 
		else if(p->CPU.ExtFamily >= 0x01 /* K10+ */)
		{
			msr = rdmsr64(K10_COFVID_STATUS);
			if(p->CPU.ExtFamily == 0x01 /* K10 */)
				currcoef = (msr & 0x3f) + 0x10;
			else /* K11+ */
				currcoef = (msr & 0x3f) + 0x08;
			currdiv = (2 << ((msr >> 6) & 0x07));
		}

		if (currcoef) 
		{
			if (currdiv) 
			{
				fsbFrequency = ((tscFrequency * currdiv) / currcoef);
				DBG("%d.%d\n", currcoef / currdiv, ((currcoef % currdiv) * 100) / currdiv);
			}
			else
			{
				fsbFrequency = (tscFrequency / currcoef);
				DBG("%d\n", currcoef);
			}
			fsbFrequency = (tscFrequency / currcoef);
			cpuFrequency = tscFrequency;
		}
	}
#endif
	else if(p->CPU.Vendor == 0x746e6543 && p->CPU.Family == 6)
	{
		switch (p->CPU.Model) {
			case CPU_VIA_NANO:
				// NOTE: TSC is constant, irrelevent of speed steping 
				break;
			default:
				break;
		}
		
		msr = rdmsr64(MSR_NANO_FCR2);
		verbose("MSR_IA32_EBL_CR_POWERON Returns 0x%X 0x%X\n", msr >> 32,  msr & 0xffffffff);
		
		//msr = msr >> 32;
		msr |= VIA_ALTERNATIVE_VENDOR_BIT;
		//msr = msr << 32;
		
		verbose("MSR_IA32_EBL_CR_POWERON Returns 0x%X 0x%X\n", msr >> 32,  msr & 0xffffffff);
		wrmsr64(MSR_NANO_FCR2, msr);
		msr = rdmsr64(MSR_NANO_FCR2);
		verbose("MSR_IA32_EBL_CR_POWERON Returns 0x%X 0x%X\n", msr >> 32,  msr & 0xffffffff);
		
		
		/* get cpuid values */
		for( ; i <= 3; i++)
		{
			do_cpuid(i, p->CPU.CPUID[i]);
		}
		//int numcpuid_supported = p->CPU.CPUID[CPUID_0][0];	// max number cpuid call
		//int numextcpuid = p->CPU.CPUID[CPUID_80][0];
															//p->CPU.Features = 0;
															//		bitfield(p->CPU.CPUID[CPUID_1][1], 0, 0) FEATURE_C
		
		// CPUID_0 -> largest cpuid val in EAX
		// CPUID_0 -> rem = vendor string
		/*
		CPUID_1 EDX:
		 0 -> FPU
		 1 -> VME
		 2 -> DE
		 3 -> PSE
		 4 -> TSC
		 5 -> MSR
		 6 -> PAE
		 7 -> MCE
		 8 -> CX8
		 9 -> APIC
		 10 ->  Reserved
		 11 -> Fast Call
		 12 -> MTTR
		 13 -> PGE
		 14 -> MCA
		 15 -> CMOV
		 16 -> PAT
		 17 -> PSE36
		 18 -> Serial Number 
		 23 -> MMX
		 24 -> FXSR
		 25 -> SSE
		 */
		
		//CPUID_80 -> largest excpuid value in EAX
		//CPUID_81,EAX -> Signature
		//CPUID_80,EDX -> Ext Features
		//CPUID_82 -> CPU String
		//CPUID_83 -> CPU String
		//CPUID_84 -> CPU String
		p->CPU.NoThreads = p->CPU.NoCores;
		
	}
	
	if (!fsbFrequency) {
		fsbFrequency = (DEFAULT_FSB * 1000);
		cpuFrequency = tscFrequency;
		msglog("CPU: fsb=0 ! using the default value 100MHz !\n");
	}
	
/*	
	p->CPU.Vendor		= p->CPU.CPUID[CPUID_0][1];
	p->CPU.Signature	= p->CPU.CPUID[CPUID_1][0];
	p->CPU.Stepping		= bitfield(p->CPU.CPUID[CPUID_1][0], 3, 0);
	p->CPU.Model		= bitfield(p->CPU.CPUID[CPUID_1][0], 7, 4);
	p->CPU.Family		= bitfield(p->CPU.CPUID[CPUID_1][0], 11, 8);
	p->CPU.ExtModel		= bitfield(p->CPU.CPUID[CPUID_1][0], 19, 16);
	p->CPU.ExtFamily	= bitfield(p->CPU.CPUID[CPUID_1][0], 27, 20);
	p->CPU.NoThreads	= bitfield(p->CPU.CPUID[CPUID_1][1], 23, 16);
*/	


	p->CPU.MaxCoef = turbo;
	p->CPU.MaxDiv = maxdiv;
	p->CPU.MinCoef = mindiv;
	p->CPU.CurrCoef = currcoef;
	p->CPU.CurrDiv = currdiv;
	p->CPU.TSCFrequency = tscFrequency;
	p->CPU.FSBFrequency = fsbFrequency;
	p->CPU.CPUFrequency = cpuFrequency;

	DBG("CPU: Vendor/Model/ExtModel: 0x%x/0x%x/0x%x\n", p->CPU.Vendor, p->CPU.Model, p->CPU.ExtModel);
	DBG("CPU: Family/ExtFamily:      0x%x/0x%x\n", p->CPU.Family, p->CPU.ExtFamily);
	DBG("CPU: MaxCoef/CurrCoef/Turbo:      0x%x/0x%x\n", p->CPU.MaxCoef, p->CPU.CurrCoef, turbo);
	DBG("CPU: MaxDiv/CurrDiv:        0x%x/0x%x\n", p->CPU.MaxDiv?2:1, p->CPU.CurrDiv?2:1);
	DBG("CPU: TSCFreq:               %dMHz\n", p->CPU.TSCFrequency / 1000000);
	DBG("CPU: FSBFreq:               %dMHz\n", p->CPU.FSBFrequency / 1000000);
	DBG("CPU: CPUFreq:               %dMHz\n", p->CPU.CPUFrequency / 1000000);
	DBG("CPU: NoCores/NoThreads:     %d/%d\n", p->CPU.NoCores, p->CPU.NoThreads);
	DBG("CPU: Features:              0x%08x\n", p->CPU.Features);
#if DEBUG_CPU
	pause();
#endif
}
