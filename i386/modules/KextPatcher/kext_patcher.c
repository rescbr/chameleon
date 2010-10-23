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
#include "hex_editor.h"


#ifndef DEBUG_KEXT_PATCHER
#define DEBUG_KEXT_PATCHER 0
#endif

#if DEBUG_KEXT_PATCHER
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif
bool patch_kext(TagPtr plist, char* plistbuffer, void* start);
bool patch_gma_kexts(TagPtr plist, char* plistbuffer, void* start);
bool patch_bcm_kext(TagPtr plist, char* plistbuffer, void* start);

static void * z_alloc(void *, u_int items, u_int size);
static void   z_free(void *, void *ptr);

uint16_t patch_gma_deviceid = 0;
uint16_t patch_bcm_deviceid = 0;

#define NEEDS_PATCHING		(patch_bcm_deviceid || patch_gma_deviceid)

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
	//register_hook_callback("LoadMatchedModules", &kext_loaded); 
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
	if(!NEEDS_PATCHING) return;	// No need to apply a patch, hardware doesn't need it
	
	int version = 0;
	//int length = *((int*)lengthtmp);
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
	
	/*
	if(strcmp(filespec, "/System/Library/Caches/com.apple.kext.caches/Startup/Extensions.mkext") == 0)
	{
		printf("Invalidating mkext %s\n", filespec);
		// 10.6 cache folder. Doesn't contain certain extensions we need, so invalidate it.
		//package->adler32++;
		// NOTE:  double check that this is needed
		package->magic = 0x00;
		return;
	}*/
	
	
	version = MKEXT_GET_VERSION(package);
	
	if(version == 0x01008000) // mkext1
	{
		// mkext1 uses lzss
		mkext1_header* package = packagetmp;
		int i;
		for(i = 0; i < MKEXT_GET_COUNT(package); i++)
		{
			DBG("Parsing kext %d\n", i);
			//mkext_kext* kext = MKEXT1_GET_KEXT(package, i);
			// uses decompress_lzss
			// TODO: handle kext

		}
	}
	else if((version & 0xFFFF0000) == 0x02000000) // mkext2
	{
		DBG("Mkext2 package located at 0x%X\n", package);

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
		if (zstream_inited) inflateEnd(&zstream);

		DBG("Inflated result is %d, in: %d bytes, out: %d bytes\n", zlib_result, zstream.total_in, zstream.total_out);
		if (zlib_result == Z_STREAM_END || zlib_result == Z_OK)
		{			
			config_file_t plistData;
			config_file_t allDicts;
			bzero(&plistData, sizeof(plistData));
			bzero(&allDicts, sizeof(allDicts));
			
			XMLParseFile( plist, &plistData.dictionary );

			int count = 0;

			allDicts.dictionary = XMLGetProperty(plistData.dictionary, kMKEXTInfoDictionariesKey);
			count = XMLTagCount(allDicts.dictionary);

			DBG("Plist contains %d kexts\n", count);
			
			
			bool patched = false;
			for(; count--; count > 0)
			{
				TagPtr kextEntry = XMLGetElement(allDicts.dictionary, count);
				patched |= patch_kext(kextEntry, plist, package);
			}
			

			if(patched)
			{
				zstream_inited = false;
				// Recompress the plist
				bzero(&zstream, sizeof(zstream));		
				zstream.next_in   = (UInt8*)plist;
				zstream.next_out  = (UInt8*)package + plist_offset;
				zstream.avail_in  = MKEXT2_GET_PLIST_FULLSIZE(package);
				zstream.avail_out = MKEXT2_GET_PLIST_FULLSIZE(package)<<2;	// Give us some extra free space, just in case
				zstream.zalloc    = Z_NULL;
				zstream.zfree     = Z_NULL;
				zstream.opaque    = Z_NULL;
				
				
				zlib_result = deflateInit2(&zstream, Z_DEFAULT_COMPRESSION,  Z_DEFLATED,15, 8 /* memLevel */, Z_DEFAULT_STRATEGY);
				if (Z_OK != zlib_result) {
					printf("ZLIB Deflate Error: %s\n", zstream.msg);
					getc();
				}
				else 
				{
					zstream_inited = true;
				}
				
				zlib_result = deflate(&zstream, Z_FINISH);
				
				if (zlib_result == Z_STREAM_END)
				{
					DBG("Deflated result is %d, in: %d bytes, out: %d bytes, full: %d\n", zlib_result, zstream.total_in, zstream.total_out, MKEXT2_GET_PLIST_FULLSIZE(package));
				} 
				else if (zlib_result == Z_OK)
				{
					/* deflate filled output buffer, meaning the data doesn't compress.
					 */
					DBG("Deflated result is %d, in: %d bytes, out: %d bytes, full: %d\n", zlib_result, zstream.total_in, zstream.total_out, MKEXT2_GET_PLIST_FULLSIZE(package));
					
				} 
				else if (zlib_result != Z_STREAM_ERROR)
				{
					printf("ZLIB Deflate Error: %s\n", zstream.msg);
					getc();
				}
				
				if(zstream.total_out != MKEXT2_GET_PLIST_COMPSIZE(package))
				{
					// Update the mkext length
					MKEXT2_HDR_CAST(package)->length = MKEXT_SWAP(MKEXT_GET_LENGTH(package) - MKEXT2_GET_PLIST_COMPSIZE(package) + zstream.total_out);
					MKEXT2_HDR_CAST(package)->plist_compressed_size = MKEXT_SWAP(zstream.total_out);
					*((int*)lengthtmp) -= MKEXT2_GET_PLIST_COMPSIZE(package);
					*((int*)lengthtmp) += zstream.total_out;
				}
								
				if (zstream_inited) deflateEnd(&zstream);
				
				
				
				// re alder32 the new mkext2 package
				MKEXT_HDR_CAST(package)->adler32 = 
					MKEXT_SWAP(Mkext_Alder32((unsigned char *)&package->version,
											 MKEXT_GET_LENGTH(package) - 0x10));
			}
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
		

	}

	
	DBG("Loading %s, length %d, version 0x%x\n", filespec, length, version);
	//getc();
}

// FIXME: only handles mkext2 entries
bool patch_kext(TagPtr plist, char* plistbuffer, void* start)
{
	char* bundleID;
	
	if(XMLGetProperty(plist, kMKEXTExecutableKey) == NULL) return false;	// Kext is a plist only kext, don't patch
	
	bundleID = XMLCastString(XMLGetProperty(plist, kPropCFBundleIdentifier));
	
	
	if(patch_gma_deviceid &&
	    (
			(strcmp(bundleID, "com.apple.driver.AppleIntelGMA950") == 0) ||
			(strcmp(bundleID, "com.apple.driver.AppleIntelIntegratedFramebuffer") == 0)
		 )
	   )
	{
		return patch_gma_kexts(plist, plistbuffer, start);
	}
	else if(patch_bcm_deviceid && (strcmp(bundleID, "com.apple.driver.AirPortBrcm43xx") == 0))
	{
		return patch_bcm_kext(plist, plistbuffer, start);

	}
	return false;
}

void KextPatcher_hook(void* arg1, void* arg2, void* arg3, void* arg4)
{
	pci_dt_t* current = arg1;
	if(current)
	{
		if(current->class_id == PCI_CLASS_DISPLAY_VGA)
		{
			if(current->vendor_id == 0x8086 && current->device_id == 0x27AE)
			{
				// TODO: patche based on dev id.
				patch_gma_deviceid = current->device_id;
			}
		}
		else if(current->class_id == PCI_CLASS_NETWORK_OTHER) 
		{
			// Patch BCM43xx
			if(current->vendor_id == 0x14E4 && ((current->device_id & 0xFF00) == 0x4300))
			{
				patch_bcm_deviceid = current->device_id;
			}
		}
	}
}

bool patch_bcm_kext(TagPtr plist, char* plistbuffer, void* start)
{
	TagPtr personality;
	personality =		XMLCastDict(XMLGetProperty(plist, kPropIOKitPersonalities));
	personality =		XMLGetProperty(personality, (const char*)"Broadcom 802.11 PCI");	
	TagPtr match_names =XMLCastArray(XMLGetProperty(personality, (const char*)"IONameMatch"));

	
	char* new_str = malloc(strlen("pci14e4,xxxx")+1);
	sprintf(new_str, "pci14e4,%02x", patch_bcm_deviceid);

	// Check to see if we *really* need to modify the plist, if not, return false
	// so that *if* this were going ot be the only modified kext, the repacking code
	// won't need to be executed.
	int count = XMLTagCount(match_names);
	while(count)
	{
		count--;
		TagPtr replace =	XMLGetElement(match_names, count);	// Modify the second entry
		char* orig_string = XMLCastString(replace);
		if(strcmp(orig_string, new_str) == 0) return false;
	}

	
	TagPtr replace =	XMLGetElement(match_names, 1);	// Modify the second entry
	char* orig_string = XMLCastString(replace);
	
	DBG("Attemting to replace '%s' with '%s'\n", orig_string, new_str);

	// TODO: verify string doesn't exist first.
	
	replace_string(orig_string, new_str, plistbuffer + XMLCastStringOffset(replace));
	
	return true;
}

bool patch_gma_kexts(TagPtr plist, char* plistbuffer, void* start)
{
	int exeutable_offset, full_size, compressed_size;
	TagPtr personality;
	long offset;
	int zlib_result;
	z_stream       zstream;
	bool           zstream_inited = false;
	mkext2_file_entry* kext;
	void* compressed_data;

	exeutable_offset = XMLCastInteger(XMLGetProperty(plist, kMKEXTExecutableKey));
	kext = (void*)((char*)start + exeutable_offset);

	full_size = MKEXT2_GET_ENTRY_FULLSIZE(kext);
	compressed_size = MKEXT2_GET_ENTRY_COMPSIZE(kext);
	compressed_data = MKEXT2_GET_ENTRY_DATA(kext);
	
	personality =		XMLCastDict(XMLGetProperty(plist, kPropIOKitPersonalities));
	if(XMLGetProperty(personality, (const char*)"Intel915"))
	{
		personality =		XMLGetProperty(personality, (const char*)"Intel915");
	}
	else
	{
		personality =		XMLGetProperty(personality, (const char*)"AppleIntelIntegratedFramebuffer");	
	}
#if DEBUG_KEXT_PATCHER
	char* pcimatch =	XMLCastString(XMLGetProperty(personality, (const char*)"IOPCIPrimaryMatch"));
#endif
	offset =		XMLCastStringOffset(XMLGetProperty(personality, (const char*)"IOPCIPrimaryMatch"));
	
	replace_string("0x27A28086", "0x27AE8086", plistbuffer + offset);
	
	DBG("Located kext %s\n", bundleID);
	DBG("PCI Match offset = %d, string = %s\n", offset, pcimatch);
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
		printf("ZLIB Inflate Error: %s\n", zstream.msg);
		getc();
	}
	else 
	{
		zstream_inited = true;
	}
	
	
	zlib_result = inflate(&zstream, Z_FINISH);
	
	DBG("Inflated result is %d, in: %d bytes, out: %d bytes, full: %d\n", zlib_result, zstream.total_in, zstream.total_out, full_size);
	
	replace_word(0x27A28086, 0x27AE8086, executable, zstream.total_out);
	if (zstream_inited) inflateEnd(&zstream);
	
	
	zstream.next_in   = (UInt8*)executable;
	//		zstream.next_out  = (UInt8*)((int)compressed_data<<1);
	zstream.next_out  = (UInt8*)compressed_data;
	
	zstream.avail_in  = full_size;
	zstream.avail_out = compressed_size;
	zstream.zalloc    = Z_NULL;
	zstream.zfree     = Z_NULL;
	zstream.opaque    = Z_NULL;
	
	
	
	// Recompress the eecutable
	zlib_result = deflateInit2(&zstream, Z_DEFAULT_COMPRESSION,  Z_DEFLATED,15, 8 /* memLevel */, Z_DEFAULT_STRATEGY);
	if (Z_OK != zlib_result) {
		printf("ZLIB Deflate Error: %s\n", zstream.msg);
		getc();
	}
	else 
	{
		zstream_inited = true;
	}
	
	zlib_result = deflate(&zstream, Z_FINISH);
	
	if (zlib_result == Z_STREAM_END)
	{
		DBG("Deflated result is %d, in: %d bytes, out: %d bytes, full: %d\n", zlib_result, zstream.total_in, zstream.total_out, full_size);
	} 
	else if (zlib_result == Z_OK)
	{
		/* deflate filled output buffer, meaning the data doesn't compress.
		 */
		DBG("Deflated result is %d, in: %d bytes, out: %d bytes, full: %d\n", zlib_result, zstream.total_in, zstream.total_out, full_size);
		
	} 
	else if (zlib_result != Z_STREAM_ERROR)
	{
		printf("ZLIB Deflate Error: %s\n", zstream.msg);
		getc();
	}
	
	if (zstream_inited) deflateEnd(&zstream);
	
	free(executable);
	
	return true;	
}