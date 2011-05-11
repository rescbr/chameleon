/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

/*
 * Copyright (c) 2010 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#include "libsaio.h"
#include "modules.h"
#include "boot.h"
#include "bootstruct.h"
#include "pci_root.h"
#include "acpi_patcher.h"


void AcpiPatcher_setupEfiConfigurationTable_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	int *ret = (int *)arg1;
	
	bool enabled = true;
	getBoolForKey(kAcpiPatcher, &enabled, &bootInfo->bootConfig);
	if (enabled == false) 
	{
		
		*ret = 0;
		return;
		
		
	}
		
	// Setup ACPI (mackerintel's patch)
	*ret = setupAcpi();

}

void AcpiPatcher_start()
{	
			
    register_hook_callback("setupEfiConfigurationTable", &AcpiPatcher_setupEfiConfigurationTable_hook);
	
}
