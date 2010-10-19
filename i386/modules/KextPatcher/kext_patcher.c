	/*
 * Copyright (c) 2010 Evan Lojewski. All rights reserved.
 *	
 *	KextPather
 *	This is an experimental module that I'm looking into implimenting.
 *	The main purpose is to replace the need for programs such as 
 *  NetbookInstaller's kext patching routines. THis way, Apple's kexts can be
 *  patched whe loaded instead. (eg: GMA950 kext, Bluetooth + Wifi kexts)
 */

#include "libsaio.h"
#include "zlib.h"
#include "kext_patcher.h"
#include "pci.h"
#include "drivers.h"
#include "mkext.h"
#include "modules.h"

void patch_kext(TagPtr plist, void* start);

static void * z_alloc(void *, u_int items, u_int size);
static void   z_free(void *, void *ptr);

typedef struct z_mem {
    uint32_t alloc_size;
    uint8_t  data[0];
} z_mem;

/*
 * Space allocation and freeing routines for use by zlib routines.
 */
void *
z_alloc(void * notused __unused, u_int num_items, u_int size)
{
    void     * result = NULL;
    z_mem    * zmem = NULL;
    uint32_t   total = num_items * size;
    uint32_t   allocSize =  total + sizeof(zmem);
    
    zmem = (z_mem *)malloc(allocSize);
    if (!zmem) {
        goto finish;
    }
    zmem->alloc_size = allocSize;
    result = (void *)&(zmem->data);
finish:
    return result;
}

void
z_free(void * notused __unused, void * ptr)
{
    uint32_t * skipper = (uint32_t *)ptr - 1;
    z_mem    * zmem = (z_mem *)skipper;
    free((void *)zmem);
    return;
}


unsigned long Mkext_Alder32( unsigned char * buffer, long length );

void KextPatcher_hook(void* current, void* arg2, void* arg3, void* arg4);

/**
 ** KextPatcher_start -> module start
 **		Notified the module system that this module will hook into the 
 **		LoadMatchedModules and LoadDriverMKext functions
 **/
void KextPatcher_start()
{		
	// Hooks into the following:
	//	execute_hook("LoadDriverMKext", (void*)package, (void*) length, NULL, NULL);
	//  execute_hook("LoadMatchedModules", module, &length, executableAddr, NULL);

	register_hook_callback("PCIDevice", &KextPatcher_hook);
	register_hook_callback("LoadMatchedModules", &kext_loaded); 
	register_hook_callback("LoadDriverMKext", &mkext_loaded); 

}

/**
 ** kext_loaded -> Called whenever a kext is in read into memory
 **		This function will be used to patch kexts ( eg AppleInteIntegratedFramebuffer)
 **		and their plists when they are loaded into memmory
 **/
void kext_loaded(void* moduletmp, void* lengthprt, void* executableAddr, void* arg3)
{
	
	//ModulePtr module = moduletmp;
	//long length = *(long*)lengthprt;
	//long length2 = strlen(module->plistAddr);
	// *(long*)lengthprt = length2 + 5 *  1024 * 1024;

	//printf("Loading %s, lenght is %d, executable is 0x%X\n", module->plistAddr, length, executableAddr);
	//getc();
}

/**
 ** mkext_loaded -> Called whenever an mkext is in read into memory
 **		This function will be used to patch mkext. Matching kexts will be
 **		Extracted, modified, and then compressed again. Note: I need to determine
 **		what sort of slowdown this will cause and if it's worth implimenting.
 **/

void mkext_loaded(void* filespec, void* packagetmp, void* lengthtmp, void* arg3)
{
	int version = 0;
	int length = (int) lengthtmp;
	mkext_basic_header* package = packagetmp;

	// Verify the MKext.
    if (( MKEXT_GET_MAGIC(package)		!= MKEXT_MAGIC ) ||
        ( MKEXT_GET_SIGNATURE(package)	!= MKEXT_SIGN )  ||
        ( MKEXT_GET_LENGTH(package)		>  kLoadSize )	 ||
        ( MKEXT_GET_CHECKSUM(package)   !=
		 Mkext_Alder32((unsigned char *)&package->version, MKEXT_GET_LENGTH(package) - 0x10) ) )
    {
        return;
		// Don't try to patch a b
    }	
	
	
	if(strcmp(filespec, "/System/Library/Caches/com.apple.kext.caches/Startup/Extensions.mkext") == 0)
	{
		printf("Invalidating mkext %s\n", filespec);
		// 10.6 cache folder. Doesn't contain certain extensions we need, so invalidate it.
		//package->adler32++;
		// NOTE:  double check that this is needed
		package->magic = 0x00;
		return;
	}
	
	
	version = MKEXT_GET_VERSION(package);
	
	if(version == 0x01008000) // mkext1
	{
		// mkext1 uses lzss
		mkext1_header* package = packagetmp;
		int i;
		for(i = 0; i < MKEXT_GET_COUNT(package); i++)
		{
			printf("Parsing kext %d\n", i);
			//mkext_kext* kext = MKEXT1_GET_KEXT(package, i);
			// uses decompress_lzss
			// TODO: handle kext

		}
	}
	else if((version & 0xFFFF0000) == 0x02000000) // mkext2
	{
		printf("Mkext2 package located at 0x%X\n", package);

		// mkext2 uses zlib		
		mkext2_header* package = packagetmp;
		z_stream       zstream;
		bool           zstream_inited = false;
		int            zlib_result;
		int plist_offset = MKEXT2_GET_PLIST(package);
		
		char* plist = malloc(MKEXT2_GET_PLIST_FULLSIZE(package));
		
		bzero(&zstream, sizeof(zstream));		
		zstream.next_in   = (UInt8*)((char*)package + plist_offset);
		zstream.avail_in  = MKEXT2_GET_PLIST_COMPSIZE(package);
		
		zstream.next_out  = (UInt8*)plist;
		zstream.avail_out = MKEXT2_GET_PLIST_FULLSIZE(package);
		
		zstream.zalloc    = z_alloc;
		zstream.zfree     = z_free;
				
		zlib_result = inflateInit(&zstream);
		if (Z_OK != zlib_result)
		{
			printf("ZLIB Error: %s\n", zstream.msg);
			getc();
		}
		else 
		{
			zstream_inited = true;
		}

		
		zlib_result = inflate(&zstream, Z_FINISH);
		printf("Inflated result is %d, in: %d bytes, out: %d bytes\n", zlib_result, zstream.total_in, zstream.total_out);
		if (zlib_result == Z_STREAM_END || zlib_result == Z_OK)
		{
			//printf("Plist contains %s\n", plist);
			
			config_file_t plistData;
			config_file_t allDicts;
			bzero(&plistData, sizeof(plistData));
			bzero(&allDicts, sizeof(allDicts));
			
			//plist += strlen("<dict><key>_MKEXTInfoDictionaries</key><array>");	// Skip kMKEXTInfoDictionariesKey. Causes issues
																				// NOTE: there will be an extra </array></dict> at the end
			/*int len =*/ XMLParseFile( plist, &plistData.dictionary );

			int count = 0;

			count = XMLTagCount(plistData.dictionary);
			if(count != 1)
			{
				error("Mkext has more than one entry, unable to patch.");
				getc();
				return;
			}
			allDicts.dictionary = XMLGetProperty(plistData.dictionary, kMKEXTInfoDictionariesKey);
			count = XMLTagCount(allDicts.dictionary);
			printf("Element type: %d\n", allDicts.dictionary->type);
			printf("Element tag: %d\n", allDicts.dictionary->tag);
			printf("Element tagNext: %d\n", allDicts.dictionary->tagNext);

			printf("Plist contains %d kexts\n", count);
			
			for(; count--; count > 0)
			{
				TagPtr kextEntry = XMLGetElement(allDicts.dictionary, count);
				patch_kext(kextEntry, package);
			}
			
			printf("kexts parsed\n");

			
		}
		else
		{
			printf("ZLIB Error: %s\n", zstream.msg);
			getc();
		}

		//config_file_t mkextPlist;
		//ParseXMLFile((char*) plist, &mkextPlist.dictionary);
		
		
		
		
		
		/*		int i;
		for(i = 0; i < MKEXT_GET_COUNT(package); i++)
		{
			printf("Parsing kext %d\n", i);
		}
		*/
		
		if (zstream_inited) inflateEnd(&zstream);

	}

	
	printf("Loading %s, length %d, version 0x%x\n", filespec, length, version);
	getc();
}

// TODO: only handles mkext2 entries
void patch_kext(TagPtr plist, void* start)
{
	int exeutable_offset;
	mkext2_file_entry* kext;
	char* bundleID;
	int full_size, compressed_size;
	void* compressed_data;
	z_stream       zstream;
	bool           zstream_inited = false;
	int zlib_result;
	
	if(XMLGetProperty(plist, kMKEXTExecutableKey) == NULL) return;	// Kext is a plist only kext, don't patch
	
	bundleID = XMLCastString(XMLGetProperty(plist, kPropCFBundleIdentifier));
	exeutable_offset = XMLCastInteger(XMLGetProperty(plist, kMKEXTExecutableKey));
	kext = (void*)((char*)start + exeutable_offset);

	full_size = MKEXT2_GET_ENTRY_FULLSIZE(kext);
	compressed_size = MKEXT2_GET_ENTRY_COMPSIZE(kext);
	compressed_data = MKEXT2_GET_ENTRY_DATA(kext);
	
	if(strcmp(bundleID, "com.apple.driver.AppleIntelGMA950") == 0)
	//if(strcmp(bundleID, "com.apple.driver.AppleACPIBatteryManager") == 0)
	{
		printf("offset is 0x%x\n", exeutable_offset);
		
		char* executable = malloc(full_size);
		
		bzero(&zstream, sizeof(zstream));		
		zstream.next_in   = (UInt8*)compressed_data;
		zstream.avail_in  = compressed_size;
		
		zstream.next_out  = (UInt8*)executable;
		zstream.avail_out = full_size;
		
		zstream.zalloc    = z_alloc;
		zstream.zfree     = z_free;
		
		zlib_result = inflateInit(&zstream);
		if (Z_OK != zlib_result)
		{
			printf("ZLIB Error: %s\n", zstream.msg);
			getc();
		}
		else 
		{
			zstream_inited = true;
		}
		
		
		zlib_result = inflate(&zstream, Z_FINISH);
		
		printf("Inflated result is %d, in: %d bytes, out: %d bytes, full: %d\n", zlib_result, zstream.total_in, zstream.total_out, full_size);

		if (zstream_inited) inflateEnd(&zstream);

		free(executable);
		
		printf("\n");
		
		getc();		
	}
}


void KextPatcher_hook(void* arg1, void* arg2, void* arg3, void* arg4)
{
	//pci_dt_t* current = arg1;
}