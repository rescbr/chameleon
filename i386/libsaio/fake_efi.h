/*
 * Copyright 2007 David F. Elliott.  All rights reserved.
 */

#ifndef __LIBSAIO_FAKE_EFI_H
#define __LIBSAIO_FAKE_EFI_H

/* Set up space for up to 10 configuration table entries */
#define MAX_CONFIGURATION_TABLE_ENTRIES 10

extern void setupFakeEfi(void);
extern char MacModel[8];
extern char MacProduct[14];
extern unsigned int ModelRev;

#endif /* !__LIBSAIO_FAKE_EFI_H */
