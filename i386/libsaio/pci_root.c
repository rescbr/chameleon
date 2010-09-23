/*
 * Copyright 2009 netkas
 */

#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"

#ifndef DEBUG_PCIROOT
#define DEBUG_PCIROOT 1
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
	extern int search_and_get_acpi_fd(const char *, const char **);

	if (rootuid < 10) return rootuid;
	rootuid = 0;	/* default uid = 0 */

	if (getValueForKey(kPCIRootUID, &val, &len, &bootInfo->bootConfig)) {
		if (isdigit(val[0])) rootuid = val[0] - '0';
	}
	/* Chameleon compatibility */
	else if (getValueForKey("PciRoot", &val, &len, &bootInfo->bootConfig)) {
		if (isdigit(val[0])) rootuid = val[0] - '0';
	}
	/* PCEFI compatibility */
	else if (getValueForKey("-pci0", &val, &len, &bootInfo->bootConfig)) {
		rootuid = 0;
	}
	else if (getValueForKey("-pci1", &val, &len, &bootInfo->bootConfig)) {
		rootuid = 1;
	}

	verbose("Using PCI-Root-UID value: %d\n", rootuid);
	return rootuid;
}
