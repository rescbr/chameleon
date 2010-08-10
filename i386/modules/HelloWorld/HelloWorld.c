/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include "libsaio.h"

void Symbols_start();

void HelloWorld_start()
{
	Symbols_start();
	printf("Hello World from a module\n");
}
