/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include "libsaio.h"
#include "modules.h"

void helloWorld(void* binary, void* arg2, void* arg3, void* arg4)
{
	printf("Hello world from ExecKernel hook. Binary located at 0x%X\n", binary);
	getc();
	
}

void HelloWorld_start()
{
	//printf("Hooking 'ExecKernel'\n");
	register_hook_callback("ExecKernel", &helloWorld);
}

