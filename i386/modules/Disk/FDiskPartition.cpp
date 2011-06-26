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
}

FDiskPartition::~FDiskPartition()
{

}