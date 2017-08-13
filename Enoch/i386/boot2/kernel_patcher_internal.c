/*
 * Original idea of patching kernel by Evan Lojewsky, 2009
 *
 * Copyright (c) 2011-2012 Frank Peng. All rights reserved.
 *
 * Correction and improvements by Clover team
 *
 * Ported and adapted to Chameleon by Micky1979 and ErmaC
 *
 *  kexts patcher by Micky1979, 2017
 */

// TODO:
// replace for loops with FindAndReplace

#include "config.h"
#include "boot.h"
#include "libsaio.h"
#include "bootstruct.h"
#include "kernel_patcher_internal.h"
#include "sse3_patcher.h"
#include "sse3_5_patcher.h"
#include "platform.h"

#include "xml.h"
#include "sl.h"

#if DEBUG_KERNEL
	#define DBG(x...)		verbose(x)
#else
	#define DBG(x...)
#endif

#define NVDAVALUE "1" // unused (old method to activate NVidia Web Drivers)

// ===================================
/*
 * Entrypoint from load.c
 */
void patch_kernel_internal(void *kernelData, u_int32_t uncompressed_size)
{
	// ================================
	if (!useDarwinVersion)
	{
		kernelOSVer = MacOSVerCurrent;
	}

	// ================================
	verbose("[ KERNEL PATCHER START ]\n");

	/* to debug kernelOSVer and see if match the OS version: */
	DBG("\n\tMacOSVerCurrent = %08x version.\n\n", MacOSVerCurrent);
	DBG("\n\tkernelOSVer = %08x version.\n\n", kernelOSVer);

	verbose("\n\tWill patch for %d.%d.%d kernel version compatible.\n\n", gDarwinMajor, gDarwinMinor, gDarwinRev);

	verbose("\tKernelBooter_kexts state: %s!\n", KernelBooter_kexts  ? " enabled" : "disabled");
	verbose("\tKernelPm           state: %s!\n", KernelPm            ? " enabled" : "disabled");
	verbose("\tKernelLapicError   state: %s!\n", KernelLapicError    ? " enabled" : "disabled");
	verbose("\tKernelLapicVersion state: %s!\n", KernelLapicVersion  ? " enabled" : "disabled");
	verbose("\tKernelHaswell      state: %s!\n", KernelHaswell       ? " enabled" : "disabled");
	verbose("\tKernelcpuFamily    state: %s!\n", KernelcpuFamily     ? " enabled" : "disabled");
	verbose("\tKernelSSE3         state: %s!\n", KernelSSE3          ? " enabled" : "disabled");
    
    u_int32_t sizeToScan = uncompressed_size; // for further expansion
    
	// Select machine arch
	if (archCpuType == CPU_TYPE_I386)
	{
		patch_kernel_32((void *)kernelData, sizeToScan);
	}
	else
	{
		patch_kernel_64((void *)kernelData, sizeToScan);
	}

	/* VMware issue
	if (patch_string_XNU_init(kernelData))
	{
		DBG("\tKernel string replaced.\n");
	}
	*/
    
    // do user's defined patches
    if (bootInfo->kernelConfig.dictionary) {
        pach_binaryUsingDictionary(kernelData,
                                   (UInt32)uncompressed_size,
                                   0,
                                   "KernelPatches",
                                   bootInfo->kernelConfig.dictionary);
    }
    
	verbose("Kernel patcher: end!\n\n");
}

// ===================================
// patches for a 64-bit kernel.
void patch_kernel_64(void *kernelData, u_int32_t uncompressed_size) // KernelPatcher_64
{
	DBG("[ 64-bit ]\n");

	UInt8       *bytes = (UInt8 *)kernelData;
	UInt32      patchLocation = 0, patchLocation1 = 0;
	UInt32      i;
	UInt32      switchaddr = 0;
	UInt32      mask_family = 0, mask_model = 0;
	UInt32      cpuid_family_addr = 0, cpuid_model_addr = 0;

	// Prelinked Extensions Patcher
	if (KernelBooter_kexts)
	{
        patch_BooterExtensions_64(kernelData);
	}
    
	// Lapic Error
	if (KernelLapicError && patch_lapic_init_64(kernelData))
	{
		verbose("\tLapic Error call removed.\n");
	}

	// Lapic Version
	if (KernelLapicVersion && patch_lapic_version_init_64(kernelData))
	{
		verbose("\tLapic Version call removed.\n");
	}

	// Power Managment
	if (KernelPm && patch_pm_init(kernelData))
	{
		verbose("\tPower Managment patch applied.\n");
	}

	// Haswell Kernel Patcher
	if ( KernelHaswell && (kernelOSVer >= MacOSVer2Int("10.8")) ) // Haswell "E" and "ULT" support
	{
		switch (Platform.CPU.Family)
		{
			case 0x06:
			{
				switch (Platform.CPU.Model)
				{
					case CPUID_MODEL_HASWELL_SVR: // Haswell-E Kernel Patcher
						if (patch_haswell_E_init(kernelData))
						{
							verbose("\tHaswell-E Kernel patch applied.\n");
						}
						break;

					case CPUID_MODEL_HASWELL_ULT: // Haswell-ULT Kernel Patcher
						if (patch_haswell_ULT_init(kernelData))
						{
							verbose("\tHaswell-ULT Kernel patch applied.\n");
						}
						break;

					default:
						verbose("\tNo Haswell-ULT/-E Kernel patch applied for this CPU.\n");
						break;
				}
			}
		}
	}

	if (KernelcpuFamily)
	{
		verbose("\t- Looking for _cpuid_set_info _panic ...\n");

		// Determine location of _cpuid_set_info _panic call for reference
		// basically looking for info_p->cpuid_model = bitfield32(reg[eax],  7,  4);
		for (i=0; i<0x1000000; i++)
		{
			if (bytes[i + 0]    == 0xC7
				&& bytes[i + 1] == 0x05
				&& bytes[i + 5] == 0x00
				&& bytes[i + 6] == 0x07
				&& bytes[i + 7] == 0x00
				&& bytes[i + 8] == 0x00
				&& bytes[i + 9] == 0x00
				&& bytes[i - 5] == 0xE8)
			{
				// matching 0xE8 for _panic call start
				patchLocation = i-5;
				break;
			}
		}

		if (!patchLocation)
		{
			verbose("\t_cpuid_set_info Unsupported CPU _panic not found \n");
			return;
		}

		// make sure only kernels for OSX 10.6.0 to 10.7.3 are being patched by this approach
		if (kernelOSVer >= MacOSVer2Int("10.6") && kernelOSVer <= MacOSVer2Int("10.7.3"))
		{

			verbose("\t- will patch kernel for OSX 10.6.0 to 10.7.3\n");

			// remove tsc_init: unknown CPU family panic for kernels prior to 10.6.2 which still had Atom support
			if (kernelOSVer < MacOSVer2Int("10.6.2"))
			{
				for (i=0; i<0x1000000; i++)
                {
					// find _tsc_init panic address by byte sequence 488d3df4632a00
					if (bytes[i]        == 0x48
						&& bytes[i + 1] == 0x8D
						&& bytes[i + 2] == 0x3D
						&& bytes[i + 3] == 0xF4
						&& bytes[i + 4] == 0x63
						&& bytes[i + 5] == 0x2A
						&& bytes[i + 6] == 0x00)
					{
						patchLocation1 = i+9;
						verbose("\tFound _tsc_init _panic address at 0x%08x\n", (unsigned int)patchLocation1);
						break;
					}
				}

				// NOP _panic call
				if (patchLocation1)
				{
					bytes[patchLocation1 + 0] = 0x90;
					bytes[patchLocation1 + 1] = 0x90;
					bytes[patchLocation1 + 2] = 0x90;
					bytes[patchLocation1 + 3] = 0x90;
					bytes[patchLocation1 + 4] = 0x90;
				}
			}
			else
			{
				// assume patching logic for OSX 10.6.2 to 10.7.3

				/*
				 * Here is our case from CPUID switch statement, it sets CPUFAMILY_UNKNOWN
				 * C7051C2C5F0000000000   mov     dword [ds:0xffffff80008a22c0], 0x0 (example from 10.7)
				 */
				switchaddr = patchLocation - 19;
				verbose("\tswitch statement patch location is 0x%08lx\n", (switchaddr+6));

				if (bytes[switchaddr + 0]    == 0xC7
					&& bytes[switchaddr + 1] == 0x05
					&& bytes[switchaddr + 5] == 0x00
					&& bytes[switchaddr + 6] == 0x00
					&& bytes[switchaddr + 7] == 0x00
					&& bytes[switchaddr + 8] == 0x00)
				{
					// Determine cpuid_family address from above mov operation
					cpuid_family_addr =
						bytes[switchaddr + 2] <<  0 |
						bytes[switchaddr + 3] <<  8 |
						bytes[switchaddr + 4] << 16 |
						bytes[switchaddr + 5] << 24;
					cpuid_family_addr = cpuid_family_addr + (switchaddr + 10);

					if (cpuid_family_addr)
					{
						// Determine cpuid_model address
						// for 10.6.2 kernels it's offset by 299 bytes from cpuid_family address
						if (kernelOSVer ==  MacOSVer2Int("10.6.2"))
						{
							cpuid_model_addr = cpuid_family_addr - 0X12B;
						}
						// for 10.6.3 to 10.6.7 it's offset by 303 bytes
						else if (kernelOSVer <= MacOSVer2Int("10.6.7"))
						{
							cpuid_model_addr = cpuid_family_addr - 0X12F;
						}
						// for 10.6.8 to 10.7.3 kernels - by 339 bytes
						else
						{
							cpuid_model_addr = cpuid_family_addr - 0X153;
						}

						verbose("\tcpuid_family address: 0x%08X\n", (unsigned int)cpuid_family_addr);
						verbose("\tcpuid_model address: 0x%08X\n",  (unsigned int)cpuid_model_addr);

						switchaddr += 6; // offset 6 bytes in mov operation to write a dword instead of zero

						// calculate mask for patching, cpuid_family mask not needed as we offset on a valid mask
						mask_model   = cpuid_model_addr - (switchaddr+14);
						//verbose("\tmodel mask 0x%08x\n", (unsigned int)mask_model);

						//verbose("\toverriding cpuid_family and cpuid_model as CPUID_INTEL_PENRYN\n");
						bytes[switchaddr+0] = (CPUFAMILY_INTEL_PENRYN & 0x000000FF) >>  0;
						bytes[switchaddr+1] = (CPUFAMILY_INTEL_PENRYN & 0x0000FF00) >>  8;
						bytes[switchaddr+2] = (CPUFAMILY_INTEL_PENRYN & 0x00FF0000) >> 16;
						bytes[switchaddr+3] = (CPUFAMILY_INTEL_PENRYN & 0xFF000000) >> 24;

						// mov  dword [ds:0xffffff80008a216d], 0x2000117
						bytes[switchaddr+4] = 0xC7;
						bytes[switchaddr+5] = 0x05;
						bytes[switchaddr+6] = (UInt8)((mask_model & 0x000000FF) >> 0);
						bytes[switchaddr+7] = (UInt8)((mask_model & 0x0000FF00) >> 8);
						bytes[switchaddr+8] = (UInt8)((mask_model & 0x00FF0000) >> 16);
						bytes[switchaddr+9] = (UInt8)((mask_model & 0xFF000000) >> 24);
						bytes[switchaddr+10] = 0x17; // cpuid_model (Penryn)
						bytes[switchaddr+11] = 0x01; // cpuid_extmodel
						bytes[switchaddr+12] = 0x00; // cpuid_extfamily
						bytes[switchaddr+13] = 0x02; // cpuid_stepping

						// fill remainder with 4 NOPs
						for (i = 14; i < 18; i++)
						{
							bytes[switchaddr+i] = 0x90;
						}
					}
				}
				else
				{
					verbose("\tUnable to determine cpuid_family address, patching aborted\n");
					return;
				}
			}

			// patch sse3
			if (KernelSSE3 && (SNOW_LEOPARD))
			{
				patch_SSE3_6((void *)bytes);
			}

			if (KernelSSE3 && (LION))
			{
				patch_SSE3_7((void *)bytes);
			}
		}

		// all 10.7.4+ kernels share common CPUID switch statement logic,
		// it needs to be exploited in diff manner due to the lack of space
		else if (kernelOSVer >= MacOSVer2Int("10.7.4"))
		{
			verbose("\t- will patch kernel for OSX %s (from 10.7.4 and newer)\n", gBootVolume->OSFullVer);

			/*
			 * Here is our switchaddress location ... it should be case 20 from CPUID switch statement
			 * 833D78945F0000  cmp        dword [ds:0xffffff80008a21d0], 0x0;
			 * 7417            je         0xffffff80002a8d71
			 */
			switchaddr = patchLocation-45;
			verbose("\tswitch statement patch location is 0x%08X\n", (unsigned int)switchaddr);

			if(bytes[switchaddr + 0]     == 0x83
				&& bytes[switchaddr + 1] == 0x3D
				&& bytes[switchaddr + 5] == 0x00
				&& bytes[switchaddr + 6] == 0x00
				&& bytes[switchaddr + 7] == 0x74)
			{

				// Determine cpuid_family address
				// 891D4F945F00    mov        dword [ds:0xffffff80008a21a0], ebx
				cpuid_family_addr =
					bytes[switchaddr - 4] <<  0 |
					bytes[switchaddr - 3] <<  8 |
					bytes[switchaddr - 2] << 16 |
					bytes[switchaddr - 1] << 24;
				cpuid_family_addr = cpuid_family_addr + switchaddr;

				if (cpuid_family_addr)
				{
					// Determine cpuid_model address
					// for 10.6.8+ kernels it's 339 bytes apart from cpuid_family address
					cpuid_model_addr = cpuid_family_addr - 0X153;

					verbose("\tcpuid_family address: 0x%08X\n", (unsigned int)cpuid_family_addr);
					verbose("\tcpuid_model address: 0x%08X\n",  (unsigned int)cpuid_model_addr);

					// Calculate masks for patching
					mask_family  = cpuid_family_addr - (switchaddr +15);
					mask_model   = cpuid_model_addr -  (switchaddr +25);
					verbose("\tfamily mask: 0x%08X \n\tmodel mask: 0x%08X\n", (unsigned int)mask_family, (unsigned int)mask_model);

					// retain original
					// test ebx, ebx
					bytes[switchaddr+0] = bytes[patchLocation-13];
					bytes[switchaddr+1] = bytes[patchLocation-12];
					// retain original, but move jump offset by 20 bytes forward
					// jne for above test
					bytes[switchaddr+2] = bytes[patchLocation-11];
					bytes[switchaddr+3] = bytes[patchLocation-10]+0x20;
					// mov ebx, 0x78ea4fbc
					bytes[switchaddr+4] = 0xBB;
					bytes[switchaddr+5] = (CPUFAMILY_INTEL_PENRYN & 0x000000FF) >>  0;
					bytes[switchaddr+6] = (CPUFAMILY_INTEL_PENRYN & 0x0000FF00) >>  8;
					bytes[switchaddr+7] = (CPUFAMILY_INTEL_PENRYN & 0x00FF0000) >> 16;
					bytes[switchaddr+8] = (CPUFAMILY_INTEL_PENRYN & 0xFF000000) >> 24;

					// mov dword, ebx
					bytes[switchaddr+9]  = 0x89;
					bytes[switchaddr+10] = 0x1D;
					// cpuid_cpufamily address 0xffffff80008a21a0
					bytes[switchaddr+11] = (UInt8)((mask_family & 0x000000FF) >> 0);
					bytes[switchaddr+12] = (UInt8)((mask_family & 0x0000FF00) >> 8);
					bytes[switchaddr+13] = (UInt8)((mask_family & 0x00FF0000) >> 16);
					bytes[switchaddr+14] = (UInt8)((mask_family & 0xFF000000) >> 24);

					// mov dword
					bytes[switchaddr+15] = 0xC7;
					bytes[switchaddr+16] = 0x05;
					// cpuid_model address 0xffffff80008b204d
					bytes[switchaddr+17] = (UInt8)((mask_model & 0x000000FF) >> 0);
					bytes[switchaddr+18] = (UInt8)((mask_model & 0x0000FF00) >> 8);
					bytes[switchaddr+19] = (UInt8)((mask_model & 0x00FF0000) >> 16);
					bytes[switchaddr+20] = (UInt8)((mask_model & 0xFF000000) >> 24);

					bytes[switchaddr+21] = 0x17; // cpuid_model
					bytes[switchaddr+22] = 0x01; // cpuid_extmodel
					bytes[switchaddr+23] = 0x00; // cpuid_extfamily
					bytes[switchaddr+24] = 0x02; // cpuid_stepping

					// fill remainder with 25 NOPs
					for (i=25; i<25+25; i++)
					{
						bytes[switchaddr+i] = 0x90;
					}
				}
			}
			else
			{
				verbose("\tUnable to determine cpuid_family address, patching aborted\n");
				return;
			}
		}

		verbose("\n");
	}
}

// ===================================
// patches for a 32-bit kernel.
void patch_kernel_32(void *kernelData, u_int32_t uncompressed_size) // KernelPatcher_32
{
	DBG("[ 32-bit ]\n");

	UInt8 *bytes = ( UInt8 *)kernelData;
	UInt32 patchLocation = 0, patchLocation1 = 0;
	UInt32 i;
	UInt32 jumpaddr;

	// Prelinked Extensions Patcher
	if (KernelBooter_kexts)
	{
		patch_BooterExtensions_32(kernelData);
	}

	// Lapic Error
	if (KernelLapicError && patch_lapic_init_32(kernelData))
	{
		verbose("\tLapic Error call removed.\n");
	}

	// Lapic Version
	if (KernelLapicVersion && patch_lapic_version_init_32(kernelData))
	{
		verbose("\tLapic Version call removed.\n");
	}

	if (KernelcpuFamily)
	{
		verbose("\t- Looking for _cpuid_set_info _panic ...\n");
		// _cpuid_set_info _panic address
		for (i = 0; i < 0x1000000; i++)
		{
			if (bytes[i]         == 0xC7
				&& bytes[i + 1]  == 0x05
				&& bytes[i + 6]  == 0x07
				&& bytes[i + 7]  == 0x00
				&& bytes[i + 8]  == 0x00
				&& bytes[i + 9]  == 0x00
				&& bytes[i + 10] == 0xC7
				&& bytes[i + 11] == 0x05
				&& bytes[i - 5]  == 0xE8)
			{
				patchLocation = i-5;
				verbose("\tFound _cpuid_set_info _panic address at 0x%08X\n", (unsigned int)patchLocation);
				break;
			}
		}

		if (!patchLocation)
		{
			verbose("\tCan't find _cpuid_set_info _panic address, patch kernel abort.\n");
			return;
		}

		// this for 10.6.0 and 10.6.1 kernel and remove tsc.c unknow cpufamily panic
		//  c70424540e5900
		// find _tsc_init panic address
		for (i = 0; i < 0x1000000; i++)
		{
			// _cpuid_set_info _panic address
			if (bytes[i]        == 0xC7
				&& bytes[i + 1] == 0x04
				&& bytes[i + 2] == 0x24
				&& bytes[i + 3] == 0x54
				&& bytes[i + 4] == 0x0E
				&& bytes[i + 5] == 0x59
				&& bytes[i + 6] == 0x00)
			{
				patchLocation1 = i+7;
				verbose("\tFound _tsc_init _panic address at 0x%08X\n", (unsigned int)patchLocation1);
				break;
			}
		}

		// found _tsc_init panic addres and patch it
		if (patchLocation1)
		{
			bytes[patchLocation1 + 0] = 0x90;
			bytes[patchLocation1 + 1] = 0x90;
			bytes[patchLocation1 + 2] = 0x90;
			bytes[patchLocation1 + 3] = 0x90;
			bytes[patchLocation1 + 4] = 0x90;
		}
		// end tsc.c panic

		//first move panic code total 5 bytes, if patch cpuid fail still can boot with kernel
		bytes[patchLocation + 0] = 0x90;
		bytes[patchLocation + 1] = 0x90;
		bytes[patchLocation + 2] = 0x90;
		bytes[patchLocation + 3] = 0x90;
		bytes[patchLocation + 4] = 0x90;

		jumpaddr = patchLocation;

		for (i = 0 ;i < 500; i++)
		{
			if (bytes[jumpaddr-i-3]    == 0x85
				&& bytes[jumpaddr-i-2] == 0xC0
				&& bytes[jumpaddr-i-1] == 0x75 )
			{
				jumpaddr -= i;
				bytes[jumpaddr-1] = 0x77;
				if(bytes[patchLocation - 17] == 0xC7)
				{
					bytes[jumpaddr] -=10;
				}
				break;
			}
		}

		if (jumpaddr == patchLocation)
		{
			verbose("\tCan't Found jumpaddr address.\n");
			return;  //can't find jump location
		}
		// patch info_p->cpufamily to CPUFAMILY_INTEL_MEROM

		if (bytes[patchLocation - 17] == 0xC7)
		{
			bytes[patchLocation - 11] = (CPUFAMILY_INTEL_MEROM & 0x000000FF) >>  0;
			bytes[patchLocation - 10] = (CPUFAMILY_INTEL_MEROM & 0x0000FF00) >>  8;
			bytes[patchLocation -  9] = (CPUFAMILY_INTEL_MEROM & 0x00FF0000) >> 16;
			bytes[patchLocation -  8] = (CPUFAMILY_INTEL_MEROM & 0xFF000000) >> 24;
		}

		//patch info->cpuid_cpufamily
		bytes[patchLocation -  7] = 0xC7;
		bytes[patchLocation -  6] = 0x05;
		bytes[patchLocation -  5] = bytes[jumpaddr + 3];
		bytes[patchLocation -  4] = bytes[jumpaddr + 4];
		bytes[patchLocation -  3] = bytes[jumpaddr + 5];
		bytes[patchLocation -  2] = bytes[jumpaddr + 6];

		bytes[patchLocation -  1] = CPUIDFAMILY_DEFAULT; //cpuid_family  need alway set 0x06
		bytes[patchLocation +  0] = CPUID_MODEL_MEROM;   //cpuid_model set CPUID_MODEL_MEROM
		bytes[patchLocation +  1] = 0x01;                //cpuid_extmodel alway set 0x01
		bytes[patchLocation +  2] = 0x00;                //cpuid_extfamily alway set 0x00
		bytes[patchLocation +  3] = 0x90;
		bytes[patchLocation +  4] = 0x90;

		if (KernelSSE3 && ( LION ))
		{
			patch_SSE3_6((void *)bytes);
		}

		if (KernelSSE3 && ( LEOPARD ))
		{
			patch_SSE3_5((void *)bytes);
		}
	}
}

// ===================================
// Power Managment
bool patch_pm_init(void *kernelData) // KernelPatchPm
{
	UInt8  *Ptr = (UInt8 *)kernelData;
	UInt8  *End = Ptr + 0x1000000;
	if (Ptr == NULL)
	{
		return false;
	}

	// Credits to RehabMan for the kernel patch information
	// XCPM (Xnu CPU Power Management)
	verbose("\t- Patching kernel power management...\n");
	while (Ptr < End)
	{
		if (KERNEL_PATCH_SIGNATURE == (*((UInt64 *)Ptr)))
		{
			// Bytes 19,20 of KernelPm patch for kernel 13.x change between kernel versions, so we skip them in search&replace
			if ((memcmp(Ptr + sizeof(UInt64),   KernelPatchPmSrc + sizeof(UInt64),   18 * sizeof(UInt8) - sizeof(UInt64)) == 0) &&
				(memcmp(Ptr + 20 * sizeof(UInt8), KernelPatchPmSrc + 20 * sizeof(UInt8), sizeof(KernelPatchPmSrc) - 20 * sizeof(UInt8)) == 0))
			{
				// Don't copy more than the source here!
				memcpy(Ptr, KernelPatchPmRepl, 18 * sizeof(UInt8)); // Copy block of memory
				memcpy(Ptr + 20 * sizeof(UInt8), KernelPatchPmRepl + 20 * sizeof(UInt8), sizeof(KernelPatchPmSrc) - 20 * sizeof(UInt8)); // Copy block of memory
				verbose("\tKernel power management patch region 1 found and patched\n");
				return true;
			}
			else if (memcmp(Ptr + sizeof(UInt64), KernelPatchPmSrc2 + sizeof(UInt64), sizeof(KernelPatchPmSrc2) - sizeof(UInt64)) == 0)
			{
				// Don't copy more than the source here!
				memcpy(Ptr, KernelPatchPmRepl2, sizeof(KernelPatchPmSrc2)); // Copy block of memory
				verbose("\tKernel power management patch region 2 found and patched\n");
				return true;
			}
		}
		// RehabMan: for 10.10 (data portion)
		else if (0x00000002000000E2ULL == (*((UInt64 *)Ptr)))
		{
			(*((UInt64 *)Ptr)) = 0x0000000000000000ULL;
			verbose("\tKernel power management patch 10.1x(data1) found and patched\n");
		}
		else if (0x0000004C000000E2ULL == (*((UInt64 *)Ptr)))
		{
			(*((UInt64 *)Ptr)) = 0x0000000000000000ULL;
			verbose("\tKernel power management patch 10.1x(data2) found and patched\n");
		}
		else if (0x00000190000000E2ULL == (*((UInt64 *)Ptr)))
		{
			(*((UInt64 *)Ptr)) = 0x0000000000000000ULL;
			verbose("\tKernel power management patch 10.10(data3) found and patched\n");
			return true;
		}
		// rehabman: change for 10.11.1 beta 15B38b
		else if (0x00001390000000E2ULL == (*((UInt64 *)Ptr)))
		{
			(*((UInt64 *)Ptr)) = 0x0000000000000000ULL;
			verbose("\tKernel power management patch 10.11(data3) found and patched\n");
			return true;
		}
		//rehabman: change for 10.11.6 security update 2017-003 15G1611
		else if (0x00001b90000000E2ULL == (*((UInt64 *)Ptr)))
		{
			(*((UInt64 *)Ptr)) = 0x0000000000000000ULL;
			verbose("\tKernel power management patch 10.11.6(2017-003 15G1611)(data3) found and patched\n");
			return true;
		}
		// sherlocks: change for 10.12 DP1
		else if (0x00003390000000E2ULL == (*((UInt64 *)Ptr)))
		{
			(*((UInt64 *)Ptr)) = 0x0000000000000000ULL;
			verbose("\tKernel power management patch 10.12 DP1 found and patched\n");
			return true;
		}
		// PMheart: change for 10.13 DP1 17A264c
		else if (0x00004000000000E2ULL == (*((UInt64 *)Ptr)))
		{
			(*((UInt64 *)Ptr)) = 0x0000000000000000ULL;
			verbose("\tKernel power management patch 10.13 DP1 found and patched\n");
			return true;
		}
		Ptr += 16;
	}
	verbose("\tKernel power management patch region not found!\n");
	return false;
}

// ===================================
// Bronya: Lapic Panic Version 64
bool patch_lapic_version_init_64(void *kernelData)  // KernelLapicVersionPatch_64
{
	UInt8       *bytes = (UInt8 *)kernelData;
	UInt32      patchLocation = 0;
	UInt32      i;

	verbose("\t- Looking for Lapic Version panic call Start\n");

	for (i = 0; i < 0x1000000; i++)
	{
		// Bronya: Snow Leopard 10.6 Lapic Version
		if (bytes[i + 0]     == 0x48
			&& bytes[i + 1]  == 0x0f
			&& bytes[i + 2]  == 0x44
			&& bytes[i + 3]  == 0xc2
			&& bytes[i + 57] == 0x31
			&& bytes[i + 58] == 0xc0)
		{
			patchLocation = i + 59;
			verbose("\tFound Snow Leopard Lapic panic at 0x%08X\n", (unsigned int)patchLocation);
			break;
		}
		// Bronya: Lion 10.7 Lapic Version
		else if (bytes[i + 0] == 0x48
			&& bytes[i + 1]   == 0x0f
			&& bytes[i + 2]   == 0x44
			&& bytes[i + 3]   == 0xc8
			&& bytes[i + 61]  == 0x30
			&& bytes[i + 62]  == 0xc0
			&& bytes[i + 63]  == 0xe8)
		{
			patchLocation = i + 63;
			verbose("\tFound Lion, Lapic Version panic at 0x%08X\n", (unsigned int)patchLocation);
			break;
		}
		// Bronya: Mountain Lion 10.8 Lapic Version
		else if (bytes[i + 0] == 0x45
			&& bytes[i + 1]   == 0x85
			&& bytes[i + 2]   == 0xf6
			&& bytes[i + 3]   == 0x48
			&& bytes[i + 4]   == 0x0f
			&& bytes[i + 5]   == 0x45
			&& bytes[i + 6]   == 0xc1
			&& bytes[i + 68]  == 0xe8
			&& bytes[i + 69]  == 0x3d
			&& bytes[i + 70]  == 0x15
			&& bytes[i + 71]  == 0xf6
			&& bytes[i + 72]  == 0xff)
		{
			patchLocation = i + 68;
			verbose("\tFound Mountain Lion, Lapic Version panic at 0x%08X\n", (unsigned int)patchLocation);
			break;
		}
		// Bronya: Mavericks 10.9 Lapic Version
		else if (bytes[i + 0] == 0xff
			&& bytes[i + 1]   == 0x50
			&& bytes[i + 2]   == 0x08
			&& bytes[i + 3]   == 0x89
			&& bytes[i + 4]   == 0xc3
			&& bytes[i + 90]  == 0xe8
			&& bytes[i + 91]  == 0x02
			&& bytes[i + 92]  == 0x17
			&& bytes[i + 93]  == 0xf4
			&& bytes[i + 94]  == 0xff)
		{
			patchLocation = i + 90;
			verbose("\tFound Mavericks Lapic Version panic at 0x%08X\n", (unsigned int)patchLocation);
			break;
		}
		// Bronya: Yosemite 10.10 lapic version
		else if (bytes[i + 0] == 0xff
			&& bytes[i + 1]   == 0x50
			&& bytes[i + 2]   == 0x08
			&& bytes[i + 38]  == 0x31
			&& bytes[i + 39]  == 0xdb
			&& bytes[i + 40]  == 0x31
			&& bytes[i + 41]  == 0xc0)
		{
			patchLocation = i + 42;
			verbose("\tFound Yosemite Lapic Version panic at 0x%08X\n", (unsigned int)patchLocation);
			break;
		}
		// Bronya: El Capitan 10.11 Lapic Version/10.12/10.13
		else if (bytes[i + 0] == 0xff
			&& bytes[i + 1]   == 0x50
			&& bytes[i + 2]   == 0x08
			&& bytes[i + 38]  == 0x31
			&& bytes[i + 39]  == 0xc0)
		{
			patchLocation = i + 40;	

			if (kernelOSVer >= MacOSVer2Int("10.13") && kernelOSVer < MacOSVer2Int("10.14"))
			{
				verbose("\tFound High Sierra Lapic Version panic at 0x%08X\n", (unsigned int)patchLocation);
			}
			else if (kernelOSVer >= MacOSVer2Int("10.12") && kernelOSVer < MacOSVer2Int("10.13"))
			{
				verbose("\tFound Sierra Lapic Version panic at 0x%08X\n", (unsigned int)patchLocation);
			}
			else
			{
				verbose("\tFound El Capitan Lapic Version panic at 0x%08X\n", (unsigned int)patchLocation);
			}
			
			break;
		}
	}

	if (!patchLocation)
	{
		verbose("\tCan't find Lapic Version panic, kernel patch aborted.\n");
		return false;
	}

	// Already patched?  May be running a non-vanilla kernel already?

	if (bytes[patchLocation + 0]    == 0x90
		&& bytes[patchLocation + 1] == 0x90
		&& bytes[patchLocation + 2] == 0x90
		&& bytes[patchLocation + 3] == 0x90
		&& bytes[patchLocation + 4] == 0x90)
	{
		verbose("\tLapic Version panic already patched, kernel file manually patched?\n");
		return false;
	}
	else
	{
		bytes[patchLocation + 0] = 0x90;
		bytes[patchLocation + 1] = 0x90;
		bytes[patchLocation + 2] = 0x90;
		bytes[patchLocation + 3] = 0x90;
		bytes[patchLocation + 4] = 0x90;
	}
	return true;
}

// ===================================
// Lapic Error Panic 64
bool patch_lapic_init_64(void *kernelData)  // KernelLapicPatch_64
{
	// Credits to donovan6000 and sherlocks for providing the lapic kernel patch source used to build this function

	UInt8       *bytes = (UInt8 *)kernelData;
	UInt32      patchLocation = 0;
	UInt32      i;

	verbose("\t- Looking for Lapic panic call Start\n");

	for (i = 0; i < 0x1000000; i++)
	{
		if (KernelLapicError
			&& (bytes[i + 0] == 0x65
			&& bytes[i + 1]  == 0x8B
			&& bytes[i + 2]  == 0x04
			&& bytes[i + 3]  == 0x25
			&& bytes[i + 4]  == 0x3C
			&& bytes[i + 5]  == 0x00
			&& bytes[i + 6]  == 0x00
			&& bytes[i + 7]  == 0x00
			&& bytes[i + 45] == 0x65
			&& bytes[i + 46] == 0x8B
			&& bytes[i + 47] == 0x04
			&& bytes[i + 48] == 0x25
			&& bytes[i + 49] == 0x3C
			&& bytes[i + 50] == 0x00
			&& bytes[i + 51] == 0x00
			&& bytes[i + 52] == 0x00))
		{
			patchLocation = i + 40;
			verbose("\tFound Snow Leopard Lapic panic at 0x%08X\n", (unsigned int)patchLocation);
			break;
		}
		else if (KernelLapicError
			&& (bytes[i + 0] == 0x65
			&& bytes[i + 1]  == 0x8B
			&& bytes[i + 2]  == 0x04
			&& bytes[i + 3]  == 0x25
			&& bytes[i + 4]  == 0x14
			&& bytes[i + 5]  == 0x00
			&& bytes[i + 6]  == 0x00
			&& bytes[i + 7]  == 0x00
			&& bytes[i + 35] == 0x65
			&& bytes[i + 36] == 0x8B
			&& bytes[i + 37] == 0x04
			&& bytes[i + 38] == 0x25
			&& bytes[i + 39] == 0x14
			&& bytes[i + 40] == 0x00
			&& bytes[i + 41] == 0x00
			&& bytes[i + 42] == 0x00))
		{
			patchLocation = i + 30;
			verbose("\tFound %sLion Lapic panic at 0x%08X\n", checkOSVersion("10.7") ? "" : "Mountain ", (unsigned int)patchLocation);
			break;
		}
		else if (KernelLapicError
			&& (bytes[i + 0] == 0x65
			&& bytes[i + 1]  == 0x8B
			&& bytes[i + 2]  == 0x04
			&& bytes[i + 3]  == 0x25
			&& bytes[i + 4]  == 0x1C
			&& bytes[i + 5]  == 0x00
			&& bytes[i + 6]  == 0x00
			&& bytes[i + 7]  == 0x00
			&& bytes[i + 36] == 0x65
			&& bytes[i + 37] == 0x8B
			&& bytes[i + 38] == 0x04
			&& bytes[i + 39] == 0x25
			&& bytes[i + 40] == 0x1C
			&& bytes[i + 41] == 0x00
			&& bytes[i + 42] == 0x00
			&& bytes[i + 43] == 0x00))
		{
			patchLocation = i + 31;
			verbose("\tFound Mavericks Lapic panic at 0x%08X\n", (unsigned int)patchLocation);
			break;
		}
                // RehabMan: 10.10.DP1 lapic
		else if (KernelLapicError
			&& (bytes[i + 0] == 0x65
			&& bytes[i + 1]  == 0x8B
			&& bytes[i + 2]  == 0x04
			&& bytes[i + 3]  == 0x25
			&& bytes[i + 4]  == 0x1C
			&& bytes[i + 5]  == 0x00
			&& bytes[i + 6]  == 0x00
			&& bytes[i + 7]  == 0x00
			&& bytes[i + 33] == 0x65
			&& bytes[i + 34] == 0x8B
			&& bytes[i + 35] == 0x04
			&& bytes[i + 36] == 0x25
			&& bytes[i + 37] == 0x1C
			&& bytes[i + 38] == 0x00
			&& bytes[i + 39] == 0x00
			&& bytes[i + 40] == 0x00))
		{
			patchLocation = i + 28;
			verbose("\tFound Yosemite Lapic panic at 0x%08X\n", (unsigned int)patchLocation);
			break;
		}
		// sherlocks: 10.11.DB1
		else if (KernelLapicError
			&& (bytes[i + 0]   == 0x65
			&& bytes[i + 1]    == 0x8B
			&& bytes[i + 2]    == 0x0C
			&& bytes[i + 3]    == 0x25
			&& bytes[i + 4]    == 0x1C
			&& bytes[i + 5]    == 0x00
			&& bytes[i + 6]    == 0x00
			&& bytes[i + 7]    == 0x00
			&& bytes[i + 1411] == 0x65
			&& bytes[i + 1412] == 0x8B
			&& bytes[i + 1413] == 0x0C
			&& bytes[i + 1414] == 0x25
			&& bytes[i + 1415] == 0x1C
			&& bytes[i + 1416] == 0x00
			&& bytes[i + 1417] == 0x00
			&& bytes[i + 1418] == 0x00))
		{
			patchLocation = i + 1400;
			verbose("\tFound El Capitan Lapic panic at 0x%08X\n", (unsigned int)patchLocation);
			break;
		}
		// sherlocks: 10.12.DP1
		else if (KernelLapicError
			&& (bytes[i+0]   == 0x65
			&& bytes[i+1]    == 0x8B
			&& bytes[i+2]    == 0x0C
			&& bytes[i+3]    == 0x25
			&& bytes[i+4]    == 0x1C
			&& bytes[i+5]    == 0x00
			&& bytes[i+6]    == 0x00
			&& bytes[i+7]    == 0x00
			&& bytes[i+1409] == 0x65
			&& bytes[i+1410] == 0x8B
			&& bytes[i+1411] == 0x0C
			&& bytes[i+1412] == 0x25
			&& bytes[i+1413] == 0x1C
			&& bytes[i+1414] == 0x00
			&& bytes[i+1415] == 0x00
			&& bytes[i+1416] == 0x00))
		{
			patchLocation = i+1398;
			DBG("\tFound Sierra Lapic panic at 0x%08X\n", (unsigned int)patchLocation);
			break;
		}
		// PMheart: 10.13.DP1
		else if (KernelLapicError
			&& (bytes[i+0]   == 0x65
			&& bytes[i+1]    == 0x8B
			&& bytes[i+2]    == 0x0C
			&& bytes[i+3]    == 0x25
			&& bytes[i+4]    == 0x1C
			&& bytes[i+5]    == 0x00
			&& bytes[i+6]    == 0x00
			&& bytes[i+7]    == 0x00
			&& bytes[i+1407] == 0x65
			&& bytes[i+1408] == 0x8B
			&& bytes[i+1409] == 0x0C
			&& bytes[i+1410] == 0x25
			&& bytes[i+1411] == 0x1C
			&& bytes[i+1412] == 0x00
			&& bytes[i+1413] == 0x00
			&& bytes[i+1414] == 0x00))
		{
			patchLocation = i+1396;
			DBG("\tFound High Sierra Lapic panic at 0x%08X\n", (unsigned int)patchLocation);
			break;
		}
	}

	if (!patchLocation)
	{
		verbose("\tCan't find Lapic panic, kernel patch aborted.\n");
		return false;
	}

	// Already patched?  May be running a non-vanilla kernel already?

	if (bytes[patchLocation + 0]    == 0x90
		&& bytes[patchLocation + 1] == 0x90
		&& bytes[patchLocation + 2] == 0x90
		&& bytes[patchLocation + 3] == 0x90
		&& bytes[patchLocation + 4] == 0x90)
	{
		verbose("\tLapic panic already patched, kernel file manually patched?\n");
		return false;
	}
	else
	{
		bytes[patchLocation + 0] = 0x90;
		bytes[patchLocation + 1] = 0x90;
		bytes[patchLocation + 2] = 0x90;
		bytes[patchLocation + 3] = 0x90;
		bytes[patchLocation + 4] = 0x90;
	}
	return true;
}

// ===================================
// Bronya: Lapic Panic Version 32 bit
bool patch_lapic_version_init_32(void *kernelData)
{
	UInt8       *bytes = (UInt8 *)kernelData;
	UInt32      patchLocation = 0;
	UInt32      i;

	verbose("\t- Looking for Lapic Version panic call Start\n");

	for (i = 0; i < 0x1000000; i++)
	{
		// Bronya: Snow Leopard 10.6 Lapic Version
		if (bytes[i + 0]     == 0x0f
			&& bytes[i + 1]  == 0x44
			&& bytes[i + 2]  == 0xc2
			&& bytes[i + 49] == 0x89
			&& bytes[i + 50] == 0x44
			&& bytes[i + 51] == 0x24
			&& bytes[i + 52] == 0x04)
		{
			patchLocation = i+60;
			verbose("\tFound Snow Leopard Lapic Version panic at 0x%08X\n", (unsigned int)patchLocation);
			break;
		}
		else
		{
			// Bronya: Lion 10.7 Lapic Version
			if (bytes[i + 0]     == 0x0f
				&& bytes[i + 1]  == 0x44
				&& bytes[i + 2]  == 0xc8
				&& bytes[i + 52] == 0x89
				&& bytes[i + 53] == 0x44
				&& bytes[i + 54] == 0x24
				&& bytes[i + 55] == 0x04)
			{
				patchLocation = i+63;
				verbose("\tFound Lion, Lion Lapic Version panic at 0x%08X\n", (unsigned int)patchLocation);
				break;
			}
		}
	}

	if (!patchLocation)
	{
		verbose("\tCan't find Lapic Version panic, kernel patch aborted.\n");
		return false;
	}

	// Already patched?  May be running a non-vanilla kernel already?

	if (bytes[patchLocation + 0]    == 0x90
		&& bytes[patchLocation + 1] == 0x90
		&& bytes[patchLocation + 2] == 0x90
		&& bytes[patchLocation + 3] == 0x90
		&& bytes[patchLocation + 4] == 0x90)
	{
		verbose("\tLapic Version panic already patched, kernel file manually patched?\n");
		return false;
	}
	else
	{
		bytes[patchLocation + 0] = 0x90;
		bytes[patchLocation + 1] = 0x90;
		bytes[patchLocation + 2] = 0x90;
		bytes[patchLocation + 3] = 0x90;
		bytes[patchLocation + 4] = 0x90;
	}
	return true;
}

// ===================================
// Lapic Error Panic 32
bool patch_lapic_init_32(void *kernelData) // KernelLapicPatch_32
{
	// Credits to donovan6000 and sherlocks for providing the lapic kernel patch source used to build this function

	UInt8       *bytes = (UInt8 *)kernelData;
	UInt32      patchLocation=0;
	UInt32      i;

	verbose("\t- Looking for Lapic panic call Start\n");

	for (i = 0; i < 0x1000000; i++)
	{
		if (bytes[i+0]     == 0x65
			&& bytes[i+1]  == 0xA1
			&& bytes[i+2]  == 0x0C
			&& bytes[i+3]  == 0x00
			&& bytes[i+4]  == 0x00
			&& bytes[i+5]  == 0x00
			&& bytes[i+30] == 0x65
			&& bytes[i+31] == 0xA1
			&& bytes[i+32] == 0x0C
			&& bytes[i+33] == 0x00
			&& bytes[i+34] == 0x00
			&& bytes[i+35] == 0x00)
		{
			patchLocation = i + 25;
			verbose("\tFound Lapic panic at 0x%08X\n", (unsigned int)patchLocation);
			break;
		}
	}

	if (!patchLocation)
	{
		verbose("\tCan't find Lapic panic, kernel patch aborted.\n");
		return false;
	}

	// Already patched?  May be running a non-vanilla kernel already?

	if (bytes[patchLocation + 0]    == 0x90
		&& bytes[patchLocation + 1] == 0x90
		&& bytes[patchLocation + 2] == 0x90
		&& bytes[patchLocation + 3] == 0x90
		&& bytes[patchLocation + 4] == 0x90)
	{
		verbose("\tLapic panic already patched, kernel file manually patched?\n");
		return false;
	}
	else
	{
		bytes[patchLocation + 0] = 0x90;
		bytes[patchLocation + 1] = 0x90;
		bytes[patchLocation + 2] = 0x90;
		bytes[patchLocation + 3] = 0x90;
		bytes[patchLocation + 4] = 0x90;
	}
	return true;
}

// ===================================
// Haswell-E Patch
bool patch_haswell_E_init(void *kernelData) // KernelHaswellEPatch
{
	// Credit to stinga11 for the patches used below
	// Based on Pike R. Alpha's Haswell patch for Mavericks

	UInt8	*Bytes;
	UInt32	Index;
	bool	PatchApplied;

	verbose("\t- Searching for Haswell-E patch pattern\n");

	Bytes = (UInt8 *)kernelData;
	PatchApplied = false;

	for (Index = 0; Index < 0x1000000; ++Index)
	{
		// sudo perl -pi -e 's|\x74\x11\x83\xF8\x3C|\x74\x11\x83\xF8\x3F|g' /System/Library/Kernels/kernel
		if (Bytes[Index]        == 0x74
			&& Bytes[Index + 1] == 0x11
			&& Bytes[Index + 2] == 0x83
			&& Bytes[Index + 3] == 0xF8
			&& Bytes[Index + 4] == 0x3C)
		{
			Bytes[Index + 4] = 0x3F;

			verbose("\tFound Haswell-E pattern #1; patched.\n");

			if (PatchApplied)
			{
				break;
			}

			PatchApplied = true;
		}

		// sudo perl -pi -e 's|\xEB\x0A\x83\xF8\x3A|\xEB\x0A\x83\xF8\x3F|g' /System/Library/Kernels/kernel
		if (Bytes[Index]        == 0xEB
			&& Bytes[Index + 1] == 0x0A
			&& Bytes[Index + 2] == 0x83
			&& Bytes[Index + 3] == 0xF8
			&& Bytes[Index + 4] == 0x3A)
		{
			Bytes[Index + 4] = 0x3F;

			verbose("\tFound Haswell-E pattern #2; patched.\n");

			if (PatchApplied)
			{
				break;
			}

			PatchApplied = true;
		}
	}

	if (!PatchApplied)
	{
		verbose("\tCan't find Haswell-E patch pattern, patch not applied.\n");
	}

	return PatchApplied;
}

// ===================================
// Haswell-ULT Patch
bool patch_haswell_ULT_init(void *kernelData) // Fake CPUFAMILY To IVYBRIDGE Patch
{
	// Credit to Tora Chi Yo for the patches used below
	// http://www.insanelymac.com/forum/topic/307721-haswell-ult-kernel-patch-for-yosemite-mavericks

	UInt8	*Bytes;
	UInt32	Index;
	bool	PatchApplied;

	verbose("\t- Searching for Haswell-ULT patch pattern\n");

	Bytes = (UInt8 *)kernelData;
	PatchApplied = false;

	for (Index = 0; Index < 0x1000000; ++Index)
	{
		// sudo perl -pi -e 's|\xbb\xdc\x82\xb2\xdc|\xbb\x35\xe8\x65\x1f|g' /System/Library/Kernels/kernel
		if (Bytes[Index]        == 0xBB
			&& Bytes[Index + 1] == 0xDC
			&& Bytes[Index + 2] == 0x82
			&& Bytes[Index + 3] == 0xB2
			&& Bytes[Index + 4] == 0xdc)
		{
			Bytes[Index + 1] = 0x35;
			Bytes[Index + 2] = 0xE8;
			Bytes[Index + 3] = 0x65;
			Bytes[Index + 4] = 0x1F;

			verbose("\tFound Haswell-ULT pattern #1; patched.\n");

			if (PatchApplied)
			{
				break;
			}

			PatchApplied = true;
		}
	}

	if (!PatchApplied)
	{
		verbose("\tCan't find Haswell-ULT patch pattern, patch not applied.\n");
	}

	return PatchApplied;
}

// ===================================
// Custom Kext injection from Extra/Extensions folder
void patch_BooterExtensions_32(void *kernelData)
{
	// KernelBooterExtensionsPatch to load extra kexts besides kernelcache
	UInt8   *Bytes;
	UInt32  Index;
	bool PatchApplied;
	int count = 0;

	verbose("\t- Searching for booter extensions pattern:\n");

	Bytes = (UInt8 *)kernelData;
	PatchApplied = false;

	if (kernelOSVer >= MacOSVer2Int("10.7") && kernelOSVer < MacOSVer2Int("10.8"))
	{
	//UInt8   KBELionSearch_i386[]   = { 0xE8, 0xAA, 0xFB, 0xFF, 0xFF, 0xEB, 0x08, 0x89, 0x34, 0x24 };
	//UInt8   KBELionReplace_i386[]  = { 0xE8, 0xAA, 0xFB, 0xFF, 0xFF, 0x90, 0x90, 0x89, 0x34, 0x24 };
		for (Index = 0; Index < 0x1000000; ++Index)
		{
			if (Bytes[Index]        == 0xE8
				&& Bytes[Index + 1] == 0xAA
				&& Bytes[Index + 2] == 0xFB
				&& Bytes[Index + 3] == 0xFF
				&& Bytes[Index + 4] == 0xFF
				&& Bytes[Index + 5] == 0xEB
				&& Bytes[Index + 6] == 0x08
				&& Bytes[Index + 7] == 0x89
				&& Bytes[Index + 8] == 0x34
				&& Bytes[Index + 9] == 0x24)
			{
				Bytes[Index + 5] = 0x90;
				Bytes[Index + 6] = 0x90;
				count ++;

				verbose("\tFound Lion pattern: patched!\n");

				if (PatchApplied)
				{
					break;
				}

				PatchApplied = true;
			}
		}
	}

	/* UNDER REVIEW, crazybirdy test this patch on Snow leopard:
	 1) He has reported has not working, but he runs the /S/L/E/Extensions.mkext: this patch allow booter extensions, so no reason to use that!
	 2) He try to load /Extra/Extensions, but the bootloader have a bug (after the RecoveryHD code inserted) and does not load the kernelcache and scan SLE

	 need to re-try later.
	 */
	if (kernelOSVer >= MacOSVer2Int("10.6") && kernelOSVer < MacOSVer2Int("10.7"))
	{
		//UInt8   KBESnowSearch_i386[]   = { 0xE8, 0xED, 0xF9, 0xFF, 0xFF, 0xEB, 0x08, 0x89, 0x1C, 0x24 };
		//UInt8   KBESnowReplace_i386[]  = { 0xE8, 0xED, 0xF9, 0xFF, 0xFF, 0x90, 0x90, 0x89, 0x1C, 0x24 };
		for (Index = 0; Index < 0x1000000; ++Index)
		{
			if (Bytes[Index]        == 0xE8
				&& Bytes[Index + 1] == 0xED
				&& Bytes[Index + 2] == 0xF9
				&& Bytes[Index + 3] == 0xFF
				&& Bytes[Index + 4] == 0xFF
				&& Bytes[Index + 5] == 0xEB
				&& Bytes[Index + 6] == 0x08
				&& Bytes[Index + 7] == 0x89
				&& Bytes[Index + 8] == 0x1C
				&& Bytes[Index + 9] == 0x24)
			{
				Bytes[Index + 5] = 0x90;
				Bytes[Index + 6] = 0x90;
				count ++;

				verbose("\tFound Snow Leopard pattern: patched!\n");

				if (PatchApplied)
				{
					break;
				}

				PatchApplied = true;
			}
		}
	}

	if (!PatchApplied)
	{
		verbose("\tCan't find Booter Extensions patch location.\n");
	}
	else
	{
		verbose("\t%d substitution(s) made.\n", count);
	}
}

void patch_BooterExtensions_64(void *kernelData)
{
	// KernelBooterExtensionsPatch to load extra kexts besides kernelcache
	UInt8   *Bytes;
	UInt32  Index;
	bool PatchApplied; // does nothing
	int count = 0;

	verbose("\t- Searching for booter extensions pattern:\n");

	Bytes = (UInt8 *)kernelData;
	PatchApplied = false;

	// High Sierra onward, need to use 10.12 instead of 10.13. kernel bug?
	// if (kernelOSVer >= MacOSVer2Int("10.13") && kernelOSVer < MacOSVer2Int("10.14"))
	if (kernelOSVer >= MacOSVer2Int("10.12"))
	{
		for (Index = 0; Index < 0x1000000; ++Index)
		{
			// High Sierra
			if (Bytes[Index]         == 0xC3
				&& Bytes[Index + 1]  == 0x48
				&& Bytes[Index + 2]  == 0x85
				&& Bytes[Index + 3]  == 0xDB
				&& Bytes[Index + 4]  == 0x74
				&& Bytes[Index + 5]  == 0x69
				&& Bytes[Index + 6]  == 0x48
				&& Bytes[Index + 7]  == 0x8B
				&& Bytes[Index + 8]  == 0x03
				&& Bytes[Index + 9]  == 0x48
				&& Bytes[Index + 10] == 0x89
				&& Bytes[Index + 11] == 0xDF
				&& Bytes[Index + 12] == 0xFF
				&& Bytes[Index + 13] == 0x50
				&& Bytes[Index + 14] == 0x28
				&& Bytes[Index + 15] == 0x48)
			{
				Bytes[Index + 4] = 0xEB;
				Bytes[Index + 5] = 0x12;
				count++;

				verbose("\tFound High Sierra SIP pattern: patched!\n");

				if (PatchApplied)
				{
					break;
				}

				PatchApplied = true;
			}
		}
	}

	// Sierra
	if (kernelOSVer >= MacOSVer2Int("10.12") && kernelOSVer < MacOSVer2Int("10.13"))
	{
		for (Index = 0; Index < 0x1000000; ++Index)
		{
			// High Sierra
			if (Bytes[Index]         == 0xC3
				&& Bytes[Index + 1]  == 0x48
				&& Bytes[Index + 2]  == 0x85
				&& Bytes[Index + 3]  == 0xDB
				&& Bytes[Index + 4]  == 0x74
				&& Bytes[Index + 5]  == 0x71
				&& Bytes[Index + 6]  == 0x48
				&& Bytes[Index + 7]  == 0x8B
				&& Bytes[Index + 8]  == 0x03
				&& Bytes[Index + 9]  == 0x48
				&& Bytes[Index + 10] == 0x89
				&& Bytes[Index + 11] == 0xDF
				&& Bytes[Index + 12] == 0xFF
				&& Bytes[Index + 13] == 0x50
				&& Bytes[Index + 14] == 0x28
				&& Bytes[Index + 15] == 0x48)
			{
				Bytes[Index + 4] = 0xEB;
				Bytes[Index + 5] = 0x12;
				count++;

				verbose("\tFound Sierra SIP pattern: patched!\n");

				if (PatchApplied)
				{
					break;
				}

				PatchApplied = true;
			}
		}
	}

	// Yosemite/El Capitan
	if (kernelOSVer >= MacOSVer2Int("10.10") && kernelOSVer < MacOSVer2Int("10.12"))
	{
		for (Index = 0; Index < 0x1000000; ++Index)
		{
			// El Capitan
			if (Bytes[Index]         == 0xC3
				&& Bytes[Index + 1]  == 0x48
				&& Bytes[Index + 2]  == 0x85
				&& Bytes[Index + 3]  == 0xDB
				&& Bytes[Index + 4]  == 0x74
				&& Bytes[Index + 5]  == 0x70
				&& Bytes[Index + 6]  == 0x48
				&& Bytes[Index + 7]  == 0x8B
				&& Bytes[Index + 8]  == 0x03
				&& Bytes[Index + 9]  == 0x48
				&& Bytes[Index + 10] == 0x89
				&& Bytes[Index + 11] == 0xDF
				&& Bytes[Index + 12] == 0xFF
				&& Bytes[Index + 13] == 0x50
				&& Bytes[Index + 14] == 0x28
				&& Bytes[Index + 15] == 0x48)
			{
				Bytes[Index + 4] = 0xEB;
				Bytes[Index + 5] = 0x12;
				count++;

				if (kernelOSVer >= MacOSVer2Int("10.11"))
				{
					verbose("\tFound El Capitan SIP pattern: patched!\n");
				}
				else
				{
					verbose("\tFound Yosemite SIP pattern: patched!\n");
				}

				if (PatchApplied)
				{
					break;
				}

				PatchApplied = true;
			}
		}
	}

	// Yosemite onward.
	// Yosemite/EL Capitan/Sierra/High Sierra
	// if (kernelOSVer >= MacOSVer2Int("10.10") && kernelOSVer < MacOSVer2Int("10.14"))
	if (kernelOSVer >= MacOSVer2Int("10.10"))
	{

		for (Index = 0; Index < 0x1000000; ++Index)
		{
			if (Bytes[Index]         == 0xE8
				&& Bytes[Index + 1]  == 0x25
				&& Bytes[Index + 2]  == 0x00
				&& Bytes[Index + 3]  == 0x00
				&& Bytes[Index + 4]  == 0x00
				&& Bytes[Index + 5]  == 0xEB
				&& Bytes[Index + 6]  == 0x05
				&& Bytes[Index + 7]  == 0xE8)
			{
				Bytes[Index + 5] = 0x90;
				Bytes[Index + 6] = 0x90;
				count++;
				
				if (kernelOSVer >= MacOSVer2Int("10.13") && kernelOSVer < MacOSVer2Int("10.14"))
				{
					verbose("\tFound High Sierra EXT pattern: patched!\n");
				}
				else if (kernelOSVer >= MacOSVer2Int("10.12") && kernelOSVer < MacOSVer2Int("10.13"))
				{
					verbose("\tFound Sierra EXT pattern: patched!\n");
				}
				else if (kernelOSVer >= MacOSVer2Int("10.11") && kernelOSVer < MacOSVer2Int("10.12"))
				{
					verbose("\tFound EL Capitan EXT pattern: patched!\n");
				}
				else
				{
					verbose("\tFound Yosemite EXT pattern: patched!\n");
				}

				if (PatchApplied)
				{
					break;
				}

				PatchApplied = true;
			}
		}
	}

	// Mountain Lion/Mavericks
	if (kernelOSVer >= MacOSVer2Int("10.8") && kernelOSVer < MacOSVer2Int("10.10"))
	{

		for (Index = 0; Index < 0x1000000; ++Index)
		{
			if (Bytes[Index]         == 0xC6
				&& Bytes[Index + 1]  == 0xE8
				&& Bytes[Index + 2]  == 0x30
				&& Bytes[Index + 3]  == 0x00
				&& Bytes[Index + 4]  == 0x00
				&& Bytes[Index + 5]  == 0x00
				&& Bytes[Index + 6]  == 0xEB
				&& Bytes[Index + 7]  == 0x08
				&& Bytes[Index + 8]  == 0x48
				&& Bytes[Index + 9]  == 0x89
				&& Bytes[Index + 10] == 0xDF)
			{
				Bytes[Index + 6] = 0x90;
				Bytes[Index + 7] = 0x90;
				count++;

				verbose("\tFound M%s EXT pattern: patched!\n", checkOSVersion("10.8") ? "ountain Lion" : "avericks");

				if (PatchApplied)
				{
					break;
				}

				PatchApplied = true;
			}
		}
	}

	// Lion 64
	if (kernelOSVer >= MacOSVer2Int("10.7") && kernelOSVer < MacOSVer2Int("10.8"))
	{
		for (Index = 0; Index < 0x1000000; ++Index)
		{
			if (Bytes[Index]        == 0xE8
				&& Bytes[Index + 1] == 0x0C
				&& Bytes[Index + 2] == 0xFD
				&& Bytes[Index + 3] == 0xFF
				&& Bytes[Index + 4] == 0xFF
				&& Bytes[Index + 5] == 0xEB
				&& Bytes[Index + 6] == 0x08
				&& Bytes[Index + 7] == 0x48
				&& Bytes[Index + 8] == 0x89
				&& Bytes[Index + 9] == 0xDF)
			{
				Bytes[Index + 5] = 0x90;
				Bytes[Index + 6] = 0x90;
				count++;

				verbose("\tFound Lion EXT pattern: patched!\n");

				if (PatchApplied)
				{
					break;
				}

				PatchApplied = true;
			}
		}
	}

	// Snow Leopard 64
	if (kernelOSVer >= MacOSVer2Int("10.6") && kernelOSVer < MacOSVer2Int("10.7"))
	{
		for (Index = 0; Index < 0x1000000; ++Index)
		{
			if (Bytes[Index]        == 0xE8
				&& Bytes[Index + 1] == 0x5A
				&& Bytes[Index + 2] == 0xFB
				&& Bytes[Index + 3] == 0xFF
				&& Bytes[Index + 4] == 0xFF
				&& Bytes[Index + 5] == 0xEB
				&& Bytes[Index + 6] == 0x08
				&& Bytes[Index + 7] == 0x48
				&& Bytes[Index + 8] == 0x89
				&& Bytes[Index + 9] == 0xDF)
			{
				Bytes[Index + 5] = 0x90;
				Bytes[Index + 6] = 0x90;
				verbose("\tFound Snow Leopard EXT pattern: patched!\n");
				count++;

				if (PatchApplied)
				{
					break;
				}

				PatchApplied = true;
			}
		}
	}

	if (!PatchApplied)
	{
		verbose("\tCan't find Booter Extensions patch location.\n");
	}
	else
	{
		verbose("\t%d substitution(s) made.\n", count);
	}
}

// ===================================
// Patch ssse3
void patch_SSE3_6(void *kernelData)
{
	UInt8 *bytes = (UInt8 *)kernelData;
	UInt32 patchLocation1 = 0;
	UInt32 patchLocation2 = 0;
	UInt32 patchlast = 0;
	UInt32 i;
	//UInt32 Length = sizeof(kernelData);

	verbose("\t- Start find SSE3 address\n");
	i = 0;
	//for (i = 0; i < Length; i++)
	while(true)
	{
		if (bytes[i]                == 0x66
			&& bytes[i + 1]         == 0x0F
			&& bytes[i + 2]         == 0x6F
			&& bytes[i + 3]         == 0x44
			&& bytes[i + 4]         == 0x0E
			&& bytes[i + 5]         == 0xF1
			&& bytes[i - 1664 - 32] == 0x55)
		{
			patchLocation1 = i-1664-32;
			verbose("\tFound SSE3 data address at 0x%08X\n", (unsigned int)patchLocation1);
		}

		// hasSSE2+..... title
		if (bytes[i]        == 0xE3
			&& bytes[i + 1] == 0x07
			&& bytes[i + 2] == 0x00
			&& bytes[i + 3] == 0x00
			&& bytes[i + 4] == 0x80
			&& bytes[i + 5] == 0x07
			&& bytes[i + 6] == 0xFF
			&& bytes[i + 7] == 0xFF
			&& bytes[i + 8] == 0x24
			&& bytes[i + 9] == 0x01)
		{
			patchLocation2 = i;
			verbose("\tFound SSE3 Title address at 0x%08X\n", (unsigned int)patchLocation2);
			break;
		}
		i++;
	}

	if (!patchLocation1 || !patchLocation2)
	{
		verbose("\tCan't found SSE3 data addres or Title address at 0x%08X 0x%08X\n", (unsigned int)patchLocation1, (unsigned int)patchLocation2);
		return;
	}

	verbose("\tFound SSE3 last data addres Start\n");
	i = patchLocation1 + 1500;
	//for (i=(patchLocation1+1500); i<(patchLocation1+3000); i++)
	while(true)
	{
		if (bytes[i]        == 0x90
			&& bytes[i + 1] == 0x90
			&& bytes[i + 2] == 0x55 )
		{
			patchlast = (i+1) - patchLocation1;
			verbose("\tFound SSE3 last data addres at 0x%08X\n",  (unsigned int)patchlast);
			break;
		}
		i++;
	}

	if (!patchlast)
	{
		verbose("\tCan't found SSE3 data last addres at 0x%08X\n",  (unsigned int)patchlast);
		return;
	}
	// patch sse3_64 data

	for (i = 0; i < patchlast; i++)
	{
		if (i < sizeof(sse3_patcher))
		{
			bytes[patchLocation1 + i] = sse3_patcher[i];
		}
		else
		{
			bytes[patchLocation1 + i] = 0x90;
		}
	}

	// patch kHasSSE3 title
	bytes[patchLocation2 + 0] = 0xFC;
	bytes[patchLocation2 + 1] = 0x05;
	bytes[patchLocation2 + 8] = 0x2C;
	bytes[patchLocation2 + 9] = 0x00;

}

// ===================================
//
void patch_SSE3_5(void *kernelData)
{
	UInt8 *bytes = (UInt8 *)kernelData;
	UInt32 patchLocation1 = 0;
	UInt32 patchLocation2 = 0;
	UInt32 patchlast=0;
	UInt32 Length = sizeof(kernelData);
	UInt32 i;

	verbose("\t- Start find SSE3 address\n");

	for (i = 256; i < (Length-256); i++)
	{
		if (bytes[i]                == 0x66
			&& bytes[i + 1]         == 0x0F
			&& bytes[i + 2]         == 0x6F
			&& bytes[i + 3]         == 0x44
			&& bytes[i + 4]         == 0x0E
			&& bytes[i + 5]         == 0xF1
			&& bytes[i - 1680 - 32] == 0x55)
		{
			patchLocation1 = i-1680-32;
			verbose("\tFound SSE3 data address at 0x%08X\n",  (unsigned int)patchLocation1);
		}

		// khasSSE2+..... title
		if (bytes[i]        == 0xF3
			&& bytes[i + 1] == 0x07
			&& bytes[i + 2] == 0x00
			&& bytes[i + 3] == 0x00
			&& bytes[i + 4] == 0x80
			&& bytes[i + 5] == 0x07
			&& bytes[i + 6] == 0xFF
			&& bytes[i + 7] == 0xFF
			&& bytes[i + 8] == 0x24
			&& bytes[i + 9] == 0x01)
		{
			patchLocation2 = i;
			verbose("\tFound SSE3 Title address at 0x%08X\n",  (unsigned int)patchLocation2);
			break;
		}
	}

	if (!patchLocation1 || !patchLocation2)
	{
		verbose("\tCan't found SSE3 data addres or Title address at 0x%08X 0x%08X\n",  (unsigned int)patchLocation1,  (unsigned int)patchLocation2);
		return;
	}

	verbose("\tFound SSE3 last data addres Start\n");

	for (i = (patchLocation1+1500);i < Length;i++)
	{
		if (bytes[i]        == 0x90
			&& bytes[i + 1] == 0x90
			&& bytes[i + 2] == 0x55)
		{
			patchlast = (i+1) - patchLocation1;
			verbose("\tFound SSE3 last data addres at 0x%08X\n",  (unsigned int)patchlast);
			break;
		}
	}

	if (!patchlast)
	{
		verbose("\tCan't found SSE3 data last addres at 0x%08X\n",  (unsigned int)patchlast);
		return;
	}

	// patech sse3_64 data

	for (i = 0; i < patchlast; i++)
	{
		if (i < sizeof(sse3_5_patcher))
		{
			bytes[patchLocation1 + i] = sse3_5_patcher[i];
		}
		else
		{
			bytes[patchLocation1 + i] = 0x90;
		}
	}

	// patch kHasSSE3 title
	bytes[patchLocation2 + 0] = 0x0C;
	bytes[patchLocation2 + 1] = 0x06;
	bytes[patchLocation2 + 8] = 0x2C;
	bytes[patchLocation2 + 9] = 0x00;

}

// ===================================
//
void patch_SSE3_7(void *kernelData)
{
	// not support yet
	return;
}

// ===================================
// root:xnu text replacement Patch
bool patch_string_XNU_init(void *kernelData) //
{

	UInt8	*Bytes;
	UInt32	Index;
	bool	PatchApplied;

	DBG("\t- Searching for \"root:xnu-\" string pattern\n");

	Bytes = (UInt8 *)kernelData;
	PatchApplied = false;

	for (Index = 0; Index < 0x1000000; ++Index)
	{
		// search for root:xnu- and replace it with ***Enoch-
		if (Bytes[Index]            == ' '
			&& Bytes[Index + 1] == 'r'
			&& Bytes[Index + 2] == 'o'
			&& Bytes[Index + 3] == 'o'
			&& Bytes[Index + 4] == 't'
			&& Bytes[Index + 5] == ':'
			&& Bytes[Index + 6] == 'x'
			&& Bytes[Index + 7] == 'n'
			&& Bytes[Index + 8] == 'u'
			&& Bytes[Index + 9] == '-' )
		{
			Bytes[Index + 1] = '*';
			Bytes[Index + 2] = '*';
			Bytes[Index + 3] = '*';
			Bytes[Index + 4] = 'E';
			Bytes[Index + 5] = 'n';
			Bytes[Index + 6] = 'o';
			Bytes[Index + 7] = 'c';
			Bytes[Index + 8] = 'h';

			DBG("\tFound \"root:xnu-\" pattern; patched.\n");

			if (PatchApplied)
			{
				break;
			}

			PatchApplied = true;
		}
	}

	if (!PatchApplied)
	{
		DBG("\tCan't find \"root:xnu-\" string pattern: patch not applied.\n");
	}

	return PatchApplied;
}

// ===================================
// (Clover)
// Patching AppleRTC to prevent CMOS reset
unsigned int AppleRTC_Patch(void *data, UInt32 DriverSize, UInt32 StartLocation)
{
    unsigned int count = 0;
    // as far as I know this works from Lion to Sierra
    UInt8   LionSearch_X64[]   = { 0x75, 0x30, 0x44, 0x89, 0xf8 };
    UInt8   LionReplace_X64[]  = { 0xeb, 0x30, 0x44, 0x89, 0xf8 };
    
    UInt8   LionSearch_i386[]  = { 0x75, 0x3d, 0x8b, 0x75, 0x08 };
    UInt8   LionReplace_i386[] = { 0xeb, 0x3d, 0x8b, 0x75, 0x08 };
    
    UInt8   MLSearch[]         = { 0x75, 0x30, 0x89, 0xd8 };
    UInt8   MLReplace[]        = { 0xeb, 0x30, 0x89, 0xd8 };
    //SunKi
    //752e0fb6 -> eb2e0fb6
    UInt8   MavSearch[]        = { 0x75, 0x2e, 0x0f, 0xb6 };
    UInt8   MavReplace[]       = { 0xeb, 0x2e, 0x0f, 0xb6};
    
    if (kernelOSVer >= MacOSVer2Int("10.7") && kernelOSVer < MacOSVer2Int("10.8"))
    {
        count = FindAndReplace(data,
                               DriverSize,
                               StartLocation,
                               LionSearch_i386,
                               sizeof(LionSearch_i386),
                               LionReplace_i386,
                               0);
        
        count = count + FindAndReplace(data,
                                       DriverSize,
                                       StartLocation,
                                       LionSearch_X64,
                                       sizeof(LionSearch_X64),
                                       LionReplace_X64,
                                       0);
    }
    else
        if (kernelOSVer >= MacOSVer2Int("10.8") && kernelOSVer < MacOSVer2Int("10.9"))
        {
            count = FindAndReplace(data,
                                   DriverSize,
                                   StartLocation,
                                   MLSearch,
                                   sizeof(MLSearch),
                                   MLReplace,
                                   0);
        }
        else
            if (kernelOSVer >= MacOSVer2Int("10.9"))
            {
                count = FindAndReplace(data,
                                       DriverSize,
                                       StartLocation,
                                       MavSearch,
                                       sizeof(MavSearch),
                                       MavReplace,
                                       0);
            }
    
    return count;
}

// ===================================
// (Clover)
// Patching AppleIntelCPUPowerManagement
unsigned int AsusAICPUPMPatch(void *data, UInt32 DriverSize, UInt32 StartLocation)
{
    unsigned int   Index1;
    unsigned int   Index2;
    unsigned int   Count = 0;
    
    UInt8   MovlE2ToEcx[] = { 0xB9, 0xE2, 0x00, 0x00, 0x00 };
    UInt8   MovE2ToCx[]   = { 0x66, 0xB9, 0xE2, 0x00 };
    UInt8   Wrmsr[]       = { 0x0F, 0x30 };
    
    UInt8	*Driver = (UInt8 *)data;
    
    //TODO: we should scan only __text __TEXT
    for (Index1 = StartLocation; Index1 < (StartLocation+DriverSize); Index1++)
    {
        // search for MovlE2ToEcx
        if (memcmp(Driver + Index1, MovlE2ToEcx, sizeof(MovlE2ToEcx)) == 0)
        {
            // search for wrmsr in next few bytes
            for (Index2 = Index1 + sizeof(MovlE2ToEcx); Index2 < Index1 + sizeof(MovlE2ToEcx) + 32; Index2++)
            {
                if (Driver[Index2] == Wrmsr[0] && Driver[Index2 + 1] == Wrmsr[1])
                {
                    // found it - patch it with nops
                    Count++;
                    Driver[Index2] = 0x90;
                    Driver[Index2 + 1] = 0x90;
                    verbose("\t%d wrmsr patched at 0x%X\n", Count, Index2);
                    break;
                } else if ((Driver[Index2] == 0xC9 && Driver[Index2 + 1] == 0xC3) ||
                           (Driver[Index2] == 0x5D && Driver[Index2 + 1] == 0xC3) ||
                           (Driver[Index2] == 0xB9 && Driver[Index2 + 3] == 0 && Driver[Index2 + 4] == 0) ||
                           (Driver[Index2] == 0x66 && Driver[Index2 + 1] == 0xB9 && Driver[Index2 + 3] == 0))
                {
                    // a leave/ret will cancel the search
                    // so will an intervening "mov[l] $xx, [e]cx"
                    break;
                }
            }
        } else if (memcmp(Driver + Index1, MovE2ToCx, sizeof(MovE2ToCx)) == 0)
        {
            // search for wrmsr in next few bytes
            for (Index2 = Index1 + sizeof(MovE2ToCx); Index2 < Index1 + sizeof(MovE2ToCx) + 32; Index2++)
            {
                if (Driver[Index2] == Wrmsr[0] && Driver[Index2 + 1] == Wrmsr[1])
                {
                    // found it - patch it with nops
                    Count++;
                    Driver[Index2] = 0x90;
                    Driver[Index2 + 1] = 0x90;
                    verbose("\t%d wrmsr patched at 0x%X\n", Count, Index2);
                    break;
                } else if ((Driver[Index2] == 0xC9 && Driver[Index2 + 1] == 0xC3) ||
                           (Driver[Index2] == 0x5D && Driver[Index2 + 1] == 0xC3) ||
                           (Driver[Index2] == 0xB9 && Driver[Index2 + 3] == 0 && Driver[Index2 + 4] == 0) ||
                           (Driver[Index2] == 0x66 && Driver[Index2 + 1] == 0xB9 && Driver[Index2 + 3] == 0))
                {
                    // a leave/ret will cancel the search
                    // so will an intervening "mov[l] $xx, [e]cx"
                    break;
                }
            }
        }
    }
    
    return Count;
}

// ===================================
// Patching IOAHCIBlockStorage to enable the trim support
unsigned int trimEnablerSata(void *data, UInt32 DriverSize, UInt32 StartLocation)
{
    // as far as I know this works from Lion to Sierra
    UInt8   Find[]    = { 0x00, 0x41, 0x50, 0x50, 0x4C, 0x45, 0x20, 0x53, 0x53, 0x44, 0x00 };
    
    UInt8   Replace[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    unsigned int count = FindAndReplace(data,
                                        DriverSize,
                                        StartLocation,
                                        Find,
                                        sizeof(Find),
                                        Replace,
                                        0);
    
    return count;
}

// ===================================
// Patching AppleAHCIPort to fix orange icon (Sata only)
unsigned int patch_AppleAHCIPort_OrangeFix(void *data, UInt32 DriverSize, UInt32 StartLocation)
{
    // as far as I know this works from lion onward
    UInt8   Find[]    = { 0x45, 0x78, 0x74, 0x65, 0x72, 0x6E, 0x61, 0x6C };
    UInt8   Replace[] = { 0x49, 0x6E, 0x74, 0x65, 0x72, 0x6E, 0x61, 0x6C };
    unsigned int count = FindAndReplace(data,
                                        DriverSize,
                                        StartLocation,
                                        Find,
                                        sizeof(Find),
                                        Replace,
                                        0);
    
    return count;
}

// ===================================
// (Micky1979)
// Patching NVDAStartupWeb
unsigned int patch_NVDAStartupWeb(void *data, UInt32 DriverSize, UInt32 StartLocation)
{
    unsigned int count = 0;
    // this patch is needed only in Sierra onward
    if (MacOSVerCurrent >= MacOSVer2Int("10.12"))
    {
        Node *nvdadrvNode = DT__FindNode("/nvdadrv", true);
        if (nvdadrvNode != 0)
        {
            DT__AddProperty(nvdadrvNode, "nvda_drv", strlen(NVDAVALUE), (void*)NVDAVALUE);
            UInt8   Find[]    = { 0x49, 0x4F, 0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x54, 0x72, 0x65, 0x65,
                0x3A, 0x2F, 0x6F, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0x73, 0x00, 0x4E, 0x56,
                0x44, 0x41, 0x53, 0x74, 0x61, 0x72, 0x74, 0x75, 0x70, 0x57, 0x65, 0x62,
                0x3A };
            
            UInt8   Replace[] = { 0x49, 0x4F, 0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x54, 0x72, 0x65, 0x65,
                0x3A, 0x2F, 0x6E, 0x76, 0x64, 0x61, 0x64, 0x72, 0x76, 0x00, 0x4E, 0x56,
                0x44, 0x41, 0x53, 0x74, 0x61, 0x72, 0x74, 0x75, 0x70, 0x57, 0x65, 0x62,
                0x3A };
            count = FindAndReplace(data,
                                   DriverSize,
                                   StartLocation,
                                   Find,
                                   sizeof(Find),
                                   Replace,
                                   0);
        }
        else
        {
            verbose("Error: Unable to add nvdadrv node\n");
        }
        
    }
    return count;
}

// ===================================
// Micky1979, 2017
// patch_prelinked_kexts
void patch_prelinked_kexts(void *kernelData,
                           u_int32_t uncompressed_size,
                           unsigned prelinkTextVmaddr,
                           unsigned prelinkTextFileOff)
{
    TagPtr KextsPatches;
    bool canPatchKexts = false;
    unsigned long prelinkDictSize;
    unsigned int   Index;
    unsigned int   Count = 0;
    
    UInt8 *Bytes = (UInt8 *)kernelData;
    
    UInt8 *prelinkDic = NULL;
    UInt32 prelinkDictStartLocation = 0;
    UInt32 prelinkDictEndLocation = 0;
    
    
    if (bootInfo->kextConfig.dictionary)
    {
        KextsPatches = XMLGetProperty(bootInfo->kextConfig.dictionary, (const char*)"KextsPatches");
    }
    
    verbose("[ KEXTS PATCHER START ]\n");
    //int lessBytes = (int)((uncompressed_size/3)*2); // speedup, the _PrelinkInfoDictionary should not be 1/3 of entire cache!
    for (Index = 0/*lessBytes*/; Index < uncompressed_size; ++Index)
    {
        //scan for _PrelinkInfoDictionary
        if (Bytes[Index]         == 0x3C
            && Bytes[Index + 1]  == 0x64
            && Bytes[Index + 2]  == 0x69
            && Bytes[Index + 3]  == 0x63
            && Bytes[Index + 4]  == 0x74
            && Bytes[Index + 5]  == 0x3E
            && Bytes[Index + 6]  == 0x3C
            && Bytes[Index + 7]  == 0x6B
            && Bytes[Index + 8]  == 0x65
            && Bytes[Index + 9]  == 0x79
            && Bytes[Index + 10] == 0x3E
            && Bytes[Index + 11] == 0x5F
            && Bytes[Index + 12] == 0x50
            && Bytes[Index + 13] == 0x72
            && Bytes[Index + 14] == 0x65
            && Bytes[Index + 15] == 0x6C
            && Bytes[Index + 16] == 0x69
            && Bytes[Index + 17] == 0x6E
            && Bytes[Index + 18] == 0x6B
            && Bytes[Index + 19] == 0x49
            && Bytes[Index + 20] == 0x6E
            && Bytes[Index + 21] == 0x66
            && Bytes[Index + 22] == 0x6F
            && Bytes[Index + 23] == 0x44
            && Bytes[Index + 24] == 0x69
            && Bytes[Index + 25] == 0x63
            && Bytes[Index + 26] == 0x74
            && Bytes[Index + 27] == 0x69
            && Bytes[Index + 28] == 0x6F
            && Bytes[Index + 29] == 0x6E
            && Bytes[Index + 30] == 0x61
            && Bytes[Index + 31] == 0x72
            && Bytes[Index + 32] == 0x79
            && Bytes[Index + 33] == 0x3C
            && Bytes[Index + 34] == 0x2F
            && Bytes[Index + 35] == 0x6B
            && Bytes[Index + 36] == 0x65
            && Bytes[Index + 37] == 0x79
            && Bytes[Index + 38] == 0x3E)
        {
            Count++;
            prelinkDictStartLocation = Index;
            DBG("\tFound _PrelinkInfoDictionary at 0x%08X index = %d\n", (unsigned int)prelinkDictStartLocation, Index);
            canPatchKexts = true;
            break;
        }
    }
  
    if (prelinkDictStartLocation)
    {
        for (Index = prelinkDictStartLocation; Index < uncompressed_size; ++Index)
        {
            // end of prelink ( <> plus some zeros)
            if (Bytes[Index]         == 0x3C
                && Bytes[Index + 1]  == 0x2F
                && Bytes[Index + 2]  == 0x64
                && Bytes[Index + 3]  == 0x69
                && Bytes[Index + 4]  == 0x63
                && Bytes[Index + 5]  == 0x74
                && Bytes[Index + 6]  == 0x3E
                && Bytes[Index + 7]  == 0x00
                && Bytes[Index + 8]  == 0x00
                && Bytes[Index + 9]  == 0x00
                && Bytes[Index + 10] == 0x00
                && Bytes[Index + 11] == 0x00
                && Bytes[Index + 12] == 0x00
                && Bytes[Index + 13] == 0x00)
            {
                Count++;
                
                if ((Count = 2))
                {
                    canPatchKexts = true;
                    prelinkDictEndLocation = Index + 7 ;
                    DBG("\tFound _PrelinkInfoDictionary end location at 0x%08X index = %d\n", (unsigned int)prelinkDictEndLocation, Index);
                }
                break;
            }
        }
    }
  
    if (canPatchKexts)
    {
        prelinkDictSize = uncompressed_size - prelinkDictStartLocation;
        prelinkDic = malloc(prelinkDictSize);
        memcpy(prelinkDic, Bytes+prelinkDictStartLocation, prelinkDictSize);
        TagPtr prelinkInfoPtr = NULL;
        XMLParseFile( (char *)prelinkDic, &prelinkInfoPtr );
        
        if (prelinkInfoPtr)
        {
            TagPtr prelinkInfoDictionary = XMLGetProperty(prelinkInfoPtr, "_PrelinkInfoDictionary");
            if (!prelinkInfoDictionary)
            {
                verbose("Unable to allocate the _PrelinkInfoDictionary, kexts patcher skipped.");
                return;
            }
            int count = XMLTagCount(prelinkInfoDictionary);
            while(count)
            {
                TagPtr sub = XMLGetElement( prelinkInfoDictionary, count-1);
                if (sub && XMLIsDict(sub))
                {
                    char* execPath = XMLCastString(XMLGetProperty(sub, (const char*)"CFBundleExecutable"));
                    
                    if (execPath != NULL)
                    {
                        UInt32 kextSize = XMLCastInteger(XMLGetProperty(sub, (const char*)"_PrelinkExecutableSize"));
                        UInt32 kextAddr = XMLCastInteger(XMLGetProperty(sub, (const char*)"_PrelinkExecutableSourceAddr"));
                        

                        if (kextAddr && kextSize)
                        {
                            // adjust binary address location
                            kextAddr -= prelinkTextVmaddr;
                            kextAddr += prelinkTextFileOff;
                            
                            DBG("\t[%d] found exec:%s (size = %u, kextAddr = 0x%X [vmaddr = 0x%X fileoff = 0x%X])\n", count, execPath,
                                (unsigned int)kextSize, (unsigned int)kextAddr, (unsigned int)prelinkTextVmaddr, (unsigned int)prelinkTextFileOff);
                            
                            if (!strcmp(execPath, "FakeSMC"))
                            {
                                FakeSMCLoaded = true;
                            }
                            
				// chameleon patches
				patchBooterDefinedKext(execPath, kernelData, kextSize, kextAddr);
                            
                            // user's defined
                            if (KextsPatches && XMLIsDict(KextsPatches))
                            {
                                pach_binaryUsingDictionary(kernelData,
                                                           kextSize,
                                                           kextAddr,
                                                           execPath,
                                                           KextsPatches);
                            }
                            
#if DEBUG_KERNEL
                            getchar();
#endif
                        }
                    }
                }
                
                count --;
            }
        }
    }
    else
    {
        verbose("Unable to find the _PrelinkInfoDictionary, kexts patcher skipped.");
    }
    
    if (prelinkDic) free(prelinkDic);
    verbose("Kexts patcher: end!\n\n");
}

void patchBooterDefinedKext(const char *kext, void *driverAddr, UInt32 DriverSize, UInt32 StartLocation)
{
	if ((!strcmp(kext, "AppleRTC")) && AppleRTCPatch)
	{
		verbose("\tPatching %s:", kext);

		unsigned int numPatches = AppleRTC_Patch(driverAddr, DriverSize, StartLocation);
		verbose(" %d substitutions made!\n", numPatches);
	}

	if ((!strcmp(kext, "AppleIntelCPUPowerManagement")) && AICPMPatch)
	{
		verbose("\tPatching %s (locked msr):\n", kext);

		unsigned int numPatches = AsusAICPUPMPatch(driverAddr, DriverSize, StartLocation);
		verbose("\t  %d substitutions made!\n", numPatches);
	}

	if ((!strcmp(kext, "NVDAStartupWeb")) && NVIDIAWebDrv)
	{
		verbose("\tPatching %s (force load w/o nvram):\n", kext);

		unsigned int numPatches = patch_NVDAStartupWeb(driverAddr, DriverSize, StartLocation);
		verbose("\t  %d substitutions made!\n", numPatches);
	}

	if ((!strcmp(kext, "IOAHCIBlockStorage")) && TrimEnablerSata)
	{
		verbose("\tPatching %s (trim enabler SATA):", kext);

		unsigned int numPatches = trimEnablerSata(driverAddr, DriverSize, StartLocation);
		verbose(" %d substitutions made!\n", numPatches);
	}

	if ((!strcmp(kext, "AppleAHCIPort")) && OrangeIconFixSata)
	{
		verbose("\tPatching %s (orange icon fix):", kext);

		unsigned int numPatches = patch_AppleAHCIPort_OrangeFix(driverAddr, DriverSize, StartLocation);
		verbose(" %d substitutions made!\n", numPatches);
	}
}

// ===================================
// Boolean function to check the full OSX match version
bool checkFullOSVer(const char *version)
{
	// We are in a version with 7 char ?
	if ( (sizeof(version) == 7) && ('.' == version[5]) && ('\0' != version[6]) ) // 10.xx.x
	{
		return (   (gBootVolume->OSFullVer[0] == version[0]) // 1
			&& (gBootVolume->OSFullVer[1] == version[1]) // 0
			&& (gBootVolume->OSFullVer[2] == version[2]) // .
			&& (gBootVolume->OSFullVer[3] == version[3]) // x
			&& (gBootVolume->OSFullVer[4] == version[4]) // x
			&& (gBootVolume->OSFullVer[5] == version[5]) // .
			&& (gBootVolume->OSFullVer[6] == version[6]) // x
		);
	}

	// We are in a version with 7 char ?
	if ( (sizeof(version) == 7) && ('.' == version[4]) && ('\0' != version[6]) ) // 10.4.11
	{
		return (   (gBootVolume->OSFullVer[0] == version[0]) // 1
			&& (gBootVolume->OSFullVer[1] == version[1]) // 0
			&& (gBootVolume->OSFullVer[2] == version[2]) // .
			&& (gBootVolume->OSFullVer[3] == version[3]) // 4
			&& (gBootVolume->OSFullVer[4] == version[4]) // .
			&& (gBootVolume->OSFullVer[5] == version[5]) // x
			&& (gBootVolume->OSFullVer[6] == version[6]) // x
		);
	}

	// We are in a version with 6 char ?
	if ( (sizeof(version) == 6) && ('.' == version[4]) && ('\0' != version[5]) ) // 10.x.x
	{
		return (   (gBootVolume->OSFullVer[0] == version[0]) // 1
			&& (gBootVolume->OSFullVer[1] == version[1]) // 0
			&& (gBootVolume->OSFullVer[2] == version[2]) // .
			&& (gBootVolume->OSFullVer[3] == version[3]) // x
			&& (gBootVolume->OSFullVer[4] == version[4]) // .
			&& (gBootVolume->OSFullVer[5] == version[5]) // x
		);
	}

	// We are in a version with 5 char ?
	if ( (sizeof(version) == 5) && ('.' != version[4]) && ('\0' != version[4]) ) // 10.xx
	{
		return (   (gBootVolume->OSFullVer[0] == version[0]) // 1
			&& (gBootVolume->OSFullVer[1] == version[1]) // 0
			&& (gBootVolume->OSFullVer[2] == version[2]) // .
			&& (gBootVolume->OSFullVer[3] == version[3]) // x
			&& (gBootVolume->OSFullVer[4] == version[4]) // x
		);
	}

	// We are in a version with 4 char ?
	if ( (sizeof(version) == 4) && ('\0' != version[3]) ) // 10.x
	{
		return (   (gBootVolume->OSFullVer[0] == version[0]) // 1
			&& (gBootVolume->OSFullVer[1] == version[1]) // 0
			&& (gBootVolume->OSFullVer[2] == version[2]) // .
			&& (gBootVolume->OSFullVer[3] == version[3]) // x
		);
	}

	// no match found
	return false;
}
