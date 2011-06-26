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
    bool        isGPTDisk();
    
private:
    UInt8       mLBABUffer[512];    // TODO: don't assume 512
    gpt_hdr*    mGPTHeader;
    gpt_ent*    mGPTEntry;
};

#endif /* GUID_PARTITION_H */
