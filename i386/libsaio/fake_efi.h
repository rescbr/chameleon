/*
 * Copyright 2007 David F. Elliott.  All rights reserved.
 */

#ifndef __LIBSAIO_FAKE_EFI_H
#define __LIBSAIO_FAKE_EFI_H

/* Set up space for up to 10 configuration table entries */
#define MAX_CONFIGURATION_TABLE_ENTRIES (uint32_t)10
#include "efi.h"
#include "SMBIOS.h"


extern void
setupFakeEfi(void);

extern EFI_STATUS addConfigurationTable();

extern struct SMBEntryPoint	*getSmbiosOriginal();

extern void setupSmbiosConfigFile(const char *filename);

extern void finalizeEFIConfigTable(void );

#endif /* !__LIBSAIO_FAKE_EFI_H */
