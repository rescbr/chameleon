/*
 * Symbols.c
 * 
 * Module loader support module. This module is the first module to ever be loaded.
 * It contains a copy of each symbol inside ov the current version of chameleon as well 
 * as a strcmp function. Chameleon calls lookup_symbol to resolve internal symbols
 * when they are requested by a module. This module does *not* depend on any intenrla
 * symbols, as such it can be loaded without a symbol table initialized.
 *
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 */

#include "Symbols.h"

void Symbols_start()
{
}

unsigned int lookup_symbol(const char* symbol, int(*strcmp)(const char*, const char*))
{
	int upperLimit = sizeof(symbolList) / sizeof(symbolList[0]) - 1;		
	int lowerLimit = 0;
	int compareIndex = (upperLimit - lowerLimit) >> 1; // Midpoint
	int result;
	
	while((result = strcmp(symbol, symbolList[compareIndex].symbol)) != 0)
	{
		if(result > 0)	// We need to search a HIGHER index
		{
			if(compareIndex != lowerLimit)
			{
				lowerLimit = compareIndex;
			}
			else
			{
				return 0xFFFFFFFF;	// Symbol not found
			}
			compareIndex = (upperLimit + lowerLimit + 1) >> 1;	// Midpoint, round up
		}
		else  // We Need to search a LOWER index
		{
			if(compareIndex != upperLimit)
			{
				upperLimit = compareIndex;
			}
			else
			{
				return 0xFFFFFFFF;	// Symbol not found
			}
			compareIndex = (upperLimit + lowerLimit) >> 1;	// Midpoint, round down
		}
	}
	return symbolList[compareIndex].addr;
}


#define __arraycount(__x)       (sizeof(__x) / sizeof(__x[0]))

long __stack_chk_guard[8] = {0, 0, 0, 0, 0, 0, 0, 0};

static void __guard_setup(void) __attribute__((constructor));
void __stack_chk_fail(void);

static void
__guard_setup(void)
{	
	/* Here the protector switches the guard
     to the "terminator canary", and cannot report failure */
	((char*)__stack_chk_guard)[0] = 0; ((char*)__stack_chk_guard)[1] = 0;
	((char*)__stack_chk_guard)[2] = '\n'; ((char*)__stack_chk_guard)[3] = 255;	
  
}

void
__stack_chk_fail()
{
  for(;;);
}