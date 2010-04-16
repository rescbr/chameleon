#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
#include "nvidia.h"
#include "ati.h"
#include "gma.h"
#include "device_inject.h"

extern void set_eth_builtin(pci_dt_t *eth_dev);
extern int ehci_acquire(pci_dt_t *pci_dev);
extern int legacy_off(pci_dt_t *pci_dev);
extern int uhci_reset(pci_dt_t *pci_dev);
extern void force_enable_hpet(pci_dt_t *lpc_dev);

static int PciToEfiOverride(const char *  propKey,
 			    unsigned long numbers[],
 			    unsigned long maxArrayCount )
{
     char * propStr;
     unsigned long    count = 0;
 
     propStr = newStringForKey( (char *) propKey , &bootInfo->pciConfig );
     if ( propStr )
     {
         char * delimiter = propStr;
         char * p = propStr;
 
         while ( count < maxArrayCount && *p != '\0' )
         {
             unsigned long val = strtoul( p, &delimiter, 16 );
             if ( p != delimiter )
             {
                 numbers[count++] = val;
                 p = delimiter;
             }
             while ( ( *p != '\0' ) && !isdigit(*p) )
                 p++;
         }
 
         free( propStr );
     }
 
     return count;
}

int hasPciToEfiMapping = -1;	/* -1: not loaded, 0: does not exist, 1: loaded */

void setup_pci_devs(pci_dt_t *pci_dt)
{
	char *devicepath;
	bool do_eth_devprop, do_gfx_devprop, fix_ehci, fix_legoff, fix_uhci, fix_usb, do_enable_hpet;
	pci_dt_t *current = pci_dt;
	char override_key[512];
 	unsigned long id_array[4];
 	int id_count;
 	char* id_keys[4] = { "vendor-id", "device-id",  "subsystem-vendor-id", "subsystem-id" };
 	int i;
 	struct DevPropDevice* device;
 	struct DevPropString *deviceString;

	do_eth_devprop = do_gfx_devprop = fix_ehci = fix_legoff = fix_uhci = fix_usb = do_enable_hpet = false;

	getBoolForKey(kEthernetBuiltIn, &do_eth_devprop, &bootInfo->bootConfig);
	getBoolForKey(kGraphicsEnabler, &do_gfx_devprop, &bootInfo->bootConfig);
	if (getBoolForKey(kUSBBusFix, &fix_usb, &bootInfo->bootConfig) && fix_usb) {
		fix_ehci = fix_uhci = true;
	} else {
		getBoolForKey(kEHCIacquire, &fix_ehci, &bootInfo->bootConfig);
		getBoolForKey(kUHCIreset, &fix_uhci, &bootInfo->bootConfig);
	}
	getBoolForKey(kUSBLegacyOff, &fix_legoff, &bootInfo->bootConfig);
	getBoolForKey(kForceHPET, &do_enable_hpet, &bootInfo->bootConfig);

	// Get some PCI stuff

	if (hasPciToEfiMapping == -1)
        {
	    hasPciToEfiMapping = (loadSystemConfig("", &bootInfo->pciConfig, "pci.plist", true) == 0 ? 1 : 0);
	    if (hasPciToEfiMapping) 
		verbose("pci.plist is found.\n");
	}

	while (current)
	{
		devicepath = get_pci_dev_path(current);

		switch (current->class_id)
		{
			case PCI_CLASS_NETWORK_ETHERNET: 
				if (do_eth_devprop)
					set_eth_builtin(current);
				break;
				
			case PCI_CLASS_DISPLAY_VGA:
			case PCI_CLASS_DISPLAY_OTHER:
				if (do_gfx_devprop)
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
				break;

			case PCI_CLASS_SERIAL_USB:
				switch (pci_config_read8(current->dev.addr, PCI_CLASS_PROG))
				{
					/* EHCI */
					case 0x20:
				    	if (fix_ehci)
							ehci_acquire(current);
						if (fix_legoff)
							legacy_off(current);
						break;

					/* UHCI */
					case 0x00:
				    	if (fix_uhci)
							uhci_reset(current);
						break;
				}
				break;

			case PCI_CLASS_BRIDGE_ISA:
				if (do_enable_hpet)
					force_enable_hpet(current);
				break;
		}
		
		setup_pci_devs(current->children);

                if (hasPciToEfiMapping) 
                {
                    /* Device ID override injection */
                    memset(id_array, sizeof(id_array), 0);
                    sprintf(override_key, "pci%04x,%04x", current->vendor_id, current->device_id);
                    id_count = PciToEfiOverride(override_key, id_array, 4);
                    device = NULL;

                    for (i = 0; i < id_count; i++)
                    {
                        uint8_t fourOctets[4];
                        uint32_t id = id_array[i];
                        if (id == 0)
                        {
                            if (i == 0)
                                id = current->vendor_id;
                            else if (i == 1)
                                id = current->device_id;
                            else 
                                continue;
                        }

                        fourOctets[0] = id;
                        fourOctets[1] = id >> 8;
                        fourOctets[2] = 0;
                        fourOctets[3] = 0;
                        if (id != 0)
                        {
                            if (device == NULL)
                            {
                                device = devprop_find_device(devicepath);
                                if (device == NULL)
                                {
                                    deviceString = devprop_create_string();
                                    device = devprop_add_device(deviceString, devicepath);
                                }
                            }
                            devprop_add_value(device, id_keys[i], fourOctets, sizeof(fourOctets));
                            verbose("%s: %s 0x%02x\n", override_key, id_keys[i], id);
                        }
                    }
                }

                current = current->next;
        }
}
