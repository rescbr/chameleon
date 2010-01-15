/*
 * Copyright 2009 netkas
 */

#include "libsaio.h"
#include "bootstruct.h"

#ifndef DEBUG_PCIROOT
#define DEBUG_PCIROOT 0
#endif

#if DEBUG_PCIROOT==1
#define DBG(x...)  printf(x)
#else
#define DBG(x...)
#endif

int rootuid = 10; //value means function wasnt ran yet

unsigned int findrootuid(unsigned char * dsdt)
{
	int i;
	for (i=0; i<64; i++) //not far than 64 symbols from pci root 
	{
		if(dsdt[i] == '_' && dsdt[i+1] == 'U' && dsdt[i+2] == 'I' && dsdt[i+3] == 'D' && dsdt[i+5] == 0x08)
		{
			return dsdt[i+4];
		}
	}
	printf("pci root uid not found\n");
	return 11;
}

unsigned int findpciroot(unsigned char * dsdt,int size)
{
	int i;
	for (i=0; i<size; i++)
	{
		if(dsdt[i] == 'P' && dsdt[i+1] == 'C' && dsdt[i+2] == 'I' && (dsdt[i+3] == 0x08 || dsdt [i+4] == 0x08))
		{
			return findrootuid(dsdt+i);
		}
	}
	
	printf("pci root not found\n");
	return 10;
}


/* Setup ACPI. Replace DSDT if DSDT.aml is found */
int getPciRootUID()
{
	int fd, version;
	void *new_dsdt;
	const char *dsdt_filename;
	const char *val;
	int user_uid_value;
	char dirspec[512];
	int len,fsize;

	if(rootuid < 10) return rootuid;
	
	if (!getValueForKey("DSDT", &dsdt_filename, &len, &bootInfo->bootConfig))
		dsdt_filename="DSDT.aml";
	
	if (getValueForKey("-pci1", &val, &len, &bootInfo->bootConfig))  //fallback
	{
		user_uid_value = 1;
		rootuid = user_uid_value;
		return rootuid;
	}
	 else user_uid_value = 0;
		
		
	// Check booting partition
	sprintf(dirspec,"%s",dsdt_filename);
	fd=open (dirspec,0);
	if (fd<0)
	{	// Check Extra on booting partition
		sprintf(dirspec,"/Extra/%s",dsdt_filename);
		fd=open (dirspec,0);
		if (fd<0)
		{	// Fall back to booter partition
			sprintf(dirspec,"bt(0,0)/Extra/%s",dsdt_filename);
			fd=open (dirspec,0);
			if (fd<0)
			{
				verbose("No DSDT found, using 0 as uid value.\n");
				rootuid = user_uid_value;
				return rootuid;
			}
		}
	}
	
	// Load replacement DSDT
	new_dsdt=(void*)MALLOC(file_size (fd));
	if (!new_dsdt)
	{
		printf("Couldn't allocate memory for DSDT\n");
		rootuid = user_uid_value;
		return rootuid;
	}
	fsize = file_size(fd);
	if (read (fd, new_dsdt, file_size (fd))!=file_size (fd))
	{
		printf("Couldn't read file\n");
		rootuid = user_uid_value;
		return rootuid;
	}
	close (fd);
	rootuid=findpciroot(new_dsdt, fsize);
	if(rootuid == 11)rootuid=0; //usualy when _UID isnt present, it means uid is zero
	if(rootuid == 10) //algo failed, PCI0 wasnt found;
	{
		printf("pci root uid value wasnt found, using zero, if you want it to be 1, use -pci1 flag");
		rootuid = user_uid_value;
	}
	free(new_dsdt);
	return rootuid;
}

