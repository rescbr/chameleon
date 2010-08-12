/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include "libsaio.h"
#include "kernel_patcher.h"
#include "platform.h"

extern PlatformInfo_t    Platform;


#define SYMBOL_CPUID_SET_INFO				0
#define SYMBOL_PANIC						1
#define SYMBOL_PMCPUEXITHALTTOOFF			2
#define SYMBOL_LAPIC_INIT					3
#define SYMBOL_COMMPAGE_STUFF_ROUTINE		4
#define NUM_SYMBOLS							5

#define SYMBOL_CPUID_SET_INFO_STRING		"_cpuid_set_info"
#define SYMBOL_PANIC_STRING					"_panic"
#define SYMBOL_PMCPUEXITHALTTOOFF_STRING	"_pmCPUExitHaltToOff"
#define SYMBOL_LAPIC_INIT_STRING			"_lapic_init"
#define SYMBOL_COMMPAGE_STUFF_ROUTINE_STRING	"_commpage_stuff_routine"

char* kernelSymbols[NUM_SYMBOLS] = {
	SYMBOL_CPUID_SET_INFO_STRING,
	SYMBOL_PANIC_STRING,
	SYMBOL_PMCPUEXITHALTTOOFF_STRING,
	SYMBOL_LAPIC_INIT_STRING,
	SYMBOL_COMMPAGE_STUFF_ROUTINE_STRING
};

UInt32 kernelSymbolAddresses[NUM_SYMBOLS] = {
	0,
	0,
	0,
	0,
	0
};


UInt32 textSection = 0;
UInt32 textAddress = 0;


void HelloWorld_start();

void KernelPatcher_start()
{
	printf("KernelPatcher(), about to call HelloWorld_start()\n");
	getc();
	HelloWorld_start();

}




void patch_kernel(void* kernelData)
{
	switch (locate_symbols((void*)kernelData)) {
		case KERNEL_32:
			patch_kernel_32((void*)kernelData);
			break;
			
		case KERNEL_64:
		default:
			patch_kernel_64((void*)kernelData);
			break;
	}
}

// patches a 64bit kernel.
void patch_kernel_64(void* kernelData)
{
	//  At the moment, the kernel patching code fails when used
	// in 64bit mode, so we don't patch it. This is due to 32bit vs 64bit
	// pointers as well as changes in structure sizes
	printf("Unable to patch 64bit kernel. Please use arch=i386.\n");
}


/**
 ** patch_kernel_32
 **		patches kernel based on cpu info determined earlier in the boot process.
 **		It the machine is vmware, remove the artificial lapic panic
 **		If the CPU is not supported, remove the cpuid_set_info panic
 **		If the CPU is and Intel Atom, inject the penryn cpuid info.
 **/
void patch_kernel_32(void* kernelData)
{
	// Remove panic in commpage
	patch_commpage_stuff_routine(kernelData);
	
	//patch_pmCPUExitHaltToOff(kernelData);	// Not working as intended, disabled for now
	
	//	if(vmware_detected)
	{
		patch_lapic_init(kernelData);
	}
	
	switch(13)//Platform.CPU.Model)
	{
			// Known good CPU's, no reason to patch kernel
		case 13:
		case CPUID_MODEL_YONAH:
		case CPUID_MODEL_MEROM:
		case CPUID_MODEL_PENRYN:
		case CPUID_MODEL_NEHALEM:
		case CPUID_MODEL_FIELDS:
		case CPUID_MODEL_DALES:
		case CPUID_MODEL_NEHALEM_EX:
			break;
			
			// Known unsuported CPU's
		case CPUID_MODEL_ATOM:
			// TODO: Impersonate CPU based on user selection
			patch_cpuid_set_info(kernelData, CPUFAMILY_INTEL_PENRYN, CPUID_MODEL_PENRYN);	// Impersonate Penryn CPU
			break;
			
			// Unknown CPU's
		default:	
			// TODO: Impersonate CPU based on user selection
			patch_cpuid_set_info(kernelData, 0, 0);	// Remove Panic Call
			
			break;
	}
}


/**
 **		This functions located the kernelSymbols[i] symbols in the mach-o header.
 **			as well as determines the start of the __TEXT segment and __TEXT,__text sections
 **/
int locate_symbols(void* kernelData)
{
	UInt16 symbolIndexes[NUM_SYMBOLS];

	struct load_command *loadCommand;
	struct symtab_command *symtableData;
	struct nlist *symbolEntry;
	
	char* symbolString;

	UInt32 kernelIndex = 0;
	kernelIndex += sizeof(struct mach_header);
	
	if(((struct mach_header*)kernelData)->magic != MH_MAGIC) return KERNEL_64;
	
	
	//printf("%d load commands beginning at 0x%X\n", (unsigned int)header->ncmds, (unsigned int)kernelIndex);
	//printf("Commands take up %d bytes\n", header->sizeofcmds);
	
	
	int cmd = 0;
	while(cmd < ((struct mach_header*)kernelData)->ncmds)	// TODO: for loop instead
	{
		cmd++;
		
		loadCommand = kernelData + kernelIndex;
		
		UInt cmdSize = loadCommand->cmdsize;
		
		
		// Locate start of _panic and _cpuid_set_info in the symbol tabe.
		// Load commands should be anded with 0x7FFFFFFF to ignore the	LC_REQ_DYLD flag
		if((loadCommand->cmd & 0x7FFFFFFF) == LC_SYMTAB)		// We only care about the symtab segment
		{
			//printf("Located symtable, length is 0x%X, 0x%X\n", (unsigned int)loadCommand->cmdsize, (unsigned int)sizeof(symtableData));
			
			symtableData = kernelData + kernelIndex;
			kernelIndex += sizeof(struct symtab_command);
			
			cmdSize -= sizeof(struct symtab_command);

			// Loop through symbol table untill all of the symbols have been found
			
			symbolString = kernelData + symtableData->stroff; 
			
			
			UInt16 symbolIndex = 0;
			UInt8 numSymbolsFound = 0;

			while(symbolIndex < symtableData->nsyms && numSymbolsFound < NUM_SYMBOLS)	// TODO: for loop
			{
				int i = 0;
				while(i < NUM_SYMBOLS)
				{
					if(strcmp(symbolString, kernelSymbols[i]) == 0)
					{
						symbolIndexes[i] = symbolIndex;
						numSymbolsFound++;				
					} 
					i++;
					
				}
				symbolString += strlen(symbolString) + 1;
				symbolIndex++;
			}
			
			// loop again
			symbolIndex = 0;
			numSymbolsFound = 0;
			while(symbolIndex < symtableData->nsyms && numSymbolsFound < NUM_SYMBOLS)	// TODO: for loop
			{
				
				symbolEntry = kernelData + symtableData->symoff + (symbolIndex * sizeof(struct nlist));
				
				int i = 0;
				while(i < NUM_SYMBOLS)
				{
					if(symbolIndex == (symbolIndexes[i] - 4))
					{
						kernelSymbolAddresses[i] = (UInt32)symbolEntry->n_value;
						numSymbolsFound++;				
					} 
					i++;
					
				}
				
				symbolIndex ++;
			}			
		// Load commands should be anded with 0x7FFFFFFF to ignore the	LC_REQ_DYLD flag
		} else if((loadCommand->cmd & 0x7FFFFFFF) == LC_SEGMENT)		// We only care about the __TEXT segment, any other load command can be ignored
		{
			
			struct segment_command *segCommand;
			
			segCommand = kernelData + kernelIndex;
			
			//printf("Segment name is %s\n", segCommand->segname);
			
			if(strcmp("__TEXT", segCommand->segname) == 0)
			{
				UInt32 sectionIndex;
				
				sectionIndex = sizeof(struct segment_command);
				
				struct section *sect;
				
				while(sectionIndex < segCommand->cmdsize)
				{
					sect = kernelData + kernelIndex + sectionIndex;
					
					sectionIndex += sizeof(struct section);
					
					
					if(strcmp("__text", sect->sectname) == 0)
					{
						// __TEXT,__text found, save the offset and address for when looking for the calls.
						textSection = sect->offset;
						textAddress = sect->addr;
						break;
					}					
				}
			}
			
			
			kernelIndex += cmdSize;
		} else {
			kernelIndex += cmdSize;
		}
	}
	
	return KERNEL_32;
}


/**
 ** Locate the fisrt instance of _panic inside of _cpuid_set_info, and either remove it
 ** Or replace it so that the cpuid is set to a valid value.
 **/
void patch_cpuid_set_info(void* kernelData, UInt32 impersonateFamily, UInt8 impersonateModel)
{
	UInt8* bytes = (UInt8*)kernelData;
	UInt32 patchLocation = (kernelSymbolAddresses[SYMBOL_CPUID_SET_INFO] - textAddress + textSection);
	UInt32 jumpLocation = 0;
	UInt32 panicAddr = kernelSymbolAddresses[SYMBOL_PANIC] - textAddress;
	if(kernelSymbolAddresses[SYMBOL_CPUID_SET_INFO] == 0)
	{
		printf("Unable to locate _cpuid_set_info\n");
		return;
		
	}
	if(kernelSymbolAddresses[SYMBOL_PANIC] == 0)
	{
		printf("Unable to locate _panic\n");
		return;
	}
	
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
		
		
		// cpufamily = CPUFAMILY_INTEL_PENRYN
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
	UInt32 patchLocation = (kernelSymbolAddresses[SYMBOL_PMCPUEXITHALTTOOFF] - textAddress + textSection);

	if(kernelSymbolAddresses[SYMBOL_PMCPUEXITHALTTOOFF] == 0)
	{
		printf("Unable to locate _pmCPUExitHaltToOff\n");
		return;
	}
	
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
	UInt32 patchLocation = (kernelSymbolAddresses[SYMBOL_LAPIC_INIT] - textAddress + textSection);
	UInt32 panicAddr = kernelSymbolAddresses[SYMBOL_PANIC] - textAddress;

	if(kernelSymbolAddresses[SYMBOL_LAPIC_INIT] == 0)
	{
		printf("Unable to locate %s\n", SYMBOL_LAPIC_INIT_STRING);
		return;
		
	}
	if(kernelSymbolAddresses[SYMBOL_PANIC] == 0)
	{
		printf("Unable to locate %s\n", SYMBOL_PANIC_STRING);
		return;
	}
	
	
	
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
	UInt32 patchLocation = (kernelSymbolAddresses[SYMBOL_COMMPAGE_STUFF_ROUTINE] - textAddress + textSection);
	UInt32 panicAddr = kernelSymbolAddresses[SYMBOL_PANIC] - textAddress;
	
	if(kernelSymbolAddresses[SYMBOL_COMMPAGE_STUFF_ROUTINE] == 0)
	{
		printf("Unable to locate %s\n", SYMBOL_COMMPAGE_STUFF_ROUTINE_STRING);
		return;
		
	}
	if(kernelSymbolAddresses[SYMBOL_PANIC] == 0)
	{
		printf("Unable to locate %s\n", SYMBOL_PANIC_STRING);
		return;
	}
	
	
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
	
	
}
