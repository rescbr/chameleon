/*
 Copyright (c) 2010, Intel Corporation
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 * Neither the name of Intel Corporation nor the names of its contributors
 may be used to endorse or promote products derived from this software
 without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the 'License').  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */


#import <Foundation/Foundation.h>
#import <IOKit/IOReturn.h>

#include "Intel_Acpi/acpidecode.h"
#include "Intel_Acpi/ppmsetup.h"

unsigned long uuid32;
#define UUID_LEN	16
#define UUID_STR_LEN UUID_LEN*2 + 8
typedef int8_t    EFI_CHAR8;

unsigned long
adler32( unsigned char * buffer, long length );
U8 GetChecksum(void *mem_addr, U32 mem_size);
void SetChecksum(struct acpi_table_header *header);
const char * getStringFromUUID(const EFI_CHAR8* eUUID);


unsigned long
adler32( unsigned char * buffer, long length )
{
    long          cnt;
    unsigned long result, lowHalf, highHalf;
    
    lowHalf  = 1;
    highHalf = 0;
	
	for ( cnt = 0; cnt < length; cnt++ )
    {
        if ((cnt % 5000) == 0)
        {
            lowHalf  %= 65521L;
            highHalf %= 65521L;
        }
		
        lowHalf  += buffer[cnt];
        highHalf += lowHalf;
    }
	
	lowHalf  %= 65521L;
	highHalf %= 65521L;
	
	result = (highHalf << 16) | lowHalf;
	
	return result;
}

//-------------------------------------------------------------------------------
//
// Procedure:    GetChecksum - Performs byte checksum
//
//-------------------------------------------------------------------------------
U8 GetChecksum(void *mem_addr, U32 mem_size)
{
    U8 *current = mem_addr;
    U8 *end = current + mem_size;
    U8 checksum = 0;
    
    for (; current < end; current++)
        checksum = checksum + *current;
    
    return (checksum);
}

void SetChecksum(struct acpi_table_header *header)
{
    header->Checksum = 0;
    header->Checksum = 0 - GetChecksum(header, header->Length);
}


/** Transform a 16 bytes hexadecimal value UUID to a string */
/* getStringFromUUID : credit to Rekursor (see convert.c) */
const char * getStringFromUUID(const EFI_CHAR8* eUUID)
{
	static char msg[UUID_STR_LEN] = "";
	if (!eUUID) return "";
	const unsigned char * uuid = (unsigned char*) eUUID;
	sprintf(msg, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		    uuid[0], uuid[1], uuid[2], uuid[3], 
		    uuid[4], uuid[5], uuid[6], uuid[7],
		    uuid[8], uuid[9], uuid[10],uuid[11],
		    uuid[12],uuid[13],uuid[14],uuid[15]);
	return msg ;
}

int main (int argc, const char * argv[]) {
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];

    mach_port_t                  masterPort;
	
	io_registry_entry_t         entry,entry2;
	
	kern_return_t                 err;
	
	CFDataRef                     data;
	
	CFAllocatorRef               allocator=kCFAllocatorDefault;
	
	const UInt8*                  rawdata = NULL;
	
	UInt32 uuid32 = 0;
	UInt32 Model32 = 0;
	const char *cfilename ;
	NSString *filename, *fullfilename ;
	NSArray *namechunks;
	NSMutableArray *chunks;	
	char * input, *output, dirspec[512];
	NSString *outStr, *inStr;

	if (argc > 3 || argc < 2) {
		printf("Usage: amlsgn input(as a file) output(as a directory)\n");
		return 1;
    } else if (argc == 2) {	
		
		/* Creating output string */
		inStr = [NSString stringWithUTF8String:argv[1]];
		chunks = [[NSMutableArray alloc] init];
		[chunks addObjectsFromArray:[inStr componentsSeparatedByString: @"/"]];
		
		fullfilename = [chunks lastObject];
		namechunks = [fullfilename componentsSeparatedByString: @"."];
		filename = [namechunks objectAtIndex:0];
		cfilename = [filename UTF8String];
		
		[chunks removeLastObject];
		outStr = [chunks componentsJoinedByString: @"/"];

	} else {
		inStr = [NSString stringWithUTF8String:argv[1]];
		BOOL isDirectory= FALSE;
		if ([[NSFileManager defaultManager] fileExistsAtPath:outStr isDirectory:&isDirectory]) {
			
			if (isDirectory == FALSE) {
				chunks = [[NSMutableArray alloc] init];
				[chunks addObjectsFromArray:[inStr componentsSeparatedByString: @"/"]];
				
				[chunks removeLastObject];
				outStr = [chunks componentsJoinedByString: @"/"];
			} else {
				outStr = [NSString stringWithUTF8String:argv[2]];

			}			
		} 

	}
	
	do {
		BOOL isDirectory= FALSE;
		if ([[NSFileManager defaultManager] fileExistsAtPath:outStr isDirectory:&isDirectory] && isDirectory) {
		
			if ([[NSFileManager defaultManager] isWritableFileAtPath:outStr])
				break;
		}
		
		outStr = [NSHomeDirectory() stringByAppendingPathComponent:  @"Desktop"];

	} while (0);
	
	
	err = IOMasterPort(MACH_PORT_NULL, &masterPort);
	
	if (err == KERN_SUCCESS){		
		
		entry2=IORegistryEntryFromPath(masterPort, [@"IODeviceTree:/efi" UTF8String]);

		if (entry2!=MACH_PORT_NULL) {			
			
			data = IORegistryEntrySearchCFProperty( entry2, kIOServicePlane, CFSTR("motherboard-name"), allocator, kIORegistryIterateRecursively);
			
			if (data != NULL) {
				rawdata=CFDataGetBytePtr(data);
				
				int len = 0;
				
				char *ModelStr = ( char* )rawdata;
				
				if (len = strlen(ModelStr)) 
				{
					Model32 = OSSwapHostToBigInt32(adler32( (unsigned char *) ModelStr, len ));
					
					printf("uuid32 0x%lu uuidStr %s\n",(unsigned long)Model32,ModelStr);
					
					UInt8 mem[1024*1024];
					
					bzero(mem, sizeof(mem));					
					
					int fdIn = open([inStr UTF8String], O_RDONLY, 0666);
					
					if (-1 == fdIn) {
						printf("Error while opening the file \n");
						return 1;
					}
					
					size_t nbRead = read(fdIn, (void*)mem, sizeof(mem));
					if (nbRead <= 0) {
						printf("Error: Unable to read the file\n");		
						close(fdIn);
						return 1;        
					}
					
					ACPI_TABLE_HEADER * head = (ACPI_TABLE_HEADER *)mem;
					
					close(fdIn);
					
					head->OemRevision = Model32;
					
					SetChecksum(head);
					
					{		
						char dir[1024];
						
						sprintf(dir,"%s/%s.%s.aml",[outStr UTF8String],cfilename,ModelStr);

						int fdOut = open(dir, O_WRONLY | O_CREAT, 0666);
						
						if (-1 == fdOut) {
							printf("Error: Can't save the file\n");
							return 1;
						}
						write(fdOut, mem, head->Length);
						
						close(fdOut);
					}
				}				
				
				IOObjectRelease(entry2);
				
				goto out;
			}			
			
		} 		
		
		printf("No Model entry \n");
			
		entry=IORegistryEntryFromPath(masterPort, [@"IODeviceTree:/efi/platform" UTF8String]);

		if (entry!=MACH_PORT_NULL){		
			
			
			data = IORegistryEntrySearchCFProperty( entry, kIOServicePlane, CFSTR("system-id"), allocator, kIORegistryIterateRecursively);
			
			if (data != NULL) {
				rawdata=CFDataGetBytePtr(data);
				
				
				const char *uuidStr = getStringFromUUID(( EFI_CHAR8* )rawdata);
				
				if (strlen(uuidStr)) 
				{
					uuid32 = OSSwapHostToBigInt32(adler32( (unsigned char *) uuidStr, UUID_STR_LEN ));
					
					printf("uuid32  : 0x%lu\n",(unsigned long)uuid32);
					printf("uuidStr : %s\n",uuidStr);

					UInt8 mem[1024*1024];
					
					bzero(mem, sizeof(mem));
					
					int fdIn = open([inStr UTF8String], O_RDONLY, 0666);
					
					if (-1 == fdIn) {
						printf("Error while opening the file \n");
						return 1;
					}
					
					size_t nbRead = read(fdIn, (void*)mem, sizeof(mem));
					if (nbRead <= 0) {
						printf("Error: Unable to read the file\n");		
						close(fdIn);
						return 1;        
					}
					
					ACPI_TABLE_HEADER * head = (ACPI_TABLE_HEADER *)mem;
					
					close(fdIn);
					
					head->OemRevision = uuid32;
					
					SetChecksum(head);
					
					{		
						char dir[1024];
						
						sprintf(dir,"%s/%s.%lu.aml",[outStr UTF8String],cfilename,(unsigned long)uuid32);
						
						int fdOut = open(dir, O_WRONLY | O_CREAT, 0666);
						
						if (-1 == fdOut) {
							printf("Error: Can't save the file\n");
							return 1;
						}
						write(fdOut, mem, head->Length);
						
						close(fdOut);
					}
				}
				
				IOObjectRelease(entry);
				
				goto out;

			}			
		}
		printf("No UUID entry\n");

	}
out:
    [pool drain];
    return 0;
}
