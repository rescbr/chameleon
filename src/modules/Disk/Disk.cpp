/*
 * Copyright (c) 2011 Evan Lojewski. All rights reserved.
 *
 */
#include <Disk.hpp>

Disk* Disk::gFirstDisk = NULL;

        
Disk::Disk()
{
	mNextDisk = NULL;
    mName = NULL;
    mPartitions = NULL;

	// Add disk entry to list
	
	Disk* list = gFirstDisk;
	if(!list) gFirstDisk = this;
	else {
		while(list->getNextDisk())
		{
			list = list->getNextDisk();
		}
		// Last disk found, add use to the end of the list
		
		list->setNextDisk(this);
	}

}

Disk::~Disk()
{
	// remove disk entry from list
	if(getPrevDisk() != NULL) getPrevDisk()->setNextDisk(getNextDisk());
	else gFirstDisk = NULL;
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