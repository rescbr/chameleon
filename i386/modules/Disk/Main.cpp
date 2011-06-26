/*
 * Copyright (c) 2011 Evan Lojewski. All rights reserved.
 *
 */

#include <IOKit/IOTypes.h>
#include <BiosDisk.hpp>

extern "C"
{
#include "libsaio.h"
    void Disk_start();
}

void Disk_start()
{
    UInt8* mbr = (UInt8*)malloc(512);
    
    BiosDisk* disk = new BiosDisk("bios:/hd0/");
    disk->Read(0, 1, mbr);
    printf("mbr[0] = 0x%X\n", mbr[0]);

    printf("mbr[510] = 0x%X\n", mbr[510]);
    printf("mbr[511] = 0x%X\n", mbr[511]);
    printf("This is a simple BootDisk_start test.\n");
    delete disk;
    
    

    halt();
}