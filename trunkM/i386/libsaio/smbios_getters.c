/*
 * Add (c) here
 *
 * Copyright .... All rights reserved.
 *
 */

#include "smbios_getters.h"
#include "bootstruct.h"

#ifndef DEBUG_SMBIOS
#define DEBUG_SMBIOS 0
#endif

#if DEBUG_SMBIOS
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

//Slice - for ACPI patcher templates
int		ModelLength = 0;
char	MacModel[8] = "MacBook";
unsigned int ModelRev = 0x00010001;
uint64_t smbios_p;
char*	gSMBIOSBoardModel;

bool getProcessorInformationExternalClock(returnType *value)
{
	value->word = Platform->CPU.FSBFrequency/1000000;
	return true;
}

bool getProcessorInformationMaximumClock(returnType *value)
{
	value->word = Platform->CPU.CPUFrequency/1000000;
	return true;
}

bool getSMBOemProcessorBusSpeed(returnType *value)
{
	if (Platform->CPU.Vendor == 0x756E6547) // Intel
	{		
		switch (Platform->CPU.Family) 
		{
			case 0x06:
			{
				switch (Platform->CPU.Model)
				{
					case 0x0D:					// ???
					case CPU_MODEL_YONAH:		// Intel Mobile Core Solo, Duo
					case CPU_MODEL_MEROM:		// Intel Mobile Core 2 Solo, Duo, Xeon 30xx, Xeon 51xx, Xeon X53xx, Xeon E53xx, Xeon X32xx
					case CPU_MODEL_PENRYN:		// Intel Core 2 Solo, Duo, Quad, Extreme, Xeon X54xx, Xeon X33xx
					case CPU_MODEL_ATOM:		// Intel Atom (45nm)
						return false;

					case 0x19:					// ??? Intel Core i5 650 @3.20 GHz 
					case CPU_MODEL_NEHALEM:		// Intel Core i7, Xeon W35xx, Xeon X55xx, Xeon E55xx LGA1366 (45nm)
					case CPU_MODEL_FIELDS:		// Intel Core i5, i7, Xeon X34xx LGA1156 (45nm)
					case CPU_MODEL_DALES:
					case CPU_MODEL_DALES_32NM:	// Intel Core i3, i5 LGA1156 (32nm)
					case CPU_MODEL_WESTMERE:	// Intel Core i7, Xeon X56xx, Xeon E56xx, Xeon W36xx LGA1366 (32nm) 6 Core
					case CPU_MODEL_NEHALEM_EX:	// Intel Xeon X75xx, Xeon X65xx, Xeon E75xx, Xeon E65x
					case CPU_MODEL_WESTMERE_EX:	// Intel Xeon E7
					{
						// thanks to dgobe for i3/i5/i7 bus speed detection
						int nhm_bus = 0x3F;
						static long possible_nhm_bus[] = {0xFF, 0x7F, 0x3F};
						unsigned long did, vid;
						int i;
						
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
						qpibusspeed = (qpimult * 2 * (Platform->CPU.FSBFrequency/1000000));
						// Rek: rounding decimals to match original mac profile info
						if (qpibusspeed%100 != 0)qpibusspeed = ((qpibusspeed+50)/100)*100;
						DBG("qpibusspeed %d\n", qpibusspeed);
						value->word = qpibusspeed;
						return true;
					}
				}
			}
		}
	}
	return false;
}

uint16_t simpleGetSMBOemProcessorType(void)
{
	if (Platform->CPU.NoCores >= 4) 
	{
		return 0x0501;	// Quad-Core Xeon
	}
	else if (Platform->CPU.NoCores == 1) 
	{
		return 0x0201;	// Core Solo
	};
	
	return 0x0301;		// Core 2 Duo
}

bool getSMBOemProcessorType(returnType *value)
{
	static bool done = false;		
		
	value->word = simpleGetSMBOemProcessorType();

	if (Platform->CPU.Vendor == 0x756E6547) // Intel
	{
		if (!done)
		{
			verbose("CPU is %s, family 0x%x, model 0x%x\n", Platform->CPU.BrandString, Platform->CPU.Family, Platform->CPU.Model);
			done = true;
		}
		
		switch (Platform->CPU.Family) 
		{
			case 0x06:
			{
				switch (Platform->CPU.Model)
				{
					case 0x0D:							// ???
					case CPU_MODEL_YONAH:				// Intel Mobile Core Solo, Duo
					case CPU_MODEL_MEROM:				// Intel Mobile Core 2 Solo, Duo, Xeon 30xx, Xeon 51xx, Xeon X53xx, Xeon E53xx, Xeon X32xx
					case CPU_MODEL_PENRYN:				// Intel Core 2 Solo, Duo, Quad, Extreme, Xeon X54xx, Xeon X33xx
					case CPU_MODEL_ATOM:				// Intel Atom (45nm)
						return true;

					case CPU_MODEL_NEHALEM:				// Intel Core i7, Xeon W35xx, Xeon X55xx, Xeon E55xx LGA1366 (45nm)
						if (strstr(Platform->CPU.BrandString, "Xeon(R)"))
							value->word = 0x0501;			// Xeon 
						else
							value->word = 0x0701;			// Core i7

						return true;

					case CPU_MODEL_FIELDS:				// Intel Core i5, i7, Xeon X34xx LGA1156 (45nm)
						if (strstr(Platform->CPU.BrandString, "Core(TM) i5"))
							value->word = 0x0601;			// Core i5
						else
							value->word = 0x0701;			// Core i7
						return true;

					case CPU_MODEL_DALES:
						if (strstr(Platform->CPU.BrandString, "Core(TM) i5"))
							value->word = 0x0601;			// Core i5
						else
							value->word = 0x0701;			// Core i7
						return true;

					case CPU_MODEL_SANDY:				// Intel Core i3, i5, i7 LGA1155 (32nm)
                    case CPU_MODEL_SANDY_XEON:			// Intel Xeon E3
					case CPU_MODEL_DALES_32NM:			// Intel Core i3, i5 LGA1156 (32nm)
						if (strstr(Platform->CPU.BrandString, "Core(TM) i3"))
							value->word = 0x0901;			// Core i3
						else
							if (strstr(Platform->CPU.BrandString, "Core(TM) i5"))
								value->word = 0x0601;		// Core i5
							else
								value->word = 0x0701;		// Core i7
						return true;

					case CPU_MODEL_WESTMERE:			// Intel Core i7, Xeon X56xx, Xeon E56xx, Xeon W36xx LGA1366 (32nm) 6 Core
					case CPU_MODEL_WESTMERE_EX:			// Intel Xeon E7
						value->word = 0x0501;				// Core i7
						return true;

					case 0x19:							// ??? Intel Core i5 650 @3.20 GHz
						value->word = 0x0601;				// Core i5
						return true;
				}
			}
		}
	}
	
	return false;
}

bool getSMBMemoryDeviceMemoryType(returnType *value)
{
	static int idx = -1;
	int	map;

	idx++;
	if (idx < MAX_RAM_SLOTS)
	{
		map = Platform->DMI.DIMM[idx];
		if (Platform->RAM.DIMM[map].InUse && Platform->RAM.DIMM[map].Type != 0)
		{
			DBG("RAM Detected Type = %d\n", Platform->RAM.DIMM[map].Type);
			value->byte = Platform->RAM.DIMM[map].Type;
			return true;
		}
	}
	
	return false;
//	value->byte = SMB_MEM_TYPE_DDR2;
//	return true;
}

bool getSMBMemoryDeviceMemorySpeed(returnType *value)
{
	static int idx = -1;
	int	map;

	idx++;
	if (idx < MAX_RAM_SLOTS)
	{
		map = Platform->DMI.DIMM[idx];
		if (Platform->RAM.DIMM[map].InUse && Platform->RAM.DIMM[map].Frequency != 0)
		{
			DBG("RAM Detected Freq = %d Mhz\n", Platform->RAM.DIMM[map].Frequency);
			value->dword = Platform->RAM.DIMM[map].Frequency;
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
		map = Platform->DMI.DIMM[idx];
		if (Platform->RAM.DIMM[map].InUse && strlen(Platform->RAM.DIMM[map].Vendor) > 0)
		{
			DBG("RAM Detected Vendor[%d]='%s'\n", idx, Platform->RAM.DIMM[map].Vendor);
			value->string = Platform->RAM.DIMM[map].Vendor;
			return true;
		}
	}

	if (!bootInfo->memDetect)
		return false;
	value->string = NOT_AVAILABLE;
	return true;
}
	
bool getSMBMemoryDeviceSerialNumber(returnType *value)
{
	static int idx = -1;
	int	map;

	idx++;

    DBG("getSMBMemoryDeviceSerialNumber index: %d, MAX_RAM_SLOTS: %d\n",idx,MAX_RAM_SLOTS);

	if (idx < MAX_RAM_SLOTS)
	{
		map = Platform->DMI.DIMM[idx];
		if (Platform->RAM.DIMM[map].InUse && strlen(Platform->RAM.DIMM[map].SerialNo) > 0)
		{
			DBG("map=%d,  RAM Detected SerialNo[%d]='%s'\n", map, idx, Platform->RAM.DIMM[map].SerialNo);
			value->string = Platform->RAM.DIMM[map].SerialNo;
			return true;
		}
	}

	if (!bootInfo->memDetect)
		return false;
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
		map = Platform->DMI.DIMM[idx];
		if (Platform->RAM.DIMM[map].InUse && strlen(Platform->RAM.DIMM[map].PartNo) > 0)
		{
			DBG("map=%d,  RAM Detected PartNo[%d]='%s'\n", map, idx, Platform->RAM.DIMM[map].PartNo);
			value->string = Platform->RAM.DIMM[map].PartNo;
			return true;
		}
	}

	if (!bootInfo->memDetect)
		return false;
	value->string = NOT_AVAILABLE;
	return true;
}


// getting smbios addr with fast compare ops, late checksum testing ...
#define COMPARE_DWORD(a,b) ( *((uint32_t *) a) == *((uint32_t *) b) )
static const char * const SMTAG = "_SM_";
static const char* const DMITAG = "_DMI_";

SMBEntryPoint *getAddressOfSmbiosTable(void)
{
	SMBEntryPoint	*smbios;
	/* 
	 * The logic is to start at 0xf0000 and end at 0xfffff iterating 16 bytes at a time looking
	 * for the SMBIOS entry-point structure anchor (literal ASCII "_SM_").
	 */
	smbios = (SMBEntryPoint*)SMBIOS_RANGE_START;
	while (smbios <= (SMBEntryPoint *)SMBIOS_RANGE_END) {
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

void getSmbiosMacModel(void)
{
#define MAX_MODEL_LEN	32
	
	//Slice - I want to use MacModel for ACPITables so I need char* representation
	const char	*value = getStringForKey("SMproductname", &bootInfo->smbiosConfig);
	int i, n=0, first=0, rev1=0, rev2=0;		
	for (i=0; i<8; i++) 
	{
		char c = value[i];
		if (isalpha(c))
		{
			MacModel[i]=c;
			n++;
		} else 
			if ((c) >= '0' && (c) <= '9')
			{
				if (first)
				{
					rev1 = rev1 * 10 + (int)(c) & 0xf;
				} else
					rev2 = rev2 * 10 + (int)(c) & 0xf;
			} else 
				first = 1;
		//				printf("char=%c first=%d rev1=%d rev2=%d\n", c, first, rev1, rev2);
	}
	for (i=n; i<8; i++) {
		MacModel[i] = 0x20;
	}
	ModelRev = (rev2 << 16) + rev1;
	//		ModelLength = (len + 1) * 2;
	//		printf("Model=%s %08x\n", MacModel, ModelRev);
	//		getc();
	
}

//static struct SMBEntryPoint *orig = NULL; // cached
//static struct SMBEntryPoint *patched = NULL; // cached
void getSmbiosProductName()
{
	//	struct SMBEntryPoint	*smbios;
	SMBSystemInformation	*p;
	char*					tempString;
	int						tmpLen;
	
	//	smbios = getSmbios(SMBIOS_ORIGINAL);
	//	if (smbios==NULL) return 0; 
	
	p = (SMBSystemInformation*) FindFirstDmiTableOfType(1, 0x19); // Type 1: (3.3.2) System Information
	if (p==NULL) return; // NULL;
	
	tempString = (char*)smbiosStringAtIndex((SMBStructHeader*)p, p->productName, &tmpLen);
	tempString[tmpLen] = 0;
	
	gSMBIOSBoardModel = malloc(tmpLen + 1);
	if(gSMBIOSBoardModel)
	{
		strncpy(gSMBIOSBoardModel, tempString, tmpLen);
		Node* node = DT__FindNode("/", false);
		DT__AddProperty(node, "orig-model", tmpLen, gSMBIOSBoardModel);
	}
	verbose("Actual model name is '%s'\n", tempString);
}

const char * smbiosStringAtIndex(SMBStructHeader* smHeader, int index, int* length )
{
    const char * last = 0;
    const char * next = (const char *) smHeader + smHeader->length;
	
    if ( length ) *length = 0;
    while ( index-- )
    {
        last = 0;
		const char * cp = 0;
		for ( cp = next; *cp || cp[1]; cp++ )
        {
            if ( *cp == '\0' )
            {
                last = next;
                next = cp + 1;
                break;
            }
        }
        if ( last == 0 ) break;
    }
	
    if ( last )
    {
        while (*last == ' ') last++;
        if (length)
        {
            UInt8 len;
            for ( len = next - last - 1; len && last[len - 1] == ' '; len-- )
                ;
            *length = len; // number of chars not counting the terminating NULL
        }
    }
	
    return last ? last : "";
}

//Slice
//#define MEGA 1000000LL  - now in mem.h
void scan_cpu_DMI(void) //PlatformInfo_t *p)
{
	//    int i=0;
	int maxClock = 0;
    SMBStructHeader * dmihdr = NULL;    
    SMBProcessorInformation* cpuInfo; // Type 4
	
	for (dmihdr = FindFirstDmiTableOfType(4, 30); dmihdr; dmihdr = FindNextDmiTableOfType(4, 30)) 
	{
		cpuInfo = (SMBProcessorInformation*)dmihdr;
		if (cpuInfo->processorType != 3) { // CPU
			continue;
		}
		//TODO validate
#if 1 //NOTYET	
		msglog("Platform CPU Info:\n FSB=%d\n MaxSpeed=%d\n CurrentSpeed=%d\n", Platform->CPU.FSBFrequency/MEGA, Platform->CPU.TSCFrequency/MEGA, Platform->CPU.CPUFrequency/MEGA);
		
		if ((cpuInfo->externalClock) && (cpuInfo->externalClock < 400)) {  //<400MHz
			Platform->CPU.FSBFrequency = (cpuInfo->externalClock) * MEGA;
		}
		maxClock = cpuInfo->maximumClock;
		if (cpuInfo->maximumClock < cpuInfo->currentClock) {
			maxClock = cpuInfo->currentClock;
		}
		if ((maxClock) && (maxClock < 10000)) {  //<10GHz
			Platform->CPU.TSCFrequency = maxClock * MEGA;
		}
		if ((cpuInfo->currentClock) && (cpuInfo->currentClock < 10000)) {  //<10GHz
			Platform->CPU.CPUFrequency = cpuInfo->currentClock * MEGA;
		}
#endif
		msglog("DMI CPU Info:\n FSB=%d\n MaxSpeed=%d\n CurrentSpeed=%d\n", cpuInfo->externalClock, cpuInfo->maximumClock, cpuInfo->currentClock);
		msglog("DMI CPU Info 2:\n Family=%x\n Socket=%x\n Cores=%d Enabled=%d Threads=%d\n", cpuInfo->processorFamily, cpuInfo->processorUpgrade, cpuInfo->coreCount, cpuInfo->coreEnabled, cpuInfo->Threads);
#if 1 //NOTYET
		if ((cpuInfo->coreCount) && (cpuInfo->coreCount<Platform->CPU.NoCores)) {
			if (cpuInfo->coreEnabled < cpuInfo->coreCount) {
				cpuInfo->coreCount = cpuInfo->coreEnabled;
			}
			Platform->CPU.NoCores = cpuInfo->coreCount;
		}
		if ((cpuInfo->Threads) && (cpuInfo->Threads<Platform->CPU.NoThreads)) {		
			Platform->CPU.NoThreads = cpuInfo->Threads;
		}
#endif
		
		return;
	}
	
	return;
}
//Slice - check other DMI info
bool scanDMI(void)
{
	SMBStructHeader * dmihdr = NULL;    
    SMBSystemEnclosure* encInfo; // Type 3
	
	for (dmihdr = FindFirstDmiTableOfType(3, 13); dmihdr; dmihdr = FindNextDmiTableOfType(3, 13)) 
	{
		encInfo = (SMBSystemEnclosure*)dmihdr;
		msglog("DMI Chassis Info:\n Type=%x\n Boot-up State=%x\n Power Supply=%x Thermal State=%x\n", encInfo->type, encInfo->bootupState, encInfo->powerSupplyState, encInfo->thermalState);
		switch (encInfo->type) {
			case 1:
			case 2:
				return FALSE;
			case 3:
			case 4:
			case 6:
			case 7:
				Platform->CPU.Mobile = FALSE;
				break;
			case 8:
			case 9:
			case 0x0A:
			case 0x0B:				
			case 0x0E:
				Platform->CPU.Mobile = TRUE;
				break;
				
			default:
				break;
		}
		return TRUE;
	}
	return FALSE;	
}
