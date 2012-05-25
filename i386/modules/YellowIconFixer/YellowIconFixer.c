/*
 * Copyright (c) 2011,2012 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#include "libsaio.h"
#include "modules.h"
#include "bootstruct.h"
#include "pci.h"
#include "device_inject.h"
#include "platform.h"

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
    struct DevPropString        *string;
	if (current && current->class_id == PCI_CLASS_STORAGE_SATA)
	{
		devicepath = get_pci_dev_path(current);
		if (devicepath)
		{			
            
            string = (struct DevPropString *)(uint32_t)get_env(envEFIString);

			if (!string)
			{
				string = devprop_create_string();
                if (!string) return;
                safe_set_env(envEFIString,(uint32_t)string);
			}
			device = devprop_add_device(string, devicepath);
            if (!device) return;

#if PROOFOFCONCEPT
            uint16_t vendor_id =  current->vendor_id & 0xFFFF;
            uint16_t device_id =  current->device_id & 0xFFFF;
            
			devprop_add_value(device, "vendor-id", (uint8_t*)&vendor_id, 4);
			devprop_add_value(device, "device-id", (uint8_t*)&device_id, 4);
#else
            devprop_add_value(device, "device-id", default_SATA_ID, SATA_ID_LEN);

#endif          
			verbose("SATA device : [%04x:%04x :: %04x] :: %s\n",  
					current->vendor_id, current->device_id,current->class_id,
					devicepath);			
		}
	}	
	
}

void YellowIconFixer_start(void);
void YellowIconFixer_start(void)
{
    register_hook_callback("PCIDevice", &SATA_hook);
}

