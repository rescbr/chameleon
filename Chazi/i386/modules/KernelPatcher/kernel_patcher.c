/*
 * Copyright (c) 2009-2010 Evan Lojewski. All rights reserved.
 *
 */

//#include "libsaio.h"
#include "boot.h"
#include "kernel_patcher.h"
#include "platform.h"
#include "modules.h"

extern PlatformInfo_t    Platform;

patchRoutine_t* patches = NULL;
kernSymbols_t* kernelSymbols = NULL;


void KernelPatcher_start()
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
}

/*
 * Register a kerenl patch
 */
void register_kernel_patch(void* patch, int arch, int cpus)
{
	// TODO: only insert valid patches based on current cpuid and architecture
	// AKA, don't at 64bit patches if it's a 32bit only machine
	patchRoutine_t* entry;
	
	// TODO: verify Platform.CPU.Model is populated this early in bootup
	// Check to ensure that the patch is valid on this machine
	// If it is not, exit early form this function
	if(cpus != Platform.CPU.Model)
	{
		if(cpus != CPUID_MODEL_ANY)
		{
			if(cpus == CPUID_MODEL_UNKNOWN)
			{
				switch(Platform.CPU.Model)
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

void register_kernel_symbol(int kernelType, const char* name)
{
	if(kernelSymbols == NULL)
	{
		kernelSymbols = malloc(sizeof(kernSymbols_t));
		kernelSymbols->next = NULL;
		kernelSymbols->symbol = (char*)name;
		kernelSymbols->addr = 0;
	}
	else {
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

kernSymbols_t* lookup_kernel_symbol(const char* name)
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

void patch_kernel(void* kernelData, void* arg2, void* arg3, void *arg4)
{
	bool			patchKernel = false; //Kpatcher - default value.
	patchRoutine_t* entry = patches;

	int arch = determineKernelArchitecture(kernelData);

	locate_symbols(kernelData);

	// check if kernel patcher is requested by user.
	getBoolForKey(kKPatcherKey, &patchKernel, &bootInfo->bootConfig);

	if (patches != NULL && patchKernel == true)
	{
		while(entry)
		{
			if (entry->validArchs == KERNEL_ANY || arch == entry->validArchs)
			{
				if (entry->patchRoutine) entry->patchRoutine(kernelData);
			}
			entry = entry->next;
		}
	}
}

int determineKernelArchitecture(void* kernelData)
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


/**
 **		This functions located the requested symbols in the mach-o file.
 **			as well as determines the start of the __TEXT segment and __TEXT,__text sections
 **/
int locate_symbols(void* kernelData)
{
	char is64 = 1;
	parse_mach(kernelData, NULL, symbol_handler);
	//handle_symtable((UInt32)kernelData, symtableData, &symbol_handler, determineKernelArchitecture(kernelData) == KERNEL_64);
	return 1 << is64;
}

long long symbol_handler(char* symbolName, long long addr, char is64)
{
	// Locate the symbol in the list, if it exists, update it's address
	kernSymbols_t *symbol = lookup_kernel_symbol(symbolName);
	
	
	
	if(symbol)
	{

		//printf("Located %sbit symbol %s at 0x%lX\n", is64 ? "64" : "32", symbolName, addr);
		//getc();
		
		symbol->addr = addr;
	}
	return 0xFFFFFFFF; // fixme
}


/**
 ** Locate the fisrt instance of _panic inside of _cpuid_set_info, and either remove it
 ** Or replace it so that the cpuid is set to a valid value.
 **/
void patch_cpuid_set_info_all(void* kernelData)
{
	switch(Platform.CPU.Model)
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
}

void patch_cpuid_set_info_64(void* kernelData, UInt32 impersonateFamily, UInt8 impersonateModel)
{
	UInt8* bytes = (UInt8*)kernelData;
	
	kernSymbols_t *symbol = lookup_kernel_symbol("_cpuid_set_info");
	
	UInt32 patchLocation = symbol ? symbol->addr - textAddress + textSection: 0; //	(kernelSymbolAddresses[SYMBOL_CPUID_SET_INFO] - textAddress + textSection);
	patchLocation -= (UInt32)kernelData;	// Remove offset
	
	
	
//	UInt32 jumpLocation = 0; unused for now
	
	
	if(symbol == 0 || symbol->addr == 0)
	{
		printf("Unable to locate _cpuid_set_info\n");
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
	
	verbose("0x%X 0x%X 0x%X 0x%X 0x%X\n", bytes[patchLocation ], bytes[patchLocation +1], bytes[patchLocation +2], bytes[patchLocation +3], bytes[patchLocation +4]);
	
	// Remove panic just in case
	// The panic instruction is exactly 5 bytes long.
	bytes[patchLocation + 0] = 0x90;
	bytes[patchLocation + 1] = 0x90;
	bytes[patchLocation + 2] = 0x90;
	bytes[patchLocation + 3] = 0x90;
	bytes[patchLocation + 4] = 0x90;
	verbose("0x%X 0x%X 0x%X 0x%X 0x%X\n", bytes[patchLocation ], bytes[patchLocation +1], bytes[patchLocation +2], bytes[patchLocation +3], bytes[patchLocation +4]);
	
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

void patch_cpuid_set_info_32(void* kernelData, UInt32 impersonateFamily, UInt8 impersonateModel)
{
	UInt8* bytes = (UInt8*)kernelData;
	
	kernSymbols_t *symbol = lookup_kernel_symbol("_cpuid_set_info");

	UInt32 patchLocation = symbol ? symbol->addr - textAddress + textSection: 0; //	(kernelSymbolAddresses[SYMBOL_CPUID_SET_INFO] - textAddress + textSection);
	patchLocation -= (UInt32)kernelData;	// Remove offset
	
	

	UInt32 jumpLocation = 0;
	
	
	if(symbol == 0 || symbol->addr == 0)
	{
		printf("Unable to locate _cpuid_set_info\n");
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
	
	verbose("0x%X 0x%X 0x%X 0x%X 0x%X\n", bytes[patchLocation ], bytes[patchLocation +1], bytes[patchLocation +2], bytes[patchLocation +3], bytes[patchLocation +4]);

	// Remove panic call, just in case the following patch routines fail
	bytes[patchLocation + 0] = 0x90;
	bytes[patchLocation + 1] = 0x90;
	bytes[patchLocation + 2] = 0x90;
	bytes[patchLocation + 3] = 0x90;
	bytes[patchLocation + 4] = 0x90;
	verbose("0x%X 0x%X 0x%X 0x%X 0x%X\n", bytes[patchLocation ], bytes[patchLocation +1], bytes[patchLocation +2], bytes[patchLocation +3], bytes[patchLocation +4]);
	
	
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
			
			
			
			patchLocation = jumpLocation;
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
void patch_pmCPUExitHaltToOff(void* kernelData)
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

void patch_lapic_init(void* kernelData)
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


void patch_commpage_stuff_routine(void* kernelData)
{
	UInt8* bytes = (UInt8*)kernelData;

	kernSymbols_t *symbol = lookup_kernel_symbol("_commpage_stuff_routine");
	if(symbol == 0 || symbol->addr == 0)
	{
		verbose("Unable to locate %s\n", "_commpage_stuff_routine");
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

void patch_lapic_interrupt(void* kernelData)
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


void patch_lapic_configure(void* kernelData)
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
