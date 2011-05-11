/*
 * Copyright 2010 AsereBLN. All rights reserved. <aserebln@googlemail.com>
 *
 * mem.h
 */

#ifndef __LIBSAIO_MEM_H
#define __LIBSAIO_MEM_H

#include "platform.h"

#if UNUSED
extern void scan_memory(PlatformInfo_t *);
#endif

extern void dumpPhysAddr(const char * title, void * a, int len);

#endif	/* __LIBSAIO_MEM_H */
