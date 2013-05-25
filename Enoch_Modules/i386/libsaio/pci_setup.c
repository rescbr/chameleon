#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
#include "modules.h"


extern void set_eth_builtin(pci_dt_t *eth_dev);
extern void set_wifi_airport(pci_dt_t *wifi_dev);
extern bool set_usb_devprop(pci_dt_t *usb_dev);
extern void notify_usb_dev(pci_dt_t *pci_dev);
extern void force_enable_hpet(pci_dt_t *lpc_dev);
extern pci_dt_t *dram_controller_dev;

void setup_pci_devs(pci_dt_t *pci_dt)
{
	char *devicepath;
	bool do_eth_devprop, do_wifi_devprop, do_usb_devprop, do_enable_hpet;
	pci_dt_t *current = pci_dt;

	do_eth_devprop = do_wifi_devprop = do_usb_devprop = do_enable_hpet = false;

	getBoolForKey(kEthernetBuiltIn, &do_eth_devprop, &bootInfo->chameleonConfig);
	getBoolForKey(kEnableWifi, &do_wifi_devprop, &bootInfo->chameleonConfig);
	getBoolForKey(kForceHPET, &do_enable_hpet, &bootInfo->chameleonConfig);

	while (current)
	{
		devicepath = get_pci_dev_path(current);

		switch (current->class_id)
		{
			case PCI_CLASS_BRIDGE_HOST:
				//DBG("Setup BRIDGE_HOST \n");
				if (current->dev.addr == PCIADDR(0, 0, 0))
				{
					dram_controller_dev = current;
				}
				break;
				
			case PCI_CLASS_NETWORK_ETHERNET: 
				//DBG("Setup ETHERNET %s enabled\n", do_eth_devprop?"":"no");
				if (do_eth_devprop)
				{
					set_eth_builtin(current);
				}
				break;

			case PCI_CLASS_SERIAL_USB:
				//DBG("USB fix \n");
				notify_usb_dev(current);
				/*if (do_usb_devprop)
				{
					set_usb_devprop(current);
				}*/
				break;

			case PCI_CLASS_BRIDGE_ISA:
				//DBG("Force HPET %s enabled\n", do_enable_hpet?"":"no");
				if (do_enable_hpet)
				{
					force_enable_hpet(current);
				}
				break;
		}
		
		execute_hook("PCIDevice", current, NULL, NULL, NULL);
		//DBG("setup_pci_devs current devID=%08x\n", current->device_id);
		setup_pci_devs(current->children);
		current = current->next;
	}
}
