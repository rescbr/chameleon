/*
 * ATIEnabler Module
 *	Enabled many nvidia and ati cards to be used out of the box in 
 *	OS X. This was converted from boot2 code to a boot2 module.
 *
 */

#include "libsaio.h"
#include "pci.h"
#include "bootstruct.h"
//#include "nvidia.h"
#include "ati5.h"
//#include "gma.h"
#include "modules.h"


#define kATIEnabler	"ATIEnabler"


void ATIEnabler_hook(void* arg1, void* arg2, void* arg3, void* arg4);

void ATIEnabler_start()
{
	register_hook_callback("PCIDevice", &ATIEnabler_hook);
}


void ATIEnabler_hook(void* arg1, void* arg2, void* arg3, void* arg4)
{	
	pci_dt_t* current = arg1;
	if(current && current->class_id == PCI_CLASS_DISPLAY_VGA)
	{
		char *devicepath = get_pci_dev_path(current);

		bool do_gfx_devprop = true;
		getBoolForKey(kATIEnabler, &do_gfx_devprop, &bootInfo->bootConfig);
				
		if (do_gfx_devprop)
		{
			switch (current->vendor_id)
			{
				case PCI_VENDOR_ID_ATI:
					verbose("ATI VGA Controller [%04x:%04x] :: %s \n", 
									current->vendor_id, current->device_id, devicepath);
					setup_ati_devprop(current); 
					break;
					
			}
		}
		 
	}
}
