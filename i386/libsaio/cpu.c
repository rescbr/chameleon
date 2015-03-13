/*
 * Copyright 2008 Islam Ahmed Zaid. All rights reserved. <azismed@gmail.com>
 * AsereBLN: 2009: cleanup and bugfix
 */

#include "libsaio.h"
#include "platform.h"
#include "cpu.h"
#include "bootstruct.h"
#include "boot.h"

#ifndef DEBUG_CPU
#define DEBUG_CPU 0
#endif

#if DEBUG_CPU
#define DBG(x...)		printf(x)
#else
#define DBG(x...)
#endif

/*
 * timeRDTSC()
 * This routine sets up PIT counter 2 to count down 1/20 of a second.
 * It pauses until the value is latched in the counter
 * and then reads the time stamp counter to return to the caller.
 */
static uint64_t timeRDTSC(void)
{
	int		attempts = 0;
	uint64_t    	latchTime;
	uint64_t	saveTime,intermediate;
	unsigned int	timerValue, lastValue;
	//boolean_t	int_enabled;
	/*
	 * Table of correction factors to account for
	 *	 - timer counter quantization errors, and
	 *	 - undercounts 0..5
	 */
#define SAMPLE_CLKS_EXACT	(((double) CLKNUM) / 20.0)
#define SAMPLE_CLKS_INT		((int) CLKNUM / 20)
#define SAMPLE_NSECS		(2000000000LL)
#define SAMPLE_MULTIPLIER	(((double)SAMPLE_NSECS)*SAMPLE_CLKS_EXACT)
#define ROUND64(x)		((uint64_t)((x) + 0.5))
	uint64_t	scale[6] = {
		ROUND64(SAMPLE_MULTIPLIER/(double)(SAMPLE_CLKS_INT-0)), 
		ROUND64(SAMPLE_MULTIPLIER/(double)(SAMPLE_CLKS_INT-1)), 
		ROUND64(SAMPLE_MULTIPLIER/(double)(SAMPLE_CLKS_INT-2)), 
		ROUND64(SAMPLE_MULTIPLIER/(double)(SAMPLE_CLKS_INT-3)), 
		ROUND64(SAMPLE_MULTIPLIER/(double)(SAMPLE_CLKS_INT-4)), 
		ROUND64(SAMPLE_MULTIPLIER/(double)(SAMPLE_CLKS_INT-5))
	};

	//int_enabled = ml_set_interrupts_enabled(false);

restart:
	if (attempts >= 9) // increase to up to 9 attempts.
	{
		// This will flash-reboot. TODO: Use tscPanic instead.
		printf("Timestamp counter calibation failed with %d attempts\n", attempts);
	}
	attempts++;
	enable_PIT2();		// turn on PIT2
	set_PIT2(0);		// reset timer 2 to be zero
	latchTime = rdtsc64();	// get the time stamp to time
	latchTime = get_PIT2(&timerValue) - latchTime; // time how long this takes
	set_PIT2(SAMPLE_CLKS_INT);	// set up the timer for (almost) 1/20th a second
	saveTime = rdtsc64();	// now time how long a 20th a second is...
	get_PIT2(&lastValue);
	get_PIT2(&lastValue);	// read twice, first value may be unreliable
	do {
		intermediate = get_PIT2(&timerValue);
		if (timerValue > lastValue)
		{
			// Timer wrapped
			set_PIT2(0);
			disable_PIT2();
			goto restart;
		}
		lastValue = timerValue;
	} while (timerValue > 5);
	printf("timerValue	  %d\n",timerValue);
	printf("intermediate  0x%016llX\n",intermediate);
	printf("saveTime	  0x%016llX\n",saveTime);
    
	intermediate -= saveTime;		// raw count for about 1/20 second
	intermediate *= scale[timerValue];	// rescale measured time spent
	intermediate /= SAMPLE_NSECS;	// so its exactly 1/20 a second
	intermediate += latchTime;		// add on our save fudge

	set_PIT2(0);			// reset timer 2 to be zero
	disable_PIT2();			// turn off PIT 2

	//ml_set_interrupts_enabled(int_enabled);
	return intermediate;
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
	 * counter 2. We run this loop 3 times to make sure the cache
	 * is hot and we take the minimum delta from all of the runs.
	 * That is to say that we're biased towards measuring the minimum
	 * number of TSC ticks that occur while waiting for the timer to
	 * expire. That theoretically helps avoid inconsistencies when
	 * running under a VM if the TSC is not virtualized and the host
	 * steals time.	 The TSC is normally virtualized for VMware.
	 */
	for(i = 0; i < 10; ++i)
	{
		enable_PIT2();
		set_PIT2_mode0(CALIBRATE_LATCH);
		tscStart = rdtsc64();
		pollCount = poll_PIT2_gate();
		tscEnd = rdtsc64();
		/* The poll loop must have run at least a few times for accuracy */
		if (pollCount <= 1)
		{
			continue;
		}
		/* The TSC must increment at LEAST once every millisecond.
		 * We should have waited exactly 30 msec so the TSC delta should
		 * be >= 30. Anything less and the processor is way too slow.
		 */
		if ((tscEnd - tscStart) <= CALIBRATE_TIME_MSEC)
		{
			continue;
		}
		// tscDelta = MIN(tscDelta, (tscEnd - tscStart))
		if ( (tscEnd - tscStart) < tscDelta )
		{
			tscDelta = tscEnd - tscStart;
		}
	}
	/* tscDelta is now the least number of TSC ticks the processor made in
	 * a timespan of 0.03 s (e.g. 30 milliseconds)
	 * Linux thus divides by 30 which gives the answer in kiloHertz because
	 * 1 / ms = kHz. But we're xnu and most of the rest of the code uses
	 * Hz so we need to convert our milliseconds to seconds. Since we're
	 * dividing by the milliseconds, we simply multiply by 1000.
	 */

	/* Unlike linux, we're not limited to 32-bit, but we do need to take care
	 * that we're going to multiply by 1000 first so we do need at least some
	 * arithmetic headroom. For now, 32-bit should be enough.
	 * Also unlike Linux, our compiler can do 64-bit integer arithmetic.
	 */
	if (tscDelta > (1ULL<<32))
	{
		retval = 0;
	}
	else
	{
		retval = tscDelta * 1000 / 30;
	}
	disable_PIT2();
	return retval;
}

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
		{
			continue;
		}
		/* The TSC must increment at LEAST once every millisecond.
		 * We should have waited exactly 30 msec so the APERF delta should
		 * be >= 30. Anything less and the processor is way too slow.
		 */
		if ((aperfEnd - aperfStart) <= CALIBRATE_TIME_MSEC)
		{
			continue;
		}
		// tscDelta = MIN(tscDelta, (tscEnd - tscStart))
		if ( (aperfEnd - aperfStart) < aperfDelta )
		{
			aperfDelta = aperfEnd - aperfStart;
		}
	}
	/* mperfDelta is now the least number of MPERF ticks the processor made in
	 * a timespan of 0.03 s (e.g. 30 milliseconds)
	 */

	if (aperfDelta > (1ULL<<32))
	{
		retval = 0;
	}
	else
	{
		retval = aperfDelta * 1000 / 30;
	}
	disable_PIT2();
	return retval;
}

/*
 * Calculates the FSB and CPU frequencies using specific MSRs for each CPU
 * - multi. is read from a specific MSR. In the case of Intel, there is:
 *	   a max multi. (used to calculate the FSB freq.),
 *	   and a current multi. (used to calculate the CPU freq.)
 * - fsbFrequency = tscFrequency / multi
 * - cpuFrequency = fsbFrequency * multi
 */
void scan_cpu(PlatformInfo_t *p)
{
	uint64_t	tscFrequency		= 0;
	uint64_t	fsbFrequency		= 0;
	uint64_t	cpuFrequency		= 0;
	uint64_t	msr			= 0;
	uint64_t	flex_ratio		= 0;

	uint32_t	max_ratio		= 0;
	uint32_t	min_ratio		= 0;
	uint32_t	reg[4]; //		= {0, 0, 0, 0};
	uint32_t	cores_per_package	= 0;
	uint32_t	logical_per_package	= 1;
	uint32_t	threads_per_core	= 1;

	uint8_t		bus_ratio_max		= 0;
	uint8_t		bus_ratio_min		= 0;
	uint8_t		currdiv			= 0;
	uint8_t		currcoef		= 0;
	uint8_t		maxdiv			= 0;
	uint8_t		maxcoef			= 0;

	const char	*newratio;
	char		str[128];
	char		*s			= 0;

	int		len			= 0;
	int		myfsb			= 0;
	int		i			= 0;

	/* get cpuid values */
	do_cpuid(0x00000000, p->CPU.CPUID[CPUID_0]); // MaxFn, Vendor
	p->CPU.Vendor = p->CPU.CPUID[CPUID_0][ebx];

	do_cpuid(0x00000001, p->CPU.CPUID[CPUID_1]); // Signature, stepping, features

	if ((p->CPU.Vendor == CPUID_VENDOR_INTEL) && ((bit(28) & p->CPU.CPUID[CPUID_1][edx]) != 0)) // Intel && HTT/Multicore
	{
		logical_per_package = bitfield(p->CPU.CPUID[CPUID_1][ebx], 23, 16);
	}

	do_cpuid(0x00000002, p->CPU.CPUID[CPUID_2]); // TLB/Cache/Prefetch

	do_cpuid(0x00000003, p->CPU.CPUID[CPUID_3]); // S/N

	/* Based on Apple's XNU cpuid.c - Deterministic cache parameters */
	if ((p->CPU.CPUID[CPUID_0][eax] > 3) && (p->CPU.CPUID[CPUID_0][eax] < 0x80000000))
	{
		for (i = 0; i < 0xFF; i++) // safe loop
		{
			do_cpuid2(0x00000004, i, reg); // AX=4: Fn, CX=i: cache index
			if (bitfield(reg[eax], 4, 0) == 0)
			{
				break;
			}
			//cores_per_package = bitfield(reg[eax], 31, 26) + 1;
		}
	}

	do_cpuid2(0x00000004, 0, p->CPU.CPUID[CPUID_4]);

	if (i > 0)
	{
		cores_per_package = bitfield(p->CPU.CPUID[CPUID_4][eax], 31, 26) + 1; // i = cache index
		threads_per_core = bitfield(p->CPU.CPUID[CPUID_4][eax], 25, 14) + 1;
	}

	if (cores_per_package == 0)
	{
		cores_per_package = 1;
	}

	if (p->CPU.CPUID[CPUID_0][0] >= 0x5)	// Monitor/Mwait
	{
		do_cpuid(5,  p->CPU.CPUID[CPUID_5]);
	}

	if (p->CPU.CPUID[CPUID_0][0] >= 6)	// Thermal/Power
	{
		do_cpuid(6, p->CPU.CPUID[CPUID_6]);
	}

	do_cpuid(0x80000000, p->CPU.CPUID[CPUID_80]);

	if ((p->CPU.CPUID[CPUID_80][0] & 0x0000000f) >= 8)
	{
		do_cpuid(0x80000008, p->CPU.CPUID[CPUID_88]);
		do_cpuid(0x80000001, p->CPU.CPUID[CPUID_81]);
	}
	else if ((p->CPU.CPUID[CPUID_80][0] & 0x0000000f) >= 1)
	{
		do_cpuid(0x80000001, p->CPU.CPUID[CPUID_81]);
	}

/*  http://www.flounder.com/cpuid_explorer2.htm
    EAX (Intel):
    31    28 27            20 19    16 1514 1312 11     8 7      4 3      0
    +--------+----------------+--------+----+----+--------+--------+--------+
    |########|Extended family |Extmodel|####|type|familyid|  model |stepping|
    +--------+----------------+--------+----+----+--------+--------+--------+

    EAX (AMD):
    31    28 27            20 19    16 1514 1312 11     8 7      4 3      0
    +--------+----------------+--------+----+----+--------+--------+--------+
    |########|Extended family |Extmodel|####|####|familyid|  model |stepping|
    +--------+----------------+--------+----+----+--------+--------+--------+
*/

	p->CPU.Vendor		= p->CPU.CPUID[CPUID_0][1];
	p->CPU.Signature	= p->CPU.CPUID[CPUID_1][0];
	p->CPU.Stepping		= (uint8_t)bitfield(p->CPU.CPUID[CPUID_1][0], 3, 0);	// stepping = cpu_feat_eax & 0xF;
	p->CPU.Model		= (uint8_t)bitfield(p->CPU.CPUID[CPUID_1][0], 7, 4);	// model = (cpu_feat_eax >> 4) & 0xF;
	p->CPU.Family		= (uint8_t)bitfield(p->CPU.CPUID[CPUID_1][0], 11, 8);	// family = (cpu_feat_eax >> 8) & 0xF;
	//p->CPU.Type		= (uint8_t)bitfield(p->CPU.CPUID[CPUID_1][0], 13, 12);	// type = (cpu_feat_eax >> 12) & 0x3;
	p->CPU.ExtModel		= (uint8_t)bitfield(p->CPU.CPUID[CPUID_1][0], 19, 16);	// ext_model = (cpu_feat_eax >> 16) & 0xF;
	p->CPU.ExtFamily	= (uint8_t)bitfield(p->CPU.CPUID[CPUID_1][0], 27, 20);	// ext_family = (cpu_feat_eax >> 20) & 0xFF;

	p->CPU.Model += (p->CPU.ExtModel << 4);

	/* get BrandString (if supported) */
	/* Copyright: from Apple's XNU cpuid.c */
	if (p->CPU.CPUID[CPUID_80][0] > 0x80000004)
	{
		bzero(str, 128);
		/*
		 * The BrandString 48 bytes (max), guaranteed to
		 * be NULL terminated.
		 */
		do_cpuid(0x80000002, reg);
		memcpy(&str[0], (char *)reg, 16);
		do_cpuid(0x80000003, reg);
		memcpy(&str[16], (char *)reg, 16);
		do_cpuid(0x80000004, reg);
		memcpy(&str[32], (char *)reg, 16);
		for (s = str; *s != '\0'; s++)
		{
			if (*s != ' ')
			{
				break;
			}
		}
		strlcpy(p->CPU.BrandString, s, 48);

		if (!strncmp(p->CPU.BrandString, CPU_STRING_UNKNOWN, MIN(sizeof(p->CPU.BrandString), (unsigned)strlen(CPU_STRING_UNKNOWN) + 1)))
		{
			/*
			 * This string means we have a firmware-programmable brand string,
			 * and the firmware couldn't figure out what sort of CPU we have.
			 */
			p->CPU.BrandString[0] = '\0';
		}
		p->CPU.BrandString[47] = '\0';
//		DBG("Brandstring = %s\n", p->CPU.BrandString);
	}

	/*
	 * Find the number of enabled cores and threads
	 * (which determines whether SMT/Hyperthreading is active).
	 */
	switch (p->CPU.Vendor)
	{
		case CPUID_VENDOR_INTEL:
			switch (p->CPU.Model)
			{
				case CPUID_MODEL_NEHALEM:
				case CPUID_MODEL_FIELDS:
				case CPUID_MODEL_DALES:
				case CPUID_MODEL_NEHALEM_EX:
				case CPUID_MODEL_JAKETOWN:
				case CPUID_MODEL_SANDYBRIDGE:
				case CPUID_MODEL_IVYBRIDGE:

				case CPUID_MODEL_HASWELL:
				case CPUID_MODEL_HASWELL_SVR:
				//case CPUID_MODEL_HASWELL_H:
				case CPUID_MODEL_HASWELL_ULT:
				case CPUID_MODEL_CRYSTALWELL:
				//case CPUID_MODEL_:
					msr = rdmsr64(MSR_CORE_THREAD_COUNT);
					p->CPU.NoCores		= (uint32_t)bitfield((uint32_t)msr, 31, 16);
					p->CPU.NoThreads	= (uint32_t)bitfield((uint32_t)msr, 15,  0);
					break;

				case CPUID_MODEL_DALES_32NM:
				case CPUID_MODEL_WESTMERE:
				case CPUID_MODEL_WESTMERE_EX:
					msr = rdmsr64(MSR_CORE_THREAD_COUNT);
					p->CPU.NoCores		= (uint32_t)bitfield((uint32_t)msr, 19, 16);
					p->CPU.NoThreads	= (uint32_t)bitfield((uint32_t)msr, 15,  0);
					break;
			}

			if (p->CPU.NoCores == 0)
			{
				p->CPU.NoCores		= cores_per_package;
				p->CPU.NoThreads	= logical_per_package;
			}
			break;

		case CPUID_VENDOR_AMD:
			p->CPU.NoCores		= (uint32_t)bitfield(p->CPU.CPUID[CPUID_88][2], 7, 0) + 1;
			p->CPU.NoThreads	= (uint32_t)bitfield(p->CPU.CPUID[CPUID_1][1], 23, 16);
			if (p->CPU.NoCores == 0)
			{
				p->CPU.NoCores = 1;
			}

			if (p->CPU.NoThreads < p->CPU.NoCores)
			{
				p->CPU.NoThreads = p->CPU.NoCores;
			}

			break;

		default:
			stop("Unsupported CPU detected! System halted.");
	}

	//workaround for N270. I don't know why it detected wrong
	// MSR is *NOT* available on the Intel Atom CPU
	if ((p->CPU.Model == CPUID_MODEL_ATOM) && (strstr(p->CPU.BrandString, "270")))
	{
		p->CPU.NoCores		= 1;
		p->CPU.NoThreads	= 2;
	}

	/* setup features */
	if ((bit(23) & p->CPU.CPUID[CPUID_1][3]) != 0)
	{
		p->CPU.Features |= CPU_FEATURE_MMX;
	}

	if ((bit(25) & p->CPU.CPUID[CPUID_1][3]) != 0)
	{
		p->CPU.Features |= CPU_FEATURE_SSE;
	}

	if ((bit(26) & p->CPU.CPUID[CPUID_1][3]) != 0)
	{
		p->CPU.Features |= CPU_FEATURE_SSE2;
	}

	if ((bit(0) & p->CPU.CPUID[CPUID_1][2]) != 0)
	{
		p->CPU.Features |= CPU_FEATURE_SSE3;
	}

	if ((bit(19) & p->CPU.CPUID[CPUID_1][2]) != 0)
	{
		p->CPU.Features |= CPU_FEATURE_SSE41;
	}

	if ((bit(20) & p->CPU.CPUID[CPUID_1][2]) != 0)
	{
		p->CPU.Features |= CPU_FEATURE_SSE42;
	}

	if ((bit(29) & p->CPU.CPUID[CPUID_81][3]) != 0)
	{
		p->CPU.Features |= CPU_FEATURE_EM64T;
	}

	if ((bit(5) & p->CPU.CPUID[CPUID_1][3]) != 0)
	{
		p->CPU.Features |= CPU_FEATURE_MSR;
	}

	if ((p->CPU.Vendor == CPUID_VENDOR_INTEL) && (p->CPU.NoThreads > p->CPU.NoCores))
	{
		p->CPU.Features |= CPU_FEATURE_HTT;
	}

	tscFrequency = measure_tsc_frequency();
	DBG("cpu freq classic = 0x%016llx\n", tscFrequency);
	// if usual method failed
	if ( tscFrequency < 1000 )	//TEST
	{
		tscFrequency = timeRDTSC() * 20;//measure_tsc_frequency();
		// DBG("cpu freq timeRDTSC = 0x%016llx\n", tscFrequency);
	}
	else
	{
		// DBG("cpu freq timeRDTSC = 0x%016llxn", timeRDTSC() * 20);
	}

	fsbFrequency = 0;
	cpuFrequency = 0;

	if (p->CPU.Vendor == CPUID_VENDOR_INTEL && ((p->CPU.Family == 0x06 && p->CPU.Model >= 0x0c) || (p->CPU.Family == 0x0f && p->CPU.Model >= 0x03)))
	{
		int intelCPU = p->CPU.Model;
		if (p->CPU.Family == 0x06)
		{
			/* Nehalem CPU model */
			switch (p->CPU.Model)
			{
				case CPUID_MODEL_NEHALEM:
				case CPUID_MODEL_FIELDS:
				case CPUID_MODEL_DALES:
				case CPUID_MODEL_DALES_32NM:
				case CPUID_MODEL_WESTMERE:
				case CPUID_MODEL_NEHALEM_EX:
				case CPUID_MODEL_WESTMERE_EX:
/* --------------------------------------------------------- */
				case CPUID_MODEL_SANDYBRIDGE:
				case CPUID_MODEL_JAKETOWN:
				case CPUID_MODEL_IVYBRIDGE_XEON:
				case CPUID_MODEL_IVYBRIDGE:
				case CPUID_MODEL_HASWELL:
				case CPUID_MODEL_HASWELL_SVR:

				case CPUID_MODEL_HASWELL_ULT:
				case CPUID_MODEL_CRYSTALWELL:
/* --------------------------------------------------------- */
					msr = rdmsr64(MSR_PLATFORM_INFO);
					DBG("msr(%d): platform_info %08x\n", __LINE__, bitfield(msr, 31, 0));
					bus_ratio_max = bitfield(msr, 15, 8);
					bus_ratio_min = bitfield(msr, 47, 40); //valv: not sure about this one (Remarq.1)
					msr = rdmsr64(MSR_FLEX_RATIO);
					DBG("msr(%d): flex_ratio %08x\n", __LINE__, bitfield(msr, 31, 0));
					if (bitfield(msr, 16, 16))
					{
						flex_ratio = bitfield(msr, 15, 8);
						// bcc9: at least on the gigabyte h67ma-ud2h,
						// where the cpu multipler can't be changed to
						// allow overclocking, the flex_ratio msr has unexpected (to OSX)
						// contents.	These contents cause mach_kernel to
						// fail to compute the bus ratio correctly, instead
						// causing the system to crash since tscGranularity
						// is inadvertently set to 0.

						if (flex_ratio == 0)
						{
							// Clear bit 16 (evidently the presence bit)
							wrmsr64(MSR_FLEX_RATIO, (msr & 0xFFFFFFFFFFFEFFFFULL));
							msr = rdmsr64(MSR_FLEX_RATIO);
							DBG("CPU: Unusable flex ratio detected. Patched MSR now %08x\n", bitfield(msr, 31, 0));
						}
						else
						{
							if (bus_ratio_max > flex_ratio)
							{
								bus_ratio_max = flex_ratio;
							}
						}
					}

					if (bus_ratio_max)
					{
						fsbFrequency = (tscFrequency / bus_ratio_max);
					}

					//valv: Turbo Ratio Limit
					if ((intelCPU != 0x2e) && (intelCPU != 0x2f))
					{
						msr = rdmsr64(MSR_TURBO_RATIO_LIMIT);

						cpuFrequency = bus_ratio_max * fsbFrequency;
						max_ratio = bus_ratio_max * 10;
					}
					else
					{
						cpuFrequency = tscFrequency;
					}
					if ((getValueForKey(kbusratio, &newratio, &len, &bootInfo->chameleonConfig)) && (len <= 4))
					{
						max_ratio = atoi(newratio);
						max_ratio = (max_ratio * 10);
						if (len >= 3)
						{
							max_ratio = (max_ratio + 5);
						}

						verbose("Bus-Ratio: min=%d, max=%s\n", bus_ratio_min, newratio);

						// extreme overclockers may love 320 ;)
						if ((max_ratio >= min_ratio) && (max_ratio <= 320))
						{
							cpuFrequency = (fsbFrequency * max_ratio) / 10;
							if (len >= 3)
							{
								maxdiv = 1;
							}
							else
							{
								maxdiv = 0;
							}
						}
						else
						{
							max_ratio = (bus_ratio_max * 10);
						}
					}
					//valv: to be uncommented if Remarq.1 didn't stick
					//if (bus_ratio_max > 0) bus_ratio = flex_ratio;
					p->CPU.MaxRatio = max_ratio;
					p->CPU.MinRatio = min_ratio;

				myfsb = fsbFrequency / 1000000;
				verbose("Sticking with [BCLK: %dMhz, Bus-Ratio: %d]\n", myfsb, max_ratio/10);  // Bungo: fixed wrong Bus-Ratio readout
				currcoef = bus_ratio_max;

				break;

			default:
				msr = rdmsr64(MSR_IA32_PERF_STATUS);
				DBG("msr(%d): ia32_perf_stat 0x%08x\n", __LINE__, bitfield(msr, 31, 0));
				currcoef = bitfield(msr, 12, 8);  // Bungo: reverted to 2263 state because of wrong old CPUs freq. calculating
				// Non-integer bus ratio for the max-multi
				maxdiv = bitfield(msr, 46, 46);
				// Non-integer bus ratio for the current-multi (undocumented)
				currdiv = bitfield(msr, 14, 14);

				// This will always be model >= 3
				if ((p->CPU.Family == 0x06 && p->CPU.Model >= 0x0e) || (p->CPU.Family == 0x0f))
				{
					/* On these models, maxcoef defines TSC freq */
					maxcoef = bitfield(msr, 44, 40);
				}
				else
				{
					// On lower models, currcoef defines TSC freq
					// XXX
					maxcoef = currcoef;
				}

				if (!currcoef)
				{
					currcoef = maxcoef;
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

					DBG("max: %d%s current: %d%s\n", maxcoef, maxdiv ? ".5" : "",currcoef, currdiv ? ".5" : "");
				}
				break;
			}
		}
		// Mobile CPU
		if (rdmsr64(MSR_IA32_PLATFORM_ID) & (1<<28))
		{
			p->CPU.Features |= CPU_FEATURE_MOBILE;
		}
	}
	else if ((p->CPU.Vendor == CPUID_VENDOR_AMD) && (p->CPU.Family == 0x0f))
	{
		switch(p->CPU.ExtFamily)
		{
			case 0x00: //* K8 *//
				msr = rdmsr64(K8_FIDVID_STATUS);
				maxcoef = bitfield(msr, 21, 16) / 2 + 4;
				currcoef = bitfield(msr, 5, 0) / 2 + 4;
				break;

			case 0x01: //* K10 *//
				msr = rdmsr64(K10_COFVID_STATUS);
				do_cpuid2(0x00000006, 0, p->CPU.CPUID[CPUID_6]);
				// EffFreq: effective frequency interface
				if (bitfield(p->CPU.CPUID[CPUID_6][2], 0, 0) == 1)
				{
					//uint64_t mperf = measure_mperf_frequency();
					uint64_t aperf = measure_aperf_frequency();
					cpuFrequency = aperf;
				}
				// NOTE: tsc runs at the maccoeff (non turbo)
				//			*not* at the turbo frequency.
				maxcoef	 = bitfield(msr, 54, 49) / 2 + 4;
				currcoef = bitfield(msr, 5, 0) + 0x10;
				currdiv = 2 << bitfield(msr, 8, 6);

				break;

			case 0x05: //* K14 *//
				msr = rdmsr64(K10_COFVID_STATUS);
				currcoef  = (bitfield(msr, 54, 49) + 0x10) << 2;
				currdiv = (bitfield(msr, 8, 4) + 1) << 2;
				currdiv += bitfield(msr, 3, 0);

				break;

			case 0x02: //* K11 *//
				// not implimented
				break;
		}

		if (maxcoef)
		{
			if (currdiv)
			{
				if (!currcoef)
				{
					currcoef = maxcoef;
				}

				if (!cpuFrequency)
				{
					fsbFrequency = ((tscFrequency * currdiv) / currcoef);
				}
				else
				{
					fsbFrequency = ((cpuFrequency * currdiv) / currcoef);
				}
				DBG("%d.%d\n", currcoef / currdiv, ((currcoef % currdiv) * 100) / currdiv);
			}
			else
			{
				if (!cpuFrequency)
				{
					fsbFrequency = (tscFrequency / maxcoef);
				}
				else
				{
					fsbFrequency = (cpuFrequency / maxcoef);
				}
				DBG("%d\n", currcoef);
			}
		}
		else if (currcoef)
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
		}
		if (!cpuFrequency)
		{
			cpuFrequency = tscFrequency;
		}
	}

#if 0
	if (!fsbFrequency)
	{
		fsbFrequency = (DEFAULT_FSB * 1000);
		DBG("CPU: fsbFrequency = 0! using the default value for FSB!\n");
		cpuFrequency = tscFrequency;
	}

	DBG("cpu freq = 0x%016llxn", timeRDTSC() * 20);

#endif

	p->CPU.MaxCoef = maxcoef;
	p->CPU.MaxDiv = maxdiv;
	p->CPU.CurrCoef = currcoef;
	p->CPU.CurrDiv = currdiv;
	p->CPU.TSCFrequency = tscFrequency;
	p->CPU.FSBFrequency = fsbFrequency;
	p->CPU.CPUFrequency = cpuFrequency;

	// keep formatted with spaces instead of tabs
	DBG("\n------------------------------\n");
   	DBG("\tCPU INFO\n");
	DBG("------------------------------\n");

	DBG("CPUID Raw Values:\n");
	for (i = 0; i < CPUID_MAX; i++)
	{
		DBG("%02d:  %08X-%08X-%08X-%08X\n", i, p->CPU.CPUID[i][eax], p->CPU.CPUID[i][ebx], p->CPU.CPUID[i][ecx], p->CPU.CPUID[i][edx]);
	}
	DBG("\n");
	DBG("Brand String:            %s\n",		p->CPU.BrandString);		// Processor name (BIOS)
	DBG("Vendor:                  0x%X\n",		p->CPU.Vendor);			// Vendor ex: GenuineIntel
	DBG("Family:                  0x%X\n",		p->CPU.Family);			// Family ex: 6 (06h)
	DBG("ExtFamily:               0x%X\n",		p->CPU.ExtFamily);
	DBG("Signature:               0x%08X\n",	p->CPU.Signature);		// CPUID signature
	/*switch (p->CPU.Type) {
		case PT_OEM:
			DBG("Processor type:          Intel Original OEM Processor\n");
			break;
		case PT_OD:
			DBG("Processor type:          Intel Over Drive Processor\n");
			break;
		case PT_DUAL:
			DBG("Processor type:          Intel Dual Processor\n");
			break;
		case PT_RES:
			DBG("Processor type:          Intel Reserved\n");
			break;
		default:
			break;
	}*/
	DBG("Model:                   0x%X\n",		p->CPU.Model);			// Model ex: 37 (025h)
	DBG("ExtModel:                0x%X\n",		p->CPU.ExtModel);
	DBG("Stepping:                0x%X\n",		p->CPU.Stepping);		// Stepping ex: 5 (05h)
	DBG("MaxCoef:                 %d\n",		p->CPU.MaxCoef);
	DBG("CurrCoef:                %d\n",		p->CPU.CurrCoef);
	DBG("MaxDiv:                  %d\n",		p->CPU.MaxDiv);
	DBG("CurrDiv:                 %d\n",		p->CPU.CurrDiv);
	DBG("TSCFreq:                 %dMHz\n",		p->CPU.TSCFrequency / 1000000);
	DBG("FSBFreq:                 %dMHz\n",		p->CPU.FSBFrequency / 1000000);
	DBG("CPUFreq:                 %dMHz\n",		p->CPU.CPUFrequency / 1000000);
	DBG("Cores:                   %d\n",		p->CPU.NoCores);		// Cores
	DBG("Logical processor:       %d\n",		p->CPU.NoThreads);		// Logical procesor
	DBG("Features:                0x%08x\n",	p->CPU.Features);

	DBG("\n---------------------------------------------\n");
#if DEBUG_CPU
	pause();
#endif
}
