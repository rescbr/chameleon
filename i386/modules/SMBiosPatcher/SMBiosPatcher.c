/*
 * Copyright 2011 cparm. All rights reserved.
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

#define kEnableSMBIOSPatcher			"EnableSMBIOSPatcher"


void getSmbiosPatched_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	struct SMBEntryPoint *patched_smb = NULL;
	
    getSmbiosTableStructure(getSmbiosOriginal()); // generate tables entry list for fast table finding
    
	if (execute_hook("isMemoryRegistred", NULL, NULL, NULL, NULL, NULL, NULL) == EFI_SUCCESS) 
        scan_memory(Platform);
    
	execute_hook("ScanMemory", NULL, NULL, NULL, NULL, NULL, NULL);	
	
	patched_smb = getSmbiosPatched(getSmbiosOriginal());
	
	if (patched_smb)
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
		const char *productName = sm_get_defstr("SMproductname", 0);	
		gPlatformName = (char *)productName;
	}	
	DBG("SMBIOS Product name: %s\n",productName);

	
}

void getboardproduct_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	setupSmbiosConfigFile("SMBIOS.plist");
	int len = 0;
	const char *val = 0;
	
	if (getValueForKey("SMboardproduct", &val, &len, &bootInfo->smbiosConfig)) {
		gboardproduct = (char *)val;
	} else {
        const char *productBoard = sm_get_defstr("SMboardproduct", 0);	
		gboardproduct =  (char *)productBoard;
	}	
	
}

void is_SMB_Patcher_Registred_Hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6){}

void SMBiosPatcher_start()
{	
    bool enable = true;
	getBoolForKey(kEnableSMBIOSPatcher, &enable, &bootInfo->bootConfig) ;
	
	enable = (execute_hook("isSMBIOSRegistred", NULL, NULL, NULL, NULL, NULL, NULL) != EFI_SUCCESS);
    
	if (enable) {
	register_hook_callback("getSmbiosPatched", &getSmbiosPatched_hook);	
	register_hook_callback("getProductNamePatched", &getProductName_hook);
    register_hook_callback("getboardproductPatched", &getboardproduct_hook);
    register_hook_callback("isSMBIOSRegistred", &is_SMB_Patcher_Registred_Hook);
    }
}
