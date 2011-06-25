/*
 * Copyright (c) 2011 Evan Lojewski. All rights reserved.
 *
 */
#include <BiosDisk.hpp>

        
BiosDisk::BiosDisk(const char* name)
{
    busType = "bios";

    // fixme
    if(name[0] != 'b' &&
       name[1] != 'i' &&
       name[2] != 'o' &&
       name[3] != 's' &&
       name[4] != ':') name = NULL;
    
    mName = name;
    
    // TODO: convert mName to bios disk id
}

BiosDisk::~BiosDisk()
{

}

IOReturn BiosDisk::Read(UInt64 sector, UInt64 size, char* buffer)
{
    if(!isValid()) return kIOReturnNoDevice;
    return kIOReturnSuccess;
}


IOReturn BiosDisk::Write(UInt64 sector, UInt64 size, char* buffer)
{
    if(!isValid()) return kIOReturnNoDevice;
    return kIOReturnNotWritable;
}
