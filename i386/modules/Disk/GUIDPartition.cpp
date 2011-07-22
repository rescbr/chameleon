/*
 * Copyright (c) 2011 Evan Lojewski. All rights reserved.
 *
 */
#include <GUIDPartition.hpp>
#include <Disk.hpp>

extern "C"
{
#include "libsaio.h"
#include "stdio.h"
}

GUIDPartition::GUIDPartition(Disk* disk, UInt8 partitionNumber) : Partition(disk, partitionNumber)
{
    UInt32 entrySize = 0;
    
    mDisk = disk;
    mNumSectors = 0;
    mBeginSector = 0;
    mPartitionNumber = partitionNumber;
    mNumPartitions = 0;
    
    if(mDisk)
    {
        // read in lba1, this contains the partition map if it's GPT
        mDisk->Read(1, 1, mLBABUffer);
        mGPTHeader = (GPTHeader*)mLBABUffer;
        entrySize = mGPTHeader->hdr_entsz;

    }
    else
    {
        mGPTHeader = NULL;
    }
    
    if(isGPTDisk())
    {
        mNumPartitions = mGPTHeader->hdr_entries;
    }
    
    mGPTHeader = NULL;

    if(entrySize && mPartitionNumber >= 0 && mPartitionNumber < mNumPartitions)
    {
        printf("Partitions %d at LBA %d\n", mPartitionNumber, 2 + (mPartitionNumber / 4));
        // TODO: Verify partition is valid + offset.
        mDisk->Read(2 + (mPartitionNumber / 4), 1, mLBABUffer);
        
        for(int i = 0; i < 10; i++)
        {
            for(int j = 0; j < 15; j++)
            {
                printf("%x ", mLBABUffer[i*15+j]);
            }
            printf("\n");
                       
        }
        
        UInt32 offset = (mPartitionNumber % 4) * entrySize;
        printf("\toffset %d at LBA %d\n", offset);

        mGPTEntry = (GPTEntry*) /*mLBABUffer*/ BIOS_ADDR + offset;
        
        printf("UUID0 %X%X%X%X\n", mGPTEntry->ent_type[3], mGPTEntry->ent_type[2], mGPTEntry->ent_type[1], mGPTEntry->ent_type[0]);
        printf("mGPTEntry = %d\n", mGPTEntry);

        mNumSectors = mGPTEntry->ent_lba_end - mGPTEntry->ent_lba_start;
        printf("mNumSectors = %d\n", mNumSectors);

        printf("LBS Start: %lld\tEnd: %lld\n", mGPTEntry->ent_lba_start, mGPTEntry->ent_lba_end);
        mBeginSector = mGPTEntry->ent_lba_start;
        //mUUID = mGPTEntry->ent_uuid;
        pause();
    }
    
}

GUIDPartition::~GUIDPartition()
{
    
}


bool GUIDPartition::isGPTDisk()
{
    static bool status = false;
    
    if(status) return status;
    
    if(!mGPTHeader) return false;
    
    if(mGPTHeader->hdr_sig[0] == 'E' &&
       mGPTHeader->hdr_sig[1] == 'F' &&
       mGPTHeader->hdr_sig[2] == 'I' &&
       mGPTHeader->hdr_sig[3] == ' ' &&
       mGPTHeader->hdr_sig[4] == 'P' &&
       mGPTHeader->hdr_sig[5] == 'A' &&
       mGPTHeader->hdr_sig[6] == 'R' &&
       mGPTHeader->hdr_sig[7] == 'T') return true;
    
    return false;
}
