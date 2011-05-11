/*
 * Copyright 2011 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 */
#include "boot.h"
#include "bootstruct.h"
#include "libsaio.h"
#include "modules.h"
#include "Platform.h"
#include "smbios_patcher.h"
#include "efi.h"

#ifndef DEBUG_SMBIOS
#define DEBUG_SMBIOS 0
#endif

#if DEBUG_SMBIOS==2
#define DBG(x...)	printf(x)
#elif DEBUG_SMBIOS==1
#define DBG(x...) msglog(x)
#else
#define DBG(x...)	
#endif

void getSmbiosPatched_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	struct SMBEntryPoint *smbios_o = (struct SMBEntryPoint *)arg1;
	struct SMBEntryPoint *patched_smb = NULL;
	
	execute_hook("ScanMemory", NULL, NULL, NULL, NULL, NULL, NULL);	
	
	patched_smb = getSmbiosPatched(smbios_o);
	
	if (patched_smb)
		smbios_p = (EFI_PTR32)patched_smb; 
	else
		verbose("Error: Could not get patched SMBIOS, fallback to original SMBIOS !!\n");
}

void smbios_helper_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	struct SMBEntryPoint *orig = (struct SMBEntryPoint *)arg1;
	getSmbiosTableStructure(orig); // generate tables entry list for fast table finding
		
	if (is_module_loaded("Memory")) 
	scan_memory(Platform);
}

void getProductName_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	setupSmbiosConfigFile("SMBIOS.plist");
	int len = 0;
	const char *val = 0;
	
	if (getValueForKey("SMproductname", &val, &len, &bootInfo->smbiosConfig)) {
		gPlatformName = (char *)val;
	} else {
		const char *productName = sm_get_defstr("SMproductname", 0);	
		gPlatformName = (char *)productName;
	}	
	DBG("SMBIOS Product name: %s\n",productName);

	
}

void SMBiosPatcher_start()
{	
	register_hook_callback("getSmbiosPatched", &getSmbiosPatched_hook);	
	register_hook_callback("smbios_helper", &smbios_helper_hook);
	register_hook_callback("getProductNamePatched", &getProductName_hook);
}
