/*
 * Copyright 2007 David F. Elliott.  All rights reserved.
 */

#ifndef __LIBSAIO_FAKE_EFI_H
#define __LIBSAIO_FAKE_EFI_H

/* Set up space for up to 10 configuration table entries */
#define MAX_CONFIGURATION_TABLE_ENTRIES 10
#include "efi.h"
#include "SMBIOS.h"

extern void
setupFakeEfi(void);

extern uint64_t acpi10_p;
extern uint64_t acpi20_p;

extern EFI_STATUS addConfigurationTable();

extern struct SMBEntryPoint	*getSmbiosOriginal();

extern void setupSmbiosConfigFile(const char *filename);

extern void local_readSMBIOS(int value);

#endif /* !__LIBSAIO_FAKE_EFI_H */
