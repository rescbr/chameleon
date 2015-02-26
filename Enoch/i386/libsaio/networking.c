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

uint32_t builtin_set	= 0;
uint8_t builtin		= 0;
extern uint32_t devices_number;
//extern uint32_t onboard_number;

int devprop_add_network_template(DevPropDevice *device, uint16_t vendor_id)
{
	builtin = 0;
	if(device)
	{

		if((vendor_id != 0x168c) && (builtin_set == 0))
		{
			builtin_set = 1;
			builtin = 0x01;
		}

		if(!devprop_add_value(device, "built-in", (uint8_t *)&builtin, 1))
		{
			return 0;
		}

		if(!devprop_add_value(device, "device_type", (uint8_t *)"Ethernet Controller", 20))
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

static network_device known_ethernet_cards[] =
{
//    { 0x0000, 0x0000, "Unknown" },
//    8169
//    { 0x10EC, 0x8169, "Realtek 8169/8110 Gigabit Ethernet" },
	{ 0x10EC, 0x8168, "Realtek RTL8111/8168 PCI-E Gigabit Ethernet" },
//    { 0x10EC, 0x8167, "Realtek 8169/8110 Gigabit Ethernet" },
	{ 0x10EC, 0x8136, "Realtek RTL8101E/RTL8102E PCI-E Fast Ethernet Controller" },
//    8139
//    { 0x10EC, 0x8139, "Realtek RTL8139/810x Family Fast Ethernet" },
//    { 0x1186, 0x1300, "Realtek RTL8139/810x Family Fast Ethernet" },
//    { 0x1113, 0x1211, "Realtek RTL8139/810x Family Fast Ethernet" },
	{ 0x11AB, 0x4320, "Marvell Yukon Gigabit Adapter 88E8001 Singleport Copper SA" },
	{ 0x11AB, 0x4364, "Marvell Yukon Gigabit Adapter 88E8056 Singleport Copper SA" },
//    Broadcom 57XX
//    { 0x14e4, 0x1600, "Broadcom 5751 Ethernet" },
//    { 0x14e4, 0x1659, "Broadcom 57XX Ethernet" },
//    { 0x14e4, 0x165A, "BCM5722 NetXtreme Server Gigabit Ethernet" },
//    { 0x14e4, 0x166A, "Broadcom 57XX Ethernet" },
//    { 0x14e4, 0x1672, "BCM5754M NetXtreme Gigabit Ethernet" },
//    { 0x14e4, 0x1673, "BCM5755M NetXtreme Gigabit Ethernet" },
//    { 0x14e4, 0x167A, "BCM5754 NetXtreme Gigabit Ethernet" },
//    { 0x14e4, 0x167B, "BCM5755 NetXtreme Gigabit Ethernet" },
//    { 0x14e4, 0x1684, "Broadcom 57XX Ethernet" },
//    { 0x14e4, 0x1691, "BCM57788 NetLink (TM) Gigabit Ethernet" },
//    { 0x14e4, 0x1693, "BCM5787M NetLink (TM) Gigabit Ethernet" },
//    { 0x14e4, 0x169B, "BCM5787 NetLink (TM) Gigabit Ethernet" },
//    { 0x14e4, 0x16B4, "Broadcom 57XX Ethernet" },
//    { 0x14e4, 0x16B5, "BCM57785 Gigabit Ethernet PCIe" },
//    { 0x14e4, 0x1712, "BCM5906 NetLink (TM) Fast Ethernet" },
//    { 0x14e4, 0x1713, "BCM5906M NetLink (TM) Fast Ethernet" },
	{ 0x1969, 0x1026, "Atheros AR8121/AR8113/AR8114 Ethernet" },
	{ 0x1969, 0x1083, "Atheros GbE LAN chip (10/100/1000 Mbit)" },
	{ 0x197B, 0x0250, "JMicron PCI Express Gigabit Ethernet Adapter (10/1000MBit)" },
//    Intel 8255x Ethernet
//    { 0x8086, 0x1051, "Intel 8255x Ethernet" },
//    { 0x8086, 0x1050, "Intel 8255x Ethernet" },
//    { 0x8086, 0x1029, "Intel 8255x Ethernet" },
//    { 0x8086, 0x1030, "Intel 8255x Ethernet" },
	{ 0x8086, 0x10CE, "Intel(R) 82567V-2 PCI-E Gigabit Network" },
	{ 0x8086, 0x10D3, "Intel(R) 82574L Gigabit Network Connection" },
//    { 0x8086, 0x1209, "Intel 8255x Ethernet" },
//    { 0x8086, 0x1227, "Intel 8255x Ethernet" },
//    { 0x8086, 0x1228, "Intel 8255x Ethernet" },
//    { 0x8086, 0x1229, "Intel 8255x Ethernet" },
//    { 0x8086, 0x1503, "Intel 82579V Gigabit Network Controller" },
//    { 0x8086, 0x2449, "Intel 8255x Ethernet" },
//    { 0x8086, 0x2459, "Intel 8255x Ethernet" },
//    { 0x8086, 0x245D, "Intel 8255x Ethernet" },
//    { 0x8086, 0x1091, "Intel 8255x Ethernet" },
//    { 0x8086, 0x1060, "Intel 8255x Ethernet" },
//    Atheros AR8151 Ethernet
//    { 0x1969, 0x1083, "Qualcomm Atheros AR8151 v2.0 Gigabit Ethernet" },
};

static network_device generic_ethernet_cards[] =
{
	{ 0x0000, 0x0000, "Generic Ethernet Controller" },
	{ 0x10EC, 0x0000, "Realtek Ethernet Controller" },
	{ 0x11AB, 0x0000, "Marvell Ethernet Controller" },
	{ 0x1969, 0x0000, "Atheros Ethernet Controller" },
	{ 0x8086, 0x0000, "Intel(R) Ethernet Controller" },
};

char *get_ethernet_model(uint32_t vendor_id, uint32_t device_id)
{
	int i = 0;
	for( ; i < sizeof(known_ethernet_cards) / sizeof(known_ethernet_cards[0]); i++)
	{
		if(vendor_id == known_ethernet_cards[i].vendor_id && device_id == known_ethernet_cards[i].device_id)
		{
			return known_ethernet_cards[i].model;
		}
	}
	i = 0;
	for ( ; i < sizeof(generic_ethernet_cards) / sizeof(generic_ethernet_cards[0]); i++)
	{
		if (vendor_id == generic_ethernet_cards[i].vendor_id)
		{
			return generic_ethernet_cards[i].model;
		}
	}
	return generic_ethernet_cards[0].model;
}

void setup_eth_builtin(pci_dt_t *eth_dev)
{
	char *devicepath	= get_pci_dev_path(eth_dev);
	char *name_model	= NULL;

	DevPropDevice *device = (DevPropDevice *)malloc(sizeof(DevPropDevice));

	verbose("LAN Controller [%04x:%04x] :: %s\n", eth_dev->vendor_id, eth_dev->device_id, devicepath);

	if (!string)
	{
		string = devprop_create_string();
		if (!string)
		{
			return;
		}
	}
    
	device = devprop_add_device(string, devicepath);
	if(device)
	{
		verbose("Setting up lan keys\n");
		name_model = get_ethernet_model(eth_dev->vendor_id, eth_dev->device_id);

		devprop_add_network_template(device, eth_dev->vendor_id);
		devprop_add_value(device, "model", (uint8_t *)name_model, (strlen(name_model) + 1));
		devprop_add_value(device, "device_type", (uint8_t *)"Ethernet Controller", 20);

		stringdata = (uint8_t*)malloc(sizeof(uint8_t) * string->length);
		if(stringdata)
		{
			memcpy(stringdata, (uint8_t *)devprop_generate_string(string), string->length);
			stringlength = string->length;
		}
	}
}

static network_device known_wifi_cards[] =
{
	{0x14e4, 0x4312, "Broadcom BCM4311 802.11a/b/g"},
	{0x14e4, 0x4315, "Broadcom BCM4312 802.11b/g Wireless LAN Controller"},
	{0x14e4, 0x4319, "Broadcom BCM4318 [AirForce 54g] 802.11a/b/g PCI Express Transceiver"},
	{0x14e4, 0x432b, "Broadcom BCM4322 802.11a/b/g/n Wireless LAN Controller"},
	{0x14e4, 0x432c, "Broadcom BCM4322 802.11b/g/n"},
	{0x14e4, 0x4331, "Broadcom BCM4331 802.11a/b/g/n"},
	{0x14e4, 0x4359, "Broadcom BCM43228 802.11a/b/g/n"},
	{0x168C, 0x0020, "Atheros AR5513 802.11abg Wireless NIC"},
	{0x168C, 0x0023, "Atheros AR5416 Wireless Network Adapter [AR5008 802.11(a)bgn]"},
	{0x168C, 0x0024, "Atheros AR5418 Wireless Network Adapter [AR5008E 802.11(a)bgn]"},
	{0x168C, 0x0027, "Atheros AR9160 Wireless Network Adapter [AR9001 802.11(a)bgn]"},
	{0x168C, 0x0029, "Atheros AR922X Wireless Network Adapter"},
	{0x168C, 0x002A, "Atheros AR928X Wireless Network Adapter"}, // "pci168c,2a"
	{0x168C, 0x002B, "Atheros AR9285 Wireless Network Adapter"},
};

void setup_wifi_airport(pci_dt_t *wlan_dev) // ARPT
{
	char tmp[16];
	builtin = 0;
	DevPropDevice *device ;
	char *devicepath = get_pci_dev_path(wlan_dev);

	verbose("Wifi Controller [%04x:%04x]\n", wlan_dev->vendor_id, wlan_dev->device_id);

	if (!string)
	{
		string = devprop_create_string();
		if (!string)
		{
			return;
		}
	}
    
	device = devprop_add_device(string, devicepath);
	if(device)
	{
		snprintf(tmp, sizeof(tmp),"Airport");
		devprop_add_value(device, "AAPL,slot-name", (uint8_t *) tmp, strlen(tmp) + 1);
		devprop_add_value(device, "device_type", (uint8_t *) tmp, strlen(tmp) + 1);
		devprop_add_value(device, "built-in", (uint8_t *)&builtin, 1);

		unsigned int i = 0;
		for( ; i < sizeof(known_wifi_cards) / sizeof(known_wifi_cards[0]); i++)
		{
			if(wlan_dev->vendor_id == known_wifi_cards[i].vendor_id && wlan_dev->device_id == known_wifi_cards[i].device_id)
			{
				verbose("Setting up wifi keys\n");

				devprop_add_value(device, "model", (uint8_t *)known_wifi_cards[i].model, (strlen(known_wifi_cards[i].model) + 1));
				// NOTE: I would set the subsystem id and subsystem vendor id here,
				// however, those values seem to be ovverriden in the boot process.
				// A better method would be injecting the DTGP dsdt method
				// and then injecting the subsystem id there.
                
				stringdata = (uint8_t *)malloc(sizeof(uint8_t) *string->length);
				if(stringdata)
				{
					memcpy(stringdata, (uint8_t *)devprop_generate_string(string), string->length);
					stringlength = string->length;
				}
				return;
			}
		}
	}
}
