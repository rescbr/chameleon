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
        
    virtual IOReturn    Read(UInt64 sector, UInt64 size, char* buffer) = 0;
    virtual IOReturn    Write(UInt64 sector, UInt64 size, char* buffer) = 0;

    bool        isValid() { return mName != NULL; };
protected:
    const char      *mName;
    const char      *busType;
    
private:

};

#endif /* DISK_H */
