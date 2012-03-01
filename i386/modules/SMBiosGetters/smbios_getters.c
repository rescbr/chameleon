/*
 * Add (c) here
 *
 * Copyright .... All rights reserved.
 *
 */

#include "smbios_getters.h"
#include "modules.h"

#ifndef DEBUG_SMBIOS
#define DEBUG_SMBIOS 0
#endif

#if DEBUG_SMBIOS
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif
static uint16_t simpleGetSMBOemProcessorType(void);


bool getProcessorInformationExternalClock(returnType *value)
{
	value->word = (uint16_t)(get_env(envFSBFreq)/1000000);
	return true;
}

bool getProcessorInformationMaximumClock(returnType *value)
{
	// Note: it seems that AppleSMBIOS use the maximum clock to set the cpu clock
	// that is showed in "About this mac" or in the System Information. 
	// in my opinion the current clock should be used for this.
	// value->word = get_env(envTSCFreq)/1000000;
	
	value->word = (uint16_t)(get_env(envCPUFreq)/1000000);
	return true;
}

bool getProcessorInformationCurrentClock(returnType *value)
{
	value->word = (uint16_t)(get_env(envCPUFreq)/1000000);
	return true;
}

bool getSMBOemProcessorBusSpeed(returnType *value)
{
	if (get_env(envVendor) == CPUID_VENDOR_INTEL) 
	{		
		switch (get_env(envFamily)) 
		{
			case 0x06:
			{
				switch (get_env(envModel))
				{
                    case CPUID_MODEL_BANIAS:	// Banias		0x09
                    case CPUID_MODEL_DOTHAN:	// Dothan		0x0D
					case CPUID_MODEL_YONAH:		// Yonah		0x0E
					case CPUID_MODEL_MEROM:		// Merom		0x0F
					case CPUID_MODEL_PENRYN:		// Penryn		0x17
					case CPUID_MODEL_ATOM:		// Atom 45nm	0x1C
						return false;
                        
					case 0x19:					// Intel Core i5 650 @3.20 Ghz
					case CPUID_MODEL_NEHALEM:		// Intel Core i7 LGA1366 (45nm)
					case CPUID_MODEL_FIELDS:		// Intel Core i5, i7 LGA1156 (45nm)
					case CPUID_MODEL_DALES:		// Intel Core i5, i7 LGA1156 (45nm) ???
					case CPUID_MODEL_DALES_32NM:	// Intel Core i3, i5, i7 LGA1156 (32nm)
					case CPUID_MODEL_WESTMERE:	// Intel Core i7 LGA1366 (32nm) 6 Core
					case CPUID_MODEL_NEHALEM_EX:	// Intel Core i7 LGA1366 (45nm) 6 Core ???
					case CPUID_MODEL_WESTMERE_EX:	// Intel Core i7 LGA1366 (45nm) 6 Core ???
					case CPUID_MODEL_SANDYBRIDGE:
					case CPUID_MODEL_JAKETOWN:
					{
						// thanks to dgobe for i3/i5/i7 bus speed detection
						int nhm_bus = 0x3F;
						static long possible_nhm_bus[] = {0xFF, 0x7F, 0x3F};
						unsigned long did, vid;
						unsigned int i;
						
						// Nehalem supports Scrubbing
						// First, locate the PCI bus where the MCH is located
						for(i = 0; i < sizeof(possible_nhm_bus); i++)
						{
							vid = pci_config_read16(PCIADDR(possible_nhm_bus[i], 3, 4), 0x00);
							did = pci_config_read16(PCIADDR(possible_nhm_bus[i], 3, 4), 0x02);
							vid &= 0xFFFF;
							did &= 0xFF00;
							
							if(vid == 0x8086 && did >= 0x2C00)
								nhm_bus = possible_nhm_bus[i]; 
						}
						
						unsigned long qpimult, qpibusspeed;
						qpimult = pci_config_read32(PCIADDR(nhm_bus, 2, 1), 0x50);
						qpimult &= 0x7F;
						DBG("qpimult %d\n", qpimult);
						qpibusspeed = (qpimult * 2 * (get_env(envFSBFreq)/1000000));
						// Rek: rounding decimals to match original mac profile info
						if (qpibusspeed%100 != 0)qpibusspeed = ((qpibusspeed+50)/100)*100;
						DBG("qpibusspeed %d\n", qpibusspeed);
						value->word = qpibusspeed;
						return true;
					}
					default:
						break; //Unsupported CPU type
				}
			}
			default:
				break; 
		}
	}
	return false;
}

static uint16_t simpleGetSMBOemProcessorType(void)
{
    uint8_t ncores = (uint8_t)get_env(envNoCores);
	if (ncores >= 4) 
	{
		return 0x0501;	// Quad-Core Xeon
	}
	if (((ncores == 1) || (ncores == 2)) && !(get_env(envExtFeatures)& CPUID_EXTFEATURE_EM64T)) 
	{
		return 0x0201;	// Core Solo / Duo
	};
	
	return 0x0301;		// Core 2 Solo / Duo
}

bool getSMBOemProcessorType(returnType *value)
{
	static bool done = false;		
    
	value->word = simpleGetSMBOemProcessorType();
    
    char * BrandString = (char*)get_env_ptr(envBrandString);
    
	if (get_env(envVendor) == CPUID_VENDOR_INTEL) 
	{
		if (!done)
		{
			verbose("CPU is %s, family 0x%x, model 0x%x\n", BrandString, (uint32_t)get_env(envFamily), (uint32_t)get_env(envModel));
			done = true;
		}
		
		switch (get_env(envFamily)) 
		{
			case 0x06:
			{
				switch (get_env(envModel))
				{
                    case CPUID_MODEL_BANIAS:	// Banias		
                    case CPUID_MODEL_DOTHAN:	// Dothan		
					case CPUID_MODEL_YONAH:				// Yonah
					case CPUID_MODEL_MEROM:				// Merom
					case CPUID_MODEL_PENRYN:				// Penryn
					case CPUID_MODEL_ATOM:				// Intel Atom (45nm)
						return true;
                        
					case CPUID_MODEL_NEHALEM:				// Intel Core i7 LGA1366 (45nm)
						if (strstr(BrandString, "Core(TM) i7"))
                            value->word = 0x0701;	// Core i7                        
						return true;
                        
					case CPUID_MODEL_FIELDS:				// Lynnfield, Clarksfield, Jasper
						if (strstr(BrandString, "Core(TM) i5"))
							value->word = 0x601;		// Core i5
						else
							value->word = 0x701;		// Core i7
						return true;
                        
					case CPUID_MODEL_DALES:				// Intel Core i5, i7 LGA1156 (45nm) (Havendale, Auburndale)
						if (strstr(BrandString, "Core(TM) i5"))
							value->word = 0x601;		// Core i5
						else
							value->word = 0x0701;		// Core i7
						return true;
                        
					case CPUID_MODEL_SANDYBRIDGE:
					case CPUID_MODEL_DALES_32NM:			// Intel Core i3, i5, i7 LGA1156 (32nm) (Clarkdale, Arrandale)
						if (strstr(BrandString, "Core(TM) i3"))
                            value->word = 0x901;	// Core i3
						else if (strstr(BrandString, "Core(TM) i5"))
                            value->word = 0x601;	// Core i5
						else if (strstr(BrandString, "Core(TM) i7"))
                            value->word = 0x0701;	// Core i7
						/*else 
                         value->word = simpleGetSMBOemProcessorType();*/
						return true;
                        
                    case CPUID_MODEL_JAKETOWN:
					case CPUID_MODEL_WESTMERE:			// Intel Core i7 LGA1366 (32nm) 6 Core (Gulftown, Westmere-EP, Westmere-WS)
					case CPUID_MODEL_WESTMERE_EX:			// Intel Core i7 LGA1366 (45nm) 6 Core ???
						value->word = 0x0501;			// Core i7
						return true;
                        
					case 0x19:							// Intel Core i5 650 @3.20 Ghz
						value->word = 0x601;			// Core i5
						return true;
					default:
						break; //Unsupported CPU type
				}
			}
			default:
				break; 
		}
	}
	
	return false;
}

bool getSMBMemoryDeviceMemoryType(returnType *value)
{
	static int idx = -1;
	if (execute_hook("isMemoryRegistred", NULL, NULL, NULL, NULL, NULL, NULL) == EFI_SUCCESS) {
		int	map;
        
        int * DmiDimm = (int*)get_env_ptr(envDmiDimm);
        RamSlotInfo_t * RamDimm = (RamSlotInfo_t*)get_env_ptr(envRamDimm);
        
		idx++;
		if (idx < MAX_RAM_SLOTS)
		{
			map = DmiDimm[idx];
			if (RamDimm[map].InUse && RamDimm[map].Type != 0)
			{
				DBG("RAM Detected Type = %d\n", RamDimm[map].Type);
				value->byte = RamDimm[map].Type;
				return true;
			}
		}
	}
	value->byte = SMB_MEM_TYPE_DDR2;
	return true;
}

bool getSMBMemoryDeviceMemorySpeed(returnType *value)
{
	static int idx = -1;
	if (execute_hook("isMemoryRegistred", NULL, NULL, NULL, NULL, NULL, NULL) == EFI_SUCCESS) {
		int	map;
        
        int * DmiDimm = (int*)get_env_ptr(envDmiDimm);
        RamSlotInfo_t * RamDimm = (RamSlotInfo_t*)get_env_ptr(envRamDimm);
        
		idx++;
		if (idx < MAX_RAM_SLOTS)
		{
			map = DmiDimm[idx];
			if (RamDimm[map].InUse && RamDimm[map].Frequency != 0)
			{
				DBG("RAM Detected Freq = %d Mhz\n", RamDimm[map].Frequency);
				value->dword = RamDimm[map].Frequency;
				return true;
			}
		}
	}
	value->dword = 800;
	return true;
}

bool getSMBMemoryDeviceManufacturer(returnType *value)
{
	static int idx = -1;
	if (execute_hook("isMemoryRegistred", NULL, NULL, NULL, NULL, NULL, NULL) == EFI_SUCCESS) {
		int	map;
        
        int * DmiDimm = (int*)get_env_ptr(envDmiDimm);
        RamSlotInfo_t * RamDimm = (RamSlotInfo_t*)get_env_ptr(envRamDimm);
        
		idx++;
		if (idx < MAX_RAM_SLOTS)
		{
			map = DmiDimm[idx];
			if (RamDimm[map].InUse && strlen(RamDimm[map].Vendor) > 0)
			{
				DBG("RAM Detected Vendor[%d]='%s'\n", idx, RamDimm[map].Vendor);
				value->string = RamDimm[map].Vendor;
				return true;
			}
		}
	}
	value->string = "N/A";
	return true;
}

bool getSMBMemoryDeviceSerialNumber(returnType *value)
{
	static int idx = -1;
	if (execute_hook("isMemoryRegistred", NULL, NULL, NULL, NULL, NULL, NULL) == EFI_SUCCESS) {
		int	map;
        
        int * DmiDimm = (int*)get_env_ptr(envDmiDimm);
        RamSlotInfo_t * RamDimm = (RamSlotInfo_t*)get_env_ptr(envRamDimm);
        
		idx++;
		if (idx < MAX_RAM_SLOTS)
		{
			map = DmiDimm[idx];
			if (RamDimm[map].InUse && strlen(RamDimm[map].SerialNo) > 0)
			{
				DBG("name = %s, map=%d,  RAM Detected SerialNo[%d]='%s'\n", name ? name : "", 
                    map, idx, RamDimm[map].SerialNo);
				value->string = RamDimm[map].SerialNo;
				return true;
			}
		}
	}
	value->string = "N/A";
	return true;
}

bool getSMBMemoryDevicePartNumber(returnType *value)
{
	static int idx = -1;
	if (execute_hook("isMemoryRegistred", NULL, NULL, NULL, NULL, NULL, NULL) == EFI_SUCCESS) {
		int	map;
        
        int * DmiDimm = (int*)get_env_ptr(envDmiDimm);
        RamSlotInfo_t * RamDimm = (RamSlotInfo_t*)get_env_ptr(envRamDimm);
        
		idx++;
		if (idx < MAX_RAM_SLOTS)
		{
			map = DmiDimm[idx];
			if (RamDimm[map].InUse && strlen(RamDimm[map].PartNo) > 0)
			{
				DBG("Ram Detected PartNo[%d]='%s'\n", idx, RamDimm[map].PartNo);
				value->string = RamDimm[map].PartNo;
				return true;
			}
		}
	}
	value->string = "N/A";
	return true;
}
