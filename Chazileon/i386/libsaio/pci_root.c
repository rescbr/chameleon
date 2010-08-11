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

static unsigned int findrootuid(unsigned char * dsdt, int len)
{
	int i;
	for (i=0; i<64 && i<len-5; i++) //not far than 64 symbols from pci root 
	{
		if(dsdt[i] == '_' && dsdt[i+1] == 'U' && dsdt[i+2] == 'I' && dsdt[i+3] == 'D' && dsdt[i+5] == 0x08)
		{
			return dsdt[i+4];
		}
	}
	return 11; // _UID isnt present on ACPI data.
}

static unsigned int findpciroot(unsigned char * dsdt,int len)
{
	int i;
	
	for (i=0; i<len-4; i++)
	{
		if(dsdt[i] == 'P' && dsdt[i+1] == 'C' && dsdt[i+2] == 'I' && (dsdt[i+3] == 0x08 || dsdt [i+4] == 0x08))
		{
			return findrootuid(dsdt+i, len-i);
		}
	}
	return 10;
}

int getPciRootUID(void)
{
	const char *val;
	const char * dsdt_filename = NULL; //Azi:dsdt
	int len, fd, fsize;
	void *new_dsdt;
	//Azi: warning: implicit declaration of function ‘search_and_get_acpi_fd’, if removed... hum... but works fine!
	extern int search_and_get_acpi_fd(const char *, const char **);
	
	if (rootuid < 10) return rootuid;
	
	// If user supplied a key...
	if (getValueForKey(kPCIRootUIDKey, &val, &len, &bootInfo->bootConfig))
	{
		if (isdigit(val[0]))
			rootuid = val[0] - '0';
		// ... use the value in it.
		verbose("Using PCI-Root-UID value from PciRoot key: %d\n", rootuid);
		goto out_out;
	}
	
	// Search for a user supplied ACPI Table, to fetch the value from.
	fd = search_and_get_acpi_fd("DSDT.aml", &dsdt_filename); //Azi:dsdt call 1
	
	// If no ACPI Table is supplied by user...
	if (fd < 0)
	{
		verbose("No ACPI Table supplied by user.\n");
		rootuid = 0; // default uid to 0.
		goto out;
	}
	
	// Found ACPI Table supplied by user, check size...
	fsize = file_size(fd);
	
	// ... try to allocate memory...
	if ((new_dsdt = malloc(fsize)) == NULL)
	{
		verbose("[ERROR] DSDT memory allocation failed.\n");
		close (fd); // ... if allocation fails close file.
		rootuid = 0; // Default uid to 0.
		goto out;
	}
	
	// Try to read file...
	if (read(fd, new_dsdt, fsize) != fsize)
	{
		verbose("[ERROR] read %s failed.\n", dsdt_filename);
		close (fd); // ... if reading fails close file.
		rootuid = 0; // Default uid to 0.
		goto out;
	}
	// else new_dsdt = supplied ACPI Table data.
	close (fd); // Supplied Table can be closed.
	
	// Find value on supplied data.
	rootuid = findpciroot(new_dsdt, fsize);
	free(new_dsdt); // Free allocated data.
	
	// If _UID isn't present on ACPI data (value 11)...
	if (rootuid < 0 || rootuid > 9) // ... it's negative or bigger than 9...
	{
		rootuid = 0; // ... default uid to 0.
	}
	else
		printf("Found UID value on ACPI Table provided by user...\n");
out:
	verbose("Using PCI-Root-UID value: %d\n", rootuid);
out_out:
	return rootuid;
}
