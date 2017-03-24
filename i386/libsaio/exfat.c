/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 1999-2004 Apple Computer, Inc.  All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * dmazar, 14/7/2011
 * support for EXFAT volume label reading
 * EXFAT info from: http://www.ntfs.com/exfat-overview.htm
 *
 * Zenith432, Nov 30 2014
 * support for reading files
 *
 * EXFAT shares partition type with NTFS (0x7) and easiest way of
 * adding it was through ntfs.c module. All functions here are called
 * from similar ntfs.c funcs as fallback (if not NTFS, maybe it's EXFAT).
 */

#include "config.h"
#include "libsaio.h"
#include "sl.h"

#pragma mark -
#pragma mark Preprocessor Definitions
#pragma mark -


#if DEBUG_EXFAT
	#define DBG(x...)		printf(x)
	#define PAUSE()			getchar()
#else
	#define DBG(x...)
	#define PAUSE()
#endif

#define	EXFAT_BBID	&gExfatID[0]
#define	EXFAT_BBIDLEN	8

#define MIN_BLOCK_SIZE_SHIFT		9
#define MAX_BLOCK_SIZE_SHIFT		12
#define MAX_BLOCK_SIZE			(1 << MAX_BLOCK_SIZE_SHIFT)
#define MAX_CLUSTER_SIZE_SHIFT		25
#define CLUST_FIRST			2		/* reserved cluster range */
#define CLUST_RSRVD			0xfffffff7	/* reserved cluster range */
#define INVALID_FAT_ADDRESS		0xffffffff
#define ATTR_DIRECTORY			0x10		/* entry is a directory name */
#define SEFLAG_ALLOCATION_POSSIBLE	1
#define SEFLAG_INVALID_FAT_CHAIN	2
#define SEFLAG_PSEUDO_ROOTDIR		0x80
#define COMPONENT_MAX_CHARS		255		/* Max # characters in path name single component */

#pragma mark -
#pragma mark Static Data
#pragma mark -

static CICell gCurrentIH = NULL;
static uint8_t gBPSShift = 0;			/* log_2(Bytes-Per-Sector) */
static uint8_t gSPCShift = 0;			/* log_2(Sectors-Per-Cluster) */
static uint32_t gFATOffset = 0;			/* in sectors */
static uint32_t gFATLength = 0;			/* in sectors */
static uint32_t gCLOffset = 0;			/* in sectors */
static uint32_t gCLCount = 0;			/* in clusters */
static uint32_t gRDCl = 0;			/* Root Directory Cluster Number */
static uint8_t* gFATCacheBuffer = NULL;
static uint32_t gCachedFATBlockAddress = 0;
static uint16_t* gUPCase = NULL;		/* If loaded, should be exactly 2^16 * sizeof(uint16_t) bytes long */
static uint8_t gBPCBShift = 0;			/* log_2(Bytes-Per-Cache-Block) */

static char const gExfatID[] = "EXFAT   ";

#pragma mark -
#pragma mark Helper Structures
#pragma mark -

struct exfat_dir_iterator
{
	uint64_t lsa;				/* Current sector address */
	uint64_t lsa_end;			/* Last sector address + 1 */
	uint8_t* buffer;
	uint32_t cluster;			/* Next cluster number */
	uint16_t residue;			/* Number of sectors in last cluster */
	uint16_t entry_offset;			/* Offset of next entry in buffer */
};

struct exfat_inode
{
	uint64_t valid_length;			/* File/Directory length */
	uint32_t first_cluster;			/* First cluster number */
	uint8_t attributes;			/* From direntry_file::attributes (truncated to 8 bits) */
	uint8_t stream_extension_flags;		/* From direntry_stream_extension::flags */
};

#pragma mark -
#pragma mark exFAT on-disk structures
#pragma mark -

/*
 * boot sector of the partition
 * http://www.ntfs.com/exfat-boot-sector.htm
 */
struct exfatbootfile {
	uint8_t		reserved1[3];		/* JumpBoot: 0xEB7690 */
	uint8_t		bf_sysid[8];		/* FileSystemName: 'EXFAT   ' */
	uint8_t		reserved2[53];		/* MustBeZero */
	uint64_t	bf_prtoff;		/* PartitionOffset: In sectors; if 0, shall be ignored */
	uint64_t	bf_vollen;		/* VolumeLength: Size of exFAT volume in sectors */
	uint32_t	bf_fatoff;		/* FatOffset: In sectors */
	uint32_t	bf_fatlen;		/* FatLength: In sectors. May exceed the required space in order to align the second FAT */
	uint32_t	bf_cloff;		/* ClusterHeapOffset: In sectors. */
	uint32_t	bf_clcnt;		/* ClusterCount: 2^32-11 is the maximum number of clusters could be described. */
	uint32_t	bf_rdircl;		/* RootDirectoryCluster. */
	uint32_t	bf_volsn;		/* VolumeSerialNumber. */
	uint16_t	bf_fsrev;		/* FileSystemRevision: as MAJOR.minor, major revision is high byte, minor is low byte; currently 01.00. */
	uint16_t	bf_volflags;		/* VolumeFlags. */
	uint8_t		bf_bpss;		/* BytesPerSectorShift: Power of 2. Minimum 9 (512 bytes per sector), maximum 12 (4096 bytes per sector) */
	uint8_t		bf_spcs;		/* SectorsPerClusterShift: Power of 2. Minimum 0 (1 sector per cluster), maximum 25 Ð BytesPerSectorShift, so max cluster size is 32 MB */
	uint8_t		bf_nfats;		/* NumberOfFats: 2 is for TexFAT only */
	uint8_t		bf_drvs;		/* DriveSelect: Extended INT 13h drive number; typically 0x80 */
	uint8_t		bf_pused;		/* PercentInUse: 0..100 Ð percentage of allocated clusters rounded down to the integer 0xFF Ð percentage is not available */
	uint8_t		reserved3[7];		/* Reserved */
	uint8_t		bootcode[390];		/* BootCode */
	uint16_t	bf_bsig;		/* BootSignature: 0xAA55 */
};

struct direntry_label {
#define DIRENTRY_TYPE_LABEL		((uint8_t) 0x83)
	uint8_t		type;			/* EntryType: 0x83 (or 0x03 if label is empty) */
	uint8_t		llen;			/* CharacterCount: Length in Unicode characters (max 11) */
#define VOLUME_LABEL_MAX_CHARS	11
	uint16_t	label[11];		/* VolumeLabel: Unicode characters (max 11) */
	uint8_t		reserved1[8];		/* Reserved */
};

#if UNUSED
struct direntry_allocation_bitmap
{
	uint8_t		type;			/* EntryType: 0x81 (or 0x01 if entry is empty) */
	uint8_t		bitmap_flags;		/* bit 0: 0 1st bitmap, 1 2nd bitmap */
	uint8_t		reserved1[18];		/* Reserved */
	uint32_t	first_cluster;		/* Cluster address of 1st data block */
	uint64_t	data_length;		/* Length of the data */
};

struct direntry_upcase_table
{
	uint8_t		type;			/* EntryType: 0x82 (or 0x02 if entry is empty) */
	uint8_t		reserved1[3];		/* Reserved */
	uint32_t	checksum;		/* Table Checksum */
	uint8_t		reserved2[12];		/* Reserved */
	uint32_t	first_cluster;		/* Cluster address of 1st data block */
	uint64_t	data_length;		/* Length of the data */
};
/*
 * Skipped:
 *  Volume GUID direntry 0xA0
 *  TexFAT Padding direntry 0xA1
 *  Windows CE Access Control Table 0xE2
 */
#endif /* UNUSED */

struct direntry_file
{
#define DIRENTRY_TYPE_FILE		((uint8_t) 0x85)
	uint8_t		type;			/* EntryType: 0x85 (or 0x05 if entry is empty) */
	uint8_t		count2;			/* Secondary Count */
	uint16_t	checksum;		/* Set Checksum */
	uint16_t	attributes;		/* File Attributes, 1 - R, 2 - H, 4 - S, 16 - D, 32 - A */
	uint8_t		reserved1[2];		/* Reserved */
	uint32_t	create_time;		/* Create Time, DOS Timestamp Format */
	uint32_t	mod_time;		/* Last Modified Time, DOS Timestamp Format */
	uint32_t	access_time;		/* Last Accessed Time, DOS Timestamp Format */
	uint8_t		create_10ms;		/* 10ms increments range 0 - 199 */
	uint8_t		mod_10ms;		/* 10ms increments range 0 - 199 */
	uint8_t		create_tzoff;		/* TZ Offset, difference to UTC in 15 min increments */
	uint8_t		mod_tzoff;		/* TZ Offset, difference to UTC in 15 min increments */
	uint8_t		access_tzoff;		/* TZ Offset, difference to UTC in 15 min increments */
	uint8_t		reserved2[7];		/* Reserved */
};

struct direntry_stream_extension
{
#define DIRENTRY_TYPE_ST_EX		((uint8_t) 0xC0)
	uint8_t		type;			/* EntryType: 0xC0 (or 0x40 if entry is empty) */
	uint8_t		flags;			/* bit 0 - Allocation Possible (1 Yes/0 No), bit 1 - No FAT Chain (1 Invalid/0 Valid) */
	uint8_t		reserved1;		/* Reserved */
	uint8_t		name_length;		/* Name Length */
	uint16_t	name_hash;		/* Name Hash */
	uint8_t		reserved2[2];		/* Reserved */
	uint64_t	valid_length;		/* Valid Data Length */
	uint8_t		reserved3[4];		/* Reserved */
	uint32_t	first_cluster;		/* Cluster address of 1st data block */
	uint64_t	data_length;		/* Length of the data */
};

struct direntry_name_extension
{
#define DIRENTRY_TYPE_NA_EX		((uint8_t) 0xC1)
	uint8_t		type;			/* EntryType: 0xC1 (or 0x41 if entry is empty) */
	uint8_t		reserved1;		/* Reserved */
#define LABEL_MAX_CHARS 15
	uint16_t	label[15];		/* 15 characters of file name (UTF16LE) */
};

#pragma mark -
#pragma mark FATCache
#pragma mark -

static
int FATCacheInit(int invalidate)
{
	if (!gFATCacheBuffer)
	{
		gFATCacheBuffer = (uint8_t*) malloc(MAX_BLOCK_SIZE);
		if (!gFATCacheBuffer)
		{
			return -1;
		}
		invalidate = 1;
	}
	if (invalidate)
	{
		gCachedFATBlockAddress = INVALID_FAT_ADDRESS;
	}
	return 0;
}

static inline
void FATCacheInvalidate(void)
{
	gCachedFATBlockAddress = INVALID_FAT_ADDRESS;
}

static inline
uint16_t CacheBlockSize(void)
{
	return (uint16_t) (1 << gBPCBShift);
}

static
int getRange(uint32_t cluster, uint32_t maxContiguousClusters, uint32_t* pNextCluster, uint32_t* pNumContiguousClusters)
{
	uint32_t count, lcba;
	uint16_t mask;
	uint8_t shift;

	if (!pNextCluster || !pNumContiguousClusters)
	{
		return -1;
	}
	count = 0;
	shift = gBPCBShift - 2;
	mask = (uint16_t) ((1 << shift) - 1);
	while (cluster >= CLUST_FIRST && cluster < CLUST_RSRVD && count < maxContiguousClusters)
	{
		++count;
		lcba = cluster >> shift;
		if (lcba != gCachedFATBlockAddress)
		{
			CacheRead(gCurrentIH,
					  (char*) gFATCacheBuffer,
					  (((long long) gFATOffset) << gBPSShift) + (((long long) lcba) << gBPCBShift),
					  CacheBlockSize(),
					  1);
			gCachedFATBlockAddress = lcba;
		}
		lcba = cluster + 1;
		cluster = OSSwapLittleToHostInt32(((uint32_t const*) gFATCacheBuffer)[cluster & mask]);
		if (cluster != lcba)
			break;
	}
	*pNextCluster = cluster;
	*pNumContiguousClusters = count;
	return 0;
}

#pragma mark -
#pragma mark Directory Iterator
#pragma mark -

static
void InitIteratorFromRoot(struct exfat_dir_iterator* pIter)
{
	pIter->lsa = 0;
	pIter->lsa_end = 0;
	pIter->cluster = gRDCl;
	pIter->residue = 0;
	pIter->entry_offset = CacheBlockSize();
}

static inline
uint64_t RoundUp(uint64_t val, uint8_t shift)
{
	return (val + (1 << shift) - 1) >> shift;		/* == RoundUpToInt(val/(2^shift)) */
}

static inline
uint16_t Residue(uint64_t val, uint8_t shift)
{
	return (-(int16_t) val) & (int16_t) ((1 << shift) - 1);	/* == (-val) mod (2^shift) */
}

static inline
uint64_t ClusterToLSA(uint32_t cluster)
{
	return (((uint64_t) (cluster - CLUST_FIRST)) << gSPCShift) + gCLOffset;
}

static
void InitIteratorFromInode(struct exfat_dir_iterator* pIter, struct exfat_inode const* pInode)
{
	if (pInode->stream_extension_flags & SEFLAG_ALLOCATION_POSSIBLE)
	{
		uint64_t sector_length = RoundUp(pInode->valid_length, gBPSShift);
		if (pInode->stream_extension_flags & SEFLAG_INVALID_FAT_CHAIN)
		{
			pIter->lsa = ClusterToLSA(pInode->first_cluster);
			pIter->lsa_end = pIter->lsa + sector_length;
			pIter->cluster = CLUST_RSRVD;
		}
		else
		{
			pIter->lsa = 0;
			pIter->lsa_end = 0;
			pIter->cluster = pInode->first_cluster;
			pIter->residue = Residue(sector_length, gSPCShift);
		}
	}
	else
	{
		pIter->lsa = 0;
		pIter->lsa_end = 0;
		pIter->cluster = CLUST_RSRVD;
	}
	pIter->entry_offset = CacheBlockSize();
}

static
uint8_t const* nextDirEntry(struct exfat_dir_iterator* pIter)
{
	uint8_t const* ret;
	uint16_t toRead;

	if (pIter->entry_offset >= CacheBlockSize())
	{
		if (pIter->lsa == pIter->lsa_end) {
			uint32_t next_cluster, contig;

			getRange(pIter->cluster, CLUST_RSRVD, &next_cluster, &contig);
			if (!contig)
			{
				return NULL;
			}
			pIter->lsa = ClusterToLSA(pIter->cluster);
			pIter->lsa_end = pIter->lsa + (((uint64_t) contig) << gSPCShift);
			if (next_cluster >= CLUST_RSRVD)
			{
				pIter->lsa_end -= pIter->residue;
			}
			pIter->cluster = next_cluster;
		}
		toRead = (uint16_t) (1 << (gBPCBShift - gBPSShift));
		if (pIter->lsa + toRead > pIter->lsa_end)
			toRead = (uint16_t) (pIter->lsa_end - pIter->lsa);
		CacheRead(gCurrentIH,
				  (char*) pIter->buffer,
				  (long long) (pIter->lsa << gBPSShift),
				  ((uint32_t) toRead) << gBPSShift,
				  1);
		pIter->lsa += toRead;
		pIter->entry_offset = 0;
	}
	ret = pIter->buffer + pIter->entry_offset;
	pIter->entry_offset += sizeof(struct direntry_file);
	return ret;
}

#pragma mark -
#pragma mark Path Search
#pragma mark -

static inline
int32_t ToUpper(uint16_t ch)
{
	if (gUPCase)
	{
		return gUPCase[ch];
	}
	if (ch >= 128)
	{
		return -1;
	}
	if ((uint8_t) ch >= 'a' && (uint8_t) ch <= 'z')
	{
		ch &= ~0x20;
	}
	return ch;
}

static
int32_t NameHash(uint16_t const* component_utf16le, uint16_t numChars)
{
	int32_t ch;
	uint16_t hash = 0;

	for (; numChars; ++component_utf16le, --numChars)
	{
		ch = ToUpper(OSSwapLittleToHostInt16(*component_utf16le));
		if (ch < 0)
		{
			return ch;
		}
		hash = (hash << 15) | (hash >> 1) + (uint8_t) ch;
		hash = (hash << 15) | (hash >> 1) + (uint8_t) (ch >> 8);
	}
	return hash;
}

static
int ComponentToInode(struct exfat_dir_iterator* iterator, uint16_t const* component_utf16le, uint16_t numChars, struct exfat_inode* out_file)
{
	union {
		uint8_t const* nde;
		struct direntry_file const* fe;
		struct direntry_stream_extension const* fse;
		struct direntry_name_extension const* ne;
	} u;
	int32_t computed_hash;
	uint8_t count2, name_length;

	computed_hash = NameHash(component_utf16le, numChars);
	while ((u.nde = nextDirEntry(iterator)))
	{
		if (!*u.nde)
		{
			break;
		}
	redo:
		if (*u.nde != DIRENTRY_TYPE_FILE)
			continue;
		count2 = u.fe->count2;
		if (count2 < 2)
		{
			continue;
		}
		out_file->attributes = (uint8_t) OSSwapLittleToHostInt16(u.fe->attributes);
		u.nde = nextDirEntry(iterator);
		if (!u.nde || !*u.nde)
			break;
		if (*u.nde != DIRENTRY_TYPE_ST_EX)
			goto redo;
		out_file->stream_extension_flags = u.fse->flags;
		name_length = u.fse->name_length;
		if (name_length != numChars)
			continue;
		if (computed_hash >= 0 && computed_hash != OSSwapLittleToHostInt16(u.fse->name_hash))
			continue;
		out_file->valid_length = OSSwapLittleToHostInt64(u.fse->valid_length);
		out_file->first_cluster = OSSwapLittleToHostInt32(u.fse->first_cluster);
		for (--count2; count2 && name_length; --count2) {
			int32_t ch1, ch2;
			uint16_t const* q;
			uint8_t t;

			u.nde = nextDirEntry(iterator);
			if (!u.nde || !*u.nde)
				goto outta_bad;
			if (*u.nde != DIRENTRY_TYPE_NA_EX)
				goto redo;
			t = name_length > LABEL_MAX_CHARS ? LABEL_MAX_CHARS : name_length;
			q = &u.ne->label[0];
			for (; t; ++q, ++component_utf16le, --t, --name_length) {
				ch1 = ToUpper(OSSwapLittleToHostInt16(*component_utf16le));
				ch2 = ToUpper(OSSwapLittleToHostInt16(*q));
				if (ch1 != ch2)
					goto abort_comparison;
			}
		}
		return 0;
	abort_comparison:;
	}
outta_bad:
	return -1;
}

static
int ExtractDirEntry(struct exfat_dir_iterator* iterator, char** name, long* flags, u_int32_t* time, long* infoValid)
{
	union {
		uint8_t const* nde;
		struct direntry_file const* fe;
		struct direntry_stream_extension const* fse;
		struct direntry_name_extension const* ne;
	} u;
	uint8_t count2, name_length, t;
	uint16_t component_full[COMPONENT_MAX_CHARS], *cp_ptr;

	while ((u.nde = nextDirEntry(iterator))) {
		if (!*u.nde)
			break;
	redo:
		if (*u.nde != DIRENTRY_TYPE_FILE)
			continue;
		count2 = u.fe->count2;
		if (count2 < 2)
			continue;
		if (flags)
			*flags = (OSSwapLittleToHostInt16(u.fe->attributes) & ATTR_DIRECTORY) ? kFileTypeDirectory : kFileTypeFlat;
		if (time)
			*time = OSSwapLittleToHostInt32(u.fe->mod_time);
		if (!name)
			goto info_valid;
		u.nde = nextDirEntry(iterator);
		if (!u.nde || !*u.nde)
			break;
		if (*u.nde != DIRENTRY_TYPE_ST_EX)
			goto redo;
		name_length = u.fse->name_length;
		cp_ptr = &component_full[0];
		for (--count2; count2 && name_length; --count2) {
			u.nde = nextDirEntry(iterator);
			if (!u.nde || !*u.nde)
				goto outta_bad;
			if (*u.nde != DIRENTRY_TYPE_NA_EX)
				goto redo;
			t = name_length > LABEL_MAX_CHARS ? LABEL_MAX_CHARS : name_length;
			memcpy(cp_ptr, &u.ne->label[0], t * sizeof(uint16_t));
			cp_ptr += t;
			name_length -= t;
		}
		/*
		 * Note: for ASCII can allocate exactly name_length + 1,
		 *   but in case of multibyte characters, allow more space.
		 */
		*name = (char*) malloc(COMPONENT_MAX_CHARS + 1);
		if (*name)
			utf_encodestr(&component_full[0], cp_ptr - &component_full[0], (uint8_t*) *name, COMPONENT_MAX_CHARS, OSLittleEndian);
	info_valid:
		if (infoValid)
			*infoValid = 1;
		return 0;
	}
outta_bad:
	return -1;
}

static
int PathToInode(char const* path, struct exfat_inode* out_file, uint8_t* buffer /* size CacheBlockSize() bytes */)
{
	struct exfat_dir_iterator iterator;
	uint8_t *ptr, *slash, ch;
	uint16_t path_utf16le[COMPONENT_MAX_CHARS];
	uint16_t numChars;
	char have_prev_inode;

	InitIteratorFromRoot(&iterator);
	iterator.buffer = buffer;
	ptr = (uint8_t*) path;	/* Note: const_cast */
	have_prev_inode = 0;
	do {
		do {
			for (slash = ptr; *slash && *slash != '/'; ++slash);
			ch = *slash;
			if (slash == ptr) {
				if (!ch) {
					if (!have_prev_inode) {
						/*
						 * Fill in pseudo-inode for Root Directory
						 */
						out_file->valid_length = 0;	/* Unknown */
						out_file->first_cluster = gRDCl;
						out_file->attributes = ATTR_DIRECTORY;
						out_file->stream_extension_flags = SEFLAG_ALLOCATION_POSSIBLE | SEFLAG_PSEUDO_ROOTDIR;
					}
					return 0;
				}
				++ptr;
				continue;
			}
			break;
		} while (1);
		*slash = 0;
		utf_decodestr(ptr, &path_utf16le[0], &numChars, sizeof path_utf16le, OSLittleEndian);
		numChars = OSSwapLittleToHostInt16(numChars);
		*slash = ch;
		ptr = slash + 1;
		if (have_prev_inode)
			InitIteratorFromInode(&iterator, out_file);
		if (ComponentToInode(&iterator, &path_utf16le[0], numChars, out_file) < 0)
			break;
		if (!ch)	/* was last component - done */
			return 0;
		if (!(out_file->attributes & ATTR_DIRECTORY))	/* not a directory and not last component - error */
			return -1;
		have_prev_inode = 1;
	} while (1);
	return -1;
}

#pragma mark -
#pragma mark exFAT Implementation
#pragma mark -

void
EXFATFree(CICell ih)
{
	if (gCurrentIH == ih) {
		gCurrentIH = NULL;
		FATCacheInvalidate();
	}
	free(ih);
}

long
EXFATInitPartition(CICell ih)
{
	uint8_t *buffer, bpss, spcs;
	struct exfatbootfile const* boot;

	if (!ih)
		return -1;
	if (gCurrentIH == ih)
	{
		CacheInit(ih, CacheBlockSize());
		return FATCacheInit(0);
	}

	buffer = (uint8_t*) malloc(BPS);
	if (!buffer)
		return -1;

	/*
	 * Read the boot sector of the filesystem, and then check the
	 * boot signature.  If not a boot sector then error out.
	 */

	Seek(ih, 0);
	Read(ih, (long) buffer, BPS);

	boot = (struct exfatbootfile const*) buffer;

	/* Looking for EXFAT signature. */
	if (memcmp((char const*) &boot->bf_sysid[0], EXFAT_BBID, EXFAT_BBIDLEN)) {
		free(buffer);
		return -1;
	}

	/*
	 * Make sure the bytes per sector and sectors per cluster are within reasonable ranges.
	 */
	bpss = boot->bf_bpss;
	if (bpss < MIN_BLOCK_SIZE_SHIFT || bpss > MAX_BLOCK_SIZE_SHIFT) {
		free(buffer);
		return -1;
	}

	spcs = boot->bf_spcs;
	if (spcs > (MAX_CLUSTER_SIZE_SHIFT - bpss)) {
		free(buffer);
		return -1;
	}

	if (FATCacheInit(1) < 0)
	{
		free(buffer);
		return -1;
	}

	gBPSShift = bpss;
	gSPCShift = spcs;
	gFATOffset = OSSwapLittleToHostInt32(boot->bf_fatoff);
	gFATLength = OSSwapLittleToHostInt32(boot->bf_fatlen);
	gCLOffset = OSSwapLittleToHostInt32(boot->bf_cloff);
	gCLCount = OSSwapLittleToHostInt32(boot->bf_clcnt);
	gRDCl = OSSwapLittleToHostInt32(boot->bf_rdircl);
	gBPCBShift = bpss + spcs;
	if (gBPCBShift > MAX_BLOCK_SIZE_SHIFT)
		gBPCBShift = MAX_BLOCK_SIZE_SHIFT;

	gCurrentIH = ih;

	CacheInit(ih, CacheBlockSize());

	free(buffer);
	return 0;
}

long
EXFATGetDirEntry(CICell ih, char * dirPath, long long * dirIndex,
				 char ** name, long * flags, u_int32_t * time,
				 FinderInfo * finderInfo, long * infoValid)
{
	struct exfat_dir_iterator* iterator;

	if (!dirPath || !dirIndex)
		return -1;

	if (EXFATInitPartition(ih) < 0)
		return -1;

	if (*dirPath == '/')
		++dirPath;

	iterator = (struct exfat_dir_iterator*) (long) *dirIndex;
	if (!iterator) {
		struct exfat_inode inode;
		uint8_t* buffer;

		buffer = (uint8_t*) malloc(CacheBlockSize() + sizeof *iterator);
		if (!buffer)
			return -1;
		iterator = (struct exfat_dir_iterator*) (buffer + CacheBlockSize());
		if (PathToInode(dirPath, &inode, buffer) < 0 ||
			!(inode.attributes & ATTR_DIRECTORY)) {
			free(buffer);
			return -1;
		}
		if (inode.stream_extension_flags & SEFLAG_PSEUDO_ROOTDIR)
			InitIteratorFromRoot(iterator);
		else
			InitIteratorFromInode(iterator, &inode);
		iterator->buffer = buffer;
		*dirIndex = (long long) (long) iterator;
	}
	if (ExtractDirEntry(iterator, name, flags, time, infoValid) < 0) {
		free(iterator->buffer);
		*dirIndex = 0;
		return -1;
	}
	return 0;
}

long
EXFATReadFile(CICell ih, char * filePath, void *base, uint64_t offset, uint64_t length)
{
	uint64_t size, toRead, leftToRead;
	struct exfat_inode inode;
	uint8_t* buffer;
	uint32_t cluster;

	if (EXFATInitPartition(ih) < 0)
		return -1;

	if (*filePath == '/')
		++filePath;

	buffer = (uint8_t*) malloc(CacheBlockSize());
	if (!buffer)
		return -1;

	if (PathToInode(filePath, &inode, buffer) < 0 ||
		(inode.attributes & ATTR_DIRECTORY) != 0) {
		free(buffer);
		return -1;
	}
	free(buffer);
	if (!(inode.stream_extension_flags & SEFLAG_ALLOCATION_POSSIBLE))
		return (!offset && !length) ? 0 : -1;
	cluster = inode.first_cluster;
	size = inode.valid_length;
	if (size == offset && !length)
		return 0;
	if (size <= offset)
		return -1;
	toRead = size - offset;
	if (length && length < toRead)
		toRead = length;
	if (inode.stream_extension_flags & SEFLAG_INVALID_FAT_CHAIN) {
		Seek(ih, (long long) ((ClusterToLSA(cluster) << gBPSShift) + offset));
		Read(ih, (long) base, (long) toRead);
		return (long) toRead;
	}
	leftToRead = toRead;
	do {
		uint64_t chunk, canRead;
		uint32_t next_cluster, contig;

		getRange(cluster, CLUST_RSRVD, &next_cluster, &contig);
		if (!contig)
			break;
		chunk = ((uint64_t) contig) << (gBPSShift + gSPCShift);
		if (offset >= chunk) {
			offset -= chunk;
			cluster = next_cluster;
			continue;
		}
		canRead = chunk - offset;
		if (canRead > leftToRead)
			canRead = leftToRead;
		Seek(ih, (long long) ((ClusterToLSA(cluster) << gBPSShift) + offset));
		Read(ih, (long) base, (long) canRead);
		base = ((uint8_t*) base) + canRead;
		cluster = next_cluster;
		offset = 0;
		leftToRead -= canRead;
	} while (leftToRead);
	return (long) (toRead - leftToRead);
}

long
EXFATGetFileBlock(CICell ih, char *filePath, unsigned long long *firstBlock)
{
	uint8_t* buffer;
	struct exfat_inode inode;
	uint32_t cluster;

	if (EXFATInitPartition(ih) < 0)
		return -1;

	if (*filePath == '/')
		++filePath;

	buffer = (uint8_t*) malloc(CacheBlockSize());
	if (!buffer)
		return -1;

	if (PathToInode(filePath, &inode, buffer) < 0 ||
		(inode.attributes & ATTR_DIRECTORY) != 0 ||
		!(inode.stream_extension_flags & SEFLAG_ALLOCATION_POSSIBLE)) {
		free(buffer);
		return -1;
	}
	free(buffer);
	cluster = inode.first_cluster;
	if (cluster < CLUST_FIRST || cluster >= CLUST_RSRVD)
		return -1;
	*firstBlock = ClusterToLSA(cluster);
	return 0;
}

long
EXFATLoadFile(CICell ih, char * filePath)
{
	return EXFATReadFile(ih, filePath, (void *)gFSLoadAddress, 0, 0);
}

/**
 * Reads volume label into str.
 * Reads boot sector, performs some checking, loads root dir
 * and parses volume label.
 */
void
EXFATGetDescription(CICell ih, char *str, long strMaxLen)
{
    struct exfatbootfile *boot;
    uint8_t bpss, spcs;
    long long  rdirOffset = 0;
    char *buf = NULL;
    struct direntry_label *dire = NULL;
    int loopControl = 0;

    DBG("EXFAT: start %x:%x\n", ih->biosdev, ih->part_no);

    buf = (char *)malloc(MAX_BLOCK_SIZE);
    if (buf == 0)
    {
        goto error;
    }

    /*
     * Read the boot sector, check signatures, and do some minimal
     * sanity checking.  NOTE: the size of the read below is intended
     * to be a multiple of all supported block sizes, so we don't
     * have to determine or change the device's block size.
     */
    Seek(ih, 0);
    Read(ih, (long)buf, MAX_BLOCK_SIZE);

    // take our boot structure
    boot = (struct exfatbootfile *) buf;

    /*
     * The first three bytes are an Intel x86 jump instruction.  I assume it
     * can be the same forms as DOS FAT:
     *    0xE9 0x?? 0x??
     *    0xEC 0x?? 0x90
     * where 0x?? means any byte value is OK.
     */
    if (boot->reserved1[0] != 0xE9
        && (boot->reserved1[0] != 0xEB || boot->reserved1[2] != 0x90))
    {
        goto error;
    }

     // Check the "EXFAT   " signature.
    if (memcmp((const char *)boot->bf_sysid, EXFAT_BBID, EXFAT_BBIDLEN) != 0)
    {
        goto error;
    }

    /*
     * Make sure the bytes per sector and sectors per cluster are
     * powers of two, and within reasonable ranges.
     */
    bpss = boot->bf_bpss;	/* Just one byte; no swapping needed */
    DBG("EXFAT: bpss=%d, bytesPerSector=%d\n", bpss, (1 << bpss));
    if (bpss < MIN_BLOCK_SIZE_SHIFT || bpss > MAX_BLOCK_SIZE_SHIFT)
    {
        DBG("EXFAT: invalid bytes per sector shift(%d)\n", bpss);
        goto error;
    }

    spcs = boot->bf_spcs;	/* Just one byte; no swapping needed */
    DBG("EXFAT: spcs=%d, sectorsPerCluster=%d\n", spcs, (1 << spcs));
    if (spcs > (MAX_CLUSTER_SIZE_SHIFT - bpss))
    {
        DBG("EXFAT: invalid sectors per cluster shift (%d)\n", spcs);
        goto error;
    }

     // calculate root dir cluster offset
    rdirOffset = OSSwapLittleToHostInt32(boot->bf_cloff) + ((long long) (OSSwapLittleToHostInt32(boot->bf_rdircl) - 2) << spcs);
    DBG("EXFAT: rdirOffset=%d\n", (int) rdirOffset);

    // load MAX_BLOCK_SIZE bytes of root dir
    Seek(ih, rdirOffset << bpss);
    Read(ih, (long)buf, MAX_BLOCK_SIZE);
    DBG("buf 0 1 2 = %x %x %x\n", 0x00ff & buf[0], 0x00ff & buf[1], 0x00ff & buf[2]);

    str[0] = '\0';

    /*
     * Search for volume label dir entry (type 0x83), convert from unicode and put to str.
     * Set loopControl var to avoid searching outside of buf.
     */
    loopControl = MAX_BLOCK_SIZE / sizeof(struct direntry_label);
    dire = (struct direntry_label *)buf;
    while (loopControl && dire->type && dire->type != DIRENTRY_TYPE_LABEL)
    {
        dire++;
        loopControl--;
    }
    if (dire->type == DIRENTRY_TYPE_LABEL && dire->llen > 0 && dire->llen <= VOLUME_LABEL_MAX_CHARS)
    {
        utf_encodestr( dire->label, (int)dire->llen, (u_int8_t *)str, strMaxLen, OSLittleEndian );
    }
    DBG("EXFAT: label=%s\n", str);

    free(buf);
    PAUSE();
    return;

 error:
	if (buf)
	{
		free(buf);
	}

	DBG("EXFAT: error\n");
	PAUSE();
	return;
}

/**
 * Sets UUID to uuidStr.
 * Reads the boot sector, does some checking, generates UUID
 * (like the one you get on Windows???)
 */
long EXFATGetUUID(CICell ih, char *uuidStr)
{
	uint32_t volsn;
	struct exfatbootfile *boot;
	void *buf = malloc(MAX_BLOCK_SIZE);
	if ( !buf )
		return -1;

	/*
	 * Read the boot sector, check signatures, and do some minimal
	 * sanity checking.	 NOTE: the size of the read below is intended
	 * to be a multiple of all supported block sizes, so we don't
	 * have to determine or change the device's block size.
	 */
	Seek(ih, 0);
	Read(ih, (long)buf, MAX_BLOCK_SIZE);

	boot = (struct exfatbootfile *) buf;

	/*
	 * Check the "EXFAT   " signature.
	 */
	if (memcmp((const char *)boot->bf_sysid, EXFAT_BBID, EXFAT_BBIDLEN) != 0)
	{
		return -1;
	}

	// Check for non-null volume serial number
	volsn = OSSwapLittleToHostInt32(boot->bf_volsn);
	if( !volsn )
	{
		return -1;
	}

	// Use UUID like the one you get on Windows
	sprintf(uuidStr, "%04X-%04X",   (unsigned short)(volsn >> 16) & 0xFFFF,
                                    (unsigned short)volsn & 0xFFFF);

	DBG("EXFATGetUUID: %x:%x = %s\n", ih->biosdev, ih->part_no, uuidStr);
	return 0;
}

/**
 * Returns true if given buffer is the boot rec of the EXFAT volume.
 */
bool EXFATProbe(const void * buffer)
{
	bool result = false;

	// boot sector structure
	const struct exfatbootfile	* boot = buffer;

	// Looking for EXFAT signature.
	if (memcmp((const char *)boot->bf_sysid, EXFAT_BBID, EXFAT_BBIDLEN) == 0)
	{
		result = true;
	}

	DBG("EXFATProbe: %d\n", result ? 1 : 0);
	return result;
}
