/*
 * Copyright 2011,2012 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 */
#include "libsaio.h"
#include "bootstruct.h"
#include "modules.h"
#include "Platform.h"
#include "efi.h"
#include "mysmbios.h"

void getSmbiosPatched_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void getProductName_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void getboardproduct_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);

void getSmbiosPatched_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
    readSMBIOSInfo(getSmbiosOriginal());
    
	execute_hook("ScanMemory", NULL, NULL, NULL, NULL, NULL, NULL);
	SMBEntryPoint *patched_smb = setupSMBIOSTable(getSmbiosOriginal());
    
	if (patched_smb != NULL)
        Register_Smbios_Efi(patched_smb);
	else
    {
		verbose("Error: Could not get patched SMBIOS, fallback to original SMBIOS !!\n");
                
        Register_Smbios_Efi(getSmbiosOriginal());
        
    }    
    
}

void getProductName_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	setupSmbiosConfigFile("SMBIOS.plist");
	int len = 0;
	const char *val = 0;
	
	if (getValueForKey("SMproductname", &val, &len, DEFAULT_SMBIOS_CONFIG)) {
        SetgPlatformName(val);
        
	} else {
        SetgPlatformName(getDefaultSMBproductName());
        
	}	
    
}

void getboardproduct_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	setupSmbiosConfigFile("SMBIOS.plist");
	int len = 0;
	const char *val = 0;
	
	if (getValueForKey("SMboardproduct", &val, &len, DEFAULT_SMBIOS_CONFIG)) {
        Setgboardproduct(val);
        
	} else {
        Setgboardproduct(getDefaultSMBBoardProduct());
    }	
	
}

void SMBiosGetters_start(void);
void SMBiosGetters_start(void)
{	
    register_hook_callback("getSmbiosPatched", &getSmbiosPatched_hook);		
    register_hook_callback("getProductNamePatched", &getProductName_hook);
    register_hook_callback("getboardproductPatched", &getboardproduct_hook);
}
