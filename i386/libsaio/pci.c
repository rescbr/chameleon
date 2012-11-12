/*
 *
 * Copyright 2008 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */

#include "libsaio.h"
#include "bootstruct.h"
#include "pci.h"
#include "modules.h"
#include "platform.h"

#ifndef DEBUG_PCI
#define DEBUG_PCI 0
#endif

#if DEBUG_PCI
#define DBG(x...)		printf(x)
#else
#define DBG(x...)
#endif

static void scan_pci_bus(pci_dt_t *start, uint8_t bus);
static void enable_pci_devs(void);

uint8_t pci_config_read8(uint32_t pci_addr, uint8_t reg)
{
	pci_addr |= reg & ~3;
	outl(PCI_ADDR_REG, pci_addr);
	return inb(PCI_DATA_REG + (reg & 3));
}

uint16_t pci_config_read16(uint32_t pci_addr, uint8_t reg)
{
	pci_addr |= reg & ~3;
	outl(PCI_ADDR_REG, pci_addr);
	return inw(PCI_DATA_REG + (reg & 2));
}

uint32_t pci_config_read32(uint32_t pci_addr, uint8_t reg)
{
	pci_addr |= reg & ~3;
	outl(PCI_ADDR_REG, pci_addr);
	return inl(PCI_DATA_REG);
}

void pci_config_write8(uint32_t pci_addr, uint8_t reg, uint8_t data)
{
	pci_addr |= reg & ~3;
	outl(PCI_ADDR_REG, pci_addr);
	outb(PCI_DATA_REG + (reg & 3), data);
}

void pci_config_write16(uint32_t pci_addr, uint8_t reg, uint16_t data)
{
	pci_addr |= reg & ~3;
	outl(PCI_ADDR_REG, pci_addr);
	outw(PCI_DATA_REG + (reg & 2), data);
}

void pci_config_write32(uint32_t pci_addr, uint8_t reg, uint32_t data)
{
	pci_addr |= reg & ~3;
	outl(PCI_ADDR_REG, pci_addr);
	outl(PCI_DATA_REG, data);
}

static void scan_pci_bus(pci_dt_t *start, uint8_t bus)
{
	pci_dt_t	*new;
	pci_dt_t	**current = &start->children;
	uint32_t	id;
	uint32_t	pci_addr;
	uint8_t		dev;
	uint8_t		func;
	uint8_t		secondary_bus;
	uint8_t		header_type;
	
	for (dev = 0; dev < 32; dev++)
	{
		for (func = 0; func < 8; func++)
		{
			pci_addr = PCIADDR(bus, dev, func);
			id = pci_config_read32(pci_addr, PCI_VENDOR_ID);
			if (!id || id == 0xffffffff)
			{
				continue;
			}
			new = (pci_dt_t*)malloc(sizeof(pci_dt_t));
            if (!new) {
                continue;
            }
			bzero(new, sizeof(pci_dt_t));
			new->dev.addr	= pci_addr;
			new->vendor_id	= id & 0xffff;
			new->device_id	= (id >> 16) & 0xffff;
			new->subsys_id.subsys_id	= pci_config_read32(pci_addr, PCI_SUBSYSTEM_VENDOR_ID);
			new->class_id	= pci_config_read16(pci_addr, PCI_CLASS_DEVICE);
			new->parent	= start;
			
			header_type = pci_config_read8(pci_addr, PCI_HEADER_TYPE);
			switch (header_type & 0x7f)
			{
				case PCI_HEADER_TYPE_BRIDGE:
				case PCI_HEADER_TYPE_CARDBUS:
					secondary_bus = pci_config_read8(pci_addr, PCI_SECONDARY_BUS);
					if (secondary_bus != 0)
					{
						scan_pci_bus(new, secondary_bus);
					}
					break;
				default:
					break;
			}
			*current = new;
			current = &new->next;
			
			if ((func == 0) && ((header_type & 0x80) == 0))
			{
				break;
			}
		}
	}
}

static void enable_pci_devs(void)
{
	uint16_t id;
	uint32_t rcba, *fd;
	
	id = pci_config_read16(PCIADDR(0, 0x00, 0), 0x00);
	/* make sure we're on Intel chipset */
	if (id != 0x8086)
		return;
	rcba = pci_config_read32(PCIADDR(0, 0x1f, 0), 0xf0) & ~1;
	fd = (uint32_t *)(rcba + 0x3418);
	/* set SMBus Disable (SD) to 0 */
	*fd &= ~0x8;
	/* and all devices? */
	//*fd = 0x1;
}


void build_pci_dt(void)
{
    pci_dt_t	*root_pci_dev;

	root_pci_dev = malloc(sizeof(pci_dt_t));    
    
    if (!root_pci_dev)
    {
        stop("Couldn't allocate memory for the pci root device\n");
        return ;
    }
    set_env(envPCIRootDev,(uint32_t)root_pci_dev);
    
	bzero(root_pci_dev, sizeof(pci_dt_t));
	enable_pci_devs();
	scan_pci_bus(root_pci_dev, 0);
#if DEBUG_PCI
#ifndef OPTION_ROM
	dump_pci_dt(root_pci_dev->children);
	pause();
#endif
#endif
}

static void setup_pci_devs(pci_dt_t *current)
{    
	
	while (current)
	{
		execute_hook("PCIDevice", (void*)current, NULL, NULL, NULL, NULL, NULL);
		
		setup_pci_devs(current->children);
		current = current->next;
	}
}

void setup_root_pci_devs(void)
{
    pci_dt_t *root_pci_dev = (pci_dt_t *)(uint32_t)get_env(envPCIRootDev);

    if (!root_pci_dev)
    {
        stop("No PCI root device\n");
        return ;
    }
	
	setup_pci_devs(root_pci_dev);
}

#ifndef OPTION_ROM
void dump_pci_dt(pci_dt_t *pci_dt)
{
	pci_dt_t	*current;
	
	current = pci_dt;
	while (current) 
	{
		printf("%02x:%02x.%x [%04x] [%04x:%04x] \n", 
			   current->dev.bits.bus, current->dev.bits.dev, current->dev.bits.func, 
			   current->class_id, current->vendor_id, current->device_id);
		dump_pci_dt(current->children);
		current = current->next;
	}
}
#endif

static int rootuid = 10; //value means function wasnt ran yet

/*
 * getPciRootUID: Copyright 2009 netkas
 */
int getPciRootUID(void)
{
	const char *val;
	int len;
	
	if (rootuid < 10) return rootuid;
	rootuid = 0;	/* default uid = 0 */
	
	if (getValueForKey(kPCIRootUID, &val, &len, DEFAULT_BOOT_CONFIG))
	{
		if (isdigit(val[0])) rootuid = val[0] - '0';
		
		if ( (rootuid >= 0) && (rootuid < 10) ) 
			goto out;
		else
			rootuid = 0;
	}	
	
	/* PCEFI compatibility */
	if (getValueForKey("-pci0", &val, &len, DEFAULT_BOOT_CONFIG))
	{
		rootuid = 0;
	}
	else if (getValueForKey("-pci1", &val, &len, DEFAULT_BOOT_CONFIG))
	{
		rootuid = 1;
	}
out:
	verbose("Using PCI-Root-UID value: %d\n", rootuid);
	return rootuid;
}