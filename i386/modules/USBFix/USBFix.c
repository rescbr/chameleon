/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include "libsaio.h"
#include "modules.h"
#include "pci.h"
#include "bootstruct.h"

#define kEnableUSBMod			"EnableUSBModule"

extern int usb_loop(void);
extern void notify_usb_dev(pci_dt_t *pci_dev);

void USBFix_pci_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void USBFix_start_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);


void USBFix_pci_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	pci_dt_t* current = arg1;
	if(current->class_id == PCI_CLASS_SERIAL_USB)
	{
		notify_usb_dev(current);
	}
}

void USBFix_start_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	usb_loop();
}

void USBFix_start(void);
void USBFix_start(void)
{
	bool enable = true;
	getBoolForKey(kEnableUSBMod, &enable, DEFAULT_BOOT_CONFIG) ;
	
	if (enable) {
		register_hook_callback("PCIDevice", &USBFix_pci_hook);
		register_hook_callback("Kernel Start", &USBFix_start_hook);
	}
	

}

