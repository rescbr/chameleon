/*
 * Copyright 2008 mackerintel
 */
/*
 * AsereBLN: cleanup
 */

#ifndef __LIBSAIO_SMBIOS_PATCHER_H
#define __LIBSAIO_SMBIOS_PATCHER_H

#include "libsaio.h"
#include "SMBIOS.h"
#include "fake_efi.h"

#define kSMBIOSdefaults		"SMBIOSdefaults"	/* smbios_patcher.c */

extern uint64_t smbios_p;

struct smbios_table_header 
{
	uint8_t		type;
	uint8_t		length;
	uint16_t	handle;
} __attribute__ ((packed));

struct smbios_property
{
	const char		*name;
	uint8_t		table_type;
	enum {SMSTRING, SMWORD, SMBYTE, SMOWORD} value_type;
	int		offset;
	int		(*auto_int) (const char *name, int table_num);
	const char	*(*auto_str) (const char *name, int table_num);
	const char	*(*auto_oword) (const char *name, int table_num);
};

struct smbios_table_description
{
	uint8_t		type;
	int		len;
	int		(*numfunc)(int tablen);
};

extern void scan_memory(PlatformInfo_t *);
extern const char* sm_get_defstr(const char * key, int table_num);
extern struct SMBEntryPoint	*getSmbiosPatched(struct SMBEntryPoint *orig);
//extern char* getSmbiosProductName();
extern struct DMIHeader* FindFirstDmiTableOfType(int type, int minlength);
extern struct DMIHeader* FindNextDmiTableOfType(int type, int minlen);
extern const char * smbiosStringAtIndex(DMIHeader*, int index, int *length );
extern void getSmbiosTableStructure(struct SMBEntryPoint *smbios);
#endif /* !__LIBSAIO_SMBIOS_PATCHER_H */
