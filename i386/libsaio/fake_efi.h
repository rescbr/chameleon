/*
 * Copyright 2007 David F. Elliott.  All rights reserved.
 */

#ifndef __LIBSAIO_FAKE_EFI_H
#define __LIBSAIO_FAKE_EFI_H

/* Set up space for up to 10 configuration table entries */
#define MAX_CONFIGURATION_TABLE_ENTRIES (uint32_t)10
#include "efi.h"
#include "SMBIOS.h"
#include "device_tree.h"


extern void
setupFakeEfi(void);

extern EFI_STATUS setup_acpi (void);

extern EFI_STATUS addConfigurationTable(EFI_GUID const *pGuid, void *table, char const *alias);

extern struct SMBEntryPoint	*getSmbiosOriginal();

extern void setupSmbiosConfigFile(const char *filename);

extern void finalizeEFIConfigTable(void );

extern EFI_STATUS Register_Acpi_Efi(void* rsd_p, unsigned char rev );

extern void Register_Smbios_Efi(void* smbios);

#endif /* !__LIBSAIO_FAKE_EFI_H */
