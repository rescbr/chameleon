#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
#include "modules.h"

#ifndef DEBUG_PCI_SETUP
	#define DEBUG_PCI_SETUP 0
#endif

#if DEBUG_PCI_SETUP
	#define DBG(x...)	printf(x)
#else
	#define DBG(x...)
#endif

extern bool setup_ati_devprop(pci_dt_t *ati_dev);
extern bool setup_nvidia_devprop(pci_dt_t *nvda_dev);
extern bool setup_gma_devprop(pci_dt_t *gma_dev);
extern bool setup_hda_devprop(pci_dt_t *hda_dev);
extern void setup_eth_builtin(pci_dt_t *eth_dev);
extern void notify_usb_dev(pci_dt_t *pci_dev);
extern void force_enable_hpet(pci_dt_t *lpc_dev);

extern pci_dt_t *dram_controller_dev;

void setup_pci_devs(pci_dt_t *pci_dt)
{
	char *devicepath;
	bool doit, do_eth_devprop, do_gfx_devprop, do_enable_hpet, do_hda_devprop;
	pci_dt_t *current = pci_dt;

	do_eth_devprop = do_gfx_devprop = do_enable_hpet = do_hda_devprop = false;

	getBoolForKey(kEthernetBuiltIn, &do_eth_devprop, &bootInfo->chameleonConfig);
	getBoolForKey(kGraphicsEnabler, &do_gfx_devprop, &bootInfo->chameleonConfig);
	getBoolForKey(kHDAEnabler, &do_hda_devprop, &bootInfo->chameleonConfig);
	getBoolForKey(kForceHPET, &do_enable_hpet, &bootInfo->chameleonConfig);

	while (current)
	{
		devicepath = get_pci_dev_path(current);

		switch (current->class_id)
		{
			case PCI_CLASS_BRIDGE_HOST:
				DBG("Setup BRIDGE_HOST \n");
				if (current->dev.addr == PCIADDR(0, 0, 0))
				{
					dram_controller_dev = current;
				}
				break; // PCI_CLASS_BRIDGE_HOST

			case PCI_CLASS_NETWORK_ETHERNET:
				DBG("Setup ETHERNET %s enabled\n", do_eth_devprop? "is":"is not");
				if (do_eth_devprop)
				{
					setup_eth_builtin(current);
				}
				break; // PCI_CLASS_NETWORK_ETHERNET

			case PCI_CLASS_DISPLAY_VGA:
				DBG("GraphicsEnabler %s enabled\n", do_gfx_devprop? "is":"is not");
				if (do_gfx_devprop)
				{
					switch (current->vendor_id)
					{
						case PCI_VENDOR_ID_ATI:
							if (getBoolForKey(kSkipAtiGfx, &doit, &bootInfo->chameleonConfig) && doit)
							{
								verbose("Skip ATi/AMD gfx device!\n");
							}
							else
							{
								setup_ati_devprop(current);
							}
							break; // PCI_VENDOR_ID_ATI

						case PCI_VENDOR_ID_INTEL:
							if (getBoolForKey(kSkipIntelGfx, &doit, &bootInfo->chameleonConfig) && doit)
							{
								verbose("Skip Intel gfx device!\n");
							}
							else
							{
								setup_gma_devprop(current);
							}
							break; // PCI_VENDOR_ID_INTEL

						case PCI_VENDOR_ID_NVIDIA:
							if (getBoolForKey(kSkipNvidiaGfx, &doit, &bootInfo->chameleonConfig) && doit)
							{
								verbose("Skip Nvidia gfx device!\n");
							}
							else
							{
								setup_nvidia_devprop(current);
							}
							break; // PCI_VENDOR_ID_NVIDIA

						default:
							break;
					}
				}
				break; // PCI_CLASS_DISPLAY_VGA

			case PCI_CLASS_MULTIMEDIA_AUDIO_DEV:
				DBG("Setup HDEF %s enabled\n", do_hda_devprop? "is":"is not");
				if (do_hda_devprop)
				{
					setup_hda_devprop(current);
				}
				break; // PCI_CLASS_MULTIMEDIA_AUDIO_DEV

			case PCI_CLASS_SERIAL_USB:
				DBG("USB\n");
				notify_usb_dev(current);
				break; // PCI_CLASS_SERIAL_USB

			case PCI_CLASS_BRIDGE_ISA:
				DBG("Force HPET %s enabled\n", do_enable_hpet? "is":"is not");
				if (do_enable_hpet)
				{
					force_enable_hpet(current);
				}
				break; // PCI_CLASS_BRIDGE_ISA

			}

		execute_hook("PCIDevice", current, NULL, NULL, NULL);
		DBG("setup_pci_devs current device ID = [%04x:%04x]\n", current->vendor_id, current->device_id);
		setup_pci_devs(current->children);
		current = current->next;
	}
}
