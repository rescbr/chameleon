#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
#include "modules.h"

#define DEBUG_PCI 0

#if DEBUG_PCI
#define DBG(x...)  msglog(x)
#else
#define DBG(x...)
#endif


void setup_pci_devs(pci_dt_t *pci_dt)
{
	pci_dt_t *current = pci_dt;


	while (current)
	{
		execute_hook("PCIDevice", current, NULL, NULL, NULL);
		DBG("setup_pci_devs current devID=%08x\n", current->device_id);
		setup_pci_devs(current->children);
		DBG("setup_pci_devs children devID=%08x\n", current->device_id);		
		current = current->next;
	}
}
