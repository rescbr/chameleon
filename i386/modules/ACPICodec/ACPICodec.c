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
	
	enable = (execute_hook("isACPIRegistred", NULL, NULL, NULL, NULL, NULL, NULL) != EFI_SUCCESS);  
    
	if (enable)
	{		
		register_hook_callback("setupEfiConfigurationTable", &ACPICodec_setupEfiConfigurationTable_hook);
        register_hook_callback("isACPIRegistred", &is_ACPI_Codec_Registred_Hook);

	}
}