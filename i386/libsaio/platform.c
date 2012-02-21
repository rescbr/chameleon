/*
 *  platform.c
 *
 * AsereBLN: cleanup
 *
 *	ALL functions and struct. here will be DEPRECATED soon, i prefer Hash. (hmm, i mean hashable structures ... of course)
 *
 */

#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
#include "platform.h"
#include "cpu.h"

#ifndef DEBUG_PLATFORM
#define DEBUG_PLATFORM 0
#endif

#if DEBUG_PLATFORM
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

PlatformInfo_t    *Platform;

/** Return if a CPU feature specified by feature is activated (true) or not (false)  */
bool platformCPUFeature(uint32_t feature)
{
	return (Platform->CPU.Features & feature) ? true : false;
}

/** Return if a CPU Extended feature specified by feature is activated (true) or not (false)  */
bool platformCPUExtFeature(uint32_t feature)
{
	return (Platform->CPU.ExtFeatures & feature) ? true : false;
}

bool platformIsIntel(void)
{
	return (Platform->CPU.Vendor == 0x756E6547) ? true : false;
}

uint32_t getCPUnCores(void)
{
	return Platform->CPU.NoCores;
}

uint32_t getCPUnThreads(void)
{
	return Platform->CPU.NoThreads;
}

uint8_t getCPUModel(void)
{
	return Platform->CPU.Model;
}

uint8_t getCPUFamily(void)
{
	return Platform->CPU.Family;
}

bool platformIsServer(void)
{
	return (Platform->CPU.isServer) ? true : false;
}

bool platformIsMobile(void)
{
	return (Platform->CPU.isMobile) ? true : false;
}

/** 
    Scan platform hardware information, called by the main entry point (common_boot() ) 
    _before_ bootConfig xml parsing settings are loaded
*/
void scan_platform(void)
{	
	Platform = malloc(sizeof(PlatformInfo_t)); 
	memset(Platform, 0, sizeof(PlatformInfo_t));
	build_pci_dt();
	scan_cpu(Platform);
}
