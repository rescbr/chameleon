
//#include "libsaio.h"
//#include "bootstruct.h"
#include "boot.h"
#include "pci.h"
#include "modules.h"
#include "modules.h"


extern void set_eth_builtin(pci_dt_t *eth_dev);
extern void notify_usb_dev(pci_dt_t *pci_dev);


void setup_pci_devs(pci_dt_t *pci_dt)
{
	bool do_eth_devprop;
	pci_dt_t *current = pci_dt;

	do_eth_devprop = false;

	getBoolForKey(kEthernetBuiltInKey, &do_eth_devprop, &bootInfo->bootConfig);

	while (current)
	{
		execute_hook("PCIDevice", current, NULL, NULL, NULL);
		
		switch (current->class_id)
		{
			case PCI_CLASS_NETWORK_ETHERNET: 
				if (do_eth_devprop)
					set_eth_builtin(current);
				break;

			case PCI_CLASS_SERIAL_USB:
				notify_usb_dev(current);
				break;
		}
		
		execute_hook("PCIDevice", current, NULL, NULL, NULL);
		
		setup_pci_devs(current->children);
		current = current->next;
	}
}
