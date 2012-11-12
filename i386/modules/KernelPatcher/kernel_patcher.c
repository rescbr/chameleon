	/*
 * Copyright (c) 2009-2010 Evan Lojewski. All rights reserved.
 *
 */

#include "libsaio.h"
#include "kernel_patcher.h"
#include "platform.h"
#include "modules.h"

#ifndef DEBUG_KERNEL_PATCHER
#define DEBUG_KERNEL_PATCHER 0
#endif

#if DEBUG_KERNEL_PATCHER
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

long long symbol_handler(char* module, char* symbolName, long long addr, char is64);

patchRoutine_t* patches = NULL;
kernSymbols_t* kernelSymbols = NULL;
static void register_kernel_patch(void* patch, int arch, int cpus);
static void register_kernel_symbol(int kernelType, const char* name);
static kernSymbols_t* lookup_kernel_symbol(const char* name);
static int determineKernelArchitecture(void* kernelData);
static int locate_symbols(void* kernelData);
static unsigned int parse_mach_64(char *module, void* binary, long long(*symbol_handler)(char*, char*, long long, char));
static unsigned int handle_symtable_64(char *module, UInt32 base, struct symtab_command* symtabCommand, long long(*symbol_handler)(char*, char*, long long, char), char is64);

/*
 * Internal patches provided by this module.
 */
static void patch_cpuid_set_info_all(void* kernelData);
static void patch_cpuid_set_info_32(void* kernelData, UInt32 impersonateFamily, UInt8 impersonateModel);
static void patch_cpuid_set_info_64(void* kernelData, UInt32 impersonateFamily, UInt8 impersonateModel);

//static void patch_pmCPUExitHaltToOff(void* kernelData);
static void patch_lapic_init(void* kernelData);
static void patch_commpage_stuff_routine(void* kernelData);
static void patch_lapic_configure(void* kernelData);
//static void patch_lapic_interrupt(void* kernelData);

void patch_kernel(void* kernelData, void* arg2, void* arg3, void *arg4, void* arg5, void* arg6);
void kernel_patcher_ignore_cache(void* arg1, void* arg2, void* arg3, void *arg4, void* arg5, void* arg6);
void kernel_patcher_ignore_cache(void* arg1, void* arg2, void* arg3, void *arg4, void* arg5, void* arg6){}

void KernelPatcher_start(void);
void KernelPatcher_start(void)
{
	register_kernel_patch(patch_cpuid_set_info_all, KERNEL_ANY, CPUID_MODEL_UNKNOWN); 

	register_kernel_patch(patch_commpage_stuff_routine, KERNEL_ANY, CPUID_MODEL_ANY);
	
	register_kernel_patch(patch_lapic_init, KERNEL_ANY, CPUID_MODEL_ANY);

	// NOTE: following is currently 32bit only
	register_kernel_patch(patch_lapic_configure, KERNEL_32, CPUID_MODEL_ANY);

	register_kernel_symbol(KERNEL_ANY, "_panic");
	register_kernel_symbol(KERNEL_ANY, "_cpuid_set_info");
	register_kernel_symbol(KERNEL_ANY, "_pmCPUExitHaltToOff");
	register_kernel_symbol(KERNEL_ANY, "_lapic_init");
	register_kernel_symbol(KERNEL_ANY, "_commpage_stuff_routine");

	// lapic_configure symbols
	register_kernel_symbol(KERNEL_ANY, "_lapic_configure");
	register_kernel_symbol(KERNEL_ANY, "_lapic_start");
	register_kernel_symbol(KERNEL_ANY, "_lapic_interrupt_base");

	// lapic_interrup symbols
	//register_kernel_patch(patch_lapic_interrupt, KERNEL_ANY, CPUID_MODEL_ANY);
	//register_kernel_symbol(KERNEL_ANY, "_lapic_interrupt");

	// TODO: register needed symbols

	register_hook_callback("ExecKernel", &patch_kernel); 
	
	replace_system_function("_getKernelCachePath", &kernel_patcher_ignore_cache);
}

/*
 * Register a kernel patch
 */
static void register_kernel_patch(void* patch, int arch, int cpus)
{
	// TODO: only insert valid patches based on current cpuid and architecture
	// AKA, don't at 64bit patches if it's a 32bit only machine
	patchRoutine_t* entry;
	
	// Check to ensure that the patch is valid on this machine
	// If it is not, exit early form this function
	if(cpus != get_env(envModel))
	{
		if(cpus != CPUID_MODEL_ANY)
		{
			if(cpus == CPUID_MODEL_UNKNOWN)
			{
				switch(get_env(envModel))
				{
					case 13:
					case CPUID_MODEL_YONAH:
					case CPUID_MODEL_MEROM:
					case CPUID_MODEL_PENRYN:
					case CPUID_MODEL_NEHALEM:
					case CPUID_MODEL_FIELDS:
					case CPUID_MODEL_DALES:
					case CPUID_MODEL_NEHALEM_EX:
						// Known cpu's we don't want to add the patch
						return;
						break;

					default:
						// CPU not in supported list, so we are going to add
						// The patch will be applied
						break;
				}
			}
			else
			{
				// Invalid cpuid for current cpu. Ignoring patch
				return;
			}
		}
	}
		
	if(patches == NULL)
	{
		patches = entry = malloc(sizeof(patchRoutine_t));
	}
	else
	{
		entry = patches;
		while(entry->next)
		{
			entry = entry->next;
		}
		
		entry->next = malloc(sizeof(patchRoutine_t));
		entry = entry->next;
	}
	
	entry->next = NULL;
	entry->patchRoutine = patch;
	entry->validArchs = arch;
	entry->validCpu = cpus;
}

static void register_kernel_symbol(int kernelType, const char* name)
{
	if(kernelSymbols == NULL)
	{
		kernelSymbols = malloc(sizeof(kernSymbols_t));
		kernelSymbols->next = NULL;
		kernelSymbols->symbol = (char*)name;
		kernelSymbols->addr = 0;
	}
	else
	{
		kernSymbols_t *symbol = kernelSymbols;
		while(symbol->next != NULL)
		{
			symbol = symbol->next;
		}
		
		symbol->next = malloc(sizeof(kernSymbols_t));
		symbol = symbol->next;

		symbol->next = NULL;
		symbol->symbol = (char*)name;
		symbol->addr = 0;
	}
}

static kernSymbols_t* lookup_kernel_symbol(const char* name)
{
	kernSymbols_t *symbol = kernelSymbols;

	while(symbol && strcmp(symbol->symbol, name) !=0)
	{
		symbol = symbol->next;
	}
	
	if(!symbol)
	{
		return NULL;
	}
	else
	{
		return symbol;
	}
}

void patch_kernel(void* kernelData, void* arg2, void* arg3, void *arg4, void* arg5, void* arg6)
{
	patchRoutine_t* entry = patches;

	int arch = determineKernelArchitecture(kernelData);

	locate_symbols(kernelData);
	
	if(patches != NULL)
	{
		while(entry)
		{
			if(entry->validArchs == KERNEL_ANY || arch == entry->validArchs)
			{
				if(entry->patchRoutine) entry->patchRoutine(kernelData);
			}
			entry = entry->next;
		}
	}
}

static int determineKernelArchitecture(void* kernelData)
{	
	if(((struct mach_header*)kernelData)->magic == MH_MAGIC)
	{
		return KERNEL_32;
	}

	if(((struct mach_header*)kernelData)->magic == MH_MAGIC_64)
	{
		return KERNEL_64;
	}
	else
	{
		return KERNEL_ERR;
	}
}

static unsigned int parse_mach_64(char *module, void* binary, long long(*symbol_handler)(char*, char*, long long, char))	// TODO: add param to specify valid archs
{	
	char is64 = false;
	unsigned int module_start = 0xFFFFFFFF;
	EFI_STATUS bind_status = EFI_SUCCESS;
    
	// TODO convert all of the structs to a union	
	struct dyld_info_command* dyldInfoCommand = NULL;	
	struct symtab_command* symtabCommand = NULL;	
	
	{
		struct segment_command *segCommand = NULL;
		struct segment_command_64 *segCommand64 = NULL;
		struct load_command *loadCommand = NULL;
		UInt32 binaryIndex = 0;
		UInt16 cmd = 0;
		
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
						
#if DEBUG_KERNEL_PATCHER
						unsigned long fileaddr;
						long   filesize;					
						vmaddr = (segCommand->vmaddr & 0x3fffffff);
						vmsize = segCommand->vmsize;	  
						fileaddr = ((unsigned long)(binary + binaryIndex) + segCommand->fileoff);
						filesize = segCommand->filesize;
						
						printf("segname: %s, vmaddr: %x, vmsize: %x, fileoff: %x, filesize: %x, nsects: %d, flags: %x.\n",
							   segCommand->segname, (unsigned)vmaddr, (unsigned)vmsize, (unsigned)fileaddr, (unsigned)filesize,
							   (unsigned) segCommand->nsects, (unsigned)segCommand->flags);
#if DEBUG_KERNEL_PATCHER==2
						
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
				case LC_SEGMENT_64:	// 64bit macho's
				{
					segCommand64 = binary + binaryIndex;
					
					//printf("Segment name is %s\n", segCommand->segname);
					
					if(strncmp("__TEXT", segCommand64->segname, sizeof("__TEXT")) == 0)
					{
						UInt32 sectionIndex;
						
#if DEBUG_KERNEL_PATCHER						
						unsigned long fileaddr;
						long   filesize;					
						vmaddr = (segCommand64->vmaddr & 0x3fffffff);
						vmsize = segCommand64->vmsize;	  
						fileaddr = ((unsigned long)(binary + binaryIndex) + segCommand64->fileoff);
						filesize = segCommand64->filesize;
						
						printf("segname: %s, vmaddr: %x, vmsize: %x, fileoff: %x, filesize: %x, nsects: %d, flags: %x.\n",
							   segCommand64->segname, (unsigned)vmaddr, (unsigned)vmsize, (unsigned)fileaddr, (unsigned)filesize,
							   (unsigned) segCommand64->nsects, (unsigned)segCommand64->flags);
#if DEBUG_KERNEL_PATCHER==2
						
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
	module_start = handle_symtable_64(module, (UInt32)binary, symtabCommand, symbol_handler, is64);

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

static unsigned int handle_symtable_64(char *module, UInt32 base, struct symtab_command* symtabCommand, long long(*symbol_handler)(char*, char*, long long, char), char is64)
{		
	unsigned int module_start = 0xFFFFFFFF;
	
	UInt32 symbolIndex = 0;
    if (!symtabCommand) {
        return 0xFFFFFFFF;
    }
	char* symbolString = base + (char*)symtabCommand->stroff;
	if(!is64)
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
					symbol_handler(module, symbolString + symbolEntry->n_un.n_strx, (long long)base + symbolEntry->n_value, is64);

					
				}
#if DEBUG_KERNEL_PATCHER
				bool isTexT = (((unsigned)symbolEntry->n_value > (unsigned)vmaddr) && ((unsigned)(vmaddr + vmsize) > (unsigned)symbolEntry->n_value ));
				printf("%s %s\n", isTexT ? "__TEXT :" : "__DATA(OR ANY) :", symbolString + symbolEntry->n_un.n_strx);
#if DEBUG_KERNEL_PATCHER==2	
				
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
	return module_start;
	
}

/**
 **		This functions located the requested symbols in the mach-o file.
 **			as well as determines the start of the __TEXT segment and __TEXT,__text sections
 **/
static int locate_symbols(void* kernelData)
{
	char is64 = 1;
	parse_mach_64("VirtualXnuSyms",kernelData, symbol_handler);
	//handle_symtable((UInt32)kernelData, symtableData, &symbol_handler, determineKernelArchitecture(kernelData) == KERNEL_64);
	return 1 << is64;
}

long long symbol_handler(char* module, char* symbolName, long long addr, char is64)
{
	// Locate the symbol in the list, if it exists, update it's address
	kernSymbols_t *symbol = lookup_kernel_symbol(symbolName);
	
	if(symbol)
	{
		symbol->addr = addr;
	}
	
	return 0xFFFFFFFF; // fixme
}

/**
 ** Locate the fisrt instance of _panic inside of _cpuid_set_info, and either remove it
 ** Or replace it so that the cpuid is set to a valid value.
 **/
static void patch_cpuid_set_info_all(void* kernelData)
{
	switch(get_env(envModel))
	{
		case CPUID_MODEL_ATOM:
			if(determineKernelArchitecture(kernelData) == KERNEL_32)
			{
				patch_cpuid_set_info_32(kernelData, CPUFAMILY_INTEL_PENRYN, CPUID_MODEL_PENRYN);
			}
			else 
			{
				patch_cpuid_set_info_64(kernelData, CPUFAMILY_INTEL_PENRYN, CPUID_MODEL_PENRYN);
			}

			break;

		default:
		{
			// AnV: Extra cpuid fix for spoofing Nehalem CPU for i5/i9
			switch(get_env(envFamily))
			{
				case 0x1E: /* Intel i5 */
				case 0x2C: /* Intel i9 */
					if(determineKernelArchitecture(kernelData) == KERNEL_32)
					{
						patch_cpuid_set_info_32(kernelData, CPUFAMILY_INTEL_NEHALEM, CPUID_MODEL_NEHALEM);
					}
					else 
					{
						patch_cpuid_set_info_64(kernelData, CPUFAMILY_INTEL_NEHALEM, CPUID_MODEL_NEHALEM);
					}
					
					break;
				
				default:
					if(determineKernelArchitecture(kernelData) == KERNEL_32)
					{
						patch_cpuid_set_info_32(kernelData, 0, 0);
					}
					else
					{
						patch_cpuid_set_info_64(kernelData, 0, 0);
					}					
					break;
			}
			break;			
		}
	}
}

static void patch_cpuid_set_info_64(void* kernelData, UInt32 impersonateFamily, UInt8 impersonateModel)
{
	UInt8* bytes = (UInt8*)kernelData;
	
	kernSymbols_t *symbol = lookup_kernel_symbol("_cpuid_set_info");
	
	UInt32 patchLocation = symbol ? symbol->addr - textAddress + textSection: 0; //	(kernelSymbolAddresses[SYMBOL_CPUID_SET_INFO] - textAddress + textSection);
	patchLocation -= (UInt32)kernelData;	// Remove offset

	//UInt32 jumpLocation = 0;

	if(symbol == 0 || symbol->addr == 0)
	{
		verbose("Unable to locate _cpuid_set_info\n");
		return;
	}
	
	symbol = lookup_kernel_symbol("_panic");
	UInt32 panicAddr = symbol ? symbol->addr - textAddress: 0; //kernelSymbolAddresses[SYMBOL_PANIC] - textAddress;
	if(symbol == 0 || symbol->addr == 0)
	{
		printf("Unable to locate _panic\n");
		return;
	}
	panicAddr -= (UInt32)kernelData;

	//TODO: don't assume it'll always work (Look for *next* function address in symtab and fail once it's been reached)
	while(  
		  (bytes[patchLocation -1] != 0xE8) ||
		  ( ( (UInt32)(panicAddr - patchLocation  - 4) + textSection ) != (UInt32)((bytes[patchLocation + 0] << 0  | 
																					bytes[patchLocation + 1] << 8  | 
																					bytes[patchLocation + 2] << 16 |
																					bytes[patchLocation + 3] << 24)))
		  )
	{
		patchLocation++;
	}
	patchLocation--;

	// Remove panic just in ca se
	// The panic instruction is exactly 5 bytes long.
	bytes[patchLocation + 0] = 0x90;
	bytes[patchLocation + 1] = 0x90;
	bytes[patchLocation + 2] = 0x90;
	bytes[patchLocation + 3] = 0x90;
	bytes[patchLocation + 4] = 0x90;

	// Check for a 10.2.0+ kernel
	if(bytes[patchLocation - 19] == 0xC7 && bytes[patchLocation - 18] == 0x05)
	{
		UInt32 cpuid_cpufamily_addr =	bytes[patchLocation - 17] << 0  |
										bytes[patchLocation - 16] << 8  |
										bytes[patchLocation - 15] << 16 |
										bytes[patchLocation - 14] << 24;
		
		// NOTE: may change, determined based on cpuid_info struct
		UInt32 cpuid_model_addr = cpuid_cpufamily_addr  - 310; 

		//ffffff8000228b3b -> 0x00490e8b
		//ffffff8000228c28 -> -237 -> 0x490D9E -> -310

		// The mov is 10 bytes
		/*
		 bytes[patchLocation - 19] = 0x90;	// c7
		 bytes[patchLocation - 18] = 0x90;	// 05
		 bytes[patchLocation - 17] = 0x90;	// family location
		 bytes[patchLocation - 16] = 0x90;	// family location
		 bytes[patchLocation - 15] = 0x90;	// family location
		 bytes[patchLocation - 14] = 0x90;	// family location
		 */
		bytes[patchLocation - 13] = (impersonateFamily & 0x000000FF) >> 0;
		bytes[patchLocation - 12] = (impersonateFamily & 0x0000FF00) >> 8;
		bytes[patchLocation - 11] = (impersonateFamily & 0x00FF0000) >> 16;	
		bytes[patchLocation - 10] = (impersonateFamily & 0xFF000000) >> 24;

		// The lea (%rip),%rip is 7 bytes
		bytes[patchLocation - 9] = 0xC7;
		bytes[patchLocation - 8] = 0x05;
		bytes[patchLocation - 7] = ((cpuid_model_addr -10) & 0x000000FF) >> 0;	// NOTE: this opcode is relative in 64bit mode, subtract offset
		bytes[patchLocation - 6] = ((cpuid_model_addr -10) & 0x0000FF00) >> 8;	
		bytes[patchLocation - 5] = ((cpuid_model_addr -10) & 0x00FF0000) >> 16;
		bytes[patchLocation - 4] = ((cpuid_model_addr -10) & 0xFF000000) >> 24;
		bytes[patchLocation - 3] = impersonateModel;	// cpuid_model

		// The xor eax eax is 2 bytes
		bytes[patchLocation - 2] = 0x01;	// cpuid_extmodel
		bytes[patchLocation - 1] = 0x00;	// cpuid_extfamily
		
		// The panic instruction is exactly 5 bytes long.
		bytes[patchLocation - 0] = 0x02;	// cpuid_stepping
		/*bytes[patchLocation + 1] = 0x90;
		bytes[patchLocation + 2] = 0x90;
		bytes[patchLocation + 3] = 0x90;
		bytes[patchLocation + 4] = 0x90;
		*/
		
		// Panic call has been removed.
		// Override the CPUID now. This requires ~ 10 bytes on 10.0.0 kernels
		// On 10.2.0+ kernels, this requires ~16 bytes
		
		// Total: 24 bytes
		printf("Running on a 10.2.0+ kernel\n");
		getc();
	}
	else {
		printf("Running on a 10.0.0 kernel, patch unsupported\n");
		getc();
	}
}

static void patch_cpuid_set_info_32(void* kernelData, UInt32 impersonateFamily, UInt8 impersonateModel)
{	
	UInt8* bytes = (UInt8*)kernelData;
	
	kernSymbols_t *symbol = lookup_kernel_symbol("_cpuid_set_info");

	UInt32 patchLocation = symbol ? symbol->addr - textAddress + textSection: 0; //	(kernelSymbolAddresses[SYMBOL_CPUID_SET_INFO] - textAddress + textSection);
	patchLocation -= (UInt32)kernelData;	// Remove offset

	UInt32 jumpLocation = 0;	

	if(symbol == 0 || symbol->addr == 0)
	{
		verbose("Unable to locate _cpuid_set_info\n");
		return;
	}

	symbol = lookup_kernel_symbol("_panic");
	UInt32 panicAddr = symbol ? symbol->addr - textAddress: 0; //kernelSymbolAddresses[SYMBOL_PANIC] - textAddress;
	if(symbol == 0 || symbol->addr == 0)
	{
		printf("Unable to locate _panic\n");
		return;
	}
	panicAddr -= (UInt32)kernelData;

	//TODO: don't assume it'll always work (Look for *next* function address in symtab and fail once it's been reached)
	while(  
		  (bytes[patchLocation -1] != 0xE8) ||
		  ( ( (UInt32)(panicAddr - patchLocation  - 4) + textSection ) != (UInt32)((bytes[patchLocation + 0] << 0  | 
																					bytes[patchLocation + 1] << 8  | 
																					bytes[patchLocation + 2] << 16 |
																					bytes[patchLocation + 3] << 24)))
		 )
	{
		patchLocation++;
	}
	patchLocation--;
	
	// Remove panic call, just in case the following patch routines fail
	bytes[patchLocation + 0] = 0x90;
	bytes[patchLocation + 1] = 0x90;
	bytes[patchLocation + 2] = 0x90;
	bytes[patchLocation + 3] = 0x90;
	bytes[patchLocation + 4] = 0x90;

	// Locate the jump call, so that 10 bytes can be reclamed.
	// NOTE: This will *NOT* be located on pre 10.6.2 kernels
	jumpLocation = patchLocation - 15;
	while((bytes[jumpLocation - 1] != 0x77 ||
		   bytes[jumpLocation] != (patchLocation - jumpLocation - -8)) &&
		  (patchLocation - jumpLocation) < 0xF0)
	{
		jumpLocation--;
	}
	
	// If found... AND we want to impersonate a specific cpumodel / family...
	if(impersonateFamily &&
	   impersonateModel  &&
	   ((patchLocation - jumpLocation) < 0xF0))
	{
		
		bytes[jumpLocation] -= 10;		// sizeof(movl	$0x6b5a4cd2,0x00872eb4) = 10bytes
		
		/* 
		 * Inpersonate the specified CPU FAMILY and CPU Model
		 */
		
		// bytes[patchLocation - 17] = 0xC7;	// already here... not needed to be done
		// bytes[patchLocation - 16] = 0x05;	// see above
		UInt32 cpuid_cpufamily_addr =	bytes[patchLocation - 15] << 0  |
		bytes[patchLocation - 14] << 8  |
		bytes[patchLocation - 13] << 16 |
		bytes[patchLocation - 12] << 24;
		
		// NOTE: may change, determined based on cpuid_info struct
		UInt32 cpuid_model_addr = cpuid_cpufamily_addr - 299; 

		// cpufamily
		bytes[patchLocation - 11] = (impersonateFamily & 0x000000FF) >> 0;
		bytes[patchLocation - 10] = (impersonateFamily & 0x0000FF00) >> 8;
		bytes[patchLocation -  9] = (impersonateFamily & 0x00FF0000) >> 16;	
		bytes[patchLocation -  8] = (impersonateFamily & 0xFF000000) >> 24;
		
		// NOPS, just in case if the jmp call wasn't patched, we'll jump to a
		// nop and continue with the rest of the patch
		// Yay two free bytes :), 10 more can be reclamed if needed, as well as a few
		// from the above code (only cpuid_model needs to be set.
		bytes[patchLocation - 7] = 0x90;
		bytes[patchLocation - 6] = 0x90;
		
		bytes[patchLocation - 5] = 0xC7;
		bytes[patchLocation - 4] = 0x05;
		bytes[patchLocation - 3] = (cpuid_model_addr & 0x000000FF) >> 0;
		bytes[patchLocation - 2] = (cpuid_model_addr & 0x0000FF00) >> 8;	
		bytes[patchLocation - 1] = (cpuid_model_addr & 0x00FF0000) >> 16;
		bytes[patchLocation - 0] = (cpuid_model_addr & 0xFF000000) >> 24;
		
		// Note: I could have just copied the 8bit cpuid_model in and saved about 4 bytes
		// so if this function need a different patch it's still possible. Also, about ten bytes previous can be freed.
		bytes[patchLocation + 1] = impersonateModel;	// cpuid_model
		bytes[patchLocation + 2] = 0x01;	// cpuid_extmodel
		bytes[patchLocation + 3] = 0x00;	// cpuid_extfamily
		bytes[patchLocation + 4] = 0x02;	// cpuid_stepping
	}
	else if(impersonateFamily && impersonateModel)
	{
		// pre 10.6.2 kernel
		// Locate the jump to directly *after* the panic call,
		jumpLocation = patchLocation - 4;
		while((bytes[jumpLocation - 1] != 0x77 ||
			   bytes[jumpLocation] != (patchLocation - jumpLocation + 4)) &&
			  (patchLocation - jumpLocation) < 0x20)
		{
			jumpLocation--;
		}
		// NOTE above isn't needed (I was going to use it, but I'm not, so instead,
		// I'll just leave it to verify the binary stucture.
		
		// NOTE: the cpumodel_familt data is not set in _cpuid_set_info
		// so we don't need to set it here, I'll get set later based on the model
		// we set now.
		
		if((patchLocation - jumpLocation) < 0x20)
		{
			UInt32 cpuid_model_addr =	(bytes[patchLocation - 14] << 0  |
										 bytes[patchLocation - 13] << 8  |
										 bytes[patchLocation - 12] << 16 |
										 bytes[patchLocation - 11] << 24);
			// Remove jump
			bytes[patchLocation - 9] = 0x90;		///  Was a jump if supported cpu
			bytes[patchLocation - 8] = 0x90;		// jumped past the panic call, we want to override the panic
			
			bytes[patchLocation - 7] = 0x90;
			bytes[patchLocation - 6] = 0x90;
			
			bytes[patchLocation - 5] = 0xC7;
			bytes[patchLocation - 4] = 0x05;
			bytes[patchLocation - 3] = (cpuid_model_addr & 0x000000FF) >> 0;
			bytes[patchLocation - 2] = (cpuid_model_addr & 0x0000FF00) >> 8;	
			bytes[patchLocation - 1] = (cpuid_model_addr & 0x00FF0000) >> 16;
			bytes[patchLocation - 0] = (cpuid_model_addr & 0xFF000000) >> 24;
			
			// Note: I could have just copied the 8bit cpuid_model in and saved about 4 bytes
			// so if this function need a different patch it's still possible. Also, about ten bytes previous can be freed.
			bytes[patchLocation + 1] = impersonateModel;	// cpuid_model
			bytes[patchLocation + 2] = 0x01;	// cpuid_extmodel
			bytes[patchLocation + 3] = 0x00;	// cpuid_extfamily
			bytes[patchLocation + 4] = 0x02;	// cpuid_stepping

#if 0
			patchLocation = jumpLocation;
#endif
			// We now have 14 bytes available for a patch
		}
		else 
		{
			// Patching failed, using NOP replacement done initialy
		}
	}
	else 
	{
		// Either We were unable to change the jump call due to the function's sctructure
		// changing, or the user did not request a patch. As such, resort to just 
		// removing the panic call (using NOP replacement above). Note that the
		// IntelCPUPM kext may still panic due to the cpu's Model ID not being patched
	}
}

/**
 ** SleepEnabler.kext replacement (for those that need it)
 ** Located the KERN_INVALID_ARGUMENT return and replace it with KERN_SUCCESS
 **/
#if 0
static void patch_pmCPUExitHaltToOff(void* kernelData)
{
	UInt8* bytes = (UInt8*)kernelData;

	kernSymbols_t *symbol = lookup_kernel_symbol("_PmCpuExitHaltToOff");
	UInt32 patchLocation = symbol ? symbol->addr - textAddress + textSection: 0;
	
	if(symbol == 0 || symbol->addr == 0)
	{
		printf("Unable to locate _pmCPUExitHaltToOff\n");
		return;
	}
	
	patchLocation -= (UInt32)kernelData;	// Remove offset

	while(bytes[patchLocation - 1]	!= 0xB8 ||
		  bytes[patchLocation]		!= 0x04 ||	// KERN_INVALID_ARGUMENT (0x00000004)
		  bytes[patchLocation + 1]	!= 0x00 ||	// KERN_INVALID_ARGUMENT
		  bytes[patchLocation + 2]	!= 0x00 ||	// KERN_INVALID_ARGUMENT
		  bytes[patchLocation + 3]	!= 0x00)	// KERN_INVALID_ARGUMENT

	{
		patchLocation++;
	}
	bytes[patchLocation] = 0x00;	// KERN_SUCCESS;
}
#endif

static void patch_lapic_init(void* kernelData)
{
	UInt8 panicIndex = 0;
	UInt8* bytes = (UInt8*)kernelData;
	
	kernSymbols_t *symbol = lookup_kernel_symbol("_lapic_init");
	UInt32 patchLocation = symbol ? symbol->addr - textAddress + textSection: 0; 
	if(symbol == 0 || symbol->addr == 0)
	{
		printf("Unable to locate %s\n", "_lapic_init");
		return;
	}
	
	symbol = lookup_kernel_symbol("_panic");
	UInt32 panicAddr = symbol ? symbol->addr - textAddress: 0; 
	if(symbol == 0 || symbol->addr == 0)
	{
		printf("Unable to locate %s\n", "_panic");
		return;
	}
	
	patchLocation -= (UInt32)kernelData;	// Remove offset
	panicAddr -= (UInt32)kernelData;	// Remove offset

	// Locate the (panicIndex + 1) panic call
	while(panicIndex < 3)	// Find the third panic call
	{
		while(  
			  (bytes[patchLocation -1] != 0xE8) ||
			  ( ( (UInt32)(panicAddr - patchLocation  - 4) + textSection ) != (UInt32)((bytes[patchLocation + 0] << 0  | 
																						bytes[patchLocation + 1] << 8  | 
																						bytes[patchLocation + 2] << 16 |
																						bytes[patchLocation + 3] << 24)))
			 )
		{
			patchLocation++;
		}
		patchLocation++;
		panicIndex++;
	}
	patchLocation--;	// Remove extra increment from the < 3 while loop
	
	bytes[--patchLocation] = 0x90;	
	bytes[++patchLocation] = 0x90;
	bytes[++patchLocation] = 0x90;
	bytes[++patchLocation] = 0x90;
	bytes[++patchLocation] = 0x90;
}

static void patch_commpage_stuff_routine(void* kernelData)
{
	UInt8* bytes = (UInt8*)kernelData;

	kernSymbols_t *symbol = lookup_kernel_symbol("_commpage_stuff_routine");
	if(symbol == 0 || symbol->addr == 0)
	{
		//printf("Unable to locate %s\n", "_commpage_stuff_routine");
		return;
		
	}
	
	UInt32 patchLocation = symbol->addr - textAddress + textSection; 

	symbol = lookup_kernel_symbol("_panic");
	if(symbol == 0 || symbol->addr == 0)
	{
		printf("Unable to locate %s\n", "_panic");
		return;
	}
	UInt32 panicAddr = symbol->addr - textAddress; 

	patchLocation -= (UInt32)kernelData;
	panicAddr -= (UInt32)kernelData;
	
	while(  
		  (bytes[patchLocation -1] != 0xE8) ||
		  ( ( (UInt32)(panicAddr - patchLocation  - 4) + textSection ) != (UInt32)((bytes[patchLocation + 0] << 0  | 
																					bytes[patchLocation + 1] << 8  | 
																					bytes[patchLocation + 2] << 16 |
																					bytes[patchLocation + 3] << 24)))
		 )
	{
		patchLocation++;
	}
	patchLocation--;
	
	// Replace panic with nops
	bytes[patchLocation + 0] = 0x90;
	bytes[patchLocation + 1] = 0x90;
	bytes[patchLocation + 2] = 0x90;
	bytes[patchLocation + 3] = 0x90;
	bytes[patchLocation + 4] = 0x90;
}
#if 0
static void patch_lapic_interrupt(void* kernelData)
{
	// NOTE: this is a hack untill I finish patch_lapic_configure
	UInt8* bytes = (UInt8*)kernelData;

	kernSymbols_t *symbol = lookup_kernel_symbol("_lapic_interrupt");
	if(symbol == 0 || symbol->addr == 0)
	{
		printf("Unable to locate %s\n", "_lapic_interrupt");
		return;
	}

	UInt32 patchLocation = symbol->addr - textAddress + textSection; 

	symbol = lookup_kernel_symbol("_panic");
	if(symbol == 0 || symbol->addr == 0)
	{
		printf("Unable to locate %s\n", "_panic");
		return;
	}
	UInt32 panicAddr = symbol->addr - textAddress; 
	
	patchLocation -= (UInt32)kernelData;
	panicAddr -= (UInt32)kernelData;
	
	while(  
		  (bytes[patchLocation -1] != 0xE8) ||
		  ( ( (UInt32)(panicAddr - patchLocation  - 4) + textSection ) != (UInt32)((bytes[patchLocation + 0] << 0  | 
																					bytes[patchLocation + 1] << 8  | 
																					bytes[patchLocation + 2] << 16 |
																					bytes[patchLocation + 3] << 24)))
		  )
	{
		patchLocation++;
	}
	patchLocation--;
	
	// Replace panic with nops
	bytes[patchLocation + 0] = 0x90;
	bytes[patchLocation + 1] = 0x90;
	bytes[patchLocation + 2] = 0x90;
	bytes[patchLocation + 3] = 0x90;
	bytes[patchLocation + 4] = 0x90;
}
#endif
static void patch_lapic_configure(void* kernelData)
{
	UInt8* bytes = (UInt8*)kernelData;
	
	UInt32 patchLocation;
	UInt32 lapicStart;
	UInt32 lapicInterruptBase;
	
	kernSymbols_t *symbol = lookup_kernel_symbol("_lapic_configure");
	if(symbol == 0 || symbol->addr == 0)
	{
		printf("Unable to locate %s\n", "_lapic_configure");
		return;
	}
	patchLocation = symbol->addr - textAddress + textSection; 
	
	symbol = lookup_kernel_symbol("_lapic_start");
	if(symbol == 0 || symbol->addr == 0)
	{
		printf("Unable to locate %s\n", "_lapic_start");
		return;
	}
	lapicStart = symbol->addr; 

	symbol = lookup_kernel_symbol("_lapic_interrupt_base");
	if(symbol == 0 || symbol->addr == 0)
	{
		printf("Unable to locate %s\n", "_lapic_interrupt_base");
		return;
	}
	lapicInterruptBase = symbol->addr;
	patchLocation -= (UInt32)kernelData;
	lapicStart -= (UInt32)kernelData;
	lapicInterruptBase -= (UInt32)kernelData;
	
	// Looking for the following:
	//movl   _lapic_start,%e_x
	//addl   $0x00000320,%e_x
	//  8b 15 __ __ __ __ 81 c2 20 03 00 00
	while(  
		  (bytes[patchLocation - 2] != 0x8b) ||
		  //bytes[patchLocation -1] != 0x15) ||	// Register, we don't care what it is
		  ( lapicStart  != (UInt32)(
									(bytes[patchLocation + 0] << 0  | 
									 bytes[patchLocation + 1] << 8  | 
									 bytes[patchLocation + 2] << 16 |
									 bytes[patchLocation + 3] << 24
									)
								   )
		  ) || 
		  (bytes[patchLocation + 4 ] != 0x81) ||
		  //(bytes[patchLocation + 5 ] != 0Cx2) ||	// register
		  (bytes[patchLocation + 6 ] != 0x20) ||
		  (bytes[patchLocation + 7 ] != 0x03) ||
		  (bytes[patchLocation + 8 ] != 0x00) ||
		  (bytes[patchLocation + 9] != 0x00)

		  )
	{
		patchLocation++;
	}
	patchLocation-=2;

	// NOTE: this is currently hardcoded, change it to be more resilient to changes
	// At a minimum, I should have this do a cheksup first and if not matching, remove the panic instead.
	
	// 8b 15 __ __ __ __ ->  movl		  _lapic_start,%edx (NOTE: this should already be here)
	/*
	bytes[patchLocation++] = 0x8B;
	bytes[patchLocation++] = 0x15;
	bytes[patchLocation++] = (lapicStart & 0x000000FF) >> 0;
	bytes[patchLocation++] = (lapicStart & 0x0000FF00) >> 8;
	bytes[patchLocation++] = (lapicStart & 0x00FF0000) >> 16;
	bytes[patchLocation++] = (lapicStart & 0xFF000000) >> 24;
	*/
	patchLocation += 6;
	
	// 81 c2 60 03 00 00 -> addl		  $0x00000320,%edx
	/*
	bytes[patchLocation++] = 0x81;
	bytes[patchLocation++] = 0xC2;
	*/
	patchLocation += 2;
	bytes[patchLocation++] = 0x60;
	/*
	bytes[patchLocation++];// = 0x03;
	bytes[patchLocation++];// = 0x00;
	bytes[patchLocation++];// = 0x00;
	*/
	patchLocation += 3;

	// c7 02 00 04 00 00 -> movl		  $0x00000400,(%edx)
	bytes[patchLocation++] = 0xC7;
	bytes[patchLocation++] = 0x02;
	bytes[patchLocation++] = 0x00;
	bytes[patchLocation++] = 0x04;
	bytes[patchLocation++] = 0x00;
	bytes[patchLocation++] = 0x00;
	
	// 83 ea 40 -> subl		  $0x40,edx
	bytes[patchLocation++] = 0x83;
	bytes[patchLocation++] = 0xEA;
	bytes[patchLocation++] = 0x40;

	// a1 __ __ __ __ -> movl		  _lapic_interrupt_base,%eax
	bytes[patchLocation++] = 0xA1;
	bytes[patchLocation++] = (lapicInterruptBase & 0x000000FF) >> 0;
	bytes[patchLocation++] = (lapicInterruptBase & 0x0000FF00) >> 8;
	bytes[patchLocation++] = (lapicInterruptBase & 0x00FF0000) >> 16;
	bytes[patchLocation++] = (lapicInterruptBase & 0xFF000000) >> 24;

	// 83 c0 0e -> addl		  $0x0e,%eax
	bytes[patchLocation++] = 0x83;
	bytes[patchLocation++] = 0xC0;
	bytes[patchLocation++] = 0x0E;

	// 89 02 -> movl		  %eax,(%edx)
	bytes[patchLocation++] = 0x89;
	bytes[patchLocation++] = 0x02;
	
	// 81c230030000		  addl		  $0x00000330,%edx
	bytes[patchLocation++] = 0x81;
	bytes[patchLocation++] = 0xC2;
	bytes[patchLocation++] = 0x30;
	bytes[patchLocation++] = 0x03;
	bytes[patchLocation++] = 0x00;
	bytes[patchLocation++] = 0x00;
	
	// a1 __ __ __ __ -> movl		  _lapic_interrupt_base,%eax
	bytes[patchLocation++] = 0xA1;
	bytes[patchLocation++] = (lapicInterruptBase & 0x000000FF) >> 0;
	bytes[patchLocation++] = (lapicInterruptBase & 0x0000FF00) >> 8;
	bytes[patchLocation++] = (lapicInterruptBase & 0x00FF0000) >> 16;
	bytes[patchLocation++] = (lapicInterruptBase & 0xFF000000) >> 24;
	
	// 83 c0 0f -> addl		  $0x0f,%eax
	bytes[patchLocation++] = 0x83;
	bytes[patchLocation++] = 0xC0;
	bytes[patchLocation++] = 0x0F;
	
	// 89 02 -> movl		  %eax,(%edx)
	bytes[patchLocation++] = 0x89;
	bytes[patchLocation++] = 0x02;
	
	// 83 ea 10 -> subl		  $0x10,edx
	bytes[patchLocation++] = 0x83;
	bytes[patchLocation++] = 0xEA;
	bytes[patchLocation++] = 0x10;

	// a1 __ __ __ __ -> movl		  _lapic_interrupt_base,%eax
	bytes[patchLocation++] = 0xA1;
	bytes[patchLocation++] = (lapicInterruptBase & 0x000000FF) >> 0;
	bytes[patchLocation++] = (lapicInterruptBase & 0x0000FF00) >> 8;
	bytes[patchLocation++] = (lapicInterruptBase & 0x00FF0000) >> 16;
	bytes[patchLocation++] = (lapicInterruptBase & 0xFF000000) >> 24;
	
	// 83 c0 0c -> addl		  $0x0c,%eax
	bytes[patchLocation++] = 0x83;
	bytes[patchLocation++] = 0xC0;
	bytes[patchLocation++] = 0x0C;

	// 89 02 -> movl		  %eax,(%edx)
	bytes[patchLocation++] = 0x89;
	bytes[patchLocation++] = 0x02;

	// Replace remaining with nops

	bytes[patchLocation++] = 0x90;
	bytes[patchLocation++] = 0x90;
	bytes[patchLocation++] = 0x90;
	bytes[patchLocation++] = 0x90;
	//	bytes[patchLocation++] = 0x90; // double check the lenght of the patch...
	//	bytes[patchLocation++] = 0x90;
}
