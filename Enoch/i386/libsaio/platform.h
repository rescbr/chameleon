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

/* CPUID Vendor */
#define CPUID_VENDOR_INTEL      0x756E6547
#define CPUID_VENDOR_AMD        0x68747541

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
#define CPUID_88			9
#define CPUID_MAX			10

#define CPU_MODEL_ANY			0x00
#define CPU_MODEL_UNKNOWN		0x01
#define CPU_MODEL_PRESCOTT		0x03			// Celeron D, Pentium 4 (90nm)
#define CPU_MODEL_NOCONA		0x04			// Xeon Nocona/Paxville, Irwindale (90nm)
#define CPU_MODEL_PRESLER		0x06			// Pentium 4, Pentium D (65nm)
#define CPU_MODEL_PENTIUM_M		0x09			// Banias Pentium M (130nm)
#define CPU_MODEL_DOTHAN		0x0D			// Dothan Pentium M, Celeron M (90nm)
#define CPU_MODEL_YONAH			0x0E			// Sossaman, Yonah
#define CPU_MODEL_MEROM			0x0F			// Allendale, Conroe, Kentsfield, Woodcrest, Clovertown, Tigerton, Merom
#define CPU_MODEL_CONROE		0x0F			// 
#define CPU_MODEL_CELERON		0x16			// Merom, Conroe (65nm)
#define CPU_MODEL_PENRYN		0x17			// Wolfdale, Yorkfield, Harpertown, Penryn
#define CPU_MODEL_WOLFDALE		0x17			// 
#define CPU_MODEL_NEHALEM		0x1A			// Bloomfield. Nehalem-EP, Nehalem-WS, Gainestown
#define CPU_MODEL_ATOM			0x1C			// Pineview, Bonnell
#define CPU_MODEL_XEON_MP		0x1D			// MP 7400
#define CPU_MODEL_FIELDS		0x1E			// Lynnfield, Clarksfield, Jasper Forest
#define CPU_MODEL_DALES			0x1F			// Havendale, Auburndale
#define CPU_MODEL_DALES_32NM		0x25			// Clarkdale, Arrandale
#define CPU_MODEL_ATOM_SAN		0x26			// Lincroft
#define CPU_MODEL_LINCROFT		0x27			// Bonnell
#define CPU_MODEL_SANDYBRIDGE		0x2A			// Sandy Bridge
#define CPU_MODEL_WESTMERE		0x2C			// Gulftown, Westmere-EP, Westmere-WS
#define CPU_MODEL_JAKETOWN		0x2D			// Sandy Bridge-E, Sandy Bridge-EP
#define CPU_MODEL_NEHALEM_EX		0x2E			// Beckton
#define CPU_MODEL_WESTMERE_EX		0x2F			// Westmere-EX
//#define CPU_MODEL_BONNELL_ATOM	0x35			// Bonnell
#define CPU_MODEL_ATOM_2000		0x36			// Cedarview / Saltwell
#define CPU_MODEL_SILVERMONT		0x37			// Atom Silvermont
#define CPU_MODEL_IVYBRIDGE		0x3A			// Ivy Bridge
#define CPU_MODEL_HASWELL		0x3C			// Haswell DT
#define CPU_MODEL_BROADWELL		0x3D			// Core M, Broadwell / Core-AVX2
#define CPU_MODEL_IVYBRIDGE_XEON	0x3E			// Ivy Bridge Xeon
#define CPU_MODEL_HASWELL_SVR		0x3F			// Haswell Server
//#define CPU_MODEL_HASWELL_H		0x??			// Haswell H
#define CPU_MODEL_HASWELL_ULT		0x45			// Haswell ULT
#define CPU_MODEL_CRYSTALWELL		0x46			// Crystal Well
// 4A silvermont / atom
#define CPU_MODEL_AVOTON		0x4D			// Silvermont/Avoton Atom C2000
// 4E Core???
#define CPU_MODEL_BRODWELL_SVR		0x4F			// Broadwell Server
#define CPU_MODEL_BRODWELL_MSVR		0x56			// Broadwell Micro Server
// 5A silvermont / atom
// 5D silvermont / atom

/* Unknown CPU */
#define CPU_STRING_UNKNOWN	"Unknown CPU Type"
#define bit(n)			(1ULL << (n))
#define bitmask(h,l)		((bit(h)|(bit(h)-1)) & ~(bit(l)-1))
#define bitfield(x,h,l)		(((x) & bitmask(h,l)) >> l)


/*
 * The CPUID_FEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 1: 
 */
#define CPUID_FEATURE_FPU       _Bit(0)   /* Floating point unit on-chip */
#define CPUID_FEATURE_VME       _Bit(1)   /* Virtual Mode Extension */
#define CPUID_FEATURE_DE        _Bit(2)   /* Debugging Extension */
#define CPUID_FEATURE_PSE       _Bit(3)   /* Page Size Extension */
#define CPUID_FEATURE_TSC       _Bit(4)   /* Time Stamp Counter */
#define CPUID_FEATURE_MSR       _Bit(5)   /* Model Specific Registers */
#define CPUID_FEATURE_PAE       _Bit(6)   /* Physical Address Extension */
#define CPUID_FEATURE_MCE       _Bit(7)   /* Machine Check Exception */
#define CPUID_FEATURE_CX8       _Bit(8)   /* CMPXCHG8B */
#define CPUID_FEATURE_APIC      _Bit(9)   /* On-chip APIC */
#define CPUID_FEATURE_SEP       _Bit(11)  /* Fast System Call */
#define CPUID_FEATURE_MTRR      _Bit(12)  /* Memory Type Range Register */
#define CPUID_FEATURE_PGE       _Bit(13)  /* Page Global Enable */
#define CPUID_FEATURE_MCA       _Bit(14)  /* Machine Check Architecture */
#define CPUID_FEATURE_CMOV      _Bit(15)  /* Conditional Move Instruction */
#define CPUID_FEATURE_PAT       _Bit(16)  /* Page Attribute Table */
#define CPUID_FEATURE_PSE36     _Bit(17)  /* 36-bit Page Size Extension */
#define CPUID_FEATURE_PSN       _Bit(18)  /* Processor Serial Number */
#define CPUID_FEATURE_CLFSH     _Bit(19)  /* CLFLUSH Instruction supported */
#define CPUID_FEATURE_DS        _Bit(21)  /* Debug Store */
#define CPUID_FEATURE_ACPI      _Bit(22)  /* Thermal monitor and Clock Ctrl */
#define CPUID_FEATURE_MMX       _Bit(23)  /* MMX supported */
#define CPUID_FEATURE_FXSR      _Bit(24)  /* Fast floating pt save/restore */
#define CPUID_FEATURE_SSE       _Bit(25)  /* Streaming SIMD extensions */
#define CPUID_FEATURE_SSE2      _Bit(26)  /* Streaming SIMD extensions 2 */
#define CPUID_FEATURE_SS        _Bit(27)  /* Self-Snoop */
#define CPUID_FEATURE_HTT       _Bit(28)  /* Hyper-Threading Technology */
#define CPUID_FEATURE_TM        _Bit(29)  /* Thermal Monitor (TM1) */
#define CPUID_FEATURE_PBE       _Bit(31)  /* Pend Break Enable */
 
#define CPUID_FEATURE_SSE3      _HBit(0)  /* Streaming SIMD extensions 3 */
#define CPUID_FEATURE_PCLMULQDQ _HBit(1)  /* PCLMULQDQ instruction */
#define CPUID_FEATURE_DTES64    _HBit(2)  /* 64-bit DS layout */
#define CPUID_FEATURE_MONITOR   _HBit(3)  /* Monitor/mwait */
#define CPUID_FEATURE_DSCPL     _HBit(4)  /* Debug Store CPL */
#define CPUID_FEATURE_VMX       _HBit(5)  /* VMX */
#define CPUID_FEATURE_SMX       _HBit(6)  /* SMX */
#define CPUID_FEATURE_EST       _HBit(7)  /* Enhanced SpeedsTep (GV3) */
#define CPUID_FEATURE_TM2       _HBit(8)  /* Thermal Monitor 2 */
#define CPUID_FEATURE_SSSE3     _HBit(9)  /* Supplemental SSE3 instructions */
#define CPUID_FEATURE_CID       _HBit(10) /* L1 Context ID */
#define CPUID_FEATURE_SEGLIM64  _HBit(11) /* 64-bit segment limit checking */
#define CPUID_FEATURE_FMA       _HBit(12) /* Fused-Multiply-Add support */
#define CPUID_FEATURE_CX16      _HBit(13) /* CmpXchg16b instruction */
#define CPUID_FEATURE_xTPR      _HBit(14) /* Send Task PRiority msgs */
#define CPUID_FEATURE_PDCM      _HBit(15) /* Perf/Debug Capability MSR */

#define CPUID_FEATURE_PCID      _HBit(17) /* ASID-PCID support */
#define CPUID_FEATURE_DCA       _HBit(18) /* Direct Cache Access */
#define CPUID_FEATURE_SSE4_1    _HBit(19) /* Streaming SIMD extensions 4.1 */
#define CPUID_FEATURE_SSE4_2    _HBit(20) /* Streaming SIMD extensions 4.2 */
#define CPUID_FEATURE_x2APIC    _HBit(21) /* Extended APIC Mode */
#define CPUID_FEATURE_MOVBE     _HBit(22) /* MOVBE instruction */
#define CPUID_FEATURE_POPCNT    _HBit(23) /* POPCNT instruction */
#define CPUID_FEATURE_TSCTMR    _HBit(24) /* TSC deadline timer */
#define CPUID_FEATURE_AES       _HBit(25) /* AES instructions */
#define CPUID_FEATURE_XSAVE     _HBit(26) /* XSAVE instructions */
#define CPUID_FEATURE_OSXSAVE   _HBit(27) /* XGETBV/XSETBV instructions */
#define CPUID_FEATURE_AVX1_0	_HBit(28) /* AVX 1.0 instructions */
#define CPUID_FEATURE_F16C	_HBit(29) /* Float16 convert instructions */
#define CPUID_FEATURE_RDRAND	_HBit(30) /* RDRAND instruction */
#define CPUID_FEATURE_VMM       _HBit(31) /* VMM (Hypervisor) present */

/*
 * Leaf 7, subleaf 0 additional features.
 * Bits returned in %ebx to a CPUID request with {%eax,%ecx} of (0x7,0x0}:
 */
#define CPUID_LEAF7_FEATURE_RDWRFSGS _Bit(0)	/* FS/GS base read/write */
#define CPUID_LEAF7_FEATURE_TSCOFF   _Bit(1)	/* TSC thread offset */
#define CPUID_LEAF7_FEATURE_BMI1     _Bit(3)	/* Bit Manipulation Instrs, set 1 */
#define CPUID_LEAF7_FEATURE_HLE      _Bit(4)	/* Hardware Lock Elision*/
#define CPUID_LEAF7_FEATURE_AVX2     _Bit(5)	/* AVX2 Instructions */
#define CPUID_LEAF7_FEATURE_SMEP     _Bit(7)	/* Supervisor Mode Execute Protect */
#define CPUID_LEAF7_FEATURE_BMI2     _Bit(8)	/* Bit Manipulation Instrs, set 2 */
#define CPUID_LEAF7_FEATURE_ENFSTRG  _Bit(9)	/* ENhanced Fast STRinG copy */
#define CPUID_LEAF7_FEATURE_INVPCID  _Bit(10)	/* INVPCID intruction, TDB */
#define CPUID_LEAF7_FEATURE_RTM      _Bit(11)	/* TBD */

/*
 * The CPUID_EXTFEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 0x80000001: 
 */
#define CPUID_EXTFEATURE_SYSCALL   _Bit(11)	/* SYSCALL/sysret */
#define CPUID_EXTFEATURE_XD	   _Bit(20)	/* eXecute Disable */

#define CPUID_EXTFEATURE_1GBPAGE   _Bit(26)	/* 1GB pages */
#define CPUID_EXTFEATURE_RDTSCP	   _Bit(27)	/* RDTSCP */
#define CPUID_EXTFEATURE_EM64T	   _Bit(29)	/* Extended Mem 64 Technology */

#define CPUID_EXTFEATURE_LAHF	   _HBit(0)	/* LAFH/SAHF instructions */

/*
 * The CPUID_EXTFEATURE_XXX values define 64-bit values
 * returned in %ecx:%edx to a CPUID request with %eax of 0x80000007: 
 */
#define CPUID_EXTFEATURE_TSCI      _Bit(8)	/* TSC Invariant */

#define	CPUID_CACHE_SIZE	16	/* Number of descriptor values */

#define CPUID_MWAIT_EXTENSION	_Bit(0)	/* enumeration of WMAIT extensions */
#define CPUID_MWAIT_BREAK	_Bit(1)	/* interrupts are break events	   */

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
#define MSR_PKGC3_IRTL          0x60A    /* RW time limit to go C3 */
// bit 15 = 1 -- the value valid for C-state PM
#define MSR_PKGC6_IRTL          0x60B    /* RW time limit to go C6 */
//MSR 0000060B                                      0000-0000-0000-8854
//Valid + 010=1024ns + 0x54=84mks
#define MSR_PKGC7_IRTL          0x60C    /* RW time limit to go C7 */
//MSR 0000060C                                      0000-0000-0000-8854
#define MSR_PKG_C2_RESIDENCY    0x60D   /* same as TSC but in C2 only */

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
#define SMB_MEM_TYPE_UNDEFINED	0
#define SMB_MEM_TYPE_OTHER		1
#define SMB_MEM_TYPE_UNKNOWN	2
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
	uint32_t		ModuleSize;					// Size of Module in MB
	uint32_t		Frequency;					// in Mhz
	const char*		Vendor;
	const char*		PartNo;
	const char*		SerialNo;
	char*			spd;						// SPD Dump
	bool			InUse;
	uint8_t			Type;
	uint8_t			BankConnections;			// table type 6, see (3.3.7)
	uint8_t			BankConnCnt;
} RamSlotInfo_t;

//==============================================================================

typedef struct _PlatformInfo_t
{
	struct CPU {
		uint32_t		Vendor;					// Vendor - char Vendor[16];
		char			BrandString[48];			// 48 Byte Branding String
		//uint16_t		Type;					// Type
		uint8_t			Family;					// Family
		uint8_t			Model;					// Model
		uint8_t			ExtModel;				// Extended Model
		uint8_t			ExtFamily;				// Extended Family
		uint8_t			Stepping;				// Stepping
		uint64_t		Features;				// CPU Features like MMX, SSE2, VT, MobileCPU
		uint64_t		ExtFeatures;
		uint32_t		CoresPerPackage;
		uint32_t		LogicalPerPackage;
		uint32_t		Signature;				// Processor Signature
		//uint8_t		Brand;
		//uint8_t		ProcessorFlag;

		uint32_t		NoCores;				// No Cores per Package
		uint32_t		NoThreads;				// Threads per Package

		//uint32_t		CacheSize[LCACHE_MAX];
		//uint32_t		CacheLineSize;

		//uint8_t		cache_info[64];				// list of cache descriptors

		uint8_t			MaxCoef;				// Max Multiplier
		uint8_t			MaxDiv;					// Min Multiplier
		uint8_t			CurrCoef;				// Current Multiplier
		uint8_t			CurrDiv;
		uint64_t		TSCFrequency;				// TSC Frequency Hz
		uint64_t		FSBFrequency;				// FSB Frequency Hz
		uint64_t		CPUFrequency;				// CPU Frequency Hz
		uint32_t		MaxRatio;				// Max Bus Ratio
		uint32_t		MinRatio;				// Min Bus Ratio
		uint32_t		CPUID[CPUID_MAX][4];			// CPUID 0..4, 80..81 Raw Values

		uint32_t		MCodeVersion;           // CPU Microcode version
	} CPU;

	struct RAM {
		uint64_t		Frequency;				// Ram Frequency
		uint32_t		Divider;				// Memory divider
		uint8_t			CAS;					// CAS 1/2/2.5/3/4/5/6/7
		uint8_t			TRC;					
		uint8_t			TRP;
		uint8_t			RAS;
		uint8_t			Channels;				// Channel Configuration Single,Dual or Triple
		uint8_t			NoSlots;				// Maximum no of slots available
		uint8_t			Type;					// Standard SMBIOS v2.5 Memory Type
		RamSlotInfo_t	DIMM[MAX_RAM_SLOTS];	// Information about each slot
	} RAM;

	struct DMI {
		int			MaxMemorySlots;		// number of memory slots populated by SMBIOS
		int			CntMemorySlots;		// number of memory slots counted
		int			MemoryModules;		// number of memory modules installed
		int			DIMM[MAX_RAM_SLOTS];	// Information and SPD mapping for each slot
	} DMI;

	uint8_t				Type;			// System Type: 1=Desktop, 2=Portable, 3=Workstation... according ACPI2.0 (FACP: PM_Profile)
	uint8_t				*UUID;
} PlatformInfo_t;

extern PlatformInfo_t Platform;

#endif /* !__LIBSAIO_PLATFORM_H */
