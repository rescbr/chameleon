/*
 * Copyright 2010 Evan Lojewski. All rights reserved.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "modules.h"

//extern void start_built_in_modules();

void* symbols_module_start = (void*)0xFFFFFFFF;	// Global, value is populated by the makefile with actual address

/** Internal symbols, however there are accessor methods **/
moduleHook_t* moduleCallbacks = NULL;
moduleList_t* loadedModules = NULL;

int init_module_system()
{
    // Start any modules that were compiled in first.
	extern void start_built_in_modules();
    start_built_in_modules();
	
	return 1;
}	

void start_built_in_module(const char* name, 
                           const char* author, 
                           const char* description,
                           UInt32 version,
                           UInt32 compat,
                           void(*start_function)(void))
{
    start_function();
    // Notify the module system that this module really exists, specificaly, let other module link with it
    module_loaded(name, author, description, version, compat);
}



/*
 * print out the information about the loaded module
 */
void module_loaded(const char* name, const char* author, const char* description, UInt32 version, UInt32 compat)
{
	moduleList_t* new_entry = (moduleList_t*)malloc(sizeof(moduleList_t));
	new_entry->next = loadedModules;

	loadedModules = new_entry;
	
    if(!name) name = "Unknown";
    if(!author) author = "Unknown";
    if(!description) description = "";
    
	new_entry->name = name;
    new_entry->author = author;
    new_entry->description = description;
	new_entry->version = version;
    new_entry->compat = compat;
}


/********************************************************************************/
/*	Module Hook Interface														*/
/********************************************************************************/


/*
 *	execute_hook(  const char* name )
 *		name - Name of the module hook
 *			If any callbacks have been registered for this hook
 *			they will be executed now in the same order that the
 *			hooks were added.
*/
int execute_hook(const char* name, void* arg1, void* arg2, void* arg3, void* arg4)
{
	moduleHook_t* hook = (moduleHook_t*)hook_exists(name);
	
	if(hook)
	{
		// Loop through all callbacks for this module
		callbackList_t* callbacks = hook->callbacks;

		while(callbacks)
		{
			// Execute callback
			callbacks->callback(arg1, arg2, arg3, arg4);
			callbacks = callbacks->next;
		}
		return 1;
	}
	else
	{
		// Callback for this hook doesn't exist;
		return 0;
	}
}



/*
 *	register_hook_callback(  const char* name,  void(*callback)())
 *		name - Name of the module hook to attach to.
 *		callbacks - The funciton pointer that will be called when the
 *			hook is executed. When registering a new callback name, the callback is added sorted.
 *			NOTE: the hooks take four void* arguments.
 */
void register_hook_callback(const char* name, void(*callback)(void*, void*, void*, void*))
{		
	moduleHook_t* hook = hook_exists(name);
	
	if(hook)
	{
		// append
		callbackList_t* newCallback = (callbackList_t*)malloc(sizeof(callbackList_t));
		newCallback->next = hook->callbacks;
		hook->callbacks = newCallback;
		newCallback->callback = callback;
	}
	else
	{
		// create new hook
		moduleHook_t* newHook = (moduleHook_t*)malloc(sizeof(moduleHook_t));		
		newHook->name = name;
		newHook->callbacks = (callbackList_t*)malloc(sizeof(callbackList_t));
		newHook->callbacks->callback = callback;
		newHook->callbacks->next = NULL;
		
		newHook->next = moduleCallbacks;
		moduleCallbacks = newHook;
		
	}
}


moduleHook_t* hook_exists(const char* name)
{
	moduleHook_t* hooks = moduleCallbacks;
	
	// look for a hook. If it exists, return the moduleHook_t*,
	// If not, return NULL.
	while(hooks)
	{
		if(strcmp(name, hooks->name) == 0)
		{
			return hooks;
		}
		hooks = hooks->next;
	}
	return NULL;
	
}

/********************************************************************************/
/*	dyld / Linker Interface														*/
/********************************************************************************/

void dyld_stub_binder()
{
	printf("ERROR: dyld_stub_binder was called, should have been take care of by the linker.\n");
	getchar();
}