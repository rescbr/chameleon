#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
//#include "gma.h"
//#include "nvidia.h"
//#include "hda.h"
#include "modules.h"
//#include "device_inject.h"

extern bool setup_ati_devprop(pci_dt_t *ati_dev);
extern bool setup_nvidia_devprop(pci_dt_t *nvda_dev);
extern bool setup_gma_devprop(pci_dt_t *gma_dev);
extern bool setup_hda_devprop(pci_dt_t *hda_dev);
extern void setup_eth_builtin(pci_dt_t *eth_dev);
extern void setup_wifi_airport(pci_dt_t *wifi_dev);
extern bool set_usb_devprop(pci_dt_t *usb_dev);
extern void notify_usb_dev(pci_dt_t *pci_dev);
extern void force_enable_hpet(pci_dt_t *lpc_dev);
extern pci_dt_t *dram_controller_dev;

void setup_pci_devs(pci_dt_t *pci_dt)
{
	char *devicepath;
	bool doit, do_eth_devprop, do_wifi_devprop, do_usb_devprop, do_gfx_devprop, do_enable_hpet, do_hda_devprop = false;
	pci_dt_t *current = pci_dt;

	//do_eth_devprop = do_wifi_devprop = do_usb_devprop = do_gfx_devprop = do_enable_hpet = do_hda_devprop = false;

	getBoolForKey(kEthernetBuiltIn, &do_eth_devprop, &bootInfo->chameleonConfig);
	getBoolForKey(kEnableWifi, &do_wifi_devprop, &bootInfo->chameleonConfig);
	getBoolForKey(kGraphicsEnabler, &do_gfx_devprop, &bootInfo->chameleonConfig);
	getBoolForKey(kUsbInject, &do_usb_devprop, &bootInfo->chameleonConfig);
	getBoolForKey(kHDAEnabler, &do_hda_devprop, &bootInfo->chameleonConfig);
	getBoolForKey(kForceHPET, &do_enable_hpet, &bootInfo->chameleonConfig);

/* tennisgeek http://forum.voodooprojects.org/index.php/topic,1333.0.html
	// Get some PCI stuff
	if (hasPciToEfiMapping == -1) {
		hasPciToEfiMapping = (loadSystemConfig("", &bootInfo->pciConfig, "pci.plist", true) == 0 ? 1 : 0);
		if (hasPciToEfiMapping) {
			verbose("pci.plist is found.\n");
		}
	}

	if (hasPciToEfiMapping) {
		// Device ID override injection
		memset(id_array, sizeof(id_array), 0);
		sprintf(override_key, "pci%04x,%04x", current->vendor_id, current->device_id);
		id_count = PciToEfiOverride(override_key, id_array, 4);
		device = NULL;
		for (i = 0; i < id_count; i++) {
			uint8_t fourOctets[4];
			uint32_t id = id_array[i];
			if (id == 0) {
				if (i == 0) {
					id = current->vendor_id;
				} else if (i == 1) {
					id = current->device_id;
				} else {
					continue;
				}
			}

			fourOctets[0] = id;
			fourOctets[1] = id >> 8;
			fourOctets[2] = 0;
			fourOctets[3] = 0;
			if (id != 0) {
				if (device == NULL) {
					device = devprop_find_device(devicepath);
					if (device == NULL) {
						deviceString = devprop_create_string();
						device = devprop_add_device(deviceString, devicepath);
					}
				}
				devprop_add_value(device, id_keys[i], fourOctets, sizeof(fourOctets));
				verbose("%s: %s 0x%02x\n", override_key, id_keys[i], id);
			}
		}
		current = current->next;
	}
*/ // tennisgeek http://forum.voodooprojects.org/index.php/topic,1333.0.html

	while (current) {
		devicepath = get_pci_dev_path(current);

		switch (current->class_id) {
			case PCI_CLASS_BRIDGE_HOST:
				//DBG("Setup BRIDGE_HOST \n");
				if (current->dev.addr == PCIADDR(0, 0, 0)) {
					dram_controller_dev = current;
				}
				break;
				
			case PCI_CLASS_NETWORK_ETHERNET: 
				//DBG("Setup ETHERNET %s enabled\n", do_eth_devprop?"":"no");
				if (do_eth_devprop) {
					setup_eth_builtin(current);
				}
				break;

			case PCI_CLASS_NETWORK_OTHER:
				//DBG("Setup WIRELESS %s enabled\n", do_wifi_devprop?"":"no");
				if (do_wifi_devprop) {
					setup_wifi_airport(current);
				}
				break;

			case PCI_CLASS_DISPLAY_VGA:
				//DBG("GraphicsEnabler %s enabled\n", do_gfx_devprop?"":"no");
				if (do_gfx_devprop) {
					switch (current->vendor_id) {
						case PCI_VENDOR_ID_ATI:
							if (getBoolForKey(kSkipAtiGfx, &doit, &bootInfo->chameleonConfig) && doit) {
								verbose("Skip ATi/AMD gfx device!\n");
							} else {
								setup_ati_devprop(current);
							}
							break;

						case PCI_VENDOR_ID_INTEL:
							if (getBoolForKey(kSkipIntelGfx, &doit, &bootInfo->chameleonConfig) && doit) {
								verbose("Skip Intel gfx device!\n");
							} else {
								setup_gma_devprop(current);
							}
							break;

						case PCI_VENDOR_ID_NVIDIA:
							if (getBoolForKey(kSkipNvidiaGfx, &doit, &bootInfo->chameleonConfig) && doit) {
								verbose("Skip Nvidia gfx device!\n");
							} else {
								setup_nvidia_devprop(current);
							}
							break;
						}
					}
					break;


			case PCI_CLASS_MULTIMEDIA_AUDIO_DEV:
				//DBG("Setup HDEF %s enabled\n", do_hda_devprop?"":"no");
				if (do_hda_devprop) {
					setup_hda_devprop(current);
				}
				break;

				case PCI_CLASS_SERIAL_USB:
					//DBG("USB fix \n");
					notify_usb_dev(current);
        			        /*if (do_usb_devprop) {
            				        set_usb_devprop(current);
            				    }*/
					break;

				case PCI_CLASS_BRIDGE_ISA:
					//DBG("Force HPET %s enabled\n", do_enable_hpet?"":"no");
					if (do_enable_hpet) {
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
