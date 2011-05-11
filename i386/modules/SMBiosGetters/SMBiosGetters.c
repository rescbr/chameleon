/*
 * Copyright 2011 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 */
#include "boot.h"
#include "bootstruct.h"
#include "libsaio.h"
#include "modules.h"
#include "Platform.h"
#include "efi.h"
#include "mysmbios.h"

void getSmbiosPatched_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	struct SMBEntryPoint *patched_smb = NULL;
	
	execute_hook("ScanMemory", NULL, NULL, NULL, NULL, NULL, NULL);
	setupSMBIOSTable();
	smbios_p = (EFI_PTR32)getSmbiosPatched();
	
	patched_smb = getSmbiosPatched();
	
	if (patched_smb)
		smbios_p = (EFI_PTR32)patched_smb; 
	else
		verbose("Error: Could not get patched SMBIOS, fallback to original SMBIOS !!\n");
	 
}

void smbios_helper_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	struct SMBEntryPoint *smbios_o = (struct SMBEntryPoint *)arg1;		
	readSMBIOSInfo(smbios_o);	
}

void getProductName_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	setupSmbiosConfigFile("SMBIOS.plist");
	int len = 0;
	const char *val = 0;
	
	if (getValueForKey("SMproductname", &val, &len, &bootInfo->smbiosConfig)) {
		gPlatformName = (char *)val;
	} else {
		gPlatformName = setDefaultSMBData();
	}	
		
}

void SMBiosGetters_start()
{	
	
	register_hook_callback("getSmbiosPatched", &getSmbiosPatched_hook);		
	register_hook_callback("smbios_helper", &smbios_helper_hook);
	register_hook_callback("getProductNamePatched", &getProductName_hook);
}
