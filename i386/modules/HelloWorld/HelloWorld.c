/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include "libsaio.h"
#include "modules.h"

void helloWorld2(void* binary, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void helloWorld(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);

void helloWorld2(void* binary, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	printf("Hello world from ExecKernel hook. Binary located at 0x%X\n", binary);
	getc();
	
}

void helloWorld(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	printf("Hello world from PreBoot hook.\n");
	getc();
	
}

void HelloWorld_start(void);
void HelloWorld_start(void)
{
	register_hook_callback("ExecKernel", &helloWorld2);
	register_hook_callback("PreBoot", &helloWorld);
}

