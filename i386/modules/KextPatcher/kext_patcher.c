/*
 * Copyright (c) 2010 Evan Lojewski. All rights reserved.
 *	
 *	KextPather
 *	This is an experimental module that I'm looking into implimenting.
 *	The main purpose is to replace the need for programs such as 
 *  NetbookInstaller's kext patching routines. THis way, Apple's kexts can be
 *  patched whe loaded instead. (eg: GMA950 kext, Bluetooth + Wifi kexts)
 */

#include "libsaio.h"
#include "kext_patcher.h"
//#include "platform.h"
#include "modules.h"

//extern PlatformInfo_t    Platform;

/**
 ** KextPatcher_start -> module start
 **		Notified the module system that this module will hook into the 
 **		LoadMatchedModules and LoadDriverMKext functions
 **/
void KextPatcher_start()
{		
	// Hooks into the following:
	//	execute_hook("LoadDriverMKext", (void*)package, (void*) length, NULL, NULL);
	//  execute_hook("LoadMatchedModules", module, &length, executableAddr, NULL);

	register_hook_callback("LoadMatchedModules", &kext_loaded); 
	register_hook_callback("LoadDriverMKext", &mkext_loaded); 

}

/**
 ** kext_loaded -> Called whenever a kext is in read into memory
 **		This function will be used to patch kexts ( eg AppleInteIntegratedFramebuffer)
 **		and their plists when they are loaded into memmory
 **/
void kext_loaded(void* module, void* length, void* executableAddr, void* arg3)
{
}

/**
 ** mkext_loaded -> Called whenever an mkext is in read into memory
 **		This function will be used to patch mkext. Matching kexts will be
 **		Extracted, modified, and then compressed again. Note: I need to determine
 **		what sort of slowdown this will cause and if it's worth implimenting.
 **/

void mkext_loaded(void* filespec, void* package, void* lenght, void* arg3)
{
}