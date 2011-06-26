/*
 * Copyright (c) 2011 Evan Lojewski. All rights reserved.
 *
 */
#include <FDiskPartition.hpp>


FDiskPartition::FDiskPartition(Disk* disk, UInt8 partitionNumber) : Partition(disk, partitionNumber)
{
    mDisk = disk;
    mNumSectors = 0;
    mBeginSector = 0;
    mPartitionNumber = partitionNumber;
    mNumPartitions = 0;
    mFdiskEntry = NULL;
    
    if(mDisk)
    {
        // read in lba0, this contains the partition map if it's FDisk / MBR
        mDisk->Read(0, 1, (UInt8*)&mLBA0);
        
        if(isMBRDisk())
        {
            // Determine number of partition. TODO: Extended partitions
            UInt8 i = 0;
            for(i = 0; i < DISK_NPART; i++)
            {
                if(mLBA0.parts[i].systid != 0)
                {
                    if(mPartitionNumber == mNumPartitions) mFdiskEntry = &mLBA0.parts[i];
                    mNumPartitions++;
                }
                
            }
            
            if(mFdiskEntry)
            {
                mNumSectors = mFdiskEntry->numsect;
                mBeginSector = mFdiskEntry->relsect;
            }
        }
    }    
}

FDiskPartition::~FDiskPartition()
{
    
}

bool FDiskPartition::isMBRDisk()
{
    return mLBA0.signature == DISK_SIGNATURE;
}
