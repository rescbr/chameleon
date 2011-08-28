

#include "SMBIOS.h"
#include "Platform.h"
#include "boot.h"

static const char * const SMTAG = "_SM_";
static const char* const DMITAG= "_DMI_";
static struct SMBEntryPoint *getAddressOfSmbiosTable(void);

static struct SMBEntryPoint *getAddressOfSmbiosTable(void)
{
	struct SMBEntryPoint	*smbios;
	/* 
	 * The logic is to start at 0xf0000 and end at 0xfffff iterating 16 bytes at a time looking
	 * for the SMBIOS entry-point structure anchor (literal ASCII "_SM_").
	 */
	smbios = (struct SMBEntryPoint*) SMBIOS_RANGE_START;
	while (smbios <= (struct SMBEntryPoint *)SMBIOS_RANGE_END)
	{
		if (COMPARE_DWORD(smbios->anchor, SMTAG)  && 
			COMPARE_DWORD(smbios->dmi.anchor, DMITAG) &&
			smbios->dmi.anchor[4]==DMITAG[4] &&
			checksum8(smbios, sizeof(struct SMBEntryPoint)) == 0)
	    {			
			return ((void*)smbios);
	    }
		smbios = (struct SMBEntryPoint*) ( ((char*) smbios) + 16 );
	}
	printf("Error: Could not find original SMBIOS !!\n");
	pause();
	return NULL;
}

struct SMBEntryPoint *getSmbiosOriginal()
{    	
    static struct SMBEntryPoint *orig = NULL; // cached
    
    if (orig == NULL)
	{
		orig = getAddressOfSmbiosTable();		
		
		if (orig)
		{
			verbose("Found System Management BIOS (SMBIOS) table\n");			
		}
        
    }
    return orig;    
}

/* get UUID or product Name from original SMBIOS, stripped version of kabyl's readSMBIOSInfo */
int readSMBIOS(int value)
{			
	
	SMBEntryPoint *eps = getSmbiosOriginal();
	if (eps == NULL) return 0;
	
	uint8_t *structPtr = (uint8_t *)eps->dmi.tableAddress;
	SMBStructHeader *structHeader = (SMBStructHeader *)structPtr;
	
	for (;((eps->dmi.tableAddress + eps->dmi.tableLength) > ((uint32_t)(uint8_t *)structHeader + sizeof(SMBStructHeader)));)
	{
		switch (structHeader->type)
		{				
			case kSMBTypeSystemInformation: 
			{
				switch (value) {
					case theUUID:
						Platform->UUID = ((SMBSystemInformation *)structHeader)->uuid;
						return 1;
						break;
					case thePlatformName:
					{
						uint8_t *stringPtr = (uint8_t *)structHeader + structHeader->length;
						uint8_t field = ((SMBSystemInformation *)structHeader)->productName;
						
						if (!field)
							return 0;
						
						for (field--; field != 0 && strlen((char *)stringPtr) > 0; 
							 field--, stringPtr = (uint8_t *)((uint32_t)stringPtr + strlen((char *)stringPtr) + 1));
						
						//DBG("original SMBIOS Product name: %s\n",(char *)stringPtr);
						gPlatformName = (char *)stringPtr;
						if (gPlatformName) return 1;
						break;
					}
					default:
						break;
				}				
				
				break;	
			}
			case kSMBTypeBaseBoard:
            {
				switch (value) {
					case theProducBoard:
					{
						uint8_t *stringPtr = (uint8_t *)structHeader + structHeader->length;
						uint8_t field = ((SMBBaseBoard *)structHeader)->product;
						
						if (!field)
							return 0;
						
						for (field--; field != 0 && strlen((char *)stringPtr) > 0; 
							 field--, stringPtr = (uint8_t *)((uint32_t)stringPtr + strlen((char *)stringPtr) + 1));
						
						gboardproduct = (char *)stringPtr;
						if (gboardproduct) return 1;
						break;
					}
					default:
						break;
				}
				break;
            }
			default:
				break;
				
		}
		
		structPtr = (uint8_t *)((uint32_t)structHeader + structHeader->length);
		for (; ((uint16_t *)structPtr)[0] != 0; structPtr++);
		
		if (((uint16_t *)structPtr)[0] == 0)
			structPtr += 2;
		
		structHeader = (SMBStructHeader *)structPtr;
	}	
	return 0;
}