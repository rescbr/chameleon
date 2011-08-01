/*
 * Copyright (c) 2011 Evan Lojewski. All rights reserved.
 *
 */

#include <IOKit/IOTypes.h>
#include <BiosDisk.hpp>
#include <GUIDPartition.hpp>
#include <FDiskPartition.hpp>

#define MAXDEV      32
#define BIOSBUFFER      512 * 8 /* 4Kb */
extern "C"
{
#include "libsaio.h"
    void Disk_start();
}


Disk*		disks[MAXDEV];
UInt8       numBiosDisks = 0;
UInt8*      diskBuffer = NULL;

void DetermineDisks();

void Disk_start()
{
    //diskBuffer = (UInt8*)malloc(BIOSBUFFER);
    /*UInt8* mbr = (UInt8*)malloc(512);
    
        disk->Read(0, 1, mbr);
        printf("mbr[0] = 0x%X\n", mbr[0]);
        
        printf("mbr[510] = 0x%X\n", mbr[510]);
        printf("mbr[511] = 0x%X\n", mbr[511]);
        
        
        GUIDPartition* partition = new GUIDPartition(disk, 0);
        if(partition->probe())
        {
            partition->Read(0, 1, mbr);
            printf("GUID[0]/[0] = 0x%X\n", mbr[0]);

        }
    }    
    */
    DetermineDisks();
    
    printf("This is a simple BootDisk_start test.\n");
    
    

    halt();
}

void DetermineDisks()
{
    /*char diskName[] = "bios:/hd%s/";
    
    for(int i = 0; i < MAXBIOSDEV; i++)
    {
        sprintf(diskName, "bios:/hd%d/", i);
        BiosDisk* disk = new BiosDisk(diskName);
        if(disk->probe())
        {
            printf("Disk %s exists\n", diskName);
            disks[numBiosDisks++] = disk;
            
            
            // Check if the disk is GUID
            GUIDPartition* partition = new GUIDPartition(disk, 0);
            UInt8 partitions = partition->getNumPartitions();
            printf("GUID Disk w/ %d disks\n", partitions);
            
            for(UInt8 partno = 0; partno < partitions; partno++)
            {
                GUIDPartition* partition = new GUIDPartition(disk, partno);
                if(partition->probe())
                {
                    printf("Partition %d valid\n", partno);
                }
            }

            
            // If not, check if the disk is MBR
            FDiskPartition* mbrdisk = new FDiskPartition(disk, 0);
            partitions = mbrdisk->getNumPartitions();
            printf("MBR Disk w/ %d disks\n", partitions);

            
        }
        else
        {
            delete disk;
        }

    }
	 */
}
