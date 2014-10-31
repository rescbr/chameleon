/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 */

/*
 * High Precision Event Timer (HPET)
 */

#include "libsaio.h"
#include "pci.h"
#include "hpet.h"

#ifndef DEBUG_HPET
#define DEBUG_HPET 0
#endif

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
	{ 0x8086, 0x24dc, "ICH5" },
	{ 0x8086, 0x2640, "ICH6" },
	{ 0x8086, 0x2641, "ICH6M" },
	{ 0x8086, 0x2670, "631xESB" },

	{ 0x8086, 0x27b0, "ICH7 DH" },
	{ 0x8086, 0x27b8, "ICH7" },
	{ 0x8086, 0x27b9, "ICH7M" },
	{ 0x8086, 0x27bd, "ICH7M DH" },

	{ 0x8086, 0x27bc, "NM10" },

	{ 0x8086, 0x2810, "ICH8R" },
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
	{ 0x8086, 0x3b00, "5 Series" },
   	{ 0x8086, 0x3b01, "Mobile 5 Series" },
	{ 0x8086, 0x3b02, "5 Series" },
	{ 0x8086, 0x3b09, "Mobile 5 Series" },

	{ 0x8086, 0x1c41, "Mobile SFF 6 Series" },
	{ 0x8086, 0x1c42, "6 Series" },
	{ 0x8086, 0x1c43, "Mobile 6 Series" },
	{ 0x8086, 0x1c44, "Z68" },
	{ 0x8086, 0x1c46, "UM67" },
	{ 0x8086, 0x1c47, "UM67" },
	{ 0x8086, 0x1c49, "HM65" },
	{ 0x8086, 0x1c4a, "H67" },
	{ 0x8086, 0x1c4b, "HM67" },
	{ 0x8086, 0x1c4c, "Q65" },
	{ 0x8086, 0x1c4d, "QS67" },
	{ 0x8086, 0x1c4e, "Q67" },
	{ 0x8086, 0x1c4f, "QM67" },
	{ 0x8086, 0x1c50, "B65" },
	{ 0x8086, 0x1c52, "C202" },
	{ 0x8086, 0x1c54, "C204" },
	{ 0x8086, 0x1c56, "C206" },
	{ 0x8086, 0x1c5c, "H61" },

	{ 0x8086, 0x1c58, "B65" },
	{ 0x8086, 0x1c59, "HM67" },
	{ 0x8086, 0x1c5a, "Q67" },

//	{ 0x8086, 0x1dc1, "PATSBURG" },

	{ 0x8086, 0x1e41, "Mobile C216/7 Series" },
	{ 0x8086, 0x1e42, "Mobile 7 Series" },
	{ 0x8086, 0x1e43, "Mobile 7 Series" },
	{ 0x8086, 0x1e44, "Z77" },
	{ 0x8086, 0x1e45, "H71" },
	{ 0x8086, 0x1e46, "Z75" },
	{ 0x8086, 0x1e47, "Q77" },
	{ 0x8086, 0x1e48, "Q75" },
	{ 0x8086, 0x1e49, "B75" },
	{ 0x8086, 0x1e4a, "H77" },
	{ 0x8086, 0x1e4b, "B75" },

	{ 0x8086, 0x8119, "SCH 8119" },

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

	{ 0x1106, 0x3372, "VT8237S" },
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

			DBG("nVidia(R) %s LPC Interface [%04x:%04x], MMIO @ 0x%lx\n", 
				lpc_controllers_nvidia[i].name, lpc_dev->vendor_id, lpc_dev->device_id, rcba);

			if (rcba == 0)
			{
				printf(" RCBA disabled; cannot force enable HPET\n");
			}
			else
			{
				val = REG32(rcba, 0x3404);
				if (val & 0x80)
				{
					// HPET is enabled in HPTC. Just not reported by BIOS
					DBG(" HPET is enabled in HPTC, just not reported by BIOS\n");
					hpet_address |= (val & 3) << 12 ;
					DBG(" HPET MMIO @ 0x%lx\n", hpet_address);
				}
				else
				{
					// HPET disabled in HPTC. Trying to enable
					DBG(" HPET is disabled in HPTC, trying to enable\n");									
					REG32(rcba, 0x3404) = val | 0x80;
					hpet_address |= (val & 3) << 12 ;
					DBG(" Force enabled HPET, MMIO @ 0x%lx\n", hpet_address);
				}

				// verify if the job is done
				val = REG32(rcba, 0x3404);
				if (!(val & 0x80))
				{
					printf(" Failed to force enable HPET\n");
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

			DBG("VIA %s LPC Interface [%04x:%04x], MMIO\n", 
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
					DBG("Force enabled HPET at 0x%lx\n", hpet_address);
				}
				else
				{
					DBG("Unable to enable HPET");
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

			DBG("Intel(R) %s LPC Interface [%04x:%04x], MMIO @ 0x%lx\n", 
				lpc_controllers_intel[i].name, lpc_dev->vendor_id, lpc_dev->device_id, rcba);

			if (rcba == 0)
				printf(" RCBA disabled; cannot force enable HPET\n");
			else
			{
				val = REG32(rcba, 0x3404);
				if (val & 0x80)
				{
					// HPET is enabled in HPTC. Just not reported by BIOS
					DBG(" HPET is enabled in HPTC, just not reported by BIOS\n");
					hpet_address |= (val & 3) << 12 ;
					DBG(" HPET MMIO @ 0x%lx\n", hpet_address);
				}
				else
				{
					// HPET disabled in HPTC. Trying to enable
					DBG(" HPET is disabled in HPTC, trying to enable\n");									
					REG32(rcba, 0x3404) = val | 0x80;
					hpet_address |= (val & 3) << 12 ;
					DBG(" Force enabled HPET, MMIO @ 0x%lx\n", hpet_address);
				}

				// verify if the job is done
				val = REG32(rcba, 0x3404);
				if (!(val & 0x80))
				{
					printf(" Failed to force enable HPET\n");
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
