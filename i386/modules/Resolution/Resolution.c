/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include "libsaio.h"
#include "915resolution.h"
#include "modules.h"
#include "bootstruct.h"

#define kEnableResolution			"EnableResolutionModule"

void Resolution_start(void);
void Resolution_start(void)
{
	bool enable = true;
	getBoolForKey(kEnableResolution, &enable, DEFAULT_BOOT_CONFIG) ;
	
	if (enable) {
		register_hook_callback("getResolution_hook", &getResolutionHook);
		patchVideoBios();
	}
	
}

