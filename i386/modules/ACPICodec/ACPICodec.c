/*
 * Copyright (c) 2010 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#include "libsaio.h"
#include "modules.h"
#include "boot.h"
#include "bootstruct.h"
#include "pci_root.h"
#include "acpi_codec.h"

#define kEnableAcpi	"EnableAcpiModule"

void ACPICodec_setupEfiConfigurationTable_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	EFI_STATUS *ret = (EFI_STATUS *)arg1;	
	    
	// Setup ACPI (mackerintel's patch)
	*ret = setupAcpi();
    
}

void is_ACPI_Codec_Registred_Hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6){}

void ACPICodec_start()
{	
	bool enable = true;
	getBoolForKey(kEnableAcpi, &enable, &bootInfo->bootConfig) ;
	
	/* 
	 * This method is not perfect (symbols collisions between two modules with that provide the same service is still possible,
	 * (as well as two module with a different service but there is more chance of collisions if dthey do the same thing)
	 * even if one of them have been disabled by this method and have no hook registred), will be deprecated soon.
	 *
	 * Possible solutions:
	 *
	 * 1 - check the symbols list each time a symbols is loaded to avoid double symbol (slow, may be buggy)
	 *
	 * 2 - categorize all symbols by callers (hard to implement)
	 *
	 * 3 - ????, will work great at least for modules with the same service
	 *
	 * 
	 *
	 */	
    enable = (execute_hook("isACPIRegistred", NULL, NULL, NULL, NULL, NULL, NULL) != EFI_SUCCESS);  
    
	if (enable) {		
		register_hook_callback("setupEfiConfigurationTable", &ACPICodec_setupEfiConfigurationTable_hook);
        register_hook_callback("isACPIRegistred", &is_ACPI_Codec_Registred_Hook);

	}
}