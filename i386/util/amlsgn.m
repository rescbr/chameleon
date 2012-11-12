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

#include <uuid/uuid.h>

#include "Intel_Acpi/acpidecode.h"
#include "Intel_Acpi/ppmsetup.h"

unsigned long uuid32;
#define UUID_LEN	 sizeof(uuid_t)
#define UUID_STR_LEN sizeof(uuid_string_t)
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
	snprintf(msg, sizeof(msg), "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		    uuid[0], uuid[1], uuid[2], uuid[3], 
		    uuid[4], uuid[5], uuid[6], uuid[7],
		    uuid[8], uuid[9], uuid[10],uuid[11],
		    uuid[12],uuid[13],uuid[14],uuid[15]);
	return msg ;
}

int main (int argc, const char * argv[]) {
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    
    mach_port_t                  masterPort;
	
	io_registry_entry_t         entry = MACH_PORT_NULL;
	
	kern_return_t                 err;
	
	CFDataRef                     data = NULL;
	
	CFAllocatorRef               allocator=kCFAllocatorDefault;
	
	const UInt8*                  rawdata = NULL;
	
	unsigned long uuid32 = 0,Model32 = 0 ;
	NSString *filename, *fullfilename ;
	NSArray *namechunks;
	NSMutableArray *chunks = nil;	
	NSString *outStr, *inStr;
    
	if (argc == 2) {	
		
		/* Creating output string */
		inStr = [NSString stringWithUTF8String:argv[1]];
		chunks = [[NSMutableArray alloc] init];
		[chunks addObjectsFromArray:[inStr componentsSeparatedByString: @"/"]];
		
		fullfilename = [chunks lastObject];
		namechunks = [fullfilename componentsSeparatedByString: @"."];
		filename = [namechunks objectAtIndex:0];
		
		[chunks removeLastObject];
		outStr = [chunks componentsJoinedByString: @"/"];
        
	} else if (argc == 3 ){
		inStr = [NSString stringWithUTF8String:argv[1]];
		BOOL isDirectory= FALSE;
        
        chunks = [[NSMutableArray alloc] init];
        [chunks addObjectsFromArray:[inStr componentsSeparatedByString: @"/"]];
        fullfilename = [chunks lastObject];
        namechunks = [fullfilename componentsSeparatedByString: @"."];
        filename = [namechunks objectAtIndex:0];
        
		if ([[NSFileManager defaultManager] fileExistsAtPath:[NSString stringWithUTF8String:argv[2]] isDirectory:&isDirectory] && isDirectory) {            
            
            outStr = [NSString stringWithUTF8String:argv[2]];	
            
		} 
        else 
        {            
            [chunks removeLastObject];
            outStr = [chunks componentsJoinedByString: @"/"];            
        }
        
	}
    else
    {
		printf("Usage: amlsgn input(as a file) output(as a directory)\n");
		goto out;
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
		
		entry=IORegistryEntryFromPath(masterPort, [@"IODeviceTree:/efi" UTF8String]);
        
		if (entry!=MACH_PORT_NULL) {			
			
			data = IORegistryEntrySearchCFProperty( entry, kIOServicePlane, CFSTR("motherboard-name"), allocator, kIORegistryIterateRecursively);
			
			if (data != NULL) {
				rawdata=CFDataGetBytePtr(data);
				
				int len = 0;
				
				char *ModelStr = ( char* )rawdata;
				
				if ((len = strlen(ModelStr))) 
				{
					Model32 = OSSwapHostToBigInt32(adler32( (unsigned char *) ModelStr, len ));
					
					printf("Model32 0x%lx ModelStr %s\n",Model32,ModelStr);
					
                    NSMutableData * mem = [[NSMutableData alloc] initWithContentsOfFile : inStr  ];
					if (!mem) {
                        NSLog(@"Error while opening the file : %@ \n",inStr);
                        CFRelease(data);
                        data = NULL;
                        goto out;
                    }
					ACPI_TABLE_HEADER * head = (ACPI_TABLE_HEADER *)[mem mutableBytes];
					
					head->OemRevision = Model32;
					
					SetChecksum(head);
					
                    if ([mem writeToFile : [NSString stringWithFormat:@"%@/%@.%@.aml",
                                            outStr, filename, [NSString stringWithUTF8String:ModelStr]] atomically: YES] == NO) {
                        
                        NSLog(@"Error: Can't save the file to %@\n",[NSString stringWithFormat:@"%@/%@.%@.aml",
                                                                     outStr, filename, [NSString stringWithUTF8String:ModelStr]]);                        
                        
                    }
                    
                    [mem release];					
				}				
                CFRelease(data);
                data = NULL;
                goto out;
			}			
			
		} 		
		
		NSLog(@"No Model entry \n");
        
		entry=IORegistryEntryFromPath(masterPort, [@"IODeviceTree:/efi/platform" UTF8String]);
        
		if (entry!=MACH_PORT_NULL){		
			
			
			data = IORegistryEntrySearchCFProperty( entry, kIOServicePlane, CFSTR("system-id"), allocator, kIORegistryIterateRecursively);
			
			if (data != NULL) {
				rawdata=CFDataGetBytePtr(data);
				
				
				const char *uuidStr = getStringFromUUID(( EFI_CHAR8* )rawdata);
				
				if (strlen(uuidStr) >= UUID_STR_LEN ) 
				{
					uuid32 = OSSwapHostToBigInt32(adler32( (unsigned char *) uuidStr, UUID_STR_LEN ));
					
					printf("uuid32  : 0x%lx\n",uuid32);
					printf("uuidStr : %s\n",uuidStr);
                    
					NSMutableData * mem = [[NSMutableData alloc] initWithContentsOfFile : inStr  ];
					if (!mem) {
                        NSLog(@"Error while opening the file : %@\n",inStr);
                        CFRelease(data);
                        data = NULL;
                        goto out;
                    }
					ACPI_TABLE_HEADER * head = (ACPI_TABLE_HEADER *)[mem mutableBytes];
                    
					head->OemRevision = uuid32;
					
					SetChecksum(head);
					
                    if ([mem writeToFile : [NSString stringWithFormat:@"%@/%@.%@.aml",outStr,filename,[NSNumber numberWithUnsignedLong:uuid32]] atomically: YES] == NO) {
                        
                        NSLog(@"Error: Can't save the file to %@ \n", [NSString stringWithFormat:@"%@/%@.%@.aml",outStr,filename,[NSNumber numberWithUnsignedLong:uuid32]]);                        
                        
                    }
                    [mem release]; 				
				}
                
                CFRelease(data);
                data = NULL;
                goto out;
                
			}			
		}
		NSLog(@"No UUID entry\n");
        
	}
	out:
    if (chunks != nil) {
        [chunks release];
    }    
    if (entry != MACH_PORT_NULL) {
        IOObjectRelease(entry);
    }
    [pool drain];
    return 0;
}
