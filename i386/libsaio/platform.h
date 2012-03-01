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

/* Size of SMBIOS UUID in bytes */
#define UUID_LEN			16

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

#define envVendor           "Vendor"
#define envCPUIDMaxBasic    "max_basic"
#define envCPUIDMaxExt      "max_ext"   
#define envMicrocodeVersion "Microcode"   
#define envSignature        "Signature"   
#define envStepping         "Stepping"   
#define envModel            "Model" 

#define envFamily           "Family"   
#define envExtModel         "ExtModel"   
#define envExtFamily        "ExtFamily"   
#define envBrand            "Brand"   
#define envFeatures         "Feat"
#define envExtFeatures      "ExtFeat"
#define envSubCstates       "sub_Csta"   
#define envExtensions       "CPUIDext"   
#define envBrandString      "BrandStr"   

#define envDynamicAcceleration  "dynAcc"
#define envInvariantAPICTimer   "invAPIC"
#define envFineGrainClockMod    "fineGrain"   
#define envNoThreads            "NoThreads"   
#define envNoCores              "NoCores"   
#define envIsMobile             "isMobile"   
#define envMaxCoef              "MaxCoef"   
#define envMaxDiv               "MaxDiv"   
#define envCurrCoef             "CurrCoef"   
#define envCurrDiv              "CurrDiv"   
#define envTSCFreq              "TSCFreq"   
#define envFSBFreq              "FSBFreq"   
#define envCPUFreq              "CPUFreq"
#define envIsServer             "isServer"


#define envCurrCoef             "CurrCoef"   
#define envCurrDiv              "CurrDiv"   
#define envTSCFreq              "TSCFreq"   
#define envFSBFreq              "FSBFreq"   
#define envCPUFreq              "CPUFreq"

#define envHardwareSignature    "HdwSign"
#define envType                 "Type"
#define envUUID                 "UUID"
#define envSysId                "SysId"

#define envgBootMode            "gBootMode"
#define envgBIOSDev             "gBIOSDev"
#define envSysConfigValid       "IsSysConf"
#define envgOverrideKernel      "IsKover"
#define envgEnableCDROMRescan   "CDRescan"
#define envgScanSingleDrive     "1Drive"
#define envgDeviceCount         "DevCnt"
#define envShouldboot           "shldboot"


#define envDriverExtSpec        "DrvXSpec"
#define envDriverSpec           "DrvSpec"
#define envDriverFileSpec       "DrvFSpes"
#define envDriverTempSpec       "DrvTSpes"
#define envDriverFileName       "DrvFName"

#define envkCache               "kCache"
#define envMKextName            "MCache"

#define envBootBanner            "Banner"
#define envBootPrompt            "Prompt"
#define envBootRescanPrompt      "PromptBis"

#define envgMenuRow              "MenuRow"
#define envgMenuHeight           "MenuH"
#define envgMenuItemCount        "MenuCnt"
#define envgMenuTop              "MenuTop"
#define envgMenuBottom           "MenuB"
#define envgMenuSelection        "MenuSel"
#define envgMenuStart            "MenuStart"
#define envgMenuEnd              "MenuEnd"
#define envArgCntRemaining       "ArgCntRem"

#define envgBootArgs				"gBootArgs"

#define envConvMem                  "ConvMem"
#define envExtMem                   "ExtMem"
#define envMemoryMap                "MemoryMap"
#define envMemoryMapCnt             "MemMapCnt"


#define envRamFrequency         "RamFreq"
#define envRamCas               "tCAS"
#define envRamTrc               "tRCD"
#define envRamRas               "tRAS"
#define envRamTrp               "tRP"
#define envRamChannels          "RamChans"
#define envRamType              "RamType"
#define envRamCas               "tCAS"
#define envRamCas               "tCAS"
#define envRamCas               "tCAS"
#define envRamCas               "tCAS"
#define envDMIMemModules        "DmiMemMod"
#define envDMIMaxMemorySlots    "DmiMaxSlt"
#define envDMICntMemorySlots    "DmiCntSlt"
#define envRamDimm              "RamDimm"
#define envDmiDimm              "DmiDimm"

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

#ifdef ShowCurrentDate
#include "efi.h"
extern char * Date(void);
extern void rtc_time(EFI_TIME *time);
#endif

#endif /* !__LIBSAIO_PLATFORM_H */
