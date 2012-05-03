/*
 * Copyright (c) 2010,2012 cparm <armelcadetpetit@gmail.com>. All rights reserved.
 *
 */

#include "libsaio.h"
#include "modules.h"
#include "bootstruct.h"
#include "pci_root.h"
#include "acpi_codec.h"

void ACPICodec_start(void);
void ACPICodec_start(void)
{   
    replace_system_function("_setup_acpi",&setupAcpi);        
}