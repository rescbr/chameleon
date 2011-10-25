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

#define kEnableSMBIOSGetters			"EnableSMBIOSGetters"


void getSmbiosPatched_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
    readSMBIOSInfo(getSmbiosOriginal());
    
	execute_hook("ScanMemory", NULL, NULL, NULL, NULL, NULL, NULL);
	SMBEntryPoint *patched_smb = setupSMBIOSTable(getSmbiosOriginal());
        	
	if (patched_smb != NULL)
		smbios_p = ((uint64_t)((uint32_t)patched_smb));
	else
    {
		verbose("Error: Could not get patched SMBIOS, fallback to original SMBIOS !!\n");
        
        struct SMBEntryPoint *smbios_o = getSmbiosOriginal();	
        smbios_p = ((uint64_t)((uint32_t)smbios_o)); 
    }    
    
}

void getProductName_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	setupSmbiosConfigFile("SMBIOS.plist");
	int len = 0;
	const char *val = 0;
	
	if (getValueForKey("SMproductname", &val, &len, &bootInfo->smbiosConfig)) {
		gPlatformName = (char *)val;
	} else {
		gPlatformName = (char *)getDefaultSMBproductName();
	}	
		
}

void getboardproduct_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	setupSmbiosConfigFile("SMBIOS.plist");
	int len = 0;
	const char *val = 0;
	
	if (getValueForKey("SMboardproduct", &val, &len, &bootInfo->smbiosConfig)) {
		gboardproduct = (char *)val;
	} else {
		gboardproduct = (char *)getDefaultSMBBoardProduct();
	}	
	
}

void is_SMB_Getters_Registred_Hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6){}

void SMBiosGetters_start()
{	
	bool enable = true;
	getBoolForKey(kEnableSMBIOSGetters, &enable, &bootInfo->bootConfig) ;
	
	enable = (execute_hook("isSMBIOSRegistred", NULL, NULL, NULL, NULL, NULL, NULL) != EFI_SUCCESS);

	if (enable) {
		register_hook_callback("getSmbiosPatched", &getSmbiosPatched_hook);		
		register_hook_callback("getProductNamePatched", &getProductName_hook);
		register_hook_callback("getboardproductPatched", &getboardproduct_hook);
		register_hook_callback("isSMBIOSRegistred", &is_SMB_Getters_Registred_Hook);
	}
}
