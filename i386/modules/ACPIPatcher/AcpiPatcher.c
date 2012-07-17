/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */

/*
 * Copyright (c) 2010,2012 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#include "libsaio.h"
#include "modules.h"
#include "bootstruct.h"
#include "acpi_patcher.h"

void AcpiPatcher_start(void);
void AcpiPatcher_start(void)
{	
  replace_system_function("_setup_acpi",&setupAcpi);
}