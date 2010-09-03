/*
 * Copyright 2010 Evan Lojewski. All rights reserved.
 *
 */

#include "boot.h"
#include "bootstruct.h"
#include "multiboot.h"
#include "modules.h"

#ifndef DEBUG_MODULES
#define DEBUG_MODULES 0
#endif

#if DEBUG_MODULES
#define DBG(x...)	printf(x); getc()
#else
#define DBG(x...)
#endif


moduleHook_t* moduleCallbacks = NULL;
moduleList_t* loadedModules = NULL;
symbolList_t* moduleSymbols = NULL;
unsigned int (*lookup_symbol)(const char*) = NULL;


void rebase_macho(void* base, char* rebase_stream, UInt32 size);
void bind_macho(void* base, char* bind_stream, UInt32 size);



#if DEBUG_MODULES
void print_hook_list()
{
	moduleHook_t* hooks = moduleCallbacks;
	while(hooks)
	{
		printf("Hook: %s\n", hooks->name);
		hooks = hooks->next;
	}
}
#endif

/*
 * Initialize the module system by loading the Symbols.dylib module.
 * Once laoded, locate the _lookup_symbol function so that internal
 * symbols can be resolved.
 */
int init_module_system()
{
	// Intialize module system
	if(load_module(SYMBOLS_MODULE))
	{
		lookup_symbol = (void*)lookup_all_symbols(SYMBOL_LOOKUP_SYMBOL);
		
		if((UInt32)lookup_symbol != 0xFFFFFFFF)
		{
			return 1;
		}
		
	}
	
	return 0;
}


/*
 * Load all modules in the /Extra/modules/ directory
 * Module depencdies will be loaded first
 * MOdules will only be loaded once. When loaded  a module must
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
	
	//Azi: this path is resolving to bt(0,0) instead of selected volume; a perfect
	// example of what made me gave up on /Extra path. Confirmed on MBR/boot0hfs only.
	// Looking further into this...
	struct dirstuff* moduleDir = opendir("/Extra/modules/");
	while(readdir(moduleDir, (const char**)&name, &flags, &time) >= 0)
	{
		if(strcmp(&name[strlen(name) - sizeof("dylib")], ".dylib") == 0)
		{
			load_module(name);
		}
	}
}


/*
 * Load a module file in /Extra/modules
 * TODO: verify version number of module
 */
int load_module(const char* module)
{
	// Check to see if the module has already been loaded
	if(is_module_laoded(module))
	{
		// NOTE: Symbols.dylib tries to load twice, this catches it as well
		// as when a module links with an already loaded module
		return 1;
	}
	
	char modString[128];
	int fh = -1;
	
	//Azi: same as #84
	sprintf(modString, "/Extra/modules/%s", module);
	fh = open(modString, 0);
	if(fh < 0)
	{
		printf("Unable to locate module %s\n", modString);
		getc();
		return 0;
	}
	
	unsigned int moduleSize = file_size(fh);
	char* module_base = (char*) malloc(moduleSize);
	if (read(fh, module_base, moduleSize) == moduleSize)
	{
		void (*module_start)(void) = NULL;

		//printf("Module %s read in.\n", modString);

		// Module loaded into memory, parse it
		module_start = parse_mach(module_base);

		if(module_start && module_start != (void*)0xFFFFFFFF)
		{
			(*module_start)();	// Start the module
			DBG("Module %s Loaded.\n", module);
		}
		else {
			printf("Unable to start %s\n", module);
			getc();
		}		
	}
	else
	{
		printf("Unable to read in module %s\n.", module);
		getc();
	}
	return 1;
}

/*
 *	execute_hook(  const char* name )
 *		name - Name of the module hook
 *			If any callbacks have been registered for this hook
 *			they will be executed now in the same order that the
 *			hooks were added.
 */
int execute_hook(const char* name, void* arg1, void* arg2, void* arg3, void* arg4)
{
	DBG("Attempting to execute hook '%s'\n", name);

	if(moduleCallbacks != NULL)
	{
		moduleHook_t* hooks = moduleCallbacks;
		
		while(hooks != NULL && strcmp(name, hooks->name) < 0)
		{
			hooks = hooks->next;
		}
		
		if(strcmp(name, hooks->name) == 0)
		{
			// Loop through all callbacks for this module
			callbackList_t* callbacks = hooks->callbacks;
			
			while(callbacks)
			{
				// Execute callback
				callbacks->callback(arg1, arg2, arg3, arg4);
				callbacks = callbacks->next;
			}
			DBG("Hook '%s' executed.\n", name);

			return 1;
		}
		else
		{
			DBG("No callbacks for '%s' hook.\n", name);

			// Callbaack for this module doesn't exist;
			//verbose("Unable execute hook '%s', no callbacks registered.\n", name);
			//pause();
			return 0;
		}


	}	
	DBG("No hooks have been registered.\n", name);
	return 0;
}



/*
 *	register_hook_callback(  const char* name,  void(*callback)())
 *		name - Name of the module hook to attach to.
 *		callbacks - The funciton pointer that will be called when the
 *			hook is executed. When registering a new callback name, the callback is added sorted.
 *			NOTE: the hooks take four void* arguments.
 *			TODO: refactor
 */
void register_hook_callback(const char* name, void(*callback)(void*, void*, void*, void*))
{
	DBG("Registering %s\n", name);
	// Locate Module hook
	if(moduleCallbacks == NULL)
	{
		moduleCallbacks = malloc(sizeof(moduleHook_t));
		moduleCallbacks->next = NULL;
		moduleCallbacks->name = name;
		// Initialize hook list
		moduleCallbacks->callbacks = (callbackList_t*)malloc(sizeof(callbackList_t));
		moduleCallbacks->callbacks->callback = callback;
		moduleCallbacks->callbacks->next = NULL;
	}
	else
	{
		moduleHook_t* hooks = moduleCallbacks;
		moduleHook_t* newHook = malloc(sizeof(moduleHook_t));;
		
		while(hooks->next != NULL && strcmp(name, hooks->name) < 0)
		{
			hooks = hooks->next;
		}
		
		DBG("%s cmp %s = %d\n", name, hooks->name, strcmp(name, hooks->name));
		
		if(hooks == NULL)
		{
			newHook->next = moduleCallbacks;
			moduleCallbacks = newHook;
			newHook->name = name;
			newHook->callbacks = (callbackList_t*)malloc(sizeof(callbackList_t));
			newHook->callbacks->callback = callback;
			newHook->callbacks->next = NULL;
			
		}
		else if(strcmp(name, hooks->name) != 0)
		{
			newHook->next = hooks->next;
			hooks->next = newHook;
			
			newHook->name = name;
			newHook->callbacks = (callbackList_t*)malloc(sizeof(callbackList_t));
			newHook->callbacks->callback = callback;
			newHook->callbacks->next = NULL;
		}
		else
		{
			callbackList_t* callbacks = hooks->callbacks;
			while(callbacks->next != NULL)
			{
				callbacks = callbacks->next;
			}
			// Add new entry to end of hook list.
			callbacks->next = (callbackList_t*)malloc(sizeof(callbackList_t));
			callbacks = callbacks->next;
			callbacks->next = NULL;
			callbacks->callback = callback;
			
		}
	}
#if DEBUG_MODULES
	print_hook_list();
	getc();
#endif
	
}


/*
 * Parse through a macho module. The module will be rebased and binded
 * as specified in the macho header. If the module is sucessfuly laoded
 * the module iinit address will be returned.
 * NOTE; all dependecies will be loaded before this module is started
 * NOTE: If the module is unable to load ot completeion, the modules
 * symbols will still be available (TODO: fix this). This should not
 * happen as all dependencies are verified before the sybols are read in.
 */
void* parse_mach(void* binary)	// TODO: add param to specify valid archs
{	
	char is64 = false;
	void (*module_start)(void) = NULL;
	
	// Module info
	char* moduleName = NULL;
	UInt32 moduleVersion = 0;
	UInt32 moduleCompat = 0;
	
	// TODO convert all of the structs to a union
	struct load_command *loadCommand = NULL;
	struct dylib_command* dylibCommand = NULL;
	struct dyld_info_command* dyldInfoCommand = NULL;
	
	struct symtab_command* symtabCommand = NULL;
	
	//struct dysymtab_command* dysymtabCommand = NULL;
	UInt32 binaryIndex = sizeof(struct mach_header);
	UInt16 cmd = 0;

	// Parse through the load commands
	if(((struct mach_header*)binary)->magic == MH_MAGIC)
	{
		is64 = 0;
	}
	else if(((struct mach_header_64*)binary)->magic != MH_MAGIC_64)
	{
		is64 = 1;
	}
	else
	{
		printf("Invalid mach magic\n");
		getc();
		return NULL;
	}


	
	if(((struct mach_header*)binary)->filetype != MH_DYLIB)
	{
		printf("Module is not a dylib. Unable to load.\n");
		getc();
		return NULL; // Module is in the incorrect format
	}
	
	while(cmd < ((struct mach_header*)binary)->ncmds)	// TODO: for loop instead
	{
		cmd++;
		
		loadCommand = binary + binaryIndex;
		UInt32 cmdSize = loadCommand->cmdsize;

		
		switch ((loadCommand->cmd & 0x7FFFFFFF))
		{
			case LC_SYMTAB:
				symtabCommand = binary + binaryIndex;
				break;
			case LC_SEGMENT:
				break;
				
			case LC_DYSYMTAB:
				break;
				
			case LC_LOAD_DYLIB:
			case LC_LOAD_WEAK_DYLIB ^ LC_REQ_DYLD:
				dylibCommand  = binary + binaryIndex;
				char* module  = binary + binaryIndex + ((UInt32)*((UInt32*)&dylibCommand->dylib.name));
				// TODO: verify version
				// =	dylibCommand->dylib.current_version;
				// =	dylibCommand->dylib.compatibility_version;

				if(!load_module(module))
				{
					// Unable to load dependancy
					return NULL;
				}
				break;
				
			case LC_ID_DYLIB:
				dylibCommand = binary + binaryIndex;
				moduleName =	binary + binaryIndex + ((UInt32)*((UInt32*)&dylibCommand->dylib.name));
				moduleVersion =	dylibCommand->dylib.current_version;
				moduleCompat =	dylibCommand->dylib.compatibility_version;
				break;

			case LC_DYLD_INFO:
				// Bind and rebase info is stored here
				dyldInfoCommand = binary + binaryIndex;
				break;
				
			case LC_UUID:
				break;
				
			default:
				DBG("Unhandled loadcommand 0x%X\n", loadCommand->cmd & 0x7FFFFFFF);
				break;
		
		}

		binaryIndex += cmdSize;
	}
	if(!moduleName) return NULL;
		

	// bind_macho uses the symbols.
	module_start = (void*)handle_symtable((UInt32)binary, symtabCommand, &add_symbol, is64);

	// Rebase the module before binding it.
	if(dyldInfoCommand && dyldInfoCommand->rebase_off)
	{
		rebase_macho(binary, (char*)dyldInfoCommand->rebase_off, dyldInfoCommand->rebase_size);
	}
	
	if(dyldInfoCommand && dyldInfoCommand->bind_off)
	{
		bind_macho(binary, (char*)dyldInfoCommand->bind_off, dyldInfoCommand->bind_size);
	}
	
	if(dyldInfoCommand && dyldInfoCommand->weak_bind_off)
	{
		// NOTE: this currently should never happen.
		bind_macho(binary, (char*)dyldInfoCommand->weak_bind_off, dyldInfoCommand->weak_bind_size);
	}
	
	if(dyldInfoCommand && dyldInfoCommand->lazy_bind_off)
	{
		// NOTE: we are binding the lazy pointers as a module is laoded,
		// This should be changed to bind when a symbol is referened at runtime instead.
		bind_macho(binary, (char*)dyldInfoCommand->lazy_bind_off, dyldInfoCommand->lazy_bind_size);
	}
	

	

	// Notify the system that it was laoded
	module_loaded(moduleName, moduleVersion, moduleCompat);
	
	return module_start;
	
}


// Based on code from dylibinfo.cpp and ImageLoaderMachOCompressed.cpp
void rebase_macho(void* base, char* rebase_stream, UInt32 size)
{
	rebase_stream += (UInt32)base;
	
	UInt8 immediate = 0;
	UInt8 opcode = 0;
	UInt8 type = 0;
	
	UInt32 segmentAddress = 0;
	
	
	
	UInt32 tmp  = 0;
	UInt32 tmp2  = 0;
	UInt8 bits = 0;
	int index = 0;
	
	int done = 0;
	unsigned int i = 0;
	
	while(/*!done &&*/ i < size)
	{
		immediate = rebase_stream[i] & REBASE_IMMEDIATE_MASK;
		opcode = rebase_stream[i] & REBASE_OPCODE_MASK;

		
		switch(opcode)
		{
			case REBASE_OPCODE_DONE:
				// Rebase complete.
				done = 1;
				break;
				
				
			case REBASE_OPCODE_SET_TYPE_IMM:
				// Set rebase type (pointer, absolute32, pcrel32)
				//printf("Rebase type = 0x%X\n", immediate);
				// NOTE: This is currently NOT used
				type = immediate;
				break;
				
				
			case REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
				// Locate address to begin rebasing
				segmentAddress = 0;

				 
				struct segment_command* segCommand = NULL; // NOTE: 32bit only
				
				unsigned int binIndex = 0;
				index = 0;
				do
				{
					segCommand = base + sizeof(struct mach_header) +  binIndex;
					
					
					binIndex += segCommand->cmdsize;
					index++;
				}
				while(index <= immediate);


				segmentAddress = segCommand->fileoff;
				
				tmp = 0;
				bits = 0;
				do
				{
					tmp |= (rebase_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(rebase_stream[i] & 0x80);
				
				segmentAddress += tmp;
				break;
				
				
			case REBASE_OPCODE_ADD_ADDR_ULEB:
				// Add value to rebase address
				tmp = 0;
				bits = 0;
				do
				{
					tmp <<= bits;
					tmp |= rebase_stream[++i] & 0x7f;
					bits += 7;
				}
				while(rebase_stream[i] & 0x80);
				
				segmentAddress +=	tmp; 
				break;
				
			case REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
				segmentAddress += immediate * sizeof(void*);
				break;
				
				
			case REBASE_OPCODE_DO_REBASE_IMM_TIMES:
				index = 0;
				for (index = 0; index < immediate; ++index) {
					rebase_location(base + segmentAddress, (char*)base);
					segmentAddress += sizeof(void*);
				}
				break;
			
				
			case REBASE_OPCODE_DO_REBASE_ULEB_TIMES:
				tmp = 0;
				bits = 0;
				do
				{
					tmp |= (rebase_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(rebase_stream[i] & 0x80);
				
				index = 0;
				for (index = 0; index < tmp; ++index) {
					//DBG("\tRebasing 0x%X\n", segmentAddress);
					rebase_location(base + segmentAddress, (char*)base);					
					segmentAddress += sizeof(void*);
				}
				break;
				
			case REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
				tmp = 0;
				bits = 0;
				do
				{
					tmp |= (rebase_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(rebase_stream[i] & 0x80);
				
				rebase_location(base + segmentAddress, (char*)base);
				
				segmentAddress += tmp + sizeof(void*);
				break;
				
			case REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
				tmp = 0;
				bits = 0;
				do
				{
					tmp |= (rebase_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(rebase_stream[i] & 0x80);
				
				
				tmp2 =  0;
				bits = 0;
				do
				{
					tmp2 |= (rebase_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(rebase_stream[i] & 0x80);
				
				index = 0;
				for (index = 0; index < tmp; ++index) {
					rebase_location(base + segmentAddress, (char*)base);
					
					segmentAddress += tmp2 + sizeof(void*);
				}
				break;
		}
		i++;
	}
}

// Based on code from dylibinfo.cpp and ImageLoaderMachOCompressed.cpp
// NOTE: this uses 32bit values, and not 64bit values. 
// There is apossibility that this could cause issues,
// however the macho file is 32 bit, so it shouldn't matter too much
void bind_macho(void* base, char* bind_stream, UInt32 size)
{	
	bind_stream += (UInt32)base;
	
	UInt8 immediate = 0;
	UInt8 opcode = 0;
	UInt8 type = 0;
	
	UInt32 segmentAddress = 0;
	
	UInt32 address = 0;
	
	SInt32 addend = 0;			// TODO: handle this
	SInt32 libraryOrdinal = 0;

	const char* symbolName = NULL;
	UInt8 symboFlags = 0;
	UInt32 symbolAddr = 0xFFFFFFFF;
	
	// Temperary variables
	UInt8 bits = 0;
	UInt32 tmp = 0;
	UInt32 tmp2 = 0;
	
	UInt32 index = 0;
	int done = 0;
	unsigned int i = 0;
	
	while(/*!done &&*/ i < size)
	{
		immediate = bind_stream[i] & BIND_IMMEDIATE_MASK;
		opcode = bind_stream[i] & BIND_OPCODE_MASK;
		
		
		switch(opcode)
		{
			case BIND_OPCODE_DONE:
				done = 1; 
				break;
				
			case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
				libraryOrdinal = immediate;
				//DBG("BIND_OPCODE_SET_DYLIB_ORDINAL_IMM: %d\n", libraryOrdinal);
				break;
				
			case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
				libraryOrdinal = 0;
				bits = 0;
				do
				{
					libraryOrdinal |= (bind_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(bind_stream[i] & 0x80);
				
				//DBG("BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB: %d\n", libraryOrdinal);

				break;
				
			case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
				// NOTE: this is wrong, fortunately we don't use it
				libraryOrdinal = -immediate;
				//DBG("BIND_OPCODE_SET_DYLIB_SPECIAL_IMM: %d\n", libraryOrdinal);

				break;
				
			case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
				symboFlags = immediate;
				symbolName = (char*)&bind_stream[++i];
				i += strlen((char*)&bind_stream[i]);
				//DBG("BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM: %s, 0x%X\n", symbolName, symboFlags);

				symbolAddr = lookup_all_symbols(symbolName);

				break;
				
			case BIND_OPCODE_SET_TYPE_IMM:
				// Set bind type (pointer, absolute32, pcrel32)
				type = immediate;
				//DBG("BIND_OPCODE_SET_TYPE_IMM: %d\n", type);

				break;
				
			case BIND_OPCODE_SET_ADDEND_SLEB:
				addend = 0;
				bits = 0;
				do
				{
					addend |= (bind_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(bind_stream[i] & 0x80);
				
				if(!(bind_stream[i-1] & 0x40)) addend *= -1;
				
				//DBG("BIND_OPCODE_SET_ADDEND_SLEB: %d\n", addend);
				break;
				
			case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
				segmentAddress = 0;

				// Locate address
				struct segment_command* segCommand = NULL;	// NOTE: 32bit only
				
				unsigned int binIndex = 0;
				index = 0;
				do
				{
					segCommand = base + sizeof(struct mach_header) +  binIndex;
					binIndex += segCommand->cmdsize;
					index++;
				}
				while(index <= immediate);
				
				segmentAddress = segCommand->fileoff;
				
				// Read in offset
				tmp  = 0;
				bits = 0;
				do
				{
					tmp |= (bind_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(bind_stream[i] & 0x80);
				
				segmentAddress += tmp;
				
				//DBG("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB: 0x%X\n", segmentAddress);
				break;
				
			case BIND_OPCODE_ADD_ADDR_ULEB:
				// Read in offset
				tmp  = 0;
				bits = 0;
				do
				{
					tmp |= (bind_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(bind_stream[i] & 0x80);
				
				segmentAddress += tmp;
				//DBG("BIND_OPCODE_ADD_ADDR_ULEB: 0x%X\n", segmentAddress);
				break;
				
			case BIND_OPCODE_DO_BIND:
				//DBG("BIND_OPCODE_DO_BIND\n");
				if(symbolAddr != 0xFFFFFFFF)
				{
					address = segmentAddress + (UInt32)base;

					((char*)address)[0] = (symbolAddr & 0x000000FF) >> 0;
					((char*)address)[1] = (symbolAddr & 0x0000FF00) >> 8;
					((char*)address)[2] = (symbolAddr & 0x00FF0000) >> 16;
					((char*)address)[3] = (symbolAddr & 0xFF000000) >> 24;
				}
				else if(strcmp(symbolName, SYMBOL_DYLD_STUB_BINDER) != 0)
				{
					printf("Unable to bind symbol %s\n", symbolName);
				}
				
				segmentAddress += sizeof(void*);
				break;
				
			case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
				//DBG("BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB\n");
				
				
				// Read in offset
				tmp  = 0;
				bits = 0;
				do
				{
					tmp |= (bind_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(bind_stream[i] & 0x80);
				
				
				
				if(symbolAddr != 0xFFFFFFFF)
				{
					address = segmentAddress + (UInt32)base;
					
					((char*)address)[0] = (symbolAddr & 0x000000FF) >> 0;
					((char*)address)[1] = (symbolAddr & 0x0000FF00) >> 8;
					((char*)address)[2] = (symbolAddr & 0x00FF0000) >> 16;
					((char*)address)[3] = (symbolAddr & 0xFF000000) >> 24;
				}
				else if(strcmp(symbolName, SYMBOL_DYLD_STUB_BINDER) != 0)
				{
					printf("Unable to bind symbol %s\n", symbolName);
				}
				segmentAddress += tmp + sizeof(void*);

				
				break;
				
			case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
				//DBG("BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED\n");
				
				if(symbolAddr != 0xFFFFFFFF)
				{
					address = segmentAddress + (UInt32)base;
					
					((char*)address)[0] = (symbolAddr & 0x000000FF) >> 0;
					((char*)address)[1] = (symbolAddr & 0x0000FF00) >> 8;
					((char*)address)[2] = (symbolAddr & 0x00FF0000) >> 16;
					((char*)address)[3] = (symbolAddr & 0xFF000000) >> 24;
				}
				else if(strcmp(symbolName, SYMBOL_DYLD_STUB_BINDER) != 0)
				{
					printf("Unable to bind symbol %s\n", symbolName);
				}
				segmentAddress += (immediate * sizeof(void*)) + sizeof(void*);

				
				break;
				
			case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:

				tmp  = 0;
				bits = 0;
				do
				{
					tmp |= (bind_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(bind_stream[i] & 0x80);

				
				tmp2  = 0;
				bits = 0;
				do
				{
					tmp2 |= (bind_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(bind_stream[i] & 0x80);
				
				
				//DBG("BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB 0x%X 0x%X\n", tmp, tmp2);

				
				if(symbolAddr != 0xFFFFFFFF)
				{
					for(index = 0; index < tmp; index++)
					{
						
						address = segmentAddress + (UInt32)base;
						
						((char*)address)[0] = (symbolAddr & 0x000000FF) >> 0;
						((char*)address)[1] = (symbolAddr & 0x0000FF00) >> 8;
						((char*)address)[2] = (symbolAddr & 0x00FF0000) >> 16;
						((char*)address)[3] = (symbolAddr & 0xFF000000) >> 24;
						
						segmentAddress += tmp2 + sizeof(void*);
					}
				}
				else if(strcmp(symbolName, SYMBOL_DYLD_STUB_BINDER) != 0)
				{
					printf("Unable to bind symbol %s\n", symbolName);
				}
				
				
				break;
				
		}
		i++;
	}
}

inline void rebase_location(UInt32* location, char* base)
{
	*location += (UInt32)base;
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
	
	if(!moduleSymbols)
	{
		moduleSymbols = entry = malloc(sizeof(symbolList_t));

	}
	else
	{
		entry = moduleSymbols;
		while(entry->next)
		{
			entry = entry->next;
		}
		
		entry->next = malloc(sizeof(symbolList_t));
		entry = entry->next;
	}

	entry->next = NULL;
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
void module_loaded(char* name, UInt32 version, UInt32 compat)
{
	moduleList_t* entry;
	/*
	DBG("\%s.dylib Version %d.%d.%d loaded\n"
		   "\tCompatibility Version: %d.%d.%d\n",
		   name,
		   (version >> 16) & 0xFFFF,
		   (version >> 8) & 0x00FF,
		   (version >> 0) & 0x00FF,
		   (compat >> 16) & 0xFFFF,
		   (compat >> 8) & 0x00FF,
		   (compat >> 0) & 0x00FF);	
	*/
	if(loadedModules == NULL)
	{
		loadedModules = entry = malloc(sizeof(moduleList_t));
	}
	else
	{
		entry = loadedModules;
		while(entry->next)
		{
			entry = entry->next;
		}
		entry->next = malloc(sizeof(moduleList_t));
		entry = entry->next;
	}
	
	entry->next = NULL;
	entry->module = name;
	entry->version = version;
	entry->compat = compat;
	
	
}

int is_module_laoded(const char* name)
{
	moduleList_t* entry = loadedModules;
	while(entry)
	{
		if(strcmp(entry->module, name) == 0)
		{
			return 1;
		}
		else
		{
			entry = entry->next;
		}

	}
	return 0;
}

// Look for symbols using the Smbols moduel function.
// If non are found, look through the list of module symbols
unsigned int lookup_all_symbols(const char* name)
{
	unsigned int addr = 0xFFFFFFFF;
	if(lookup_symbol && (UInt32)lookup_symbol != 0xFFFFFFFF)
	{
		addr = lookup_symbol(name);
		if(addr != 0xFFFFFFFF)
		{
			//DBG("Internal symbol %s located at 0x%X\n", name, addr);
			return addr;
		}
	}

	
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
	if(strcmp(name, SYMBOL_DYLD_STUB_BINDER) != 0)
	{
		printf("Unable to locate symbol %s\n", name);
		getc();
	}
	return 0xFFFFFFFF;
}


/*
 * parse the symbol table
 * Lookup any undefined symbols
 */
 
unsigned int handle_symtable(UInt32 base, struct symtab_command* symtabCommand, long long(*symbol_handler)(char*, long long, char), char is64)
{
	// TODO: verify that the _TEXT,_text segment starts at the same locaiton in the file. If not
	//			subtract the vmaddress and add the actual file address back on. (NOTE: if compiled properly, not needed)
	
	unsigned int module_start = 0xFFFFFFFF;
	
	UInt32 symbolIndex = 0;
	char* symbolString = base + (char*)symtabCommand->stroff;
	//char* symbolTable = base + symtabCommand->symoff;
	if(!is64)
	{
		
		while(symbolIndex < symtabCommand->nsyms)
		{
			
			struct nlist* symbolEntry = (void*)base + symtabCommand->symoff + (symbolIndex * sizeof(struct nlist));
			
			// If the symbol is exported by this module
			if(symbolEntry->n_value &&
			   symbol_handler(symbolString + symbolEntry->n_un.n_strx, (long long)base + symbolEntry->n_value, is64) != 0xFFFFFFFF)
			{
				
				// Module start located. Start is an alias so don't register it
				module_start = base + symbolEntry->n_value;
			}
			
			symbolEntry+= sizeof(struct nlist);
			symbolIndex++;	// TODO remove
		}
	}
	else
	{
		
		while(symbolIndex < symtabCommand->nsyms)
		{
			
			struct nlist_64* symbolEntry = (void*)base + symtabCommand->symoff + (symbolIndex * sizeof(struct nlist_64));
			
			// If the symbol is exported by this module
			if(symbolEntry->n_value &&
			   symbol_handler(symbolString + symbolEntry->n_un.n_strx, (long long)base + symbolEntry->n_value, is64) != 0xFFFFFFFF)
			{
				
				// Module start located. Start is an alias so don't register it
				module_start = base + symbolEntry->n_value;
			}
			
			symbolEntry+= sizeof(struct nlist);
			symbolIndex++;	// TODO remove
		}
	}
		
	return module_start;
	
}


/*
 * Locate the symbol for an already loaded function and modify the beginning of
 * the function to jump directly to the new one
 * example: replace_function("_HelloWorld_start", &replacement_start);
 */
int replace_function(const char* symbol, void* newAddress)
{
	UInt32* jumpPointer = malloc(sizeof(UInt32*));	 
	// TODO: look into using the next four bytes of the function instead
	// Most functions should support this, as they probably will be at 
	// least 10 bytes long, but you never know, this is sligtly safer as
	// function can be as small as 6 bytes.
	UInt32 addr = lookup_all_symbols(symbol);
	
	char* binary = (char*)addr;
	if(addr != 0xFFFFFFFF)
	{
		*binary++ = 0xFF;	// Jump
		*binary++ = 0x25;	// Long Jump
		*((UInt32*)binary) = (UInt32)jumpPointer;
		
		*jumpPointer = (UInt32)newAddress;
		
		return 1;
	}
	else 
	{
		return 0;
	}

}