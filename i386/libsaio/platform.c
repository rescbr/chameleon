/*
 *  platform.c
 *
 * AsereBLN: cleanup
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
inline bool platformCPUFeature(uint32_t feature)
{
	return (Platform->CPU.Features & feature);
}

/** Return if a CPU Extended feature specified by feature is activated (true) or not (false)  */
inline bool platformCPUExtFeature(uint32_t feature)
{
	return (Platform->CPU.ExtFeatures & feature);
}

/** 
    Scan platform hardware information, called by the main entry point (common_boot() ) 
    _before_ bootConfig xml parsing settings are loaded
*/
void scan_platform(void)
{	
	Platform = malloc(sizeof(Platform));
	memset(Platform, 0, sizeof(Platform));
	build_pci_dt();
	scan_cpu(Platform);
}
