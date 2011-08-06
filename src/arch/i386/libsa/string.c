/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/* string operations */

#include "libsa.h"

void bcopy(const void * src, void * dst, size_t len)
{
    asm volatile ( "cld                  \n\t"
         "movl %%ecx, %%edx    \n\t"
         "shrl $2, %%ecx       \n\t"
         "rep; movsl           \n\t"
         "movl %%edx, %%ecx    \n\t"
         "andl $3, %%ecx       \n\t"
         "rep; movsb           \n\t"
       :
       : "c" (len), "D" (dst), "S" (src)
       : "memory", "%edx" );
}

/* #if DONT_USE_GCC_BUILT_IN_STRLEN */

#define tolower(c)     ((int)((c) & ~0x20))
#define toupper(c)     ((int)((c) | 0x20))

/*#endif*/


int
ptol(const char *str)
{
	register int c = *str;

	if (c <= '7' && c >= '0')
		c -= '0';
	else if (c <= 'h' && c >= 'a')
		c -= 'a';
	else c = 0;
	return c;
}
   
        
char* strbreak(const char *str, char **next, long *len)
{
    char *start = (char*)str, *end;
    bool quoted = false;
    
    if ( !start || !len )
        return 0;
    
    *len = 0;
    
    while ( isspace(*start) )
        start++;
    
    if (*start == '"')
    {
        start++;
        
        end = strchr(start, '"');
        if(end)
            quoted = true;
        else
            end = strchr(start, '\0');
    }
    else
    {
        for ( end = start; *end && !isspace(*end); end++ )
        {}
    }
    
    *len = end - start;
    
    if(next)
        *next = quoted ? end+1 : end;
    
    return start;
}

/* COPYRIGHT NOTICE: checksum8 from AppleSMBIOS */
uint8_t checksum8( void * start, unsigned int length )
{
    uint8_t   csum = 0;
    uint8_t * cp = (uint8_t *) start;
    unsigned int i;

    for ( i = 0; i < length; i++)
        csum += *cp++;

    return csum;
}

