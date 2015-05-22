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
//#include "convert.h"
#include "device_inject.h"
#include "networking.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define HEADER      __FILE__ " [" TOSTRING(__LINE__) "]: "

#ifndef DEBUG_ETHERNET
	#define DEBUG_ETHERNET 0
#endif

#if DEBUG_ETHERNET
	#define DBG(x...)	printf(x)
#else
	#define DBG(x...)
#endif

#ifndef DEBUG_WLAN
	#define DEBUG_WLAN 0
#endif

#if DEBUG_WLAN
	#define DBG(x...)	printf(x)
#else
	#define DBG(x...)
#endif

uint32_t builtin_set	= 0;
uint8_t builtin		= 0;
extern uint32_t location_number;

static network_device known_ethernet_cards[] =
{
	// Realtek
	{ PCI_VENDOR_ID_REALTEK, 0x8129, "8129 Gigabit Ethernet" },
	{ PCI_VENDOR_ID_REALTEK, 0x8136, "RTL8101E/RTL8102E PCI-E Fast Ethernet Controller" },
	{ PCI_VENDOR_ID_REALTEK, 0x8139, "RTL8139/810x Family Fast Ethernet" },
	{ PCI_VENDOR_ID_REALTEK, 0x8167, "8169/8110 Gigabit Ethernet" },
	{ PCI_VENDOR_ID_REALTEK, 0x8168, "RTL8111/8168 PCI-E Gigabit Ethernet" },
	{ PCI_VENDOR_ID_REALTEK, 0x8169, "8169/8110 Gigabit Ethernet" },

	{ 0x1113, 0x1211, "Realtek RTL8139 Family Fast Ethernet" },
	{ 0x1500, 0x1360, "Realtek RTL8139 Family Fast Ethernet" },
	{ 0x4033, 0x1360, "Realtek RTL8139 Family Fast Ethernet" },
	{ 0x1186, 0x1300, "Realtek RTL8139 Family Fast Ethernet" },
	{ 0x1186, 0x1340, "Realtek RTL8139 Family Fast Ethernet" },
	{ 0x13d1, 0xab06, "Realtek RTL8139 Family Fast Ethernet" },
	{ 0x1259, 0xa117, "Realtek RTL8139 Family Fast Ethernet" },
	{ 0x1259, 0xa11e, "Realtek RTL8139 Family Fast Ethernet" },
	{ 0x14ea, 0xab06, "Realtek RTL8139 Family Fast Ethernet" },
	{ 0x14ea, 0xab07, "Realtek RTL8139 Family Fast Ethernet" },
	{ 0x11db, 0x1234, "Realtek RTL8139 Family Fast Ethernet" },
	{ 0x1432, 0x9130, "Realtek RTL8139 Family Fast Ethernet" },
	{ 0x02ac, 0x1012, "Realtek RTL8139 Family Fast Ethernet" },
	{ 0x018a, 0x0106, "Realtek RTL8139 Family Fast Ethernet" },
	{ 0x126c, 0x1211, "Realtek RTL8139 Family Fast Ethernet" },
	{ 0x1743, 0x8139, "Realtek RTL8139 Family Fast Ethernet" },
	{ 0x021b, 0x8139, "Realtek RTL8139 Family Fast Ethernet" },

	// Marvell
	{ PCI_VENDOR_ID_MARVELL, 0x4320, "Yukon Gigabit Adapter 88E8001 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4340, "Yukon Gigabit Adapter 88E8021 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4341, "Yukon Gigabit Adapter 88E8022 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4342, "Yukon Gigabit Adapter 88E8061 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4343, "Yukon Gigabit Adapter 88E8062 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4344, "Yukon Gigabit Adapter 88E8021 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4345, "Yukon Gigabit Adapter 88E8022 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4346, "Yukon Gigabit Adapter 88E8061 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4347, "Yukon Gigabit Adapter 88E8062 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4350, "Yukon Gigabit Adapter 88E8035 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4351, "Yukon Gigabit Adapter 88E8036 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4352, "Yukon Gigabit Adapter 88E8038 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4353, "Yukon Gigabit Adapter 88E8039 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4354, "Yukon Gigabit Adapter 88E8040 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4355, "Yukon Gigabit Adapter 88E8040T Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4356, "Yukon Gigabit Adapter 88EC033 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4357, "Yukon Gigabit Adapter 88E8042 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x435A, "Yukon Gigabit Adapter 88E8048 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4360, "Yukon Gigabit Adapter 88E8052 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4361, "Yukon Gigabit Adapter 88E8050 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4362, "Yukon Gigabit Adapter 88E8053 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4363, "Yukon Gigabit Adapter 88E8055 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4364, "Yukon Gigabit Adapter 88E8056 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4365, "Yukon Gigabit Adapter 8E8070 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4366, "Yukon Gigabit Adapter 88EC036 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4367, "Yukon Gigabit Adapter 88EC032 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4368, "Yukon Gigabit Adapter 88EC034 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4369, "Yukon Gigabit Adapter 88EC042 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x436A, "Yukon Gigabit Adapter 88E8058 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x436B, "Yukon Gigabit Adapter 88E8071 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x436C, "Yukon Gigabit Adapter 88E8072 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x436D, "Yukon Gigabit Adapter 88E8055 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4370, "Yukon Gigabit Adapter 88E8075 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4380, "Yukon Gigabit Adapter 88E8057 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4381, "Yukon Gigabit Adapter 88E8059 Singleport Copper SA" },
	{ PCI_VENDOR_ID_MARVELL, 0x4382, "Yukon Gigabit Adapter 88E8079 Singleport Copper SA" },
//	{ PCI_VENDOR_ID_MARVELL, 0x5005, "Belkin F5D5005 Gigabit Desktop Network PCI Card" },

	// Broadcom
	{ PCI_VENDOR_ID_BROADCOM, 0x1600, "BCM5752 Gigabit Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1655, "BCM5717 Gigabit Ethernet PCIe" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1656, "BCM5718 Gigabit Ethernet PCIe" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1657, "BCM5719 Gigabit Ethernet PCIe" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1659, "BCM5721 Gigabit Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x165A, "BCM5722 Gigabit Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x166A, "BCM5780 Gigabit Ethernet" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1672, "BCM5754M Gigabit Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1673, "BCM5755M Gigabit Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x167A, "BCM5754 Gigabit Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x167B, "BCM5755 Gigabit Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1684, "BCM5764M Gigabit Ethernet PCIe" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1691, "BCM57788 Gigabit Ethernet PCIe" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1693, "BCM5787M Gigabit Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x169B, "BCM5787 Gigabit Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x16B4, "BCM57765 Gigabit Ethernet PCIe" },
	{ PCI_VENDOR_ID_BROADCOM, 0x16B5, "BCM57785 Gigabit Ethernet PCIe" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1712, "BCM5906 Fast Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1713, "BCM5906M Fast Ethernet PCI Express" },

	// JMicron
	{ PCI_VENDOR_ID_JMICRON, 0x0250, "JMC250 PCI Express Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_JMICRON, 0x0260, "JMC260 PCI Express Gigabit Ethernet Controller" },

	// Intel
	{ PCI_VENDOR_ID_INTEL, 0x1000, "82542" },
	{ PCI_VENDOR_ID_INTEL, 0x1029, "8255x" },
	{ PCI_VENDOR_ID_INTEL, 0x1030, "8255x" },
	{ PCI_VENDOR_ID_INTEL, 0x1031, "8255x" },
	{ PCI_VENDOR_ID_INTEL, 0x1032, "8255x" },
	{ PCI_VENDOR_ID_INTEL, 0x1033, "8255x" },
	{ PCI_VENDOR_ID_INTEL, 0x1034, "8255x" },
	{ PCI_VENDOR_ID_INTEL, 0x1038, "8255x" },
	{ PCI_VENDOR_ID_INTEL, 0x1039, "8255x" },
	{ PCI_VENDOR_ID_INTEL, 0x103A, "8255x" },
	{ PCI_VENDOR_ID_INTEL, 0x103B, "8255x" },
	{ PCI_VENDOR_ID_INTEL, 0x103C, "8255x" },
	{ PCI_VENDOR_ID_INTEL, 0x103D, "8255x" },
	{ PCI_VENDOR_ID_INTEL, 0x103E, "8255x" },
	{ PCI_VENDOR_ID_INTEL, 0x1049, "82566MM" },
	{ PCI_VENDOR_ID_INTEL, 0x104A, "82566DM" },
	{ PCI_VENDOR_ID_INTEL, 0x104B, "82574L" },
	{ PCI_VENDOR_ID_INTEL, 0x104C, "82562V" },
	{ PCI_VENDOR_ID_INTEL, 0x104D, "82566MC" },
	{ PCI_VENDOR_ID_INTEL, 0x1050, "82562EZ" },
	{ PCI_VENDOR_ID_INTEL, 0x1051, "82801EB/ER" },
	{ PCI_VENDOR_ID_INTEL, 0x1052, "PRO/100 VE Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1053, "PRO/100 VE Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1054, "PRO/100 VE Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1055, "PRO/100 VE Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1056, "PRO/100 VE Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1057, "PRO/100 VE Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1059, "82551QM" },
//	{ PCI_VENDOR_ID_INTEL, 0x105b  "82546GB" },
//	{ PCI_VENDOR_ID_INTEL, 0x105E, "82546GB" },
//	{ PCI_VENDOR_ID_INTEL, 0x105F, "82571EB" },
	{ PCI_VENDOR_ID_INTEL, 0x1060, "82571EB" },
	{ PCI_VENDOR_ID_INTEL, 0x1064, "82562ET/EZ/GT/GZ" },
	{ PCI_VENDOR_ID_INTEL, 0x1065, "82562ET/EZ/GT/GZ" },
	{ PCI_VENDOR_ID_INTEL, 0x1066, "82562 EM/EX/GX" },
	{ PCI_VENDOR_ID_INTEL, 0x1067, "82562 EM/EX/GX" },
	{ PCI_VENDOR_ID_INTEL, 0x1068, "82562ET/EZ/GT/GZ" },
	{ PCI_VENDOR_ID_INTEL, 0x1069, "82562EM/EX/GX" },
	{ PCI_VENDOR_ID_INTEL, 0x106A, "82562G" },
	{ PCI_VENDOR_ID_INTEL, 0x106B, "82562G" },
	{ PCI_VENDOR_ID_INTEL, 0x1075, "82547GI" },
	{ PCI_VENDOR_ID_INTEL, 0x1076, "82541GI" },
	{ PCI_VENDOR_ID_INTEL, 0x1077, "82541GI" },
	{ PCI_VENDOR_ID_INTEL, 0x1078, "82541ER" },
	{ PCI_VENDOR_ID_INTEL, 0x1079, "82546GB" },
	{ PCI_VENDOR_ID_INTEL, 0x107a, "82546GB" },
	{ PCI_VENDOR_ID_INTEL, 0x107b, "82546GB" },
	{ PCI_VENDOR_ID_INTEL, 0x107c, "82541PI" },
//	{ PCI_VENDOR_ID_INTEL, 0x107D, "82572EI" },
//	{ PCI_VENDOR_ID_INTEL, 0x107E, "82572EI" },
//	{ PCI_VENDOR_ID_INTEL, 0x107F, "82572EI Gigabit Ethernet Controller" },
//	{ PCI_VENDOR_ID_INTEL, 0x108a, "82546GB Gigabit Ethernet Controller" },
//	{ PCI_VENDOR_ID_INTEL, 0x108B, "82573V Gigabit Ethernet Controller (Copper)" },
//	{ PCI_VENDOR_ID_INTEL, 0x108C, "82573E Gigabit Ethernet Controller (Copper)" },
	{ PCI_VENDOR_ID_INTEL, 0x1091, "PRO/100 VM Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1092, "PRO/100 VE Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1093, "PRO/100 VM Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1094, "PRO/100 VE Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1095, "PRO/100 VE Network Connection" },
//	{ PCI_VENDOR_ID_INTEL, 0x1096, "80003ES2LAN Gigabit Ethernet Controller (Copper)" },
//	{ PCI_VENDOR_ID_INTEL, 0x1098, "80003ES2LAN Gigabit Ethernet Controller (Serdes)" },
//	{ PCI_VENDOR_ID_INTEL, 0x109A, "82573L Gigabit Ethernet Controller" },
//	{ PCI_VENDOR_ID_INTEL, 0x10A4, "82571EB Gigabit Ethernet Controller" },
//	{ PCI_VENDOR_ID_INTEL, 0x10A5, "82571EB Gigabit Ethernet Controller (Fiber)" },
//	{ PCI_VENDOR_ID_INTEL, 0x10BA, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10BC, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10B9, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10BB, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10BD, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10BF, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10C0, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10C2, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10C3, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10C4, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10C5, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10CB, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10CC, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10CD, "Intel " },
	{ PCI_VENDOR_ID_INTEL, 0x10CE, "82567V-2 Gigabit Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x10D3, "82574L Gigabit Network Connection" },
//	{ PCI_VENDOR_ID_INTEL, 0x10D5, "Intel " },
	{ PCI_VENDOR_ID_INTEL, 0x10d6, "82575GB Gigabit Network Connection" },
//	{ PCI_VENDOR_ID_INTEL, 0x10D9, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10DA, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10DE, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10DF, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10E5, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10EA, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10EB, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10EF, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10F5, "Intel " },
	{ PCI_VENDOR_ID_INTEL, 0x10F6, "82574L" },
	{ PCI_VENDOR_ID_INTEL, 0x10F0, "82578DC Gigabit Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x10FE, "82552 10/100 Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1209, "8255xER/82551IT Fast Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x1227, "82865 EtherExpress PRO/100A" },
	{ PCI_VENDOR_ID_INTEL, 0x1228, "82556 EtherExpress PRO/100 Smart" },
	{ PCI_VENDOR_ID_INTEL, 0x1229, "82557/8/9/0/1 Ethernet Pro 100" },
//	{ PCI_VENDOR_ID_INTEL, 0x1501, "82567V-3 Gigabit Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1502, "82579LM Gigabit Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1503, "82579V Gigabit Network Controller" },
//	{ PCI_VENDOR_ID_INTEL, 0x150C, "82583V Gigabit Network Connection" },
//	{ PCI_VENDOR_ID_INTEL, 0x1525, "82567V-4 Gigabit Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x153A, "Ethernet Connection I217-LM" },
	{ PCI_VENDOR_ID_INTEL, 0x153B, "Ethernet Connection I217-V" },
	{ PCI_VENDOR_ID_INTEL, 0x1559, "Ethernet Connection I218-V" },
	{ PCI_VENDOR_ID_INTEL, 0x155A, "Ethernet Connection I218-LM" },
	{ PCI_VENDOR_ID_INTEL, 0x15A0, "Ethernet Connection (2) I218-LM" },
	{ PCI_VENDOR_ID_INTEL, 0x15A1, "Ethernet Connection (2) I218-V" },
	{ PCI_VENDOR_ID_INTEL, 0x15A2, "Ethernet Connection (3) I218-LM" },
	{ PCI_VENDOR_ID_INTEL, 0x15A3, "Ethernet Connection (3) I218-V" },
	{ PCI_VENDOR_ID_INTEL, 0x2449, "82801BA/BAM/CA/CAM Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x2459, "82801E Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x245D, "82801E Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x27DC, "NM10/ICH7 Family LAN Controller" },
//	{ PCI_VENDOR_ID_INTEL, 0x294C, "82566DC-2 Gigabit Network Connection" },

//	Atheros (Qualcomm)
	{ PCI_VENDOR_ID_QUALCOMM, 0x1026, "AR8121/AR8113/AR8114 Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x1062, "AR8132 Fast Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x1063, "AR8131 Gigabit Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x1066, "AR8121/AR8113/AR8114 Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x1067, "L1c Gigabit Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x1073, "AR8151 v1.0 Gigabit 1000" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x1083, "GbE LAN chip (10/100/1000 Mbit)" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x1090, "AR8162 Fast Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x1091, "AR8161 Gigabit Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x10a0, "QCA8172 Fast Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x10a1, "QCA8171 Gigabit Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x2048, "L2 Fast Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x2060, "AR8152 v1.1 Fast 10/100" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x2062, "AR8152 v2.0 Fast 10/100" }
};
#define ETH_DEVICES_LEN (sizeof(known_ethernet_cards) / sizeof(known_ethernet_cards[0]))

static network_device known_wifi_cards[] =
{
	// Broadcom
	{PCI_VENDOR_ID_BROADCOM, 0x4312, "BCM4311 802.11a/b/g"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4313, "BCM4311 802.11a" },
	{PCI_VENDOR_ID_BROADCOM, 0x4315, "BCM4312 802.11b/g Wireless LAN Controller"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4318, "BCM4318 [AirForce One 54g] 802.11g Wireless LAN Controller"},
	{PCI_VENDOR_ID_BROADCOM, 0x4319, "BCM4318 [AirForce 54g] 802.11a/b/g PCI Express Transceiver"},
	{PCI_VENDOR_ID_BROADCOM, 0x4320, "BCM4306 802.11b/g Wireless LAN Controller"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4321, "BCM4321 802.11a Wireless Network Controller"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4322, "BCM4322 802.11bgn Wireless Network Controller"},
	{PCI_VENDOR_ID_BROADCOM, 0x4324, "BCM4309 802.11abg Wireless Network Controller"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4325, "BCM4306 802.11bg Wireless Network Controller"},
	{PCI_VENDOR_ID_BROADCOM, 0x4328, "BCM4321 802.11a/b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4329, "BCM4321 802.11b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x432a, "BCM4321 802.11an Wireless Network Controller"},
	{PCI_VENDOR_ID_BROADCOM, 0x432b, "BCM4322 802.11a/b/g/n Wireless LAN Controller"},
	{PCI_VENDOR_ID_BROADCOM, 0x432c, "BCM4322 802.11b/g/n"},
	{PCI_VENDOR_ID_BROADCOM, 0x432d, "BCM4322 802.11an Wireless Network Controller"},
	{PCI_VENDOR_ID_BROADCOM, 0x4331, "BCM4331 802.11a/b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4350, "BCM43222 Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4351, "BCM43222 802.11abgn Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4353, "BCM43224 802.11a/b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4357, "BCM43225 802.11b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4358, "BCM43227 802.11b/g/n"},
	{PCI_VENDOR_ID_BROADCOM, 0x4359, "BCM43228 802.11a/b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4360, "BCM4360 802.11ac Wireless Network Adapter"},
	{PCI_VENDOR_ID_BROADCOM, 0x4365, "BCM43142 802.11b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43a0, "BCM4360 802.11ac Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43a1, "BCM4360 802.11ac Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43a2, "BCM4360 802.11ac Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43a9, "BCM43217 802.11b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43aa, "BCM43131 802.11b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43b1, "BCM4352 802.11ac Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43ba, "BCM43602 802.11ac Wireless LAN SoC"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43bb, "BCM43602 802.11ac Wireless LAN SoC"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43bc, "BCM43602 802.11ac Wireless LAN SoC"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43d3, "BCM43567 802.11ac Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43d9, "BCM43570 802.11ac Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43df, "BCM4354 802.11ac Wireless LAN SoC"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43ec, "BCM4356 802.11ac Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0xa8d8, "BCM43224/5 Wireless Network Adapter"},

	// Atheros
	{PCI_VENDOR_ID_ATHEROS, 0x0020, "AR5513 802.11abg Wireless NIC"},
	{PCI_VENDOR_ID_ATHEROS, 0x0023, "AR5416 Wireless Network Adapter"},
	{PCI_VENDOR_ID_ATHEROS, 0x0024, "AR5418 Wireless Network Adapter"},
	{PCI_VENDOR_ID_ATHEROS, 0x0027, "AR9160 Wireless Network Adapter"},
	{PCI_VENDOR_ID_ATHEROS, 0x0029, "AR922X Wireless Network Adapter"},
	{PCI_VENDOR_ID_ATHEROS, 0x002A, "AR928X Wireless Network Adapter"}, // "pci168c,2a"
	{PCI_VENDOR_ID_ATHEROS, 0x002B, "AR9285 Wireless Network Adapter"},
	{PCI_VENDOR_ID_ATHEROS, 0x002C, "AR2427 Wireless Network Adapter"},
	{PCI_VENDOR_ID_ATHEROS, 0x002D, "AR9227 Wireless Network Adapter"},
	{PCI_VENDOR_ID_ATHEROS, 0x002E, "AR9287 Wireless Network Adapter"},
	{PCI_VENDOR_ID_ATHEROS, 0x0030, "AR93xx Wireless Network Adapter"},
	{PCI_VENDOR_ID_ATHEROS, 0x0032, "AR9485 Wireless Network Adapter"},
	{PCI_VENDOR_ID_ATHEROS, 0x0033, "AR9580 Wireless Network Adapter"},
	{PCI_VENDOR_ID_ATHEROS, 0x0034, "AR9462 Wireless Network Adapter"},
	{PCI_VENDOR_ID_ATHEROS, 0x0036, "AR9565 Wireless Network Adapter"},
	{PCI_VENDOR_ID_ATHEROS, 0x0037, "AR9485 Wireless Network Adapter"}
};
#define WLAN_DEVICES_LEN (sizeof(known_wifi_cards) / sizeof(known_wifi_cards[0]))

char *get_ethernet_model(uint32_t vendor_id, uint32_t device_id)
{
	static char desc[128];

	const char *name_format  = NULL;
	int i;

	/* Get format for vendor ID */
	switch ( vendor_id )
	{
		case PCI_VENDOR_ID_REALTEK:
			name_format = "Realteck %s"; break;

		case PCI_VENDOR_ID_MARVELL:
			name_format = "Marvell %s"; break;

		case PCI_VENDOR_ID_JMICRON:
			name_format = "JMicron %s"; break;

		case PCI_VENDOR_ID_ATHEROS:
		case PCI_VENDOR_ID_QUALCOMM:
			name_format = "Atheros %s"; break;

		case PCI_VENDOR_ID_BROADCOM:
			name_format = "Broadcom %s"; break;

		case PCI_VENDOR_ID_INTEL:
			name_format = "Intel %s"; break;

		default:
			break;
	}

	for( i = 0 ; i < ETH_DEVICES_LEN; i++ )
	{
		if ( ( vendor_id == known_ethernet_cards[i].vendor_id ) && ( device_id == known_ethernet_cards[i].device_id ) )
		{
			snprintf( desc, sizeof(desc), name_format, known_ethernet_cards[i].model );
			return desc;
		}
	}
	// NOT in know table... assign generic
	/* Not in table */
	snprintf( desc, sizeof(desc), name_format, "Ethernet Controller" );
	return desc;
}

char *get_wlan_model(uint32_t vendor_id, uint32_t device_id)
{

	static char desc[128];

	const char *name_format  = NULL;
	int i;

	/* Get format for vendor ID */
	switch ( vendor_id )
	{
		case PCI_VENDOR_ID_REALTEK:
			name_format = "Realteck %s"; break;

		case PCI_VENDOR_ID_ATHEROS:
		case PCI_VENDOR_ID_QUALCOMM:
			name_format = "Atheros %s"; break;

		case PCI_VENDOR_ID_BROADCOM:
			name_format = "Broadcom %s"; break;

		default:
			break;
	}

	for( i = 0 ; i < WLAN_DEVICES_LEN; i++ )
	{
		if ( ( vendor_id == known_wifi_cards[i].vendor_id ) && ( device_id == known_wifi_cards[i].device_id ) )
		{
			snprintf( desc, sizeof(desc), name_format, known_wifi_cards[i].model );
			return desc;
		}
	}
	// NOT in know table... assign generic
	/* Not in table */
	snprintf( desc, sizeof(desc), name_format, "Wireless Controller" );
	return desc;
}

void setup_eth_devdrop(pci_dt_t *eth_dev)
{
	char *devicepath = get_pci_dev_path(eth_dev);
	char *name_model = NULL;
	builtin = 0;

	bool do_eth_devprop = getBoolForKey(kEthernetBuiltIn, &do_eth_devprop, &bootInfo->chameleonConfig);

	DevPropDevice *device = (DevPropDevice *)malloc(sizeof(DevPropDevice));

	verbose("\tClass code: [%04X]\n", eth_dev->class_id);
	verbose("\tEthernetBuiltIn = %s\n", do_eth_devprop ? "Yes" : "No");

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
		name_model = get_ethernet_model(eth_dev->vendor_id, eth_dev->device_id);

		if ( do_eth_devprop )
		{
			verbose("\tLocation number: %d\n", location_number);
			verbose("\tSetting up lan keys\n");
			if( ( eth_dev->vendor_id != PCI_VENDOR_ID_ATHEROS ) && ( builtin_set == 0 ))
			{
				builtin_set = 1;
				builtin = 0x01;
			}

			devprop_add_value(device, "location", (uint8_t *)&location_number, 1);

//			devprop_add_value(device, "AAPL,slot-name", "Slot-x"
//			devprop_add_value(device, "device-id",
//			devprop_add_value(device, "revision-id",
//			devprop_add_value(device, "subsystem-id",
//			devprop_add_value(device, "subsystem-vendor-id",

			devprop_add_value(device, "built-in", (uint8_t *)&builtin, 1);
			devprop_add_value(device, "model", (uint8_t *)name_model, (strlen(name_model) + 1));
			devprop_add_value(device, "device_type", (uint8_t *)"ethernet", sizeof("Ethernet"));

			stringdata = (uint8_t *)malloc(sizeof(uint8_t) * string->length);
			if(stringdata)
			{
				memcpy(stringdata, (uint8_t *)devprop_generate_string(string), string->length);
				stringlength = string->length;
			}

		}
	}

	verbose("\t%s [%04x:%04x]\n\t%s\n",name_model, eth_dev->vendor_id, eth_dev->device_id, devicepath);

	// location number
	location_number++;
}

void setup_wifi_devdrop(pci_dt_t *wlan_dev) // ARPT
{
	char *devicepath = get_pci_dev_path(wlan_dev);
	char *name_model = NULL;

	builtin = 0;

	bool do_wifi_devprop = getBoolForKey(kEnableWifi, &do_wifi_devprop, &bootInfo->chameleonConfig);

	DevPropDevice *device = (DevPropDevice *)malloc(sizeof(DevPropDevice));

	verbose("\tClass code: [%04X]\n", wlan_dev->class_id);
	verbose("\tEnableWifi = %s\n", do_wifi_devprop ? "Yes" : "No");

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
		name_model = get_wlan_model(wlan_dev->vendor_id, wlan_dev->device_id);

		if ( do_wifi_devprop )
		{
			verbose("\tSetting up wifi keys\n");
			devprop_add_value(device, "built-in", (uint8_t *)&builtin, 1);
			devprop_add_value(device, "model", (uint8_t *)name_model, (strlen(name_model) + 1));
//			devprop_add_value(device, "name", (uint8_t *)"AirPort Extreme", sizeof("AirPort Extreme"));

//			devprop_add_value(device, "vendor-id",
//			devprop_add_value(device, "device-id",
//			devprop_add_value(device, "subsystem-id",
//			devprop_add_value(device, "subsystem-vendor-id",
			// NOTE: I would set the subsystem id and subsystem vendor id here,
			// however, those values seem to be ovverriden in the boot process.
			// A better method would be injecting the DTGP dsdt method
			// and then injecting the subsystem id there.

			devprop_add_value(device, "device_type", (uint8_t *)"Airport", sizeof("Airport"));
			devprop_add_value(device, "AAPL,slot-name", (uint8_t *)"Airport", sizeof("Airport"));

			stringdata = (uint8_t *)malloc(sizeof(uint8_t) *string->length);
			if(stringdata)
			{
				memcpy(stringdata, (uint8_t *)devprop_generate_string(string), string->length);
				stringlength = string->length;
			}
		}
	}

	verbose("\t%s [%04x:%04x]\n\t%s\n", name_model, wlan_dev->vendor_id, wlan_dev->device_id, devicepath);
}
