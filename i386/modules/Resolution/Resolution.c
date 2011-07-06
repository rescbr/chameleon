/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include "libsaio.h"
#include "915resolution.h"
#include "modules.h"
#include "bootstruct.h"

#define kEnableResolution			"EnableResolutionModule"


void Resolution_start()
{
	bool enable = true;
	getBoolForKey(kEnableResolution, &enable, &bootInfo->bootConfig) ;
	
	if (enable) {
		register_hook_callback("getResolution_hook", &getResolutionHook);
		patchVideoBios();
	}
	
}

