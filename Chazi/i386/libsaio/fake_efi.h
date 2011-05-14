/*
 * Copyright 2007 David F. Elliott.  All rights reserved.
 */

#ifndef __LIBSAIO_FAKE_EFI_H
#define __LIBSAIO_FAKE_EFI_H

#include "pci.h" //Azi: needed for pci_dt...

/* Set up space for up to 10 configuration table entries */
#define MAX_CONFIGURATION_TABLE_ENTRIES 10

extern void setupFakeEfi(void);
extern void setup_pci_devs(pci_dt_t *pci_dt);
 
#endif /* !__LIBSAIO_FAKE_EFI_H */
