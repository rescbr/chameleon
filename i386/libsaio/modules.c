/*
 * Copyright 2010 Evan Lojewski. All rights reserved.
 *
 */

/*
 * Copyright 2012 Cadet-petit Armel <armelcadetpetit@gmail.com>. All rights reserved.
 *
 * Cleaned, Added (runtime) bundles module support, based on a dylib environement.
 *
 */
#include "libsaio.h"
#include "bootstruct.h"
#include "modules.h"

#ifndef DEBUG_MODULES
#define DEBUG_MODULES 0
#endif

#if DEBUG_MODULES
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

// NOTE: Global so that modules can link with this
unsigned long long textAddress = 0;
unsigned long long textSection = 0;

moduleHook_t* moduleCallbacks = NULL;
symbolList_t* moduleSymbols = NULL;
unsigned int (*lookup_symbol)(const char*, int(*strcmp_callback)(const char*, const char*)) = NULL;
moduleHook_t* get_callback(const char* name);
static unsigned int load_module(char * name, char* module);

#if DEBUG_MODULES
VOID print_hook_list()
{
	moduleHook_t* hooks = moduleCallbacks;
	printf("Hook list: \n");
	while(hooks)
	{
		printf("*  %s\n", hooks->name);
		hooks = hooks->next;
	}
	printf("\n");
}

VOID print_symbol_list()
{
	symbolList_t* symbol = moduleSymbols;
	printf("Symbol list: \n");
	while(symbol)
	{
		printf("*  %s : %s\n", symbol->module, symbol->symbol);
		symbol = symbol->next;
	}
	printf("\n");
}
#endif

#if DYLIB_SUPPORT

typedef struct dylbList_t
{
	char* dylib_name;
	struct dylbList_t* next;
} dylbList_t;

dylbList_t* loadedDylib = NULL;


static EFI_STATUS is_system_loaded(void)
{
	msglog("* Attempting to load system module\n");
    
    
    if((UInt32)lookup_symbol != 0xFFFFFFFF)
    {
#if	DEBUG_MODULES == 2
        printf("System successfully Loaded.\n");
        getc();    
#endif
        return EFI_SUCCESS;
    }
    
    
#if	DEBUG_MODULES == 2
    printf("Failed to load system module\n");
    getc();    
#endif
	return EFI_LOAD_ERROR;
}

/*
 * print out the information about the loaded module
 */
static VOID add_dylib(const char* name)
{
	dylbList_t* new_entry = malloc(sizeof(dylbList_t));
	if (new_entry)
	{	
		new_entry->next = loadedDylib;
		
		loadedDylib = new_entry;
		
		new_entry->dylib_name = (char*)name;		
	}
}

static EFI_STATUS is_dylib_loaded(const char* name)
{
	dylbList_t* entry = loadedDylib;
	while(entry)
	{
		DBG("Comparing %s with %s\n", name, entry->dylib_name);
		char *fullname = newStringWithFormat("%s.dylib",name);
		if(fullname && ((strcmp(entry->dylib_name, name) == 0) || (strcmp(entry->dylib_name, fullname) == 0)))
		{
			free(fullname);
			DBG("Located module %s\n", name);
			return EFI_SUCCESS;
		}
		else
		{
			entry = entry->next;
		}
		
	}
	DBG("Module %s not found\n", name);
	
	return EFI_NOT_FOUND;
}

/*
 * Load a dylib 
 *
 * ex: load_dylib("/Extra/modules/", "AcpiCodec.dylib");
 */
VOID load_dylib(const char * dylib_path, const char * name)
{
    void (*module_start)(void);

    char *tmp = newStringWithFormat("%s%s",dylib_path, name);								
    if (!tmp) {
        return;
    }
    char *dylib_name = newString(name);								
    if (!dylib_name) {
        free(tmp);
        return;
    }
    msglog("* Attempting to load module: %s\n", tmp);
    module_start = (void*)load_module(dylib_name,tmp);
    
    if(module_start && ( module_start != (void*)0xFFFFFFFF))
    {
        add_dylib(name);
        module_start();
    }
    else
    {
        // failed to load or already loaded
        free(tmp);
        free(dylib_name);
        
    }
}

/*
 * Load all dylib in the /Extra/modules/ directory
 */

VOID load_all_dylib(void)
{
	char* name;
	long flags;
	long time;
    
    if (is_system_loaded() != EFI_SUCCESS) return;
    
    long         ret, length, flags, time, bundleType;
    long long	 index;
    long         result = -1;
    const char * name;
    void (*module_start)(void);
	
	DBG("FileLoadBundles in %s\n",dirSpec);
    
    index = 0;
    while (1) {
        ret = GetDirEntry("/Extra/modules/", &index, &name, &flags, &time);
        if (ret == -1) break;       
		
        // Make sure this is not a directory.
        if ((flags & kFileTypeMask) == kFileTypeDirectory) continue;
        
        if ((strncmp("Symbols.dylib",name, sizeof("Symbols.dylib"))) == 0) continue; // if we found Symbols.dylib, just skip it
        
        if (is_dylib_loaded(name) == EFI_SUCCESS) continue;
        
        
        // Make sure this is a kext.
        length = strlen(name);
        if (strncmp(name + length - 6, ".dylib", 6)) continue;
		
        char *tmp = newStringWithFormat( "/Extra/modules/%s",name);								
        if (!tmp) {
            continue;
        }
        char *dylib_name = newString(name);								
        if (!dylib_name) {
            free(tmp);
            continue;
        }
        msglog("* Attempting to load module: %s\n", tmp);
        module_start = (void*)load_module(dylib_name,tmp);
        
        if(module_start && ( module_start != (void*)0xFFFFFFFF))
        {
            add_dylib(name);
            module_start();
        }
        else
        {
            // failed to load or already loaded
            free(tmp);
            free(dylib_name);
            
        }
    }	
	
#if DEBUG_MODULES
	print_symbol_list();
#endif
}

#endif

/*
 * Load a module file 
 */
static unsigned int load_module(char * name, char* module)
{
	unsigned int module_start = 0xFFFFFFFF;
    
	
	int fh = -1;
	
	fh = open(module);
	if(fh < 0)
	{		
#if DEBUG_MODULES
		DBG("Unable to locate module %s\n", module);
		getc();
#else
		msglog("Unable to locate module %s\n", module);
#endif
		return 0xFFFFFFFF;		
	}
	
	{
		int moduleSize = file_size(fh);
		
		char* module_base = NULL;
		
		if (moduleSize > 0)
		{
			module_base = (char*) malloc(moduleSize);
		}		
		
		if (!module_base)
		{
			goto out;
		}
		
		if (read(fh, module_base, moduleSize) == moduleSize)
		{
			
			DBG("Module %s read in.\n", module);
			
			// Module loaded into memory, parse it
			module_start = parse_mach(name, module_base, &add_symbol);            
			
			
		}
		else
		{
			printf("Unable to read in module %s\n.", module);
#if DEBUG_MODULES		
			getc();
#endif
			free(module_base);
		}
	}
out:
	close(fh);
	return module_start;
}

moduleHook_t* get_callback(const char* name)
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
	return NULL;
	
}

/*
 *	execute_hook(  const char* name )
 *		name - Name of the module hook
 *			If any callbacks have been registered for this hook
 *			they will be executed now in the same order that the
 *			hooks were added.
 */
EFI_STATUS execute_hook(const char* name, void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	DBG("Attempting to execute hook '%s'\n", name);
	moduleHook_t* hook = get_callback(name);
	
	if(hook)
	{
		// Loop through all callbacks for this module
		callbackList_t* callbacks = hook->callbacks;
		
		while(callbacks)
		{
			// Execute callback
			callbacks->callback(arg1, arg2, arg3, arg4, arg5, arg6);
			callbacks = callbacks->next;
		}
		DBG("Hook '%s' executed.\n", name); 
		return EFI_SUCCESS;
	}
	
	// Callback for this hook doesn't exist;
	DBG("No callbacks for '%s' hook.\n", name);
	return EFI_NOT_FOUND;
	
}

/*
 *	register_hook_callback(  const char* name,  void(*callback)())
 *		name - Name of the module hook to attach to.
 *		callbacks - The funciton pointer that will be called when the
 *			hook is executed. When registering a new callback name, the callback is added sorted.
 *			NOTE: the hooks take four void* arguments.
 */
static VOID __register_hook_callback(moduleHook_t* hook, const char* name, void(*callback)(void*, void*, void*, void*, void*, void*))
{
	if(hook)
	{
		// append
		callbackList_t* newCallback = malloc(sizeof(callbackList_t));
		if (!newCallback) {
			DBG("Unable to allocate memory for callback \n");
			return;
		}
		newCallback->next = hook->callbacks;
		hook->callbacks = newCallback;
		newCallback->callback = callback;
	}
	else
	{
		// create new hook
		moduleHook_t* newHook = malloc(sizeof(moduleHook_t));
		if (!newHook) {
			DBG("Unable to allocate memory for hook '%s'.\n", name);
			return;
		}
		newHook->name = name;
		newHook->callbacks = malloc(sizeof(callbackList_t));
		if (!newHook->callbacks) {
			DBG("Unable to allocate memory for callback \n");
			free(newHook);
			return;
		}
		newHook->callbacks->callback = callback;
		newHook->callbacks->next = NULL;
		
		newHook->next = moduleCallbacks;
		moduleCallbacks = newHook;
		
	}
	
#if DEBUG_MODULES
	print_hook_list();
	getc();
#endif
}

VOID register_hook_callback(const char* name, void(*callback)(void*, void*, void*, void*, void*, void*))
{	
	DBG("Adding callback for hook '%s'.\n", name);
	
	moduleHook_t* hook = get_callback(name);
	
	__register_hook_callback(hook, name, callback);
}

VOID register_one_callback(const char* name, void(*callback)(void*, void*, void*, void*, void*, void*))
{	
	DBG("Adding one callback for hook '%s'.\n", name);
	
	moduleHook_t* hook = get_callback(name);
	
	if (hook) 
	{
		return;
	}	
	__register_hook_callback(hook, name, callback);
}

#if DEBUG_MODULES
unsigned long vmaddr;
long   vmsize;
#endif

/*
 * Parse through a macho module. The module will be rebased and binded
 * as specified in the macho header. If the module is successfully loaded
 * the module iinit address will be returned.
 * NOTE; all dependecies will be loaded before this module is started
 * NOTE: If the module is unable to load ot completeion, the modules
 * symbols will still be available (TODO: fix this). This should not
 * happen as all dependencies are verified before the sybols are read in.
 */
#if macho_64
unsigned int parse_mach(char *module, void* binary, long long(*symbol_handler)(char*, char*, long long, char))	// TODO: add param to specify valid archs
#else
unsigned int parse_mach(char *module, void* binary, long long(*symbol_handler)(char*, char*, long long))	// TODO: add param to specify valid archs
#endif
{	
#if macho_64
	char is64 = false;
#endif
	unsigned int module_start = 0xFFFFFFFF;
	EFI_STATUS bind_status = EFI_SUCCESS;
    
	// TODO convert all of the structs to a union	
	struct dyld_info_command* dyldInfoCommand = NULL;	
	struct symtab_command* symtabCommand = NULL;	
	
	{
		struct segment_command *segCommand = NULL;
#if macho_64
		struct segment_command_64 *segCommand64 = NULL;
#endif
		struct load_command *loadCommand = NULL;
		UInt32 binaryIndex = 0;
		UInt16 cmd = 0;
		
		// Parse through the load commands
		if(((struct mach_header*)binary)->magic == MH_MAGIC)
		{
#if macho_64
			is64 = false;
#endif
			binaryIndex += sizeof(struct mach_header);
		}
#if macho_64
		else if(((struct mach_header_64*)binary)->magic == MH_MAGIC_64)
		{
			// NOTE: modules cannot be 64bit...
			is64 = true;
			binaryIndex += sizeof(struct mach_header_64);
		}
#endif
		else
		{
			printf("Modules: Invalid mach magic\n");
			getc();
			return 0xFFFFFFFF;
		}
		
		
		
		/*if(((struct mach_header*)binary)->filetype != MH_DYLIB)
		 {
		 printf("Module is not a dylib. Unable to load.\n");
		 getc();
		 return NULL; // Module is in the incorrect format
		 }*/
		
		while(cmd < ((struct mach_header*)binary)->ncmds)
		{
			cmd++;
			
			loadCommand = binary + binaryIndex;
			UInt32 cmdSize = loadCommand->cmdsize;
			
			
			switch ((loadCommand->cmd & 0x7FFFFFFF))
			{
				case LC_SYMTAB:
					symtabCommand = binary + binaryIndex;
					break;
					
				case LC_SEGMENT: // 32bit macho
				{
					segCommand = binary + binaryIndex;
					
					//printf("Segment name is %s\n", segCommand->segname);
					
					if(strncmp("__TEXT", segCommand->segname, sizeof("__TEXT")) == 0)
					{
						UInt32 sectionIndex;
						
#if DEBUG_MODULES
						unsigned long fileaddr;
						long   filesize;					
						vmaddr = (segCommand->vmaddr & 0x3fffffff);
						vmsize = segCommand->vmsize;	  
						fileaddr = ((unsigned long)(binary + binaryIndex) + segCommand->fileoff);
						filesize = segCommand->filesize;
						
						printf("segname: %s, vmaddr: %x, vmsize: %x, fileoff: %x, filesize: %x, nsects: %d, flags: %x.\n",
							   segCommand->segname, (unsigned)vmaddr, (unsigned)vmsize, (unsigned)fileaddr, (unsigned)filesize,
							   (unsigned) segCommand->nsects, (unsigned)segCommand->flags);
#if DEBUG_MODULES==2
						
						getc();
#endif
#endif
						
						sectionIndex = sizeof(struct segment_command);
						
						struct section *sect;
						
						while(sectionIndex < segCommand->cmdsize)
						{
							sect = binary + binaryIndex + sectionIndex;
							
							sectionIndex += sizeof(struct section);
							
							
							if(strncmp("__text", sect->sectname,sizeof("__text")) == 0)
							{
								// __TEXT,__text found, save the offset and address for when looking for the calls.
								textSection = sect->offset;
								textAddress = sect->addr;
								break;
							}					
						}
					}
					break;
				}	
#if macho_64
				case LC_SEGMENT_64:	// 64bit macho's
				{
					segCommand64 = binary + binaryIndex;
					
					//printf("Segment name is %s\n", segCommand->segname);
					
					if(strncmp("__TEXT", segCommand64->segname, sizeof("__TEXT")) == 0)
					{
						UInt32 sectionIndex;
						
#if DEBUG_MODULES						
						unsigned long fileaddr;
						long   filesize;					
						vmaddr = (segCommand64->vmaddr & 0x3fffffff);
						vmsize = segCommand64->vmsize;	  
						fileaddr = ((unsigned long)(binary + binaryIndex) + segCommand64->fileoff);
						filesize = segCommand64->filesize;
						
						printf("segname: %s, vmaddr: %x, vmsize: %x, fileoff: %x, filesize: %x, nsects: %d, flags: %x.\n",
							   segCommand64->segname, (unsigned)vmaddr, (unsigned)vmsize, (unsigned)fileaddr, (unsigned)filesize,
							   (unsigned) segCommand64->nsects, (unsigned)segCommand64->flags);
#if DEBUG_MODULES==2
						
						getc();
#endif
#endif
						
						sectionIndex = sizeof(struct segment_command_64);
						
						struct section_64 *sect;
						
						while(sectionIndex < segCommand64->cmdsize)
						{
							sect = binary + binaryIndex + sectionIndex;
							
							sectionIndex += sizeof(struct section_64);
							
							
							if(strncmp("__text", sect->sectname, sizeof("__text")) == 0)
							{
								// __TEXT,__text found, save the offset and address for when looking for the calls.
								textSection = sect->offset;
								textAddress = sect->addr;
								
								break;
							}					
						}
					}
					
					break;
				}
#endif
				case LC_DYSYMTAB:
					break;
					
				case LC_LOAD_DYLIB:
				case LC_LOAD_WEAK_DYLIB ^ LC_REQ_DYLD:
                    break;
                    
				case LC_ID_DYLIB:
                    break;
                    
					
				case LC_DYLD_INFO:
					// Bind and rebase info is stored here
					dyldInfoCommand = binary + binaryIndex;
					break;
					
				case LC_UUID:
					break;
					
				case LC_UNIXTHREAD:
					break;
					
				default:
					DBG("Unhandled loadcommand 0x%X\n", loadCommand->cmd & 0x7FFFFFFF);
					break;
					
			}
			
			binaryIndex += cmdSize;
		}
		//if(!moduleName) return NULL;
	}		
	
	// bind_macho uses the symbols.
#if macho_64
	module_start = handle_symtable(module, (UInt32)binary, symtabCommand, symbol_handler, is64);
#else
	module_start = handle_symtable(module, (UInt32)binary, symtabCommand, symbol_handler);
#endif
	// Rebase the module before binding it.
	if(dyldInfoCommand && dyldInfoCommand->rebase_off)
	{
		rebase_macho(binary, (char*)dyldInfoCommand->rebase_off, dyldInfoCommand->rebase_size);
	}
	
	if(dyldInfoCommand && dyldInfoCommand->bind_off)
	{
		bind_status = bind_macho(module, binary, (char*)dyldInfoCommand->bind_off, dyldInfoCommand->bind_size);
	}
	
	if(dyldInfoCommand && dyldInfoCommand->weak_bind_off && (bind_status == EFI_SUCCESS))
	{
		// NOTE: this currently should never happen.
		bind_status = bind_macho(module, binary, (char*)dyldInfoCommand->weak_bind_off, dyldInfoCommand->weak_bind_size);
	}
	
	if(dyldInfoCommand && dyldInfoCommand->lazy_bind_off && (bind_status == EFI_SUCCESS))
	{
		// NOTE: we are binding the lazy pointers as a module is laoded,
		// This should be changed to bind when a symbol is referened at runtime instead.
		bind_status = bind_macho(module, binary, (char*)dyldInfoCommand->lazy_bind_off, dyldInfoCommand->lazy_bind_size);
	}
	
    if (bind_status != EFI_SUCCESS) {
        module_start = 0xFFFFFFFF;
    }
    
	return  module_start;
	
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
	UInt32 index = 0;
	
	//int done = 0;
	unsigned int i = 0;
	
	while(/*!done &&*/ i < size)
	{
		immediate = rebase_stream[i] & REBASE_IMMEDIATE_MASK;
		opcode = rebase_stream[i] & REBASE_OPCODE_MASK;
		
		
		switch(opcode)
		{
			case REBASE_OPCODE_DONE:
				// Rebase complete.
				//done = 1;
				break;
				
				
			case REBASE_OPCODE_SET_TYPE_IMM:
				// Set rebase type (pointer, absolute32, pcrel32)
				//DBG("Rebase type = 0x%X\n", immediate);
				type = immediate;
				break;
				
				
			case REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
			{
				// Locate address to begin rebasing
				//segmentAddress = 0;
				
				struct segment_command* segCommand = NULL; // NOTE: 32bit only
				
				{
					unsigned int binIndex = 0;
					index = 0;
					do
					{
						segCommand = base + sizeof(struct mach_header) +  binIndex;
						
						
						binIndex += segCommand->cmdsize;
						index++;
					}
					while(index <= immediate);
				}				
				
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
			}				
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
				for (index = 0; index < immediate; ++index)
				{
					rebase_location(base + segmentAddress, (char*)base, type);
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
				
				for (index = 0; index < tmp; ++index)
				{
					//DBG("\tRebasing 0x%X\n", segmentAddress);
					rebase_location(base + segmentAddress, (char*)base, type);					
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
				
				rebase_location(base + segmentAddress, (char*)base, type);
				
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
				
				for (index = 0; index < tmp; ++index)
				{
					
					rebase_location(base + segmentAddress, (char*)base, type);
					
					segmentAddress += tmp2 + sizeof(void*);
				}
				break;
			default:
				break;
		}
		i++;
	}
}

// Based on code from dylibinfo.cpp and ImageLoaderMachOCompressed.cpp
// NOTE: this uses 32bit values, and not 64bit values. 
// There is apossibility that this could cause issues,
// however the macho file is 32 bit, so it shouldn't matter too much
EFI_STATUS bind_macho(char* module, void* base, char* bind_stream, UInt32 size)
{	
	bind_stream += (UInt32)base;
	
	UInt8 immediate = 0;
	UInt8 opcode = 0;
	
	UInt32 segmentAddress = 0;
	
	UInt32 address = 0;
	
	SInt32 addend = 0;			// TODO: handle this
	
	const char* symbolName = NULL;
	UInt32 symbolAddr = 0xFFFFFFFF;
	
	// Temperary variables
	UInt8 bits = 0;
	UInt32 tmp = 0;
	UInt32 tmp2 = 0;
	
	UInt32 index = 0;
	unsigned int i = 0;
	
	while(i < size)
	{
		immediate = bind_stream[i] & BIND_IMMEDIATE_MASK;
		opcode = bind_stream[i] & BIND_OPCODE_MASK;
		
		
		switch(opcode)
		{
			case BIND_OPCODE_DONE:
				break;
				
			case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
            {
#if DEBUG_MODULES==2
				SInt32 libraryOrdinal = immediate;
				DBG("BIND_OPCODE_SET_DYLIB_ORDINAL_IMM: %d\n", libraryOrdinal);
#endif
				break;
            }
			case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
            {
#if DEBUG_MODULES==2
				SInt32 libraryOrdinal = 0;
				UInt8 bits = 0;
				do
				{
					libraryOrdinal |= (bind_stream[++i] & 0x7f) << bits;
					bits += 7;
				}
				while(bind_stream[i] & 0x80);
				
				DBG("BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB: %d\n", libraryOrdinal);
#else
                do
				{
					i++;
				}
				while(bind_stream[i] & 0x80);
#endif
				break;
			}	
			case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
            {
#if DEBUG_MODULES==2
                // NOTE: this is wrong, fortunately we don't use it
				SInt32 libraryOrdinal = -immediate;
				DBG("BIND_OPCODE_SET_DYLIB_SPECIAL_IMM: %d\n", libraryOrdinal);
#endif		
				break;
			}	
			case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
            {
				symbolName = (char*)&bind_stream[++i];
				i += strlen((char*)&bind_stream[i]);
#if DEBUG_MODULES==2
				UInt8 symbolFlags = immediate;
                DBG("BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM: %s, 0x%X\n", symbolName, symbolFlags);
#endif				
				symbolAddr = lookup_all_symbols(NULL ,symbolName);
				
				break;
            }
			case BIND_OPCODE_SET_TYPE_IMM:
            {
				// Set bind type (pointer, absolute32, pcrel32)
#if DEBUG_MODULES==2
                UInt8 type = immediate;
				DBG("BIND_OPCODE_SET_TYPE_IMM: %d\n", type);
#endif
				break;
            }
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
#if DEBUG_MODULES==2
				DBG("BIND_OPCODE_SET_ADDEND_SLEB: %d\n", addend);
#endif
                
				break;
				
			case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
			{				
				// Locate address
				struct segment_command* segCommand = NULL;	// NOTE: 32bit only
				
				{
					unsigned int binIndex = 0;
					index = 0;
					do
					{
						segCommand = base + sizeof(struct mach_header) +  binIndex;
						binIndex += segCommand->cmdsize;
						index++;
					}while(index <= immediate);
				}
				
				
				segmentAddress = segCommand->fileoff;
				
				// Read in offset
				tmp  = 0;
				bits = 0;
				do
				{
					tmp |= (bind_stream[++i] & 0x7f) << bits;
					bits += 7;
				}while(bind_stream[i] & 0x80);
				
				segmentAddress += tmp;
#if DEBUG_MODULES==2
				DBG("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB: 0x%X\n", segmentAddress);
#endif
				break;
			}				
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
#if DEBUG_MODULES==2
				DBG("BIND_OPCODE_ADD_ADDR_ULEB: 0x%X\n", segmentAddress);
#endif
                
				break;
				
			case BIND_OPCODE_DO_BIND:
#if DEBUG_MODULES==2
				DBG("BIND_OPCODE_DO_BIND\n");
#endif                
                
                if(symbolAddr != 0xFFFFFFFF)
				{
					address = segmentAddress + (UInt32)base;
					
					bind_location((UInt32*)address, (char*)symbolAddr, addend, BIND_TYPE_POINTER);
				}
				else if(symbolName && (strncmp(symbolName, SYMBOL_DYLD_STUB_BINDER,sizeof(SYMBOL_DYLD_STUB_BINDER)) != 0))
				{
					printf("Unable to bind symbol %s needed by %s\n", symbolName, module);
                    getc();
                    goto error;
                    
				}
				
				segmentAddress += sizeof(void*);
				break;
				
			case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
#if DEBUG_MODULES==2
				DBG("BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB\n");
#endif		
				
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
					
					bind_location((UInt32*)address, (char*)symbolAddr, addend, BIND_TYPE_POINTER);
				}
				else if(symbolName && (strncmp(symbolName, SYMBOL_DYLD_STUB_BINDER,sizeof(SYMBOL_DYLD_STUB_BINDER)) != 0))
				{
					printf("Unable to bind symbol %s needed by %s\n", symbolName, module);
                    getc();
                    goto error;                    
                    
				}
                
				segmentAddress += tmp + sizeof(void*);
				
				
				break;
				
			case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
#if DEBUG_MODULES==2
				DBG("BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED\n");
#endif						
                
                if(symbolAddr != 0xFFFFFFFF)
				{
					address = segmentAddress + (UInt32)base;
					
					bind_location((UInt32*)address, (char*)symbolAddr, addend, BIND_TYPE_POINTER);
				}
				else if(symbolName && (strncmp(symbolName, SYMBOL_DYLD_STUB_BINDER,sizeof(SYMBOL_DYLD_STUB_BINDER)) != 0))
				{
					printf("Unable to bind symbol %s needed by %s\n", symbolName, module);
                    getc();
                    goto error;                    
                    
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
				
#if DEBUG_MODULES==2
				DBG("BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB 0x%X 0x%X\n", tmp, tmp2);
#endif				
                
                if(symbolAddr != 0xFFFFFFFF)
				{
					for(index = 0; index < tmp; index++)
					{
						
						address = segmentAddress + (UInt32)base;
						
						bind_location((UInt32*)address, (char*)symbolAddr, addend, BIND_TYPE_POINTER);
						
						segmentAddress += tmp2 + sizeof(void*);
					}
				}
				else if(symbolName && (strncmp(symbolName, SYMBOL_DYLD_STUB_BINDER,sizeof(SYMBOL_DYLD_STUB_BINDER)) != 0))
				{
					printf("Unable to bind symbol %s needed by %s\n", symbolName, module);
                    getc();
                    goto error;                    
                    
				}				
				
				break;
			default:
				break;
				
		}
		i++;
	}
    return EFI_SUCCESS;
error:
    return EFI_NOT_FOUND;
}

void rebase_location(UInt32* location, char* base, int type)
{	
	switch(type)
	{
		case REBASE_TYPE_POINTER:
		case REBASE_TYPE_TEXT_ABSOLUTE32:
			*location += (UInt32)base;
			break;
			
		default:
			break;
	}
}

void bind_location(UInt32* location, char* value, UInt32 addend, int type)
{	
	// do actual update
	char* newValue = value + addend;
	
	switch (type) {
		case BIND_TYPE_POINTER:
		case BIND_TYPE_TEXT_ABSOLUTE32:
			break;
			
		case BIND_TYPE_TEXT_PCREL32:
			newValue -=  ((UInt32)location + 4);
			
			break;
		default:
			return;
	}
	*location = (UInt32)newValue;
	
	
}

/*
 * add_symbol
 * This function adds a symbol from a module to the list of known symbols 
 * possibly change to a pointer and add this to the Symbol module so that it can
 * adjust it's internal symbol list (sort) to optimize locating new symbols
 * NOTE: returns the address if the symbol is "start", else returns 0xFFFFFFFF
 */
#if macho_64
long long add_symbol(char* module,char* symbol, long long addr, char is64)
#else
long long add_symbol(char* module,char* symbol, long long addr)
#endif
{
#if macho_64
	if(is64) return  0xFFFFFFFF; // Fixme
#endif
	// This only can handle 32bit symbols 
	symbolList_t* new_entry= malloc(sizeof(symbolList_t));
	DBG("Adding symbol %s at 0x%X\n", symbol, addr);
	if (new_entry)
	{	
		new_entry->next = moduleSymbols;
		
		moduleSymbols = new_entry;
		
		new_entry->addr = (UInt32)addr;
        new_entry->module = module;
		new_entry->symbol = symbol;
		return addr;
	}	
	
	return 0xFFFFFFFF;
	
}

// Look for symbols using the Smbols moduel function.
// If non are found, look through the list of module symbols
unsigned int lookup_all_symbols(const char* module, const char* name)
{
    
    unsigned int addr = 0xFFFFFFFF;
    
    do {
        
        if ((module != NULL) && (strncmp(module,SYMBOLS_BUNDLE,sizeof(SYMBOLS_BUNDLE)) != 0))
            break;        
        
        if(lookup_symbol && (UInt32)lookup_symbol != 0xFFFFFFFF)
        {
            addr = lookup_symbol(name, &strcmp);
            if(addr != 0xFFFFFFFF)
            {
                DBG("Internal symbol %s located at 0x%X\n", name, addr);
                goto out;
            }
        } 
        
    } while (0);
    
    
	{
		symbolList_t* entry = moduleSymbols;
		while(entry)
		{
            if ((module != NULL) && (strcmp(entry->module,module) != 0))
            {
                entry = entry->next;
                continue; 
            }
            
            if(strcmp(entry->symbol, name) == 0)
            {
                DBG("External symbol %s located at 0x%X\n", name, entry->addr);
                addr = entry->addr;
                goto out;
            }
            else
            {
                entry = entry->next;
            }             
            
		}
	}
	
#if DEBUG_MODULES
	if(strncmp(name, SYMBOL_DYLD_STUB_BINDER, sizeof(SYMBOL_DYLD_STUB_BINDER)) != 0)
	{
		verbose("Unable to locate symbol %s\n", name);
		getc();
	}
#endif
out:
    return addr;
    
}


/*
 * parse the symbol table
 * Lookup any undefined symbols
 */
#if macho_64
unsigned int handle_symtable(char *module, UInt32 base, struct symtab_command* symtabCommand, long long(*symbol_handler)(char*, char*, long long, char), char is64)
#else
unsigned int handle_symtable(char *module, UInt32 base, struct symtab_command* symtabCommand, long long(*symbol_handler)(char*, char*, long long))
#endif
{		
	unsigned int module_start = 0xFFFFFFFF;
	
	UInt32 symbolIndex = 0;
    if (!symtabCommand) {
        return 0xFFFFFFFF;
    }
	char* symbolString = base + (char*)symtabCommand->stroff;
#if macho_64
	if(!is64)
#endif
	{
		struct nlist* symbolEntry = (void*)base + symtabCommand->symoff;
		while(symbolIndex < symtabCommand->nsyms)
		{						
			if(symbolEntry->n_value)
			{				
				if(strstr(symbolString + symbolEntry->n_un.n_strx, "module_start") || (strncmp(symbolString + symbolEntry->n_un.n_strx, "start", sizeof("start")) == 0))
				{
					module_start = base + symbolEntry->n_value;
					DBG("n_value %x module_start %x\n", (unsigned)symbolEntry->n_value, (unsigned)module_start);
				}
				else
				{
#if macho_64
					symbol_handler(module, symbolString + symbolEntry->n_un.n_strx, (long long)base + symbolEntry->n_value, is64);
#else
					symbol_handler(module, symbolString + symbolEntry->n_un.n_strx, (long long)base + symbolEntry->n_value);
#endif

				}
#if DEBUG_MODULES
				bool isTexT = (((unsigned)symbolEntry->n_value > (unsigned)vmaddr) && ((unsigned)(vmaddr + vmsize) > (unsigned)symbolEntry->n_value ));
				printf("%s %s\n", isTexT ? "__TEXT :" : "__DATA(OR ANY) :", symbolString + symbolEntry->n_un.n_strx);
#if DEBUG_MODULES==2	
				
				if(strcmp(symbolString + symbolEntry->n_un.n_strx, "_BootHelp_txt") == 0)
				{
					long long addr = (long long)base + symbolEntry->n_value;
					unsigned char *BootHelp = NULL;
					BootHelp  = (unsigned char*)(UInt32)addr;					
					printf("method 1: __DATA : BootHelp_txt[0] %x\n", BootHelp[0]);
					
					long long addr2 = symbolEntry->n_value;
					unsigned char *BootHelp2 = NULL;
					BootHelp2  = (unsigned char*)(UInt32)addr2;					
					printf("method 2:  __DATA : BootHelp_txt[0] %x\n", BootHelp2[0]);
				}
#endif
#endif
				
			}
			
			symbolEntry++;
			symbolIndex++;	// TODO remove
		}
	}
#if macho_64
	else
	{
		struct nlist_64* symbolEntry = (void*)base + symtabCommand->symoff;
		// NOTE First entry is *not* correct, but we can ignore it (i'm getting radar:// right now)	
		while(symbolIndex < symtabCommand->nsyms)
		{	
			
			if(strstr(symbolString + symbolEntry->n_un.n_strx, "module_start") || (strncmp(symbolString + symbolEntry->n_un.n_strx, "start",sizeof("start")) == 0))
			{
				module_start = (unsigned int)(base + symbolEntry->n_value);
			}
			else
			{
				symbol_handler(module, symbolString + symbolEntry->n_un.n_strx, (long long)base + symbolEntry->n_value, is64);
			}
			
			symbolEntry++;
			symbolIndex++;	// TODO remove
		}
	}
#endif
	
	return module_start;
	
}


/*
 * Locate the symbol for an already loaded function and modify the beginning of
 * the function to jump directly to the new one
 * example: replace_system_function("_getc", &replacement);
 *          replace_function(module_name,"_getc", &replacement);
 *          replace_function_any("_getc", &replacement);
 */
EFI_STATUS replace_function(const char* module, const char* symbol, void* newAddress)
{		 
	// TODO: look into using the next four bytes of the function instead
	// Most functions should support this, as they probably will be at 
	// least 10 bytes long, but you never know, this is sligtly safer as
	// function can be as small as 6 bytes.
	UInt32 addr = lookup_all_symbols(module, symbol);
	
	char* binary = (char*)addr;
	if(addr != 0xFFFFFFFF)
	{
		UInt32* jumpPointer = malloc(sizeof(UInt32));
		if (!jumpPointer) {
			return EFI_OUT_OF_RESOURCES;
		}
		
		*binary++ = 0xFF;	// Jump
		*binary++ = 0x25;	// Long Jump
		*((UInt32*)binary) = (UInt32)jumpPointer;
		
		*jumpPointer = (UInt32)newAddress;
		
		return EFI_SUCCESS;
	}
	
	return EFI_NOT_FOUND;
	
}

EFI_STATUS replace_system_function(const char* symbol, void* newAddress)
{		 
	
	return replace_function(SYMBOLS_BUNDLE,symbol,newAddress);
	
}

EFI_STATUS replace_function_any(const char* symbol, void* newAddress)
{		 
	
	return replace_function(NULL,symbol,newAddress);
	
}
/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the 'License').  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 *  drivers.c - Driver Loading Functions.
 *
 *  Copyright (c) 2000 Apple Computer, Inc.
 *
 *  DRI: Josh de Cesare
 */

/*
 * Copyright 2012 Cadet-petit Armel <armelcadetpetit@gmail.com>. All rights reserved.
 *
 * Cleaned, Added (runtime) bundles support.
 *
 */
#include <mach-o/fat.h>
#include <libkern/OSByteOrder.h>
#include <mach/machine.h>

#include "sl.h"
#include "xml.h"

struct Module {  
	struct Module *nextModule;
	long          willLoad;
	TagPtr        dict;
    TagPtr        personalities;
#if 0
	config_file_t *LanguageConfig; // we will use an xml file instead of pure txt file ... it's easier to parse
#endif
	//pool_t        workspace;
	char          *plistAddr;
	long          plistLength;
	char          *executablePath;
	char          *bundlePath;
	long          bundlePathLength;
};
typedef struct Module Module, *ModulePtr;


enum {
	kCFBundleType2,
	kCFBundleType3
};

enum {
	BundlePriorityNull = 0,
	BundlePriorityInit = 1,
	BundlePrioritySystem = 2,
	BundlePrioritySystemLib = 3,
    BundlePriorityNormalPriority = 4,
    BundlePriorityLowestPriority = 99,
	BundlePriorityEnd = 100 // can not be assigned
    
};

static ModulePtr gModuleHead;
static char *    gModulesSpec;
static char *    gDriverSpec;
static char *    gFileSpec;
static char *    gTempSpec;
static char *    gFileName;
static int       gLowestLoadPriority;

static long ParseXML(char *buffer, ModulePtr *module);
static ModulePtr FindBundle( char * bundle_id );

static void
FreeBundleSupport( void )
{
    
    if ( gModulesSpec )         free(gModulesSpec);
    if ( gDriverSpec)           free(gDriverSpec);
    if ( gFileSpec  )           free(gFileSpec);
    if ( gTempSpec )            free(gTempSpec);
    if ( gFileName )            free(gFileName);
}

//==========================================================================
// InitBundleSupport

long
InitBundleSupport( void )
{
	DBG("InitBundleSupport\n");
    
    static bool BundleSet = false;
    
    if (BundleSet == true)  return 0;    
    
    gModulesSpec    = malloc( DEFAULT_BUNDLE_SPEC_SIZE );
    gDriverSpec     = malloc( DEFAULT_BUNDLE_SPEC_SIZE );
    gFileSpec       = malloc( DEFAULT_BUNDLE_SPEC_SIZE );
    gTempSpec       = malloc( DEFAULT_BUNDLE_SPEC_SIZE );
    gFileName       = malloc( DEFAULT_BUNDLE_SPEC_SIZE );
    
    if ( !gModulesSpec || !gDriverSpec || !gFileSpec || !gTempSpec || !gFileName )
        goto error;
    
    BundleSet = true;
    
    return 0;
error:
    FreeBundleSupport();
    return 1;
}

//==========================================================================
// LoadBundles

long LoadBundles( char * dirSpec )
{	
	DBG("LoadBundles\n");
    
    if ( InitBundleSupport() != 0 )
        return 1;
	
	
	strlcpy(gModulesSpec, dirSpec, DEFAULT_BUNDLE_SPEC_SIZE);
    strlcat(gModulesSpec, "Modules", DEFAULT_BUNDLE_SPEC_SIZE);
    FileLoadBundles(gModulesSpec, 0);		
    
	
    MatchBundlesLibraries();
	
    LoadMatchedBundles();
    
	DBG("LoadBundles Finished\n");
    
    return 0;
}

//==========================================================================
// FileLoadBundles
long
FileLoadBundles( char * dirSpec, long plugin )
{
    long         ret, length, flags, time, bundleType;
    long long	 index;
    long         result = -1;
    const char * name;
    
	DBG("FileLoadBundles in %s\n",dirSpec);
    
    index = 0;
    while (1) {
        ret = GetDirEntry(dirSpec, &index, &name, &flags, &time);
        if (ret == -1) break;
		
        // Make sure this is a directory.
        if ((flags & kFileTypeMask) != kFileTypeDirectory) continue;
        
        // Make sure this is a kext.
        length = strlen(name);
        if (strncmp(name + length - 7, ".bundle",7)) continue;
		
        // Save the file name.
        strlcpy(gFileName, name, DEFAULT_BUNDLE_SPEC_SIZE);
		DBG("Load Bundles %s\n",gFileName);
        
        // Determine the bundle type.
        snprintf(gTempSpec,DEFAULT_BUNDLE_SPEC_SIZE,"%s/%s", dirSpec, gFileName);
        ret = GetFileInfo(gTempSpec, "Contents", &flags, &time);
        if (ret == 0) bundleType = kCFBundleType2;
        else bundleType = kCFBundleType3;
		
		DBG("Bundles type = %d\n",bundleType);
        
        if (!plugin)
            snprintf(gDriverSpec, DEFAULT_BUNDLE_SPEC_SIZE,"%s/%s/%sPlugIns", dirSpec, gFileName,
                    (bundleType == kCFBundleType2) ? "Contents/" : "");
		
        ret = LoadBundlePList( dirSpec, gFileName, bundleType);
		
        if (result != 0)
			result = ret;
		
        if (!plugin) 
			FileLoadBundles(gDriverSpec, 1);
    }
	
    return result;
}


static void add_bundle(ModulePtr module,char* name)
{
	ModulePtr new_entry= malloc(sizeof(Module));
	DBG("Adding bundle %s \n", name );
	if (new_entry)
	{	
		new_entry->nextModule = gModuleHead;
		
		gModuleHead = new_entry;
		
		new_entry->executablePath = module->executablePath;
        new_entry->bundlePath = module->bundlePath;
        new_entry->bundlePathLength = module->bundlePathLength;
        new_entry->plistAddr = module->plistAddr;
        new_entry->willLoad = module->willLoad;
        new_entry->dict = module->dict;
        new_entry->plistLength = module->plistLength;
        new_entry->personalities = module->personalities;
        
	}	
	
}

//==========================================================================
// LoadBundlePList

long
LoadBundlePList( char * dirSpec, char * name, long bundleType )
{
    long      length, executablePathLength, bundlePathLength;
    ModulePtr module = 0;
    char *    buffer = 0;
    char *    tmpExecutablePath = 0;
    char *    tmpBundlePath = 0;
    long      ret = -1;
	DBG("LoadBundlePList\n");
    
    do {
        // Save the driver path.
        
        snprintf(gFileSpec,DEFAULT_BUNDLE_SPEC_SIZE,"%s/%s/%s", dirSpec, name,
                (bundleType == kCFBundleType2) ? "Contents/MacOS/" : "");
        executablePathLength = strlen(gFileSpec) + 1;
		
        tmpExecutablePath = malloc(executablePathLength);
        if (tmpExecutablePath == 0) break;
        
        strlcpy(tmpExecutablePath, gFileSpec, executablePathLength);
        
        snprintf(gFileSpec, DEFAULT_BUNDLE_SPEC_SIZE,"%s/%s", dirSpec, name);
        bundlePathLength = strlen(gFileSpec) + 1;
		
        tmpBundlePath = malloc(bundlePathLength);
        if (tmpBundlePath == 0) break;
		
        strlcpy(tmpBundlePath, gFileSpec, bundlePathLength);
        
        
        // Construct the file spec to the plist, then load it.
		
        snprintf(gFileSpec, DEFAULT_BUNDLE_SPEC_SIZE,"%s/%s/%sInfo.plist", dirSpec, name,
                (bundleType == kCFBundleType2) ? "Contents/" : "");
		
		DBG("Loading Bundle PList %s\n",gFileSpec);
        
        length = LoadFile(gFileSpec);
        if (length == -1) break;
		
        length = length + 1;
        buffer = malloc(length);
        if (buffer == 0) break;
		
        strlcpy(buffer, (char *)kLoadAddr, length);
		
        // Parse the plist.
		
        ret = ParseXML(buffer, &module);
        if (ret != 0 ) { printf("Unable to read plist of %s",name); break; }
		
		if (!module) {ret = -1;break;} // Should never happen but it will make the compiler happy
        
        // Allocate memory for the driver path and the plist.
        
        module->executablePath = tmpExecutablePath;
        module->bundlePath = tmpBundlePath;
        module->bundlePathLength = bundlePathLength;
        module->plistAddr = malloc(length);
		
        if ((module->executablePath == 0) || (module->bundlePath == 0) || (module->plistAddr == 0))
        {
            if ( module->plistAddr ) free(module->plistAddr);
			ret = -1;
            break;
        }       
		
        // Add the plist to the module.
		
        strlcpy(module->plistAddr, (char *)kLoadAddr, length);
        module->plistLength = length;
		
        // Add the module to the module list.
        
        add_bundle(module, name);  
        
        
        ret = 0;
    }
    while (0);
    
    if ( buffer )        free( buffer );
    if ( module )        free( module );
    
    if (ret != 0) {
        if ( tmpExecutablePath ) free( tmpExecutablePath );
        if ( tmpBundlePath ) free( tmpBundlePath );
    }	
    return ret;
}

#define WillLoadBundles                                                                             \
module = gModuleHead;                                                                               \
while (module != NULL)                                                                              \
{                                                                                                   \
if (module->willLoad == willLoad)                                                               \
{                                                                                               \
prop = XMLGetProperty(module->dict, kPropCFBundleExecutable);                               \
\
if (prop != 0)                                                                              \
{                                                                                           \
fileName = prop->string;                                                                \
snprintf(gFileSpec, DEFAULT_BUNDLE_SPEC_SIZE,"%s%s", module->executablePath, fileName);                           \
\
module_start = (void*)load_module((char*)fileName,gFileSpec);                           \
if(!module_start || (*module_start == (void*)0xFFFFFFFF))                               \
{                                                                                       \
if (module->willLoad > BundlePrioritySystemLib)                                                           \
{                                                                                   \
module->willLoad = BundlePriorityNull ;                                                          \
printf("Unable to start %s\n", gFileSpec);                                      \
}                                                                                   \
} else module_start();                                                                                       \
if (module->willLoad == BundlePrioritySystem)                                                              \
{                                                                                       \
lookup_symbol = (void*)lookup_all_symbols(SYMBOLS_BUNDLE,SYMBOL_LOOKUP_SYMBOL);     \
if((UInt32)lookup_symbol != 0xFFFFFFFF)                                             \
{                                                                                   \
msglog("%s successfully Loaded.\n", gFileSpec);                                 \
} else return -1;                                                                   \
}                                                                                       \
}                                                                                           \
}                                                                                               \
module = module->nextModule;                                                                    \
}       

//==========================================================================
// LoadMatchedBundles

long LoadMatchedBundles( void )
{
    TagPtr        prop;
    ModulePtr     module;
    char          *fileName;
    void (*module_start)(void);
    long willLoad;
    
	DBG("LoadMatchedBundles\n");
    
	int priority_end = MIN(gLowestLoadPriority+1, BundlePriorityEnd);
    
    for (willLoad = BundlePrioritySystem; willLoad < priority_end ; willLoad++)
    {
        
        WillLoadBundles ;
        
    }
    
    return 0;
}

//==========================================================================
// MatchBundlesLibraries

long MatchBundlesLibraries( void )
{
    
    TagPtr     prop, prop2;
    ModulePtr  module, module2,dummy_module;
	
    // Check for active modules with the same Bundle IDs or same principal class, only one must remain  (except for type 3 aka system libs)
    {
        module = gModuleHead;
        
        while (module != 0)
        {
            if (!(module->willLoad > BundlePriorityInit)) // if the module load priority is not higher than initialized, continue
            {
                module = module->nextModule;
                continue;
            }
            
            prop = XMLGetProperty(module->dict, kPropNSPrincipalClass);
            prop2 = XMLGetProperty(module->dict, kPropCFBundleIdentifier); 
            
            if (prop != 0 && prop2 != 0)
            {                    
                module2 = gModuleHead;
                
                TagPtr prop3,prop4;
                
                while (module2 != 0)
                {
                    prop3 = XMLGetProperty(module2->dict, kPropNSPrincipalClass);
                    prop4 = XMLGetProperty(module2->dict, kPropCFBundleIdentifier);              
                    
                    if ((prop3 != 0) && (prop4 != 0) && (module != module2))
                    {
                        
                        if ((module2->willLoad == BundlePrioritySystemLib) && ((strcmp(prop2->string, prop4->string)!= 0) /*&& (!strcmp(prop->string, prop3->string))*/)) {
                            continue;
                        }
                        
                        if ((!strcmp(prop2->string, prop4->string)) || (!strcmp(prop->string, prop3->string))) {
                            if (module2->willLoad > BundlePriorityNull) module2->willLoad = BundlePriorityNull;
                        }
                        
                    }
                    module2 = module2->nextModule;
                }  
                
            }
            
            module = module->nextModule;
        }
    }
    
    // Check for dependencies (it works in most cases, still a little buggy but should be sufficient for what we have to do, 
	// clearly the Achilles' heel of this implementation, please use dependencies with caution !!!)
    dummy_module = gModuleHead;
    while (dummy_module != 0)
    {
        module = gModuleHead;
        
        while (module != 0)
        {
            if (module->willLoad > BundlePrioritySystemLib)
            {                     
                prop = XMLGetProperty(module->dict, kPropOSBundleLibraries);
                if (prop != 0)
                {
                    prop = prop->tag;
                    while (prop != 0)
                    {
                        module2 = gModuleHead;
                        while (module2 != 0)
                        {
                            prop2 = XMLGetProperty(module2->dict, kPropCFBundleIdentifier);
                            if ((prop2 != 0) && (!strncmp(prop->string, prop2->string, strlen( prop->string))))
                            {
                                // found a parent
                                
                                if (module2->willLoad > BundlePriorityInit)
                                {                                    
                                    // parent is active
                                    if (module->willLoad == BundlePriorityNull) {
                                        module->willLoad = BundlePriorityNormalPriority;
                                    }                                        
                                    
                                    // Check if the version of the parent >= version of the child
                                    if (strtol(XMLCastString ( XMLGetProperty(module2->dict,"CFBundleShortVersionString") ), NULL, 10) >= strtol(XMLCastString( prop->tag ), NULL, 10)) {
                                        
                                        if ((module2->willLoad >= module->willLoad) && (module->willLoad > BundlePrioritySystemLib))
											module->willLoad = MIN(MAX(module->willLoad, module2->willLoad+1), BundlePriorityLowestPriority); // child must be loaded after the parent, this allow the to find symbols of the parent while we bind the child.
                                        
                                    } else {
                                        module->willLoad = BundlePriorityNull;
                                        goto nextmodule;
                                    }
                                    break;
                                }
                                
                            }
							if (module->willLoad != BundlePriorityNull) module->willLoad = BundlePriorityNull;
                            module2 = module2->nextModule;
                        }
                        if (module->willLoad == BundlePriorityNull) goto nextmodule;
                        prop = prop->tagNext;
                    }
                }
            }
        nextmodule:
            module = module->nextModule;
        }
        
		dummy_module = dummy_module->nextModule;
    }	
	
    // Get the lowest load priority
    {
        gLowestLoadPriority = BundlePriorityNormalPriority;
        module = gModuleHead;
        
        while (module != 0)
        {
            if (module->willLoad > BundlePriorityInit)
            {                
                gLowestLoadPriority = MIN(MAX(module->willLoad,gLowestLoadPriority), BundlePriorityLowestPriority);                 
            }
            module = module->nextModule;
        }
    }
    
    return 0;
}


//==========================================================================
// FindBundle

static ModulePtr
FindBundle( char * bundle_id )
{
    ModulePtr module;
    TagPtr    prop;
	DBG("FindBundle %s\n",bundle_id);
    
    module = gModuleHead;
    
    while (module != 0)
    {
        prop = XMLGetProperty(module->dict, kPropCFBundleIdentifier);
        if ((prop != 0) && !strcmp(bundle_id, prop->string)) break;
        module = module->nextModule;
    }
    
    return module;
}

//==========================================================================
// GetBundleDict

void *
GetBundleDict( char * bundle_id )
{
    ModulePtr module;
	DBG("GetBundleDict %s\n",bundle_id);
    
    module = FindBundle( bundle_id );
    
    if (module != 0)
    {
        return (void *)module->dict;
    }
    
    return 0;
}

//==========================================================================
// GetBundlePersonality

void *
GetBundlePersonality( char * bundle_id )
{
    ModulePtr module;
	DBG("GetBundlePersonalities %s\n",bundle_id);
    
    module = FindBundle( bundle_id );
    
    if (module != 0)
    {
        return (void *)module->personalities;
    }
    
    return 0;
}

//==========================================================================
// GetBundlePath

char *
GetBundlePath( char * bundle_id )
{
    ModulePtr module;
	DBG("GetBundlePath %s\n",bundle_id);
    
    module = FindBundle( bundle_id );
    
    if (module != 0)
    {
        return module->bundlePath;
    }
    
    return 0;
}

//==========================================================================
// ParseXML

static long
ParseXML( char * buffer, ModulePtr * module )
{
	long       length, pos;
	TagPtr     moduleDict, prop;
	ModulePtr  tmpModule;
	
    pos = 0;
	DBG("ParseXML\n");
    
	if (!module) {
        return -1;
    }
	
    while (1)
    {
        length = XMLParseNextTag(buffer + pos, &moduleDict);
        if (length == -1) break;
		
        pos += length;
		
        if (moduleDict == 0) continue;
        if (moduleDict->type == kTagTypeDict) break;
		
        XMLFreeTag(moduleDict);
    }
	
    if (length == -1) 
    {
        return -1;
    }
	
    
    tmpModule = malloc(sizeof(Module));
    if (tmpModule == 0)
    {
        XMLFreeTag(moduleDict);
        return -1;
    }
    tmpModule->dict = moduleDict;
    
    do {
        prop = XMLGetProperty(moduleDict, kPropOSBundleEnabled);
        if ((prop != 0) && prop->string)
        {
            if ( (strlen(prop->string) >= 1) && (prop->string[0] == 'N' || prop->string[0] == 'n') )
            {
                tmpModule->willLoad = 0;
                break;
            }
        }
        prop = XMLGetProperty(moduleDict, kPropNSPrincipalClass);
        if ((prop != 0) && prop->string)
        {
            if (!strncmp(prop->string,SYSLIB_CLASS, sizeof(SYSLIB_CLASS))) 
            {
                tmpModule->willLoad = BundlePrioritySystemLib;
                break;
            }
            if (!strncmp(prop->string,SYS_CLASS, sizeof(SYS_CLASS))) 
            {
                tmpModule->willLoad = BundlePrioritySystem;
                break;
            }
        }        
        
        prop = XMLGetProperty(moduleDict, kPropOSBundlePriority);
        if ((prop != 0) && prop->string)
        {
            int tmpwillLoad;
            if ((tmpwillLoad = strtoul(prop->string, NULL, 10)) > BundlePrioritySystemLib )
            {            
                tmpModule->willLoad = MIN(tmpwillLoad, BundlePriorityLowestPriority);
                break;
                
            }
            
        }
		
        tmpModule->willLoad = BundlePriorityNormalPriority;
		
#if 0
		prop = XMLGetProperty(moduleDict, kPropCFBundleLocalizations);
        if ((prop != 0) && prop->string)
        {
			char * path = newStringWithFormat("%s%s",module->bundlePath,prop->string);
			if (path == NULL) {
				break;
			}
			
            if (loadConfigFile(path, module->LanguageConfig) != 0)
			{
				free(path);
				path = NULL;
				
				const char * default_local;
				if ((default_local = getStringForKey(kPropCFBundleLocalizations, DEFAULT_BOOT_CONFIG))) 
				{
					path = newStringWithFormat("%s%s",module->bundlePath,default_local);
					if (path == NULL) {
						break;
					}
					loadConfigFile(path, module->LanguageConfig);
				}

			}
			
			if (path) {
				free(path);
			}            
        }
#endif
        
    } while (0);
    
    // Get the personalities.
    tmpModule->personalities = XMLGetProperty(moduleDict, kPropIOKitPersonalities);
    
    *module = tmpModule;
	
	
	
    return 0;
}
