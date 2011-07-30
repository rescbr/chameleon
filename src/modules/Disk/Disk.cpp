/*
 * Copyright (c) 2011 Evan Lojewski. All rights reserved.
 *
 */
#include <Disk.hpp>

        
Disk::Disk()
{
    mName = NULL;
    mPartitions = NULL;
}

Disk::~Disk()
{

}

Partition*  Disk::getPartition(UInt32 index)
{
    PartitionList* current = mPartitions;
    while(current)
    {
        if(current->entry->getPartitionNumber() == index)
        {
            return current->entry;
        }
        else
        {
            current = current->next;
        }
    }
    
    return NULL;
}

void Disk::addPartition(Partition* partition)
{
    PartitionList* list = new PartitionList;
    list->entry = partition;
    list->next = mPartitions;
    mPartitions = list;
}