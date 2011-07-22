/*
 * Copyright (c) 2011 Evan Lojewski. All rights reserved.
 *
 */
#include <Partition.hpp>
#include <Disk.hpp>   
Partition::Partition(Disk* disk, UInt8 partitionNumber)
{
    mDisk = disk;
    mNumSectors = 0;
    mBeginSector = 0;
    mPartitionNumber = partitionNumber;
    mNumPartitions = 0;
}

Partition::~Partition()
{

}

IOReturn Partition::Read(UInt64 sector, UInt64 size, UInt8* buffer)
{
    if(probe() && mDisk->probe()) return mDisk->Read(sector + mBeginSector, size, buffer);
    else return kIOReturnUnsupported;
}

IOReturn Partition::Write(UInt64 sector, UInt64 size, UInt8* buffer)
{
    if(probe() && mDisk->probe()) return mDisk->Write(sector + mBeginSector, size, buffer);
    else return kIOReturnUnsupported;
}

bool Partition::probe()
{
    return (mDisk != NULL) && (mPartitionNumber != INVALID_PARTITION) && mNumSectors;
}

/*
Partition::getUUID()
{
    return mUUID;
}
*/