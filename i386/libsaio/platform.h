/*
 *  platform.h
 *  AsereBLN: reworked and extended
 *
 */

#ifndef __LIBSAIO_PLATFORM_H
#define __LIBSAIO_PLATFORM_H

#include "libsaio.h"
#include "cpuid.h"
#include "cpu_data.h"

#define bitmask(h,l)		((_Bit(h)|(_Bit(h)-1)) & ~(_Bit(l)-1))
#define bitfield(x,h,l)		(((x) & bitmask(h,l)) >> l)

#define quad(hi,lo)         (((uint64_t)(hi)) << 32 | (lo))

/* Only for 32bit values */
#define bit32(n)		(1U << (n))
#define bitmask32(h,l)		((bit32(h)|(bit32(h)-1)) & ~(bit32(l)-1))
#define bitfield32(x,h,l)	((((x) & bitmask32(h,l)) >> l))

/* Additional models supported by Chameleon (NOT SUPPORTED BY THE APPLE'S ORIGINAL KERNEL) */
#define CPUID_MODEL_BANIAS          0x09
#define CPUID_MODEL_DOTHAN          0x0D
#define CPUID_MODEL_ATOM			0x1C

/* CPUID Vendor */
#define CPUID_VENDOR_INTEL      0x756E6547
#define CPUID_VENDOR_AMD        0x68747541

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

typedef struct _RamSlotInfo_t {
    uint32_t		ModuleSize;						// Size of Module in MB
    uint32_t		Frequency; // in Mhz
    const char*		Vendor;
    const char*		PartNo;
    const char*		SerialNo;
    char*			spd;							// SPD Dump
    bool			InUse;
    uint8_t			Type;
    uint8_t			BankConnections; // table type 6, see (3.3.7)
    uint8_t			BankConnCnt;
    
} RamSlotInfo_t;

#define envVendor           "boot.cpu.vendor"
#define envFamily           "boot.cpu.family"

#define envCPUIDMaxBasic    "boot.cpu.max_basic"
#define envCPUIDMaxExt      "boot.cpu.max_ext"   
#define envMicrocodeVersion "boot.cpu.microcode_version"   
#define envSignature        "boot.cpu.signature"   
#define envStepping         "boot.cpu.stepping"   
#define envModel            "boot.cpu.model" 
#define envBrandString      "boot.cpu.brand_string"   
#define envNoThreads        "boot.cpu.logical_per_package"   
#define envNoCores          "boot.cpu.cores_per_package"
#define envExtModel         "boot.cpu.extmodel"   
#define envExtFamily        "boot.cpu.extfamily"   
#define envBrand            "boot.cpu.brand"   
#define envFeatures         "boot.cpu.feature_bits"
#define envExtFeatures      "boot.cpu.extfeature_bits"
#define envCacheSize        "boot.cpu.cache_size"               //cache_size[LCACHE_MAX]
#define envCacheLinesize    "boot.cpu.cache_linesize"
#define envLeaf7Features    "boot.cpu.cpuid_leaf7_features"
#define cpuid_features()        ((uint32_t)get_env(envFeatures))
#define cpuid_leaf7_features()  ((uint32_t)get_env(envLeaf7Features))
#define envTSC__            "boot.cpu.__tsc"

#define envSubCstates       "boot.cpu.mwait.sub_Cstates"
#define envExtensions       "boot.cpu.mwait.extensions"   

#define envDynamicAcceleration  "boot.cpu.thermal.dynamic_acceleration"
#define envInvariantAPICTimer   "boot.cpu.thermal.invariant_APIC_timer"
#define envFineGrainClockMod    "boot.cpu.thermal.fine_grain_clock_mod"   

#define envIsMobile             "boot.hw.is_mobile"   
#define envMaxCoef              "boot.hw.maxcoef"   
#define envMaxDiv               "boot.hw.maxdiv"   
#define envCurrCoef             "boot.hw.currcoef"   
#define envCurrDiv              "boot.hw.currdiv"   
#define envTSCFreq              "boot.hw.tscfrequency"   
#define envFSBFreq              "boot.hw.busfrequency"   
#define envCPUFreq              "boot.hw.cpufrequency"
#define envIsServer             "boot.hw.is_server"
#define envHardwareSignature    "boot.hw.signature"
#define envType                 "boot.hw.type"
#define envUUID                 "boot.hw.uuid_ptr"
#define envSysId                "boot.hw.sysid"
#define envgBIOSDev             "boot.hw.bios_device"
#define envgDeviceCount         "boot.hw.device_count"
#define envarchCpuType			"boot.hw.cputype"

#define envPCIRootDev           "boot.hw.pci_root_dev"

#define envgHaveKernelCache		"boot.kern.HaveKernelCache"
#define envAdler32				"boot.kern.adler32"
#define envkCacheFile           "boot.kern.kernelcache"
#define envMKextName            "boot.kern.mkextcache"
#define envArgCntRemaining      "boot.kern.argCount_remaining"
#define envgBootArgs			"boot.kern.boot_args"

#define envgBootMode            "boot.options.boot_mode"
#define envSysConfigValid       "boot.options.sysconfing_valid"
#define envgOverrideKernel      "boot.options.kernel_overide"
#define envgEnableCDROMRescan   "boot.options.rescan_cdrom"
#define envgScanSingleDrive     "boot.options.single_drive"
#define envShouldboot           "boot.options.should_boot"
#define envgVerboseMode         "boot.options.boot_verbose"

#define envDriverExtSpec        "boot.drivers.extspec"
#define envDriverSpec           "boot.drivers.spec"
#define envDriverFileSpec       "boot.drivers.filespec"
#define envDriverTempSpec       "boot.drivers.tempspec"
#define envDriverFileName       "boot.drivers.filename"

#define envBootBanner            "boot.ui.banner"
#define envBootPrompt            "boot.ui.prompt"
#define envBootRescanPrompt      "boot.ui.promptrescan"

#define envgMenuRow              "boot.ui.menu_row"
#define envgMenuHeight           "boot.ui.menu_height"
#define envgMenuItemCount        "boot.ui.menu_itemcount"
#define envgMenuTop              "boot.ui.menu_top"
#define envgMenuBottom           "boot.ui.menu_bottom"
#define envgMenuSelection        "boot.ui.menu_selection"
#define envgMenuStart            "boot.ui.menu_start"
#define envgMenuEnd              "boot.ui.menu_end"

#define envConsoleErr			"boot.console.stderr"
#define envErrno                "boot.errno"

#define envDeviceNumber         "boot.dev.efi.devcount"
#define envEFIString            "boot.dev.efi.efistring"

#define envgBootFileType		"boot.disk.Bootfiletype"
#define envHFSLoadVerbose		"boot.disk.HFSLoadVerbose"
#define envgFSLoadAddress		"boot.disk.FSLoadAddress"
#define envgBIOSBootVolume		"boot.disk.BIOSBootVolume"
#define envgBootVolume			"boot.disk.BootVolume"

#define envConvMem               "boot.memmap.Conventional"
#define envExtMem                "boot.memmap.Extended"
#define envMemoryMap             "boot.memmap.Address"
#define envMemoryMapCnt          "boot.memmap.Count"
#define envMemoryMapNode		 "boot.memmap.devNode"

#define envRamFrequency         "boot.ram.frequency"
#define envRamCas               "boot.ram.tCAS"
#define envRamTrc               "boot.ram.tRCD"
#define envRamRas               "boot.ram.tRAS"
#define envRamTrp               "boot.ram.tRP"
#define envRamChannels          "boot.ram.channels"
#define envRamType              "boot.ram.type"
#define envRamDimm              "boot.ram.dimm"

#define envDMIMemModules        "boot.dmi.memory_modules"
#define envDMIMaxMemorySlots    "boot.dmi.max_slots"
#define envDMICntMemorySlots    "boot.dmi.slots_count"
#define envDmiDimm              "boot.dmi.dimm"

#define envVBEModeInfoBlock		"boot.video.VBEModeInfoBlock"

/* helpers ... */
#define set_errno(x) safe_set_env(envErrno,x)
#define get_errno()  ((int)get_env(envErrno))

#if UNUSED
typedef struct _PlatformInfo_t {
	
    struct CPU_t {
        uint64_t		Features;		// CPU Features like MMX, SSE2, VT ...
        uint64_t		ExtFeatures;    // CPU Extended Features like SYSCALL, XD, EM64T, LAHF ...
        uint32_t		Vendor;			// Vendor
        uint32_t		Signature;		// Signature
        uint8_t         Stepping;		// Stepping
        uint8_t         Model;			// Model
        uint8_t         ExtModel;		// Extended Model
        uint8_t         Family;			// Family
        uint8_t         ExtFamily;		// Extended Family
        uint32_t		NoCores;		// No Cores per Package
        uint32_t		NoThreads;		// Threads per Package
        uint8_t			MaxCoef;		// Max Multiplier
        uint8_t			MaxDiv;
        uint8_t			CurrCoef;		// Current Multiplier
        uint8_t			CurrDiv;
        uint64_t		TSCFrequency;		// TSC Frequency Hz
        uint64_t		FSBFrequency;		// FSB Frequency Hz
        uint64_t		CPUFrequency;		// CPU Frequency Hz
        char			BrandString[48];	// 48 Byte Branding String
        uint8_t         Brand; 
        uint32_t		MicrocodeVersion;   // The microcode version number a.k.a. signature a.k.a. BIOS ID 
        
        bool           isMobile;        
        bool		   isServer;			// Unlike isMobile, if this value is set it will disable all kind of detection and enforce "Server" as platform (must be set by user)
        
        boolean_t	dynamic_acceleration;
        boolean_t	invariant_APIC_timer;
        boolean_t	fine_grain_clock_mod;
        
        uint32_t    cpuid_max_basic;
		uint32_t    cpuid_max_ext;
        uint32_t	sub_Cstates;
        uint32_t    extensions;
        
    } CPU;
    
    struct RAM_t {
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
        int			MaxMemorySlots;		// number of memory slots polulated by SMBIOS
        int			CntMemorySlots;		// number of memory slots counted
        int			MemoryModules;		// number of memory modules installed
        int			DIMM[MAX_RAM_SLOTS];	// Information and SPD mapping for each slot
    } DMI;
    
	uint8_t				Type;			// System Type: 1=Desktop, 2=Portable... according ACPI2.0 (FACP: PreferredProfile)
	uint8_t				*UUID;          // SMBios UUID
	uint32_t			hardware_signature;
	int8_t				sysid[16];
    
} PlatformInfo_t;
#endif

#endif /* !__LIBSAIO_PLATFORM_H */
