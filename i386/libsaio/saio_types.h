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
/* Useful types. */

#ifndef __LIBSAIO_SAIO_TYPES_H
#define __LIBSAIO_SAIO_TYPES_H

#include <sys/reboot.h>
#include <sys/types.h>
#include "bios.h"
#ifdef NBP_SUPPORT
#include "nbp_cmd.h"
#else
#include <IOKit/IOTypes.h>
#endif
#include "bootXnu.h"

#if DEBUG
#define DEBUG_DISK(x)    printf x
#else
#define DEBUG_DISK(x)
#endif

/*
 * Keys used in system Boot.plist
 */
#define kGraphicsModeKey	"Graphics Mode"
#define kTextModeKey		"Text Mode"
#define kQuietBootKey		"Quiet Boot"
#define kKernelFlagsKey		"Kernel Flags"
#define kMKextCacheKey		"MKext Cache"
#define kKernelNameKey		"Kernel"
#define kKernelCacheKey     "Kernel Cache"
#define kUseKernelCache     "UseKernelCache"    
#define kBootDeviceKey		"Boot Device"
#define kTimeoutKey			"Timeout"
#define kRootDeviceKey		"rd"
#define kBootUUIDKey		"boot-uuid"
#define kHelperRootUUIDKey	"Root UUID"
#define kPlatformKey		"platform"
#define kCDROMPromptKey		"CD-ROM Prompt"
#define kCDROMOptionKey		"CD-ROM Option Key"
#define kRescanPromptKey	"Rescan Prompt"
#define kRescanKey		    "Rescan"
#define kScanSingleDriveKey	"Scan Single Drive"
#define kInsantMenuKey		"Instant Menu"
#define kDefaultKernel		"mach_kernel"
#define kWaitForKeypressKey	"Wait"
/* AsereBLN: added the other keys */

#define kProductVersion		"ProductVersion"	/* boot.c */
#define karch				"arch"				/* boot.c */
#define kDeviceProperties	"device-properties"	/* device_inject.c */
#define kHidePartition		"Hide Partition"	/* disk.c */
#define kRenamePartition	"Rename Partition"	/* disk.c */
#define kSMBIOSKey			"SMBIOS"			/* fake_efi.c */
#define kSystemID			"SystemId"			/* fake_efi.c */
#define kSystemType			"SystemType"		/* fake_efi.c */
#define kPCIRootUID			"PCIRootUID"		/* pci_root.c */
#define kDefaultPartition	"Default Partition"	/* sys.c */
#define kAcpiMethod			"Acpi2Method"				// 2 (= method 2) for some machines that may hang on acpi 2 (aka acpi 64 bit) detection (replace the old kUnsafeACPI "UnsafeACPI") 	

#define PLATFORM_NAME_LEN 64
#define ROOT_PATH_LEN 256

#define DEFAULT_SCREEN_WIDTH 1024
#define DEFAULT_SCREEN_HEIGHT 768

/*
 * Max size fo config data array, in bytes.
 */
#define IO_CONFIG_DATA_SIZE		40960 // was 4096 // was 163840

#define rand_tab_len 100

struct ran_obj {
	int tab[rand_tab_len];
	int rmin;	
	int n;
};

typedef struct tm_t
{
    int tm_sec  ;    /* seconds after minute [0-61] (61 allows for 2 leap-seconds)*/
    int tm_min  ;    /* minutes after hour [0-59] */
    int tm_hour ;    /* hours after midnight [0-23] */
    int tm_mday ;    /* day of the month [1-31] */
    int tm_mon  ;    /* month of year [0-11] */
    int tm_year ;    /* current year-1900 */
    
}tm_t;

typedef unsigned long entry_t;

typedef struct {
    unsigned int sectors:8;
    unsigned int heads:8;
    unsigned int cylinders:16;
} compact_diskinfo_t;

struct driveParameters {
    int cylinders;
    int sectors;
    int heads;
    int totalDrives;
};

struct Tag {
	long       type;
	char       *string;
	long	   offset;
	struct Tag *tag;
	struct Tag *tagNext;
};
typedef struct Tag Tag, *TagPtr;

typedef struct {
	char	plist[IO_CONFIG_DATA_SIZE];	// buffer for plist
	TagPtr	dictionary;		// buffer for xml dictionary
	bool  canOverride;  // flag to mark a dictionary can be overriden
} config_file_t;

/*
 * BIOS drive information.
 */
struct boot_drive_info {
    struct drive_params {
	unsigned short buf_size;
	unsigned short info_flags;
	unsigned long  phys_cyls;
	unsigned long  phys_heads;
	unsigned long  phys_spt;
	unsigned long long phys_sectors;
	unsigned short phys_nbps;
	unsigned short dpte_offset;
	unsigned short dpte_segment;
	unsigned short key;
	unsigned char  path_len;
	unsigned char  reserved1;
	unsigned short reserved2;
	unsigned char  bus_type[4];
	unsigned char  interface_type[8];
	unsigned char  interface_path[8];
	unsigned char  dev_path[8];
	unsigned char  reserved3;
	unsigned char  checksum;
    } params __attribute__((packed));
    struct drive_dpte {
	unsigned short io_port_base;
	unsigned short control_port_base;
	unsigned char  head_flags;
	unsigned char  vendor_info;
	unsigned char  irq         : 4;
	unsigned char  irq_unused  : 4;
	unsigned char  block_count;
	unsigned char  dma_channel : 4;
	unsigned char  dma_type    : 4;
	unsigned char  pio_type    : 4;
	unsigned char  pio_unused  : 4;
	unsigned short option_flags;
	unsigned short reserved;
	unsigned char  revision;
	unsigned char  checksum;
    } dpte __attribute__((packed));
} __attribute__((packed));
typedef struct boot_drive_info boot_drive_info_t;

struct driveInfo {
    boot_drive_info_t di;
    int uses_ebios;
    int no_emulation;
    int biosdev;
    int valid;
};

typedef struct FinderInfo {
    unsigned char data[16];
} FinderInfo;

struct         BootVolume;
typedef struct BootVolume * BVRef;
typedef struct BootVolume * CICell;

typedef long (*FSInit)(CICell ih);
typedef long (*FSLoadFile)(CICell ih, char * filePath);
typedef long (*FSReadFile)(CICell ih, char *filePath, void *base, uint64_t offset, uint64_t length);
typedef long (*FSGetFileBlock)(CICell ih, char *filePath, unsigned long long *firstBlock);
typedef long (*FSGetDirEntry)(CICell ih, char * dirPath, long long * dirIndex,
                              char ** name, long * flags, long * time,
                              FinderInfo * finderInfo, long * infoValid);
typedef long (*FSGetUUID)(CICell ih, char *uuidStr);
typedef void (*BVGetDescription)(CICell ih, char * str, long strMaxLen);
// Can be just pointed to free or a special free function
typedef void (*BVFree)(CICell ih);

struct iob {
    unsigned int   i_flgs;          /* see F_* below */
    unsigned int   i_offset;        /* seek byte offset in file */
    int            i_filesize;      /* size of file */
    char *         i_buf;           /* file load address */
};

#define BPS        512              /* sector size of the device */
#define F_READ     0x1              /* file opened for reading */
#define F_WRITE    0x2              /* file opened for writing */
#define F_ALLOC    0x4              /* buffer allocated */
#define F_FILE     0x8              /* file instead of device */
#define F_NBSF     0x10             /* no bad sector forwarding */
#define F_SSI      0x40             /* set skip sector inhibit */
#define F_MEM      0x80             /* memory instead of file or device */

struct dirstuff {
    char *         dir_path;        /* directory path */
    long long      dir_index;       /* directory entry index */
    BVRef          dir_bvr;         /* volume reference */
};

#define BVSTRLEN 32

struct BootVolume {
    BVRef            next;            /* list linkage pointer */
    int              biosdev;         /* BIOS device number */
    int              type;            /* device type (floppy, hd, network) */
    unsigned int     flags;           /* attribute flags */
    BVGetDescription description;     /* BVGetDescription function */
    int              part_no;         /* partition number (1 based) */
    unsigned int     part_boff;       /* partition block offset */
    unsigned int     part_type;       /* partition type */
    unsigned int     fs_boff;         /* 1st block # of next read */
    unsigned int     fs_byteoff;      /* Byte offset for read within block */
    FSLoadFile       fs_loadfile;     /* FSLoadFile function */
    FSReadFile       fs_readfile;     /* FSReadFile function */
    FSGetDirEntry    fs_getdirentry;  /* FSGetDirEntry function */
    FSGetFileBlock   fs_getfileblock; /* FSGetFileBlock function */
    FSGetUUID        fs_getuuid;      /* FSGetUUID function */
    unsigned int     bps;             /* bytes per sector for this device */
    char             name[BVSTRLEN];  /* (name of partition) */
    char             type_name[BVSTRLEN]; /* (type of partition, eg. Apple_HFS) */
    BVFree           bv_free;         /* BVFree function */
    uint32_t         modTime;
    char			       label[BVSTRLEN]; /* partition volume label */
    char			       altlabel[BVSTRLEN]; /* partition volume label */
    bool             filtered;        /* newFilteredBVChain() will set to TRUE */
    bool             visible;         /* will shown in the device list */
    char			 OSVersion[8];
    bool			 kernelfound;       /* mach_kernel found in default location, currently only /mach_kernel is supported */
    bool             OSisServer;        /* 1 = OS X server , 0 = OS X client, not to be confused with Platform->CPU.isServer which means it tries to emulate an xserve in the smbios */
};

enum {
  kBVFlagPrimary          = 0x01,
  kBVFlagNativeBoot       = 0x02,
  kBVFlagForeignBoot      = 0x04,
  kBVFlagBootable         = 0x08,
  kBVFlagEFISystem        = 0x10,
#ifdef BOOT_HELPER_SUPPORT
  kBVFlagBooter           = 0x20,
#endif
  kBVFlagSystemVolume     = 0x40
};

enum {
    kBIOSDevTypeFloppy    = 0x00,
    kBIOSDevTypeHardDrive = 0x80,
    kBIOSDevTypeNetwork   = 0xE0,
    kBIOSDevUnitMask      = 0x0F,
    kBIOSDevTypeMask      = 0xF0,
    kBIOSDevMask          = 0xFF
};

enum {
	
	kPartitionTypeHFS	  = 0xAF,
	kPartitionTypeHPFS    = 0x07,
	kPartitionTypeFAT16   = 0x06,
	kPartitionTypeFAT32   = 0x0c,
	kPartitionTypeEXT3    = 0x83,
	kPartitionTypeFreeBSD = 0xa5,
	kPartitionTypeOpenBSD = 0xa6,
	kPartitionTypeBEFS	  = 0xEB
};

//#define BIOS_DEV_TYPE(d)  ((d) & kBIOSDevTypeMask)
#define BIOS_DEV_UNIT(bvr)  ((bvr)->biosdev - (bvr)->type)

#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

#define	round2(x, m)	(((x) + (m / 2)) & ~(m - 1))
#define roundup2(x, m)  (((x) + m - 1) & ~(m - 1))

#define MAKEKERNDEV(t, u, p)  MAKEBOOTDEV(t, 0, 0, u, p)

enum {
    kNetworkDeviceType = kBIOSDevTypeNetwork,
    kBlockDeviceType   = kBIOSDevTypeHardDrive
};

enum {
    kCursorTypeHidden    = 0x0100,
    kCursorTypeUnderline = 0x0607
};

#endif /* !__LIBSAIO_SAIO_TYPES_H */
