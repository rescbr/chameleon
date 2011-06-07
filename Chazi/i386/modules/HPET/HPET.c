/*
 * Force HPET enabled, via fix from:
 *
 * http://forum.voodooprojects.org/index.php/topic,1596.0.html
 */

//#include "libsaio.h"
#include "boot.h"
#include "pci.h"
#include "hpet.h"
#include "modules.h"

/*#ifndef DEBUG_HPET
#define DEBUG_HPET 0
#endif

#if DEBUG_HPET
#define DBG(x...) printf(x)
#else
#define DBG(x...)
#endif
*/


void force_enable_hpet_intel(pci_dt_t *lpc_dev);
void force_enable_hpet_via(pci_dt_t *lpc_dev);


void HPET_hook(void* arg1, void* arg2, void* arg3, void* arg4)
{	
	bool do_enable_hpet = false;
	pci_dt_t* current = arg1;
	
	if (current->class_id != PCI_CLASS_BRIDGE_ISA)
		return;
	
	getBoolForKey(kForceHPETKey, &do_enable_hpet, &bootInfo->bootConfig);
	
	if (do_enable_hpet)
		force_enable_hpet(current);
}

void HPET_start()
{
	register_hook_callback("PCIDevice", &HPET_hook);
}

// Intel chipsets
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
	
	{ 0x8086, 0x27bc, "NM10" }, // Atom
	
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

// VIA chipsets
static struct lpc_controller_t lpc_controllers_via[] = {
	// Default unknown chipset
	{ 0, 0, "" },
	
	{ 0x1106, 0x3372, "VT8237S" },
};

void force_enable_hpet(pci_dt_t *lpc_dev)
{
	switch(lpc_dev->vendor_id)
	{
		case 0x8086:
			force_enable_hpet_intel(lpc_dev);
			break;
		
		case 0x1106:
			force_enable_hpet_via(lpc_dev);
			break;
	}
	
/*#if DEBUG_HPET
	pause(); - getchar(); //getc(); Azi: getc stuff
#endif*/
}

void force_enable_hpet_via(pci_dt_t *lpc_dev)
{	
	int		 i;
	uint32_t val, hpet_address = 0xFED00000;
	
	for (i = 1; i < sizeof(lpc_controllers_via) / sizeof(lpc_controllers_via[0]); i++)
	{
		if ((lpc_controllers_via[i].vendor == lpc_dev->vendor_id) 
			&& (lpc_controllers_via[i].device == lpc_dev->device_id))
		{	
			val = pci_config_read32(lpc_dev->dev.addr, 0x68);
			
			verbose("VIA %s LPC Interface [%04x:%04x], MMIO\n", 
				lpc_controllers_via[i].name, lpc_dev->vendor_id, lpc_dev->device_id);
			
			if (val & 0x80)
			{
				hpet_address = (val & ~0x3ff);
				verbose("HPET at 0x%lx\n", hpet_address);
			}
			else 
			{
				val = 0xfed00000 | 0x80;
				
				pci_config_write32(lpc_dev->dev.addr, 0x68, val);
				
				val = pci_config_read32(lpc_dev->dev.addr, 0x68);
				
				if (val & 0x80)
				{
					hpet_address = (val & ~0x3ff);
					verbose("Force enabled HPET at 0x%lx\n", hpet_address);
				}
				else
				{
					verbose("Unable to enable HPET");
				}
			}
		}
	}
}

void force_enable_hpet_intel(pci_dt_t *lpc_dev)
{
	int			i;
	void	   *rcba;
	uint32_t	val, hpet_address = 0xFED00000;
	
	/* LPC on Intel ICH is always (?) at 00:1f.0 */
	for(i = 1; i < sizeof(lpc_controllers_intel) / sizeof(lpc_controllers_intel[0]); i++)
	{
		if ((lpc_controllers_intel[i].vendor == lpc_dev->vendor_id) 
			&& (lpc_controllers_intel[i].device == lpc_dev->device_id))
		{
			rcba = (void *)(pci_config_read32(lpc_dev->dev.addr, 0xF0) & 0xFFFFC000);
			
			verbose("Intel(R) %s LPC Interface [%04x:%04x], MMIO @ 0x%lx\n", 
				lpc_controllers_intel[i].name, lpc_dev->vendor_id, lpc_dev->device_id, rcba);
			
			if (rcba == 0)
				verbose(" RCBA disabled; cannot force enable HPET\n");
			else
			{
				val = REG32(rcba, 0x3404);
				
				if (val & 0x80)
				{
					// HPET is enabled in HPTC. Just not reported by BIOS
					verbose(" HPET is enabled in HPTC, just not reported by BIOS\n");
					hpet_address |= (val & 3) << 12 ;
					verbose(" HPET MMIO @ 0x%lx\n", hpet_address);
				}
				else
				{
					// HPET disabled in HPTC. Trying to enable
					verbose(" HPET is disabled in HPTC, trying to enable\n");									
					REG32(rcba, 0x3404) = val | 0x80;
					hpet_address |= (val & 3) << 12 ;
					verbose(" Force enabled HPET, MMIO @ 0x%lx\n", hpet_address);
				}
				
				// verify if the job is done
				val = REG32(rcba, 0x3404);
				
				if (!(val & 0x80))
					verbose(" Failed to force enable HPET\n");
			}
			
			break;
		}
	}
}
