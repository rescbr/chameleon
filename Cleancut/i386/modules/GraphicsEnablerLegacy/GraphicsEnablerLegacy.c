/*
 * GraphicsEnabler Module
 *	Enabled many nvidia and ati cards to be used out of the box in 
 *	OS X. This was converted from boot2 code to a boot2 module.
 *
 */

#include "saio_internal.h"
#include "bootstruct.h"
#include "../modules/GraphicsEnablerLegacy/pci_old.h"
#include "nvidia.h"
#include "ati.h"
#include "gma.h"
#include "modules.h"

#define kGraphicsEnabler	"GraphicsEnabler"

void GraphicsEnabler_hook(void* arg1, void* arg2, void* arg3, void* arg4);

void GraphicsEnablerLegacy_start()
{
	register_hook_callback("PCIDevice", &GraphicsEnabler_hook);
}


void GraphicsEnabler_hook(void* arg1, void* arg2, void* arg3, void* arg4)
{
	pci_dt_t* current = arg1;
	
	if(current->class_id != PCI_CLASS_DISPLAY_VGA) return;
	
	char *devicepath = get_pci_dev_path(current);

	bool do_gfx_devprop = true;
	getBoolForKey(kGraphicsEnabler, &do_gfx_devprop, &bootInfo->bootConfig);

	if (do_gfx_devprop)
	{
		switch (current->vendor_id)
		{
			case PCI_VENDOR_ID_ATI:
				verbose("ATI VGA Controller [%04x:%04x] :: %s \n", 
						current->vendor_id, current->device_id, devicepath);
				setup_ati_devprop(current); 
				break;

			case PCI_VENDOR_ID_INTEL:
				verbose("Intel Graphics Controller [%04x:%04x] :: %s \n",
						current->vendor_id, current->device_id, devicepath);
				setup_gma_devprop(current);
				break;

			case PCI_VENDOR_ID_NVIDIA: 
				setup_nvidia_devprop(current);
				break;
		}
	}
}
