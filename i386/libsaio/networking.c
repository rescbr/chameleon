/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 * Merge into file from module compcept by ErmaC and Marchrius 
 *
 */

#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
#include "platform.h"
#include "device_inject.h"
#include "networking.h"

#ifndef DEBUG_NETWORKING
	#define DEBUG_NETWORKING 0
#endif

#if DEBUG_NETWORKING
	#define DBG(x...)	printf(x)
#else
	#define DBG(x...)
#endif

uint32_t	builtin_set	= 0;
extern uint32_t devices_number;

int devprop_add_network_template(DevPropDevice *device, uint16_t vendor_id)
{
	if(!device)
	{
		return 0;
	}

	uint8_t builtin = 0x0;
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

void setup_eth_builtin(pci_dt_t *eth_dev)
{
	char *devicepath = get_pci_dev_path(eth_dev);
	DevPropDevice *device = NULL;

	verbose("LAN Controller [%04x:%04x] :: %s\n", eth_dev->vendor_id, eth_dev->device_id, devicepath);

	if(!string)
	{
		string = devprop_create_string();
	}

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

