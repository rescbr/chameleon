/*
 * Copyright 2010 Evan Lojewski. All rights reserved.
 *
 */
#ifndef CONFIG_MACHO_DEBUG
#define CONFIG_MACHO_DEBUG 0
#endif


#include "boot.h"
#include "modules.h"

extern void start_built_in_modules();

#if CONFIG_MACHO_DEBUG
#define DBG(x...)	printf(x);
#define DBGPAUSE()	getchar()
#else
#define DBG(x...)
#define DBGPAUSE()
#endif

// NOTE: Global so that modules can link with this
UInt64 textAddress = 0;
UInt64 textSection = 0;

/********************************************************************************/
/*	Macho Parser																*/
/********************************************************************************/

/*
 * Parse through a macho module. The module will be rebased and binded
 * as specified in the macho header. If the module is sucessfuly laoded
 * the module iinit address will be returned.
 * NOTE; all dependecies will be loaded before this module is started
 * NOTE: If the module is unable to load ot completeion, the modules
 * symbols will still be available.
 */
void* parse_mach(void* binary, 
                 int(*dylib_loader)(char*), 
                 long long(*symbol_handler)(char*, long long, char),
                 void (*section_handler)(char* section, char* segment, long long offset, long long address)
)
{	
	char is64 = false;
	void (*module_start)(void) = NULL;
	
	// Module info
	/*char* moduleName = NULL;
	 UInt32 moduleVersion = 0;
	 UInt32 moduleCompat = 0;
	 */
	// TODO convert all of the structs to a union
	struct load_command *loadCommand = NULL;
	struct dylib_command* dylibCommand = NULL;
	struct dyld_info_command* dyldInfoCommand = NULL;
	
	struct symtab_command* symtabCommand = NULL;
	struct segment_command *segCommand = NULL;
	struct segment_command_64 *segCommand64 = NULL;
	
	//struct dysymtab_command* dysymtabCommand = NULL;
	UInt32 binaryIndex = 0;
	UInt16 cmd = 0;
	
	textSection = 0;
	textAddress = 0;	// reinitialize text location in case it doesn't exist;
	
	// Parse through the load commands
	if(((struct mach_header*)binary)->magic == MH_MAGIC)
	{
		is64 = false;
		binaryIndex += sizeof(struct mach_header);
	}
	else if(((struct mach_header_64*)binary)->magic == MH_MAGIC_64)
	{
		// NOTE: modules cannot be 64bit...
		is64 = true;
		binaryIndex += sizeof(struct mach_header_64);
	}
	else
	{
		verbose("Invalid mach magic 0x%X\n", ((struct mach_header*)binary)->magic);
		//getchar();
		return NULL;
	}
	
	
	
	/*if(((struct mach_header*)binary)->filetype != MH_DYLIB)
	 {
	 printf("Module is not a dylib. Unable to load.\n");
	 getchar();
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
                    
                    UInt32 sectionIndex;
                    
                    sectionIndex = sizeof(struct segment_command);
                    
                    struct section *sect;
                    
                    while(sectionIndex < segCommand->cmdsize)
                    {
                        sect = binary + binaryIndex + sectionIndex;
                        
                        sectionIndex += sizeof(struct section);
                        
                        if(section_handler) section_handler(sect->sectname, segCommand->segname, sect->offset, sect->addr);
                        
                        
                        
                        if((strcmp("__TEXT", segCommand->segname) == 0) && (strcmp("__text", sect->sectname) == 0))
                        {
                            // __TEXT,__text found, save the offset and address for when looking for the calls.
                            textSection = sect->offset;
                            textAddress = sect->addr;
                        }					
                    }
                }
				break;
			case LC_SEGMENT_64:	// 64bit macho's
                {
                    segCommand64 = binary + binaryIndex;				
                    UInt32 sectionIndex;
                    
                    sectionIndex = sizeof(struct segment_command_64);
                    
                    struct section_64 *sect;
                    
                    while(sectionIndex < segCommand64->cmdsize)
                    {
                        sect = binary + binaryIndex + sectionIndex;
                        
                        sectionIndex += sizeof(struct section_64);
                        
                        if(section_handler) section_handler(sect->sectname, segCommand->segname, sect->offset, sect->addr);
                        
                        
                        if((strcmp("__TEXT", segCommand->segname) == 0) && (strcmp("__text", sect->sectname) == 0))
                        {
                            // __TEXT,__text found, save the offset and address for when looking for the calls.
                            textSection = sect->offset;
                            textAddress = sect->addr;
                        }					
                    }	
				}			
				break;
				
				
			case LC_LOAD_DYLIB:
			case LC_LOAD_WEAK_DYLIB ^ LC_REQ_DYLD:
				dylibCommand  = binary + binaryIndex;
				char* module  = binary + binaryIndex + ((UInt32)*((UInt32*)&dylibCommand->dylib.name));
				// Possible enhancments: verify version
				// =	dylibCommand->dylib.current_version;
				// =	dylibCommand->dylib.compatibility_version;
				if(dylib_loader)
				{
					char* name = malloc(strlen(module) + strlen(".dylib") + 1);
					sprintf(name, "%s.dylib", module);
					
					if (!dylib_loader(name))
					{
						// NOTE: any symbols exported by dep will be replace with the void function
						free(name);
					}
				}
				
				break;
				
			case LC_ID_DYLIB:
				dylibCommand = binary + binaryIndex;
				/*moduleName =	binary + binaryIndex + ((UInt32)*((UInt32*)&dylibCommand->dylib.name));
				 moduleVersion =	dylibCommand->dylib.current_version;
				 moduleCompat =	dylibCommand->dylib.compatibility_version;
				 */
				break;
				
			case LC_DYLD_INFO:
			//case LC_DYLD_INFO_ONLY:	// compressed info, 10.6+ macho files, already handeled
				// Bind and rebase info is stored here
				dyldInfoCommand = binary + binaryIndex;
				break;
				
			case LC_DYSYMTAB:
			case LC_UUID:
			case LC_UNIXTHREAD:
				break;
				
			default:
				DBG("Unhandled loadcommand 0x%X\n", loadCommand->cmd & 0x7FFFFFFF);
				break;
				
		}
		
		binaryIndex += cmdSize;
	}

	// bind_macho uses the symbols, if the textAdd does not exist (Symbols.dylib, no code), addresses are static and not relative
	module_start = (void*)handle_symtable((UInt32)binary, symtabCommand, symbol_handler, is64);
	
	if(dyldInfoCommand)
	{
		// Rebase the module before binding it.
		if(dyldInfoCommand->rebase_off)		rebase_macho(binary, (char*)dyldInfoCommand->rebase_off,	dyldInfoCommand->rebase_size);
		// Bind all symbols. 
		if(dyldInfoCommand->bind_off)		bind_macho(binary,   (UInt8*)dyldInfoCommand->bind_off,		dyldInfoCommand->bind_size);
		if(dyldInfoCommand->weak_bind_off)	bind_macho(binary,   (UInt8*)dyldInfoCommand->weak_bind_off,	dyldInfoCommand->weak_bind_size);
		if(dyldInfoCommand->lazy_bind_off)	bind_macho(binary,   (UInt8*)dyldInfoCommand->lazy_bind_off,	dyldInfoCommand->lazy_bind_size);
	}
	
	return module_start;
	
}

/*
 * parse the symbol table
 * Lookup any undefined symbols
 */

void* handle_symtable(UInt32 base, struct symtab_command* symtabCommand, long long(*symbol_handler)(char*, long long, char), char is64)
{
	
	void* module_start	= (void*) -1;	
	UInt32 symbolIndex			= 0;
	char* symbolString			= base + (char*)symtabCommand->stroff;

	if(!is64)
	{
		struct nlist* symbolEntry = (void*)base + symtabCommand->symoff;
		while(symbolIndex < symtabCommand->nsyms)
		{
			// If the symbol is exported by this module
			if(symbolEntry->n_value &&
			   symbol_handler(symbolString + symbolEntry->n_un.n_strx, textAddress ? (long long)base + symbolEntry->n_value : symbolEntry->n_value, is64) != 0xFFFFFFFF)
			{
				
				// Module start located. Start is an alias so don't register it
				module_start = (void*)(textAddress ? base + symbolEntry->n_value : symbolEntry->n_value);
			}
			
			symbolEntry++;
			symbolIndex++;	// TODO remove
		}
	}
	else
	{
		/*
		struct nlist_64* symbolEntry = (void*)base + symtabCommand->symoff;
		// NOTE First entry is *not* correct, but we can ignore it (i'm getting radar:// right now, verify later)	
		while(symbolIndex < symtabCommand->nsyms)
		{
			
			
			// If the symbol is exported by this module
			if(symbolEntry->n_value &&
			   symbol_handler(symbolString + symbolEntry->n_un.n_strx, textAddress ? (long long)base + symbolEntry->n_value : symbolEntry->n_value, is64) != 0xFFFFFFFF)
			{
				
				// Module start located. Start is an alias so don't register it
				module_start = textAddress ? base + symbolEntry->n_value : symbolEntry->n_value;
			}
			
			symbolEntry++;
			symbolIndex++;	// TODO remove
		}
		 */
	}
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
	unsigned int i = 0;
	
	while(i < size)
	{
		immediate = rebase_stream[i] & REBASE_IMMEDIATE_MASK;
		opcode = rebase_stream[i] & REBASE_OPCODE_MASK;
		
		
		switch(opcode)
		{
			case REBASE_OPCODE_DONE:
				// Rebase complete, reset vars
				immediate = 0;
				opcode = 0;
				type = 0;
				segmentAddress = 0;
			default:
				break;
				
				
			case REBASE_OPCODE_SET_TYPE_IMM:
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
				
				index = 0;
				for (index = 0; index < tmp; ++index) {
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
				
				index = 0;
				for (index = 0; index < tmp; ++index) {
					
					rebase_location(base + segmentAddress, (char*)base, type);
					
					segmentAddress += tmp2 + sizeof(void*);
				}
				break;
		}
		i++;
	}
}

inline void rebase_location(UInt32* location, char* base, int type)
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


UInt32 read_uleb(UInt8* bind_stream, unsigned int* i)
{
    // Read in offset
    UInt32 tmp  = 0;
    UInt8 bits = 0;
    do
    {
        if(bits < sizeof(UInt32)*8)   // hack
        {
            tmp |= (bind_stream[++(*i)] & 0x7f) << bits;
            bits += 7;
        }
        else
        {
            ++(*i);
        }
    }
    while(bind_stream[*i] & 0x80);
    return tmp;
}


// Based on code from dylibinfo.cpp and ImageLoaderMachOCompressed.cpp
// NOTE: this uses 32bit values, and not 64bit values. 
// There is a possibility that this could cause issues,
// however the modules are 32 bits, so it shouldn't matter too much
void bind_macho(void* base, UInt8* bind_stream, UInt32 size)
{	
	bind_stream += (UInt32)base;
	
	UInt8 immediate = 0;
	UInt8 opcode = 0;
	UInt8 type = BIND_TYPE_POINTER;
	
	UInt32 segmentAddress = 0;
	
	UInt32 address = 0;
	
	SInt32 addend = 0;
	SInt32 libraryOrdinal = 0;
	
	const char* symbolName = NULL;
	UInt8 symboFlags = 0;
	UInt32 symbolAddr = 0xFFFFFFFF;
	
	// Temperary variables
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
				// reset vars
				type = BIND_TYPE_POINTER;
				segmentAddress = 0;
				address = 0;
				addend = 0;
				libraryOrdinal = 0;
				symbolAddr = 0xFFFFFFFF;
			default:
				break;
				
			case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
				libraryOrdinal = immediate;
				break;
				
			case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
				libraryOrdinal = read_uleb(bind_stream, &i);
				break;
				
			case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
				libraryOrdinal = immediate ? (SInt8)(BIND_OPCODE_MASK | immediate) : immediate;				
				break;
				
			case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
				symboFlags = immediate;
				symbolName = (char*)&bind_stream[++i];
				i += strlen((char*)&bind_stream[i]);

				symbolAddr = lookup_all_symbols(symbolName);
				break;
				
			case BIND_OPCODE_SET_TYPE_IMM:
				type = immediate;
				break;
				
			case BIND_OPCODE_SET_ADDEND_SLEB:
				addend = read_uleb(bind_stream, &i);				
				if(!(bind_stream[i-1] & 0x40)) addend *= -1;
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
								
				segmentAddress += read_uleb(bind_stream, &i);
				break;
				
			case BIND_OPCODE_ADD_ADDR_ULEB:
				segmentAddress += read_uleb(bind_stream, &i);
				break;
				
			case BIND_OPCODE_DO_BIND:
				if(symbolAddr != 0xFFFFFFFF)
				{
					address = segmentAddress + (UInt32)base;
						
					bind_location((UInt32*)address, (char*)symbolAddr, addend, type);
				}
				else
				{
					printf("Unable to bind symbol %s\n", symbolName);
					getchar();
				}
				
				segmentAddress += sizeof(void*);
				break;
				
			case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
				// Read in offset
				tmp  = read_uleb(bind_stream, &i);

				if(symbolAddr != 0xFFFFFFFF)
				{
					address = segmentAddress + (UInt32)base;
					
					bind_location((UInt32*)address, (char*)symbolAddr, addend, type);
				}
				else
				{
					printf("Unable to bind symbol %s\n", symbolName);
					getchar();
				}

				segmentAddress += tmp + sizeof(void*);
				
				
				break;
				
			case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
				if(symbolAddr != 0xFFFFFFFF)
				{
					address = segmentAddress + (UInt32)base;
					
					bind_location((UInt32*)address, (char*)symbolAddr, addend, type);
				}
				else
				{
					printf("Unable to bind symbol %s\n", symbolName);
					getchar();
				}
				segmentAddress += (immediate * sizeof(void*)) + sizeof(void*);
				
				
				break;
				
			case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
				tmp  = read_uleb(bind_stream, &i);				
				
				tmp2  = read_uleb(bind_stream, &i);				
				
				if(symbolAddr != 0xFFFFFFFF)
				{
					for(index = 0; index < tmp; index++)
					{
						
						address = segmentAddress + (UInt32)base;
						bind_location((UInt32*)address, (char*)symbolAddr, addend, type);
						segmentAddress += tmp2 + sizeof(void*);
					}
				}
				else
				{
					printf("Unable to bind symbol %s\n", symbolName);
					getchar();
				}
				break;
		}
		i++;
	}
}
                 

inline void bind_location(UInt32* location, char* value, UInt32 addend, int type)
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
	//DBG("Binding 0x%X to 0x%X (was 0x%X)\n", location, newValue, *location);
	*location = (UInt32)newValue;
}

/*
* Locate the symbol for an already loaded function and modify the beginning of
* the function to jump directly to the new one
* example: replace_function("_HelloWorld_start", &replacement_start);
*/
int replace_function(const char* symbol, void* newAddress)
{
	UInt32* jumpPointer = malloc(sizeof(UInt32*));	 
	UInt32 addr = lookup_all_symbols(symbol);
	
	char* binary = (char*)addr;
	if(addr != 0xFFFFFFFF)
	{
		//DBG("Replacing %s to point to 0x%x\n", symbol, newAddress);
		*binary++ = 0xFF;	// Jump
		*binary++ = 0x25;	// Long Jump
		*((UInt32*)binary) = (UInt32)jumpPointer;
		
		*jumpPointer = (UInt32)newAddress;
		return 1;
	}
	return 0;
}

/********************************************************************************/
/*	dyld / Linker Interface														*/
/********************************************************************************/

void dyld_stub_binder()
{
	printf("ERROR: dyld_stub_binder was called, should have been take care of by the linker.\n");
	getchar();
}