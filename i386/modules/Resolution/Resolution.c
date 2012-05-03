/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include "libsaio.h"
#include "915resolution.h"
#include "modules.h"
#include "bootstruct.h"

void Resolution_start(void);
void Resolution_start(void)
{
	register_hook_callback("getResolution_hook", &getResolutionHook);
    patchVideoBios();	
}

