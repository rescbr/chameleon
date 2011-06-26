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
}

GUIDPartition::~GUIDPartition()
{
    
}