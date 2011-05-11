/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include "libsaio.h"
#include "915resolution.h"
#include "modules.h"

void Resolution_start()
{
	register_hook_callback("getResolution_hook", &getResolutionHook);
	patchVideoBios();
}

