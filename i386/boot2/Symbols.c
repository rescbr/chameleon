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

static int strcmp(const char * s1, const char * s2);

void Symbols_start()
{
}

unsigned int lookup_symbol(const char* symbol)
{
	if(strcmp(symbol, "dyld_stub_binder") == 0) return lookup_symbol("_dyld_stub_binder");	// ugly hack
	   
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

/*
 * strcmp - Copied from libsa/string.c due to symbols not able to be resolved at this point
 */
static int strcmp(const char * s1, const char * s2)
{
	while (*s1 && (*s1 == *s2)) {
		s1++;
		s2++;
	}
	return (*s1 - *s2);
}



