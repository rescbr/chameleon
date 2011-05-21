/*
 * GraphicsEnabler Module
 *	Enabled many nvidia and ati cards to be used out of the box in 
 *	OS X. This was converted from boot2 code to a boot2 module.
 *
 */

#include "saio_internal.h"
#include "bootstruct.h"
#include "../modules/ATIEnablerLegacy/pci_old.h"
#include "ati.h"
#include "modules.h"


#define kGraphicsEnablerKey		"GraphicsEnabler" // change?

void GraphicsEnabler_hook(void* arg1, void* arg2, void* arg3, void* arg4);


void ATIEnablerLegacy_start()
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
	
	//Azi: ati.c doesn't seem to have "fail" code... check better!***
	if (do_gfx_devprop && (current->vendor_id == PCI_VENDOR_ID_ATI))
	{
		verbose("ATI VGA Controller [%04x:%04x] :: %s \n", 
				current->vendor_id, current->device_id, devicepath);
		setup_ati_devprop(current);
	}
	else
		verbose("Not an ATI card...??\n");
}
