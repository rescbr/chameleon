/*-
 * Copyright (c) 2005-2009 Jung-uk Kim <jkim@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Detect SMBIOS and export information about the SMBIOS into the
 * environment.
 *
 * System Management BIOS Reference Specification, v2.6 Final
 * http://www.dmtf.org/standards/published_documents/DSP0134_2.6.0.pdf
 */

/*
 * 2.1.1 SMBIOS Structure Table Entry Point
 *
 * "On non-EFI systems, the SMBIOS Entry Point structure, described below, can
 * be located by application software by searching for the anchor-string on
 * paragraph (16-byte) boundaries within the physical memory address range
 * 000F0000h to 000FFFFFh. This entry point encapsulates an intermediate anchor
 * string that is used by some existing DMI browsers."
 */

#include "libsaio.h"
#include "SMBIOS.h"
#include "Platform.h"

#define SMBIOS_START            0xf0000
#define SMBIOS_LENGTH           0x10000
#define SMBIOS_STEP             0x10
#define SMBIOS_SIG              "_SM_"
#define SMBIOS_DMI_SIG          "_DMI_"

#define SMBIOS_GET8(base, off)  (*(uint8_t *)((base) + (off)))
#define SMBIOS_GET16(base, off) (*(uint16_t *)((base) + (off)))
#define SMBIOS_GET32(base, off) (*(uint32_t *)((base) + (off)))

#define SMBIOS_GETLEN(base)     SMBIOS_GET8(base, 0x01)
#define SMBIOS_GETSTR(base)     ((base) + SMBIOS_GETLEN(base))

static uint8_t
smbios_checksum(const caddr_t addr, const uint8_t len)
{
	uint8_t         sum;
	int             i;
	
	for (sum = 0, i = 0; i < len; i++)
		sum += SMBIOS_GET8(addr, i);
	return (sum);
}

static caddr_t
smbios_sigsearch(const caddr_t addr, const uint32_t len)
{
	caddr_t         cp;
	
	/* Search on 16-byte boundaries. */
	for (cp = addr; cp < addr + len; cp += SMBIOS_STEP)
		if (strncmp(cp, SMBIOS_SIG, 4) == 0 &&
			smbios_checksum(cp, SMBIOS_GET8(cp, 0x05)) == 0 &&
			strncmp(cp + 0x10, SMBIOS_DMI_SIG, 5) == 0 &&
			smbios_checksum(cp + 0x10, 0x0f) == 0)
			return (cp);
	return (NULL);
}

struct SMBEntryPoint *getSmbiosOriginal()
{    	
    static caddr_t smbios = NULL; // cached
    
    if (smbios == NULL)
	{
		/* Search signatures and validate checksums. */
		smbios = smbios_sigsearch((caddr_t)ptov(SMBIOS_START), SMBIOS_LENGTH);
		
		if (smbios)
		{
			verbose("Found System Management BIOS (SMBIOS) table\n");			
		}
        
    }
    return (struct SMBEntryPoint *)smbios;    
}

/* get product Name from original SMBIOS */
char* readDefaultPlatformName(void)
{			
	
	SMBEntryPoint *eps = getSmbiosOriginal();
	if (eps == NULL) return NULL;
	
	uint8_t *structPtr = (uint8_t *)eps->dmi.tableAddress;
	SMBStructHeader *structHeader = (SMBStructHeader *)structPtr;
	
	for (;((eps->dmi.tableAddress + eps->dmi.tableLength) > ((uint32_t)(uint8_t *)structHeader + sizeof(SMBStructHeader)));)
	{
		switch (structHeader->type)
		{				
			case kSMBTypeSystemInformation: 
			{
				uint8_t *stringPtr = (uint8_t *)structHeader + structHeader->length;
				uint8_t field = ((SMBSystemInformation *)structHeader)->productName;
				
				if (!field)
					return NULL;
				
				for (field--; field != 0 && strlen((char *)stringPtr) > 0; 
					 field--, stringPtr = (uint8_t *)((uint32_t)stringPtr + strlen((char *)stringPtr) + 1));
				
				//DBG("original SMBIOS Product name: %s\n",(char *)stringPtr);
				if (stringPtr)
					return (char *)stringPtr;
				else 
					return NULL;			
				
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
	return NULL;
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
                        safe_set_env(envUUID,(uint32_t)((SMBSystemInformation *)structHeader)->uuid);					
						return 1;
						break;
					case thePlatformName:
					{
						uint8_t *stringPtr = (uint8_t *)structHeader + structHeader->length;
						uint8_t field = ((SMBSystemInformation *)structHeader)->productName;
						
						if (!field)
							return 0;
						
						for (field--; field != 0 && strlen((char *)stringPtr) > 0; 
							 field--, stringPtr = (uint8_t *)((uint32_t)stringPtr + strlen((char *)stringPtr) + 1)){};
						
						//DBG("original SMBIOS Product name: %s\n",(char *)stringPtr);
                        SetgPlatformName((char *)stringPtr);
						if (GetgPlatformName()) return 1;
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
						
						Setgboardproduct((char *)stringPtr);
						if (Getgboardproduct()) return 1;
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