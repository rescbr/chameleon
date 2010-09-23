#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
#include "modules.h"

void setup_pci_devs(pci_dt_t *pci_dt)
{
	pci_dt_t *current = pci_dt;


	while (current)
	{
		execute_hook("PCIDevice", current, NULL, NULL, NULL);
				
		setup_pci_devs(current->children);
		current = current->next;
	}
}
