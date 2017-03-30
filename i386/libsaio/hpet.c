/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 */

/*
 * High Precision Event Timer (HPET)
 */

#include "config.h"
#include "libsaio.h"
#include "pci.h"
#include "hpet.h"

#if DEBUG_HPET
	#define DBG(x...) printf(x)
#else
	#define DBG(x...)
#endif

void force_enable_hpet_intel(pci_dt_t *lpc_dev);
void force_enable_hpet_nvidia(pci_dt_t *lpc_dev);
void force_enable_hpet_via(pci_dt_t *lpc_dev);

/*
 * Force HPET enabled
 *
 * VIA fix from http://forum.voodooprojects.org/index.php/topic,1596.0.html
 */

static struct lpc_controller_t lpc_controllers_intel[] = {

	// Default unknown chipset
	{ 0, 0, "" },

	// Intel
	{ 0x8086, 0x0f1c, "Bay Trail SoC" },

	{ 0x8086, 0x1c41, "Cougar Point" },
	{ 0x8086, 0x1c42, "Cougar Point Desktop" },
	{ 0x8086, 0x1c43, "Cougar Point Mobile" },
	{ 0x8086, 0x1c44, "Cougar Point" },
	{ 0x8086, 0x1c45, "Cougar Point" },
	{ 0x8086, 0x1c46, "Cougar Point" },
	{ 0x8086, 0x1c47, "Cougar Point" },
	{ 0x8086, 0x1c48, "Cougar Point" },
	{ 0x8086, 0x1c49, "Cougar Point" },
	{ 0x8086, 0x1c4a, "Cougar Point" },
	{ 0x8086, 0x1c4b, "Cougar Point" },
	{ 0x8086, 0x1c4c, "Cougar Point" },
	{ 0x8086, 0x1c4d, "Cougar Point" },
	{ 0x8086, 0x1c4e, "Cougar Point" },
	{ 0x8086, 0x1c4f, "Cougar Point" },
	{ 0x8086, 0x1c50, "Cougar Point" },
	{ 0x8086, 0x1c51, "Cougar Point" },
	{ 0x8086, 0x1c52, "Cougar Point" },
	{ 0x8086, 0x1c53, "Cougar Point" },
	{ 0x8086, 0x1c54, "Cougar Point" },
	{ 0x8086, 0x1c55, "Cougar Point" },
	{ 0x8086, 0x1c56, "Cougar Point" },
	{ 0x8086, 0x1c57, "Cougar Point" },
	{ 0x8086, 0x1c58, "Cougar Point" },
	{ 0x8086, 0x1c59, "Cougar Point" },
	{ 0x8086, 0x1c5a, "Cougar Point" },
	{ 0x8086, 0x1c5b, "Cougar Point" },
	{ 0x8086, 0x1c5c, "Cougar Point" },
	{ 0x8086, 0x1c5d, "Cougar Point" },
	{ 0x8086, 0x1c5e, "Cougar Point" },
	{ 0x8086, 0x1c5f, "Cougar Point" },
	{ 0x8086, 0x1d40, "Patsburg" },
	{ 0x8086, 0x1d41, "Patsburg" },
	{ 0x8086, 0x1e40, "Panther Point" },
	{ 0x8086, 0x1e41, "Panther Point" },
	{ 0x8086, 0x1e42, "Panther Point" },
	{ 0x8086, 0x1e43, "Panther Point" },
	{ 0x8086, 0x1e44, "Panther Point" },
	{ 0x8086, 0x1e45, "Panther Point" },
	{ 0x8086, 0x1e46, "Panther Point" },
	{ 0x8086, 0x1e47, "Panther Point" },
	{ 0x8086, 0x1e48, "Panther Point" },
	{ 0x8086, 0x1e49, "Panther Point" },
	{ 0x8086, 0x1e4a, "Panther Point" },
	{ 0x8086, 0x1e4b, "Panther Point" },
	{ 0x8086, 0x1e4c, "Panther Point" },
	{ 0x8086, 0x1e4d, "Panther Point" },
	{ 0x8086, 0x1e4e, "Panther Point" },
	{ 0x8086, 0x1e4f, "Panther Point" },
	{ 0x8086, 0x1e50, "Panther Point" },
	{ 0x8086, 0x1e51, "Panther Point" },
	{ 0x8086, 0x1e52, "Panther Point" },
	{ 0x8086, 0x1e53, "Panther Point" },
	{ 0x8086, 0x1e54, "Panther Point" },
	{ 0x8086, 0x1e55, "Panther Point" },
	{ 0x8086, 0x1e56, "Panther Point" },
	{ 0x8086, 0x1e57, "Panther Point" },
	{ 0x8086, 0x1e58, "Panther Point" },
	{ 0x8086, 0x1e59, "Panther Point" },
	{ 0x8086, 0x1e5a, "Panther Point" },
	{ 0x8086, 0x1e5b, "Panther Point" },
	{ 0x8086, 0x1e5c, "Panther Point" },
	{ 0x8086, 0x1e5d, "Panther Point" },
	{ 0x8086, 0x1e5e, "Panther Point" },
	{ 0x8086, 0x1e5f, "Panther Point" },
	{ 0x8086, 0x1f38, "Avoton SoC" },
	{ 0x8086, 0x1f39, "Avoton SoC" },
	{ 0x8086, 0x1f3a, "Avoton SoC" },
	{ 0x8086, 0x1f3b, "Avoton SoC" },

	{ 0x8086, 0x229c, "Braswell SoC" },
	{ 0x8086, 0x2390, "Coleto Creek" },
	{ 0x8086, 0x2310, "DH89xxCC" },
	{ 0x8086, 0x2410, "ICH" },
	{ 0x8086, 0x2420, "ICH0" },
	{ 0x8086, 0x2440, "ICH2" },
	{ 0x8086, 0x244c, "ICH2-M" },
	{ 0x8086, 0x2480, "ICH3-S" },
	{ 0x8086, 0x248c, "ICH3-M" },
	{ 0x8086, 0x24c0, "ICH4" },
	{ 0x8086, 0x24cc, "ICH4-M" },
	{ 0x8086, 0x2450, "C-ICH" },
	{ 0x8086, 0x24d0, "ICH5/ICH5R" },
	{ 0x8086, 0x25a1, "6300ESB" },
	{ 0x8086, 0x2640, "ICH6/ICH6R" },
	{ 0x8086, 0x2641, "ICH6-M" },
	{ 0x8086, 0x2642, "ICH6W/ICH6RW" },
	{ 0x8086, 0x2670, "631xESB/632xESB" },
	{ 0x8086, 0x2671, "631xESB/632xESB" },
	{ 0x8086, 0x2672, "631xESB/632xESB" },
	{ 0x8086, 0x2673, "631xESB/632xESB" },
	{ 0x8086, 0x2674, "631xESB/632xESB" },
	{ 0x8086, 0x2675, "631xESB/632xESB" },
	{ 0x8086, 0x2676, "631xESB/632xESB" },
	{ 0x8086, 0x2677, "631xESB/632xESB" },
	{ 0x8086, 0x2678, "631xESB/632xESB" },
	{ 0x8086, 0x2679, "631xESB/632xESB" },
	{ 0x8086, 0x267a, "631xESB/632xESB" },
	{ 0x8086, 0x267b, "631xESB/632xESB" },
	{ 0x8086, 0x267c, "631xESB/632xESB" },
	{ 0x8086, 0x267d, "631xESB/632xESB" },
	{ 0x8086, 0x267e, "631xESB/632xESB" },
	{ 0x8086, 0x267f, "631xESB/632xESB" },
	{ 0x8086, 0x27b0, "ICH7DH" },
	{ 0x8086, 0x27b8, "ICH7/ICH7R" },
	{ 0x8086, 0x27b9, "ICH7-M/ICH7-U" },
	{ 0x8086, 0x27bc, "NM10" },
	{ 0x8086, 0x27bd, "ICH7-M DH" },


	{ 0x8086, 0x2810, "ICH8/ICH8R" },
	{ 0x8086, 0x2811, "ICH8M-E" },
	{ 0x8086, 0x2812, "ICH8DH" },
	{ 0x8086, 0x2814, "ICH8DO" },
	{ 0x8086, 0x2815, "ICH8M" },

	{ 0x8086, 0x2912, "ICH9DH" },
	{ 0x8086, 0x2914, "ICH9DO" },
	{ 0x8086, 0x2916, "ICH9R" },
	{ 0x8086, 0x2917, "ICH9M-E" },
	{ 0x8086, 0x2918, "ICH9" },
	{ 0x8086, 0x2919, "ICH9M" },

	{ 0x8086, 0x3a14, "ICH10DO" },
	{ 0x8086, 0x3a16, "ICH10R" },
	{ 0x8086, 0x3a18, "ICH10" },
	{ 0x8086, 0x3a1a, "ICH10D" },
	{ 0x8086, 0x3b00, "PCH Desktop Full Featured" },
	{ 0x8086, 0x3b01, "PCH Mobile Full Featured" },
	{ 0x8086, 0x3b02, "P55" },
	{ 0x8086, 0x3b03, "PM55" },
	{ 0x8086, 0x3b06, "H55" },
	{ 0x8086, 0x3b07, "QM57" },
	{ 0x8086, 0x3b08, "H57" },
	{ 0x8086, 0x3b09, "HM55" },
	{ 0x8086, 0x3b0a, "Q57" },
	{ 0x8086, 0x3b0b, "HM57" },
	{ 0x8086, 0x3b0d, "PCH Mobile SFF Full Featured" },
	{ 0x8086, 0x3b0f, "QS57" },
	{ 0x8086, 0x3b12, "3400" },
	{ 0x8086, 0x3b14, "3420" },
	{ 0x8086, 0x3b16, "3450" },

	{ 0x8086, 0x5031, "EP80579" },

	{ 0x8086, 0x8c40, "Lynx Point" },
	{ 0x8086, 0x8c41, "Lynx Point" },
	{ 0x8086, 0x8c42, "Lynx Point" },
	{ 0x8086, 0x8c43, "Lynx Point" },
	{ 0x8086, 0x8c44, "Lynx Point" },
	{ 0x8086, 0x8c45, "Lynx Point" },
	{ 0x8086, 0x8c46, "Lynx Point" },
	{ 0x8086, 0x8c47, "Lynx Point" },
	{ 0x8086, 0x8c48, "Lynx Point" },
	{ 0x8086, 0x8c49, "Lynx Point" },
	{ 0x8086, 0x8c4a, "Lynx Point" },
	{ 0x8086, 0x8c4b, "Lynx Point" },
	{ 0x8086, 0x8c4c, "Lynx Point" },
	{ 0x8086, 0x8c4d, "Lynx Point" },
	{ 0x8086, 0x8c4e, "Lynx Point" },
	{ 0x8086, 0x8c4f, "Lynx Point" },
	{ 0x8086, 0x8c50, "Lynx Point" },
	{ 0x8086, 0x8c51, "Lynx Point" },
	{ 0x8086, 0x8c52, "Lynx Point" },
	{ 0x8086, 0x8c53, "Lynx Point" },
	{ 0x8086, 0x8c54, "Lynx Point" },
	{ 0x8086, 0x8c55, "Lynx Point" },
	{ 0x8086, 0x8c56, "Lynx Point" },
	{ 0x8086, 0x8c57, "Lynx Point" },
	{ 0x8086, 0x8c58, "Lynx Point" },
	{ 0x8086, 0x8c59, "Lynx Point" },
	{ 0x8086, 0x8c5a, "Lynx Point" },
	{ 0x8086, 0x8c5b, "Lynx Point" },
	{ 0x8086, 0x8c5c, "Lynx Point" },
	{ 0x8086, 0x8c5d, "Lynx Point" },
	{ 0x8086, 0x8c5e, "Lynx Point" },
	{ 0x8086, 0x8c5f, "Lynx Point" },
	{ 0x8086, 0x8cc1, "9 Series" },
	{ 0x8086, 0x8cc2, "9 Series" },
	{ 0x8086, 0x8cc3, "9 Series" },
	{ 0x8086, 0x8cc4, "9 Series" },
	{ 0x8086, 0x8cc6, "9 Series" },
	{ 0x8086, 0x8d40, "Wellsburg" },
	{ 0x8086, 0x8d41, "Wellsburg" },
	{ 0x8086, 0x8d42, "Wellsburg" },
	{ 0x8086, 0x8d43, "Wellsburg" },
	{ 0x8086, 0x8d44, "Wellsburg" },
	{ 0x8086, 0x8d45, "Wellsburg" },
	{ 0x8086, 0x8d46, "Wellsburg" },
	{ 0x8086, 0x8d47, "Wellsburg" },
	{ 0x8086, 0x8d48, "Wellsburg" },
	{ 0x8086, 0x8d49, "Wellsburg" },
	{ 0x8086, 0x8d4a, "Wellsburg" },
	{ 0x8086, 0x8d4b, "Wellsburg" },
	{ 0x8086, 0x8d4c, "Wellsburg" },
	{ 0x8086, 0x8d4d, "Wellsburg" },
	{ 0x8086, 0x8d4e, "Wellsburg" },
	{ 0x8086, 0x8d4f, "Wellsburg" },
	{ 0x8086, 0x8d50, "Wellsburg" },
	{ 0x8086, 0x8d51, "Wellsburg" },
	{ 0x8086, 0x8d52, "Wellsburg" },
	{ 0x8086, 0x8d53, "Wellsburg" },
	{ 0x8086, 0x8d54, "Wellsburg" },
	{ 0x8086, 0x8d55, "Wellsburg" },
	{ 0x8086, 0x8d56, "Wellsburg" },
	{ 0x8086, 0x8d57, "Wellsburg" },
	{ 0x8086, 0x8d58, "Wellsburg" },
	{ 0x8086, 0x8d59, "Wellsburg" },
	{ 0x8086, 0x8d5a, "Wellsburg" },
	{ 0x8086, 0x8d5b, "Wellsburg" },
	{ 0x8086, 0x8d5c, "Wellsburg" },
	{ 0x8086, 0x8d5d, "Wellsburg" },
	{ 0x8086, 0x8d5e, "Wellsburg" },
	{ 0x8086, 0x8d5f, "Wellsburg" },

	{ 0x8086, 0x9c40, "Lynx Point_LP" },
	{ 0x8086, 0x9c41, "Lynx Point_LP" },
	{ 0x8086, 0x9c42, "Lynx Point_LP" },
	{ 0x8086, 0x9c43, "Lynx Point_LP" },
	{ 0x8086, 0x9c44, "Lynx Point_LP" },
	{ 0x8086, 0x9c45, "Lynx Point_LP" },
	{ 0x8086, 0x9c46, "Lynx Point_LP" },
	{ 0x8086, 0x9c47, "Lynx Point_LP" },
	{ 0x8086, 0x9cc1, "Wildcat Point_LP" },
	{ 0x8086, 0x9cc2, "Wildcat Point_LP" },
	{ 0x8086, 0x9cc3, "Wildcat Point_LP" },
	{ 0x8086, 0x9cc5, "Wildcat Point_LP" },
	{ 0x8086, 0x9cc6, "Wildcat Point_LP" },
	{ 0x8086, 0x9cc7, "Wildcat Point_LP" },
	{ 0x8086, 0x9cc9, "Wildcat Point_LP" },

};

static struct lpc_controller_t lpc_controllers_nvidia[] = {

	// Default unknown chipset
	{ 0, 0, "" },

	// nVidia
	{ 0x10de, 0x0aac, "MCP79" },
	{ 0x10de, 0x0aae, "MCP79" },
	{ 0x10de, 0x0aaf, "MCP79" },
	{ 0x10de, 0x0d80, "MCP89" },
	{ 0x10de, 0x0d81, "MCP89" },
	{ 0x10de, 0x0d82, "MCP89" },
	{ 0x10de, 0x0d83, "MCP89" },

};

static struct lpc_controller_t lpc_controllers_via[] = {
	// Default unknown chipset
	{ 0, 0, "" },
	{ 0x1106, 0x3050, "VT82C596A" },
	{ 0x1106, 0x3051, "VT82C596B" },
	{ 0x1106, 0x8235, "VT8231" },
	{ 0x1106, 0x3074, "VT8233" },
	{ 0x1106, 0x3147, "VT8233A" },
	{ 0x1106, 0x3177, "VT8235" },
	{ 0x1106, 0x3227, "VT8237R" },
	{ 0x1106, 0x3337, "VT8237A" },
	{ 0x1106, 0x3372, "VT8237S" },
	{ 0x1106, 0x3287, "VT8251" },
	{ 0x1106, 0x8324, "CX700" },
	{ 0x1106, 0x8353, "VX800/VX820" },
	{ 0x1106, 0x8409, "VX855/VX875" },
};

/* ErmaC add lpc for nVidia */
void force_enable_hpet_nvidia(pci_dt_t *lpc_dev)
{
	uint32_t	val, hpet_address = 0xFED00000;
	int i;
	void		*rcba;

	for(i = 1; i < sizeof(lpc_controllers_nvidia) / sizeof(lpc_controllers_nvidia[0]); i++)
	{
		if ((lpc_controllers_nvidia[i].vendor == lpc_dev->vendor_id) && (lpc_controllers_nvidia[i].device == lpc_dev->device_id))
		{

			rcba = (void *)(pci_config_read32(lpc_dev->dev.addr, 0xF0) & 0xFFFFC000);

			DBG("\tnVidia(R) %s LPC Interface [%04x:%04x], MMIO @ 0x%lx\n", 
				lpc_controllers_nvidia[i].name, lpc_dev->vendor_id, lpc_dev->device_id, rcba);

			if (rcba == 0)
			{
				printf("\tRCBA disabled; cannot force enable HPET\n");
			}
			else
			{
				val = REG32(rcba, 0x3404);
				if (val & 0x80)
				{
					// HPET is enabled in HPTC. Just not reported by BIOS
					DBG("\tHPET is enabled in HPTC, just not reported by BIOS\n");
					hpet_address |= (val & 3) << 12 ;
					DBG("\tHPET MMIO @ 0x%lx\n", hpet_address);
				}
				else
				{
					// HPET disabled in HPTC. Trying to enable
					DBG("\tHPET is disabled in HPTC, trying to enable\n");									
					REG32(rcba, 0x3404) = val | 0x80;
					hpet_address |= (val & 3) << 12 ;
					DBG("\tForce enabled HPET, MMIO @ 0x%lx\n", hpet_address);
				}

				// verify if the job is done
				val = REG32(rcba, 0x3404);
				if (!(val & 0x80))
				{
					printf("\tFailed to force enable HPET\n");
				}
			}
			break;
		}
	}
}

void force_enable_hpet_via(pci_dt_t *lpc_dev)
{
	uint32_t	val, hpet_address = 0xFED00000;
	int i;

	for(i = 1; i < sizeof(lpc_controllers_via) / sizeof(lpc_controllers_via[0]); i++)
	{
		if ((lpc_controllers_via[i].vendor == lpc_dev->vendor_id) && (lpc_controllers_via[i].device == lpc_dev->device_id))
		{
			val = pci_config_read32(lpc_dev->dev.addr, 0x68);

			DBG("\tVIA %s LPC Interface [%04x:%04x], MMIO\n", 
				lpc_controllers_via[i].name, lpc_dev->vendor_id, lpc_dev->device_id);

			if (val & 0x80)
			{
				hpet_address = (val & ~0x3ff);
				DBG("HPET at 0x%lx\n", hpet_address);
			}
			else
			{
				val = 0xfed00000 | 0x80;
				pci_config_write32(lpc_dev->dev.addr, 0x68, val);
				val = pci_config_read32(lpc_dev->dev.addr, 0x68);
				if (val & 0x80)
				{
					hpet_address = (val & ~0x3ff);
					DBG("\tForce enabled HPET at 0x%lx\n", hpet_address);
				}
				else
				{
					DBG("\tUnable to enable HPET");
				}
			}
		}
	}
}

void force_enable_hpet_intel(pci_dt_t *lpc_dev)
{
	uint32_t	val, hpet_address = 0xFED00000;
	int i;
	void		*rcba;

	/* LPC on Intel ICH is always (?) at 00:1f.0 */
	for(i = 1; i < sizeof(lpc_controllers_intel) / sizeof(lpc_controllers_intel[0]); i++)
	{
		if ((lpc_controllers_intel[i].vendor == lpc_dev->vendor_id) && (lpc_controllers_intel[i].device == lpc_dev->device_id))
		{

			rcba = (void *)(pci_config_read32(lpc_dev->dev.addr, 0xF0) & 0xFFFFC000);

			DBG("\tIntel(R) %s LPC Interface [%04x:%04x], MMIO @ 0x%lx\n", 
				lpc_controllers_intel[i].name, lpc_dev->vendor_id, lpc_dev->device_id, rcba);

			if (rcba == 0)
			{
				printf("\tRCBA disabled; cannot force enable HPET\n");
			}
			else
			{
				val = REG32(rcba, 0x3404);
				if (val & 0x80)
				{
					// HPET is enabled in HPTC. Just not reported by BIOS
					DBG("\tHPET is enabled in HPTC, just not reported by BIOS\n");
					hpet_address |= (val & 3) << 12 ;
					DBG("\tHPET MMIO @ 0x%lx\n", hpet_address);
				}
				else
				{
					// HPET disabled in HPTC. Trying to enable
					DBG("\tHPET is disabled in HPTC, trying to enable\n");									
					REG32(rcba, 0x3404) = val | 0x80;
					hpet_address |= (val & 3) << 12 ;
					DBG("\tForce enabled HPET, MMIO @ 0x%lx\n", hpet_address);
				}

				// verify if the job is done
				val = REG32(rcba, 0x3404);
				if (!(val & 0x80))
				{
					printf("\tFailed to force enable HPET\n");
				}
			}
			break;
		}
	}
}

void force_enable_hpet(pci_dt_t *lpc_dev)
{
	switch(lpc_dev->vendor_id)
	{
		case 0x8086:
			force_enable_hpet_intel(lpc_dev);
			break;

		case 0x10de:
			force_enable_hpet_nvidia(lpc_dev);
			break;

		case 0x1106:
			force_enable_hpet_via(lpc_dev);
			break;
	}

#if DEBUG_HPET
	printf("Press [Enter] to continue...\n");
	getchar();
#endif

}
