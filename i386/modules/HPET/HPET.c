/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

#include "libsaio.h"
#include "modules.h"
#include "bootstruct.h"
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

static void force_enable_hpet_intel(pci_dt_t *lpc_dev);
static void force_enable_hpet_via(pci_dt_t *lpc_dev);
static void force_enable_hpet(pci_dt_t *lpc_dev);
void HPET_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);

void HPET_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	pci_dt_t* current = arg1;
	
	if(current->class_id != PCI_CLASS_BRIDGE_ISA) return;
	
	
	force_enable_hpet(current);
}

void HPET_start(void);
void HPET_start(void)
{
	
	register_hook_callback("PCIDevice", &HPET_hook);
}

/*
 * Force HPET enabled
 *
 * via fix from http://forum.voodooprojects.org/index.php/topic,1596.0.html
 */

static struct lpc_controller_t lpc_controllers_intel[] = {
	
	// Default unknown chipset
	{ 0, 0, "" },
	
	// Intel
	{ 0x8086, 0x24dc, "ICH5" },
	{ 0x8086, 0x2640, "ICH6" },
	{ 0x8086, 0x2641, "ICH6M" },
	
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
};

static struct lpc_controller_t lpc_controllers_via[] = {
	// Default unknown chipset
	{ 0, 0, "" },
	
	{ 0x1106, 0x3372, "VT8237S" },
};


static void force_enable_hpet(pci_dt_t *lpc_dev)
{
	switch(lpc_dev->vendor_id)
	{
		case 0x8086:
			force_enable_hpet_intel(lpc_dev);
			break;
			
		case 0x1106:
			force_enable_hpet_via(lpc_dev);
			break;
		default:
			break;
	}
	
	
#if DEBUG_HPET
	printf("Press [Enter] to continue...\n");
	getc();
#endif
}

static void force_enable_hpet_via(pci_dt_t *lpc_dev)
{
	uint32_t	val;
#if DEBUG_HPET
    uint32_t hpet_address = 0xFED00000;
#endif
	unsigned int i;
	
	for(i = 1; i < sizeof(lpc_controllers_via) / sizeof(lpc_controllers_via[0]); i++)
	{
		if (	(lpc_controllers_via[i].vendor == lpc_dev->vendor_id) 
			&& (lpc_controllers_via[i].device == lpc_dev->device_id))
		{	
			val = pci_config_read32(lpc_dev->dev.addr, 0x68);
			
			DBG("VIA %s LPC Interface [%04x:%04x], MMIO\n", 
				lpc_controllers_via[i].name, lpc_dev->vendor_id, lpc_dev->device_id);
			
            if (val & 0x80)
			{
#if DEBUG_HPET

				hpet_address = (val & ~0x3ff);
				DBG("HPET at 0x%lx\n", hpet_address);
#endif
			}
			else 
			{
				val = 0xfed00000 | 0x80;
				pci_config_write32(lpc_dev->dev.addr, 0x68, val);
#if DEBUG_HPET

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
#endif
			}
		}
	}
}



static void force_enable_hpet_intel(pci_dt_t *lpc_dev)
{
	uint32_t	val;
#if DEBUG_HPET
    uint32_t hpet_address = 0xFED00000;
#endif
	unsigned int i;
	void		*rcba;
	
	/* LPC on Intel ICH is always (?) at 00:1f.0 */
	for(i = 1; i < sizeof(lpc_controllers_intel) / sizeof(lpc_controllers_intel[0]); i++)
	{
		if (	(lpc_controllers_intel[i].vendor == lpc_dev->vendor_id) 
			&& (lpc_controllers_intel[i].device == lpc_dev->device_id))
		{	
			
			rcba = (void *)(pci_config_read32(lpc_dev->dev.addr, 0xF0) & 0xFFFFC000);
			
			DBG("Intel(R) %s LPC Interface [%04x:%04x], MMIO @ 0x%lx\n", 
				lpc_controllers_intel[i].name, lpc_dev->vendor_id, lpc_dev->device_id, rcba);
			
			if (rcba == 0)
				DBG(" RCBA disabled; cannot force enable HPET\n");
			else
			{
				val = REG32(rcba, 0x3404);
				if (val & 0x80)
				{
#if DEBUG_HPET
					// HPET is enabled in HPTC. Just not reported by BIOS
					DBG(" HPET is enabled in HPTC, just not reported by BIOS\n");
					hpet_address |= (val & 3) << 12 ;
					DBG(" HPET MMIO @ 0x%lx\n", hpet_address);
#endif
				}
				else
				{                    
					// HPET disabled in HPTC. Trying to enable
					DBG(" HPET is disabled in HPTC, trying to enable\n");									
					REG32(rcba, 0x3404) = val | 0x80;
#if DEBUG_HPET

					hpet_address |= (val & 3) << 12 ;
					DBG(" Force enabled HPET, MMIO @ 0x%lx\n", hpet_address);
#endif
				}
				
				
#if DEBUG_HPET	
                // verify if the job is done
				val = REG32(rcba, 0x3404);
                
				if (!(val & 0x80))
					printf(" Failed to force enable HPET\n");
#endif
/*
#define HPET_CONFIG             0x10    // General configuration register 
#define HPET_CNF_LEG_RT         0x00000002
#define HPET_CNF_ENABLE         0x00000001
                
                val = REG32(hpet_address, HPET_CONFIG);
                val &= ~HPET_CNF_LEG_RT;
                val |= HPET_CNF_ENABLE;
                REG32(hpet_address, HPET_CONFIG) = val;
*/
			}
			break;
			
		}
	}
}