/*
 * Copyright 2009 Evan Lojewski. All rights reserved.
 *
 */

#include "boot.h"
#include "bootstruct.h"
#include "multiboot.h"
#include "modules.h"


unsigned int (*lookup_symbol)(const char*) = NULL;


void rebase_macho(void* base, char* rebase_stream, UInt32 size);
void bind_macho(void* base, char* bind_stream, UInt32 size);

int load_module(const char* module)
{
	char modString[128];
	int fh = -1;
	sprintf(modString, "/Extra/modules/%s.dylib", module);
	fh = open(modString, 0);
	if(fh < 0)
	{
		printf("Unable to locate module %s\n", modString);
		getc();
		return ERROR;
	}
	
	unsigned int moduleSize = file_size(fh);
	char* module_base = (char*) malloc(moduleSize);
	if (read(fh, module_base, moduleSize) == moduleSize)
	{
		void (*module_start)(void) = NULL;

		printf("Module %s read in.\n", modString);

		// Module loaded into memory, parse it
		module_start = parse_mach(module_base);

		if(module_start)
		{
			(*module_start)();	// Start the module
			printf("Module started\n");
		}
		else {
			printf("Unabel to locate module start\n");
		}

		
		getc();
		
		// TODO: Add module to loaded list if loaded successfuly
		
	}
	else
	{
		printf("Unable to read in module %s.dylib\n.", module);
		getc();
	}
	return 1;
}


void* parse_mach(void* binary)
{
	int (*module_init)(void) = NULL;
	void (*module_start)(void) = NULL;


	char* moduleName = NULL;
	UInt32 moduleVersion = 0;
	UInt32 moduleCompat = 0;
	
	char* symbolStub = NULL;
	char* nonlazy = NULL;
	char* nonlazy_variables = NULL;

	char* symbolTable = NULL;
	// TODO convert all of the structs a union
	struct load_command *loadCommand;
	struct dylib_command* dylibCommand;
	struct dyld_info_command* dyldInfoCommand;
	struct symtab_command* symtabCommand;
	struct segment_command* segmentCommand;
	struct dysymtab_command* dysymtabCommand;
	struct section* section;
	UInt32 binaryIndex = sizeof(struct mach_header);
	UInt16 cmd = 0;

	// Parse hthrough th eload commands
	if(((struct mach_header*)binary)->magic != MH_MAGIC)
	{
		printf("Module is not 32bit\n");
		return NULL;	// 32bit only
	}
	
	if(((struct mach_header*)binary)->filetype != MH_DYLIB)
	{
		printf("Module is not a dylib. Unable to load.\n");
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

				UInt32 symbolIndex = 0;
				char* symbolString = (UInt32)binary + (char*)symtabCommand->stroff;
				symbolTable = binary + symtabCommand->symoff;
				
				int externalIndex = 0;
				while(symbolIndex < symtabCommand->nsyms)
				{
					
					struct nlist* symbolEntry = binary + symtabCommand->symoff + (symbolIndex * sizeof(struct nlist));
					
					// If the symbol is exported by this module
					if(symbolEntry->n_value)
					{
						if(strcmp(symbolString + symbolEntry->n_un.n_strx, "_start") == 0)
						{
							// Module init located. Don't register it, only called once
							module_init = binary + symbolEntry->n_value;
						}
						else if(strcmp(symbolString + symbolEntry->n_un.n_strx, "start") == 0)
						{
							// Module start located. Start is an alias so don't register it
							module_start = binary + symbolEntry->n_value;
						}
						else
						{
							add_symbol(symbolString + symbolEntry->n_un.n_strx, binary + symbolEntry->n_value); 
						}
					}
					// External symbol
					else if(symbolEntry->n_type & 0x01 && symbolStub)
					{
						printf("Located external symbol %s", symbolString + symbolEntry->n_un.n_strx);
						printf(" stub at 0x%X, (0x%X)\n", symbolStub + STUB_ENTRY_SIZE * externalIndex - (UInt32)binary, symbolStub + STUB_ENTRY_SIZE * externalIndex);
						getc();
						// Patch stub
						if(lookup_symbol == NULL)
						{
							printf("Symbol.dylib not loaded. Module loader not setup.\n");
							return NULL;
						}
						else
						{
							void* symbolAddress = (void*)lookup_symbol(symbolString + symbolEntry->n_un.n_strx);
							
							if((0xFFFFFFFF == (UInt32)symbolAddress) && 
							   strcmp(symbolString + symbolEntry->n_un.n_strx, SYMBOL_DYLD_STUB_BINDER) != 0)
							{
								printf("Unable to locate symbol %s\n", symbolString + symbolEntry->n_un.n_strx);
								
							}
							else 
							{
								char* patchLocation = symbolStub + STUB_ENTRY_SIZE * externalIndex;
								//patch with far jump ;
								/*
								printf("0x%X 0x%X 0x%X 0x%X 0x%X 0x%X\n", 
									   patchLocation[0],
									   patchLocation[1],
									   patchLocation[2],
									   patchLocation[3],
									   patchLocation[4],
									   patchLocation[5]);
								*/
								
								// Point the symbol stub to the nonlazy pointers
								patchLocation[0] = 0xFF;	// Should already be this
								patchLocation[1] = 0x25;	// Should already be this
								patchLocation[2] = ((UInt32)(nonlazy + externalIndex * 4) & 0x000000FF) >> 0;
								patchLocation[3] = ((UInt32)(nonlazy + externalIndex * 4) & 0x0000FF00) >> 8;
								patchLocation[4] = ((UInt32)(nonlazy + externalIndex * 4) & 0x00FF0000) >> 16;
								patchLocation[5] = ((UInt32)(nonlazy + externalIndex * 4) & 0xFF000000) >> 24;
								
								// Set the nonlazy pointer to the correct address
								patchLocation = (nonlazy + externalIndex*4);
								patchLocation[0] =((UInt32)symbolAddress & 0x000000FF) >> 0;
								patchLocation[1] =((UInt32)symbolAddress & 0x0000FF00) >> 8;
								patchLocation[2] =((UInt32)symbolAddress & 0x00FF0000) >> 16;
								patchLocation[3] =((UInt32)symbolAddress & 0xFF000000) >> 24;
								
								
								externalIndex++;

							}

														
						}
											 
					}

					symbolEntry+= sizeof(struct nlist);
					symbolIndex++;	// TODO remove
				}
				break;

			case LC_SEGMENT:
				segmentCommand = binary + binaryIndex;

				UInt32 sections =  segmentCommand->nsects;
				section = binary + binaryIndex + sizeof(struct segment_command);
				while(sections)
				{
					// Look for the __symbol_stub section
					if(strcmp(section->sectname, "__nl_symbol_ptr") == 0)
					{
						/*printf("\tSection non lazy pointers at 0x%X, %d symbols\n",
							   section->offset, 
							   section->size / 4);
						*/
						switch(section->flags)
						{
							case S_NON_LAZY_SYMBOL_POINTERS:
								nonlazy_variables = binary + section->offset;
									break;
								
							case S_LAZY_SYMBOL_POINTERS:
								nonlazy = binary + section->offset;
								// Fucntions
								break;
								
							default:
								printf("Unhandled __nl_symbol_ptr section\n");
								getc();
								break;
						}
						//getc();
					}
					else if(strcmp(section->sectname, "__symbol_stub") == 0)
					{
						/*printf("\tSection __symbol_stub at 0x%X (0x%X), %d symbols\n",
							   section->offset, 
							   section->size / 6);*/
						symbolStub = binary + section->offset;
						//getc();
					}

					
					sections--;
					section++;
				}
						  
				
				break;
				
			case LC_DYSYMTAB:
				dysymtabCommand = binary + binaryIndex;
				//printf("Unhandled loadcommand LC_DYSYMTAB\n");
				break;
				
			case LC_LOAD_DYLIB:
				//printf("Unhandled loadcommand LC_LOAD_DYLIB\n");
				break;
				
			case LC_ID_DYLIB:
				dylibCommand = binary + binaryIndex;
				moduleName =	binary + binaryIndex + ((UInt32)*((UInt32*)&dylibCommand->dylib.name));
				moduleVersion =	dylibCommand->dylib.current_version;
				moduleCompat =	dylibCommand->dylib.compatibility_version;
				break;

			case LC_DYLD_INFO:
				dyldInfoCommand = binary + binaryIndex;
				/*
				 printf("LC_DYLD_INFO:\n"
					   "\tRebase Offset: 0x%X\n"
					   "\tBind Offset: 0x%X\n"
					   "\tWeak Bind Offset: 0x%X\n"
					   "\tLazy Bind Offset: 0x%X\n"
					   "\tExport Offset: 0x%X\n",
					   dyldInfoCommand->rebase_off,
					   dyldInfoCommand->bind_off,
					   dyldInfoCommand->weak_bind_off,
					   dyldInfoCommand->lazy_bind_off,
					   dyldInfoCommand->export_off);
				*/
				if(dyldInfoCommand->bind_off)
				{
					bind_macho(binary, (char*)dyldInfoCommand->bind_off, dyldInfoCommand->bind_size);
				}
				
				/*
				 // This is done later on using the __symbol_stub instead
				if(dyldInfoCommand->lazy_bind_off)
				{
					bind_macho(binary, (char*)dyldInfoCommand->lazy_bind_off, dyldInfoCommand->lazy_bind_size);
					
				}*/
				
				if(dyldInfoCommand->rebase_off)
				{
					rebase_macho(binary, (char*)dyldInfoCommand->rebase_off, dyldInfoCommand->rebase_size);
				}
				
				
				break;

			case LC_UUID:
				// We don't care about the UUID at the moment
				break;
				
			default:
				printf("Unhandled loadcommand 0x%X\n", loadCommand->cmd & 0x7FFFFFFF);
				break;
		
		}

		binaryIndex += cmdSize;
	}
	if(!moduleName) return NULL;
	
	
	// TODO: Initialize the module.
	printf("Calling _start\n");
	
	getc();
	if(module_init) 
	{
		jump_pointer(module_init);
	 }
	module_loaded(moduleName, moduleVersion, moduleCompat);
	getc();
	 //*/
	return module_start; // TODO populate w/ address of _start
	
}


// This is for debugging, remove the jump_pointer funciton later
int jump_pointer(int (*pointer)())
{
	printf("Jumping to function at 0x%X\n", pointer);

	getc();
	return (*pointer)();
}

// Based on code from dylibinfo.cpp and ImageLoaderMachOCompressed.cpp
void rebase_macho(void* base, char* rebase_stream, UInt32 size)
{
	rebase_stream += (UInt32)base;
	
	UInt8 immediate = 0;
	UInt8 opcode = 0;
	UInt8 type = 0;
	
	UInt32 segmentAddress = 0;
	UInt32 address = 0;
	
	
	
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

				struct segment_command* segCommand = NULL;
				
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
				
				//printf("Address = 0x%X\n", segmentAddress);
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
				
				address +=	tmp; 
				//printf("Address (add) = 0x%X\n", address);
				break;
				
			case REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
				address += immediate * sizeof(void*);
				//printf("Address (immscaled) = 0x%X\n", address);

				break;
				
			case REBASE_OPCODE_DO_REBASE_IMM_TIMES:
				//printf("Rebase %d time(s)\n", immediate);
				index = 0;
				for (index = 0; index < immediate; ++index) {
					//printf("\tRebasing 0x%X\n", segmentAddress);
					
					UInt32* addr = base + segmentAddress;
					addr[0] += (UInt32)base;
					
					
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
				
				//printf("Rebase %d time(s)\n", tmp);
				
				index = 0;
				for (index = 0; index < tmp; ++index) {
					//printf("\tRebasing 0x%X\n", segmentAddress);
					
					UInt32* addr = base + segmentAddress;
					addr[0] += (UInt32)base;
					
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
				
				
				//printf("Rebase and add 0x%X\n", tmp);
				//printf("\tRebasing 0x%X\n", segmentAddress);
				UInt32* addr = base + segmentAddress;
				addr[0] += (UInt32)base;
				
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
				
				//printf("Rebase 0x%X times, skiping 0x%X\n",tmp, tmp2);
				index = 0;
				for (index = 0; index < tmp; ++index) {
					//printf("\tRebasing 0x%X\n", segmentAddress);

					UInt32* addr = base + segmentAddress;
					addr[0] += (UInt32)base;
					
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
	
	SInt32 addend = 0;
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
				//printf("BIND_OPCODE_SET_DYLIB_ORDINAL_IMM: %d\n", libraryOrdinal);
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
				
				//printf("BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB: %d\n", libraryOrdinal);

				break;
				
			case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
				// NOTE: this is wrong, fortunately we don't use it
				libraryOrdinal = -immediate;
				//printf("BIND_OPCODE_SET_DYLIB_SPECIAL_IMM: %d\n", libraryOrdinal);

				break;
				
			case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
				symboFlags = immediate;
				symbolName = (char*)&bind_stream[++i];
				i += strlen((char*)&bind_stream[i]);
				//printf("BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM: %s, 0x%X\n", symbolName, symboFlags);

				symbolAddr = lookup_symbol(symbolName);

				break;
				
			case BIND_OPCODE_SET_TYPE_IMM:
				// Set bind type (pointer, absolute32, pcrel32)
				type = immediate;
				//printf("BIND_OPCODE_SET_TYPE_IMM: %d\n", type);

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
				
				//printf("BIND_OPCODE_SET_ADDEND_SLEB: %d\n", addend);
				break;
				
			case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
				segmentAddress = 0;

				// Locate address
				struct segment_command* segCommand = NULL;
				
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
				
				//printf("BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB: 0x%X\n", segmentAddress);
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
				//printf("BIND_OPCODE_ADD_ADDR_ULEB: 0x%X\n", segmentAddress);
				break;
				
			case BIND_OPCODE_DO_BIND:
				//printf("BIND_OPCODE_DO_BIND\n");
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
				//printf("BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB\n");
				
				
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
				//printf("BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED\n");
				
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
				
				
				//printf("BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB 0x%X 0x%X\n", tmp, tmp2);

				
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

/*
 * add_symbol
 * This function adds a symbol from a module to the list of known symbols 
 * TODO: actualy do something... 
 * possibly change to a pointer and add this to the Symbol module
 */
void add_symbol(const char* symbol, void* addr)
{
	printf("Adding symbol %s at 0x%X\n", symbol, addr);
	
	//hack
	if(strcmp(symbol, "_lookup_symbol") == 0)
	{
		printf("Initializing lookup symbol function\n");
		lookup_symbol = addr;
	}
}


/*
 * print out the information about the loaded module
 * In the future, add to a list of laoded modules
 * so that if another module depends on it we don't
 * try to reload the same module.
 */
void module_loaded(const char* name, UInt32 version, UInt32 compat)
{
	printf("\%s.dylib Version %d.%d.%d oaded\n"
		   "\tCompatibility Version: %d.%d.%d\n",
		   name,
		   (version >> 16) & 0xFFFF,
		   (version >> 8) & 0x00FF,
		   (version >> 0) & 0x00FF,
		   (compat >> 16) & 0xFFFF,
		   (compat >> 8) & 0x00FF,
		   (compat >> 0) & 0x00FF);		
	
}
