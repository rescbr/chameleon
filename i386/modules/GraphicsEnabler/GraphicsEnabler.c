/*
 * GraphicsEnabler Module
 *	Enabled many nvidia and ati cards to be used out of the box in 
 *	OS X. This was converted from boot2 code to a boot2 module.
 *
 */

#include "libsaio.h"
#include "pci.h"
#include "bootstruct.h"
#include "nvidia.h"
#include "ati.h"
#include "gma.h"
#include "modules.h"
#include "device_inject.h"


void GraphicsEnabler_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void GraphicsEnabler_start(void);

void GraphicsEnabler_start(void)
{
	register_device_inject();
	register_hook_callback("PCIDevice", &GraphicsEnabler_hook);
}

void GraphicsEnabler_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	pci_dt_t* current = arg1;
	
	if(current && current->class_id == PCI_CLASS_DISPLAY_VGA)
	{
        
		switch (current->vendor_id)
		{
			case PCI_VENDOR_ID_ATI:
				
				setup_ati_devprop(current);
				break;
				
			case PCI_VENDOR_ID_INTEL: 
				setup_gma_devprop(current);
				break;
                
			case PCI_VENDOR_ID_NVIDIA: 
				setup_nvidia_devprop(current);
				break;
			default:
				break;
		}		
        
	}
}