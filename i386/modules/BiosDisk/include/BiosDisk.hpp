/*
 * Copyright (c) 2011 Evan Lojewski. All rights reserved.
 *
 */
#ifndef BIOSDISK_H
#define BIOSDISK_H

#include <IOKit/IOTypes.h>
#include <Disk.hpp>


#define super Disk

class BiosDisk : public Disk
{
public:
    BiosDisk(const char* name);
    ~BiosDisk();
        
    virtual IOReturn    Read(UInt64 sector, UInt64 size, char* buffer);
    virtual IOReturn    Write(UInt64 sector, UInt64 size, char* buffer);

protected:
    
private:
    UInt8           mDiskID;
};

#endif /* BIOSDISK_H */
