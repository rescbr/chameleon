/*
 * Memory Module
 * Scans the dram controller and(/or spd), and notifies OS X of the memory modules.
 *	This was converted from boot2 code to a boot2 module.
 *
 */

//#include "libsaio.h"
#include "boot.h"
#include "pci.h"
#include "platform.h"
#include "dram_controllers.h"
#include "spd.h"
//#include "mem.h"
#include "modules.h"

#define kUseMemDetectKey "UseMemDetect"

pci_dt_t *dram_controller_dev;


void Memory_hook(void* arg1, void* arg2, void* arg3, void* arg4);
void Memory_PCIDevice_hook(void* arg1, void* arg2, void* arg3, void* arg4);


void Memory_start()
{
	register_hook_callback("PCIDevice", &Memory_PCIDevice_hook);
	register_hook_callback("ScanMemory", &Memory_hook);
	
}

void Memory_PCIDevice_hook(void* arg1, void* arg2, void* arg3, void* arg4)
{
	pci_dt_t* current = arg1;
	
	if (current->class_id == PCI_CLASS_BRIDGE_HOST
		&& (current->dev.addr == PCIADDR(0, 0, 0)))
	{
		dram_controller_dev = current;
	}
}

void Memory_hook(void* arg1, void* arg2, void* arg3, void* arg4)
{	
	bool useAutodetection = true;
	getBoolForKey(kUseMemDetectKey, &useAutodetection, &bootInfo->bootConfig);
	
	if (useAutodetection)
	{
		if (dram_controller_dev != NULL)
		{
			// Rek: pci dev ram controller direct and fully informative scan ...
			scan_dram_controller(dram_controller_dev);
		}
	}
	//Azi: gone on Kabyl's smbios update...???
	// unfortunately still necesary for some comp where spd cant read correct speed
//	scan_memory(&Platform);
	
	scan_spd(&Platform); // check Mek's implementation!
}
