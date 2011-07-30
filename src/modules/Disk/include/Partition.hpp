/*
 * Copyright (c) 2011 Evan Lojewski. All rights reserved.
 *
 */
#ifndef PARTITION_H
#define PARTITION_H

#include <IOKit/IOTypes.h>

#define INVALID_PARTITION   (-1)

class Disk;

class Partition
{
public:
    Partition(Disk* disk, UInt8 partitionNumber);
    ~Partition();
        
    virtual IOReturn    Read(UInt64 sector, UInt64 size, UInt8* buffer);
    virtual IOReturn    Write(UInt64 sector, UInt64 size, UInt8* buffer);
    
    virtual bool        probe();    
    
    //virtual uuid_t      getUUID();
    
    virtual UInt32      getPartitionNumber() { return mPartitionNumber; };
    virtual UInt8       getNumPartitions() { return mNumPartitions; };

protected:
    Disk                *mDisk;
    UInt64              mNumSectors;
    UInt64              mBeginSector;
    SInt8               mPartitionNumber;
    UInt8               mNumPartitions;
    //uuid_t              mUUID;

private:
    
};


class PartitionList
{
public:
    Partition       *entry;
    PartitionList   *next;
};


#endif /* PARTITION_H */
