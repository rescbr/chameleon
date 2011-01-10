/*
 * Copyright 2008 Islam Ahmed Zaid. All rights reserved.  <azismed@gmail.com>
 * AsereBLN: 2009: cleanup and bugfix
 * valv:     2010: fine-tuning and additions
 */

#include "libsaio.h"
#include "platform.h"
#include "cpu.h"
#include "boot.h"
#include "bootstruct.h"

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
	const char	*newratio, *newfsb;
	int			len, myfsb, i;
	uint64_t	tscFrequency, fsbFrequency, cpuFrequency, fsbi;
	uint64_t	msr, flex_ratio = 0;
	uint32_t	tms, ida, max_ratio, min_ratio;
	uint8_t		bus_ratio_max, maxdiv, bus_ratio_min, currdiv;
	bool		fix_fsb, did, core_i, turbo, isatom, fsbad;

	max_ratio = min_ratio = myfsb = bus_ratio_max = maxdiv = bus_ratio_min = currdiv = i = 0;

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
		printf("CPUID Raw Values:\n");
		for (i=0; i<CPUID_MAX; i++) {
			printf("%02d: %08x-%08x-%08x-%08x\n", i,
				p->CPU.CPUID[i][0], p->CPU.CPUID[i][1],
				p->CPU.CPUID[i][2], p->CPU.CPUID[i][3]);
		}
	}
#endif
	p->CPU.Vendor		= p->CPU.CPUID[CPUID_0][1];
	p->CPU.Signature	= p->CPU.CPUID[CPUID_1][0];
	p->CPU.Stepping		= bitfield(p->CPU.CPUID[CPUID_1][0], 3, 0);
	p->CPU.Model		= bitfield(p->CPU.CPUID[CPUID_1][0], 7, 4);
	p->CPU.Type			= bitfield(p->CPU.CPUID[CPUID_1][0], 13, 12);
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
	p->CPU.Features |= (CPU_FEATURE_MMX | CPU_FEATURE_SSE | CPU_FEATURE_SSE2 | CPU_FEATURE_MSR | CPU_FEATURE_APIC | CPU_FEATURE_TM1 | CPU_FEATURE_ACPI) & p->CPU.CPUID[CPUID_1][3];
	p->CPU.Features |= (CPU_FEATURE_SSE3 | CPU_FEATURE_SSE41 | CPU_FEATURE_SSE42 | CPU_FEATURE_EST | CPU_FEATURE_TM2 | CPU_FEATURE_SSSE3 | CPU_FEATURE_xAPIC) & p->CPU.CPUID[CPUID_1][2];
	p->CPU.Features |= (CPU_FEATURE_EM64T | CPU_FEATURE_XD) & p->CPU.CPUID[CPUID_81][3];
	p->CPU.Features |= (CPU_FEATURE_LAHF) & p->CPU.CPUID[CPUID_81][2];


	//if ((CPU_FEATURE_HTT & p->CPU.CPUID[CPUID_1][3]) != 0) {
	if (p->CPU.NoThreads > p->CPU.NoCores) {
		p->CPU.Features |= CPU_FEATURE_HTT;
	}
	 

	tscFrequency = measure_tsc_frequency();
	fsbFrequency = 0;
	cpuFrequency = 0;
	fsbi = 0;
	fix_fsb = false;
	did = false;
	core_i = false;
	turbo = false;
	isatom = false;
	fsbad = false;

	if ((p->CPU.Vendor == 0x756E6547 /* Intel */) && ((p->CPU.Family == 0x06) || (p->CPU.Family == 0x0f)))
	{
		verbose("CPU: ");
		int tjmax = 0;
		msr = rdmsr64(MSR_IA32_PLATFORM_ID);
		if((((msr >> 50) & 0x01) == 1) || (rdmsr64(0x17) & (1<<28)))
		{
			p->CPU.Features |= CPU_FEATURE_MOBILE;
			verbose("Mobile ");
		}
		verbose("%s\n", p->CPU.BrandString);
		
		if ((p->CPU.Family == 0x06 && p->CPU.Model >= 0x0c) || (p->CPU.Family == 0x0f && p->CPU.Model >= 0x03))
		{
			if (p->CPU.Family == 0x06)
			{
				int intelCPU = p->CPU.Model;
				int Stepp = p->CPU.Stepping;
				int bus;

				switch (intelCPU)
				{
					case 0xc:		// Core i7 & Atom
						if (strstr(p->CPU.BrandString, "Atom")) goto teleport;
					case 0x1a:		// Core i7 LGA1366, Xeon 5500, "Bloomfield", "Gainstown", 45nm
					case 0x1e:		// Core i7, i5 LGA1156, "Clarksfield", "Lynnfield", "Jasper", 45nm
					case 0x1f:		// Core i7, i5, Nehalem
					case 0x25:		// Core i7, i5, i3 LGA1156, "Westmere", "Clarkdale", "Arrandale", 32nm
					case 0x2c:		// Core i7 LGA1366, Six-core, "Westmere", "Gulftown", 32nm
					case 0x2e:		// Core i7, Nehalem-Ex Xeon, "Beckton"
					case 0x2f:		// Core i7, Nehalem-Ex Xeon, "Eagleton"
						core_i = true;
						tjmax = (rdmsr64(MSR_THERMAL_TARGET) >> 16) & 0xff;
						msr = rdmsr64(MSR_PLATFORM_INFO);
						bus_ratio_max = (msr >> 8) & 0xff;
						bus_ratio_min = (msr >> 40) & 0xff; //valv: not sure about this one (Remarq.1)
						verbose("CPU: Flex-Ratio = %d ", bus_ratio_max);
						min_ratio = bus_ratio_min * 10;
						msr = rdmsr64(MSR_FLEX_RATIO);
						if ((msr >> 16) & 0x01)
						{
							flex_ratio = (msr >> 8) & 0xff;
							verbose(">> %d", flex_ratio);
							if(bus_ratio_max > flex_ratio) bus_ratio_max = flex_ratio;
						}
						verbose("\n");
						if(bus_ratio_max) fsbFrequency = (tscFrequency / bus_ratio_max);

						//valv: Turbo Ratio Limit
						if ((intelCPU != 0x2e) && (intelCPU != 0x2f))
						{
							turbo = true;
							msr = rdmsr64(MSR_TURBO_RATIO_LIMIT);
							
							p->CPU.Tone = (msr >> 0) & 0xff;
							p->CPU.Ttwo = (msr >> 8) & 0xff;
							p->CPU.Tthr = (msr >> 16) & 0xff;
							p->CPU.Tfor = (msr >> 24) & 0xff;

							cpuFrequency = bus_ratio_max * fsbFrequency;
							max_ratio = bus_ratio_max * 10;
						}
						else cpuFrequency = tscFrequency;

						if ((getValueForKey(kbusratio, &newratio, &len, &bootInfo->bootConfig)) && (len <= 4))
						{
							max_ratio = atoi(newratio);
							max_ratio = (max_ratio * 10);
							if (len >= 3) max_ratio = (max_ratio + 5);

							verbose("Bus-Ratio: min=%d%s, max=%d%s\n", bus_ratio_min, bus_ratio_max);

							// extreme overclockers may love 320 ;)
							if ((max_ratio >= min_ratio) && (max_ratio <= 320))
							{
								cpuFrequency = (fsbFrequency * max_ratio) / 10;
								if (len >= 3) maxdiv = 1;
								else maxdiv = 0;
								verbose("Sticking with [BCLK: %dMhz, Bus-Ratio: %s]\n", myfsb, newratio);
							}
							else max_ratio = (bus_ratio_max * 10);
						}
						//valv: to be uncommented if Remarq.1 didn't stick
						/*if(bus_ratio_max > 0) bus_ratio = flex_ratio;*/
						p->CPU.MaxRatio = max_ratio;
						p->CPU.MinRatio = min_ratio;
						
						//fsbi = fsbFrequency;
						if(getIntForKey(kForceFSB, &myfsb, &bootInfo->bootConfig)) goto forcefsb;
						break;
					case 0xd:		// Pentium M, Dothan, 90nm
					case 0xe:		// Core Duo/Solo, Pentium M DC
						goto teleport;
					case 0xf:		// Core Xeon, Core 2 DC, 65nm
						switch (Stepp)
						{
							case 0x2:
								tjmax = 95;
								break;
							case 0x6:
								if (p->CPU.NoCores = 2) tjmax = 80;
								if (p->CPU.NoCores = 4) tjmax = 90;
								else tjmax = 85;
								break;
							case 0xb:
								tjmax = 90;
								break;
							case 0xd:
							default:
							teleport:
								msr = rdmsr64(MSR_IA32_EXT_CONFIG);
								if(msr & (1 << 30)) tjmax = 85;
								break;
						}
					case 0x1c:		// Atom :)
						switch (Stepp)
						{
							case 0xa:
								tjmax = 100;
								break;
							case 0x2:
							default:
								tjmax = 90;
								break;
						}
					case 0x17:		// Core 2 Duo/Extreme, Xeon, 45nm
						switch (Stepp)
						{
							case 0x6:		// Mobile Core2 Duo
								tjmax = 104;
								break;
							case 0xa:		// Mobile Centrino 2
								tjmax = 105;
								break;
							default:
								if (platformCPUFeature(CPU_FEATURE_MOBILE)) tjmax = 105;
								break;
						}
					case 0x16:		// Celeron, Core 2 SC, 65nm
					case 0x27:		// Atom Lincroft, 45nm
						core_i = false;
						//valv: todo: msr_therm2_ctl (0x19d) bit 16 (mode of automatic thermal monitor): 0=tm1, 1=tm2
						//also, if bit 3 of misc_enable is cleared the above would have no effect
						if (strstr(p->CPU.BrandString, "Atom"))
							isatom = true;
						if(!isatom && (platformCPUFeature(CPU_FEATURE_TM1)))
						{
							msr_t msr32;
							msr32 = rdmsr(MSR_IA32_MISC_ENABLE);

							//thermally-initiated on-die modulation of the stop-clock duty cycle
							if(!(rdmsr64(MSR_IA32_MISC_ENABLE) & (1 << 3))) msr32.lo |= (1 << 3);
							verbose("CPU: Thermal Monitor:              TM, ");
							
							//BIOS must enable this feature if the TM2 feature flag (CPUID.1:ECX[8]) is set
							if(platformCPUFeature(CPU_FEATURE_TM2))
							{
								//thermally-initiated frequency transitions
								msr32.lo |= (1 << 13);
								verbose("TM2, ");
							}
							msr32.lo |= (1 << 17);
							verbose("PROCHOT, ");
							msr32.lo |= (1 << 10);
							verbose("FERR\n");
							
							bool oem_ssdt, tmpval;
							oem_ssdt = false;
							
							oem_ssdt = getBoolForKey(kOEMSSDT, &tmpval, &bootInfo->bootConfig)&&tmpval;
							if(oem_ssdt)
							{
								bool c2e, c4e, hc4e;
								c2e = c4e = hc4e = false;

								getBoolForKey(kC2EEnable, &c2e, &bootInfo->bootConfig);
								if(c2e) msr32.lo |= (1 << 26);
								
								getBoolForKey(kC4EEnable, &c4e, &bootInfo->bootConfig);
								if((c4e) && platformCPUFeature(CPU_FEATURE_MOBILE)) msr32.hi |= (1 << (32 - 32));
								getBoolForKey(kHardC4EEnable, &hc4e, &bootInfo->bootConfig);
								if((hc4e) && platformCPUFeature(CPU_FEATURE_MOBILE)) msr32.hi |= (1 << (33 - 32));
							}
							
							msr32.hi |= (1 << (36 - 32)); // EMTTM

							wrmsr(MSR_IA32_MISC_ENABLE, msr32);
							
							msr32 = rdmsr(PIC_SENS_CFG);
							msr32.lo |= (1 << 21);
							wrmsr(PIC_SENS_CFG, msr32);
						}
						
						if (rdmsr64(MSR_IA32_EXT_CONFIG) & (1 << 27))
						{
							wrmsr64(MSR_IA32_EXT_CONFIG, (rdmsr64(MSR_IA32_EXT_CONFIG) | (1 << 28)));
							delay(1);
							did = rdmsr64(MSR_IA32_EXT_CONFIG) & (1 << 28);
						}
						getBoolForKey(kFixFSB, &fix_fsb, &bootInfo->bootConfig);
						if(fix_fsb)
						{
							msr = rdmsr64(MSR_FSB_FREQ);
							bus = (msr >> 0) & 0x7;
							if(p->CPU.Model == 0xd && bus == 0)
							{
								fsbFrequency = 100000000;
								myfsb = 100;
							}
							else if(p->CPU.Model == 0xe && p->CPU.ExtModel == 1) goto ratio;
							else
							{
								switch (bus)
								{
									case 0:
										fsbFrequency = 266666667;
										myfsb = 266;
										break;
									case 1:
										fsbFrequency = 133333333;
										myfsb = 133;
										break;
									case 3:
										fsbFrequency = 166666667;
										myfsb = 166;
										break;
									case 4:
										fsbFrequency = 333333333;
										myfsb = 333;
										break;
									case 5:
										fsbFrequency = 100000000;
										myfsb = 100;
										break;
									case 6:
										fsbFrequency = 400000000;
										myfsb = 400;
										break;
									case 2:
									default:
										fsbFrequency = 200000000;
										myfsb = 200;
										break;
								}
							}
							uint64_t minfsb = 183000000, maxfsb = 185000000;
							if (((fsbFrequency > minfsb) && (fsbFrequency < maxfsb)) || (!fsbFrequency))
							{
								fsbFrequency = 200000000;
								fsbad = true;
							}
							goto ratio;
						}
					case 0x1d:		// Xeon MP MP 7400
					// for 0x2a & 0x2b turbo is true;
					//case 0x2a:		// SNB
					//case 0x2b:		// SNB Xeon
					default:
						if(getIntForKey(kForceFSB, &myfsb, &bootInfo->bootConfig))
						{
							forcefsb:
							switch(myfsb)
							{
								case 133:
									fsbFrequency = 133333333;
									break;
								case 166:
									fsbFrequency = 166666667;
									break;
								case 233:
									fsbFrequency = 233333333;
									break;
								case 266:
									fsbFrequency = 266666667;
									break;
								case 333:
									fsbFrequency = 333333333;
									break;
								case 100:
								case 200:
								case 400:
									fsbFrequency = (myfsb * 1000000);
									break;
								default:
									getValueForKey(kForceFSB, &newfsb, &len, &bootInfo->bootConfig);
									if((len <= 3) && (myfsb < 400))
									{
										fsbFrequency = (myfsb * 1000000);
										verbose("Specified FSB: %dMhz. Assuming you know what you 're doing !\n", myfsb);
									}
									else if(core_i) fsbFrequency = 133333333;
									else fsbFrequency = 200000000;
									break;
							}
							if(core_i)
							{
								cpuFrequency = (fsbFrequency * max_ratio) / 10;
								verbose("Sticking with [BCLK: %dMhz, Bus-Ratio: %s]\n", myfsb, newratio);
								break;
							}
							fix_fsb = true;
						}
						goto ratio;
						break;
				}
			}
			else
			{
				ratio:
				msr = rdmsr64(MSR_IA32_PERF_STATUS);
				maxdiv = (msr >> 46) & 0x01;
				//valv: this seems to be bit 15 instead of 14.
				currdiv = (msr >> 15) & 0x01;
				uint8_t XE = (msr >> 31) & 0x01;

				msr_t msr32;
				msr32 = rdmsr(MSR_IA32_PERF_STATUS);
				bus_ratio_min = (msr32.lo >> 24) & 0x1f;
				min_ratio = bus_ratio_min * 10;
				if(currdiv) min_ratio = min_ratio + 5;
				
				if(XE || (p->CPU.Family == 0x0f)) bus_ratio_max = (msr32.hi >> (40-32)) & 0x1f;
				else bus_ratio_max = ((rdmsr64(MSR_IA32_PLATFORM_ID) >> 8) & 0x1f);
				/* On lower models, currcoef defines TSC freq */
				if (((p->CPU.Family == 0x06) && (p->CPU.Model < 0x0e)) && (p->CPU.Family != 0x0f)) bus_ratio_max = bus_ratio_min;
				// bad hack! Could force a value relying on kpstates, but I fail to see its benefits.
				if(bus_ratio_min == 0) bus_ratio_min = bus_ratio_max;
				
				if(p->CPU.Family == 0x0f)
				{
					getBoolForKey(kFixFSB, &fix_fsb, &bootInfo->bootConfig);
					if(fix_fsb)
					{
						msr = rdmsr64(MSR_EBC_FREQUENCY_ID);
						int bus = (msr >> 16) & 0x7;
						switch (bus)
						{
							case 0:
								fsbFrequency = 266666667;
								myfsb = 266;
								break;
							case 1:
								fsbFrequency = 133333333;
								myfsb = 133;
								break;
							case 3:
								fsbFrequency = 166666667;
								myfsb = 166;
								break;
							case 2:
							default:
								fsbFrequency = 200000000;
								myfsb = 200;
								break;
						}
						uint64_t minfsb = 183000000, maxfsb = 185000000;
						if (((fsbFrequency > minfsb) && (fsbFrequency < maxfsb)) || (!fsbFrequency))
						{
							fsbFrequency = 200000000;
							fsbad = true;
						}
					}
				}

				if(fix_fsb)
				{
					if (bus_ratio_max)
					{
						if (maxdiv) fsbi = ((tscFrequency * 2) / ((bus_ratio_max * 2) + 1));
						else fsbi = (tscFrequency / bus_ratio_max);
					}
					ratio_gn:
					if ((getValueForKey(kbusratio, &newratio, &len, &bootInfo->bootConfig)) && (len <= 4))
					{
						max_ratio = atoi(newratio);
						max_ratio = (max_ratio * 10);
						if (len >= 3) max_ratio = (max_ratio + 5);

						verbose("Bus-Ratio defaults: min=%d%s, max=%d%s\n", bus_ratio_min, currdiv ? ".5" : "", bus_ratio_max, maxdiv ? ".5" : "");
						if ((max_ratio >= min_ratio) && (max_ratio < 200))
						{
							cpuFrequency = (fsbFrequency * max_ratio) / 10;
							if (len >= 3) maxdiv = 1;
							else maxdiv = 0;
							verbose("Sticking with [FSB: %dMhz, Bus-Ratio: %s]\n", myfsb, newratio);
						}
						else
						{
							printf("Bus-Ratio: Lowest allowed = %d%s. ", bus_ratio_min, currdiv ? ".5" : "");
							goto ratio_vldt;
						}
					}
					else
					{
						ratio_vldt:
						if (maxdiv)
						{
							cpuFrequency = ((fsbFrequency * ((bus_ratio_max * 2) + 1)) / 2);
							max_ratio = (bus_ratio_max * 10) + 5;
						}
						else
						{
							cpuFrequency = (fsbFrequency * bus_ratio_max);
							max_ratio = bus_ratio_max * 10;
						}
						verbose("CPU: Sticking with: [FSB: %dMhz, Bus-Ratio: %d%s] %s\n", myfsb, bus_ratio_max, maxdiv ? ".5" : "", newratio ? "instead" : "");
					}
				}
				else
				{
					if (bus_ratio_max)
					{
						if (maxdiv)
						{
							fsbFrequency = ((tscFrequency * 2) / ((bus_ratio_max * 2) + 1));
							max_ratio = ((bus_ratio_max * 10) + 5);
						}
						else
						{
							fsbFrequency = (tscFrequency / bus_ratio_max);
							max_ratio = (bus_ratio_max * 10);
						}

						myfsb = (fsbFrequency / 1000000);
						if (getValueForKey(kbusratio, &newratio, &len, &bootInfo->bootConfig)) goto ratio_gn;
						else cpuFrequency = ((fsbFrequency * max_ratio) / 10);

						DBG("max: %d%s current: %d%s\n", bus_ratio_max, maxdiv ? ".5" : "", bus_ratio_min, currdiv ? ".5" : "");
					}
				}
				p->CPU.MaxRatio = max_ratio;
				p->CPU.MinRatio = min_ratio;
			}
		}

		// on-die sensor
		if (p->CPU.CPUID[CPUID_0][0] >= 0x6)
		{
			// Highest Basic Functions Number
			do_cpuid(6, p->CPU.CPUID[CPUID_81]);
			tms = bitfield(p->CPU.CPUID[CPUID_81][0], 0, 0);
			ida = bitfield(p->CPU.CPUID[CPUID_81][0], 1, 1);
			if(tms != 0)
			{
				int temp, utjmax;
				if (tjmax == 0) tjmax = 100;
				if((getIntForKey(kTjmax, &utjmax, &bootInfo->bootConfig)) && ((70 <= utjmax) && (utjmax <= 110))) tjmax = utjmax;
				msr = rdmsr64(MSR_THERMAL_STATUS);
				//if ((msr & 0x3) == 0x3)
				if (((msr >> 31) & 0x1) == 1)
				{
					temp = tjmax - ((msr >> 16) & 0x7F);
					verbose("CPU: Tjmax ~ %d°C 	           Temperature= ~ %d°C\n", tjmax, temp);
				}
				else temp = -1;
			}
			if(ida == 0)
			{
				verbose("CPU: Attempting to enable IDA      ");
				msr_t msr;
				msr = rdmsr(MSR_IA32_MISC_ENABLE);
				msr.hi |= (0 << (38-32));
				wrmsr(MSR_IA32_MISC_ENABLE, msr);
				delay(1);
				if(bitfield(p->CPU.CPUID[CPUID_81][0], 1, 1) == 0) verbose("Failed!\n");
				else verbose("Succeded!\n");
			}
			else verbose("CPU: IDA:                          Enabled!\n");
		}
	}
//#if 0
	else if(p->CPU.Vendor == 0x68747541 /* AMD */ && p->CPU.Family == 0x0f) // valv: work in progress
	{
		verbose("CPU: ");
		// valv: very experimental mobility check
		if (p->CPU.CPUID[0x80000000][0] >= 0x80000007)
		{
			uint32_t amo, const_tsc;
			do_cpuid(0x80000007, p->CPU.CPUID[CPUID_MAX]);
			amo = bitfield(p->CPU.CPUID[CPUID_MAX][0], 6, 6);
			const_tsc = bitfield(p->CPU.CPUID[CPUID_MAX][3], 8, 8);
			
			if (const_tsc != 0) verbose("Constant TSC!\n");
			if (amo == 1)
			{
				p->CPU.Features |= CPU_FEATURE_MOBILE;
				if (!strstr(p->CPU.BrandString, "obile")) verbose("Mobile ");
			}
		}
		//valv: 2nd attemp; just in case
		if (!platformCPUFeature(CPU_FEATURE_MOBILE))
		{
			if (strstr(p->CPU.BrandString, "obile"))
			{
				p->CPU.Features |= CPU_FEATURE_MOBILE;
			}
		}
		verbose("%s\n", p->CPU.BrandString);

		if(p->CPU.ExtFamily == 0x00 /* K8 */)
		{
			msr = rdmsr64(K8_FIDVID_STATUS);
			bus_ratio_max = (msr & 0x3f) / 2 + 4;
			currdiv = (msr & 0x01) * 2;
			if (bus_ratio_max)
			{
				if (currdiv)
				{
					fsbFrequency = ((tscFrequency * currdiv) / bus_ratio_max); // ?
					DBG("%d.%d\n", bus_ratio_max / currdiv, ((bus_ratio_max % currdiv) * 100) / currdiv);
				}
				else
				{
					fsbFrequency = (tscFrequency / bus_ratio_max);
					DBG("%d\n", bus_ratio_max);
				}
				//fsbFrequency = (tscFrequency / bus_ratio_max); // ?
				cpuFrequency = tscFrequency; // ?
			}
		}
		else if(p->CPU.ExtFamily >= 0x01 /* K10+ */)
		{
			msr = rdmsr64(K10_COFVID_STATUS);
			currdiv = (2 << ((msr >> 6) & 0x07)) / 2;
			msr = rdmsr64(AMD_10H_11H_CONFIG);
			if(p->CPU.ExtFamily == 0x01 /* K10 */)
			{
				bus_ratio_max = ((msr) & 0x3F);
				//currdiv = (((msr) >> 6) & 0x07);
				//cpuFrequency = 100 * (bus_ratio_max + 0x08) / (1 << currdiv);
			}
			else /* K11+ */
			{
				bus_ratio_max = ((msr) & 0x3F);
				//currdiv = (((msr) >> 6) & 0x07);
				//cpuFrequency = 100 * (bus_ratio_max + 0x10) / (1 << currdiv);
			}
			fsbFrequency = (tscFrequency / bus_ratio_max);
			cpuFrequency = tscFrequency;
		}
		
		p->CPU.MaxRatio = bus_ratio_max * 10;
		
		// valv: to be moved to acpi_patcher when ready
/*		msr_t amsr = rdmsr(K8_FIDVID_STATUS);
		uint8_t max_fid = (amsr.lo & 0x3F) >> 16;
		uint8_t min_fid = (amsr.lo & 0x3F) >> 8;
		uint8_t max_vid = (amsr.hi & 0x3F) >> 16;
		uint8_t min_vid = (amsr.hi & 0x3F) >> 8;
		verbose("AMD: max[fid: %d, vid: %d] min[fid: %d, vid: %d]\n", max_fid, max_vid, min_fid, min_vid);


			case 0x10:	// phenom
				msr = rdmsr64(AMD_10H_11H_CONFIG);
				bus_ratio_max = ((msr) & 0x3F);
				currdiv = (((msr) >> 6) & 0x07);
				cpuFrequency = 100 * (bus_ratio_max + 0x08) / (1 << currdiv);
				break;
			case 0x11:	// shangai
				msr = rdmsr64(AMD_10H_11H_CONFIG);
				bus_ratio_max = ((msr) & 0x3F);
				currdiv = (((msr) >> 6) & 0x07);
				cpuFrequency = 100 * (bus_ratio_max + 0x10) / (1 << currdiv);
				break;
		}
*/
	}
	else if(p->CPU.Vendor == 0x746e6543 /* CENTAUR */ && p->CPU.Family == 6) //valv: partial!
	{
		msr = rdmsr64(MSR_EBL_CR_POWERON);
		int bus = (msr >> 18) & 0x3;
		switch (bus)
		{
			case 1:
				fsbFrequency = 133333333;
				break;
			case 2:
				fsbFrequency = 200000000;
				break;
			case 3:
				fsbFrequency = 166666667;
				break;
			case 0:
			default:				
				fsbFrequency = 100000000;
				break;
		}
		msr_t msr;
		msr = rdmsr(MSR_IA32_PERF_STATUS);
		bus_ratio_min = (msr.lo >> 24) & 0x1f;
		min_ratio = bus_ratio_min * 10;
		bus_ratio_max = (msr.hi >> (40-32)) & 0x1f;
		max_ratio = bus_ratio_max * 10;
		cpuFrequency = ((fsbFrequency * max_ratio) / 10);
	}

	if (!fsbFrequency)
	{
		fsbFrequency = (DEFAULT_FSB * 1000);
		cpuFrequency = tscFrequency;
		DBG("0 ! using the default value for FSB !\n");
	}

//#endif

	p->CPU.MaxDiv = maxdiv;
	p->CPU.CurrDiv = currdiv;
	p->CPU.TSCFrequency = tscFrequency;
	p->CPU.FSBFrequency = fsbFrequency;
	p->CPU.CPUFrequency = cpuFrequency;
	p->CPU.ISerie = false;
	p->CPU.Turbo = false;

	if(!fsbad) p->CPU.FSBIFrequency = fsbFrequency;
	else p->CPU.FSBIFrequency = fsbi;

	if (platformCPUFeature(CPU_FEATURE_EST))
	{
		msr_t msr32;
		msr32 = rdmsr(MSR_IA32_MISC_ENABLE);
		if (!(rdmsr64(MSR_IA32_MISC_ENABLE) & (1 << 16)))
		{	//valv: we can also attempt to enable
			msr32.lo |= (1 << 16);
			// Lock till next reset!
			msr32.lo |= (1 << 20);
			wrmsr(MSR_IA32_MISC_ENABLE, msr32);
			delay(1);
			if(rdmsr64(MSR_IA32_MISC_ENABLE) & (1 << 16))
			{
				p->CPU.EST = 1;
				verbose("CPU: EIST Successfully Enabled!\n");
			}
			else
			{
				p->CPU.EST = 0;
				verbose("CPU: EIST couldn't be enabled!\n");
			}
		}

		else p->CPU.EST = 1;
	}
	
	if(core_i) p->CPU.ISerie = true;
		DBG("CPU: Vendor/Family/ExtFamily:      0x%x/0x%x/0x%x\n", p->CPU.Vendor, p->CPU.Family, p->CPU.ExtFamily);
		DBG("CPU: Model/ExtModel/Stepping:      0x%x/0x%x/0x%x\n", p->CPU.Model, p->CPU.ExtModel, p->CPU.Stepping);
		DBG("CPU: Multipliers x10:              max=%d, min=%d\n", p->CPU.MaxRatio, p->CPU.MinRatio);
	if(turbo)
	{
		DBG("Turbo Ratio:                       %d/%d/%d/%d\n", p->CPU.Tone, p->CPU.Ttwo, p->CPU.Tthr, p->CPU.Tfor);
		p->CPU.Turbo = true;
	}
		DBG("CPU: MaxDiv/CurrDiv:               0x%x/0x%x\n", p->CPU.MaxDiv, p->CPU.CurrDiv);
		DBG("CPU: TSCFreq:                      %dMHz\n", p->CPU.TSCFrequency / 1000000);
		DBG("CPU: CPUFreq:                      %dMHz\n", p->CPU.CPUFrequency / 1000000);
		DBG("CPU: FSBFreq:                      %dMHz\n", p->CPU.FSBFrequency / 1000000);
	if(did)
	{
		p->CPU.SLFM = did;
		DBG("CPU: SLFM:                         %d\n", p->CPU.SLFM);
	}
		if(platformCPUFeature(CPU_FEATURE_EST))
		DBG("CPU: Enhanced SpeedStep:           %d\n", p->CPU.EST);
		DBG("CPU: NoCores/NoThreads:            %d/%d\n", p->CPU.NoCores, p->CPU.NoThreads);
		DBG("CPU: Features:                     0x%08x\n", p->CPU.Features);
}
