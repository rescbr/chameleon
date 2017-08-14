/*
 *  platform.h
 *  AsereBLN: reworked and extended
 *
 */

#ifndef __LIBSAIO_PLATFORM_H
#define __LIBSAIO_PLATFORM_H

//#include "libsaio.h"

extern bool platformCPUFeature(uint32_t);
extern void scan_platform(void);
extern void dumpPhysAddr(const char * title, void * a, int len);

/* CPUID index into cpuid_raw */
#define CPUID_0				0
#define CPUID_1				1
#define CPUID_2				2
#define CPUID_3				3
#define CPUID_4				4
#define CPUID_5				5
#define CPUID_6				6
#define CPUID_80			7
#define CPUID_81			8
#define CPUID_85			9
#define CPUID_86			10
#define CPUID_87			11
#define CPUID_88			12
#define CPUID_81E			13
#define CPUID_MAX			14

#define CPUID_MODEL_ANY			0x00
#define CPUID_MODEL_UNKNOWN		0x01
#define CPUID_MODEL_PRESCOTT		0x03			// Celeron D, Pentium 4 (90nm)
#define CPUID_MODEL_NOCONA		0x04			// Xeon Nocona/Paxville, Irwindale (90nm)
#define CPUID_MODEL_PRESLER		0x06			// Pentium 4, Pentium D (65nm)
#define CPUID_MODEL_PENTIUM_M		0x09			// Banias Pentium M (130nm)
#define CPUID_MODEL_DOTHAN		0x0D			// Dothan Pentium M, Celeron M (90nm)
#define CPUID_MODEL_YONAH		0x0E			// Sossaman, Yonah
#define CPUID_MODEL_MEROM		0x0F			// Allendale, Conroe, Kentsfield, Woodcrest, Clovertown, Tigerton, Merom
#define CPUID_MODEL_CONROE		0x16			// Merom, Conroe (65nm), Celeron (45nm)
#define CPUID_MODEL_PENRYN		0x17			// Wolfdale, Yorkfield, Harpertown, Penryn
#define CPUID_MODEL_WOLFDALE		0x17			// Xeon 31xx, 33xx, 52xx, 54xx, Core 2 Quad 8xxx and 9xxx
#define CPUID_MODEL_NEHALEM		0x1A			// Bloomfield. Nehalem-EP, Nehalem-WS, Gainestown
#define CPUID_MODEL_ATOM		0x1C			// Pineview, Bonnell
#define CPUID_MODEL_XEON_MP		0x1D			// MP 7400
#define CPUID_MODEL_FIELDS		0x1E			// Core i7 and i5 Processor - Clarksfield, Lynnfield, Jasper Forest
#define CPUID_MODEL_CLARKDALE		0x1F			// Core i7 and i5 Processor - Nehalem (Havendale, Auburndale)
#define CPUID_MODEL_DALES		0x25			// Westmere Client - Clarkdale, Arrandale
#define CPUID_MODEL_ATOM_SAN		0x26			// Lincroft
#define CPUID_MODEL_LINCROFT		0x27			// Bonnell, penwell
#define CPUID_MODEL_SANDYBRIDGE		0x2A			// Sandy Bridge
#define CPUID_MODEL_WESTMERE		0x2C			// Gulftown, Westmere-EP, Westmere-WS
#define CPUID_MODEL_JAKETOWN		0x2D			// Sandy Bridge-E, Sandy Bridge-EP
#define CPUID_MODEL_NEHALEM_EX		0x2E			// Nehalem-EX Xeon - Beckton
#define CPUID_MODEL_WESTMERE_EX		0x2F			// Westmere-EX Xeon - Eagleton
#define CPUID_MODEL_CLOVERVIEW		0x35			// Atom Family Bonnell, cloverview
#define CPUID_MODEL_ATOM_2000		0x36			// Cedarview / Saltwell
#define CPUID_MODEL_ATOM_3700		0x37			// Atom E3000, Z3000 Atom Silvermont **BYT
#define CPUID_MODEL_IVYBRIDGE		0x3A			// Ivy Bridge
#define CPUID_MODEL_HASWELL		0x3C			// Haswell DT ex.i7 4790K
#define CPUID_MODEL_HASWELL_U5		0x3D			// Haswell U5  5th generation Broadwell, Core M / Core-AVX2
#define CPUID_MODEL_IVYBRIDGE_XEON	0x3E			// Ivy Bridge Xeon
#define CPUID_MODEL_HASWELL_SVR		0x3F			// Haswell Server, Xeon E5-2600/1600 v3 (Haswell-E) **HSX
//#define CPUID_MODEL_HASWELL_H		0x??			// Haswell H
#define CPUID_MODEL_HASWELL_ULT		0x45			// Haswell ULT, 4th gen Core, Xeon E3-12xx v3 C8/C9/C10
#define CPUID_MODEL_HASWELL_ULX		0x46			// Crystal Well, 4th gen Core, Xeon E3-12xx v3
#define CPUID_MODEL_BROADWELL_HQ	0x47			// Broadwell BDW
#define CPUID_MODEL_MERRIFIELD		0x4A			// Future Atom E3000, Z3000 silvermont / atom (Marrifield)
#define CPUID_MODEL_BRASWELL		0x4C			// Atom (Braswell)
#define CPUID_MODEL_AVOTON		0x4D			// Silvermont/Avoton Atom C2000 **AVN
#define CPUID_MODEL_SKYLAKE		0x4E			// Future Core **SKL
#define CPUID_MODEL_BRODWELL_SVR	0x4F			// Broadwell Server **BDX
#define CPUID_MODEL_SKYLAKE_AVX		0x55			// Skylake with AVX-512 support.
#define CPUID_MODEL_BRODWELL_MSVR	0x56			// Broadwell Micro Server, Future Xeon **BDX-DE
#define CPUID_MODEL_KNIGHT		0x57			// Knights Landing
#define CPUID_MODEL_ANNIDALE		0x5A			// Silvermont, Future Atom E3000, Z3000 (Annidale)
#define CPUID_MODEL_GOLDMONT		0x5C
#define CPUID_MODEL_VALLEYVIEW		0x5D			// Silvermont, Future Atom E3000, Z3000
#define CPUID_MODEL_SKYLAKE_S		0x5E			// Skylake **SKL
#define CPUID_MODEL_CANNONLAKE		0x66
#define CPUID_MODEL_DENVERTON		0x5F			// Goldmont Microserver
#define CPUID_MODEL_XEON_MILL		0x85			// Knights Mill
#define CPUID_MODEL_KABYLAKE1		0x8E			// Kabylake Mobile
#define CPUID_MODEL_KABYLAKE2		0x9E			// Kabylake Dektop

/* CPUID Vendor */
#define	CPUID_VID_INTEL			"GenuineIntel"
#define	CPUID_VID_AMD			"AuthenticAMD"

#define CPUID_VENDOR_INTEL		0x756E6547
#define CPUID_VENDOR_AMD		0x68747541

/* This spells out "GenuineIntel".  */
//#define is_intel \
//  ebx == 0x756e6547 && ecx == 0x6c65746e && edx == 0x49656e69

/* This spells out "AuthenticAMD".  */
//#define is_amd \
//  ebx == 0x68747541 && ecx == 0x444d4163 && edx == 0x69746e65

/* Unknown CPU */
#define CPU_STRING_UNKNOWN		"Unknown CPU Typ"

//definitions from Apple XNU

/* CPU defines */
#define bit(n)				(1ULL << (n))
#define bitmask(h,l)			((bit(h) | (bit(h)-1)) & ~(bit(l)-1))
#define bitfield(x,h,l)			(((x) & bitmask(h,l)) >> l)
#define hbit(n)				(1ULL << ((n)+32))
#define min(a,b)			((a) < (b) ? (a) : (b))
#define quad32(hi,lo)			((((uint32_t)(hi)) << 16) | (((uint32_t)(lo)) & 0xFFFF))
#define quad64(hi,lo)			((((uint64_t)(hi)) << 32) | (((uint64_t)(lo)) & 0xFFFFFFFFUL))

/*
 * The CPUID_FEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 1: 
 */
#define CPUID_FEATURE_FPU		bit(0)   /* Floating point unit on-chip */
#define CPUID_FEATURE_VME		bit(1)   /* Virtual Mode Extension */
#define CPUID_FEATURE_DE		bit(2)   /* Debugging Extension */
#define CPUID_FEATURE_PSE		bit(3)   /* Page Size Extension */
#define CPUID_FEATURE_TSC		bit(4)   /* Time Stamp Counter */
#define CPUID_FEATURE_MSR		bit(5)   /* Model Specific Registers */
#define CPUID_FEATURE_PAE		bit(6)   /* Physical Address Extension */
#define CPUID_FEATURE_MCE		bit(7)   /* Machine Check Exception */
#define CPUID_FEATURE_CX8		bit(8)   /* CMPXCHG8B */
#define CPUID_FEATURE_APIC		bit(9)   /* On-chip APIC */
#define CPUID_FEATURE_SEP		bit(11)  /* Fast System Call */
#define CPUID_FEATURE_MTRR		bit(12)  /* Memory Type Range Register */
#define CPUID_FEATURE_PGE		bit(13)  /* Page Global Enable */
#define CPUID_FEATURE_MCA		bit(14)  /* Machine Check Architecture */
#define CPUID_FEATURE_CMOV		bit(15)  /* Conditional Move Instruction */
#define CPUID_FEATURE_PAT		bit(16)  /* Page Attribute Table */
#define CPUID_FEATURE_PSE36		bit(17)  /* 36-bit Page Size Extension */
#define CPUID_FEATURE_PSN		bit(18)  /* Processor Serial Number */
#define CPUID_FEATURE_CLFSH		bit(19)  /* CLFLUSH Instruction supported */
#define CPUID_FEATURE_DS		bit(21)  /* Debug Store */
#define CPUID_FEATURE_ACPI		bit(22)  /* Thermal monitor and Clock Ctrl */
#define CPUID_FEATURE_MMX		bit(23)  /* MMX supported */
#define CPUID_FEATURE_FXSR		bit(24)  /* Fast floating pt save/restore */
#define CPUID_FEATURE_SSE		bit(25)  /* Streaming SIMD extensions */
#define CPUID_FEATURE_SSE2		bit(26)  /* Streaming SIMD extensions 2 */
#define CPUID_FEATURE_SS		bit(27)  /* Self-Snoop */
#define CPUID_FEATURE_HTT		bit(28)  /* Hyper-Threading Technology */
#define CPUID_FEATURE_TM		bit(29)  /* Thermal Monitor (TM1) */
#define CPUID_FEATURE_PBE		bit(31)  /* Pend Break Enable */

#define CPUID_FEATURE_SSE3		hbit(0)  /* Streaming SIMD extensions 3 */
#define CPUID_FEATURE_PCLMULQDQ		hbit(1)  /* PCLMULQDQ Instruction */
#define CPUID_FEATURE_DTES64		hbit(2)  /* 64-bit DS layout */
#define CPUID_FEATURE_MONITOR		hbit(3)  /* Monitor/mwait */
#define CPUID_FEATURE_DSCPL		hbit(4)  /* Debug Store CPL */
#define CPUID_FEATURE_VMX		hbit(5)  /* VMX */
#define CPUID_FEATURE_SMX		hbit(6)  /* SMX */
#define CPUID_FEATURE_EST		hbit(7)  /* Enhanced SpeedsTep (GV3) */
#define CPUID_FEATURE_TM2		hbit(8)  /* Thermal Monitor 2 */
#define CPUID_FEATURE_SSSE3		hbit(9)  /* Supplemental SSE3 instructions */
#define CPUID_FEATURE_CID		hbit(10) /* L1 Context ID */
#define CPUID_FEATURE_SEGLIM64		hbit(11) /* 64-bit segment limit checking */
#define CPUID_FEATURE_FMA		hbit(12) /* Fused-Multiply-Add support */
#define CPUID_FEATURE_CX16		hbit(13) /* CmpXchg16b instruction */
#define CPUID_FEATURE_xTPR		hbit(14) /* Send Task PRiority msgs */
#define CPUID_FEATURE_PDCM		hbit(15) /* Perf/Debug Capability MSR */

#define CPUID_FEATURE_PCID		hbit(17) /* ASID-PCID support */
#define CPUID_FEATURE_DCA		hbit(18) /* Direct Cache Access */
#define CPUID_FEATURE_SSE4_1		hbit(19) /* Streaming SIMD extensions 4.1 */
#define CPUID_FEATURE_SSE4_2		hbit(20) /* Streaming SIMD extensions 4.2 */
#define CPUID_FEATURE_x2APIC		hbit(21) /* Extended APIC Mode */
#define CPUID_FEATURE_MOVBE		hbit(22) /* MOVBE instruction */
#define CPUID_FEATURE_POPCNT		hbit(23) /* POPCNT instruction */
#define CPUID_FEATURE_TSCTMR		hbit(24) /* TSC deadline timer */
#define CPUID_FEATURE_AES		hbit(25) /* AES instructions */
#define CPUID_FEATURE_XSAVE		hbit(26) /* XSAVE instructions */
#define CPUID_FEATURE_OSXSAVE		hbit(27) /* XGETBV/XSETBV instructions */
#define CPUID_FEATURE_AVX1_0		hbit(28) /* AVX 1.0 instructions */
#define CPUID_FEATURE_F16C		hbit(29) /* Float16 convert instructions */
#define CPUID_FEATURE_RDRAND		hbit(30) /* RDRAND instruction */
#define CPUID_FEATURE_VMM		hbit(31) /* VMM (Hypervisor) present */

/*
 * Leaf 7, subleaf 0 additional features.
 * Bits returned in %ebx to a CPUID request with {%eax,%ecx} of (0x7,0x0}:
 */
#define CPUID_LEAF7_FEATURE_RDWRFSGS	bit(0)	/* FS/GS base read/write */
#define CPUID_LEAF7_FEATURE_TSCOFF	bit(1)	/* TSC thread offset */
#define CPUID_LEAF7_FEATURE_BMI1	bit(3)	/* Bit Manipulation Instrs, set 1 */
#define CPUID_LEAF7_FEATURE_HLE		bit(4)	/* Hardware Lock Elision*/
#define CPUID_LEAF7_FEATURE_AVX2	bit(5)	/* AVX2 Instructions */
#define CPUID_LEAF7_FEATURE_SMEP	bit(7)	/* Supervisor Mode Execute Protect */
#define CPUID_LEAF7_FEATURE_BMI2	bit(8)	/* Bit Manipulation Instrs, set 2 */
#define CPUID_LEAF7_FEATURE_ENFSTRG	bit(9)	/* ENhanced Fast STRinG copy */
#define CPUID_LEAF7_FEATURE_INVPCID	bit(10)	/* INVPCID intruction, TDB */
#define CPUID_LEAF7_FEATURE_RTM		bit(11)	/* TBD */

/*
 * The CPUID_EXTFEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 0x80000001: 
 */
#define CPUID_EXTFEATURE_SYSCALL	bit(11)	/* SYSCALL/sysret */
#define CPUID_EXTFEATURE_XD		bit(20)	/* eXecute Disable */

#define CPUID_EXTFEATURE_1GBPAGE	bit(26)	/* 1GB pages support */
#define CPUID_EXTFEATURE_RDTSCP		bit(27)	/* RDTSCP */
#define CPUID_EXTFEATURE_EM64T		bit(29)	/* Extended Mem 64 Technology */


#define CPUID_EXTFEATURE_LAHF		hbit(0)	/* LAFH/SAHF instructions */

/*
 * The CPUID_EXTFEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 0x80000007: 
 */
#define CPUID_EXTFEATURE_TSCI		bit(8)	/* TSC Invariant */

#define	CPUID_CACHE_SIZE		16	/* Number of descriptor values */

#define CPUID_MWAIT_EXTENSION		bit(0)	/* enumeration of WMAIT extensions */
#define CPUID_MWAIT_BREAK		bit(1)	/* interrupts are break events	   */

//-- processor type -> p_type:
#define PT_OEM				0x00	// Intel Original OEM Processor;
#define PT_OD				0x01 	// Intel Over Drive Processor;
#define PT_DUAL				0x02	// Intel Dual Processor;
#define PT_RES				0x03	// Intel Reserved;

/* Known MSR registers */
#define MSR_IA32_PLATFORM_ID		0x0017
#define IA32_APIC_BASE			0x001B  /* used also for AMD */
#define MSR_CORE_THREAD_COUNT		0x0035	/* limited use - not for Penryn or older */
#define IA32_TSC_ADJUST			0x003B
#define MSR_IA32_BIOS_SIGN_ID		0x008B	/* microcode version */
#define MSR_FSB_FREQ			0x00CD	/* limited use - not for i7 */
#define	MSR_PLATFORM_INFO		0x00CE	/* limited use - MinRatio for i7 but Max for Yonah	*/

/* turbo for penryn */
#define MSR_PKG_CST_CONFIG_CONTROL	0x00E2		// sandy and ivy
#define MSR_PMG_IO_CAPTURE_BASE		0x00E4
#define IA32_MPERF			0x00E7		// TSC in C0 only
#define IA32_APERF			0x00E8		// actual clocks in C0
#define MSR_IA32_EXT_CONFIG		0x00EE		// limited use - not for i7
#define MSR_FLEX_RATIO			0x0194		// limited use - not for Penryn or older
						//see no value on most CPUs
#define	MSR_IA32_PERF_STATUS		0x0198
#define MSR_IA32_PERF_CONTROL		0x0199
#define MSR_IA32_CLOCK_MODULATION	0x019A
#define MSR_THERMAL_STATUS		0x019C
#define MSR_IA32_MISC_ENABLE		0x01A0
#define MSR_THERMAL_TARGET		0x01A2		// TjMax limited use - not for Penryn or older
#define MSR_MISC_PWR_MGMT		0x01AA
#define MSR_TURBO_RATIO_LIMIT		0x01AD		// limited use - not for Penryn or older

#define IA32_ENERGY_PERF_BIAS		0x01B0
#define MSR_PACKAGE_THERM_STATUS	0x01B1
#define IA32_PLATFORM_DCA_CAP		0x01F8
#define MSR_POWER_CTL			0x01FC		// MSR 000001FC  0000-0000-0004-005F

// Nehalem (NHM) adds support for additional MSRs
#define MSR_SMI_COUNT                   0x034
#define MSR_NHM_PLATFORM_INFO           0x0ce
#define MSR_NHM_SNB_PKG_CST_CFG_CTL     0x0e2
#define MSR_PKG_C3_RESIDENCY            0x3f8
#define MSR_PKG_C6_RESIDENCY            0x3f9
#define MSR_CORE_C3_RESIDENCY           0x3fc
#define MSR_CORE_C6_RESIDENCY           0x3fd

// Sandy Bridge & JakeTown specific 'Running Average Power Limit' MSR's.
#define MSR_RAPL_POWER_UNIT		0x606		// R/O
//MSR 00000606                                      0000-0000-000A-1003
#define MSR_PKGC3_IRTL			0x60A		// RW time limit to go C3
// bit 15 = 1 -- the value valid for C-state PM
#define MSR_PKGC6_IRTL			0x60B		// RW time limit to go C6
//MSR 0000060B                                      0000-0000-0000-8854
//Valid + 010=1024ns + 0x54=84mks
#define MSR_PKGC7_IRTL			0x60C		// RW time limit to go C7
//MSR 0000060C                                      0000-0000-0000-8854

// Sandy Bridge (SNB) adds support for additional MSRs
#define MSR_PKG_C7_RESIDENCY		0x3FA
#define MSR_CORE_C7_RESIDENCY		0x3FE
#define MSR_PKG_C2_RESIDENCY		0x60D		// same as TSC but in C2 only

#define MSR_PKG_RAPL_POWER_LIMIT	0x610		//MSR 00000610  0000-A580-0000-8960
#define MSR_PKG_ENERGY_STATUS		0x611		//MSR 00000611  0000-0000-3212-A857
#define MSR_PKG_POWER_INFO		0x614		//MSR 00000614  0000-0000-01E0-02F8

// Sandy Bridge IA (Core) domain MSR's.
#define MSR_PP0_POWER_LIMIT		0x638
#define MSR_PP0_ENERGY_STATUS		0x639
#define MSR_PP0_POLICY			0x63A
#define MSR_PP0_PERF_STATUS		0x63B

// Sandy Bridge Uncore (IGPU) domain MSR's (Not on JakeTown).
#define MSR_PP1_POWER_LIMIT		0x640
#define MSR_PP1_ENERGY_STATUS	0x641
#define MSR_PP1_POLICY			0x642

// JakeTown only Memory MSR's.
#define MSR_PKG_PERF_STATUS		0x613 
#define MSR_DRAM_POWER_LIMIT	    	0x618
#define MSR_DRAM_ENERGY_STATUS		0x619
#define MSR_DRAM_PERF_STATUS		0x61B
#define MSR_DRAM_POWER_INFO		0x61C

// Ivy Bridge
#define MSR_CONFIG_TDP_NOMINAL		0x648
#define MSR_CONFIG_TDP_LEVEL1		0x649
#define MSR_CONFIG_TDP_LEVEL2		0x64A
#define MSR_CONFIG_TDP_CONTROL		0x64B		// write once to lock
#define MSR_TURBO_ACTIVATION_RATIO	0x64C

// Haswell (HSW) adds support for additional MSRs
#define MSR_PKG_C8_RESIDENCY            0x630
#define MSR_PKG_C9_RESIDENCY            0x631
#define MSR_PKG_C10_RESIDENCY           0x632

// Skylake (SKL) adds support for additional MSRs
#define MSR_PKG_WEIGHTED_CORE_C0_RES    0x658
#define MSR_PKG_ANY_CORE_C0_RES         0x659
#define MSR_PKG_ANY_GFXE_C0_RES         0x65A
#define MSR_PKG_BOTH_CORE_GFXE_C0_RES   0x65B

/* AMD Defined MSRs */
#define MSR_K6_EFER			0xC0000080	// extended feature register
#define MSR_K6_STAR			0xC0000081	// legacy mode SYSCALL target
#define MSR_K6_WHCR			0xC0000082	// long mode SYSCALL target
#define MSR_K6_UWCCR			0xC0000085
#define MSR_K6_EPMR			0xC0000086
#define MSR_K6_PSOR			0xC0000087
#define MSR_K6_PFIR			0xC0000088

#define MSR_K7_EVNTSEL0			0xC0010000
#define MSR_K7_PERFCTR0			0xC0010004
#define MSR_K7_HWCR			0xC0010015
#define MSR_K7_CLK_CTL			0xC001001b
#define MSR_K7_FID_VID_CTL		0xC0010041

#define AMD_K8_PERF_STS 0xC0010042
#define AMD_PSTATE_LIMIT 0xC0010061 // max enabled p-state (msr >> 4) & 7
#define AMD_PSTATE_CONTROL 0xC0010062 // switch to p-state
#define AMD_PSTATE0_STS 0xC0010064
#define AMD_COFVID_STS 0xC0010071 // current p-state (msr >> 16) & 7

#define MSR_AMD_MPERF			0x000000E7
#define MSR_AMD_APERF			0x000000E8


#define DEFAULT_FSB			100000          /* for now, hardcoding 100MHz for old CPUs */

// DFE: This constant comes from older xnu:
#define CLKNUM				1193182		/* formerly 1193167 */

/* CPU Features */
#define CPU_FEATURE_MMX			0x00000001		// MMX Instruction Set
#define CPU_FEATURE_SSE			0x00000002		// SSE Instruction Set
#define CPU_FEATURE_SSE2		0x00000004		// SSE2 Instruction Set
#define CPU_FEATURE_SSE3		0x00000008		// SSE3 Instruction Set
#define CPU_FEATURE_SSE41		0x00000010		// SSE41 Instruction Set
#define CPU_FEATURE_SSE42		0x00000020		// SSE42 Instruction Set
#define CPU_FEATURE_EM64T		0x00000040		// 64Bit Support
#define CPU_FEATURE_HTT			0x00000080		// HyperThreading
#define CPU_FEATURE_MOBILE		0x00000100		// Mobile CPU
#define CPU_FEATURE_MSR			0x00000200		// MSR Support

/* SMBIOS Memory Types */ 
#define SMB_MEM_TYPE_UNDEFINED		0
#define SMB_MEM_TYPE_OTHER		1
#define SMB_MEM_TYPE_UNKNOWN		2
#define SMB_MEM_TYPE_DRAM		3
#define SMB_MEM_TYPE_EDRAM		4
#define SMB_MEM_TYPE_VRAM		5
#define SMB_MEM_TYPE_SRAM		6
#define SMB_MEM_TYPE_RAM		7
#define SMB_MEM_TYPE_ROM		8
#define SMB_MEM_TYPE_FLASH		9
#define SMB_MEM_TYPE_EEPROM		10
#define SMB_MEM_TYPE_FEPROM		11
#define SMB_MEM_TYPE_EPROM		12
#define SMB_MEM_TYPE_CDRAM		13
#define SMB_MEM_TYPE_3DRAM		14
#define SMB_MEM_TYPE_SDRAM		15
#define SMB_MEM_TYPE_SGRAM		16
#define SMB_MEM_TYPE_RDRAM		17
#define SMB_MEM_TYPE_DDR		18
#define SMB_MEM_TYPE_DDR2		19
#define SMB_MEM_TYPE_FBDIMM		20
#define SMB_MEM_TYPE_DDR3		24			// Supported in 10.5.6+ AppleSMBIOS
#define SMB_MEM_TYPE_DDR4		26

/* Memory Configuration Types */ 
#define SMB_MEM_CHANNEL_UNKNOWN		0
#define SMB_MEM_CHANNEL_SINGLE		1
#define SMB_MEM_CHANNEL_DUAL		2
#define SMB_MEM_CHANNEL_TRIPLE		3

/* Maximum number of ram slots */
#define MAX_RAM_SLOTS			8

#define RAM_SLOT_ENUMERATOR		{0, 2, 4, 1, 3, 5, 6, 8, 10, 7, 9, 11}

/* Maximum number of SPD bytes */
#define MAX_SPD_SIZE			256

/* Size of SMBIOS UUID in bytes */
#define UUID_LEN			16

typedef struct _RamSlotInfo_t
{
	uint32_t		ModuleSize;			// Size of Module in MB
	uint32_t		Frequency;			// in Mhz
	const char*		Vendor;
	const char*		PartNo;
	const char*		SerialNo;
	char*			spd;				// SPD Dump
	bool			InUse;
	uint8_t			Type;
	uint8_t			BankConnections;		// table type 6, see (3.3.7)
	uint8_t			BankConnCnt;
} RamSlotInfo_t;

//==============================================================================

typedef struct _PlatformInfo_t
{
	struct CPU {
		uint32_t		Features;		// CPU Features like MMX, SSE2, VT, MobileCPU
		uint32_t		Vendor;			// Vendor
		uint32_t		CoresPerPackage;
		uint32_t		LogicalPerPackage;
		uint32_t		Signature;		// Processor Signature
		uint32_t		Stepping;		// Stepping
		uint32_t		Model;			// Model
		//uint32_t		Type;			// Processor Type
		uint32_t		ExtModel;		// Extended Model
		uint32_t		Family;			// Family
		uint32_t		ExtFamily;		// Extended Family
		uint32_t		NoCores;		// No Cores per Package
		uint32_t		NoThreads;		// Threads per Package
		uint8_t			MaxCoef;		// Max Multiplier
		uint8_t			MaxDiv;			// Min Multiplier
		uint8_t			CurrCoef;		// Current Multiplier
		uint8_t			CurrDiv;
		uint64_t		TSCFrequency;		// TSC Frequency Hz
		uint64_t		FSBFrequency;		// FSB Frequency Hz
		uint64_t		CPUFrequency;		// CPU Frequency Hz
		uint32_t		MaxRatio;		// Max Bus Ratio
		uint32_t		MinRatio;		// Min Bus Ratio
		char			BrandString[48];	// 48 Byte Branding String
		uint32_t		CPUID[CPUID_MAX][4];	// CPUID 0..4, 80..81 Raw Values

	} CPU;

	struct DMI
	{
		int			MaxMemorySlots;		// number of memory slots populated by SMBIOS
		int			CntMemorySlots;		// number of memory slots counted
		int			MemoryModules;		// number of memory modules installed
		int			DIMM[MAX_RAM_SLOTS];	// Information and SPD mapping for each slot
	} DMI;

	struct RAM
	{
		uint64_t		Frequency;		// Ram Frequency
		uint32_t		Divider;		// Memory divider
		uint8_t			CAS;			// CAS 1/2/2.5/3/4/5/6/7
		uint8_t			TRC;
		uint8_t			TRP;
		uint8_t			RAS;
		uint8_t			Channels;		// Channel Configuration Single,Dual, Triple or Quad
		uint8_t			NoSlots;		// Maximum no of slots available
		uint8_t			Type;			// Standard SMBIOS v2.5 Memory Type
		RamSlotInfo_t	DIMM[MAX_RAM_SLOTS];		// Information about each slot
	} RAM;

	uint8_t				Type;			// system-type: 1=Desktop, 2=Portable, 3=Workstation... according ACPI2.0 (FACP: PM_Profile)
	uint8_t				*UUID;			// system-id (SMBIOS Table 1: system uuid)
	uint32_t			HWSignature;		// machine-signature (FACS: Hardware Signature)
} PlatformInfo_t;

extern PlatformInfo_t Platform;

#endif /* !__LIBSAIO_PLATFORM_H */
