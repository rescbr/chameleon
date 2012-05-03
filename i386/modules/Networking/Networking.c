/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include "libsaio.h"
#include "modules.h"
#include "bootstruct.h"
#include "pci.h"
#include "device_inject.h"

#ifndef DEBUG_ETHERNET
#define DEBUG_ETHERNET 0
#endif

#if DEBUG_ETHERNET
#define DBG(x...) printf(x)
#else
#define DBG(x...)
#endif
#define kEnableWifi			"EnableWifi"	
#define kEthernetBuiltIn	"EthernetBuiltIn"	

static void set_eth_builtin(pci_dt_t *eth_dev);
static void set_wifi_airport(pci_dt_t *wlan_dev);
static int devprop_add_network_template(struct DevPropDevice *device, uint16_t vendor_id);


void Networking_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);

uint32_t builtin_set = 0;

void Networking_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	pci_dt_t* current = arg1;
	
	if(current->class_id == PCI_CLASS_NETWORK_ETHERNET)
	{
		// LAN
		
		bool do_eth_devprop = true;	
		getBoolForKey(kEthernetBuiltIn, &do_eth_devprop, DEFAULT_BOOT_CONFIG);
		
		if (do_eth_devprop)
		{
			set_eth_builtin(current);
		}
	}
	else if(current->class_id == PCI_CLASS_NETWORK_OTHER)
	{
		// WIFI
		bool do_wifi_devprop = true;	
		getBoolForKey(kEnableWifi, &do_wifi_devprop, DEFAULT_BOOT_CONFIG);
		
		if (do_wifi_devprop)		
            set_wifi_airport(current);
		
	}
	
}

void Networking_start(void);
void Networking_start(void)
{	
	register_hook_callback("PCIDevice", &Networking_hook);
}

/* a fine place for this code */

static int devprop_add_network_template(struct DevPropDevice *device, uint16_t vendor_id)
{
	uint8_t builtin = 0x0;
	
	if(device)
	{
		
		if((vendor_id != 0x168c) && (builtin_set == 0)) 
		{
			builtin_set = 1;
			builtin = 0x01;
		}
		
		if(!devprop_add_value(device, "built-in", (uint8_t*)&builtin, 1))
		{
			return 0;
		}
		
		devices_number++;
		return 1;
	}
	else
	{
		return 0;
	}
	
}

static void set_eth_builtin(pci_dt_t *eth_dev)
{
	char *devicepath = get_pci_dev_path(eth_dev);
    if (!devicepath) {
        return ;
    }
	struct DevPropDevice *device /*= (struct DevPropDevice*)malloc(sizeof(struct DevPropDevice))*/;
	
	verbose("LAN Controller [%04x:%04x] :: %s\n", eth_dev->vendor_id, eth_dev->device_id, devicepath);
	
	if (!string)
		string = devprop_create_string();
	
	device = devprop_add_device(string, devicepath);
	if(device)
	{
		verbose("Setting up lan keys\n");
		devprop_add_network_template(device, eth_dev->vendor_id);
		stringdata = (uint8_t*)malloc(sizeof(uint8_t) * string->length);
		if(stringdata)
		{
			memcpy(stringdata, (uint8_t*)devprop_generate_string(string), string->length);
			stringlength = string->length;
		}
	}
}


struct wifi_cards
{
	uint16_t	vendor_id;
	uint16_t	device_id;
	char*		model;
};

struct wifi_cards known_wifi_cards[] =
{
	{0x14e4, 0x4315, "Dell Wireless 1395"},
	{0x14e4, 0x432b, "Dell Wireless 1510"},
	{0x168C, 0x002B, "Atheros 9285 8802.11 b/g/n Wireless Network Adapter"},
};

static void set_wifi_airport(pci_dt_t *wlan_dev)
{
	char tmp[16];
	
	char *devicepath = get_pci_dev_path(wlan_dev);
    if (!devicepath) {
        return ;
    }
	struct DevPropDevice *device /*= (struct DevPropDevice*)malloc(sizeof(struct DevPropDevice))*/;
	
	verbose("Wifi Controller [%04x:%04x] :: %s\n", wlan_dev->vendor_id, wlan_dev->device_id, devicepath);
	
	if (!string)
		string = devprop_create_string();
	
	device = devprop_add_device(string, devicepath);
	if(device)
	{
		sprintf(tmp, "Airport");
		devprop_add_value(device, "AAPL,slot-name", (uint8_t *) tmp, strlen(tmp) + 1);
		devprop_add_value(device, "device_type", (uint8_t *) tmp, strlen(tmp) + 1);
        
		
		unsigned int i = 0;
		for( ; i < sizeof(known_wifi_cards) / sizeof(known_wifi_cards[0]); i++)
		{
			if(wlan_dev->vendor_id == known_wifi_cards[i].vendor_id &&
			   wlan_dev->device_id == known_wifi_cards[i].device_id)
			{
				verbose("Setting up wifi keys\n");
				
				devprop_add_value(device, "model", (uint8_t*)known_wifi_cards[i].model, (strlen(known_wifi_cards[i].model) + 1));
				
				// NOTE: I would set the subsystem id and subsystem vendor id here,
				// however, those values seem to be ovverriden in the boot process.
				// A batter method would be injecting the DTGP dsdt method
				// and then injectinve the subsystem id there.
				
				stringdata = (uint8_t*)malloc(sizeof(uint8_t) * string->length);
				if(stringdata)
				{
					memcpy(stringdata, (uint8_t*)devprop_generate_string(string), string->length);
					stringlength = string->length;
				}
				
				return;
				
			}
		}		
	}
}
