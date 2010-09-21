/*
 * Copyright 2008 mackerintel
 */

#ifndef __LIBSAIO_PCI_ROOT_H
#define __LIBSAIO_PCI_ROOT_H

#include "libsaio.h"


extern int getPciRootUID(void);
unsigned int findpciroot(unsigned char * dsdt,int len);

#endif /* !__LIBSAIO_DSDT_PATCHER_H */
