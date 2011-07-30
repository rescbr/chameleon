/*
 * Copyright (c) 2011 Evan Lojewski. All rights reserved.
 *
 */
#ifndef GUID_PARTITION_H
#define GUID_PARTITION_H

#include <IOKit/IOTypes.h>
#include <Partition.hpp>
#include <IOKit/storage/IOGUIDPartitionScheme.h>

#pragma pack(push, 1)                        /* (enable 8-bit struct packing) */

typedef struct
{
    UInt8  hdr_sig[8];
    UInt32 hdr_revision;
    UInt32 hdr_size;
    UInt32 hdr_crc_self;
    UInt32 __reserved;
    UInt64 hdr_lba_self;
    UInt64 hdr_lba_alt;
    UInt64 hdr_lba_start;
    UInt64 hdr_lba_end;
    UInt8  hdr_uuid[16];
    UInt64 hdr_lba_table;
    UInt32 hdr_entries;
    UInt32 hdr_entsz;
    UInt32 hdr_crc_table;
    UInt32 padding;
} GPTHeader;

/* Partition map entry. */

typedef struct
{
    UInt8  ent_type[16];
    UInt8  ent_uuid[16];
    UInt64 ent_lba_start;
    UInt64 ent_lba_end;
    UInt64 ent_attr;
    UInt16 ent_name[36];
} GPTEntry;

#pragma pack(pop)

class GUIDPartition : public Partition
{
public:
    GUIDPartition(Disk* disk, UInt8 partitionNumber);
    ~GUIDPartition();
    
protected:
    bool        isGPTDisk();
    
private:
    UInt8       mLBABUffer[512*4    ];    // TODO: don't assume 512
    GPTHeader*  mGPTHeader;
    GPTEntry*   mGPTEntry;
};

#endif /* GUID_PARTITION_H */
