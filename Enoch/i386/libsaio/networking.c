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

static network_device known_ethernet_cards[] =
{

	// Realtek
	{ PCI_VENDOR_ID_REALTEK, 0x8129, "Realtek 8129 Gigabit Ethernet" },
	{ PCI_VENDOR_ID_REALTEK, 0x8136, "Realtek RTL8101E/RTL8102E PCI-E Fast Ethernet Controller" },
	{ PCI_VENDOR_ID_REALTEK, 0x8139, "Realtek RTL8139/810x Family Fast Ethernet" },
	{ PCI_VENDOR_ID_REALTEK, 0x8167, "Realtek 8169/8110 Gigabit Ethernet" },
	{ PCI_VENDOR_ID_REALTEK, 0x8168, "Realtek RTL8111/8168 PCI-E Gigabit Ethernet" },
	{ PCI_VENDOR_ID_REALTEK, 0x8169, "Realtek 8169/8110 Gigabit Ethernet" },

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
	{ PCI_VENDOR_ID_MARVELL, 0x4320, "Marvell 88E8001 Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4340, "Marvell 88E8021 PCI-X IPMI Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4341, "Marvell 88E8022 PCI-X IPMI Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4342, "Marvell 88E8061 PCI-E IPMI Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4343, "Marvell 88E8062 PCI-E IPMI Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4344, "Marvell 88E8021 PCI-X IPMI Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4345, "Marvell 88E8022 PCI-X IPMI Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4346, "Marvell 88E8061 PCI-E IPMI Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4347, "Marvell 88E8062 PCI-E IPMI Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4350, "Marvell 88E8035 PCI-E Fast Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4351, "Marvell 88E8036 PCI-E Fast Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4352, "Marvell 88E8038 PCI-E Fast Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4353, "Marvell 88E8039 PCI-E Fast Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4354, "Marvell 88E8040 PCI-E Fast Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4355, "Marvell 88E8040T PCI-E Fast Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4356, "Marvell 88EC033 PCI-E Fast Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4357, "Marvell 88E8042 PCI-E Fast Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x435A, "Marvell 88E8048 PCI-E Fast Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4360, "Marvell 88E8052 PCI-E ASF Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4361, "Marvell 88E8050 PCI-E ASF Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4362, "Marvell 88E8053 PCI-E Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4363, "Marvell 88E8055 PCI-E Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4364, "Marvell 88E8056 PCI-E Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4365, "Marvell 8E8070 based Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4366, "Marvell 88EC036 PCI-E Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4367, "Marvell 88EC032 Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4368, "Marvell 88EC034 Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4369, "Marvell 88EC042 Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x436A, "Marvell 88E8058 PCI-E Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x436B, "Marvell 88E8071 PCI-E Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x436C, "Marvell 88E8072 PCI-E Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x436D, "Marvell 88E8055 PCI-E Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4370, "Marvell 88E8075 PCI-E Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4380, "Marvell 88E8057 PCI-E Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4381, "Marvell 88E8059 PCI-E Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x4382, "Marvell 88E8079 PCI-E Gigabit Ethernet Controller" },
//	{ PCI_VENDOR_ID_MARVELL, 0x5005, "Belkin F5D5005 Gigabit Desktop Network PCI Card" },

	// Broadcom
	{ PCI_VENDOR_ID_BROADCOM, 0x1600, "Broadcom BCM5752 Gigabit Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1655, "Broadcom BCM5717 Gigabit Ethernet PCIe" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1656, "Broadcom BCM5718 Gigabit Ethernet PCIe" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1657, "Broadcom BCM5719 Gigabit Ethernet PCIe" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1659, "Broadcom BCM5721 Gigabit Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x165A, "Broadcom BCM5722 Gigabit Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x166A, "Broadcom BCM5780 Gigabit Ethernet" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1672, "Broadcom BCM5754M Gigabit Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1673, "Broadcom BCM5755M Gigabit Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x167A, "Broadcom BCM5754 Gigabit Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x167B, "Broadcom BCM5755 Gigabit Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1684, "Broadcom BCM5764M Gigabit Ethernet PCIe" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1691, "Broadcom BCM57788 Gigabit Ethernet PCIe" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1693, "Broadcom BCM5787M Gigabit Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x169B, "Broadcom BCM5787 Gigabit Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x16B4, "Broadcom BCM57765 Gigabit Ethernet PCIe" },
	{ PCI_VENDOR_ID_BROADCOM, 0x16B5, "Broadcom BCM57785 Gigabit Ethernet PCIe" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1712, "Broadcom BCM5906 Fast Ethernet PCI Express" },
	{ PCI_VENDOR_ID_BROADCOM, 0x1713, "Broadcom BCM5906M Fast Ethernet PCI Express" },

	// JMicron
	{ PCI_VENDOR_ID_JMICRON, 0x0250, "JMicron JMC250 PCI Express Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_JMICRON, 0x0260, "JMicron JMC260 PCI Express Gigabit Ethernet Controller" },

	// Intel
//	{ PCI_VENDOR_ID_INTEL, 0x1000, "Intel 82542 Gigabit Ethernet Controller (Fiber)" },
	{ PCI_VENDOR_ID_INTEL, 0x1029, "Intel 8255x Ethernet" },
	{ PCI_VENDOR_ID_INTEL, 0x1030, "Intel 8255x Ethernet" },
	{ PCI_VENDOR_ID_INTEL, 0x1031, "Intel 8255x Ethernet" },
	{ PCI_VENDOR_ID_INTEL, 0x1032, "Intel 8255x Ethernet" },
	{ PCI_VENDOR_ID_INTEL, 0x1033, "Intel 8255x Ethernet" },
	{ PCI_VENDOR_ID_INTEL, 0x1034, "Intel 8255x Ethernet" },
	{ PCI_VENDOR_ID_INTEL, 0x1038, "Intel 8255x Ethernet" },
	{ PCI_VENDOR_ID_INTEL, 0x1039, "Intel 8255x Ethernet" },
	{ PCI_VENDOR_ID_INTEL, 0x103A, "Intel 8255x Ethernet" },
	{ PCI_VENDOR_ID_INTEL, 0x103B, "Intel 8255x Ethernet" },
	{ PCI_VENDOR_ID_INTEL, 0x103C, "Intel 8255x Ethernet" },
	{ PCI_VENDOR_ID_INTEL, 0x103D, "Intel 8255x Ethernet" },
	{ PCI_VENDOR_ID_INTEL, 0x103E, "Intel 8255x Ethernet" },
//	{ PCI_VENDOR_ID_INTEL, 0x1049, "Intel 82566MM Gigabit Network Connection" },
//	{ PCI_VENDOR_ID_INTEL, 0x104A, "Intel 82566DM Gigabit Network Connection" },
//	{ PCI_VENDOR_ID_INTEL, 0x104B, "Intel 82566DC Gigabit Network Connection" },
//	{ PCI_VENDOR_ID_INTEL, 0x104C, "Intel 82562V 10/100 Network Connection" },
//	{ PCI_VENDOR_ID_INTEL, 0x104D, "Intel 82566MC Gigabit Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1050, "Intel 82562EZ 10/100 Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x1051, "Intel 82801EB/ER (ICH5/ICH5R) integrated LAN Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x1052, "Intel PRO/100 VE Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1053, "Intel PRO/100 VE Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1054, "Intel PRO/100 VE Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1055, "Intel PRO/100 VE Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1056, "Intel PRO/100 VE Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1057, "Intel PRO/100 VE Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1059, "Intel 82551QM Ethernet Controller" },
//	{ PCI_VENDOR_ID_INTEL, 0x105b  "Intel 82546GB Gigabit Ethernet Controller (Copper)" },
//	{ PCI_VENDOR_ID_INTEL, 0x105E, "Intel 82546GB Gigabit Ethernet Controller" },
//	{ PCI_VENDOR_ID_INTEL, 0x105F, "Intel 82571EB Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x1060, "Intel 82571EB Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x1064, "Intel 82562ET/EZ/GT/GZ - PRO/100 VE (LOM) Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x1065, "Intel 82562ET/EZ/GT/GZ - PRO/100 VE Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x1066, "Intel 82562 EM/EX/GX - PRO/100 VM (LOM) Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x1067, "Intel 82562 EM/EX/GX - PRO/100 VM Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x1068, "Intel 82562ET/EZ/GT/GZ - PRO/100 VE (LOM) Ethernet Controller Mobile" },
	{ PCI_VENDOR_ID_INTEL, 0x1069, "Intel 82562EM/EX/GX - PRO/100 VM (LOM) Ethernet Controller Mobile" },
	{ PCI_VENDOR_ID_INTEL, 0x106A, "Intel 82562G - PRO/100 VE (LOM) Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x106B, "Intel 82562G - PRO/100 VE Ethernet Controller Mobile" },
	{ PCI_VENDOR_ID_INTEL, 0x1075, "82547GI Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x1076, "82541GI Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x1077, "82541GI Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x1078, "82541ER Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x1079, "82546GB Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x107a, "82546GB Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x107b, "82546GB Gigabit Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x107c, "82541PI Gigabit Ethernet Controller" },
//	{ PCI_VENDOR_ID_INTEL, 0x107D, "Intel 82572EI Gigabit Ethernet Controller (Copper)" },
//	{ PCI_VENDOR_ID_INTEL, 0x107E, "Intel 82572EI Gigabit Ethernet Controller (Fiber)" },
//	{ PCI_VENDOR_ID_INTEL, 0x107F, "Intel 82572EI Gigabit Ethernet Controller" },
//	{ PCI_VENDOR_ID_INTEL, 0x108a, "Intel 82546GB Gigabit Ethernet Controller" },
//	{ PCI_VENDOR_ID_INTEL, 0x108B, "Intel 82573V Gigabit Ethernet Controller (Copper)" },
//	{ PCI_VENDOR_ID_INTEL, 0x108C, "Intel 82573E Gigabit Ethernet Controller (Copper)" },
	{ PCI_VENDOR_ID_INTEL, 0x1091, "Intel PRO/100 VM Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1092, "Intel PRO/100 VE Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1093, "Intel PRO/100 VM Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1094, "Intel PRO/100 VE Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1095, "Intel PRO/100 VE Network Connection" },
//	{ PCI_VENDOR_ID_INTEL, 0x1096, "Intel 80003ES2LAN Gigabit Ethernet Controller (Copper)" },
//	{ PCI_VENDOR_ID_INTEL, 0x1098, "Intel 80003ES2LAN Gigabit Ethernet Controller (Serdes)" },
//	{ PCI_VENDOR_ID_INTEL, 0x109A, "Intel 82573L Gigabit Ethernet Controller" },
//	{ PCI_VENDOR_ID_INTEL, 0x10A4, "Intel 82571EB Gigabit Ethernet Controller" },
//	{ PCI_VENDOR_ID_INTEL, 0x10A5, "Intel 82571EB Gigabit Ethernet Controller (Fiber)" },
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
	{ PCI_VENDOR_ID_INTEL, 0x10CE, "Intel 82567V-2 Gigabit Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x10D3, "Intel 82574L Gigabit Network Connection" },
//	{ PCI_VENDOR_ID_INTEL, 0x10D5, "Intel " },
	{ PCI_VENDOR_ID_INTEL, 0x10d6, "Intel 82575GB Gigabit Network Connection" },
//	{ PCI_VENDOR_ID_INTEL, 0x10D9, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10DA, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10DE, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10DF, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10E5, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10EA, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10EB, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10EF, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10F5, "Intel " },
//	{ PCI_VENDOR_ID_INTEL, 0x10F6, "Intel " },
	{ PCI_VENDOR_ID_INTEL, 0x10F0, "Intel 82578DC Gigabit Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x10FE, "Intel 82552 10/100 Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1209, "Intel 8255xER/82551IT Fast Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x1227, "Intel 82865 EtherExpress PRO/100A" },
	{ PCI_VENDOR_ID_INTEL, 0x1228, "Intel 82556 EtherExpress PRO/100 Smart" },
	{ PCI_VENDOR_ID_INTEL, 0x1229, "Intel 82557/8/9/0/1 Ethernet Pro 100" },
//	{ PCI_VENDOR_ID_INTEL, 0x1501, "Intel 82567V-3 Gigabit Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1502, "Intel 82579LM Gigabit Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x1503, "Intel 82579V Gigabit Network Controller" },
//	{ PCI_VENDOR_ID_INTEL, 0x150C, "Intel 82583V Gigabit Network Connection" },
//	{ PCI_VENDOR_ID_INTEL, 0x1525, "Intel 82567V-4 Gigabit Network Connection" },
	{ PCI_VENDOR_ID_INTEL, 0x153A, "Intel Ethernet Connection I217-LM" },
	{ PCI_VENDOR_ID_INTEL, 0x153B, "Intel Ethernet Connection I217-V" },
	{ PCI_VENDOR_ID_INTEL, 0x1559, "Intel Ethernet Connection I218-V" },
	{ PCI_VENDOR_ID_INTEL, 0x155A, "Intel Ethernet Connection I218-LM" },
	{ PCI_VENDOR_ID_INTEL, 0x15A0, "Intel Ethernet Connection (2) I218-LM" },
	{ PCI_VENDOR_ID_INTEL, 0x15A1, "Intel Ethernet Connection (2) I218-V" },
	{ PCI_VENDOR_ID_INTEL, 0x15A2, "Intel Ethernet Connection (3) I218-LM" },
	{ PCI_VENDOR_ID_INTEL, 0x15A3, "Intel Ethernet Connection (3) I218-V" },
	{ PCI_VENDOR_ID_INTEL, 0x2449, "Intel 82801BA/BAM/CA/CAM Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x2459, "Intel 82801E Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x245D, "Intel 82801E Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x27DC, "Intel NM10/ICH7 Family LAN Controller" },
//	{ PCI_VENDOR_ID_INTEL, 0x294C, "Intel 82566DC-2 Gigabit Network Connection" },

//	Atheros (Qualcomm)
	{ PCI_VENDOR_ID_QUALCOMM, 0x1026, "Atheros AR8121/AR8113/AR8114 Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x1062, "Atheros AR8132 Fast Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x1063, "Atheros AR8131 Gigabit Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x1066, "Atheros AR8121/AR8113/AR8114 Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x1067, "Atheros L1c Gigabit Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x1073, "Atheros AR8151 v1.0 Gigabit 1000" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x1083, "Atheros GbE LAN chip (10/100/1000 Mbit)" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x1090, "Atheros AR8162 Fast Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x1091, "Atheros AR8161 Gigabit Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x10a0, "Atheros QCA8172 Fast Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x10a1, "Atheros QCA8171 Gigabit Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x2048, "Atheros L2 Fast Ethernet" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x2060, "Atheros AR8152 v1.1 Fast 10/100" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x2062, "Atheros AR8152 v2.0 Fast 10/100" },

};

static network_device generic_ethernet_cards[] =
{
	{ 0x0000, 0x0000, "Generic Ethernet Controller" },
	{ PCI_VENDOR_ID_REALTEK, 0x0000, "Realtek Ethernet Controller" },
	{ PCI_VENDOR_ID_MARVELL, 0x0000, "Marvell Ethernet Controller" },
	{ PCI_VENDOR_ID_QUALCOMM, 0x0000, "Atheros Ethernet Controller" },
	{ PCI_VENDOR_ID_INTEL, 0x0000, "Intel(R) Ethernet Controller" },
};

static network_device known_wifi_cards[] =
{
	// Broadcom
	{PCI_VENDOR_ID_BROADCOM, 0x4312, "Broadcom BCM4311 802.11a/b/g"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4313, "Broadcom BCM4311 802.11a" },
	{PCI_VENDOR_ID_BROADCOM, 0x4315, "Broadcom BCM4312 802.11b/g Wireless LAN Controller"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4318, "Broadcom BCM4318 [AirForce One 54g] 802.11g Wireless LAN Controller"},
	{PCI_VENDOR_ID_BROADCOM, 0x4319, "Broadcom BCM4318 [AirForce 54g] 802.11a/b/g PCI Express Transceiver"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4320, "Broadcom BCM4306 802.11b/g Wireless LAN Controller"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4321, "Broadcom BCM4321 802.11a Wireless Network Controller"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4322, "Broadcom BCM4322 802.11bgn Wireless Network Controller"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4324, "Broadcom BCM4309 802.11abg Wireless Network Controller"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4325, "Broadcom BCM4306 802.11bg Wireless Network Controller"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4328, "Broadcom BCM4321 802.11a/b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4329, "Broadcom BCM4321 802.11b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x432a, "Broadcom BCM4321 802.11an Wireless Network Controller"},
	{PCI_VENDOR_ID_BROADCOM, 0x432b, "Broadcom BCM4322 802.11a/b/g/n Wireless LAN Controller"},
	{PCI_VENDOR_ID_BROADCOM, 0x432c, "Broadcom BCM4322 802.11b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x432d, "Broadcom BCM4322 802.11an Wireless Network Controller"},
	{PCI_VENDOR_ID_BROADCOM, 0x4331, "Broadcom BCM4331 802.11a/b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4350, "Broadcom BCM43222 Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4351, "Broadcom BCM43222 802.11abgn Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4353, "Broadcom BCM43224 802.11a/b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4357, "Broadcom BCM43225 802.11b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4358, "Broadcom BCM43227 802.11b/g/n"},
	{PCI_VENDOR_ID_BROADCOM, 0x4359, "Broadcom BCM43228 802.11a/b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4360, "Broadcom BCM4360 802.11ac Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0x4365, "Broadcom BCM43142 802.11b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43a0, "Broadcom BCM4360 802.11ac Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43a1, "Broadcom BCM4360 802.11ac Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43a2, "Broadcom BCM4360 802.11ac Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43a9, "Broadcom BCM43217 802.11b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43aa, "Broadcom BCM43131 802.11b/g/n"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43b1, "Broadcom BCM4352 802.11ac Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43ba, "Broadcom BCM43602 802.11ac Wireless LAN SoC"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43bb, "Broadcom BCM43602 802.11ac Wireless LAN SoC"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43bc, "Broadcom BCM43602 802.11ac Wireless LAN SoC"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43d3, "Broadcom BCM43567 802.11ac Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43d9, "Broadcom BCM43570 802.11ac Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43df, "Broadcom BCM4354 802.11ac Wireless LAN SoC"},
//	{PCI_VENDOR_ID_BROADCOM, 0x43ec, "Broadcom BCM4356 802.11ac Wireless Network Adapter"},
//	{PCI_VENDOR_ID_BROADCOM, 0xa8d8, "Broadcom BCM43224/5 Wireless Network Adapter"},

	// Atheros
	{PCI_VENDOR_ID_ATHEROS, 0x0020, "Atheros AR5513 802.11abg Wireless NIC"},
	{PCI_VENDOR_ID_ATHEROS, 0x0023, "Atheros AR5416 Wireless Network Adapter [AR5008 802.11(a)bgn]"},
	{PCI_VENDOR_ID_ATHEROS, 0x0024, "Atheros AR5418 Wireless Network Adapter [AR5008E 802.11(a)bgn]"},
	{PCI_VENDOR_ID_ATHEROS, 0x0027, "Atheros AR9160 Wireless Network Adapter [AR9001 802.11(a)bgn]"},
	{PCI_VENDOR_ID_ATHEROS, 0x0029, "Atheros AR922X Wireless Network Adapter"},
	{PCI_VENDOR_ID_ATHEROS, 0x002A, "Atheros AR928X Wireless Network Adapter"}, // "pci168c,2a"
	{PCI_VENDOR_ID_ATHEROS, 0x002B, "Atheros AR9285 Wireless Network Adapter"},
//	{PCI_VENDOR_ID_ATHEROS, 0x002c, "Atheros AR2427 802.11bg Wireless Network Adapter (PCI-Express)"},
//	{PCI_VENDOR_ID_ATHEROS, 0x002d, "Atheros AR9227 Wireless Network Adapter"},
//	{PCI_VENDOR_ID_ATHEROS, 0x002e, "Atheros AR9287 Wireless Network Adapter (PCI-Express)"},
//	{PCI_VENDOR_ID_ATHEROS, 0x0030, "Atheros AR93xx Wireless Network Adapter"},
//	{PCI_VENDOR_ID_ATHEROS, 0x0032, "Atheros AR9485 Wireless Network Adapter"},
//	{PCI_VENDOR_ID_ATHEROS, 0x0033, "Atheros AR9580 Wireless Network Adapter"},
//	{PCI_VENDOR_ID_ATHEROS, 0x0034, "Atheros AR9462 Wireless Network Adapter"},
//	{PCI_VENDOR_ID_ATHEROS, 0x0036, "Atheros QCA9565 / AR9565 Wireless Network Adapter"},
//	{PCI_VENDOR_ID_ATHEROS, 0x0037, "Atheros AR9485 Wireless Network Adapter"},
};


int devprop_add_network_template(DevPropDevice *device, uint16_t vendor_id)
{
	builtin = 0;
	if(device)
	{

		if((vendor_id != PCI_VENDOR_ID_ATHEROS ) && ( builtin_set == 0 ))
		{
			builtin_set = 1;
			builtin = 0x01;
		}

		if(!devprop_add_value(device, "built-in", (uint8_t *)&builtin, 1))
		{
			return 0;
		}

		if(!devprop_add_value(device, "device_type", (uint8_t *)"ethernet", sizeof("ethernet")))
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
		devprop_add_value(device, "device_type", (uint8_t *)"ethernet", sizeof("ethernet"));

		stringdata = (uint8_t*)malloc(sizeof(uint8_t) * string->length);
		if(stringdata)
		{
			memcpy(stringdata, (uint8_t *)devprop_generate_string(string), string->length);
			stringlength = string->length;
		}
	}
}

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
