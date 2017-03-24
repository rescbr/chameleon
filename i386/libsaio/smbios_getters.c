/*
 * Add (c) here
 *
 * Copyright .... All rights reserved.
 *
 */

#include "config.h"
#include "smbios_getters.h"
#include "bootstruct.h"

#if DEBUG_SMBIOS
	#define DBG(x...)	printf(x)
#else
	#define DBG(x...)
#endif

#define XEON "Xeon"
#define CORE_M "Core(TM) M"
#define CORE_M3 "Core(TM) m3"
#define CORE_M5 "Core(TM) m5"
#define CORE_M7 "Core(TM) m7"
#define CORE_I3 "Core(TM) i3"
#define CORE_I5 "Core(TM) i5"
#define CORE_I7 "Core(TM) i7"

bool getProcessorInformationExternalClock(returnType *value)
{
	if (Platform.CPU.Vendor == CPUID_VENDOR_INTEL) // Intel
	{
		switch (Platform.CPU.Family)
		{
			case 0x06:
				switch (Platform.CPU.Model)
				{
					// set external clock to 0 for SANDY
					// removes FSB info from system profiler as on real mac's.
					case CPUID_MODEL_SANDYBRIDGE:
					case CPUID_MODEL_JAKETOWN:
					case CPUID_MODEL_IVYBRIDGE_XEON:
					case CPUID_MODEL_IVYBRIDGE:
					case CPUID_MODEL_HASWELL:
					case CPUID_MODEL_HASWELL_SVR:
					case CPUID_MODEL_HASWELL_ULT:
					case CPUID_MODEL_HASWELL_ULX:

						value->word = 0;
						break;
					default:
						value->word = (uint16_t)(Platform.CPU.FSBFrequency/1000000LL);
						break;
				}
				break;

			default:
				value->word = (uint16_t)(Platform.CPU.FSBFrequency/1000000LL);
				break;
		}
	}
	else
	{
		value->word = (uint16_t)(Platform.CPU.FSBFrequency/1000000LL);
	}

	return true;
}

bool getProcessorInformationMaximumClock(returnType *value)
{
	value->word = (uint16_t)(Platform.CPU.CPUFrequency/1000000LL);
	return true;
}

bool getSMBOemProcessorBusSpeed(returnType *value)
{
	if (Platform.CPU.Vendor == CPUID_VENDOR_INTEL) // Intel
	{
		switch (Platform.CPU.Family)
		{
			case 0x06:
				switch (Platform.CPU.Model)
				{
					case CPUID_MODEL_PENTIUM_M:
					case CPUID_MODEL_DOTHAN:	// Intel Pentium M
					case CPUID_MODEL_YONAH:		// Intel Mobile Core Solo, Duo
					case CPUID_MODEL_MEROM:		// Intel Mobile Core 2 Solo, Duo, Xeon 30xx, Xeon 51xx, Xeon X53xx, Xeon E53xx, Xeon X32xx
					case CPUID_MODEL_PENRYN:	// Intel Core 2 Solo, Duo, Quad, Extreme, Xeon X54xx, Xeon X33xx
					case CPUID_MODEL_ATOM:		// Intel Atom (45nm)
						return false;

					case 0x19:
					case CPUID_MODEL_NEHALEM:	// Intel Core i7, Xeon W35xx, Xeon X55xx, Xeon E55xx LGA1366 (45nm)
					case CPUID_MODEL_FIELDS:	// Intel Core i5, i7, Xeon X34xx LGA1156 (45nm)
					case CPUID_MODEL_CLARKDALE:
					case CPUID_MODEL_DALES:	// Intel Core i3, i5 LGA1156 (32nm)
					case CPUID_MODEL_WESTMERE:	// Intel Core i7, Xeon X56xx, Xeon E56xx, Xeon W36xx LGA1366 (32nm) 6 Core
					case CPUID_MODEL_NEHALEM_EX:	// Intel Xeon X75xx, Xeon X65xx, Xeon E75xx, Xeon E65x
					case CPUID_MODEL_WESTMERE_EX:	// Intel Xeon E7
					case CPUID_MODEL_SANDYBRIDGE:	// Intel Core i3, i5, i7 LGA1155 (32nm)
					case CPUID_MODEL_JAKETOWN:	// Intel Core i7, Xeon E5 LGA2011 (32nm)
					case CPUID_MODEL_IVYBRIDGE:	// Intel Core i3, i5, i7 LGA1155 (22nm)
					case CPUID_MODEL_IVYBRIDGE_XEON:
					case CPUID_MODEL_HASWELL:
					case CPUID_MODEL_HASWELL_U5:
					{
						// thanks to dgobe for i3/i5/i7 bus speed detection
						int nhm_bus = 0x3F;
						static long possible_nhm_bus[] = {0xFF, 0x7F, 0x3F};
						unsigned long did, vid;
						unsigned long qpimult, qpibusspeed = 0;
						int i;
						
						// Nehalem supports Scrubbing
						// First, locate the PCI bus where the MCH is located
						for(i = 0; i < (sizeof(possible_nhm_bus)/sizeof(possible_nhm_bus[0])); i++)
						{
							vid = (pci_config_read16(PCIADDR(possible_nhm_bus[i], 3, 4), 0x00) & 0xFFFF);
							did = (pci_config_read16(PCIADDR(possible_nhm_bus[i], 3, 4), 0x02) & 0xFF00);
							
							if(vid == 0x8086 && did >= 0x2C00)
							{
								nhm_bus = possible_nhm_bus[i];
							}
						}

						qpimult = (pci_config_read32(PCIADDR(nhm_bus, 2, 1), 0x50) & 0x7F);
						DBG("qpimult %d\n", qpimult);
						qpibusspeed = (qpimult * 2 * (Platform.CPU.FSBFrequency/1000000LL));
						// Rek: rounding decimals to match original mac profile info
						if (qpibusspeed % 100 != 0)
						{
							qpibusspeed = ((qpibusspeed + 50) / 100) * 100;
						}
						DBG("qpibusspeed %d\n", qpibusspeed);
						value->word = qpibusspeed;
						return true;
					}
					break;

					default:
						break;
				}
				break;

			default:
				break;
		}
	}

	return false; //Unsupported CPU type
}

//bool getSMBOemPlatformFeature(returnType *value)
//{
//    value->word = (uint64_t)(0x0000000000000001);
//    return true;
//}

uint16_t simpleGetSMBOemProcessorType(void)
{
	if (Platform.CPU.NoCores >= 4)
	{
		return 0x402;	// 1026 - Quad-Core Xeon
	}
	else if (Platform.CPU.NoCores == 1)
	{
		return 0x201;	// 513 - Core Duo
	};
	
	return 0x301;		// 769 - Core 2 Duo
}

bool getSMBOemProcessorType(returnType *value)
{
	static bool done = false;

	value->word = simpleGetSMBOemProcessorType();

	if (Platform.CPU.Vendor == CPUID_VENDOR_INTEL) // Intel
	{
		if (!done)
		{
			verbose("CPU is %s, family 0x%x, model 0x%x\n", Platform.CPU.BrandString, (uint32_t)Platform.CPU.Family, (uint32_t)Platform.CPU.Model);
			done = true;
		}

		switch (Platform.CPU.Family)
		{
			case 0x0F:
			case 0x06:

				switch (Platform.CPU.Model)
				{
					case CPUID_MODEL_PENTIUM_M:
					case CPUID_MODEL_DOTHAN:			// 0x0D - Intel Pentium M model D
					case CPUID_MODEL_PRESCOTT:
					case CPUID_MODEL_NOCONA:

						if (strstr(Platform.CPU.BrandString, XEON))
						{
							value->word = 0x402;		// 1026 - Xeon
							return true;
						}

						return true;

					case CPUID_MODEL_PRESLER:
					case CPUID_MODEL_CONROE:
					case CPUID_MODEL_YONAH:				// 0x0E - Intel Mobile Core Solo, Duo
						value->word = 0x201;			// 513
						return true;

					case CPUID_MODEL_MEROM:				// 0x0F - Intel Mobile Core 2 Solo, Duo, Xeon 30xx, Xeon 51xx, Xeon X53xx, Xeon E53xx, Xeon X32xx
					case CPUID_MODEL_XEON_MP:			// 0x1D - Six-Core Xeon 7400, "Dunnington", 45nm
					case CPUID_MODEL_PENRYN:			// 0x17 - Intel Core 2 Solo, Duo, Quad, Extreme, Xeon X54xx, Xeon X33xx

						if (strstr(Platform.CPU.BrandString, XEON))
						{
							value->word = 0x402;		// 1026 - Xeon
							return true;
						}

						if (Platform.CPU.NoCores <= 2)
						{
							value->word = 0x301;		// 769 - Core 2 Duo
							return true;
						}
						else
						{
							value->word = 0x402;		// 1026 - Core 2 Quad as Xeon
							return true;
						}

						return true;

					case CPUID_MODEL_LINCROFT:			// 0x27 - Intel Atom, "Lincroft", 45nm
					case CPUID_MODEL_ATOM:				// 0x1C - Intel Atom (45nm)

						return true;

					case CPUID_MODEL_NEHALEM_EX:			// 0x2E - Nehalem-ex, "Beckton", 45nm
					case CPUID_MODEL_NEHALEM:			// 0x1A - Intel Core i7, Xeon W35xx, Xeon X55xx, Xeon E55xx LGA1366 (45nm)
					case CPUID_MODEL_FIELDS:			// 0x1E - Intel Core i5, i7, Xeon X34xx LGA1156 (45nm)
					case CPUID_MODEL_CLARKDALE:			// 0x1F - Intel Core i5, i7 LGA1156 (45nm) (Havendale, Auburndale)

						if (strstr(Platform.CPU.BrandString, XEON))
						{
							value->word = 0x501;		// 1281 - Lynnfiled Quad-Core Xeon
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_I3))
						{
							value->word = 0x901;		// 2305 - Core i3
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_I5))
						{
							value->word = 0x601;			// Core i5
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_I7))
						{
							value->word = 0x701;			// 1793 - Core i7
							return true;
						}

						if (Platform.CPU.NoCores <= 2)
						{
							value->word = 0x901;			//  - Core i3
							return true;
						}

						return true;

					case CPUID_MODEL_DALES:				// 0x25 - Intel Core i3, i5 LGA1156 (32nm) (Clarkdale, Arrandale)
					case CPUID_MODEL_WESTMERE:			// 0x2C - Intel Core i7, Xeon X56xx, Xeon E56xx, Xeon W36xx LGA1366 (32nm) 6 Core
					case CPUID_MODEL_WESTMERE_EX:			// 0x2F - Intel Xeon E7

						if (strstr(Platform.CPU.BrandString, XEON))
						{
							value->word = 0x501;		// 1281 - Xeon
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_I3))
						{
							value->word = 0x901;		// 2305 - Core i3
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_I5))
						{
							value->word = 0x602;		// 1538 - Core i5
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_I7))
						{
							value->word = 0x702;		// 1794 -Core i7
							return true;
						}

						if (Platform.CPU.NoCores <= 2)
						{
							value->word = 0x901;		//  - Core i3
							return true;
						}

						return true;

					case CPUID_MODEL_JAKETOWN:			// 0x2D - Intel Core i7, Xeon E5-xxxx LGA2011 (32nm)
					case CPUID_MODEL_SANDYBRIDGE:			// 0x2A - Intel Core i3, i5, i7 LGA1155 (32nm)

						if (strstr(Platform.CPU.BrandString, XEON))
						{
							value->word = 0x501;		// 1281 - Xeon
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_I3))
						{
							value->word = 0x902;		// 2306 -Core i3
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_I5))
						{
							value->word = 0x603;		// 1539 - Core i5
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_I7))
						{
							value->word = 0x703;		// 1795 - Core i7
							return true;
						}

						if (Platform.CPU.NoCores <= 2)
						{
							value->word = 0x902;		//  - Core i5
							return true;
						}

						return true;

					case CPUID_MODEL_IVYBRIDGE:			// 0x3A - Intel Core i3, i5, i7 LGA1155 (22nm)

						if (strstr(Platform.CPU.BrandString, XEON))
						{
							value->word = 0xA01;		// 2561 - Xeon
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_I3))
						{
							value->word = 0x903;		// 2307 - Core i3 - Apple doesn't use it
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_I5))
						{
							value->word = 0x604;		// 1540 - Core i5
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_I7))
						{
							value->word = 0x704;		// 1796 - Core i7
							return true;
						}

						if (Platform.CPU.NoCores <= 2)
						{
							value->word = 0x903;		//  - Core i5
							return true;
						}

						return true;

						case CPUID_MODEL_HASWELL_U5:		// 0x3D -

						if (strstr(Platform.CPU.BrandString, CORE_M))
						{
							value->word = 0xB06;		// 2822
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_I3))
						{
							value->word = 0x906;		// 2310 - Apple doesn't use it
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_I5))
						{
							value->word = 0x606;		// 1542
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_I7))
						{
							value->word = 0x706;		// 1798
							return true;
						}

						if (Platform.CPU.NoCores <= 2)
						{
							value->word = 0x606;		// 1542
							return true;
						}

//						value->word = 0x706;			// 1798
						return true;

					case CPUID_MODEL_IVYBRIDGE_XEON:		// 0x3E - Mac Pro 6,1

						value->word = 0xA01;			// 2561 - Xeon
						return true;

					case CPUID_MODEL_ATOM_3700:			// 0x37
					case CPUID_MODEL_HASWELL:			// 0x3C
					case CPUID_MODEL_HASWELL_SVR:			// 0x3F
					case CPUID_MODEL_HASWELL_ULT:			// 0x45
					case CPUID_MODEL_HASWELL_ULX:			// 0x46
					case CPUID_MODEL_BROADWELL_HQ:			// 0x47
					case CPUID_MODEL_SKYLAKE:			// 0x4E
					case CPUID_MODEL_SKYLAKE_AVX:			// 0x55
					case CPUID_MODEL_SKYLAKE_S:			// 0x5E

						if (strstr(Platform.CPU.BrandString, XEON))
						{
							value->word = 0xA01;		// 2561 - Xeon
							return true;
						}
						if (strstr(Platform.CPU.BrandString, CORE_I3))
						{
							value->word = 0x904;		// 2308 - Core i3 - Apple doesn't use it - but we yes:-)
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_I5))
						{
							value->word = 0x605;		// 1541 - Core i5
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_I7))
						{
							value->word = 0x705;		// 1797 - Core i7
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_M))
						{
							value->word = 0xB06;		// 2822
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_M3))
						{
							value->word = 0xC05;
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_M5))
						{
							value->word = 0xD05;
							return true;
						}

						if (strstr(Platform.CPU.BrandString, CORE_M7))
						{
							value->word = 0xE05;
							return true;
						}

						if (Platform.CPU.NoCores <= 2)
						{
							value->word = 0x904;		//  - Core i3
							return true;
						}

						return true;

					case 0x15:					// EP80579 integrated processor

						value->word = 0x301;			// 769
						return true;

					case 0x13:					// Core i5, Xeon MP, "Havendale", "Auburndale", 45nm
					case 0x19:					// Intel Core i5 650 @3.20 Ghz

						value->word = 0x601;			// 1537 - Core i5
						return true;

					default:

						return true;
						break; // Unsupported CPU type
				}
				break;

			default:
				break;
		}
	}
/*
	if (Platform.CPU.Vendor == CPUID_VENDOR_AMD) // AMD
	{
		value->word = simpleGetSMBOemProcessorType();
		return true;
	}
*/
	return false;
}

bool getSMBMemoryDeviceMemoryType(returnType *value)
{
	static int idx = -1;
	int	map;

	idx++;
	if (idx < MAX_RAM_SLOTS)
	{
		map = Platform.DMI.DIMM[idx];
		if (Platform.RAM.DIMM[map].InUse && Platform.RAM.DIMM[map].Type != 0)
		{
			DBG("RAM Detected Type = %d\n", Platform.RAM.DIMM[map].Type);
			value->byte = Platform.RAM.DIMM[map].Type;
			return true;
		}
	}

	return false;
//	value->byte = SMB_MEM_TYPE_DDR2;
//	return true;
}

bool getSMBMemoryDeviceMemoryErrorHandle(returnType *value)
{
	value->word = 0xFFFF;
	return true;
}

bool getSMBMemoryDeviceMemorySpeed(returnType *value)
{
	static int idx = -1;
	int	map;

	idx++;
	if (idx < MAX_RAM_SLOTS)
	{
		map = Platform.DMI.DIMM[idx];
		if (Platform.RAM.DIMM[map].InUse && Platform.RAM.DIMM[map].Frequency != 0)
		{
			DBG("RAM Detected Freq = %d Mhz\n", Platform.RAM.DIMM[map].Frequency);
			value->dword = Platform.RAM.DIMM[map].Frequency;
			return true;
		}
	}

	return false;
//	value->dword = 800;
//	return true;
}

bool getSMBMemoryDeviceManufacturer(returnType *value)
{
	static int idx = -1;
	int	map;

	idx++;
	if (idx < MAX_RAM_SLOTS)
	{
		map = Platform.DMI.DIMM[idx];
		if (Platform.RAM.DIMM[map].InUse && strlen(Platform.RAM.DIMM[map].Vendor) > 0)
		{
			DBG("RAM Detected Vendor[%d]='%s'\n", idx, Platform.RAM.DIMM[map].Vendor);
			value->string = Platform.RAM.DIMM[map].Vendor;
			return true;
		}
	}

	if (!bootInfo->memDetect)
	{
		return false;
	}
	value->string = NOT_AVAILABLE;
	return true;
}

bool getSMBMemoryDeviceSerialNumber(returnType *value)
{
	static int idx = -1;
	int	map;

	idx++;

	//DBG("getSMBMemoryDeviceSerialNumber index: %d, MAX_RAM_SLOTS: %d\n", idx, MAX_RAM_SLOTS);

	if (idx < MAX_RAM_SLOTS)
	{
		map = Platform.DMI.DIMM[idx];
		if (Platform.RAM.DIMM[map].InUse && strlen(Platform.RAM.DIMM[map].SerialNo) > 0)
		{
			DBG("map=%d,  RAM Detected SerialNo[%d]='%s'\n", map, idx, Platform.RAM.DIMM[map].SerialNo);
			value->string = Platform.RAM.DIMM[map].SerialNo;
			return true;
		}
	}

	if (!bootInfo->memDetect)
	{
		return false;
	}
	value->string = NOT_AVAILABLE;
	return true;
}

bool getSMBMemoryDevicePartNumber(returnType *value)
{
	static int idx = -1;
	int	map;

	idx++;
	if (idx < MAX_RAM_SLOTS)
	{
		map = Platform.DMI.DIMM[idx];
		if (Platform.RAM.DIMM[map].InUse && strlen(Platform.RAM.DIMM[map].PartNo) > 0)
		{
			DBG("map=%d,  RAM Detected PartNo[%d]='%s'\n", map, idx, Platform.RAM.DIMM[map].PartNo);
			value->string = Platform.RAM.DIMM[map].PartNo;
			return true;
		}
	}

	if (!bootInfo->memDetect)
	{
		return false;
	}
	value->string = NOT_AVAILABLE;
	return true;
}

// getting smbios addr with fast compare ops, late checksum testing ...
#define COMPARE_DWORD(a,b) ( *((uint32_t *) a) == *((uint32_t *) b) )
static const char *const SMTAG = "_SM_";
//static const char *const SM3TAG = "_SM3_"; // smbios3_decode
static const char *const DMITAG = "_DMI_";

SMBEntryPoint *getAddressOfSmbiosTable(void)
{
	SMBEntryPoint	*smbios;
	/*
	 * The logic is to start at 0xf0000 and end at 0xfffff iterating 16 bytes at a time looking
	 * for the SMBIOS entry-point structure anchor (literal ASCII "_SM_").
	 */
	smbios = (SMBEntryPoint*)SMBIOS_RANGE_START;
	while (smbios <= (SMBEntryPoint *)SMBIOS_RANGE_END)
	{
		if (COMPARE_DWORD(smbios->anchor, SMTAG)  &&
			COMPARE_DWORD(smbios->dmi.anchor, DMITAG) &&
			smbios->dmi.anchor[4] == DMITAG[4] &&
			checksum8(smbios, sizeof(SMBEntryPoint)) == 0)
		{
			return smbios;
	    }
		smbios = (SMBEntryPoint*)(((char*)smbios) + 16);
	}
	printf("ERROR: Unable to find SMBIOS!\n");
	pause();
	return NULL;
}
