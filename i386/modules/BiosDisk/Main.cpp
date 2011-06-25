/*
 * Copyright (c) 2011 Evan Lojewski. All rights reserved.
 *
 */

#include <IOKit/IOTypes.h>
#include <BiosDisk.hpp>

extern "C"
{
#include "libsaio.h"
    void BiosDisk_start();
}

void BiosDisk_start()
{
    BiosDisk* disk = new BiosDisk("bios:/hd0/");

    printf("This is a simple BootDisk_start test.\n");
    delete disk;

    halt();
}