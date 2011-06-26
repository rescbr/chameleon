/*
 * Copyright (c) 2011 Evan Lojewski. All rights reserved.
 *
 */
#ifndef GUID_PARTITION_H
#define GUID_PARTITION_H

#include <IOKit/IOTypes.h>
#include <Partition.hpp>
#include <IOKit/storage/IOGUIDPartitionScheme.h>

class GUIDPartition : Partition
{
public:
    GUIDPartition(Disk* disk, UInt8 partitionNumber);
    ~GUIDPartition();
    
protected:
    
private:
};

#endif /* FDISK_PARTITION_H */
