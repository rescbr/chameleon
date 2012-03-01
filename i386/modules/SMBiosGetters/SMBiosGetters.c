/*
 * Copyright 2011 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 */
#include "libsaio.h"
#include "bootstruct.h"
#include "modules.h"
#include "Platform.h"
#include "efi.h"
#include "mysmbios.h"

#define kEnableSMBIOSGetters			"EnableSMBIOSGetters"
void getSmbiosPatched_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void getProductName_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void getboardproduct_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void is_SMB_Getters_Registred_Hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);

void getSmbiosPatched_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
    readSMBIOSInfo(getSmbiosOriginal());
    
	execute_hook("ScanMemory", NULL, NULL, NULL, NULL, NULL, NULL);
	SMBEntryPoint *patched_smb = setupSMBIOSTable(getSmbiosOriginal());
    
	if (patched_smb != NULL)
		//smbios_p = ((uint64_t)((uint32_t)patched_smb));
        Register_Smbios_Efi(patched_smb);
    
	else
    {
		verbose("Error: Could not get patched SMBIOS, fallback to original SMBIOS !!\n");
        
        //struct SMBEntryPoint *smbios_o = getSmbiosOriginal();	
        //smbios_p = ((uint64_t)((uint32_t)smbios_o));
        
        Register_Smbios_Efi(getSmbiosOriginal());
        
    }    
    
}

void getProductName_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	setupSmbiosConfigFile("SMBIOS.plist");
	int len = 0;
	const char *val = 0;
	
	if (getValueForKey("SMproductname", &val, &len, DEFAULT_SMBIOS_CONFIG)) {
		//gPlatformName = (char *)val;
        SetgPlatformName(val);
        
	} else {
		//gPlatformName = (char *)getDefaultSMBproductName();
        SetgPlatformName(getDefaultSMBproductName());
        
	}	
    
}

void getboardproduct_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	setupSmbiosConfigFile("SMBIOS.plist");
	int len = 0;
	const char *val = 0;
	
	if (getValueForKey("SMboardproduct", &val, &len, DEFAULT_SMBIOS_CONFIG)) {
		//gboardproduct = (char *)val;
        Setgboardproduct(val);
        
	} else {
		//gboardproduct = (char *)getDefaultSMBBoardProduct();
        Setgboardproduct(getDefaultSMBBoardProduct());
    }	
	
}

void is_SMB_Getters_Registred_Hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6){}

void SMBiosGetters_start(void);
void SMBiosGetters_start(void)
{	
	bool enable = true;
	getBoolForKey(kEnableSMBIOSGetters, &enable, DEFAULT_BOOT_CONFIG) ;
	
	enable = (execute_hook("isSMBIOSRegistred", NULL, NULL, NULL, NULL, NULL, NULL) != EFI_SUCCESS);

	if (enable) {
		register_hook_callback("getSmbiosPatched", &getSmbiosPatched_hook);		
		register_hook_callback("getProductNamePatched", &getProductName_hook);
		register_hook_callback("getboardproductPatched", &getboardproduct_hook);
		register_hook_callback("isSMBIOSRegistred", &is_SMB_Getters_Registred_Hook);
	}
}
