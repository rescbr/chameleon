/*
 * Copyright (c) 2011 Evan Lojewski. All rights reserved.
 *
 */
#ifndef DISK_H
#define DISK_H

#include <IOKit/IOTypes.h>

class Disk
{
public:
    Disk();
    Disk(const char* name);
    ~Disk();
        
    virtual IOReturn    Read(UInt64 sector, UInt64 size, UInt8* buffer) = 0;
    virtual IOReturn    Write(UInt64 sector, UInt64 size, UInt8* buffer) = 0;

    virtual bool        isValid()           { return mName != NULL && mBytesPerSector; };
    virtual UInt32      bytesPerSector()    { return mBytesPerSector; };
protected:
    const char      *mName;
    const char      *mBusType;
    
    UInt32          mBytesPerSector;
private:

};

#endif /* DISK_H */
