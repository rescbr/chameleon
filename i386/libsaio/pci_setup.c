#include "config.h"
#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
#include "modules.h"

#if DEBUG_PCI_SETUP
	#define DBG(x...)	printf(x)
#else
	#define DBG(x...)
#endif

extern bool setup_ati_devprop(pci_dt_t *ati_dev);
extern bool setup_nvidia_devprop(pci_dt_t *nvda_dev);
extern bool setup_gma_devprop(pci_dt_t *gma_dev);
extern bool setup_hda_devprop(pci_dt_t *hda_dev);
extern void setup_eth_devdrop(pci_dt_t *eth_dev);
extern void setup_wifi_devdrop(pci_dt_t *wifi_dev);

extern void notify_usb_dev(pci_dt_t *pci_dev);
extern void force_enable_hpet(pci_dt_t *lpc_dev);

extern pci_dt_t *dram_controller_dev;

void setup_pci_devs(pci_dt_t *pci_dt)
{
	char *devicepath;

	bool do_gfx_devprop = false;
	bool do_skip_n_devprop = false;
	bool do_skip_a_devprop = false;
	bool do_skip_i_devprop = false;

	bool do_enable_hpet = false;
	bool do_hda_devprop = false;

	pci_dt_t *current = pci_dt;

	// GraphicsEnabler
	getBoolForKey(kGraphicsEnabler, &do_gfx_devprop, &bootInfo->chameleonConfig);

	// Skip keys
	getBoolForKey(kSkipNvidiaGfx, &do_skip_n_devprop, &bootInfo->chameleonConfig);
	getBoolForKey(kSkipAtiGfx, &do_skip_a_devprop, &bootInfo->chameleonConfig);
	getBoolForKey(kSkipIntelGfx, &do_skip_i_devprop, &bootInfo->chameleonConfig);

	// HDAEnable
	getBoolForKey(kHDAEnabler, &do_hda_devprop, &bootInfo->chameleonConfig);

	// ForceHPET
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
				verbose("[ ETHERNET DEVICE INFO ]\n");
				setup_eth_devdrop(current);
				verbose("\n");
				break; // PCI_CLASS_NETWORK_ETHERNET

			case PCI_CLASS_NETWORK_OTHER:
				DBG("Setup WIRELESS %s enabled\n", do_wifi_devprop? "is":"is not");
				verbose("[ WIRELESS DEVICE INFO ]\n");
				setup_wifi_devdrop(current);
				verbose("\n");
				break; // PCI_CLASS_NETWORK_OTHER

			case PCI_CLASS_DISPLAY_VGA:
				DBG("GraphicsEnabler %s enabled\n", do_gfx_devprop? "is":"is not");
				if (do_gfx_devprop)
				{
					switch (current->vendor_id)
					{
						case PCI_VENDOR_ID_ATI:
							if ( do_skip_a_devprop )
							{
								verbose("Skip ATi/AMD gfx device!\n");
							}
							else
							{
								verbose("[ ATi GFX DEVICE INFO ]\n");
								setup_ati_devprop(current);
								verbose("\n");
							}
							break; // PCI_VENDOR_ID_ATI

						case PCI_VENDOR_ID_INTEL:
							if ( do_skip_i_devprop )
							{
								verbose("Skip Intel gfx device!\n");
							}
							else
							{
								verbose("[ INTEL GMA DEVICE INFO ]\n");
								setup_gma_devprop(current);
								verbose("\n");
							}
							break; // PCI_VENDOR_ID_INTEL

						case PCI_VENDOR_ID_NVIDIA:
							if ( do_skip_n_devprop )
							{
								verbose("Skip Nvidia gfx device!\n");
							}
							else
							{
								verbose("[ NVIDIA GFX DEVICE INFO ]\n");
								setup_nvidia_devprop(current);
								verbose("\n");
							}
							break; // PCI_VENDOR_ID_NVIDIA

						default:
							break;
					}
				}
				break; // PCI_CLASS_DISPLAY_VGA

			case PCI_CLASS_MULTIMEDIA_AUDIO_DEV:
				DBG("Setup HDEF %s enabled\n", do_hda_devprop ? "is":"is not");
				if (do_hda_devprop)
				{
					verbose("[ AUDIO DEVICE INFO ]\n");
					setup_hda_devprop(current);
					verbose("\n");
				}
				break; // PCI_CLASS_MULTIMEDIA_AUDIO_DEV

			case PCI_CLASS_SERIAL_USB:
				DBG("USB\n");
				notify_usb_dev(current);
				break; // PCI_CLASS_SERIAL_USB

			case PCI_CLASS_SERIAL_FIREWIRE:
				DBG("FireWire\n");
				verbose("[ FIREWIRE DEVICE INFO ]\n");
				verbose("\tClass code: [%04X]\n\tFireWire device [%04x:%04x]-[%04x:%04x]\n\t%s\n",
					current->class_id,current->vendor_id, current->device_id,
					current->subsys_id.subsys.vendor_id,
					current->subsys_id.subsys.device_id, devicepath);
//				set_fwr_devdrop(current);
				verbose("\n");
				break; // PCI_CLASS_SERIAL_FIREWIRE

			case PCI_CLASS_BRIDGE_ISA:
				DBG("Force HPET %s enabled\n", do_enable_hpet ? "is":"is not");
				if (do_enable_hpet)
				{
					verbose("[ HPET ]\n");
					force_enable_hpet(current);
					verbose("\n");
				}
				break; // PCI_CLASS_BRIDGE_ISA

			}

		execute_hook("PCIDevice", current, NULL, NULL, NULL);
		DBG("setup_pci_devs current device ID = [%04x:%04x]\n", current->vendor_id, current->device_id);
		setup_pci_devs(current->children);
		current = current->next;
	}
}
