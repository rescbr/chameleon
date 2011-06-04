/*
 *	ATIGraphicsEnabler Module ---
 *	  Enables many ATI HD cards to be used out of the box in OS X.
 *	  This was converted from boot2 code ( > r784) to a boot2 module.
 *
 */

#include "saio_internal.h"
#include "bootstruct.h"
#include "pci.h"
#include "modules.h"

#define kGraphicsEnablerKey "GraphicsEnabler"

extern bool setup_ati_devprop(pci_dt_t *ati_dev);
void GraphicsEnabler_hook(void* arg1, void* arg2, void* arg3, void* arg4);


void AMDGraphicsEnabler_start()
{
	register_hook_callback("PCIDevice", &GraphicsEnabler_hook);
}

void GraphicsEnabler_hook(void* arg1, void* arg2, void* arg3, void* arg4)
{
	pci_dt_t* current = arg1;
	
	if (current->class_id != PCI_CLASS_DISPLAY_VGA) return;
	
	char *devicepath = get_pci_dev_path(current);
	
	bool do_gfx_devprop = true;
	getBoolForKey(kGraphicsEnablerKey, &do_gfx_devprop, &bootInfo->bootConfig);
	
	if (do_gfx_devprop && (current->vendor_id == PCI_VENDOR_ID_ATI))
	{
		verbose("AMD/ATI VGA Controller [%04x:%04x] :: %s \n",
				current->vendor_id, current->device_id, devicepath);
		setup_ati_devprop(current);
	}
	else
		verbose("[%04x:%04x] :: %s, is not a AMD/ATI VGA Controller.\n",
				current->vendor_id, current->device_id, devicepath);
}
