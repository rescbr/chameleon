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
        mDisk->Read(2 + (mPartitionNumber / 4), 1, mLBABUffer);
        
        UInt32 offset = (mPartitionNumber % 4) * entrySize;
        
        mGPTEntry = (GPTEntry*) mLBABUffer + offset;
        
        // TODO: verify partition type != NULL
        
        mNumSectors = (mGPTEntry->ent_lba_end - mGPTEntry->ent_lba_start) + 1;
        mBeginSector = mGPTEntry->ent_lba_start;
        //mUUID = mGPTEntry->ent_uuid;
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
