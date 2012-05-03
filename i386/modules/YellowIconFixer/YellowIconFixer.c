/*
 * Copyright (c) 2011,2012 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#include "libsaio.h"
#include "modules.h"
#include "bootstruct.h"
#include "pci.h"
#include "device_inject.h"

#ifndef DEBUG_SATA
#define DEBUG_SATA 0
#endif

#if DEBUG_SATA
#define DBG(x...) printf(x)
#else
#define DBG(x...)
#endif
	
void SATA_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);

uint8_t default_SATA_ID[]= {
	0x81, 0x26, 0x00, 0x00
};
#define SATA_ID_LEN ( sizeof(default_SATA_ID) / sizeof(uint8_t) )

void SATA_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	pci_dt_t* current = arg1;
	struct DevPropDevice		*device;
	char						*devicepath;
	if (current && current->class_id == PCI_CLASS_STORAGE_SATA)
	{
		devicepath = get_pci_dev_path(current);
		if (devicepath)
		{
			verbose("SATA device : [%04x:%04x :: %04x] :: %s\n",  
					current->vendor_id, current->device_id,current->class_id,
					devicepath);
			
			if (!string)
			{
				string = devprop_create_string();
			}
			device = devprop_add_device(string, devicepath);
			
			devprop_add_value(device, "device-id", default_SATA_ID, SATA_ID_LEN);
						
			
			stringdata = malloc(sizeof(uint8_t) * string->length);
			memcpy(stringdata, (uint8_t*)devprop_generate_string(string), string->length);
			stringlength = string->length;
			
		}
	}	
	
}

void YellowIconFixer_start(void);
void YellowIconFixer_start(void)
{
    register_hook_callback("PCIDevice", &SATA_hook);
}

