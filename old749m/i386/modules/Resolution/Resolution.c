/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

//#include "libsaio.h"
//#include "915resolution.h"
extern void patchVideoBios(void);

void Resolution_start()
{
	patchVideoBios();
}

