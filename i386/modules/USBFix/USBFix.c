/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include "libsaio.h"
#include "modules.h"
#include "pci.h"

int usb_loop();
void notify_usb_dev(pci_dt_t *pci_dev);

void USBFix_pci_hook(pci_dt_t* current, void* arg2, void* arg3, void* arg4)
{
	if(current->class_id != PCI_CLASS_SERIAL_USB) return;

	notify_usb_dev(current);
}

void USBFix_start_hook(void* arg1, void* arg2, void* arg3, void* arg4)
{
	usb_loop();
}


void USBFix_start()
{
	register_hook_callback("PCIDevice", &USBFix_pci_hook);
	register_hook_callback("Kernel Start", &USBFix_start_hook);

}

