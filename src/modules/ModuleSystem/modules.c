/*
 * Copyright 2010 Evan Lojewski. All rights reserved.
 *
 */
#ifdef CONFIG_MODULESYSTEM_MODULE
#ifndef CONFIG_MODULE_DEBUG
#define CONFIG_MODULE_DEBUG 0
#endif

#include "boot.h"
#include "modules.h"
//#include <vers.h>

extern void start_built_in_modules();

#if CONFIG_MODULE_DEBUG
#define DBG(x...)	printf(x);
#define DBGPAUSE()	getchar()
#else
#define DBG(x...)
#define DBGPAUSE()
#endif

// NOTE: Global so that modules can link with this
UInt64 textAddress = 0;
UInt64 textSection = 0;

void* symbols_module_start = (void*)0xFFFFFFFF;	// Global, value is populated by the makefile with actual address

/** Internal symbols, however there are accessor methods **/
moduleHook_t* moduleCallbacks = NULL;
moduleList_t* loadedModules = NULL;
symbolList_t* moduleSymbols = NULL;
unsigned int (*lookup_symbol)(const char*) = NULL;


/*
 * Initialize the module system by loading the Symbols.dylib module.
 * Once loaded, locate the _lookup_symbol function so that internal
 * symbols can be resolved.
 */
int init_module_system()
{
    // Start any modules that were compiled in first.
    start_built_in_modules();
    
    
	int retVal = 0;
	void (*module_start)(void) = NULL;
	char* module_data = symbols_module_start + BOOT2_ADDR;
    
	// Intialize module system
	if(symbols_module_start != (void*)0xFFFFFFFF)
	{
		// Module system  was compiled in (Symbols.dylib addr known)
		module_start = parse_mach(module_data, &load_module, &add_symbol, NULL);
		
		if(module_start && module_start != (void*)0xFFFFFFFF)
		{
			// Notify the system that it was laoded
			module_loaded(SYMBOLS_MODULE, SYMBOLS_AUTHOR, SYMBOLS_DESCRIPTION, SYMBOLS_VERSION, SYMBOLS_COMPAT);
			
			(*module_start)();	// Start the module. This will point to load_all_modules due to the way the dylib was constructed.
			execute_hook("ModulesLoaded", NULL, NULL, NULL, NULL);
			DBG("Module %s Loaded.\n", SYMBOLS_MODULE);
			retVal = 1;

		}
		else
		{
            module_data -= 0x10;    // XCODE 4 HACK
            module_start = parse_mach(module_data, &load_module, &add_symbol, NULL);
            
            if(module_start && module_start != (void*)0xFFFFFFFF)
            {
                // Notify the system that it was laoded
                module_loaded(SYMBOLS_MODULE, SYMBOLS_AUTHOR, SYMBOLS_DESCRIPTION, SYMBOLS_VERSION, SYMBOLS_COMPAT);
                
                (*module_start)();	// Start the module. This will point to load_all_modules due to the way the dylib was constructed.
                execute_hook("ModulesLoaded", NULL, NULL, NULL, NULL);
                DBG("Module %s Loaded.\n", SYMBOLS_MODULE);
                retVal = 1;
                
            }
            else
            {
                // The module does not have a valid start function
                printf("Unable to start %s\n", SYMBOLS_MODULE); getchar();
            }		
		}		
	}
	return retVal;
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
 * Load all modules in the /Extra/modules/ directory
 * Module depencdies will be loaded first
 * Modules will only be loaded once. When loaded  a module must
 * setup apropriete function calls and hooks as required.
 * NOTE: To ensure a module loads after another you may 
 * link one module with the other. For dyld to allow this, you must
 * reference at least one symbol within the module.
 */
void load_all_modules()
{
	char* name;
	long flags;
	long time;
	struct dirstuff* moduleDir = opendir("/Extra/modules/");
	while(readdir(moduleDir, (const char**)&name, &flags, &time) >= 0)
	{
		if(strcmp(&name[strlen(name) - sizeof("dylib")], ".dylib") == 0)
		{
			char* tmp = malloc(strlen(name) + 1);
			strcpy(tmp, name);
			
			if(!load_module(tmp))
			{
				// failed to load
				// free(tmp);
			}
		}
		else 
		{
			DBG("Ignoring %s\n", name);
		}

	}
}


/*
 * Load a module file in /Extra/modules/
 */
int load_module(char* module)
{
	int retVal = 1;
	void (*module_start)(void) = NULL;
	char modString[128];
	int fh = -1;

	// Check to see if the module has already been loaded
	if(is_module_loaded(module))
	{
		return 1;
	}
	
	sprintf(modString, MODULE_PATH "%s", module);
	fh = open(modString, 0);
	if(fh < 0)
	{
		DBG("WARNING: Unable to locate module %s\n", modString); DBGPAUSE();
		return 0;
	}
	
	unsigned int moduleSize = file_size(fh);
	char* module_base = (char*) malloc(moduleSize);
	if (moduleSize && read(fh, module_base, moduleSize) == moduleSize)
	{
		// Module loaded into memory, parse it
		module_start = parse_mach(module_base, &load_module, &add_symbol, NULL);

		if(module_start && module_start != (void*)0xFFFFFFFF)
		{
			// Notify the system that it was laoded
			module_loaded(module, NULL, NULL, 0, 0 /*moduleName, moduleVersion, moduleCompat*/);
			(*module_start)();	// Start the module
			DBG("Module %s Loaded.\n", module); DBGPAUSE();
		}
#if CONFIG_MODULE_DEBUG
		else // The module does not have a valid start function. This may be a library.
		{
			printf("WARNING: Unable to start %s\n", module);
			getchar();
		}
#else
		else msglog("WARNING: Unable to start %s\n", module);
#endif
	}
	else
	{
		DBG("Unable to read in module %s\n.", module); DBGPAUSE();
		retVal = 0;
	}

	close(fh);
	return retVal;
}

/*
 * add_symbol
 * This function adds a symbol from a module to the list of known symbols 
 * possibly change to a pointer and add this to the Symbol module so that it can
 * adjust it's internal symbol list (sort) to optimize locating new symbols
 * NOTE: returns the address if the symbol is "start", else returns 0xFFFFFFFF
 */
long long add_symbol(char* symbol, long long addr, char is64)
{
	if(is64) return  0xFFFFFFFF; // Fixme

	// This only can handle 32bit symbols 
	symbolList_t* entry;
	//DBG("Adding symbol %s at 0x%X\n", symbol, addr);
	
	entry = malloc(sizeof(symbolList_t));
	entry->next = moduleSymbols;
	moduleSymbols = entry;
	
	entry->addr = (UInt32)addr;
	entry->symbol = symbol;
	
	if(strcmp(symbol, "start") == 0)
	{
		return addr;
	}
	else
	{
		return 0xFFFFFFFF; // fixme
	}
}


/*
 * print out the information about the loaded module
 */
void module_loaded(const char* name, const char* author, const char* description, UInt32 version, UInt32 compat)
{
	moduleList_t* new_entry = malloc(sizeof(moduleList_t));
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
    
    msglog("Module '%s' by '%s' Loaded.\n", name, author);
    msglog("\tDescription: %s\n", description);
    msglog("\tVersion: %d\n", version); // todo: sperate to major.minor.bugfix
    msglog("\tCompat:  %d\n", compat);  // todo: ^^^ major.minor.bugfix
}

int is_module_loaded(const char* name)
{
	// todo sorted search
	moduleList_t* entry = loadedModules;
	while(entry)
	{
		if(strcmp(entry->name, name) == 0)
		{
			DBG("Located module %s\n", name); DBGPAUSE();
			return 1;
		}
		else
		{
			entry = entry->next;
		}

	}
	
	DBG("Module %s not found\n", name); DBGPAUSE();
	return 0;
}

/*
 *	lookup symbols in all loaded modules. Thins inludes boot syms due to Symbols.dylib construction
 *
 */
unsigned int lookup_all_symbols(const char* name)
{
	symbolList_t* entry = moduleSymbols;
	while(entry)
	{
		if(strcmp(entry->symbol, name) == 0)
		{
			//DBG("External symbol %s located at 0x%X\n", name, entry->addr);
			return entry->addr;
		}
		else
		{
			entry = entry->next;
		}
	}
	
#if CONFIG_MODULE_DEBUG
	printf("Unable to locate symbol %s\n", name);
	getchar();
#endif
	
	if(strcmp(name, VOID_SYMBOL) == 0) return 0xFFFFFFFF;
	// In the event that a symbol does not exist
	// Return a pointer to a void function.
	else return lookup_all_symbols(VOID_SYMBOL);	
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
	DBG("Attempting to execute hook '%s'\n", name); DBGPAUSE();
	moduleHook_t* hook = hook_exists(name);
	
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
		DBG("Hook '%s' executed.\n", name); DBGPAUSE();
		return 1;
	}
	else
	{
		// Callback for this hook doesn't exist;
		DBG("No callbacks for '%s' hook.\n", name);
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
	DBG("Adding callback for '%s' hook.\n", name); DBGPAUSE();
	
	moduleHook_t* hook = hook_exists(name);
	
	if(hook)
	{
		// append
		callbackList_t* newCallback = malloc(sizeof(callbackList_t));
		newCallback->next = hook->callbacks;
		hook->callbacks = newCallback;
		newCallback->callback = callback;
	}
	else
	{
		// create new hook
		moduleHook_t* newHook = malloc(sizeof(moduleHook_t));		
		newHook->name = name;
		newHook->callbacks = malloc(sizeof(callbackList_t));
		newHook->callbacks->callback = callback;
		newHook->callbacks->next = NULL;
		
		newHook->next = moduleCallbacks;
		moduleCallbacks = newHook;
		
	}
	
#if CONFIG_MODULE_DEBUG
	//print_hook_list();
	//getchar();
#endif
	
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
			//DBG("Located hook %s\n", name);
			return hooks;
		}
		hooks = hooks->next;
	}
	//DBG("Hook %s does not exist\n", name);
	return NULL;
	
}

#if CONFIG_MODULE_DEBUG
void print_hook_list()
{
	printf("---Hook Table---\n");
	
	moduleHook_t* hooks = moduleCallbacks;
	while(hooks)
	{
		printf("Hook: %s\n", hooks->name);
		hooks = hooks->next;
	}
}

#endif

/********************************************************************************/
/*	dyld / Linker Interface														*/
/********************************************************************************/

void dyld_stub_binder()
{
	printf("ERROR: dyld_stub_binder was called, should have been take care of by the linker.\n");
	getchar();
}

#else /* CONFIG_MODULES */

int init_module_system()
{
    return 0;
}

void load_all_modules()
{
    
}

int execute_hook(const char* name, void* arg1, void* arg2, void* arg3, void* arg4)
{
    return 0;
}
#endif