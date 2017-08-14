/*
 * Copyright 2008 Islam Ahmed Zaid. All rights reserved. <azismed@gmail.com>
 * AsereBLN: 2009: cleanup and bugfix
 * Bronya:   2015 Improve AMD support, cleanup and bugfix
 */

#include "config.h"
#include "libsaio.h"
#include "platform.h"
#include "cpu.h"
#include "bootstruct.h"
#include "boot.h"

#if DEBUG_CPU
	#define DBG(x...)		printf(x)
#else
	#define DBG(x...)
#endif

#define UI_CPUFREQ_ROUNDING_FACTOR	10000000

clock_frequency_info_t gPEClockFrequencyInfo;

//static __unused uint64_t rdtsc32(void)
//{
//	unsigned int lo,hi;
//	__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
//	return ((uint64_t)hi << 32) | lo;
//}

uint64_t getCycles(void)
{
#if defined(__ARM_ARCH_7A__)
    uint32_t r;
    asm volatile("mrc p15, 0, %0, c9, c13, 0\t\n" : "=r" (r)); /* Read PMCCNTR       */
    return ((uint64_t)r) << 6;                                 /* 1 tick = 64 clocks */
#elif defined(__x86_64__)
    unsigned a, d;
    asm volatile("rdtsc" : "=a" (a), "=d" (d));
    return ((uint64_t)a) | (((uint64_t)d) << 32);
#elif defined(__i386__)
    uint64_t ret;
    asm volatile("rdtsc": "=A" (ret));
    return ret;
#else
    return 0;
#endif
}

/*
 * timeRDTSC()
 * This routine sets up PIT counter 2 to count down 1/20 of a second.
 * It pauses until the value is latched in the counter
 * and then reads the time stamp counter to return to the caller.
 */
static uint64_t timeRDTSC(void)
{
	int		attempts = 0;
	uint32_t    	latchTime;
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
	if (attempts >= 3) // increase to up to 9 attempts.
	{
		// This will flash-reboot. TODO: Use tscPanic instead.
		//printf("Timestamp counter calibation failed with %d attempts\n", attempts);
	}
	attempts++;
	enable_PIT2();		// turn on PIT2
	set_PIT2(0);		// reset timer 2 to be zero
	latchTime = getCycles();	// get the time stamp to time
	latchTime = get_PIT2(&timerValue) - latchTime; // time how long this takes
	set_PIT2(SAMPLE_CLKS_INT);	// set up the timer for (almost) 1/20th a second
	saveTime = getCycles();	// now time how long a 20th a second is...
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
	//printf("timerValue	  %d\n",timerValue);
	//printf("intermediate  0x%016llX\n",intermediate);
	//printf("saveTime	  0x%016llX\n",saveTime);
    
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
static uint64_t __unused measure_tsc_frequency(void)
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
		tscStart = getCycles();
		pollCount = poll_PIT2_gate();
		tscEnd = getCycles();
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

static uint64_t	rtc_set_cyc_per_sec(uint64_t cycles);
#define RTC_FAST_DENOM	0xFFFFFFFF

inline static uint32_t
create_mul_quant_GHZ(int shift, uint32_t quant)
{
	return (uint32_t)((((uint64_t)NSEC_PER_SEC/20) << shift) / quant);
}

struct	{
	mach_timespec_t			calend_offset;
	boolean_t			calend_is_set;

	int64_t				calend_adjtotal;
	int32_t				calend_adjdelta;

	uint32_t			boottime;

	mach_timebase_info_data_t	timebase_const;

	decl_simple_lock_data(,lock)	/* real-time clock device lock */
} rtclock;

uint32_t		rtc_quant_shift;	/* clock to nanos right shift */
uint32_t		rtc_quant_scale;	/* clock to nanos multiplier */
uint64_t		rtc_cyc_per_sec;	/* processor cycles per sec */
uint64_t		rtc_cycle_count;	/* clocks in 1/20th second */

static uint64_t rtc_set_cyc_per_sec(uint64_t cycles)
{

	if (cycles > (NSEC_PER_SEC/20))
	{
		// we can use just a "fast" multiply to get nanos
		rtc_quant_shift = 32;
		rtc_quant_scale = create_mul_quant_GHZ(rtc_quant_shift, (uint32_t)cycles);
		rtclock.timebase_const.numer = rtc_quant_scale; // timeRDTSC is 1/20
		rtclock.timebase_const.denom = (uint32_t)RTC_FAST_DENOM;
	}
	else
	{
		rtc_quant_shift = 26;
		rtc_quant_scale = create_mul_quant_GHZ(rtc_quant_shift, (uint32_t)cycles);
		rtclock.timebase_const.numer = NSEC_PER_SEC/20; // timeRDTSC is 1/20
		rtclock.timebase_const.denom = (uint32_t)cycles;
	}
	rtc_cyc_per_sec = cycles*20;	// multiply it by 20 and we are done..
	// BUT we also want to calculate...

	cycles = ((rtc_cyc_per_sec + (UI_CPUFREQ_ROUNDING_FACTOR/2))
              / UI_CPUFREQ_ROUNDING_FACTOR)
	* UI_CPUFREQ_ROUNDING_FACTOR;

	/*
	 * Set current measured speed.
	 */
	if (cycles >= 0x100000000ULL)
	{
		gPEClockFrequencyInfo.cpu_clock_rate_hz = 0xFFFFFFFFUL;
	}
	else
	{
		gPEClockFrequencyInfo.cpu_clock_rate_hz = (unsigned long)cycles;
	}
	gPEClockFrequencyInfo.cpu_frequency_hz = cycles;

	//printf("[RTCLOCK_1] frequency %llu (%llu) %llu\n", cycles, rtc_cyc_per_sec,timeRDTSC() * 20);
	return(rtc_cyc_per_sec);
}

// Bronya C1E fix
static void post_startup_cpu_fixups(void)
{
	/*
	 * Some AMD processors support C1E state. Entering this state will
	 * cause the local APIC timer to stop, which we can't deal with at
	 * this time.
	 */

	uint64_t reg;
	verbose("\tLooking to disable C1E if is already enabled by the BIOS:\n");
	reg = rdmsr64(MSR_AMD_INT_PENDING_CMP_HALT);
	/* Disable C1E state if it is enabled by the BIOS */
	if ((reg >> AMD_ACTONCMPHALT_SHIFT) & AMD_ACTONCMPHALT_MASK)
	{
		reg &= ~(AMD_ACTONCMPHALT_MASK << AMD_ACTONCMPHALT_SHIFT);
		wrmsr64(MSR_AMD_INT_PENDING_CMP_HALT, reg);
		verbose("\tC1E disabled!\n");
	}
}

/*
 * Large memcpy() into MMIO space can take longer than 1 clock tick (55ms).
 *   The timer interrupt must remain responsive when updating VRAM so
 *   as not to miss timer interrupts during countdown().
 *
 * If interrupts are enabled, use normal memcpy.
 *
 * If interrupts are disabled, breaks memcpy down
 *   into 128K chunks, times itself and makes a bios
 *   real-mode call every 25 msec in order to service
 *   pending interrupts.
 *
 * -- zenith432, May 22nd, 2016
 */
void *memcpy_interruptible(void *dst, const void *src, size_t len)
{
	uint64_t tscFreq, lastTsc;
	uint32_t eflags, threshold;
	ptrdiff_t offset;
	const size_t chunk = 131072U;	// 128K

	if (len <= chunk)
	{
		/*
		 * Short memcpy - use normal.
		 */
		return memcpy(dst, src, len);
	}

	__asm__ volatile("pushfl; popl %0" : "=r"(eflags));
	if (eflags & 0x200U)
	{
		/*
		 * Interrupts are enabled - use normal memcpy.
		 */
		return memcpy(dst, src, len);
	}

	tscFreq = Platform.CPU.TSCFrequency;
	if ((uint32_t) (tscFreq >> 32))
	{
		/*
		 * If TSC Frequency >= 2 ** 32, use a default time threshold.
		 */
		threshold = (~0U) / 40U;
	}
	else if (!(uint32_t) tscFreq)
	{
		/*
		 * If early on and TSC Frequency hasn't been estimated yet,
		 *   use normal memcpy.
		 */
		return memcpy(dst, src, len);
	}
	else
	{
		threshold = ((uint32_t) tscFreq) / 40U;
	}

	/*
	 * Do the work
	 */
	offset = 0;
	lastTsc = getCycles();
	do
	{
		(void) memcpy((char*) dst + offset, (const char*) src + offset, chunk);
		offset += (ptrdiff_t) chunk;
		len -= chunk;
		if ((getCycles() - lastTsc) < threshold)
		{
			continue;
		}
		(void) readKeyboardStatus();	// visit real-mode
		lastTsc = getCycles();
	}
	while (len > chunk);
	if (len)
	{
		(void) memcpy((char*) dst + offset, (const char*) src + offset, len);
	}
	return dst;
}

/*
 * Calculates the FSB and CPU frequencies using specific MSRs for each CPU
 * - multi. is read from a specific MSR. In the case of Intel, there is:
 *	   a max multi. (used to calculate the FSB freq.),
 *	   and a current multi. (used to calculate the CPU freq.)
 * - busFrequency = tscFrequency / multi
 * - cpuFrequency = busFrequency * multi
 */

/* Decimal powers: */
#define kilo (1000ULL)
#define Mega (kilo * kilo)
#define Giga (kilo * Mega)
#define Tera (kilo * Giga)
#define Peta (kilo * Tera)

#define quad(hi,lo)	(((uint64_t)(hi)) << 32 | (lo))

void get_cpuid(PlatformInfo_t *p)
{

	char		str[128];
	uint32_t	reg[4];
	char		*s			= 0;

	do_cpuid(0x00000000, p->CPU.CPUID[CPUID_0]); // MaxFn, Vendor
	do_cpuid(0x00000001, p->CPU.CPUID[CPUID_1]); // Signature, stepping, features
	do_cpuid(0x00000002, p->CPU.CPUID[CPUID_2]); // TLB/Cache/Prefetch

	do_cpuid(0x00000003, p->CPU.CPUID[CPUID_3]); // S/N
	do_cpuid(0x80000000, p->CPU.CPUID[CPUID_80]); // Get the max extended cpuid

	if ((p->CPU.CPUID[CPUID_80][0] & 0x0000000f) >= 8)
	{
		do_cpuid(0x80000008, p->CPU.CPUID[CPUID_88]);
		do_cpuid(0x80000001, p->CPU.CPUID[CPUID_81]);
	}
	else if ((p->CPU.CPUID[CPUID_80][0] & 0x0000000f) >= 1)
	{
		do_cpuid(0x80000001, p->CPU.CPUID[CPUID_81]);
	}

// ==============================================================

	/* get BrandString (if supported) */
	/* Copyright: from Apple's XNU cpuid.c */
	if (p->CPU.CPUID[CPUID_80][0] > 0x80000004)
	{
		bzero(str, 128);
		/*
		 * The BrandString 48 bytes (max), guaranteed to
		 * be NULL terminated.
		 */
		do_cpuid(0x80000002, reg);          // Processor Brand String
		memcpy(&str[0], (char *)reg, 16);


		do_cpuid(0x80000003, reg);          // Processor Brand String
		memcpy(&str[16], (char *)reg, 16);
		do_cpuid(0x80000004, reg);          // Processor Brand String
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
//		DBG("\tBrandstring = %s\n", p->CPU.BrandString);
	}

// ==============================================================

	switch(p->CPU.BrandString[0])
	{
		case 'A':
			/* AMD Processors */
			// The cache information is only in ecx and edx so only save
			// those registers

			do_cpuid(5,  p->CPU.CPUID[CPUID_5]); // Monitor/Mwait

			do_cpuid(0x80000005, p->CPU.CPUID[CPUID_85]); // TLB/Cache/Prefetch
			do_cpuid(0x80000006, p->CPU.CPUID[CPUID_86]); // TLB/Cache/Prefetch
			do_cpuid(0x80000008, p->CPU.CPUID[CPUID_88]);
			do_cpuid(0x8000001E, p->CPU.CPUID[CPUID_81E]);

			break;

		case 'G':
			/* Intel Processors */
			do_cpuid2(0x00000004, 0, p->CPU.CPUID[CPUID_4]); // Cache Index for Inte

			if (p->CPU.CPUID[CPUID_0][0] >= 0x5)	// Monitor/Mwait
			{
				do_cpuid(5,  p->CPU.CPUID[CPUID_5]);
			}

			if (p->CPU.CPUID[CPUID_0][0] >= 6)	// Thermal/Power
			{
				do_cpuid(6, p->CPU.CPUID[CPUID_6]);
			}

			break;
	}
}

void scan_cpu(PlatformInfo_t *p)
{
	verbose("[ CPU INFO ]\n");
	get_cpuid(p);

	uint64_t	busFCvtt2n;
	uint64_t	tscFCvtt2n;
	uint64_t	tscFreq			= 0;
	uint64_t	busFrequency		= 0;
	uint64_t	cpuFrequency		= 0;
	uint64_t	msr			= 0;
	uint64_t	flex_ratio		= 0;
	uint64_t	cpuid_features;

	uint32_t	max_ratio		= 0;
	uint32_t	min_ratio		= 0;
	uint32_t	reg[4];
	uint32_t	cores_per_package	= 0;
	uint32_t	logical_per_package	= 1;
	uint32_t	threads_per_core	= 1;

	uint8_t		bus_ratio_max		= 0;
	uint8_t		bus_ratio_min		= 0;
	uint32_t	currdiv			= 0;
	uint32_t	currcoef		= 0;
	uint8_t		maxdiv			= 0;
	uint8_t		maxcoef			= 0;
	uint8_t		pic0_mask		= 0;
	uint32_t	cpuMultN2		= 0;

	const char	*newratio;

	int		len			= 0;
	int		myfsb			= 0;
	int		i			= 0;


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
	///////////////////-- MaxFn,Vendor --////////////////////////
	p->CPU.Vendor		= p->CPU.CPUID[CPUID_0][1];

	///////////////////-- Signature, stepping, features -- //////
	cpuid_features = quad(p->CPU.CPUID[CPUID_1][ecx], p->CPU.CPUID[CPUID_1][edx]);
	if (bit(28) & p->CPU.CPUID[CPUID_1][edx]) // HTT/Multicore
	{
		logical_per_package = bitfield(p->CPU.CPUID[CPUID_1][ebx], 23, 16);
	}
	else
	{
		logical_per_package = 1;
	}

	p->CPU.Signature	= p->CPU.CPUID[CPUID_1][0];
	p->CPU.Stepping		= (uint8_t)bitfield(p->CPU.CPUID[CPUID_1][0], 3, 0);	// stepping = cpu_feat_eax & 0xF;
	p->CPU.Model		= (uint8_t)bitfield(p->CPU.CPUID[CPUID_1][0], 7, 4);	// model = (cpu_feat_eax >> 4) & 0xF;
	p->CPU.Family		= (uint8_t)bitfield(p->CPU.CPUID[CPUID_1][0], 11, 8);	// family = (cpu_feat_eax >> 8) & 0xF;
	//p->CPU.Type		= (uint8_t)bitfield(p->CPU.CPUID[CPUID_1][0], 13, 12);	// type = (cpu_feat_eax >> 12) & 0x3;
	p->CPU.ExtModel		= (uint8_t)bitfield(p->CPU.CPUID[CPUID_1][0], 19, 16);	// ext_model = (cpu_feat_eax >> 16) & 0xF;
	p->CPU.ExtFamily	= (uint8_t)bitfield(p->CPU.CPUID[CPUID_1][0], 27, 20);	// ext_family = (cpu_feat_eax >> 20) & 0xFF;

	if (p->CPU.Family == 0x0f)
	{
		p->CPU.Family += p->CPU.ExtFamily;
	}

	if (p->CPU.Family == 0x0f || p->CPU.Family == 0x06)
	{
		p->CPU.Model += (p->CPU.ExtModel << 4);
	}

	switch (p->CPU.Vendor)
	{
		case CPUID_VENDOR_INTEL:
		{
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
					cores_per_package = bitfield(reg[eax], 31, 26) + 1;
				}
			}

			if (i > 0)
			{
				cores_per_package = bitfield(p->CPU.CPUID[CPUID_4][eax], 31, 26) + 1; // i = cache index
				threads_per_core = bitfield(p->CPU.CPUID[CPUID_4][eax], 25, 14) + 1;
			}

			if (cores_per_package == 0)
			{
				cores_per_package = 1;
			}

			switch (p->CPU.Model)
			{
				case CPUID_MODEL_NEHALEM: // Intel Core i7 LGA1366 (45nm)
				case CPUID_MODEL_FIELDS: // Intel Core i5, i7 LGA1156 (45nm)
				case CPUID_MODEL_CLARKDALE: // Intel Core i3, i5, i7 LGA1156 (32nm)
				case CPUID_MODEL_NEHALEM_EX:
				case CPUID_MODEL_JAKETOWN:
				case CPUID_MODEL_SANDYBRIDGE:
				case CPUID_MODEL_IVYBRIDGE:
				case CPUID_MODEL_IVYBRIDGE_XEON:
				case CPUID_MODEL_HASWELL_U5:
				case CPUID_MODEL_HASWELL:
				case CPUID_MODEL_HASWELL_SVR:
				case CPUID_MODEL_HASWELL_ULT:
				case CPUID_MODEL_HASWELL_ULX:
				case CPUID_MODEL_BROADWELL_HQ:
				case CPUID_MODEL_BRASWELL:
				case CPUID_MODEL_AVOTON:
				case CPUID_MODEL_SKYLAKE:
				case CPUID_MODEL_BRODWELL_SVR:
				case CPUID_MODEL_BRODWELL_MSVR:
				case CPUID_MODEL_KNIGHT:
				case CPUID_MODEL_ANNIDALE:
				case CPUID_MODEL_GOLDMONT:
				case CPUID_MODEL_VALLEYVIEW:
				case CPUID_MODEL_SKYLAKE_S:
				case CPUID_MODEL_SKYLAKE_AVX:
				case CPUID_MODEL_CANNONLAKE:
					msr = rdmsr64(MSR_CORE_THREAD_COUNT); // 0x35
					p->CPU.NoCores		= (uint32_t)bitfield((uint32_t)msr, 31, 16);
					p->CPU.NoThreads	= (uint32_t)bitfield((uint32_t)msr, 15,  0);
					break;

				case CPUID_MODEL_DALES:
				case CPUID_MODEL_WESTMERE: // Intel Core i7 LGA1366 (32nm) 6 Core
				case CPUID_MODEL_WESTMERE_EX:
					msr = rdmsr64(MSR_CORE_THREAD_COUNT);
					p->CPU.NoCores		= (uint32_t)bitfield((uint32_t)msr, 19, 16);
					p->CPU.NoThreads	= (uint32_t)bitfield((uint32_t)msr, 15,  0);
					break;
				case CPUID_MODEL_ATOM_3700:
					p->CPU.NoCores		= 4;
					p->CPU.NoThreads	= 4;
					break;
				case CPUID_MODEL_ATOM:
					p->CPU.NoCores		= 2;
					p->CPU.NoThreads	= 2;
					break;
				default:
					p->CPU.NoCores		= 0;
					break;
			}

			// workaround for Xeon Harpertown and Yorkfield
			if ((p->CPU.Model == CPUID_MODEL_PENRYN) &&
				(p->CPU.NoCores	== 0))
			{
				if ((strstr(p->CPU.BrandString, "X54")) ||
					(strstr(p->CPU.BrandString, "E54")) ||
					(strstr(p->CPU.BrandString, "W35")) ||
					(strstr(p->CPU.BrandString, "X34")) ||
					(strstr(p->CPU.BrandString, "X33")) ||
					(strstr(p->CPU.BrandString, "L33")) ||
					(strstr(p->CPU.BrandString, "X32")) ||
					(strstr(p->CPU.BrandString, "L3426")) ||
					(strstr(p->CPU.BrandString, "L54")))
				{
					p->CPU.NoCores		= 4;
					p->CPU.NoThreads	= 4;
				} else if (strstr(p->CPU.BrandString, "W36")) {
					p->CPU.NoCores		= 6;
					p->CPU.NoThreads	= 6;
				} else { //other Penryn and Wolfdale
					p->CPU.NoCores		= 0;
					p->CPU.NoThreads	= 0;
				}
			}

			if (p->CPU.NoCores == 0)
			{
				p->CPU.NoCores		= cores_per_package;
				p->CPU.NoThreads	= logical_per_package;
			}

			// MSR is *NOT* available on the Intel Atom CPU
			// workaround for N270. I don't know why it detected wrong
			if ((p->CPU.Model == CPUID_MODEL_ATOM) && (strstr(p->CPU.BrandString, "270")))
			{
				p->CPU.NoCores		= 1;
				p->CPU.NoThreads	= 2;
			}

			// workaround for Quad
			if ( strstr(p->CPU.BrandString, "Quad") )
			{
				p->CPU.NoCores		= 4;
				p->CPU.NoThreads	= 4;
			}
		}

		break;

		case CPUID_VENDOR_AMD:
		{
			post_startup_cpu_fixups();

			if (p->CPU.ExtFamily < 0x8)
			{
				cores_per_package = bitfield(p->CPU.CPUID[CPUID_88][ecx], 7, 0) + 1;
				//threads_per_core = cores_per_package;
			}
			else

			// Bronya : test for SMT
			// Properly calculate number of cores on AMD Zen
			// TODO: Check MSR for SMT
			if (p->CPU.ExtFamily >= 0x8)
			{
				uint64_t cores = 0;
				uint64_t logical = 0;

				cores = bitfield(p->CPU.CPUID[CPUID_81E][ebx], 7, 0); // cores
				logical = bitfield(p->CPU.CPUID[CPUID_81E][ebx], 15, 8) + 1; // 2

				cores_per_package = (bitfield(p->CPU.CPUID[CPUID_88][ecx], 7, 0) + 1) / logical; //8 cores

				//threads_per_core = cores_per_package;

			}

			if (cores_per_package == 0)
			{
				cores_per_package = 1;
			}

			p->CPU.NoCores		= cores_per_package;
			p->CPU.NoThreads	= logical_per_package;

			if (p->CPU.NoCores == 0)
			{
				p->CPU.NoCores = 1;
				p->CPU.NoThreads	= 1;
			}
		}
		break;

		default :
			stop("Unsupported CPU detected! System halted.");
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

	if ((bit(5) & p->CPU.CPUID[CPUID_1][3]) != 0)
	{
		p->CPU.Features |= CPU_FEATURE_MSR;
	}

	if ((p->CPU.NoThreads > p->CPU.NoCores))
	{
		p->CPU.Features |= CPU_FEATURE_HTT;
	}

	if ((bit(29) & p->CPU.CPUID[CPUID_81][3]) != 0)
	{
		p->CPU.Features |= CPU_FEATURE_EM64T;
	}

	pic0_mask = inb(0x21U);
	outb(0x21U, 0xFFU);     // mask PIC0 interrupts for duration of timing tests

	uint64_t cycles;
	cycles = timeRDTSC();
	tscFreq = rtc_set_cyc_per_sec(cycles);
	DBG("cpu freq classic = 0x%016llx\n", tscFreq);
	// if usual method failed
	if ( tscFreq < 1000 )	//TEST
	{
		tscFreq = measure_tsc_frequency();//timeRDTSC() * 20;//measure_tsc_frequency();
		// DBG("cpu freq timeRDTSC = 0x%016llx\n", tscFrequency);
	}

	if (p->CPU.Vendor==CPUID_VENDOR_INTEL && ((p->CPU.Family == 0x06 && p->CPU.Model >= 0x0c) || (p->CPU.Family == 0x0f && p->CPU.Model >= 0x03)))
	{
		int intelCPU = p->CPU.Model;
		if (p->CPU.Family == 0x06)
		{
			/* Nehalem CPU model */
			switch (p->CPU.Model)
			{
				case CPUID_MODEL_NEHALEM:
				case CPUID_MODEL_FIELDS:
				case CPUID_MODEL_CLARKDALE:
				case CPUID_MODEL_DALES:
				case CPUID_MODEL_WESTMERE:
				case CPUID_MODEL_NEHALEM_EX:
				case CPUID_MODEL_WESTMERE_EX:
/* --------------------------------------------------------- */
				case CPUID_MODEL_SANDYBRIDGE:
				case CPUID_MODEL_JAKETOWN:
				case CPUID_MODEL_IVYBRIDGE_XEON:
				case CPUID_MODEL_IVYBRIDGE:
				case CPUID_MODEL_ATOM_3700:
				case CPUID_MODEL_HASWELL:
				case CPUID_MODEL_HASWELL_U5:
				case CPUID_MODEL_HASWELL_SVR:

				case CPUID_MODEL_HASWELL_ULT:
				case CPUID_MODEL_HASWELL_ULX:
				case CPUID_MODEL_BROADWELL_HQ:
				case CPUID_MODEL_SKYLAKE_S:
/* --------------------------------------------------------- */
					msr = rdmsr64(MSR_PLATFORM_INFO);
					DBG("msr(%d): platform_info %08llx\n", __LINE__, bitfield(msr, 31, 0));
					bus_ratio_max = bitfield(msr, 15, 8);
					bus_ratio_min = bitfield(msr, 47, 40); //valv: not sure about this one (Remarq.1)
					msr = rdmsr64(MSR_FLEX_RATIO);
					DBG("msr(%d): flex_ratio %08llx\n", __LINE__, bitfield(msr, 31, 0));
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
							DBG("CPU: Unusable flex ratio detected. Patched MSR now %08llx\n", bitfield(msr, 31, 0));
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
						busFrequency = (tscFreq / bus_ratio_max);
					}

					//valv: Turbo Ratio Limit
					if ((intelCPU != 0x2e) && (intelCPU != 0x2f))
					{
						msr = rdmsr64(MSR_TURBO_RATIO_LIMIT);

						cpuFrequency = bus_ratio_max * busFrequency;
						max_ratio = bus_ratio_max * 10;
					}
					else
					{
						cpuFrequency = tscFreq;
					}

					if ((getValueForKey(kbusratio, &newratio, &len, &bootInfo->chameleonConfig)) && (len <= 4))
					{
						max_ratio = atoi(newratio);
						max_ratio = (max_ratio * 10);
						if (len >= 3)
						{
							max_ratio = (max_ratio + 5);
						}

						verbose("\tBus-Ratio: min=%d, max=%s\n", bus_ratio_min, newratio);

						// extreme overclockers may love 320 ;)
						if ((max_ratio >= min_ratio) && (max_ratio <= 320))
						{
							cpuFrequency = (busFrequency * max_ratio) / 10;
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

				myfsb = busFrequency / 1000000;
				verbose("\tSticking with [BCLK: %dMhz, Bus-Ratio: %d]\n", myfsb, max_ratio/10);  // Bungo: fixed wrong Bus-Ratio readout
				currcoef = bus_ratio_max;

				break;

			default:
				msr = rdmsr64(MSR_IA32_PERF_STATUS);
				DBG("msr(%d): ia32_perf_stat 0x%08llx\n", __LINE__, bitfield(msr, 31, 0));
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
						busFrequency = ((tscFreq * 2) / ((maxcoef * 2) + 1));
					}
					else
					{
						busFrequency = (tscFreq / maxcoef);
					}

					if (currdiv)
					{
						cpuFrequency = (busFrequency * ((currcoef * 2) + 1) / 2);
					}
					else
					{
						cpuFrequency = (busFrequency * currcoef);
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

	else if (p->CPU.Vendor==CPUID_VENDOR_AMD)
	{
		switch(p->CPU.Family)
		{
			case 0xF: /* K8 */
			{
				uint64_t fidvid = 0;
				uint64_t cpuMult;
				uint64_t cpuFid;

				fidvid = rdmsr64(AMD_K8_PERF_STS);
				cpuFid = bitfield(fidvid, 5, 0);

				cpuMult = (cpuFid + 0x8) * 10 / 2;
				currcoef = cpuMult;

				cpuMultN2 = (fidvid & (uint64_t)bit(0));
				currdiv = cpuMultN2;
				/****** Addon END ******/
			}
				break;

			case 0x10: /*** AMD Family 10h ***/
			{

				uint64_t prfsts = 0;
				uint64_t cpuMult;
				uint64_t divisor = 0;
				uint64_t cpuDid;
				uint64_t cpuFid;

				prfsts  = rdmsr64(AMD_COFVID_STS);
				cpuDid = bitfield(prfsts, 8, 6);
				cpuFid = bitfield(prfsts, 5, 0);
				if (cpuDid == 0) divisor = 2;
				else if (cpuDid == 1) divisor = 4;
				else if (cpuDid == 2) divisor = 8;
				else if (cpuDid == 3) divisor = 16;
				else if (cpuDid == 4) divisor = 32;

				cpuMult = ((cpuFid + 0x10) * 10) / (2^cpuDid);
				currcoef = cpuMult;

				cpuMultN2 = (prfsts & (uint64_t)bit(0));
				currdiv = cpuMultN2;

				/****** Addon END ******/
			}
			break;

			case 0x11: /*** AMD Family 11h ***/
			{

				uint64_t prfsts;
				uint64_t cpuMult;
				uint64_t divisor = 0;
				uint64_t cpuDid;
				uint64_t cpuFid;

				prfsts  = rdmsr64(AMD_COFVID_STS);

				cpuDid = bitfield(prfsts, 8, 6);
				cpuFid = bitfield(prfsts, 5, 0);
				if (cpuDid == 0) divisor = 2;
				else if (cpuDid == 1) divisor = 4;
				else if (cpuDid == 2) divisor = 8;
				else if (cpuDid == 3) divisor = 16;
				else if (cpuDid == 4) divisor = 0;
				cpuMult = ((cpuFid + 0x8) * 10 ) / divisor;
				currcoef = cpuMult;

				cpuMultN2 = (prfsts & (uint64_t)bit(0));
				currdiv = cpuMultN2;

				/****** Addon END ******/
			}
                break;

			case 0x12: /*** AMD Family 12h ***/
			{
				// 8:4 CpuFid: current CPU core frequency ID
				// 3:0 CpuDid: current CPU core divisor ID
				uint64_t prfsts,CpuFid,CpuDid;
				prfsts = rdmsr64(AMD_COFVID_STS);

				CpuDid = bitfield(prfsts, 3, 0) ;
				CpuFid = bitfield(prfsts, 8, 4) ;
				uint64_t divisor;
				switch (CpuDid)
				{
					case 0: divisor = 1; break;
					case 1: divisor = (3/2); break;
					case 2: divisor = 2; break;
					case 3: divisor = 3; break;
					case 4: divisor = 4; break;
					case 5: divisor = 6; break;
					case 6: divisor = 8; break;
					case 7: divisor = 12; break;
					case 8: divisor = 16; break;
					default: divisor = 1; break;
				}
				currcoef = ((CpuFid + 0x10) * 10) / divisor;

				cpuMultN2 = (prfsts & (uint64_t)bit(0));
				currdiv = cpuMultN2;

			}
				break;

			case 0x14: /* K14 */

			{
				// 8:4: current CPU core divisor ID most significant digit
				// 3:0: current CPU core divisor ID least significant digit
				uint64_t prfsts;
				prfsts = rdmsr64(AMD_COFVID_STS);

				uint64_t CpuDidMSD,CpuDidLSD;
				CpuDidMSD = bitfield(prfsts, 8, 4) ;
				CpuDidLSD  = bitfield(prfsts, 3, 0) ;

				uint64_t frequencyId = tscFreq/Mega;
				currcoef = (((frequencyId + 5) / 100) + 0x10) * 10 /
					(CpuDidMSD + (CpuDidLSD  * 0.25) + 1);
				currdiv = ((CpuDidMSD) + 1) << 2;
				currdiv += bitfield(prfsts, 3, 0);

				cpuMultN2 = (prfsts & (uint64_t)bit(0));
				currdiv = cpuMultN2;
			}

				break;

			case 0x15: /*** AMD Family 15h ***/
			case 0x06: /*** AMD Family 06h ***/
			{

				uint64_t prfsts = 0;
				uint64_t cpuMult;
				//uint64_t divisor = 0;
				uint64_t cpuDid;
				uint64_t cpuFid;

				prfsts  = rdmsr64(AMD_COFVID_STS);
				cpuDid = bitfield(prfsts, 8, 6);
				cpuFid = bitfield(prfsts, 5, 0);

				cpuMult = ((cpuFid + 0x10) * 10) / (2^cpuDid);
				currcoef = cpuMult;

				cpuMultN2 = (prfsts & 0x01) * 1;//(prfsts & (uint64_t)bit(0));
				currdiv = cpuMultN2;
			}
				break;

			case 0x16: /*** AMD Family 16h kabini ***/
			{
				uint64_t prfsts = 0;
				uint64_t cpuMult;
				uint64_t divisor = 0;
				uint64_t cpuDid;
				uint64_t cpuFid;
				prfsts  = rdmsr64(AMD_COFVID_STS);
				cpuDid = bitfield(prfsts, 8, 6);
				cpuFid = bitfield(prfsts, 5, 0);
				if (cpuDid == 0) divisor = 1;
				else if (cpuDid == 1) divisor = 2;
				else if (cpuDid == 2) divisor = 4;
				else if (cpuDid == 3) divisor = 8;
				else if (cpuDid == 4) divisor = 16;

				cpuMult = ((cpuFid + 0x10) * 10) / divisor;
				currcoef = cpuMult;

				cpuMultN2 = (prfsts & (uint64_t)bit(0));
				currdiv = cpuMultN2;

				/****** Addon END ******/
			}
				break;

			case 0x17: /*** AMD Family 17h Ryzen ***/
			{
				uint64_t cpuMult;
				uint64_t CpuDfsId;
				uint64_t CpuFid;
				uint64_t fid = 0;
				uint64_t prfsts = 0;

				prfsts = rdmsr64(AMD_PSTATE0_STS);

				CpuDfsId = bitfield(prfsts, 13, 8);
				CpuFid = bitfield(prfsts, 7, 0);

				cpuMult = (CpuFid * 10 / CpuDfsId) * 2;

				currcoef = cpuMult;

				fid = (int)(cpuMult / 10);

				uint8_t fdiv = cpuMult - (fid * 10);
				if (fdiv > 0) {
					currdiv = 1;
				}

				/****** Addon END ******/
			}
				break;

			default:
			{
				currcoef = tscFreq / (200 * Mega);
			}
		}

		#define nya(x) x/10,x%10

		if (currcoef)
		{
			if (currdiv)
			{
				currcoef = nya(currcoef);

				busFrequency = ((tscFreq * 2) / ((currcoef * 2) + 1));
				busFCvtt2n = ((1 * Giga) << 32) / busFrequency;
				tscFCvtt2n = busFCvtt2n * 2 / (1 + (2 * currcoef));
				cpuFrequency = ((1 * Giga)  << 32) / tscFCvtt2n;

				DBG("%d.%d\n", currcoef / currdiv, ((currcoef % currdiv) * 100) / currdiv);
			}
			else
			{
				currcoef = nya(currcoef);

				busFrequency = (tscFreq / currcoef);
				busFCvtt2n = ((1 * Giga) << 32) / busFrequency;
				tscFCvtt2n = busFCvtt2n / currcoef;
				cpuFrequency = ((1 * Giga)  << 32) / tscFCvtt2n;
				DBG("%d\n", currcoef);
			}
		}
		else if (!cpuFrequency)
		{
			cpuFrequency = tscFreq;
		}
	}

#if 0
	if (!busFrequency)
	{
		busFrequency = (DEFAULT_FSB * 1000);
		DBG("\tCPU: busFrequency = 0! using the default value for FSB!\n");
		cpuFrequency = tscFreq;
	}

	DBG("\tcpu freq = 0x%016llxn", timeRDTSC() * 20);

#endif

	outb(0x21U, pic0_mask);     // restore PIC0 interrupts

	p->CPU.MaxCoef = maxcoef = currcoef;
	p->CPU.MaxDiv = maxdiv = currdiv;
	p->CPU.CurrCoef = currcoef;
	p->CPU.CurrDiv = currdiv;
	p->CPU.TSCFrequency = tscFreq;
	p->CPU.FSBFrequency = busFrequency;
	p->CPU.CPUFrequency = cpuFrequency;

	// keep formatted with spaces instead of tabs

	DBG("\tCPUID Raw Values:\n");
	for (i = 0; i < CPUID_MAX; i++)
	{
		DBG("\t%02d:  %08X-%08X-%08X-%08X\n", i, p->CPU.CPUID[i][eax], p->CPU.CPUID[i][ebx], p->CPU.CPUID[i][ecx], p->CPU.CPUID[i][edx]);
	}
	DBG("\n");
	DBG("\tBrand String:            %s\n",		p->CPU.BrandString);		// Processor name (BIOS)
	DBG("\tVendor:                  0x%X\n",	p->CPU.Vendor);			// Vendor ex: GenuineIntel
	DBG("\tFamily:                  0x%X\n",	p->CPU.Family);			// Family ex: 6 (06h)
	DBG("\tExtFamily:               0x%X\n",	p->CPU.ExtFamily);
	DBG("\tSignature:               0x%08X\n",	p->CPU.Signature);		// CPUID signature
	/*switch (p->CPU.Type) {
		case PT_OEM:
			DBG("\tProcessor type:          Intel Original OEM Processor\n");
			break;
		case PT_OD:
			DBG("\tProcessor type:          Intel Over Drive Processor\n");
			break;
		case PT_DUAL:
			DBG("\tProcessor type:          Intel Dual Processor\n");
			break;
		case PT_RES:
			DBG("\tProcessor type:          Intel Reserved\n");
			break;
		default:
			break;
	}*/
	DBG("\tModel:                   0x%X\n",	p->CPU.Model);			// Model ex: 37 (025h)
	DBG("\tExtModel:                0x%X\n",	p->CPU.ExtModel);
	DBG("\tStepping:                0x%X\n",	p->CPU.Stepping);		// Stepping ex: 5 (05h)
	DBG("\tMaxCoef:                 %d\n",		p->CPU.MaxCoef);
	DBG("\tCurrCoef:                %d\n",		p->CPU.CurrCoef);
	DBG("\tMaxDiv:                  %d\n",		p->CPU.MaxDiv);
	DBG("\tCurrDiv:                 %d\n",		p->CPU.CurrDiv);
	DBG("\tTSCFreq:                 %dMHz\n",	p->CPU.TSCFrequency / 1000000);
	DBG("\tFSBFreq:                 %dMHz\n",	p->CPU.FSBFrequency / 1000000);
	DBG("\tCPUFreq:                 %dMHz\n",	p->CPU.CPUFrequency / 1000000);
	DBG("\tCores:                   %d\n",		p->CPU.NoCores);		// Cores
	DBG("\tLogical processor:       %d\n",		p->CPU.NoThreads);		// Logical procesor
	DBG("\tFeatures:                0x%08x\n",	p->CPU.Features);
//	DBG("\tMicrocode version:       %d\n",		p->CPU.MCodeVersion);		// CPU microcode version

	verbose("\n");
#if DEBUG_CPU
	pause();
#endif
}
