/*
 * Copyright (c) 2011 Evan Lojewski. All rights reserved.
 *
 */
#ifndef BIOSDISK_H
#define BIOSDISK_H

#include <IOKit/IOTypes.h>
#include <Disk.hpp>


#define super Disk

/*
 * BIOS drive information.
 */
struct boot_drive_info {
	struct drive_params {
        UInt16 buf_size;
        UInt16 info_flags;
        UInt32 phys_cyls;
        UInt32 phys_heads;
        UInt32 phys_spt;
        UInt64 phys_sectors;
        UInt16 phys_nbps;
        UInt16 dpte_offset;
        UInt16 dpte_segment;
        /*
        UInt16 key;
        UInt8  path_len;
        UInt8  reserved1;
        UInt16 reserved2;
        UInt8  bus_type[4];
        UInt8  interface_type[8];
        UInt8  interface_path[8];
        UInt8  dev_path[8];
        UInt8  reserved3;
        UInt8  checksum;
         */
	} params;
	struct drive_dpte {
        UInt16 io_port_base;
        UInt16 control_port_base;
        UInt8  head_flags;
        UInt8  vendor_info;
        UInt8  irq		   : 4;
        UInt8  irq_unused  : 4;
        UInt8  block_count;
        UInt8  dma_channel : 4;
        UInt8  dma_type	   : 4;
        UInt8  pio_type	   : 4;
        UInt8  pio_unused  : 4;
        UInt16 option_flags;
        UInt16 reserved;
        UInt8  revision;
        UInt8  checksum;
	} dpte;
} __attribute__((packed));
typedef struct boot_drive_info boot_drive_info_t;


class BiosDisk : public Disk
{
public:
    BiosDisk(const char* name);
    ~BiosDisk();
        
    virtual IOReturn    Read(UInt64 sector, UInt64 size, UInt8* buffer);
    virtual IOReturn    Write(UInt64 sector, UInt64 size, UInt8* buffer);
        virtual bool        isValid() { return (mValid && (mName != NULL) && (mDiskID & 0x80)); };

protected:
    UInt8               BIOSRead(UInt16 cylinder, UInt8 head, UInt8 sector, UInt8 count);
    
    UInt8               EBIOSRead(UInt64 sector, UInt8 count);
    UInt8               EBIOSWrite(UInt64 sector, UInt8 count);

    UInt8               GetDriveInfo();

private:
    UInt8               mDiskID;
    
    UInt8               mUsesEBIOS;
    bool                mNoEmulation;
    bool                mValid;
    
    boot_drive_info_t* mDriveInfo;

};


#endif /* BIOSDISK_H */
