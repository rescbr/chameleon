/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include "libsaio.h"
#include "modules.h"
#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
#include "hpet.h"

#ifndef DEBUG_HPET
#define DEBUG_HPET 0
#endif

#if DEBUG_HPET
#define DBG(x...) printf(x)
#else
#define DBG(x...)
#endif

void set_eth_builtin(pci_dt_t *eth_dev);


void Ethernet_hook(void* arg1, void* arg2, void* arg3, void* arg4)
{
	pci_dt_t* current = arg1;
	
	if(current->class_id != PCI_CLASS_NETWORK_ETHERNET) return;
	
	
	bool do_eth_devprop = false;	
	getBoolForKey(kEthernetBuiltIn, &do_eth_devprop, &bootInfo->bootConfig);
	
	if (do_eth_devprop)
		set_eth_builtin(current);
}

void Ethernet_start()
{
	register_hook_callback("PCIDevice", &Ethernet_hook);
}

/* a fine place for this code */

int devprop_add_network_template(struct DevPropDevice *device, uint16_t vendor_id)
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

void set_eth_builtin(pci_dt_t *eth_dev)
{
	char *devicepath = get_pci_dev_path(eth_dev);
	struct DevPropDevice *device = (struct DevPropDevice*)malloc(sizeof(struct DevPropDevice));
	
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
