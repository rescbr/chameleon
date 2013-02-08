/*
 *  __libeg_init.c
 *  Chameleon
 *
 *  Created by cparm on 22/01/13.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 */

#include "stdio.h"
#include "libegint.h"
#include <pexpert/i386/modules.h>

void GUI_diplay_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void GUI_diplay_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	extern BOOLEAN LoadGui(VOID);
	LoadGui();
}


void Libeg_start(void);
void Libeg_start(void)
{	
	
	register_hook_callback("GUI_Display", &GUI_diplay_hook);	
}