/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Copyright 1993 NeXT Computer, Inc.
 * All rights reserved.
 */

/*  Copyright 2007 David Elliott
 2007-12-30 dfe
 - Enhanced code to normalize segment/offset to huge pointers so that any
 linear address within the first MB of memory can be passed to BIOS
 functions.  This allows some of the __DATA sections to span into the
 next segment and also allows stack variables to be used whereas the
 old code could only operate on static data in the first 64k.
 NOTE: Requires bios.s change to respect DS.
 */
/*  Copyright 2007 VMware Inc.
 2007-12-29 dfe
 - Added ebiosEjectMedia
 */

/*
 * Copyright (c) 2011 Evan Lojewski.
 *  - Converted to c++
 *  - Converted to UInt* style variable types
 */

#include <BiosDisk.hpp>

extern "C"
{
#include "libsaio.h"
#include "stdio.h"
}
        
BiosDisk::BiosDisk(const char* name)
{
    // Initialize
    mValid = false;
    mDiskID = 0;
    mUsesEBIOS = 0;
    mNoEmulation = false;
    mBytesPerSector = 0;

    
    // The correct format should be:
    // bios:/hdX/
    mBusType = "bios";

    if(strncmp(mBusType, name, 4) != 0) name = NULL;
    
    mName = name;
    
    sscanf(name, "bios:/hd%d/", &mDiskID);
    mDiskID += 0x80;    // 0x80 = fixed disk
    
    mDriveInfo = (boot_drive_info_t*)malloc(sizeof(boot_drive_info_t));

    if(BIOSRead(0, 0, 1, 1) == 0 && GetDriveInfo() == 0) 
    {
        mBytesPerSector = mDriveInfo->params.phys_nbps;
        printf("Disk: 0x%X (%d sectors)\n", mDiskID, mBytesPerSector);   
    }
    else
    {
        mValid = 0;    // force invalid.
    }
}

BiosDisk::~BiosDisk()
{
    if(mDriveInfo) free(mDriveInfo);
}

IOReturn BiosDisk::Read(UInt64 sector, UInt64 size, UInt8* buffer)
{
    if(!isValid()) return kIOReturnNoDevice;
    
    // Assume EBIOS capable for now...
    if(EBIOSRead(sector, size) == kIOReturnSuccess)
    {
        bcopy((void*)BIOS_ADDR, buffer, size * mBytesPerSector);
        return kIOReturnSuccess;
    }
    else return kIOReturnIOError;
}


IOReturn BiosDisk::Write(UInt64 sector, UInt64 size, UInt8* buffer)
{
    
    if(!isValid()) return kIOReturnNoDevice;
    
    bcopy(buffer, (void*)BIOS_ADDR, size * mBytesPerSector);
    
    if(EBIOSWrite(sector, size) == 0)
    {
        return kIOReturnSuccess;
    }
    else return kIOReturnIOError;
}


UInt8 BiosDisk::BIOSRead(UInt16 cylinder, UInt8 head, UInt8 sector, UInt8 count)
{
    sector++;   // starts at 1
    
    biosBuf_t bb;
    int i;
    
    bb.intno = 0x13;
    bb.eax.r.h = 0x02;

    for (i=0;;)
    {
        bb.ecx.r.h = cylinder;
        bb.ecx.r.l = ((cylinder & 0x300) >> 2) | (sector & 0x3F);
        bb.edx.r.h = head;
        bb.edx.r.l = mDiskID;
        bb.eax.r.l = count;
        bb.ebx.rr  = OFFSET(ptov(BIOS_ADDR));
        bb.es      = SEGMENT(ptov(BIOS_ADDR));

        bios(&bb);
        
        /* In case of a successful call, make sure we set AH (return code) to zero. */
        if (bb.flags.cf == 0) bb.eax.r.h = 0;
        
        /* Now we can really check for the return code (AH) value. */
        if ((bb.eax.r.h == 0x00) || (i++ >= 5)) break;
        
        /* reset disk subsystem and try again */
        bb.eax.r.h = 0x00;
        bios(&bb);
    }
    return bb.eax.r.h;
}

UInt8 BiosDisk::EBIOSRead(UInt64 sector, UInt8 count)
{
    biosBuf_t bb;
    
    UInt8 i;
    struct {
        UInt8  size;
        UInt8  reserved;
        UInt8  numblocks;
        UInt8  reserved2;
        UInt16 bufferOffset;
        UInt16 bufferSegment;
        UInt64 startblock;
    } addrpacket __attribute__((aligned(16))) = {0};
    addrpacket.size = sizeof(addrpacket);
    
    for (i=0;;) {
        bb.intno   = 0x13;
        bb.eax.r.h = 0x42;
        bb.edx.r.l = mDiskID;
        bb.esi.rr  = NORMALIZED_OFFSET((unsigned)&addrpacket);
        bb.ds      = NORMALIZED_SEGMENT((unsigned)&addrpacket);
        addrpacket.reserved = addrpacket.reserved2 = 0;
        addrpacket.numblocks     = count;
        addrpacket.bufferOffset  = OFFSET(ptov(BIOS_ADDR));
        addrpacket.bufferSegment = SEGMENT(ptov(BIOS_ADDR));
        addrpacket.startblock    = sector;
        bios(&bb);
        
        /* In case of a successful call, make sure we set AH (return code) to zero. */
        if (bb.flags.cf == 0)
            bb.eax.r.h = 0;
        
        /* Now we can really check for the return code (AH) value. */
        if ((bb.eax.r.h == 0x00) || (i++ >= 5))
            break;
        
        /* reset disk subsystem and try again */
        bb.eax.r.h = 0x00;
        bios(&bb);
    }
    return bb.eax.r.h;
}

UInt8 BiosDisk::EBIOSWrite(UInt64 sector, UInt8 count)
{
    biosBuf_t bb;

    UInt8 i;
    static struct {
        UInt8  size;
        UInt8  reserved;
        UInt8  numblocks;
        UInt8  reserved2;
        UInt16 bufferOffset;
        UInt16 bufferSegment;
        UInt64 startblock;
    } addrpacket __attribute__((aligned(16))) = {0};
    addrpacket.size = sizeof(addrpacket);
    
    for (i=0;;) {
        bb.intno   = 0x13;
        bb.eax.r.l = 0; /* Don't verify */
        bb.eax.r.h = 0x43;
        bb.edx.r.l = mDiskID;
        bb.esi.rr  = NORMALIZED_OFFSET((unsigned)&addrpacket);
        bb.ds      = NORMALIZED_SEGMENT((unsigned)&addrpacket);
        addrpacket.reserved = addrpacket.reserved2 = 0;
        addrpacket.numblocks     = count;
        addrpacket.bufferOffset  = OFFSET(ptov(BIOS_ADDR));
        addrpacket.bufferSegment = SEGMENT(ptov(BIOS_ADDR));
        addrpacket.startblock    = sector;
        bios(&bb);
        
        /* In case of a successful call, make sure we set AH (return code) to zero. */
        if (bb.flags.cf == 0)
            bb.eax.r.h = 0;
        
        /* Now we can really check for the return code (AH) value. */
        if ((bb.eax.r.h == 0x00) || (i++ >= 5))
            break;
        
        /* reset disk subsystem and try again */
        bb.eax.r.h = 0x00;
        bios(&bb);
    }
    return bb.eax.r.h;
}

UInt8 BiosDisk::GetDriveInfo()
{
    // we use the BIOS_ADDR (disk buffer) to ensure that the buffer
    // is in low mem
    boot_drive_info_t* actualInfo = mDriveInfo;
    mDriveInfo = (boot_drive_info_t*)BIOS_ADDR;
    biosBuf_t bb;
	int ret = 0;
	
#if UNUSED
	if (maxhd == 0) {
		bb.intno = 0x13;
		bb.eax.r.h = 0x08;
		bb.edx.r.l = 0x80;
		bios(&bb);
		if (bb.flags.cf == 0)
			maxhd = 0x7f + bb.edx.r.l;
	};
	
#endif
	
	/* Check for El Torito no-emulation mode. */
	//dp->no_emulation = is_no_emulation(drive);
    
	/* Check drive for EBIOS support. */
	bb.intno = 0x13;
	bb.eax.r.h = 0x41;
	bb.edx.r.l = mDiskID;
	bb.ebx.rr = 0x55aa;
	bios(&bb);
	
	if ((bb.ebx.rr == 0xaa55) && (bb.flags.cf == 0)) {
		/* Get flags for supported operations. */
		mUsesEBIOS = bb.ecx.r.l;
	}
	
	if (mUsesEBIOS & (EBIOS_ENHANCED_DRIVE_INFO | EBIOS_LOCKING_ACCESS | EBIOS_FIXED_DISK_ACCESS)) {
		/* Get EBIOS drive info. */
		
		mDriveInfo->params.buf_size = sizeof(mDriveInfo->params);
		bb.intno = 0x13;
		bb.eax.r.h = 0x48;
		bb.edx.r.l = mDiskID;
		bb.esi.rr = NORMALIZED_OFFSET((unsigned)&mDriveInfo->params);
		bb.ds	  = NORMALIZED_SEGMENT((unsigned)&mDriveInfo->params);
		bios(&bb);
        
		if (bb.flags.cf != 0 /* || params.phys_sectors < 2097152 */) {
			mUsesEBIOS = 0;
			mDriveInfo->params.buf_size = 1;
		}
		else
		{
			if (mDiskID >= BASE_HD_DRIVE &&
				(mUsesEBIOS & EBIOS_ENHANCED_DRIVE_INFO) &&
				mDriveInfo->params.buf_size >= 30 &&
				!(mDriveInfo->params.dpte_offset == 0xFFFF && mDriveInfo->params.dpte_segment == 0xFFFF)) {
                void *ptr = (void *)(mDriveInfo->params.dpte_offset + ((unsigned int)mDriveInfo->params.dpte_segment << 4));
                bcopy(ptr, &mDriveInfo->dpte, sizeof(mDriveInfo->dpte));
			}
		}
	}
    
    /*
     * zef: This code will fail on recent JMicron and Intel option ROMs
     */ 
    //	if (mDriveInfo->params.phys_heads == 0 || mDriveInfo->params.phys_spt == 0) {
    //		/* Either it's not EBIOS, or EBIOS didn't tell us. */
    //		bb.intno = 0x13;
    //		bb.eax.r.h = 0x08;
    //		bb.edx.r.l = drive;
    //		bios(&bb);
    //		if (bb.flags.cf == 0 && bb.eax.r.h == 0) {
    //			unsigned long cyl;
    //			unsigned long sec;
    //			unsigned long hds;
    //		
    //			hds = bb.edx.r.h;
    //			sec = bb.ecx.r.l & 0x3F;
    //			if ((dp->uses_ebios & EBIOS_ENHANCED_DRIVE_INFO) && (sec != 0)) {
    //				cyl = (mDriveInfo->params.phys_sectors / ((hds + 1) * sec)) - 1;
    //			} else {
    //				cyl = bb.ecx.r.h | ((bb.ecx.r.l & 0xC0) << 2);
    //			}
    //			mDriveInfo->params.phys_heads = hds; 
    //			mDriveInfo->params.phys_spt = sec;
    //			mDriveInfo->params.phys_cyls = cyl;
    //		} else {
    //			ret = -1;
    //		}
    //	}
    /*
     if (dp->no_emulation) {
     // Some BIOSes give us erroneous EBIOS support information.
     // Assume that if you're on a CD, then you can use
     // EBIOS disk calls.
     //
     dp->uses_ebios |= EBIOS_FIXED_DISK_ACCESS;
     }*/
    
	if (ret == 0) {
		mValid = 1;
	}

    bcopy(mDriveInfo, actualInfo, sizeof(boot_drive_info_t));
    mDriveInfo = actualInfo;
    
	return ret;
}

