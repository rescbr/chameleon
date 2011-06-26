/*
 * Copyright (c) 2011 Evan Lojewski. All rights reserved.
 *
 */
#include <GUIDPartition.hpp>


GUIDPartition::GUIDPartition(Disk* disk, UInt8 partitionNumber) : Partition(disk, partitionNumber)
{
    mDisk = disk;
    mNumSectors = 0;
    mBeginSector = 0;
    mPartitionNumber = partitionNumber;
    mNumPartitions = 0;
    
    if(mDisk)
    {
        // read in lba1, this contains the partition map if it's GPT
        mDisk->Read(1, 1, mLBABUffer);
        mGPTHeader = (gpt_hdr*)mLBABUffer;
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
    
    if(mPartitionNumber >= 0 && mPartitionNumber < mNumPartitions)
    {
        // read in partition entry.
        mDisk->Read(2 + mPartitionNumber, 1, mLBABUffer);
        
        mGPTEntry = (gpt_ent*) mLBABUffer;
        
        mNumSectors = mGPTEntry->ent_lba_end - mGPTEntry->ent_lba_start;
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
