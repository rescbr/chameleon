/*
 *	IntelGraphicsEnabler Module
 *		Enables a few Intel cards to be used out of the box in OS X.
 *
 */

#include "saio_internal.h"
#include "bootstruct.h"
#include "pci.h"
#include "gma.h"
#include "modules.h"

#define kGraphicsEnablerKey "GraphicsEnabler"

void GraphicsEnabler_hook(void* arg1, void* arg2, void* arg3, void* arg4);


void IntelGraphicsEnabler_start()
{
	register_hook_callback("PCIDevice", &GraphicsEnabler_hook);
}


void GraphicsEnabler_hook(void* arg1, void* arg2, void* arg3, void* arg4)
{
	pci_dt_t* current = arg1;
	
	if (current->class_id != PCI_CLASS_DISPLAY_VGA) return;
	
	char *devicepath = get_pci_dev_path(current);
	
	bool do_gfx_devprop = true;
	getBoolForKey(kGraphicsEnablerKey, &do_gfx_devprop, &bootInfo->chameleonConfig);
	
	if (do_gfx_devprop && (current->vendor_id == PCI_VENDOR_ID_INTEL))
	{	
		verbose("Intel VGA Controller [%04x:%04x] :: %s \n",
				current->vendor_id, current->device_id, devicepath);
		setup_gma_devprop(current);
	}
	else
		verbose("[%04x:%04x] :: %s, is not a Intel VGA Controller.\n",
				current->vendor_id, current->device_id, devicepath);
}
