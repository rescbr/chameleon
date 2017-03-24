/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 * Merge into file from module compcept by ErmaC and Marchrius 
 *
 */

#ifndef __LIBSAIO_NETWORKING_H
#define __LIBSAIO_NETWORKING_H

void setup_eth_devdrop(pci_dt_t *eth_dev);
void setup_wifi_devdrop(pci_dt_t *wlan_dev);

char *get_ethernet_model(uint32_t vendor_id, uint32_t device_id);
char *get_wlan_model(uint32_t vendor_id, uint32_t device_id);

struct network_device;
typedef struct {
	uint16_t	vendor_id;
	uint16_t	device_id;
	char*		model;
} network_device;

#endif /* !__LIBSAIO_NETWORKING_H */
