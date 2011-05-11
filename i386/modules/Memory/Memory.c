/*
 * DRAM Controller Module
 * Scans the dram controller and notifies OS X of the memory modules.
 *	This was converted from boot2 code to a boot2 module.
 *
 */

#include "libsaio.h"
#include "pci.h"
#include "platform.h"
#include "dram_controllers.h"
#include "spd.h"
#include "mem.h"
#include "boot.h"
#include "bootstruct.h"
#include "modules.h"

#define kUseMemDetect		"UseMemDetect"	    

pci_dt_t * dram_controller_dev = NULL;
pci_dt_t * smbus_controller_dev = NULL;


void Memory_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void Memory_PCIDevice_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);


void Memory_start()
{
	register_hook_callback("PCIDevice", &Memory_PCIDevice_hook);
	register_hook_callback("ScanMemory", &Memory_hook);
	
}

void Memory_PCIDevice_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	pci_dt_t* current = arg1;
	if(current->class_id == PCI_CLASS_BRIDGE_HOST)
	{
		dram_controller_dev = current;
	}
	else if(is_smbus_controller(current))
	{
		smbus_controller_dev = current;
	}

}

void Memory_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	bool useAutodetection = true;
    getBoolForKey(kUseMemDetect, &useAutodetection, &bootInfo->bootConfig);
	
	
    if (useAutodetection) {
		
		if (dram_controller_dev!=NULL) {
			scan_dram_controller(dram_controller_dev); // Rek: pci dev ram controller direct and fully informative scan ...
		}
				
		if(smbus_controller_dev)
		{
			scan_spd(Platform, smbus_controller_dev);
		}
    }
	
}