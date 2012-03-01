/*
 * Copyright (c) 2010 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#include "libsaio.h"
#include "modules.h"
#include "bootstruct.h"
#include "pci_root.h"
#include "acpi_codec.h"

#define kEnableAcpi	"EnableAcpiModule"

void is_ACPI_Codec_Registred_Hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void ACPICodec_start(void);

void is_ACPI_Codec_Registred_Hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6){}

void ACPICodec_start(void)
{	
	bool enable = true;
	getBoolForKey(kEnableAcpi, &enable, DEFAULT_BOOT_CONFIG) ;
	
	enable = (execute_hook("isACPIRegistred", NULL, NULL, NULL, NULL, NULL, NULL) != EFI_SUCCESS);  
    
	if (enable)
	{		
		replace_system_function("_setup_acpi",&setupAcpi);
        
        register_hook_callback("isACPIRegistred", &is_ACPI_Codec_Registred_Hook);
        
	}
}