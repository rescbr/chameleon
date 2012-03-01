/*
 * Copyright 2009 netkas
 */

#include "pci_root.h"
#include "bootstruct.h"
#include "Platform.h"

#ifndef DEBUG_PCIROOT
#define DEBUG_PCIROOT 0
#endif

#if DEBUG_PCIROOT
#define DBG(x...)  printf(x)
#else
#define DBG(x...)
#endif

static int rootuid = 10; //value means function wasnt ran yet


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
