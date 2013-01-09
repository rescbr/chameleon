/*-
 * Copyright (C) 2008 Jason Evans <jasone@FreeBSD.org>.
 * Copyright (c) 2008-2012 Hewlett-Packard Development Company, L.P.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice(s), this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified other than the possible
 *    addition of one or more copyright notices.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice(s), this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 *
 * This allocator implementation is designed to provide scalable performance
 * for multi-threaded programs on multi-processor systems.  The following
 * features are included for this purpose:
 *
 *   + Multiple arenas are used if there are multiple CPUs, which reduces lock
 *     contention and cache sloshing.
 *
 *   + Thread-specific caching is used if there are multiple threads, which
 *     reduces the amount of locking.
 *
 *   + Cache line sharing between arenas is avoided for internal data
 *     structures.
 *
 *   + Memory is managed in chunks and runs (chunks can be split into runs),
 *     rather than as individual pages.  This provides a constant-time
 *     mechanism for associating allocations with particular arenas.
 *
 * Allocation requests are rounded up to the nearest size class, and no record
 * of the original request size is maintained.  Allocations are broken into
 * categories according to size class.  Assuming runtime defaults, 4 kB pages
 * and a 16 byte quantum on a 32-bit system, the size classes in each category
 * are as follows:
 *
 *   |=======================================|
 *   | Category | Subcategory      |    Size |
 *   |=======================================|
 *   | Small    | Tiny             |       2 |
 *   |          |                  |       4 |
 *   |          |                  |       8 |
 *   |          |------------------+---------|
 *   |          | Quantum-spaced   |      16 |
 *   |          |                  |      32 |
 *   |          |                  |      48 |
 *   |          |                  |     ... |
 *   |          |                  |      96 |
 *   |          |                  |     112 |
 *   |          |                  |     128 |
 *   |          |------------------+---------|
 *   |          | Cacheline-spaced |     192 |
 *   |          |                  |     256 |
 *   |          |                  |     320 |
 *   |          |                  |     384 |
 *   |          |                  |     448 |
 *   |          |                  |     512 |
 *   |          |------------------+---------|
 *   |          | Sub-page         |     760 |
 *   |          |                  |    1024 |
 *   |          |                  |    1280 |
 *   |          |                  |     ... |
 *   |          |                  |    3328 |
 *   |          |                  |    3584 |
 *   |          |                  |    3840 |
 *   |=======================================|
 *   | Large                       |    4 kB |
 *   |                             |    8 kB |
 *   |                             |   12 kB |
 *   |                             |     ... |
 *   |                             | 1012 kB |
 *   |                             | 1016 kB |
 *   |                             | 1020 kB |
 *   |=======================================|
 *   | Huge                        |    1 MB |
 *   |                             |    2 MB |
 *   |                             |    3 MB |
 *   |                             |     ... |
 *   |=======================================|
 *
 * A different mechanism is used for each category:
 *
 *   Small : Each size class is segregated into its own set of runs.  Each run
 *           maintains a bitmap of which regions are free/allocated.
 *
 *   Large : Each allocation is backed by a dedicated run.  Metadata are stored
 *           in the associated arena chunk header maps.
 *
 *   Huge : Each allocation is backed by a dedicated contiguous set of chunks.
 *          Metadata are stored in a separate red-black tree.
 *
 *******************************************************************************
 */


/*
 * MALLOC_PRODUCTION disables assertions and statistics gathering.  It also
 * defaults the A and J runtime options to off.  These settings are appropriate
 * for production systems.
 */
/* #define	MALLOC_PRODUCTION */

#ifndef MALLOC_PRODUCTION
/*
 * MALLOC_DEBUG enables assertions and other sanity checks, and disables
 * inline functions.
 */
#  define MALLOC_DEBUG

/* MALLOC_STATS enables statistics calculation. */
#  define MALLOC_STATS
#endif

/*
 * MALLOC_TINY enables support for tiny objects, which are smaller than one
 * quantum.
 */
#define	MALLOC_TINY

/*
 * MALLOC_DSS enables use of sbrk(2) to allocate chunks from the data storage
 * segment (DSS).  In an ideal world, this functionality would be completely
 * unnecessary, but we are burdened by history and the lack of resource limits
 * for anonymous mapped memory.
 */
#define	MALLOC_DSS // The default and the only option availaible (no mmap on this environemment)

#define PATH_MAX    1024   /* max bytes in pathname */
#define	issetugid() 0
#define	__DECONST(type, var)	((type)(uintptr_t)(const void *)(var))
#define	EINVAL		22		/* Invalid argument */
#define	ENOMEM		12		/* Cannot allocate memory */

/* __FBSDID("$FreeBSD: head/lib/libc/stdlib/malloc.c 182225 2008-08-27 02:00:53Z jasone $"); */

#include "libsa.h"
#include "memory.h"
#define abort() iloop()

#include <limits.h>
#ifndef SIZE_T_MAX
#  define SIZE_T_MAX	SIZE_MAX
#endif
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include "rb.h"

#ifdef MALLOC_DEBUG
/* Disable inlining to make debugging easier. */
#  define inline
#endif

/*
 * The const_size2bin table is sized according to PAGESIZE_2POW, but for
 * correctness reasons, we never assume that
 * (pagesize == (1U << * PAGESIZE_2POW)).
 *
 * Minimum alignment of allocations is 2^QUANTUM_2POW bytes.
 */
#ifdef __i386__
#  define PAGESIZE_2POW		12
#  define QUANTUM_2POW		4
#  define SIZEOF_PTR_2POW	2
#endif

#define	QUANTUM			((size_t)(1U << QUANTUM_2POW))
#define	QUANTUM_MASK		(QUANTUM - 1)

#define	SIZEOF_PTR		(1U << SIZEOF_PTR_2POW)

/* sizeof(int) == (1U << SIZEOF_INT_2POW). */
#ifndef SIZEOF_INT_2POW
#  define SIZEOF_INT_2POW	2
#endif

/*
 * Size and alignment of memory chunks that are allocated by the OS's virtual
 * memory system.
 */
#define	CHUNK_2POW_DEFAULT	20

/* Maximum number of dirty pages per arena. */
#define	DIRTY_MAX_DEFAULT	(1U << 9)

/*
 * Maximum size of L1 cache line.  This is used to avoid cache line aliasing.
 * In addition, this controls the spacing of cacheline-spaced size classes.
 */
#define	CACHELINE_2POW		6
#define	CACHELINE		((size_t)(1U << CACHELINE_2POW))
#define	CACHELINE_MASK		(CACHELINE - 1)

/*
 * Subpages are an artificially designated partitioning of pages.  Their only
 * purpose is to support subpage-spaced size classes.
 *
 * There must be at least 4 subpages per page, due to the way size classes are
 * handled.
 */
#define	SUBPAGE_2POW		8
#define	SUBPAGE			((size_t)(1U << SUBPAGE_2POW))
#define	SUBPAGE_MASK		(SUBPAGE - 1)

#ifdef MALLOC_TINY
/* Smallest size class to support. */
#  define TINY_MIN_2POW		1
#endif

/*
 * Maximum size class that is a multiple of the quantum, but not (necessarily)
 * a power of 2.  Above this size, allocations are rounded up to the nearest
 * power of 2.
 */
#define	QSPACE_MAX_2POW_DEFAULT	7

/*
 * Maximum size class that is a multiple of the cacheline, but not (necessarily)
 * a power of 2.  Above this size, allocations are rounded up to the nearest
 * power of 2.
 */
#define	CSPACE_MAX_2POW_DEFAULT	9

/*
 * RUN_MAX_OVRHD indicates maximum desired run header overhead.  Runs are sized
 * as small as possible such that this setting is still honored, without
 * violating other constraints.  The goal is to make runs as small as possible
 * without exceeding a per run external fragmentation threshold.
 *
 * We use binary fixed point math for overhead computations, where the binary
 * point is implicitly RUN_BFP bits to the left.
 *
 * Note that it is possible to set RUN_MAX_OVRHD low enough that it cannot be
 * honored for some/all object sizes, since there is one bit of header overhead
 * per object (plus a constant).  This constraint is relaxed (ignored) for runs
 * that are so small that the per-region overhead is greater than:
 *
 *   (RUN_MAX_OVRHD / (reg_size << (3+RUN_BFP))
 */
#define	RUN_BFP			12
/*                                    \/   Implicit binary fixed point. */
#define	RUN_MAX_OVRHD		0x0000003dU
#define	RUN_MAX_OVRHD_RELAX	0x00001800U

/* Put a cap on small object run size.  This overrides RUN_MAX_OVRHD. */
#define	RUN_MAX_SMALL	(12 * pagesize)

#if (defined(__clang__)) || (defined(__llvm__)) || (defined(__APPLE__))
#if !defined(HEAP_TRACKING)
#define je_malloc            malloc
#define je_free              free
#define je_realloc           realloc
#define je_memalign          memalign
#define je_valloc            valloc
#define je_calloc            calloc
#define je_posix_memalign    posix_memalign
#endif
#else
#if !defined(HEAP_TRACKING)
void* malloc(size_t size) __attribute__ ((weak, alias ("je_malloc")));
void  free(void* ptr) __attribute__ ((weak, alias ("je_free")));
void* realloc(void* ptr, size_t size) __attribute__ ((weak, alias ("je_realloc")));
void* memalign(size_t boundary, size_t size) __attribute__ ((weak, alias ("je_memalign")));
void* valloc(size_t size) __attribute__ ((weak, alias ("je_valloc")));
void* calloc(size_t num, size_t size) __attribute__ ((weak, alias("je_calloc")));
int   posix_memalign(void **memptr, size_t alignment, size_t size) __attribute__ ((weak, alias("je_posix_memalign")));
#endif
#endif

/******************************************************************************/

/* Set to true once the allocator has been initialized. */
static bool malloc_initialized = false;

/******************************************************************************/
/*
 * Statistics data structures.
 */

#ifdef MALLOC_STATS

typedef struct malloc_bin_stats_s malloc_bin_stats_t;
struct malloc_bin_stats_s {
	/*
	 * Number of allocation requests that corresponded to the size of this
	 * bin.
	 */
	uint64_t	nrequests;
    
    
	/* Total number of runs created for this bin's size class. */
	uint64_t	nruns;
    
	/*
	 * Total number of runs reused by extracting them from the runs tree for
	 * this bin's size class.
	 */
	uint64_t	reruns;
    
	/* High-water mark for this bin. */
	unsigned long	highruns;
    
	/* Current number of runs in this bin. */
	unsigned long	curruns;
};

typedef struct arena_stats_s arena_stats_t;
struct arena_stats_s {
	/* Number of bytes currently mapped. */
	size_t		mapped;
    
	/*
	 * Total number of purge sweeps, total number of madvise calls made,
	 * and total pages purged in order to keep dirty unused memory under
	 * control.
	 */
	uint64_t	npurge;
	uint64_t	nmadvise;
	uint64_t	purged;
    
	/* Per-size-category statistics. */
	size_t		allocated_small;
	uint64_t	nmalloc_small;
	uint64_t	ndalloc_small;
    
	size_t		allocated_large;
	uint64_t	nmalloc_large;
	uint64_t	ndalloc_large;
	
};

typedef struct chunk_stats_s chunk_stats_t;
struct chunk_stats_s {
	/* Number of chunks that were allocated. */
	uint64_t	nchunks;
    
	/* High-water mark for number of chunks allocated. */
	unsigned long	highchunks;
    
	/*
	 * Current number of chunks allocated.  This value isn't maintained for
	 * any other purpose, so keep track of it in order to be able to set
	 * highchunks.
	 */
	unsigned long	curchunks;
};

#endif /* #ifdef MALLOC_STATS */

/******************************************************************************/
/*
 * Extent data structures.
 */

/* Tree of extents. */
typedef struct extent_node_s extent_node_t;
struct extent_node_s {
	/* Linkage for the size/address-ordered tree. */
	rb_node(extent_node_t) link_szad;
    
	/* Linkage for the address-ordered tree. */
	rb_node(extent_node_t) link_ad;
    
	/* Pointer to the extent that this tree node is responsible for. */
	void	*addr;
    
	/* Total region size. */
	size_t	size;
};
typedef rb_tree(extent_node_t) extent_tree_t;

/******************************************************************************/
/*
 * Arena data structures.
 */

typedef struct arena_s arena_t;
typedef struct arena_bin_s arena_bin_t;

/* Each element of the chunk map corresponds to one page within the chunk. */
typedef struct arena_chunk_map_s arena_chunk_map_t;
struct arena_chunk_map_s {
	/*
	 * Linkage for run trees.  There are two disjoint uses:
	 *
	 * 1) arena_t's runs_avail tree.
	 * 2) arena_run_t conceptually uses this linkage for in-use non-full
	 *    runs, rather than directly embedding linkage.
	 */
	rb_node(arena_chunk_map_t)	link;
    
	/*
	 * Run address (or size) and various flags are stored together.  The bit
	 * layout looks like (assuming 32-bit system):
	 *
	 *   ???????? ???????? ????---- ---kdzla
	 *
	 * ? : Unallocated: Run address for first/last pages, unset for internal
	 *                  pages.
	 *     Small: Run address.
	 *     Large: Run size for first page, unset for trailing pages.
	 * - : Unused.
	 * k : key?
	 * d : dirty?
	 * z : zeroed?
	 * l : large?
	 * a : allocated?
	 *
	 * Following are example bit patterns for the three types of runs.
	 *
	 * r : run address
	 * s : run size
	 * x : don't care
	 * - : 0
	 * [dzla] : bit set
	 *
	 *   Unallocated:
	 *     ssssssss ssssssss ssss---- --------
	 *     xxxxxxxx xxxxxxxx xxxx---- ----d---
	 *     ssssssss ssssssss ssss---- -----z--
	 *
	 *   Small:
	 *     rrrrrrrr rrrrrrrr rrrr---- -------a
	 *     rrrrrrrr rrrrrrrr rrrr---- -------a
	 *     rrrrrrrr rrrrrrrr rrrr---- -------a
	 *
	 *   Large:
	 *     ssssssss ssssssss ssss---- ------la
	 *     -------- -------- -------- ------la
	 *     -------- -------- -------- ------la
	 */
	size_t				bits;
#define	CHUNK_MAP_KEY		((size_t)0x10U)
#define	CHUNK_MAP_DIRTY		((size_t)0x08U)
#define	CHUNK_MAP_ZEROED	((size_t)0x04U)
#define	CHUNK_MAP_LARGE		((size_t)0x02U)
#define	CHUNK_MAP_ALLOCATED	((size_t)0x01U)
};
typedef rb_tree(arena_chunk_map_t) arena_avail_tree_t;
typedef rb_tree(arena_chunk_map_t) arena_run_tree_t;

/* Arena chunk header. */
typedef struct arena_chunk_s arena_chunk_t;
struct arena_chunk_s {
	/* Arena that owns the chunk. */
	arena_t		*arena;
    
	/* Linkage for the arena's chunks_dirty tree. */
	rb_node(arena_chunk_t) link_dirty;
    
	/* Number of dirty pages. */
	size_t		ndirty;
    
	/* Map of pages within chunk that keeps track of free/large/small. */
	arena_chunk_map_t map[1]; /* Dynamically sized. */
};
typedef rb_tree(arena_chunk_t) arena_chunk_tree_t;

typedef struct arena_run_s arena_run_t;
struct arena_run_s {
#ifdef MALLOC_DEBUG
	uint32_t	magic;
#  define ARENA_RUN_MAGIC 0x384adf93
#endif
    
	/* Bin this run is associated with. */
	arena_bin_t	*bin;
    
	/* Index of first element that might have a free region. */
	unsigned	regs_minelm;
    
	/* Number of free regions in run. */
	unsigned	nfree;
    
	/* Bitmask of in-use regions (0: in use, 1: free). */
	unsigned	regs_mask[1]; /* Dynamically sized. */
};

struct arena_bin_s {
	/*
	 * Current run being used to service allocations of this bin's size
	 * class.
	 */
	arena_run_t	*runcur;
    
	/*
	 * Tree of non-full runs.  This tree is used when looking for an
	 * existing run when runcur is no longer usable.  We choose the
	 * non-full run that is lowest in memory; this policy tends to keep
	 * objects packed well, and it can also help reduce the number of
	 * almost-empty chunks.
	 */
	arena_run_tree_t runs;
    
	/* Size of regions in a run for this bin's size class. */
	size_t		reg_size;
    
	/* Total size of a run for this bin's size class. */
	size_t		run_size;
    
	/* Total number of regions in a run for this bin's size class. */
	uint32_t	nregs;
    
	/* Number of elements in a run's regs_mask for this bin's size class. */
	uint32_t	regs_mask_nelms;
    
	/* Offset of first region in a run for this bin's size class. */
	uint32_t	reg0_offset;
    
#ifdef MALLOC_STATS
	/* Bin statistics. */
	malloc_bin_stats_t stats;
#endif
};

struct arena_s {
#ifdef MALLOC_DEBUG
	uint32_t		magic;
#  define ARENA_MAGIC 0x947d3d24
#endif
    
#ifdef MALLOC_STATS
	arena_stats_t		stats;
#endif
    
	/* Tree of dirty-page-containing chunks this arena manages. */
	arena_chunk_tree_t	chunks_dirty;
    
	/*
	 * In order to avoid rapid chunk allocation/deallocation when an arena
	 * oscillates right on the cusp of needing a new chunk, cache the most
	 * recently freed chunk.  The spare is left in the arena's chunk trees
	 * until it is deleted.
	 *
	 * There is one spare chunk per arena, rather than one spare total, in
	 * order to avoid interactions between multiple threads that could make
	 * a single spare inadequate.
	 */
	arena_chunk_t		*spare;
    
	/*
	 * Current count of pages within unused runs that are potentially
	 * dirty, and for which madvise(... MADV_DONTNEED) has not been called.
	 * By tracking this, we can institute a limit on how much dirty unused
	 * memory is mapped for each arena.
	 */
	size_t			ndirty;
    
	/*
	 * Size/address-ordered tree of this arena's available runs.  This tree
	 * is used for first-best-fit run allocation.
	 */
	arena_avail_tree_t	runs_avail;
    
	/*
	 * bins is used to store rings of free regions of the following sizes,
	 * assuming a 16-byte quantum, 4kB pagesize, and default MALLOC_OPTIONS.
	 *
	 *   bins[i] | size |
	 *   --------+------+
	 *        0  |    2 |
	 *        1  |    4 |
	 *        2  |    8 |
	 *   --------+------+
	 *        3  |   16 |
	 *        4  |   32 |
	 *        5  |   48 |
	 *        6  |   64 |
	 *           :      :
	 *           :      :
	 *       33  |  496 |
	 *       34  |  512 |
	 *   --------+------+
	 *       35  | 1024 |
	 *       36  | 2048 |
	 *   --------+------+
	 */
	arena_bin_t		bins[1]; /* Dynamically sized. */
};

/******************************************************************************/
/*
 * Data.
 */

/* Number of CPUs. */
static unsigned		ncpus;

/* VM page size. */
static size_t		pagesize;
static size_t		pagesize_mask;
static size_t		pagesize_2pow;

/* Various bin-related settings. */
#ifdef MALLOC_TINY		/* Number of (2^n)-spaced tiny bins. */
#  define		ntbins	((unsigned)(QUANTUM_2POW - TINY_MIN_2POW))
#else
#  define		ntbins	0
#endif
static unsigned		nqbins; /* Number of quantum-spaced bins. */
static unsigned		ncbins; /* Number of cacheline-spaced bins. */
static unsigned		nsbins; /* Number of subpage-spaced bins. */
static unsigned		nbins;
#ifdef MALLOC_TINY
#  define		tspace_max	((size_t)(QUANTUM >> 1))
#endif
#define			qspace_min	QUANTUM
static size_t		qspace_max;
static size_t		cspace_min;
static size_t		cspace_max;
static size_t		sspace_min;
static size_t		sspace_max;
#define			bin_maxclass	sspace_max

static uint8_t const	*size2bin;
/*
 * const_size2bin is a static constant lookup table that in the common case can
 * be used as-is for size2bin.  For dynamically linked programs, this avoids
 * a page of memory overhead per process.
 */
#define	S2B_1(i)	i,
#define	S2B_2(i)	S2B_1(i) S2B_1(i)
#define	S2B_4(i)	S2B_2(i) S2B_2(i)
#define	S2B_8(i)	S2B_4(i) S2B_4(i)
#define	S2B_16(i)	S2B_8(i) S2B_8(i)
#define	S2B_32(i)	S2B_16(i) S2B_16(i)
#define	S2B_64(i)	S2B_32(i) S2B_32(i)
#define	S2B_128(i)	S2B_64(i) S2B_64(i)
#define	S2B_256(i)	S2B_128(i) S2B_128(i)
static const uint8_t	const_size2bin[(1U << PAGESIZE_2POW) - 255] = {
	S2B_1(0xffU)		/*    0 */
#if (QUANTUM_2POW == 4)
    /* 64-bit system ************************/
#  ifdef MALLOC_TINY
	S2B_2(0)		/*    2 */
	S2B_2(1)		/*    4 */
	S2B_4(2)		/*    8 */
	S2B_8(3)		/*   16 */
#    define S2B_QMIN 3
#  else
	S2B_16(0)		/*   16 */
#    define S2B_QMIN 0
#  endif
	S2B_16(S2B_QMIN + 1)	/*   32 */
	S2B_16(S2B_QMIN + 2)	/*   48 */
	S2B_16(S2B_QMIN + 3)	/*   64 */
	S2B_16(S2B_QMIN + 4)	/*   80 */
	S2B_16(S2B_QMIN + 5)	/*   96 */
	S2B_16(S2B_QMIN + 6)	/*  112 */
	S2B_16(S2B_QMIN + 7)	/*  128 */
#  define S2B_CMIN (S2B_QMIN + 8)
#else
    /* 32-bit system ************************/
#  ifdef MALLOC_TINY
	S2B_2(0)		/*    2 */
	S2B_2(1)		/*    4 */
	S2B_4(2)		/*    8 */
#    define S2B_QMIN 2
#  else
	S2B_8(0)		/*    8 */
#    define S2B_QMIN 0
#  endif
	S2B_8(S2B_QMIN + 1)	/*   16 */
	S2B_8(S2B_QMIN + 2)	/*   24 */
	S2B_8(S2B_QMIN + 3)	/*   32 */
	S2B_8(S2B_QMIN + 4)	/*   40 */
	S2B_8(S2B_QMIN + 5)	/*   48 */
	S2B_8(S2B_QMIN + 6)	/*   56 */
	S2B_8(S2B_QMIN + 7)	/*   64 */
	S2B_8(S2B_QMIN + 8)	/*   72 */
	S2B_8(S2B_QMIN + 9)	/*   80 */
	S2B_8(S2B_QMIN + 10)	/*   88 */
	S2B_8(S2B_QMIN + 11)	/*   96 */
	S2B_8(S2B_QMIN + 12)	/*  104 */
	S2B_8(S2B_QMIN + 13)	/*  112 */
	S2B_8(S2B_QMIN + 14)	/*  120 */
	S2B_8(S2B_QMIN + 15)	/*  128 */
#  define S2B_CMIN (S2B_QMIN + 16)
#endif
    /****************************************/
	S2B_64(S2B_CMIN + 0)	/*  192 */
	S2B_64(S2B_CMIN + 1)	/*  256 */
	S2B_64(S2B_CMIN + 2)	/*  320 */
	S2B_64(S2B_CMIN + 3)	/*  384 */
	S2B_64(S2B_CMIN + 4)	/*  448 */
	S2B_64(S2B_CMIN + 5)	/*  512 */
#  define S2B_SMIN (S2B_CMIN + 6)
	S2B_256(S2B_SMIN + 0)	/*  768 */
	S2B_256(S2B_SMIN + 1)	/* 1024 */
	S2B_256(S2B_SMIN + 2)	/* 1280 */
	S2B_256(S2B_SMIN + 3)	/* 1536 */
	S2B_256(S2B_SMIN + 4)	/* 1792 */
	S2B_256(S2B_SMIN + 5)	/* 2048 */
	S2B_256(S2B_SMIN + 6)	/* 2304 */
	S2B_256(S2B_SMIN + 7)	/* 2560 */
	S2B_256(S2B_SMIN + 8)	/* 2816 */
	S2B_256(S2B_SMIN + 9)	/* 3072 */
	S2B_256(S2B_SMIN + 10)	/* 3328 */
	S2B_256(S2B_SMIN + 11)	/* 3584 */
	S2B_256(S2B_SMIN + 12)	/* 3840 */
#if (PAGESIZE_2POW == 13)
	S2B_256(S2B_SMIN + 13)	/* 4096 */
	S2B_256(S2B_SMIN + 14)	/* 4352 */
	S2B_256(S2B_SMIN + 15)	/* 4608 */
	S2B_256(S2B_SMIN + 16)	/* 4864 */
	S2B_256(S2B_SMIN + 17)	/* 5120 */
	S2B_256(S2B_SMIN + 18)	/* 5376 */
	S2B_256(S2B_SMIN + 19)	/* 5632 */
	S2B_256(S2B_SMIN + 20)	/* 5888 */
	S2B_256(S2B_SMIN + 21)	/* 6144 */
	S2B_256(S2B_SMIN + 22)	/* 6400 */
	S2B_256(S2B_SMIN + 23)	/* 6656 */
	S2B_256(S2B_SMIN + 24)	/* 6912 */
	S2B_256(S2B_SMIN + 25)	/* 7168 */
	S2B_256(S2B_SMIN + 26)	/* 7424 */
	S2B_256(S2B_SMIN + 27)	/* 7680 */
	S2B_256(S2B_SMIN + 28)	/* 7936 */
#endif
};
#undef S2B_1
#undef S2B_2
#undef S2B_4
#undef S2B_8
#undef S2B_16
#undef S2B_32
#undef S2B_64
#undef S2B_128
#undef S2B_256
#undef S2B_QMIN
#undef S2B_CMIN
#undef S2B_SMIN


/* Various chunk-related settings. */
static size_t		chunksize;
static size_t		chunksize_mask; /* (chunksize - 1). */
static size_t		chunk_npages;
static size_t		arena_chunk_header_npages;
static size_t		arena_maxclass; /* Max size class for arenas. */

/********/
/*
 * Chunks.
 */

/* Tree of chunks that are stand-alone huge allocations. */
static extent_tree_t	huge;

/*
 * Protects sbrk() calls.  This avoids malloc races among threads, though it
 * does not protect against races with threads that call sbrk() directly.
 */
/* Base address of the DSS. */
static void		*dss_base;
/* Current end of the DSS, or ((void *)-1) if the DSS is exhausted. */
static void		*dss_prev;
/* Current upper limit on DSS addresses. */
static void		*dss_max;

/*
 * Trees of chunks that were previously allocated (trees differ only in node
 * ordering).  These are used when allocating chunks, in an attempt to re-use
 * address space.  Depending on function, different tree orderings are needed,
 * which is why there are two trees with the same contents.
 */
static extent_tree_t	dss_chunks_szad;
static extent_tree_t	dss_chunks_ad;

#ifdef MALLOC_STATS
/* Huge allocation statistics. */
static uint64_t		huge_nmalloc;
static uint64_t		huge_ndalloc;
static size_t		huge_allocated;
#endif

/****************************/
/*
 * base (internal allocation).
 */

/*
 * Current pages that are being used for internal memory allocations.  These
 * pages are carved up in cacheline-size quanta, so that there is no chance of
 * false cache line sharing.
 */
static void		*base_pages;
static void		*base_next_addr;
static void		*base_past_addr; /* Addr immediately past base_pages. */
static extent_node_t	*base_nodes;
#ifdef MALLOC_STATS
static size_t		base_mapped;
#endif

/********/
/*
 * Arenas.
 */

/*
 * Arenas that are used to service external requests.  Not all elements of the
 * arenas array are necessarily used; arenas are created lazily as needed.
 */
static arena_t		**arenas;
static unsigned		narenas;

#ifdef MALLOC_STATS
/* Chunk statistics. */
static chunk_stats_t	stats_chunks;
#endif

/*******************************/
/*
 * Runtime configuration options.
 */
const char	*_malloc_options;

#ifndef MALLOC_PRODUCTION
static bool	opt_abort = true;
static bool	opt_junk = true;
#else
static bool	opt_abort = false;
static bool	opt_junk = false;
#endif
static size_t	opt_dirty_max = DIRTY_MAX_DEFAULT;
static bool	opt_print_stats = false;
static size_t	opt_qspace_max_2pow = QSPACE_MAX_2POW_DEFAULT;
static size_t	opt_cspace_max_2pow = CSPACE_MAX_2POW_DEFAULT;
static size_t	opt_chunk_2pow = CHUNK_2POW_DEFAULT;
static bool	opt_sysv = false;
static bool	opt_xmalloc = false;
static bool	opt_zero = false;
static int	opt_narenas_lshift = 0;

/******************************************************************************/
/*
 * Begin function prototypes for non-inline static functions.
 */

static void	wrtmessage(const char *p1, const char *p2, const char *p3,
                       const char *p4);
#ifdef MALLOC_STATS
static void	malloc_printf(const char *format, ...);
#endif
static char	*umax2s(uintmax_t x, char *s);
static bool	base_pages_alloc_dss(size_t minsize);
static bool	base_pages_alloc(size_t minsize);
static void	*base_alloc(size_t size);
static extent_node_t *base_node_alloc(void);
static void	base_node_dealloc(extent_node_t *node);
#ifdef MALLOC_STATS
static void	stats_print(arena_t *arena);
#endif
static void	*chunk_alloc_dss(size_t size);
static void	*chunk_recycle_dss(size_t size, bool zero);
static void	*chunk_alloc(size_t size, bool zero);
static extent_node_t *chunk_dealloc_dss_record(void *chunk, size_t size);
static bool	chunk_dealloc_dss(void *chunk, size_t size);
static void	chunk_dealloc(void *chunk, size_t size);

static void	arena_run_split(arena_t *arena, arena_run_t *run, size_t size,
                            bool large, bool zero);
static arena_chunk_t *arena_chunk_alloc(arena_t *arena);
static void	arena_chunk_dealloc(arena_t *arena, arena_chunk_t *chunk);
static arena_run_t *arena_run_alloc(arena_t *arena, size_t size, bool large,
                                    bool zero);
static void	arena_purge(arena_t *arena);
static void	arena_run_dalloc(arena_t *arena, arena_run_t *run, bool dirty);
static void	arena_run_trim_head(arena_t *arena, arena_chunk_t *chunk,
                                arena_run_t *run, size_t oldsize, size_t newsize);
static void	arena_run_trim_tail(arena_t *arena, arena_chunk_t *chunk,
                                arena_run_t *run, size_t oldsize, size_t newsize, bool dirty);
static arena_run_t *arena_bin_nonfull_run_get(arena_t *arena, arena_bin_t *bin);
static void	*arena_bin_malloc_hard(arena_t *arena, arena_bin_t *bin);
static size_t	arena_bin_run_size_calc(arena_bin_t *bin, size_t min_run_size);

static void	*arena_malloc_large(arena_t *arena, size_t size, bool zero);
static void	*arena_palloc(arena_t *arena, size_t alignment, size_t size,
                          size_t alloc_size);
static size_t	arena_salloc(const void *ptr);

static void	arena_dalloc_large(arena_t *arena, arena_chunk_t *chunk,
                               void *ptr);
static void	arena_ralloc_large_shrink(arena_t *arena, arena_chunk_t *chunk,
                                      void *ptr, size_t size, size_t oldsize);
static bool	arena_ralloc_large_grow(arena_t *arena, arena_chunk_t *chunk,
                                    void *ptr, size_t size, size_t oldsize);
static bool	arena_ralloc_large(void *ptr, size_t size, size_t oldsize);
static void	*arena_ralloc(void *ptr, size_t size, size_t oldsize);
static bool	arena_new(arena_t *arena);
static arena_t	*arenas_extend(unsigned ind);

static void	*huge_malloc(size_t size, bool zero);
static void	*huge_palloc(size_t alignment, size_t size);
static void	*huge_ralloc(void *ptr, size_t size, size_t oldsize);
static void	huge_dalloc(void *ptr);
#ifdef MALLOC_DEBUG
static void	size2bin_validate(void);
#endif
static bool	size2bin_init(void);
static bool	size2bin_init_hard(void);
static unsigned	malloc_ncpus(void);
static bool	malloc_init_hard(void);
void		_malloc_prefork(void);
void		_malloc_postfork(void);

/*
 * End function prototypes.
 */
/******************************************************************************/

static void
wrtmessage(const char *p1, const char *p2, const char *p3, const char *p4)
{    
	__printf("%s", p1);
	__printf("%s", p2);
	__printf("%s", p3);
	__printf("%s", p4);
}

#define	_malloc_message malloc_message
void	(*_malloc_message)(const char *p1, const char *p2, const char *p3,
                           const char *p4) = wrtmessage;

/*
 * We don't want to depend on vsnprintf() for production builds, since that can
 * cause unnecessary bloat for static binaries.  umax2s() provides minimal
 * integer printing functionality, so that malloc_printf() use can be limited to
 * MALLOC_STATS code.
 */
#define	UMAX2S_BUFSIZE	21
static char *
umax2s(uintmax_t x, char *s)
{
	unsigned i;
    
	i = UMAX2S_BUFSIZE - 1;
	s[i] = '\0';
	do {
		i--;
		s[i] = "0123456789"[x % 10];
		x /= 10;
	} while (x > 0);
    
	return (&s[i]);
}

/*
 * Define a custom assert() in order to reduce the chances of deadlock during
 * assertion failure.
 */
#ifdef MALLOC_DEBUG
#  define assert(e) do {						\
if (!(e)) {							\
char line_buf[UMAX2S_BUFSIZE];				\
_malloc_message(__FILE__, ":", umax2s(__LINE__,		\
line_buf), ": Failed assertion: ");			\
_malloc_message("\"", #e, "\"\n", "");			\
abort();						\
}								\
} while (0)
#else
#define assert(e)
#endif

static inline const char *
_getprogname(void)
{
    
	return ("<jemalloc>");
}

#ifdef MALLOC_STATS
/*
 * Print to stderr in such a way as to (hopefully) avoid memory allocation.
 */
static void
malloc_printf(const char *format, ...)
{
	char buf[4096];
	va_list ap;
    
	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);
	_malloc_message(buf, "", "", "");
}
#endif

/*
 * Round up val to the next power of two
 */
static inline size_t
next_power_of_two(size_t val)
{
	val--;
	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;
	val |= val >> 16;
#if (__SIZEOF_SIZE_T__ == 8U)
	val |= val >> 32;
#endif
	++val;
	return val;
}


/******************************************************************************/
/*
 * Begin Utility functions/macros.
 */

/* Return the chunk address for allocation address a. */
#define	CHUNK_ADDR2BASE(a)						\
((void *)((uintptr_t)(a) & ~chunksize_mask))

/* Return the chunk offset of address a. */
#define	CHUNK_ADDR2OFFSET(a)						\
((size_t)((uintptr_t)(a) & chunksize_mask))

/* Return the smallest chunk multiple that is >= s. */
#define	CHUNK_CEILING(s)						\
(((s) + chunksize_mask) & ~chunksize_mask)

/* Return the smallest quantum multiple that is >= a. */
#define	QUANTUM_CEILING(a)						\
(((a) + QUANTUM_MASK) & ~QUANTUM_MASK)

/* Return the smallest cacheline multiple that is >= s. */
#define	CACHELINE_CEILING(s)						\
(((s) + CACHELINE_MASK) & ~CACHELINE_MASK)

/* Return the smallest subpage multiple that is >= s. */
#define	SUBPAGE_CEILING(s)						\
(((s) + SUBPAGE_MASK) & ~SUBPAGE_MASK)

/* Return the smallest pagesize multiple that is >= s. */
#define	PAGE_CEILING(s)							\
(((s) + pagesize_mask) & ~pagesize_mask)

#ifdef MALLOC_TINY
/* Compute the smallest power of 2 that is >= x. */
static inline size_t
pow2_ceil(size_t x)
{
    
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
#if (SIZEOF_PTR == 8)
	x |= x >> 32;
#endif
	x++;
	return (x);
}
#endif

/******************************************************************************/

static bool
base_pages_alloc_dss(size_t minsize)
{
    
	/*
	 * Do special DSS allocation here, since base allocations don't need to
	 * be chunk-aligned.
	 */
	if (dss_prev != (void *)-1) {
		intptr_t incr;
		size_t csize = CHUNK_CEILING(minsize);
        
		do {
			/* Get the current end of the DSS. */
			dss_max = sbrk(0);
            
			/*
			 * Calculate how much padding is necessary to
			 * chunk-align the end of the DSS.  Don't worry about
			 * dss_max not being chunk-aligned though.
			 */
			incr = (intptr_t)chunksize
            - (intptr_t)CHUNK_ADDR2OFFSET(dss_max);
			assert(incr >= 0);
			if ((size_t)incr < minsize)
				incr += csize;
            
			dss_prev = sbrk(incr);
			if (dss_prev == dss_max) {
				/* Success. */
				dss_max = (void *)((intptr_t)dss_prev + incr);
				base_pages = dss_prev;
				base_next_addr = base_pages;
				base_past_addr = dss_max;
#ifdef MALLOC_STATS
				base_mapped += incr;
#endif
				return (false);
			}
		} while (dss_prev != (void *)-1);
	}
    
	return (true);
}

static bool
base_pages_alloc(size_t minsize)
{
    
    if (base_pages_alloc_dss(minsize) == false)
        return (false);
    
	return (true);
}

static void *
base_alloc(size_t size)
{
	void *ret;
	size_t csize;
    
	/* Round size up to nearest multiple of the cacheline size. */
	csize = CACHELINE_CEILING(size);
    
	/* Make sure there's enough space for the allocation. */
	if ((uintptr_t)base_next_addr + csize > (uintptr_t)base_past_addr) {
		if (base_pages_alloc(csize)) {
			return (NULL);
		}
	}
	/* Allocate. */
	ret = base_next_addr;
	base_next_addr = (void *)((uintptr_t)base_next_addr + csize);
    
	return (ret);
}

static extent_node_t *
base_node_alloc(void)
{
	extent_node_t *ret;
    
	if (base_nodes != NULL) {
		ret = base_nodes;
		base_nodes = *(extent_node_t **)ret;
	} else {
		ret = (extent_node_t *)base_alloc(sizeof(extent_node_t));
	}
    
	return (ret);
}

static void
base_node_dealloc(extent_node_t *node)
{
    
	*(extent_node_t **)node = base_nodes;
	base_nodes = node;
}

/******************************************************************************/

#ifdef MALLOC_STATS
static void
stats_print(arena_t *arena)
{
	unsigned i, gap_start;
    
	malloc_printf("dirty: %zu page%s dirty, %llu sweep%s,"
                  " %llu madvise%s, %llu page%s purged\n",
                  arena->ndirty, arena->ndirty == 1 ? "" : "s",
                  arena->stats.npurge, arena->stats.npurge == 1 ? "" : "s",
                  arena->stats.nmadvise, arena->stats.nmadvise == 1 ? "" : "s",
                  arena->stats.purged, arena->stats.purged == 1 ? "" : "s");
    
	malloc_printf("            allocated      nmalloc      ndalloc\n");
	malloc_printf("small:   %12zu %12llu %12llu\n",
                  arena->stats.allocated_small, arena->stats.nmalloc_small,
                  arena->stats.ndalloc_small);
	malloc_printf("large:   %12zu %12llu %12llu\n",
                  arena->stats.allocated_large, arena->stats.nmalloc_large,
                  arena->stats.ndalloc_large);
	malloc_printf("total:   %12zu %12llu %12llu\n",
                  arena->stats.allocated_small + arena->stats.allocated_large,
                  arena->stats.nmalloc_small + arena->stats.nmalloc_large,
                  arena->stats.ndalloc_small + arena->stats.ndalloc_large);
	malloc_printf("mapped:  %12zu\n", arena->stats.mapped);
    
	
	malloc_printf("bins:     bin   size regs pgs  requests   "
				  "newruns    reruns maxruns curruns\n");
	
	for (i = 0, gap_start = UINT_MAX; i < nbins; i++) {
		if (arena->bins[i].stats.nruns == 0) {
			if (gap_start == UINT_MAX)
				gap_start = i;
		} else {
			if (gap_start != UINT_MAX) {
				if (i > gap_start + 1) {
					/* Gap of more than one size class. */
					malloc_printf("[%u..%u]\n",
                                  gap_start, i - 1);
				} else {
					/* Gap of one size class. */
					malloc_printf("[%u]\n", gap_start);
				}
				gap_start = UINT_MAX;
			}
			malloc_printf(
                          "%13u %1s %4u %4u %3u %9llu %9llu"
                          " %9llu %7lu %7lu\n",
                          i,
                          i < ntbins ? "T" : i < ntbins + nqbins ? "Q" :
                          i < ntbins + nqbins + ncbins ? "C" : "S",
                          arena->bins[i].reg_size,
                          arena->bins[i].nregs,
                          arena->bins[i].run_size >> pagesize_2pow,
						  
                          arena->bins[i].stats.nrequests,
                          arena->bins[i].stats.nruns,
                          arena->bins[i].stats.reruns,
                          arena->bins[i].stats.highruns,
                          arena->bins[i].stats.curruns);
		}
	}
	if (gap_start != UINT_MAX) {
		if (i > gap_start + 1) {
			/* Gap of more than one size class. */
			malloc_printf("[%u..%u]\n", gap_start, i - 1);
		} else {
			/* Gap of one size class. */
			malloc_printf("[%u]\n", gap_start);
		}
	}
}
#endif

/*
 * End Utility functions/macros.
 */
/******************************************************************************/
/*
 * Begin extent tree code.
 */

static inline int
extent_szad_comp(extent_node_t *a, extent_node_t *b)
{
	int ret;
	size_t a_size = a->size;
	size_t b_size = b->size;
    
	ret = (a_size > b_size) - (a_size < b_size);
	if (ret == 0) {
		uintptr_t a_addr = (uintptr_t)a->addr;
		uintptr_t b_addr = (uintptr_t)b->addr;
        
		ret = (a_addr > b_addr) - (a_addr < b_addr);
	}
    
	return (ret);
}

/* Wrap red-black tree macros in functions. */
rb_wrap(__attribute__ ((__unused__)) static, extent_tree_szad_, extent_tree_t, extent_node_t,
        link_szad, extent_szad_comp)

static inline int
extent_ad_comp(extent_node_t *a, extent_node_t *b)
{
	uintptr_t a_addr = (uintptr_t)a->addr;
	uintptr_t b_addr = (uintptr_t)b->addr;
    
	return ((a_addr > b_addr) - (a_addr < b_addr));
}

/* Wrap red-black tree macros in functions. */
rb_wrap(__attribute__ ((__unused__)) static, extent_tree_ad_, extent_tree_t, extent_node_t, link_ad,
        extent_ad_comp)

/*
 * End extent tree code.
 */
/******************************************************************************/
/*
 * Begin chunk management functions.
 */

static void *
chunk_alloc_dss(size_t size)
{
    
	/*
	 * sbrk() uses a signed increment argument, so take care not to
	 * interpret a huge allocation request as a negative increment.
	 */
	if ((intptr_t)size < 0)
		return (NULL);
    
	if (dss_prev != (void *)-1) {
		intptr_t incr;
        
		/*
		 * The loop is necessary to recover from races with other
		 * threads that are using the DSS for something other than
		 * malloc.
		 */
		do {
			void *ret;
            
			/* Get the current end of the DSS. */
			dss_max = sbrk(0);
            
			/*
			 * Calculate how much padding is necessary to
			 * chunk-align the end of the DSS.
			 */
			incr = (intptr_t)size
            - (intptr_t)CHUNK_ADDR2OFFSET(dss_max);
			if (incr == (intptr_t)size)
				ret = dss_max;
			else {
				ret = (void *)((intptr_t)dss_max + incr);
				incr += size;
			}
            
			dss_prev = sbrk(incr);
			if (dss_prev == dss_max) {
				/* Success. */
				dss_max = (void *)((intptr_t)dss_prev + incr);
				return (ret);
			}
		} while (dss_prev != (void *)-1);
	}
    
	return (NULL);
}

static void *
chunk_recycle_dss(size_t size, bool zero)
{
	extent_node_t *node, key;
    
	key.addr = NULL;
	key.size = size;
	node = extent_tree_szad_nsearch(&dss_chunks_szad, &key);
	if (node != NULL) {
		void *ret = node->addr;
        
		/* Remove node from the tree. */
		extent_tree_szad_remove(&dss_chunks_szad, node);
		if (node->size == size) {
			extent_tree_ad_remove(&dss_chunks_ad, node);
			base_node_dealloc(node);
		} else {
			/*
			 * Insert the remainder of node's address range as a
			 * smaller chunk.  Its position within dss_chunks_ad
			 * does not change.
			 */
			assert(node->size > size);
			node->addr = (void *)((uintptr_t)node->addr + size);
			node->size -= size;
			extent_tree_szad_insert(&dss_chunks_szad, node);
		}
        
		if (zero)
			memset(ret, 0, size);
		return (ret);
	}
    
	return (NULL);
}

static void *
chunk_alloc(size_t size, bool zero)
{
	void *ret;
    
	assert(size != 0);
	assert((size & chunksize_mask) == 0);
    
    
    ret = chunk_recycle_dss(size, zero);
    if (ret != NULL) {
        goto RETURN;
    }
    
    ret = chunk_alloc_dss(size);
    if (ret != NULL)
        goto RETURN;
    
	
    
	/* All strategies for allocation failed. */
	ret = NULL;
RETURN:
#ifdef MALLOC_STATS
	if (ret != NULL) {
		stats_chunks.nchunks += (size / chunksize);
		stats_chunks.curchunks += (size / chunksize);
	}
	if (stats_chunks.curchunks > stats_chunks.highchunks)
		stats_chunks.highchunks = stats_chunks.curchunks;
#endif
    
	assert(CHUNK_ADDR2BASE(ret) == ret);
	return (ret);
}

static extent_node_t *
chunk_dealloc_dss_record(void *chunk, size_t size)
{
	extent_node_t *node, *prev, key;
    
	key.addr = (void *)((uintptr_t)chunk + size);
	node = extent_tree_ad_nsearch(&dss_chunks_ad, &key);
	/* Try to coalesce forward. */
	if (node != NULL && node->addr == key.addr) {
		/*
		 * Coalesce chunk with the following address range.  This does
		 * not change the position within dss_chunks_ad, so only
		 * remove/insert from/into dss_chunks_szad.
		 */
		extent_tree_szad_remove(&dss_chunks_szad, node);
		node->addr = chunk;
		node->size += size;
		extent_tree_szad_insert(&dss_chunks_szad, node);
	} else {
		/*
		 * Coalescing forward failed, so insert a new node.  Drop
		 * dss_mtx during node allocation, since it is possible that a
		 * new base chunk will be allocated.
		 */
		node = base_node_alloc();
		if (node == NULL)
			return (NULL);
		node->addr = chunk;
		node->size = size;
		extent_tree_ad_insert(&dss_chunks_ad, node);
		extent_tree_szad_insert(&dss_chunks_szad, node);
	}
    
	/* Try to coalesce backward. */
	prev = extent_tree_ad_prev(&dss_chunks_ad, node);
	if (prev != NULL && (void *)((uintptr_t)prev->addr + prev->size) ==
	    chunk) {
		/*
		 * Coalesce chunk with the previous address range.  This does
		 * not change the position within dss_chunks_ad, so only
		 * remove/insert node from/into dss_chunks_szad.
		 */
		extent_tree_szad_remove(&dss_chunks_szad, prev);
		extent_tree_ad_remove(&dss_chunks_ad, prev);
        
		extent_tree_szad_remove(&dss_chunks_szad, node);
		node->addr = prev->addr;
		node->size += prev->size;
		extent_tree_szad_insert(&dss_chunks_szad, node);
        
		base_node_dealloc(prev);
	}
    
	return (node);
}

static bool
chunk_dealloc_dss(void *chunk, size_t size)
{
    
	if ((uintptr_t)chunk >= (uintptr_t)dss_base
	    && (uintptr_t)chunk < (uintptr_t)dss_max) {
		extent_node_t *node;
        
		/* Try to coalesce with other unused chunks. */
		node = chunk_dealloc_dss_record(chunk, size);
		if (node != NULL) {
			chunk = node->addr;
			size = node->size;
		}
        
		/* Get the current end of the DSS. */
		dss_max = sbrk(0);
        
		/*
		 * Try to shrink the DSS if this chunk is at the end of the
		 * DSS.  The sbrk() call here is subject to a race condition
		 * with threads that use brk(2) or sbrk(2) directly, but the
		 * alternative would be to leak memory for the sake of poorly
		 * designed multi-threaded programs.
		 */
		if ((void *)((uintptr_t)chunk + size) == dss_max
		    && (dss_prev = sbrk(-(intptr_t)size)) == dss_max) {
			/* Success. */
			dss_max = (void *)((intptr_t)dss_prev - (intptr_t)size);
            
			if (node != NULL) {
				extent_tree_szad_remove(&dss_chunks_szad, node);
				extent_tree_ad_remove(&dss_chunks_ad, node);
				base_node_dealloc(node);
			}
		}
        
		return (false);
	}
    
	return (true);
}

static void
chunk_dealloc(void *chunk, size_t size)
{
    
	assert(chunk != NULL);
	assert(CHUNK_ADDR2BASE(chunk) == chunk);
	assert(size != 0);
	assert((size & chunksize_mask) == 0);
    
#ifdef MALLOC_STATS
	stats_chunks.curchunks -= (size / chunksize);
#endif
    
    if (chunk_dealloc_dss(chunk, size) == false)
        return;
    
}

/*
 * End chunk management functions.
 */
/******************************************************************************/
/*
 * Begin arena.
 */

/*
 * Choose an arena based on a per-thread value (fast-path code, calls slow-path
 * code if necessary).
 */
static inline arena_t *
choose_arena(void)
{
	arena_t *ret;    
	
	ret = arenas[0];
    
	assert(ret != NULL);
	return (ret);
}

static inline int
arena_chunk_comp(arena_chunk_t *a, arena_chunk_t *b)
{
	uintptr_t a_chunk = (uintptr_t)a;
	uintptr_t b_chunk = (uintptr_t)b;
    
	assert(a != NULL);
	assert(b != NULL);
    
	return ((a_chunk > b_chunk) - (a_chunk < b_chunk));
}

/* Wrap red-black tree macros in functions. */
rb_wrap(__attribute__ ((__unused__)) static, arena_chunk_tree_dirty_, arena_chunk_tree_t,
        arena_chunk_t, link_dirty, arena_chunk_comp)

static inline int
arena_run_comp(arena_chunk_map_t *a, arena_chunk_map_t *b)
{
	uintptr_t a_mapelm = (uintptr_t)a;
	uintptr_t b_mapelm = (uintptr_t)b;
    
	assert(a != NULL);
	assert(b != NULL);
    
	return ((a_mapelm > b_mapelm) - (a_mapelm < b_mapelm));
}

/* Wrap red-black tree macros in functions. */
rb_wrap(__attribute__ ((__unused__)) static, arena_run_tree_, arena_run_tree_t, arena_chunk_map_t,
        link, arena_run_comp)

static inline int
arena_avail_comp(arena_chunk_map_t *a, arena_chunk_map_t *b)
{
	int ret;
	size_t a_size = a->bits & ~pagesize_mask;
	size_t b_size = b->bits & ~pagesize_mask;
    
	ret = (a_size > b_size) - (a_size < b_size);
	if (ret == 0) {
		uintptr_t a_mapelm, b_mapelm;
        
		if ((a->bits & CHUNK_MAP_KEY) == 0)
			a_mapelm = (uintptr_t)a;
		else {
			/*
			 * Treat keys as though they are lower than anything
			 * else.
			 */
			a_mapelm = 0;
		}
		b_mapelm = (uintptr_t)b;
        
		ret = (a_mapelm > b_mapelm) - (a_mapelm < b_mapelm);
	}
    
	return (ret);
}

/* Wrap red-black tree macros in functions. */
rb_wrap(__attribute__ ((__unused__)) static, arena_avail_tree_, arena_avail_tree_t,
        arena_chunk_map_t, link, arena_avail_comp)

static inline void *
arena_run_reg_alloc(arena_run_t *run, arena_bin_t *bin)
{
	void *ret;
	unsigned i, mask, bit, regind;
    
	assert(run->magic == ARENA_RUN_MAGIC);
	assert(run->regs_minelm < bin->regs_mask_nelms);
    
	/*
	 * Move the first check outside the loop, so that run->regs_minelm can
	 * be updated unconditionally, without the possibility of updating it
	 * multiple times.
	 */
	i = run->regs_minelm;
	mask = run->regs_mask[i];
	if (mask != 0) {
		/* Usable allocation found. */
		bit = ffs((int)mask) - 1;
        
		regind = ((i << (SIZEOF_INT_2POW + 3)) + bit);
		assert(regind < bin->nregs);
		ret = (void *)(((uintptr_t)run) + bin->reg0_offset
                       + (bin->reg_size * regind));
        
		/* Clear bit. */
		mask ^= (1U << bit);
		run->regs_mask[i] = mask;
        
		return (ret);
	}
    
	for (i++; i < bin->regs_mask_nelms; i++) {
		mask = run->regs_mask[i];
		if (mask != 0) {
			/* Usable allocation found. */
			bit = ffs((int)mask) - 1;
            
			regind = ((i << (SIZEOF_INT_2POW + 3)) + bit);
			assert(regind < bin->nregs);
			ret = (void *)(((uintptr_t)run) + bin->reg0_offset
                           + (bin->reg_size * regind));
            
			/* Clear bit. */
			mask ^= (1U << bit);
			run->regs_mask[i] = mask;
            
			/*
			 * Make a note that nothing before this element
			 * contains a free region.
			 */
			run->regs_minelm = i; /* Low payoff: + (mask == 0); */
            
			return (ret);
		}
	}
	/* Not reached. */
	assert(0);
	return (NULL);
}

static inline void
arena_run_reg_dalloc(arena_run_t *run, arena_bin_t *bin, void *ptr, size_t size)
{
	unsigned diff, regind, elm, bit;
    
	assert(run->magic == ARENA_RUN_MAGIC);
    
	/*
	 * Avoid doing division with a variable divisor if possible.  Using
	 * actual division here can reduce allocator throughput by over 20%!
	 */
	diff = (unsigned)((uintptr_t)ptr - (uintptr_t)run - bin->reg0_offset);
	if ((size & (size - 1)) == 0) {
		/*
		 * log2_table allows fast division of a power of two in the
		 * [1..128] range.
		 *
		 * (x / divisor) becomes (x >> log2_table[divisor - 1]).
		 */
		static const unsigned char log2_table[] = {
		    0, 1, 0, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 4,
		    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5,
		    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6,
		    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7
		};
        
		if (size <= 128)
			regind = (diff >> log2_table[size - 1]);
		else if (size <= 32768)
			regind = diff >> (8 + log2_table[(size >> 8) - 1]);
		else
			regind = diff / size;
	} else if (size < qspace_max) {
		/*
		 * To divide by a number D that is not a power of two we
		 * multiply by (2^21 / D) and then right shift by 21 positions.
		 *
		 *   X / D
		 *
		 * becomes
		 *
		 *   (X * qsize_invs[(D >> QUANTUM_2POW) - 3])
		 *       >> SIZE_INV_SHIFT
		 *
		 * We can omit the first three elements, because we never
		 * divide by 0, and QUANTUM and 2*QUANTUM are both powers of
		 * two, which are handled above.
		 */
#define	SIZE_INV_SHIFT 21
#define	QSIZE_INV(s) (((1U << SIZE_INV_SHIFT) / (s << QUANTUM_2POW)) + 1)
		static const unsigned qsize_invs[] = {
		    QSIZE_INV(3),
		    QSIZE_INV(4), QSIZE_INV(5), QSIZE_INV(6), QSIZE_INV(7)
#if (QUANTUM_2POW < 4)
		    ,
		    QSIZE_INV(8), QSIZE_INV(9), QSIZE_INV(10), QSIZE_INV(11),
		    QSIZE_INV(12),QSIZE_INV(13), QSIZE_INV(14), QSIZE_INV(15)
#endif
		};
		assert(QUANTUM * (((sizeof(qsize_invs)) / sizeof(unsigned)) + 3)
               >= (1U << QSPACE_MAX_2POW_DEFAULT));
        
		if (size <= (((sizeof(qsize_invs) / sizeof(unsigned)) + 2) <<
                     QUANTUM_2POW)) {
			regind = qsize_invs[(size >> QUANTUM_2POW) - 3] * diff;
			regind >>= SIZE_INV_SHIFT;
		} else
			regind = diff / size;
#undef QSIZE_INV
	} else if (size < cspace_max) {
#define	CSIZE_INV(s) (((1U << SIZE_INV_SHIFT) / (s << CACHELINE_2POW)) + 1)
		static const unsigned csize_invs[] = {
		    CSIZE_INV(3),
		    CSIZE_INV(4), CSIZE_INV(5), CSIZE_INV(6), CSIZE_INV(7)
		};
		assert(CACHELINE * (((sizeof(csize_invs)) / sizeof(unsigned)) +
                            3) >= (1U << CSPACE_MAX_2POW_DEFAULT));
        
		if (size <= (((sizeof(csize_invs) / sizeof(unsigned)) + 2) <<
                     CACHELINE_2POW)) {
			regind = csize_invs[(size >> CACHELINE_2POW) - 3] *
            diff;
			regind >>= SIZE_INV_SHIFT;
		} else
			regind = diff / size;
#undef CSIZE_INV
	} else {
#define	SSIZE_INV(s) (((1U << SIZE_INV_SHIFT) / (s << SUBPAGE_2POW)) + 1)
		static const unsigned ssize_invs[] = {
		    SSIZE_INV(3),
		    SSIZE_INV(4), SSIZE_INV(5), SSIZE_INV(6), SSIZE_INV(7),
		    SSIZE_INV(8), SSIZE_INV(9), SSIZE_INV(10), SSIZE_INV(11),
		    SSIZE_INV(12), SSIZE_INV(13), SSIZE_INV(14), SSIZE_INV(15)
#if (PAGESIZE_2POW == 13)
		    ,
		    SSIZE_INV(16), SSIZE_INV(17), SSIZE_INV(18), SSIZE_INV(19),
		    SSIZE_INV(20), SSIZE_INV(21), SSIZE_INV(22), SSIZE_INV(23),
		    SSIZE_INV(24), SSIZE_INV(25), SSIZE_INV(26), SSIZE_INV(27),
		    SSIZE_INV(28), SSIZE_INV(29), SSIZE_INV(29), SSIZE_INV(30)
#endif
		};
		assert(SUBPAGE * (((sizeof(ssize_invs)) / sizeof(unsigned)) + 3)
               >= (1U << PAGESIZE_2POW));
        
		if (size < (((sizeof(ssize_invs) / sizeof(unsigned)) + 2) <<
                    SUBPAGE_2POW)) {
			regind = ssize_invs[(size >> SUBPAGE_2POW) - 3] * diff;
			regind >>= SIZE_INV_SHIFT;
		} else
			regind = diff / size;
#undef SSIZE_INV
	}
#undef SIZE_INV_SHIFT
	assert(diff == regind * size);
	assert(regind < bin->nregs);
    
	elm = regind >> (SIZEOF_INT_2POW + 3);
	if (elm < run->regs_minelm)
		run->regs_minelm = elm;
	bit = regind - (elm << (SIZEOF_INT_2POW + 3));
	assert((run->regs_mask[elm] & (1U << bit)) == 0);
	run->regs_mask[elm] |= (1U << bit);
}

static void
arena_run_split(arena_t *arena, arena_run_t *run, size_t size, bool large,
                bool zero)
{
	arena_chunk_t *chunk;
	size_t old_ndirty, run_ind, total_pages, need_pages, rem_pages, i;
    
	chunk = (arena_chunk_t *)CHUNK_ADDR2BASE(run);
	old_ndirty = chunk->ndirty;
	run_ind = (unsigned)(((uintptr_t)run - (uintptr_t)chunk)
                         >> pagesize_2pow);
	total_pages = (chunk->map[run_ind].bits & ~pagesize_mask) >>
    pagesize_2pow;
	need_pages = (size >> pagesize_2pow);
	assert(need_pages > 0);
	assert(need_pages <= total_pages);
	rem_pages = total_pages - need_pages;
    
	arena_avail_tree_remove(&arena->runs_avail, &chunk->map[run_ind]);
    
	/* Keep track of trailing unused pages for later use. */
	if (rem_pages > 0) {
		chunk->map[run_ind+need_pages].bits = (rem_pages <<
                                               pagesize_2pow) | (chunk->map[run_ind+need_pages].bits &
                                                                 pagesize_mask);
		chunk->map[run_ind+total_pages-1].bits = (rem_pages <<
                                                  pagesize_2pow) | (chunk->map[run_ind+total_pages-1].bits &
                                                                    pagesize_mask);
		arena_avail_tree_insert(&arena->runs_avail,
                                &chunk->map[run_ind+need_pages]);
	}
    
	for (i = 0; i < need_pages; i++) {
		/* Zero if necessary. */
		if (zero) {
			if ((chunk->map[run_ind + i].bits & CHUNK_MAP_ZEROED)
			    == 0) {
				memset((void *)((uintptr_t)chunk + ((run_ind
                                                     + i) << pagesize_2pow)), 0, pagesize);
				/* CHUNK_MAP_ZEROED is cleared below. */
			}
		}
        
		/* Update dirty page accounting. */
		if (chunk->map[run_ind + i].bits & CHUNK_MAP_DIRTY) {
			chunk->ndirty--;
			arena->ndirty--;
			/* CHUNK_MAP_DIRTY is cleared below. */
		}
        
		/* Initialize the chunk map. */
		if (large) {
			chunk->map[run_ind + i].bits = CHUNK_MAP_LARGE
            | CHUNK_MAP_ALLOCATED;
		} else {
			chunk->map[run_ind + i].bits = (size_t)run
            | CHUNK_MAP_ALLOCATED;
		}
	}
    
	/*
	 * Set the run size only in the first element for large runs.  This is
	 * primarily a debugging aid, since the lack of size info for trailing
	 * pages only matters if the application tries to operate on an
	 * interior pointer.
	 */
	if (large)
		chunk->map[run_ind].bits |= size;
    
	if (chunk->ndirty == 0 && old_ndirty > 0)
		arena_chunk_tree_dirty_remove(&arena->chunks_dirty, chunk);
}

static arena_chunk_t *
arena_chunk_alloc(arena_t *arena)
{
	arena_chunk_t *chunk;
	size_t i;
    
	if (arena->spare != NULL) {
		chunk = arena->spare;
		arena->spare = NULL;
	} else {
		chunk = (arena_chunk_t *)chunk_alloc(chunksize, true);
		if (chunk == NULL)
			return (NULL);
#ifdef MALLOC_STATS
		arena->stats.mapped += chunksize;
#endif
        
		chunk->arena = arena;
        
		/*
		 * Claim that no pages are in use, since the header is merely
		 * overhead.
		 */
		chunk->ndirty = 0;
        
		/*
		 * Initialize the map to contain one maximal free untouched run.
		 */
		for (i = 0; i < arena_chunk_header_npages; i++)
			chunk->map[i].bits = 0;
		chunk->map[i].bits = arena_maxclass | CHUNK_MAP_ZEROED;
		for (i++; i < chunk_npages-1; i++) {
			chunk->map[i].bits = CHUNK_MAP_ZEROED;
		}
		chunk->map[chunk_npages-1].bits = arena_maxclass |
        CHUNK_MAP_ZEROED;
	}
    
	/* Insert the run into the runs_avail tree. */
	arena_avail_tree_insert(&arena->runs_avail,
                            &chunk->map[arena_chunk_header_npages]);
    
	return (chunk);
}

static void
arena_chunk_dealloc(arena_t *arena, arena_chunk_t *chunk)
{
    
	if (arena->spare != NULL) {
		if (arena->spare->ndirty > 0) {
			arena_chunk_tree_dirty_remove(
                                          &chunk->arena->chunks_dirty, arena->spare);
			arena->ndirty -= arena->spare->ndirty;
		}
		chunk_dealloc((void *)arena->spare, chunksize);
#ifdef MALLOC_STATS
		arena->stats.mapped -= chunksize;
#endif
	}
    
	/*
	 * Remove run from runs_avail, regardless of whether this chunk
	 * will be cached, so that the arena does not use it.  Dirty page
	 * flushing only uses the chunks_dirty tree, so leaving this chunk in
	 * the chunks_* trees is sufficient for that purpose.
	 */
	arena_avail_tree_remove(&arena->runs_avail,
                            &chunk->map[arena_chunk_header_npages]);
    
	arena->spare = chunk;
}

static arena_run_t *
arena_run_alloc(arena_t *arena, size_t size, bool large, bool zero)
{
	arena_chunk_t *chunk;
	arena_run_t *run;
	arena_chunk_map_t *mapelm, key;
    
	assert(size <= arena_maxclass);
	assert((size & pagesize_mask) == 0);
    
	/* Search the arena's chunks for the lowest best fit. */
	key.bits = size | CHUNK_MAP_KEY;
	mapelm = arena_avail_tree_nsearch(&arena->runs_avail, &key);
	if (mapelm != NULL) {
		arena_chunk_t *run_chunk = CHUNK_ADDR2BASE(mapelm);
		size_t pageind = ((uintptr_t)mapelm - (uintptr_t)run_chunk->map)
        / sizeof(arena_chunk_map_t);
        
		run = (arena_run_t *)((uintptr_t)run_chunk + (pageind
                                                      << pagesize_2pow));
		arena_run_split(arena, run, size, large, zero);
		return (run);
	}
    
	/*
	 * No usable runs.  Create a new chunk from which to allocate the run.
	 */
	chunk = arena_chunk_alloc(arena);
	if (chunk == NULL)
		return (NULL);
	run = (arena_run_t *)((uintptr_t)chunk + (arena_chunk_header_npages <<
                                              pagesize_2pow));
	/* Update page map. */
	arena_run_split(arena, run, size, large, zero);
	return (run);
}

static void
arena_purge(arena_t *arena)
{
	arena_chunk_t *chunk;
	size_t i, npages;
#ifdef MALLOC_DEBUG
	size_t ndirty = 0;
    
	rb_foreach_begin(arena_chunk_t, link_dirty, &arena->chunks_dirty,
                     chunk) {
		ndirty += chunk->ndirty;
	} rb_foreach_end(arena_chunk_t, link_dirty, &arena->chunks_dirty, chunk)
	assert(ndirty == arena->ndirty);
#endif
	assert(arena->ndirty > opt_dirty_max);
    
#ifdef MALLOC_STATS
	arena->stats.npurge++;
#endif
    
	/*
	 * Iterate downward through chunks until enough dirty memory has been
	 * purged.  Terminate as soon as possible in order to minimize the
	 * number of system calls, even if a chunk has only been partially
	 * purged.
	 */
	while (arena->ndirty > (opt_dirty_max >> 1)) {
		chunk = arena_chunk_tree_dirty_last(&arena->chunks_dirty);
		assert(chunk != NULL);
        
		for (i = chunk_npages - 1; chunk->ndirty > 0; i--) {
			assert(i >= arena_chunk_header_npages);
            
			if (chunk->map[i].bits & CHUNK_MAP_DIRTY) {
				chunk->map[i].bits ^= CHUNK_MAP_DIRTY;
				/* Find adjacent dirty run(s). */
				for (npages = 1; i > arena_chunk_header_npages
                     && (chunk->map[i - 1].bits &
                         CHUNK_MAP_DIRTY); npages++) {
                         i--;
                         chunk->map[i].bits ^= CHUNK_MAP_DIRTY;
                     }
				chunk->ndirty -= npages;
				arena->ndirty -= npages;
				
#ifdef MALLOC_STATS
				arena->stats.nmadvise++;
				arena->stats.purged += npages;
#endif
				if (arena->ndirty <= (opt_dirty_max >> 1))
					break;
			}
		}
        
		if (chunk->ndirty == 0) {
			arena_chunk_tree_dirty_remove(&arena->chunks_dirty,
                                          chunk);
		}
	}
}

static void
arena_run_dalloc(arena_t *arena, arena_run_t *run, bool dirty)
{
	arena_chunk_t *chunk;
	size_t size, run_ind, run_pages;
    
	chunk = (arena_chunk_t *)CHUNK_ADDR2BASE(run);
	run_ind = (size_t)(((uintptr_t)run - (uintptr_t)chunk)
                       >> pagesize_2pow);
	assert(run_ind >= arena_chunk_header_npages);
	assert(run_ind < chunk_npages);
	if ((chunk->map[run_ind].bits & CHUNK_MAP_LARGE) != 0)
		size = chunk->map[run_ind].bits & ~pagesize_mask;
	else
		size = run->bin->run_size;
	run_pages = (size >> pagesize_2pow);
    
	/* Mark pages as unallocated in the chunk map. */
	if (dirty) {
		size_t i;
        
		for (i = 0; i < run_pages; i++) {
			assert((chunk->map[run_ind + i].bits & CHUNK_MAP_DIRTY)
                   == 0);
			chunk->map[run_ind + i].bits = CHUNK_MAP_DIRTY;
		}
        
		if (chunk->ndirty == 0) {
			arena_chunk_tree_dirty_insert(&arena->chunks_dirty,
                                          chunk);
		}
		chunk->ndirty += run_pages;
		arena->ndirty += run_pages;
	} else {
		size_t i;
        
		for (i = 0; i < run_pages; i++) {
			chunk->map[run_ind + i].bits &= ~(CHUNK_MAP_LARGE |
                                              CHUNK_MAP_ALLOCATED);
		}
	}
	chunk->map[run_ind].bits = size | (chunk->map[run_ind].bits &
                                       pagesize_mask);
	chunk->map[run_ind+run_pages-1].bits = size |
    (chunk->map[run_ind+run_pages-1].bits & pagesize_mask);
    
	/* Try to coalesce forward. */
	if (run_ind + run_pages < chunk_npages &&
	    (chunk->map[run_ind+run_pages].bits & CHUNK_MAP_ALLOCATED) == 0) {
		size_t nrun_size = chunk->map[run_ind+run_pages].bits &
        ~pagesize_mask;
        
		/*
		 * Remove successor from runs_avail; the coalesced run is
		 * inserted later.
		 */
		arena_avail_tree_remove(&arena->runs_avail,
                                &chunk->map[run_ind+run_pages]);
        
		size += nrun_size;
		run_pages = size >> pagesize_2pow;
        
		assert((chunk->map[run_ind+run_pages-1].bits & ~pagesize_mask)
               == nrun_size);
		chunk->map[run_ind].bits = size | (chunk->map[run_ind].bits &
                                           pagesize_mask);
		chunk->map[run_ind+run_pages-1].bits = size |
        (chunk->map[run_ind+run_pages-1].bits & pagesize_mask);
	}
    
	/* Try to coalesce backward. */
	if (run_ind > arena_chunk_header_npages && (chunk->map[run_ind-1].bits &
                                                CHUNK_MAP_ALLOCATED) == 0) {
		size_t prun_size = chunk->map[run_ind-1].bits & ~pagesize_mask;
        
		run_ind -= prun_size >> pagesize_2pow;
        
		/*
		 * Remove predecessor from runs_avail; the coalesced run is
		 * inserted later.
		 */
		arena_avail_tree_remove(&arena->runs_avail,
                                &chunk->map[run_ind]);
        
		size += prun_size;
		run_pages = size >> pagesize_2pow;
        
		assert((chunk->map[run_ind].bits & ~pagesize_mask) ==
               prun_size);
		chunk->map[run_ind].bits = size | (chunk->map[run_ind].bits &
                                           pagesize_mask);
		chunk->map[run_ind+run_pages-1].bits = size |
        (chunk->map[run_ind+run_pages-1].bits & pagesize_mask);
	}
    
	/* Insert into runs_avail, now that coalescing is complete. */
	arena_avail_tree_insert(&arena->runs_avail, &chunk->map[run_ind]);
    
	/* Deallocate chunk if it is now completely unused. */
	if ((chunk->map[arena_chunk_header_npages].bits & (~pagesize_mask |
                                                       CHUNK_MAP_ALLOCATED)) == arena_maxclass)
		arena_chunk_dealloc(arena, chunk);
    
	/* Enforce opt_dirty_max. */
	if (arena->ndirty > opt_dirty_max)
		arena_purge(arena);
}

static void
arena_run_trim_head(arena_t *arena, arena_chunk_t *chunk, arena_run_t *run,
                    size_t oldsize, size_t newsize)
{
	size_t pageind = ((uintptr_t)run - (uintptr_t)chunk) >> pagesize_2pow;
	size_t head_npages = (oldsize - newsize) >> pagesize_2pow;
    
	assert(oldsize > newsize);
    
	/*
	 * Update the chunk map so that arena_run_dalloc() can treat the
	 * leading run as separately allocated.
	 */
	chunk->map[pageind].bits = (oldsize - newsize) | CHUNK_MAP_LARGE |
    CHUNK_MAP_ALLOCATED;
	chunk->map[pageind+head_npages].bits = newsize | CHUNK_MAP_LARGE |
    CHUNK_MAP_ALLOCATED;
    
	arena_run_dalloc(arena, run, false);
}

static void
arena_run_trim_tail(arena_t *arena, arena_chunk_t *chunk, arena_run_t *run,
                    size_t oldsize, size_t newsize, bool dirty)
{
	size_t pageind = ((uintptr_t)run - (uintptr_t)chunk) >> pagesize_2pow;
	size_t npages = newsize >> pagesize_2pow;
    
	assert(oldsize > newsize);
    
	/*
	 * Update the chunk map so that arena_run_dalloc() can treat the
	 * trailing run as separately allocated.
	 */
	chunk->map[pageind].bits = newsize | CHUNK_MAP_LARGE |
    CHUNK_MAP_ALLOCATED;
	chunk->map[pageind+npages].bits = (oldsize - newsize) | CHUNK_MAP_LARGE
    | CHUNK_MAP_ALLOCATED;
    
	arena_run_dalloc(arena, (arena_run_t *)((uintptr_t)run + newsize),
                     dirty);
}

static arena_run_t *
arena_bin_nonfull_run_get(arena_t *arena, arena_bin_t *bin)
{
	arena_chunk_map_t *mapelm;
	arena_run_t *run;
	unsigned i, remainder;
    
	/* Look for a usable run. */
	mapelm = arena_run_tree_first(&bin->runs);
	if (mapelm != NULL) {
		/* run is guaranteed to have available space. */
		arena_run_tree_remove(&bin->runs, mapelm);
		run = (arena_run_t *)(mapelm->bits & ~pagesize_mask);
#ifdef MALLOC_STATS
		bin->stats.reruns++;
#endif
		return (run);
	}
	/* No existing runs have any space available. */
    
	/* Allocate a new run. */
	run = arena_run_alloc(arena, bin->run_size, false, false);
	if (run == NULL)
		return (NULL);
    
	/* Initialize run internals. */
	run->bin = bin;
    
	for (i = 0; i < bin->regs_mask_nelms - 1; i++)
		run->regs_mask[i] = UINT_MAX;
	remainder = bin->nregs & ((1U << (SIZEOF_INT_2POW + 3)) - 1);
	if (remainder == 0)
		run->regs_mask[i] = UINT_MAX;
	else {
		/* The last element has spare bits that need to be unset. */
		run->regs_mask[i] = (UINT_MAX >> ((1U << (SIZEOF_INT_2POW + 3))
                                          - remainder));
	}
    
	run->regs_minelm = 0;
    
	run->nfree = bin->nregs;
#ifdef MALLOC_DEBUG
	run->magic = ARENA_RUN_MAGIC;
#endif
    
#ifdef MALLOC_STATS
	bin->stats.nruns++;
	bin->stats.curruns++;
	if (bin->stats.curruns > bin->stats.highruns)
		bin->stats.highruns = bin->stats.curruns;
#endif
	return (run);
}

/* bin->runcur must have space available before this function is called. */
static inline void *
arena_bin_malloc_easy(arena_t *arena, arena_bin_t *bin, arena_run_t *run)
{
	void *ret;
    
	assert(run->magic == ARENA_RUN_MAGIC);
	assert(run->nfree > 0);
    
	ret = arena_run_reg_alloc(run, bin);
	assert(ret != NULL);
	run->nfree--;
    
	return (ret);
}

/* Re-fill bin->runcur, then call arena_bin_malloc_easy(). */
static void *
arena_bin_malloc_hard(arena_t *arena, arena_bin_t *bin)
{
    
	bin->runcur = arena_bin_nonfull_run_get(arena, bin);
	if (bin->runcur == NULL)
		return (NULL);
	assert(bin->runcur->magic == ARENA_RUN_MAGIC);
	assert(bin->runcur->nfree > 0);
    
	return (arena_bin_malloc_easy(arena, bin, bin->runcur));
}

/*
 * Calculate bin->run_size such that it meets the following constraints:
 *
 *   *) bin->run_size >= min_run_size
 *   *) bin->run_size <= arena_maxclass
 *   *) bin->run_size <= RUN_MAX_SMALL
 *   *) run header overhead <= RUN_MAX_OVRHD (or header overhead relaxed).
 *
 * bin->nregs, bin->regs_mask_nelms, and bin->reg0_offset are
 * also calculated here, since these settings are all interdependent.
 */
static size_t
arena_bin_run_size_calc(arena_bin_t *bin, size_t min_run_size)
{
	size_t try_run_size, good_run_size;
	unsigned good_nregs, good_mask_nelms, good_reg0_offset;
	unsigned try_nregs, try_mask_nelms, try_reg0_offset;
    
	assert(min_run_size >= pagesize);
	assert(min_run_size <= arena_maxclass);
	assert(min_run_size <= RUN_MAX_SMALL);
    
	/*
	 * Calculate known-valid settings before entering the run_size
	 * expansion loop, so that the first part of the loop always copies
	 * valid settings.
	 *
	 * The do..while loop iteratively reduces the number of regions until
	 * the run header and the regions no longer overlap.  A closed formula
	 * would be quite messy, since there is an interdependency between the
	 * header's mask length and the number of regions.
	 */
	try_run_size = min_run_size;
	try_nregs = ((try_run_size - sizeof(arena_run_t)) / bin->reg_size)
    + 1; /* Counter-act try_nregs-- in loop. */
	do {
		try_nregs--;
		try_mask_nelms = (try_nregs >> (SIZEOF_INT_2POW + 3)) +
        ((try_nregs & ((1U << (SIZEOF_INT_2POW + 3)) - 1)) ? 1 : 0);
		try_reg0_offset = try_run_size - (try_nregs * bin->reg_size);
	} while (sizeof(arena_run_t) + (sizeof(unsigned) * (try_mask_nelms - 1))
             > try_reg0_offset);
    
	/* run_size expansion loop. */
	do {
		/*
		 * Copy valid settings before trying more aggressive settings.
		 */
		good_run_size = try_run_size;
		good_nregs = try_nregs;
		good_mask_nelms = try_mask_nelms;
		good_reg0_offset = try_reg0_offset;
        
		/* Try more aggressive settings. */
		try_run_size += pagesize;
		try_nregs = ((try_run_size - sizeof(arena_run_t)) /
                     bin->reg_size) + 1; /* Counter-act try_nregs-- in loop. */
		do {
			try_nregs--;
			try_mask_nelms = (try_nregs >> (SIZEOF_INT_2POW + 3)) +
            ((try_nregs & ((1U << (SIZEOF_INT_2POW + 3)) - 1)) ?
             1 : 0);
			try_reg0_offset = try_run_size - (try_nregs *
                                              bin->reg_size);
		} while (sizeof(arena_run_t) + (sizeof(unsigned) *
                                        (try_mask_nelms - 1)) > try_reg0_offset);
	} while (try_run_size <= arena_maxclass && try_run_size <= RUN_MAX_SMALL
             && RUN_MAX_OVRHD * (bin->reg_size << 3) > RUN_MAX_OVRHD_RELAX
             && (try_reg0_offset << RUN_BFP) > RUN_MAX_OVRHD * try_run_size);
    
	assert(sizeof(arena_run_t) + (sizeof(unsigned) * (good_mask_nelms - 1))
           <= good_reg0_offset);
	assert((good_mask_nelms << (SIZEOF_INT_2POW + 3)) >= good_nregs);
    
	/* Copy final settings. */
	bin->run_size = good_run_size;
	bin->nregs = good_nregs;
	bin->regs_mask_nelms = good_mask_nelms;
	bin->reg0_offset = good_reg0_offset;
    
	return (good_run_size);
}

static inline void *
arena_malloc_small(arena_t *arena, size_t size, bool zero)
{
	void *ret;
	arena_bin_t *bin;
	arena_run_t *run;
	size_t binind;
    
	binind = size2bin[size];
	assert(binind < nbins);
	bin = &arena->bins[binind];
	size = bin->reg_size;
	
	if ((run = bin->runcur) != NULL && run->nfree > 0)
		ret = arena_bin_malloc_easy(arena, bin, run);
	else
		ret = arena_bin_malloc_hard(arena, bin);
    
	if (ret == NULL) {
		return (NULL);
	}
    
#ifdef MALLOC_STATS
	bin->stats.nrequests++;
	arena->stats.nmalloc_small++;
	arena->stats.allocated_small += size;
#endif
    
	if (zero == false) {
		if (opt_junk)
			memset(ret, 0xa5, size);
		else if (opt_zero)
			memset(ret, 0, size);
	} else
		memset(ret, 0, size);
    
	return (ret);
}

static void *
arena_malloc_large(arena_t *arena, size_t size, bool zero)
{
	void *ret;
    
	/* Large allocation. */
	size = PAGE_CEILING(size);
	
	ret = (void *)arena_run_alloc(arena, size, true, zero);
	if (ret == NULL) {
		return (NULL);
	}
#ifdef MALLOC_STATS
	arena->stats.nmalloc_large++;
	arena->stats.allocated_large += size;
#endif
    
	if (zero == false) {
		if (opt_junk)
			memset(ret, 0xa5, size);
		else if (opt_zero)
			memset(ret, 0, size);
	}
    
	return (ret);
}

static inline void *
arena_malloc(arena_t *arena, size_t size, bool zero)
{
    
	assert(arena != NULL);
	assert(arena->magic == ARENA_MAGIC);
	assert(size != 0);
	assert(QUANTUM_CEILING(size) <= arena_maxclass);
    
	if (size <= bin_maxclass) {
		return (arena_malloc_small(arena, size, zero));
	} else
		return (arena_malloc_large(arena, size, zero));
}

static inline void *
imalloc(size_t size)
{
    
	assert(size != 0);
    
	if (size <= arena_maxclass)
		return (arena_malloc(choose_arena(), size, false));
	else
		return (huge_malloc(size, false));
}

static inline void *
icalloc(size_t size)
{
    
	if (size <= arena_maxclass)
		return (arena_malloc(choose_arena(), size, true));
	else
		return (huge_malloc(size, true));
}

/* Only handles large allocations that require more than page alignment. */
static void *
arena_palloc(arena_t *arena, size_t alignment, size_t size, size_t alloc_size)
{
	void *ret;
	size_t offset;
	arena_chunk_t *chunk;
    
	assert((size & pagesize_mask) == 0);
	assert((alignment & pagesize_mask) == 0);    
	
	ret = (void *)arena_run_alloc(arena, alloc_size, true, false);
	if (ret == NULL) {
		return (NULL);
	}
    
	chunk = (arena_chunk_t *)CHUNK_ADDR2BASE(ret);
    
	offset = (uintptr_t)ret & (alignment - 1);
	assert((offset & pagesize_mask) == 0);
	assert(offset < alloc_size);
	if (offset == 0)
		arena_run_trim_tail(arena, chunk, ret, alloc_size, size, false);
	else {
		size_t leadsize, trailsize;
        
		leadsize = alignment - offset;
		if (leadsize > 0) {
			arena_run_trim_head(arena, chunk, ret, alloc_size,
                                alloc_size - leadsize);
			ret = (void *)((uintptr_t)ret + leadsize);
		}
        
		trailsize = alloc_size - leadsize - size;
		if (trailsize != 0) {
			/* Trim trailing space. */
			assert(trailsize < alloc_size);
			arena_run_trim_tail(arena, chunk, ret, size + trailsize,
                                size, false);
		}
	}
    
#ifdef MALLOC_STATS
	arena->stats.nmalloc_large++;
	arena->stats.allocated_large += size;
#endif
    
	if (opt_junk)
		memset(ret, 0xa5, size);
	else if (opt_zero)
		memset(ret, 0, size);
	return (ret);
}

static inline void *
ipalloc(size_t alignment, size_t size)
{
	void *ret;
	size_t ceil_size;
    
	/*
	 * Round size up to the nearest multiple of alignment.
	 *
	 * This done, we can take advantage of the fact that for each small
	 * size class, every object is aligned at the smallest power of two
	 * that is non-zero in the base two representation of the size.  For
	 * example:
	 *
	 *   Size |   Base 2 | Minimum alignment
	 *   -----+----------+------------------
	 *     96 |  1100000 |  32
	 *    144 | 10100000 |  32
	 *    192 | 11000000 |  64
	 *
	 * Depending on runtime settings, it is possible that arena_malloc()
	 * will further round up to a power of two, but that never causes
	 * correctness issues.
	 */
	ceil_size = (size + (alignment - 1)) & (-alignment);
	/*
	 * (ceil_size < size) protects against the combination of maximal
	 * alignment and size greater than maximal alignment.
	 */
	if (ceil_size < size) {
		/* size_t overflow. */
		return (NULL);
	}
    
	if (ceil_size <= pagesize || (alignment <= pagesize
                                  && ceil_size <= arena_maxclass))
		ret = arena_malloc(choose_arena(), ceil_size, false);
	else {
		size_t run_size;
        
		/*
		 * We can't achieve subpage alignment, so round up alignment
		 * permanently; it makes later calculations simpler.
		 */
		alignment = PAGE_CEILING(alignment);
		ceil_size = PAGE_CEILING(size);
		/*
		 * (ceil_size < size) protects against very large sizes within
		 * pagesize of SIZE_T_MAX.
		 *
		 * (ceil_size + alignment < ceil_size) protects against the
		 * combination of maximal alignment and ceil_size large enough
		 * to cause overflow.  This is similar to the first overflow
		 * check above, but it needs to be repeated due to the new
		 * ceil_size value, which may now be *equal* to maximal
		 * alignment, whereas before we only detected overflow if the
		 * original size was *greater* than maximal alignment.
		 */
		if (ceil_size < size || ceil_size + alignment < ceil_size) {
			/* size_t overflow. */
			return (NULL);
		}
        
		/*
		 * Calculate the size of the over-size run that arena_palloc()
		 * would need to allocate in order to guarantee the alignment.
		 */
		if (ceil_size >= alignment)
			run_size = ceil_size + alignment - pagesize;
		else {
			/*
			 * It is possible that (alignment << 1) will cause
			 * overflow, but it doesn't matter because we also
			 * subtract pagesize, which in the case of overflow
			 * leaves us with a very large run_size.  That causes
			 * the first conditional below to fail, which means
			 * that the bogus run_size value never gets used for
			 * anything important.
			 */
			run_size = (alignment << 1) - pagesize;
		}
        
		if (run_size <= arena_maxclass) {
			ret = arena_palloc(choose_arena(), alignment, ceil_size,
                               run_size);
		} else if (alignment <= chunksize)
			ret = huge_malloc(ceil_size, false);
		else
			ret = huge_palloc(alignment, ceil_size);
	}
    
	assert(((uintptr_t)ret & (alignment - 1)) == 0);
	return (ret);
}

/* Return the size of the allocation pointed to by ptr. */
static size_t
arena_salloc(const void *ptr)
{
	size_t ret;
	arena_chunk_t *chunk;
	size_t pageind, mapbits;
    
	assert(ptr != NULL);
	assert(CHUNK_ADDR2BASE(ptr) != ptr);
    
	chunk = (arena_chunk_t *)CHUNK_ADDR2BASE(ptr);
	pageind = (((uintptr_t)ptr - (uintptr_t)chunk) >> pagesize_2pow);
	mapbits = chunk->map[pageind].bits;
	assert((mapbits & CHUNK_MAP_ALLOCATED) != 0);
	if ((mapbits & CHUNK_MAP_LARGE) == 0) {
		arena_run_t *run = (arena_run_t *)(mapbits & ~pagesize_mask);
		assert(run->magic == ARENA_RUN_MAGIC);
		ret = run->bin->reg_size;
	} else {
		ret = mapbits & ~pagesize_mask;
		assert(ret != 0);
	}
    
	return (ret);
}

static inline size_t
isalloc(const void *ptr)
{
	size_t ret;
	arena_chunk_t *chunk;
    
	assert(ptr != NULL);
    
	chunk = (arena_chunk_t *)CHUNK_ADDR2BASE(ptr);
	if (chunk != ptr) {
		/* Region. */
		assert(chunk->arena->magic == ARENA_MAGIC);
        
		ret = arena_salloc(ptr);
	} else {
		extent_node_t *node, key;
        
		/* Chunk (huge allocation). */
        
        
		/* Extract from tree of huge allocations. */
		key.addr = __DECONST(void *, ptr);
		node = extent_tree_ad_search(&huge, &key);
		assert(node != NULL);
        
		ret = node->size;
        
	}
    
	return (ret);
}

static inline void
arena_dalloc_small(arena_t *arena, arena_chunk_t *chunk, void *ptr,
                   arena_chunk_map_t *mapelm)
{
	arena_run_t *run;
	arena_bin_t *bin;
	size_t size;
    
	run = (arena_run_t *)(mapelm->bits & ~pagesize_mask);
	assert(run->magic == ARENA_RUN_MAGIC);
	bin = run->bin;
	size = bin->reg_size;
    
	if (opt_junk)
		memset(ptr, 0x5a, size);
    
	arena_run_reg_dalloc(run, bin, ptr, size);
	run->nfree++;
    
	if (run->nfree == bin->nregs) {
		/* Deallocate run. */
		if (run == bin->runcur)
			bin->runcur = NULL;
		else if (bin->nregs != 1) {
			size_t run_pageind = (((uintptr_t)run -
                                   (uintptr_t)chunk)) >> pagesize_2pow;
			arena_chunk_map_t *run_mapelm =
            &chunk->map[run_pageind];
			/*
			 * This block's conditional is necessary because if the
			 * run only contains one region, then it never gets
			 * inserted into the non-full runs tree.
			 */
			arena_run_tree_remove(&bin->runs, run_mapelm);
		}
#ifdef MALLOC_DEBUG
		run->magic = 0;
#endif
		arena_run_dalloc(arena, run, true);
#ifdef MALLOC_STATS
		bin->stats.curruns--;
#endif
	} else if (run->nfree == 1 && run != bin->runcur) {
		/*
		 * Make sure that bin->runcur always refers to the lowest
		 * non-full run, if one exists.
		 */
		if (bin->runcur == NULL)
			bin->runcur = run;
		else if ((uintptr_t)run < (uintptr_t)bin->runcur) {
			/* Switch runcur. */
			if (bin->runcur->nfree > 0) {
				arena_chunk_t *runcur_chunk =
                CHUNK_ADDR2BASE(bin->runcur);
				size_t runcur_pageind =
                (((uintptr_t)bin->runcur -
                  (uintptr_t)runcur_chunk)) >> pagesize_2pow;
				arena_chunk_map_t *runcur_mapelm =
                &runcur_chunk->map[runcur_pageind];
                
				/* Insert runcur. */
				arena_run_tree_insert(&bin->runs,
                                      runcur_mapelm);
			}
			bin->runcur = run;
		} else {
			size_t run_pageind = (((uintptr_t)run -
                                   (uintptr_t)chunk)) >> pagesize_2pow;
			arena_chunk_map_t *run_mapelm =
            &chunk->map[run_pageind];
            
			assert(arena_run_tree_search(&bin->runs, run_mapelm) ==
                   NULL);
			arena_run_tree_insert(&bin->runs, run_mapelm);
		}
	}
#ifdef MALLOC_STATS
	arena->stats.allocated_small -= size;
	arena->stats.ndalloc_small++;
#endif
}

static void
arena_dalloc_large(arena_t *arena, arena_chunk_t *chunk, void *ptr)
{
	/* Large allocation. */
    
#ifndef MALLOC_STATS
	if (opt_junk)
#endif
	{
		size_t pageind = ((uintptr_t)ptr - (uintptr_t)chunk) >>
        pagesize_2pow;
		size_t size = chunk->map[pageind].bits & ~pagesize_mask;
        
#ifdef MALLOC_STATS
		if (opt_junk)
#endif
			memset(ptr, 0x5a, size);
#ifdef MALLOC_STATS
		arena->stats.allocated_large -= size;
#endif
	}
#ifdef MALLOC_STATS
	arena->stats.ndalloc_large++;
#endif
    
	arena_run_dalloc(arena, (arena_run_t *)ptr, true);
}

static inline void
arena_dalloc(arena_t *arena, arena_chunk_t *chunk, void *ptr)
{
	size_t pageind;
	arena_chunk_map_t *mapelm;
    
	assert(arena != NULL);
	assert(arena->magic == ARENA_MAGIC);
	assert(chunk->arena == arena);
	assert(ptr != NULL);
	assert(CHUNK_ADDR2BASE(ptr) != ptr);
    
	pageind = (((uintptr_t)ptr - (uintptr_t)chunk) >> pagesize_2pow);
	mapelm = &chunk->map[pageind];
	assert((mapelm->bits & CHUNK_MAP_ALLOCATED) != 0);
	if ((mapelm->bits & CHUNK_MAP_LARGE) == 0) {
		/* Small allocation. */
		arena_dalloc_small(arena, chunk, ptr, mapelm);
	} else
		arena_dalloc_large(arena, chunk, ptr);
}

static inline void
idalloc(void *ptr)
{
	arena_chunk_t *chunk;
    
	assert(ptr != NULL);
    
	chunk = (arena_chunk_t *)CHUNK_ADDR2BASE(ptr);
	if (chunk != ptr)
		arena_dalloc(chunk->arena, chunk, ptr);
	else
		huge_dalloc(ptr);
}

static void
arena_ralloc_large_shrink(arena_t *arena, arena_chunk_t *chunk, void *ptr,
                          size_t size, size_t oldsize)
{
    
	assert(size < oldsize);
    
	/*
	 * Shrink the run, and make trailing pages available for other
	 * allocations.
	 */
	
	arena_run_trim_tail(arena, chunk, (arena_run_t *)ptr, oldsize, size,
                        true);
#ifdef MALLOC_STATS
	arena->stats.allocated_large -= oldsize - size;
#endif
}

static bool
arena_ralloc_large_grow(arena_t *arena, arena_chunk_t *chunk, void *ptr,
                        size_t size, size_t oldsize)
{
	size_t pageind = ((uintptr_t)ptr - (uintptr_t)chunk) >> pagesize_2pow;
	size_t npages = oldsize >> pagesize_2pow;
    
	assert(oldsize == (chunk->map[pageind].bits & ~pagesize_mask));
    
	/* Try to extend the run. */
	assert(size > oldsize);
	
	if (pageind + npages < chunk_npages && (chunk->map[pageind+npages].bits
                                            & CHUNK_MAP_ALLOCATED) == 0 && (chunk->map[pageind+npages].bits &
                                                                            ~pagesize_mask) >= size - oldsize) {
		/*
		 * The next run is available and sufficiently large.  Split the
		 * following run, then merge the first part with the existing
		 * allocation.
		 */
		arena_run_split(arena, (arena_run_t *)((uintptr_t)chunk +
                                               ((pageind+npages) << pagesize_2pow)), size - oldsize, true,
                        false);
        
		chunk->map[pageind].bits = size | CHUNK_MAP_LARGE |
        CHUNK_MAP_ALLOCATED;
		chunk->map[pageind+npages].bits = CHUNK_MAP_LARGE |
        CHUNK_MAP_ALLOCATED;
        
#ifdef MALLOC_STATS
		arena->stats.allocated_large += size - oldsize;
#endif
		return (false);
	}
    
	return (true);
}

/*
 * Try to resize a large allocation, in order to avoid copying.  This will
 * always fail if growing an object, and the following run is already in use.
 */
static bool
arena_ralloc_large(void *ptr, size_t size, size_t oldsize)
{
	size_t psize;
    
	psize = PAGE_CEILING(size);
	if (psize == oldsize) {
		/* Same size class. */
		if (opt_junk && size < oldsize) {
			memset((void *)((uintptr_t)ptr + size), 0x5a, oldsize -
                   size);
		}
		return (false);
	} else {
		arena_chunk_t *chunk;
		arena_t *arena;
        
		chunk = (arena_chunk_t *)CHUNK_ADDR2BASE(ptr);
		arena = chunk->arena;
		assert(arena->magic == ARENA_MAGIC);
        
		if (psize < oldsize) {
			/* Fill before shrinking in order avoid a race. */
			if (opt_junk) {
				memset((void *)((uintptr_t)ptr + size), 0x5a,
                       oldsize - size);
			}
			arena_ralloc_large_shrink(arena, chunk, ptr, psize,
                                      oldsize);
			return (false);
		} else {
			bool ret = arena_ralloc_large_grow(arena, chunk, ptr,
                                               psize, oldsize);
			if (ret == false && opt_zero) {
				memset((void *)((uintptr_t)ptr + oldsize), 0,
                       size - oldsize);
			}
			return (ret);
		}
	}
}

static void *
arena_ralloc(void *ptr, size_t size, size_t oldsize)
{
	void *ret;
	size_t copysize;
    
	/* Try to avoid moving the allocation. */
	if (size <= bin_maxclass) {
		if (oldsize <= bin_maxclass && size2bin[size] ==
		    size2bin[oldsize])
			goto IN_PLACE;
	} else {
		if (oldsize > bin_maxclass && oldsize <= arena_maxclass) {
			assert(size > bin_maxclass);
			if (arena_ralloc_large(ptr, size, oldsize) == false)
				return (ptr);
		}
	}
    
	/*
	 * If we get here, then size and oldsize are different enough that we
	 * need to move the object.  In that case, fall back to allocating new
	 * space and copying.
	 */
	ret = arena_malloc(choose_arena(), size, false);
	if (ret == NULL)
		return (NULL);
    
	/* Junk/zero-filling were already done by arena_malloc(). */
	copysize = (size < oldsize) ? size : oldsize;
	memcpy(ret, ptr, copysize);
	idalloc(ptr);
	return (ret);
IN_PLACE:
	if (opt_junk && size < oldsize)
		memset((void *)((uintptr_t)ptr + size), 0x5a, oldsize - size);
	else if (opt_zero && size > oldsize)
		memset((void *)((uintptr_t)ptr + oldsize), 0, size - oldsize);
	return (ptr);
}

static inline void *
iralloc(void *ptr, size_t size)
{
	size_t oldsize;
    
	assert(ptr != NULL);
	assert(size != 0);
    
	oldsize = isalloc(ptr);
    
	if (size <= arena_maxclass)
		return (arena_ralloc(ptr, size, oldsize));
	else
		return (huge_ralloc(ptr, size, oldsize));
}

static bool
arena_new(arena_t *arena)
{
	unsigned i;
	arena_bin_t *bin;
	size_t prev_run_size;
#ifdef MALLOC_STATS
	memset(&arena->stats, 0, sizeof(arena_stats_t));
#endif
    
	/* Initialize chunks. */
	arena_chunk_tree_dirty_new(&arena->chunks_dirty);
	arena->spare = NULL;
    
	arena->ndirty = 0;
    
	arena_avail_tree_new(&arena->runs_avail);
    
    
	/* Initialize bins. */
	prev_run_size = pagesize;
    
	i = 0;
#ifdef MALLOC_TINY
	/* (2^n)-spaced tiny bins. */
	for (; i < ntbins; i++) {
		bin = &arena->bins[i];
		bin->runcur = NULL;
		arena_run_tree_new(&bin->runs);
        
		bin->reg_size = (1U << (TINY_MIN_2POW + i));
        
		prev_run_size = arena_bin_run_size_calc(bin, prev_run_size);
        
#ifdef MALLOC_STATS
		memset(&bin->stats, 0, sizeof(malloc_bin_stats_t));
#endif
	}
#endif
    
	/* Quantum-spaced bins. */
	for (; i < ntbins + nqbins; i++) {
		bin = &arena->bins[i];
		bin->runcur = NULL;
		arena_run_tree_new(&bin->runs);
        
		bin->reg_size = (i - ntbins + 1) << QUANTUM_2POW;
        
		prev_run_size = arena_bin_run_size_calc(bin, prev_run_size);
        
#ifdef MALLOC_STATS
		memset(&bin->stats, 0, sizeof(malloc_bin_stats_t));
#endif
	}
    
	/* Cacheline-spaced bins. */
	for (; i < ntbins + nqbins + ncbins; i++) {
		bin = &arena->bins[i];
		bin->runcur = NULL;
		arena_run_tree_new(&bin->runs);
        
		bin->reg_size = cspace_min + ((i - (ntbins + nqbins)) <<
                                      CACHELINE_2POW);
        
		prev_run_size = arena_bin_run_size_calc(bin, prev_run_size);
        
#ifdef MALLOC_STATS
		memset(&bin->stats, 0, sizeof(malloc_bin_stats_t));
#endif
	}
    
	/* Subpage-spaced bins. */
	for (; i < nbins; i++) {
		bin = &arena->bins[i];
		bin->runcur = NULL;
		arena_run_tree_new(&bin->runs);
        
		bin->reg_size = sspace_min + ((i - (ntbins + nqbins + ncbins))
                                      << SUBPAGE_2POW);
        
		prev_run_size = arena_bin_run_size_calc(bin, prev_run_size);
        
#ifdef MALLOC_STATS
		memset(&bin->stats, 0, sizeof(malloc_bin_stats_t));
#endif
	}
    
#ifdef MALLOC_DEBUG
	arena->magic = ARENA_MAGIC;
#endif
    
	return (false);
}

/* Create a new arena and insert it into the arenas array at index ind. */
static arena_t *
arenas_extend(unsigned ind)
{
	arena_t *ret;
    
	/* Allocate enough space for trailing bins. */
	ret = (arena_t *)base_alloc(sizeof(arena_t)
                                + (sizeof(arena_bin_t) * (nbins - 1)));
	if (ret != NULL && arena_new(ret) == false) {
		arenas[ind] = ret;
		return (ret);
	}
	/* Only reached if there is an OOM error. */
    
	/*
	 * OOM here is quite inconvenient to propagate, since dealing with it
	 * would require a check for failure in the fast path.  Instead, punt
	 * by using arenas[0].  In practice, this is an extremely unlikely
	 * failure.
	 */
	_malloc_message(_getprogname(),
                    ": (malloc) Error initializing arena\n", "", "");
	if (opt_abort)
		abort();
    
	return (arenas[0]);
}

/*
 * End arena.
 */
/******************************************************************************/
/*
 * Begin general internal functions.
 */

static void *
huge_malloc(size_t size, bool zero)
{
	void *ret;
	size_t csize;
	extent_node_t *node;
    
	/* Allocate one or more contiguous chunks for this request. */
    
	csize = CHUNK_CEILING(size);
	if (csize == 0) {
		/* size is large enough to cause size_t wrap-around. */
		return (NULL);
	}
    
	/* Allocate an extent node with which to track the chunk. */
	node = base_node_alloc();
	if (node == NULL)
		return (NULL);
    
	ret = chunk_alloc(csize, zero);
	if (ret == NULL) {
		base_node_dealloc(node);
		return (NULL);
	}
    
	/* Insert node into huge. */
	node->addr = ret;
	node->size = csize;
    
	extent_tree_ad_insert(&huge, node);
#ifdef MALLOC_STATS
	huge_nmalloc++;
	huge_allocated += csize;
#endif
    
	if (zero == false) {
		if (opt_junk)
			memset(ret, 0xa5, csize);
		else if (opt_zero)
			memset(ret, 0, csize);
	}
    
	return (ret);
}

/* Only handles large allocations that require more than chunk alignment. */
static void *
huge_palloc(size_t alignment, size_t size)
{
	void *ret;
	size_t alloc_size, chunk_size, offset;
	extent_node_t *node;
    
	/*
	 * This allocation requires alignment that is even larger than chunk
	 * alignment.  This means that huge_malloc() isn't good enough.
	 *
	 * Allocate almost twice as many chunks as are demanded by the size or
	 * alignment, in order to assure the alignment can be achieved, then
	 * unmap leading and trailing chunks.
	 */
	assert(alignment >= chunksize);
    
	chunk_size = CHUNK_CEILING(size);
    
	if (size >= alignment)
		alloc_size = chunk_size + alignment - chunksize;
	else
		alloc_size = (alignment << 1) - chunksize;
    
	/* Allocate an extent node with which to track the chunk. */
	node = base_node_alloc();
	if (node == NULL)
		return (NULL);
    
	ret = chunk_alloc(alloc_size, false);
	if (ret == NULL) {
		base_node_dealloc(node);
		return (NULL);
	}
    
	offset = (uintptr_t)ret & (alignment - 1);
	assert((offset & chunksize_mask) == 0);
	assert(offset < alloc_size);
	if (offset == 0) {
		/* Trim trailing space. */
		chunk_dealloc((void *)((uintptr_t)ret + chunk_size), alloc_size
                      - chunk_size);
	} else {
		size_t trailsize;
        
		/* Trim leading space. */
		chunk_dealloc(ret, alignment - offset);
        
		ret = (void *)((uintptr_t)ret + (alignment - offset));
        
		trailsize = alloc_size - (alignment - offset) - chunk_size;
		if (trailsize != 0) {
		    /* Trim trailing space. */
		    assert(trailsize < alloc_size);
		    chunk_dealloc((void *)((uintptr_t)ret + chunk_size),
                          trailsize);
		}
	}
    
	/* Insert node into huge. */
	node->addr = ret;
	node->size = chunk_size;
    
	extent_tree_ad_insert(&huge, node);
#ifdef MALLOC_STATS
	huge_nmalloc++;
	huge_allocated += chunk_size;
#endif
    
	if (opt_junk)
		memset(ret, 0xa5, chunk_size);
	else if (opt_zero)
		memset(ret, 0, chunk_size);
    
	return (ret);
}

static void *
huge_ralloc(void *ptr, size_t size, size_t oldsize)
{
	void *ret;
	size_t copysize;
    
	/* Avoid moving the allocation if the size class would not change. */
	if (oldsize > arena_maxclass &&
	    CHUNK_CEILING(size) == CHUNK_CEILING(oldsize)) {
		if (opt_junk && size < oldsize) {
			memset((void *)((uintptr_t)ptr + size), 0x5a, oldsize
                   - size);
		} else if (opt_zero && size > oldsize) {
			memset((void *)((uintptr_t)ptr + oldsize), 0, size
                   - oldsize);
		}
		return (ptr);
	}
    
	/*
	 * If we get here, then size and oldsize are different enough that we
	 * need to use a different size class.  In that case, fall back to
	 * allocating new space and copying.
	 */
	ret = huge_malloc(size, false);
	if (ret == NULL)
		return (NULL);
    
	copysize = (size < oldsize) ? size : oldsize;
	memcpy(ret, ptr, copysize);
	idalloc(ptr);
	return (ret);
}

static void
huge_dalloc(void *ptr)
{
	extent_node_t *node, key;
    
	/* Extract from tree of huge allocations. */
	key.addr = ptr;
	node = extent_tree_ad_search(&huge, &key);
	assert(node != NULL);
	assert(node->addr == ptr);
	extent_tree_ad_remove(&huge, node);
    
#ifdef MALLOC_STATS
	huge_ndalloc++;
	huge_allocated -= node->size;
#endif
    
	/* Unmap chunk. */
	if (opt_junk)
		memset(node->addr, 0x5a, node->size);
	
	chunk_dealloc(node->addr, node->size);
    
	base_node_dealloc(node);
}

void
malloc_print_stats(void)
{
    
	if (opt_print_stats) {
		char s[UMAX2S_BUFSIZE];
		_malloc_message("___ Begin malloc statistics ___\n", "", "",
                        "");
		_malloc_message("Assertions ",
#ifdef NDEBUG
                        "disabled",
#else
                        "enabled",
#endif
                        "\n", "");
		_malloc_message("Boolean MALLOC_OPTIONS: ",
                        opt_abort ? "A" : "a", "", "");
		_malloc_message( "D", "", "", "");
		_malloc_message(opt_junk ? "J" : "j", "", "", "");
		_malloc_message("m", "", "", "");
		_malloc_message("Pu",
                        opt_sysv ? "V" : "v",
                        opt_xmalloc ? "X" : "x",
                        opt_zero ? "Z\n" : "z\n");
        
		_malloc_message("CPUs: ", umax2s(ncpus, s), "\n", "");
		_malloc_message("Max arenas: ", umax2s(narenas, s), "\n", "");
		
		_malloc_message("Pointer size: ", umax2s(sizeof(void *), s),
                        "\n", "");
		_malloc_message("Quantum size: ", umax2s(QUANTUM, s), "\n", "");
		_malloc_message("Cacheline size (assumed): ", umax2s(CACHELINE,
                                                             s), "\n", "");
#ifdef MALLOC_TINY
		_malloc_message("Tiny 2^n-spaced sizes: [", umax2s((1U <<
                                                            TINY_MIN_2POW), s), "..", "");
		_malloc_message(umax2s((qspace_min >> 1), s), "]\n", "", "");
#endif
		_malloc_message("Quantum-spaced sizes: [", umax2s(qspace_min,
                                                          s), "..", "");
		_malloc_message(umax2s(qspace_max, s), "]\n", "", "");
		_malloc_message("Cacheline-spaced sizes: [", umax2s(cspace_min,
                                                            s), "..", "");
		_malloc_message(umax2s(cspace_max, s), "]\n", "", "");
		_malloc_message("Subpage-spaced sizes: [", umax2s(sspace_min,
                                                          s), "..", "");
		_malloc_message(umax2s(sspace_max, s), "]\n", "", "");
		
		_malloc_message("Max dirty pages per arena: ",
                        umax2s(opt_dirty_max, s), "\n", "");
        
		_malloc_message("Chunk size: ", umax2s(chunksize, s), "", "");
		_malloc_message(" (2^", umax2s(opt_chunk_2pow, s), ")\n", "");
        
#ifdef MALLOC_STATS
		{
			size_t allocated, mapped;
			
			unsigned i;
			arena_t *arena;
            
			/* Calculate and print allocated/mapped stats. */
            
			/* arenas. */
			for (i = 0, allocated = 0; i < narenas; i++) {
				if (arenas[i] != NULL) {
					allocated +=
                    arenas[i]->stats.allocated_small;
					allocated +=
                    arenas[i]->stats.allocated_large;
				}
			}
            
			/* huge/base. */
			allocated += huge_allocated;
			mapped = stats_chunks.curchunks * chunksize;
            
			mapped += base_mapped;
            
			malloc_printf("Allocated: %zu, mapped: %zu\n",
                          allocated, mapped);            
			
            
			/* Print chunk stats. */
			{
				chunk_stats_t chunks_stats;
                
				chunks_stats = stats_chunks;
                
				malloc_printf("chunks: nchunks   "
                              "highchunks    curchunks\n");
				malloc_printf("  %13llu%13lu%13lu\n",
                              chunks_stats.nchunks,
                              chunks_stats.highchunks,
                              chunks_stats.curchunks);
			}
            
			/* Print chunk stats. */
			malloc_printf(
                          "huge: nmalloc      ndalloc    allocated\n");
			malloc_printf(" %12llu %12llu %12zu\n",
                          huge_nmalloc, huge_ndalloc, huge_allocated);
            
			/* Print stats for each arena. */
			for (i = 0; i < narenas; i++) {
				arena = arenas[i];
				if (arena != NULL) {
					malloc_printf(
                                  "\narenas[%u]:\n", i);
					stats_print(arena);
				}
			}
		}
#endif /* #ifdef MALLOC_STATS */
		_malloc_message("--- End malloc statistics ---\n", "", "", "");
	}
}

#ifdef MALLOC_DEBUG
static void
size2bin_validate(void)
{
	size_t i, size, binind;
    
	assert(size2bin[0] == 0xffU);
	i = 1;
#  ifdef MALLOC_TINY
	/* Tiny. */
	for (; i < (1U << TINY_MIN_2POW); i++) {
		size = pow2_ceil(1U << TINY_MIN_2POW);
		binind = ffs((int)(size >> (TINY_MIN_2POW + 1)));
		assert(size2bin[i] == binind);
	}
	for (; i < qspace_min; i++) {
		size = pow2_ceil(i);
		binind = ffs((int)(size >> (TINY_MIN_2POW + 1)));
		assert(size2bin[i] == binind);
	}
#  endif
	/* Quantum-spaced. */
	for (; i <= qspace_max; i++) {
		size = QUANTUM_CEILING(i);
		binind = ntbins + (size >> QUANTUM_2POW) - 1;
		assert(size2bin[i] == binind);
	}
	/* Cacheline-spaced. */
	for (; i <= cspace_max; i++) {
		size = CACHELINE_CEILING(i);
		binind = ntbins + nqbins + ((size - cspace_min) >>
                                    CACHELINE_2POW);
		assert(size2bin[i] == binind);
	}
	/* Sub-page. */
	for (; i <= sspace_max; i++) {
		size = SUBPAGE_CEILING(i);
		binind = ntbins + nqbins + ncbins + ((size - sspace_min)
                                             >> SUBPAGE_2POW);
		assert(size2bin[i] == binind);
	}
}
#endif

static bool
size2bin_init(void)
{
    
	if (opt_qspace_max_2pow != QSPACE_MAX_2POW_DEFAULT
	    || opt_cspace_max_2pow != CSPACE_MAX_2POW_DEFAULT)
		return (size2bin_init_hard());
    
	size2bin = const_size2bin;
#ifdef MALLOC_DEBUG
	assert(sizeof(const_size2bin) == bin_maxclass + 1);
	size2bin_validate();
#endif
	return (false);
}

static bool
size2bin_init_hard(void)
{
	size_t i, size, binind;
	uint8_t *custom_size2bin;
    
	assert(opt_qspace_max_2pow != QSPACE_MAX_2POW_DEFAULT
           || opt_cspace_max_2pow != CSPACE_MAX_2POW_DEFAULT);
    
	custom_size2bin = (uint8_t *)base_alloc(bin_maxclass + 1);
	if (custom_size2bin == NULL)
		return (true);
    
	custom_size2bin[0] = 0xffU;
	i = 1;
#ifdef MALLOC_TINY
	/* Tiny. */
	for (; i < (1U << TINY_MIN_2POW); i++) {
		size = pow2_ceil(1U << TINY_MIN_2POW);
		binind = ffs((int)(size >> (TINY_MIN_2POW + 1)));
		custom_size2bin[i] = binind;
	}
	for (; i < qspace_min; i++) {
		size = pow2_ceil(i);
		binind = ffs((int)(size >> (TINY_MIN_2POW + 1)));
		custom_size2bin[i] = binind;
	}
#endif
	/* Quantum-spaced. */
	for (; i <= qspace_max; i++) {
		size = QUANTUM_CEILING(i);
		binind = ntbins + (size >> QUANTUM_2POW) - 1;
		custom_size2bin[i] = binind;
	}
	/* Cacheline-spaced. */
	for (; i <= cspace_max; i++) {
		size = CACHELINE_CEILING(i);
		binind = ntbins + nqbins + ((size - cspace_min) >>
                                    CACHELINE_2POW);
		custom_size2bin[i] = binind;
	}
	/* Sub-page. */
	for (; i <= sspace_max; i++) {
		size = SUBPAGE_CEILING(i);
		binind = ntbins + nqbins + ncbins + ((size - sspace_min) >>
                                             SUBPAGE_2POW);
		custom_size2bin[i] = binind;
	}
    
	size2bin = custom_size2bin;
#ifdef MALLOC_DEBUG
	size2bin_validate();
#endif
	return (false);
}

static unsigned
malloc_ncpus(void)
{    
	return (1); // single - threaded
}

static inline bool
malloc_init(void)
{
    
	if (malloc_initialized == false)
		return (malloc_init_hard());
    
	return (false);
}

static bool
malloc_init_hard(void)
{
	unsigned i;
	char buf[PATH_MAX + 1];
	const char *opts;
    
	if (malloc_initialized) {
		
		return (false);
	}
    
	/* Get number of CPUs. */
	ncpus = malloc_ncpus();
    
	/* Get page size. */
	{
		long result = PAGE_SIZE;        
		assert(result != -1);
		pagesize = (unsigned)result;
        
		/*
		 * We assume that pagesize is a power of 2 when calculating
		 * pagesize_mask and pagesize_2pow.
		 */
		assert(((result - 1) & result) == 0);
		pagesize_mask = result - 1;
		pagesize_2pow = ffs((int)result) - 1;
	}
    
	for (i = 0; i < 3; i++) {
		unsigned j;
        
		/* Get runtime configuration. */
		switch (i) {
            case 0:
			{
				/* No configuration specified. */
				buf[0] = '\0';
				opts = buf;
			}
                break;
            case 1:
			{
				/* No configuration specified. */
				buf[0] = '\0';
				opts = buf;
			}
                break;
            case 2:
                if (_malloc_options != NULL) {
                    /*
                     * Use options that were compiled into the
                     * program.
                     */
                    opts = _malloc_options;
                } else {
                    /* No configuration specified. */
                    buf[0] = '\0';
                    opts = buf;
                }
                break;
            default:
                /* NOTREACHED */
                assert(false);
		}
        
		for (j = 0; opts[j] != '\0'; j++) {
			unsigned k, nreps;
			bool nseen;
            
			/* Parse repetition count, if any. */
			for (nreps = 0, nseen = false;; j++, nseen = true) {
				switch (opts[j]) {
					case '0': case '1': case '2': case '3':
					case '4': case '5': case '6': case '7':
					case '8': case '9':
						nreps *= 10;
						nreps += opts[j] - '0';
						break;
					default:
						goto MALLOC_OUT;
				}
			}
        MALLOC_OUT:
			if (nseen == false)
				nreps = 1;
            
			for (k = 0; k < nreps; k++) {
				switch (opts[j]) {
                    case 'a':
                        opt_abort = false;
                        break;
                    case 'A':
                        opt_abort = true;
                        break;
                    case 'b':
                        break;
                    case 'B':
                        break;
                    case 'c':
                        if (opt_cspace_max_2pow - 1 >
                            opt_qspace_max_2pow &&
                            opt_cspace_max_2pow >
                            CACHELINE_2POW)
                            opt_cspace_max_2pow--;
                        break;
                    case 'C':
                        if (opt_cspace_max_2pow < pagesize_2pow
                            - 1)
                            opt_cspace_max_2pow++;
                        break;
                    case 'd':
                        break;
                    case 'D':
                        break;
                    case 'f':
                        opt_dirty_max >>= 1;
                        break;
                    case 'F':
                        if (opt_dirty_max == 0)
                            opt_dirty_max = 1;
                        else if ((opt_dirty_max << 1) != 0)
                            opt_dirty_max <<= 1;
                        break;
                    case 'j':
                        opt_junk = false;
                        break;
                    case 'J':
                        opt_junk = true;
                        break;
                    case 'k':
                        /*
                         * Chunks always require at least one
                         * header page, so chunks can never be
                         * smaller than two pages.
                         */
                        if (opt_chunk_2pow > pagesize_2pow + 1)
                            opt_chunk_2pow--;
                        break;
                    case 'K':
                        if (opt_chunk_2pow + 1 <
                            (sizeof(size_t) << 3))
                            opt_chunk_2pow++;
                        break;
                    case 'm':
                        break;
                    case 'M':
                        break;
                    case 'n':
                        opt_narenas_lshift--;
                        break;
                    case 'N':
                        opt_narenas_lshift++;
                        break;
                    case 'p':
                        opt_print_stats = false;
                        break;
                    case 'P':
                        opt_print_stats = true;
                        break;
                    case 'q':
                        if (opt_qspace_max_2pow > QUANTUM_2POW)
                            opt_qspace_max_2pow--;
                        break;
                    case 'Q':
                        if (opt_qspace_max_2pow + 1 <
                            opt_cspace_max_2pow)
                            opt_qspace_max_2pow++;
                        break;
                    case 'u':
                        break;
                    case 'U':
                        break;
                    case 'v':
                        opt_sysv = false;
                        break;
                    case 'V':
                        opt_sysv = true;
                        break;
                    case 'x':
                        opt_xmalloc = false;
                        break;
                    case 'X':
                        opt_xmalloc = true;
                        break;
                    case 'z':
                        opt_zero = false;
                        break;
                    case 'Z':
                        opt_zero = true;
                        break;
                    default: {
                        char cbuf[2];
                        
                        cbuf[0] = opts[j];
                        cbuf[1] = '\0';
                        _malloc_message(_getprogname(),
                                        ": (malloc) Unsupported character "
                                        "in malloc options: '", cbuf,
                                        "'\n");
                    }
				}
			}
		}
	}
    
	/* Set variables according to the value of opt_[qc]space_max_2pow. */
	qspace_max = (1U << opt_qspace_max_2pow);
	cspace_min = CACHELINE_CEILING(qspace_max);
	if (cspace_min == qspace_max)
		cspace_min += CACHELINE;
	cspace_max = (1U << opt_cspace_max_2pow);
	sspace_min = SUBPAGE_CEILING(cspace_max);
	if (sspace_min == cspace_max)
		sspace_min += SUBPAGE;
	assert(sspace_min < pagesize);
	sspace_max = pagesize - SUBPAGE;
    
#ifdef MALLOC_TINY
	assert(QUANTUM_2POW >= TINY_MIN_2POW);
#endif
	assert(ntbins <= QUANTUM_2POW);
	nqbins = qspace_max >> QUANTUM_2POW;
	ncbins = ((cspace_max - cspace_min) >> CACHELINE_2POW) + 1;
	nsbins = ((sspace_max - sspace_min) >> SUBPAGE_2POW) + 1;
	nbins = ntbins + nqbins + ncbins + nsbins;
    
	if (size2bin_init()) {
		return (true);
	}
    
	/* Set variables according to the value of opt_chunk_2pow. */
	chunksize = (1LU << opt_chunk_2pow);
	chunksize_mask = chunksize - 1;
	chunk_npages = (chunksize >> pagesize_2pow);
	{
		size_t header_size;
        
		/*
		 * Compute the header size such that it is large enough to
		 * contain the page map.
		 */
		header_size = sizeof(arena_chunk_t) +
        (sizeof(arena_chunk_map_t) * (chunk_npages - 1));
		arena_chunk_header_npages = (header_size >> pagesize_2pow) +
        ((header_size & pagesize_mask) != 0);
	}
	arena_maxclass = chunksize - (arena_chunk_header_npages <<
                                  pagesize_2pow);
    
    
#ifdef MALLOC_STATS
	memset(&stats_chunks, 0, sizeof(chunk_stats_t));
#endif
    
	/* Various sanity checks that regard configuration. */
	assert(chunksize >= pagesize);
    
	/* Initialize chunks data. */
	
	extent_tree_ad_new(&huge);
	
	dss_base = sbrk(0);
	dss_prev = dss_base;
	dss_max = dss_base;
	extent_tree_szad_new(&dss_chunks_szad);
	extent_tree_ad_new(&dss_chunks_ad);
#ifdef MALLOC_STATS
	huge_nmalloc = 0;
	huge_ndalloc = 0;
	huge_allocated = 0;
#endif
    
	/* Initialize base allocation data structures. */
#ifdef MALLOC_STATS
	base_mapped = 0;
#endif
	/*
	 * Allocate a base chunk here, since it doesn't actually have to be
	 * chunk-aligned.  Doing this before allocating any other chunks allows
	 * the use of space that would otherwise be wasted.
	 */
	base_pages_alloc(0);
	
	base_nodes = NULL;
    
	/* Determine how many arenas to use. */
	narenas = ncpus;
	if (opt_narenas_lshift > 0) {
		if ((narenas << opt_narenas_lshift) > narenas)
			narenas <<= opt_narenas_lshift;
		/*
		 * Make sure not to exceed the limits of what base_alloc() can
		 * handle.
		 */
		if (narenas * sizeof(arena_t *) > chunksize)
			narenas = chunksize / sizeof(arena_t *);
	} else if (opt_narenas_lshift < 0) {
		if ((narenas >> -opt_narenas_lshift) < narenas)
			narenas >>= -opt_narenas_lshift;
		/* Make sure there is at least one arena. */
		if (narenas == 0)
			narenas = 1;
	}
    
	if (narenas > 1) {
		static const unsigned primes[] = {1, 3, 5, 7, 11, 13, 17, 19,
		    23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83,
		    89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149,
		    151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211,
		    223, 227, 229, 233, 239, 241, 251, 257, 263};
		unsigned nprimes, parenas;
        
		/*
		 * Pick a prime number of hash arenas that is more than narenas
		 * so that direct hashing of pthread_self() pointers tends to
		 * spread allocations evenly among the arenas.
		 */
		assert((narenas & 1) == 0); /* narenas must be even. */
		nprimes = (sizeof(primes) >> SIZEOF_INT_2POW);
		parenas = primes[nprimes - 1]; /* In case not enough primes. */
		for (i = 1; i < nprimes; i++) {
			if (primes[i] > narenas) {
				parenas = primes[i];
				break;
			}
		}
		narenas = parenas;
	}
    
    
	/* Allocate and initialize arenas. */
	arenas = (arena_t **)base_alloc(sizeof(arena_t *) * narenas);
	if (arenas == NULL) {
		return (true);
	}
	/*
	 * Zero the array.  In practice, this should always be pre-zeroed,
	 * since it was just mmap()ed, but let's be sure.
	 */
	memset(arenas, 0, sizeof(arena_t *) * narenas);
    
	/*
	 * Initialize one arena here.  The rest are lazily created in
	 * choose_arena_hard().
	 */
	arenas_extend(0);
	if (arenas[0] == NULL) {
		return (true);
	}    
    
	malloc_initialized = true;
	return (false);
}

/*
 * End general internal functions.
 */
/******************************************************************************/
/*
 * Begin malloc(3)-compatible functions.
 */

#if defined(HEAP_TRACKING)
#define FUNC_NAME(x) x##_real
#else
#define FUNC_NAME(x) x
#endif

void *
FUNC_NAME(je_malloc)(size_t size);
void *
FUNC_NAME(je_malloc)(size_t size)
{
	void *ret;
    
	if (malloc_init()) {
		ret = NULL;
		goto RETURN;
	}
    
	if (size == 0) {
		if (opt_sysv == false)
			size = 1;
		else {
			ret = NULL;
			goto RETURN;
		}
	}
    
	ret = imalloc(size);
    
RETURN:
	if (ret == NULL) {
		if (opt_xmalloc) {
			_malloc_message(_getprogname(),
                            ": (malloc) Error in malloc(): out of memory\n", "",
                            "");
			abort();
		}
	}
    
	return (ret);
}

/* 
 * NOTE: this function assumes that any checks on alignment have already been done and that
 * malloc_init() has been called
 */
static int
memalign_base(void **memptr, size_t alignment, size_t size, const char *caller)
{
	int ret;
	void *result;
    
	result = ipalloc(alignment, size);
	
	if (result == NULL) {
		if (opt_xmalloc) {
			_malloc_message(_getprogname(),
                            ": (malloc) Error in ",
                            caller,
                            "(): out of memory\n");
			abort();
		}
		ret = ENOMEM;
		goto RETURN;
	}
    
	*memptr = result;
	ret = 0;
    
RETURN:
	return (ret);
}

int
FUNC_NAME(je_posix_memalign)(void **memptr, size_t alignment, size_t size);
int
FUNC_NAME(je_posix_memalign)(void **memptr, size_t alignment, size_t size)
{
	int ret;
    
	if (malloc_init())
		ret = ENOMEM;
        else {
            /* Make sure that alignment is a large enough power of 2. */
            if (((alignment - 1) & alignment) != 0
                || alignment < sizeof(void *)) {
                if (opt_xmalloc) {
                    _malloc_message(_getprogname(),
                                    ": (malloc) Error in posix_memalign(): "
                                    "invalid alignment\n", "", "");
                    abort();
                }
                ret = EINVAL;
                goto RETURN;
            }
            
            ret = memalign_base(memptr, alignment, size, __func__);
        }
    
RETURN:
	return (ret);
}

void*
FUNC_NAME(je_memalign)(size_t boundary, size_t size);
void*
FUNC_NAME(je_memalign)(size_t boundary, size_t size)
{
	void *result = NULL;
	size_t alignment = boundary;
    
	if (malloc_init())
		result = NULL;
        else {
            /* Use normal malloc */
            if (alignment <= QUANTUM) {
                return FUNC_NAME(je_malloc)(size);
            }
            
            /* Round up to nearest power of 2 if not power of 2 */
            if ((alignment & (alignment - 1)) != 0) {
                alignment = next_power_of_two(alignment);
            }
            
            (void)memalign_base(&result, alignment, size, __func__);
        }
    
	return (result);
}

void*
FUNC_NAME(je_valloc)(size_t size);
void*
FUNC_NAME(je_valloc)(size_t size)
{
	void *result = NULL;
    
	if (malloc_init())
		result = NULL;
        else 
            (void)memalign_base(&result, pagesize, size, __func__);
            
            return result;
}

void *
FUNC_NAME(je_calloc)(size_t num, size_t size);
void *
FUNC_NAME(je_calloc)(size_t num, size_t size)
{
	void *ret;
	size_t num_size;
    
	if (malloc_init()) {
		num_size = 0;
		ret = NULL;
		goto RETURN;
	}
    
	num_size = num * size;
	if (num_size == 0) {
		if ((opt_sysv == false) && ((num == 0) || (size == 0)))
			num_size = 1;
		else {
			ret = NULL;
			goto RETURN;
		}
        /*
         * Try to avoid division here.  We know that it isn't possible to
         * overflow during multiplication if neither operand uses any of the
         * most significant half of the bits in a size_t.
         */
	} else if (((num | size) & (SIZE_T_MAX << (sizeof(size_t) << 2)))
               && (num_size / size != num)) {
		/* size_t overflow. */
		ret = NULL;
		goto RETURN;
	}
    
	ret = icalloc(num_size);
    
RETURN:
	if (ret == NULL) {
		if (opt_xmalloc) {
			_malloc_message(_getprogname(),
                            ": (malloc) Error in calloc(): out of memory\n", "",
                            "");
			abort();
		}
	}
    
	return (ret);
}

void *
FUNC_NAME(je_realloc)(void *ptr, size_t size);
void *
FUNC_NAME(je_realloc)(void *ptr, size_t size)
{
	void *ret;
    
	if (size == 0) {
		if (opt_sysv == false)
			size = 1;
		else {
			if (ptr != NULL)
				idalloc(ptr);
			ret = NULL;
			goto RETURN;
		}
	}
    
	if (ptr != NULL) {
		assert(malloc_initialized);
        
		ret = iralloc(ptr, size);
        
		if (ret == NULL) {
			if (opt_xmalloc) {
				_malloc_message(_getprogname(),
                                ": (malloc) Error in realloc(): out of "
                                "memory\n", "", "");
				abort();
			}
		}
	} else {
		if (malloc_init())
			ret = NULL;
		else
			ret = imalloc(size);
        
		if (ret == NULL) {
			if (opt_xmalloc) {
				_malloc_message(_getprogname(),
                                ": (malloc) Error in realloc(): out of "
                                "memory\n", "", "");
				abort();
			}
		}
	}
    
RETURN:
	return (ret);
}

void
FUNC_NAME(je_free)(void *ptr);
void
FUNC_NAME(je_free)(void *ptr)
{
    
	if (ptr != NULL) {
		assert(malloc_initialized);
        
		idalloc(ptr);
	}
}

/*
 * End malloc(3)-compatible functions.
 */
/******************************************************************************/
/*
 * Begin non-standard functions.
 */

size_t
malloc_usable_size(const void *ptr)
{
    
	assert(ptr != NULL);
    
	return (isalloc(ptr));
}

/*
 * End non-standard functions.
 */
/******************************************************************************/