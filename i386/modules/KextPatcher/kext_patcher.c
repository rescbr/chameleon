/*
 * Copyright (c) 2010 Evan Lojewski. All rights reserved.
 *	
 *	KextPather
 *	The main purpose of this moduleis to replace the need for programs such as 
 *  NetbookInstaller's kext patching routines. This way, Apple's kexts can be
 *  patched whe loaded instead. (eg: GMA950 kext, Bluetooth + Wifi kexts)
 */
#ifndef DEBUG_KEXT_PATCHER
#define DEBUG_KEXT_PATCHER 0
#endif

#if DEBUG_KEXT_PATCHER
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif


#include "libsaio.h"
#include "zlib.h"
#include "kext_patcher.h"
#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
#include "drivers.h"
#include "mkext.h"
#include "modules.h"
#include "hex_editor.h"


uint16_t patch_gma_deviceid = 0;
uint16_t patch_bcm_deviceid = 0;
uint16_t patch_atheros_deviceid = 0;
uint16_t patch_hda_codec = 0x00;		// TODO; detect proper codec


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
 ** kext_loaded -> Called whenever a kext is read into memory
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
	const char* hda_codec		= 0;
	int len						= 0;
	mkext_basic_header* package = packagetmp;
	int version					= MKEXT_GET_VERSION(package);

	if (getValueForKey(kHDACodec, &hda_codec, &len, &bootInfo->bootConfig))
	{
		patch_hda_codec = 0;
		int index = 0;
		while(len--)
		{
			patch_hda_codec <<= 4;
			patch_hda_codec |= chartohex(hda_codec[index]);
			index++;
		}
	}
		
	if(!NEEDS_PATCHING) return;	// No need to apply a patch, hardware doesn't need it
	


	// Verify the MKext.
    if (( MKEXT_GET_MAGIC(package)		!= MKEXT_MAGIC ) ||
        ( MKEXT_GET_SIGNATURE(package)	!= MKEXT_SIGN )  ||
        ( MKEXT_GET_LENGTH(package)		>  kLoadSize )	 ||
        ( MKEXT_GET_CHECKSUM(package)   !=
		 Adler32((unsigned char *)&package->version, MKEXT_GET_LENGTH(package) - 0x10) ) )
    {
        return; // Don't try to patch a bad mkext
    }	
		
	
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
		mkext2_header*	package = packagetmp;
		z_stream		zstream;
		bool			zstream_inited = false;
		int				zlib_result;
		int				plist_offset = MKEXT2_GET_PLIST(package);
		
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
			TagPtr plistData;
			TagPtr allDicts;
			bzero(&plistData, sizeof(plistData));
			bzero(&allDicts, sizeof(allDicts));
			
			XMLParseFile( plist, &plistData );

			int count;

			allDicts = XMLGetProperty(plistData, kMKEXTInfoDictionariesKey);
			
			bool patched = false;
			for(count = XMLTagCount(allDicts);
				count > 0;
				count--)
			{
				TagPtr kextEntry = XMLGetElement(allDicts, count);
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
				
				
				
				// re adler32 the new mkext2 package
				MKEXT_HDR_CAST(package)->adler32 = MKEXT_SWAP(Adler32((unsigned char *)&package->version,
																	  MKEXT_GET_LENGTH(package) - 0x10));
			}
		}
		else
		{
			printf("ZLIB Error: %s\n", zstream.msg);
			getc();
		}
	}
}

// FIXME: only handles mkext2 entries
bool patch_kext(TagPtr plist, char* plistbuffer, void* start)
{
	char* bundleID;
	
	if(XMLGetProperty(plist, kMKEXTExecutableKey) == NULL) return false;	// Kext is a plist only kext, don't patch
	
	bundleID = XMLCastString(XMLGetProperty(plist, kPropCFBundleIdentifier));
	
	
	if(patch_gma_deviceid &&
	    ((strcmp(bundleID, "com.apple.driver.AppleIntelGMA950") == 0) ||
		 (strcmp(bundleID, "com.apple.driver.AppleIntelIntegratedFramebuffer") == 0)))
	{
		return patch_gma_kexts(plist, plistbuffer, start);
	}
	else if(patch_bcm_deviceid && (strcmp(bundleID, "com.apple.driver.AirPortBrcm43xx") == 0))
	{
		return patch_bcm_kext(plist, plistbuffer, start);
	}
	else if(patch_hda_codec && strcmp(bundleID, "com.apple.driver.AppleHDA") == 0)
	{
		return patch_hda_kext(plist, plistbuffer, start);
	}
	else if(patch_hda_codec && strcmp(bundleID, "com.apple.driver.AppleHDAController") == 0)
	{
		return patch_hda_controller(plist, plistbuffer, start);
	}
	else if(patch_atheros_deviceid && strcmp(bundleID, "com.apple.driver.AirPort.Atheros21") == 0)
	{
		return patch_atheros_kext(plist, plistbuffer, start);
	}
	
	return false;
}

void KextPatcher_hook(void* arg1, void* arg2, void* arg3, void* arg4)
{
	pci_dt_t* current = arg1;
	if(current)
	{
		switch(current->class_id)
		{
			case PCI_CLASS_DISPLAY_VGA:
				if(current->vendor_id == 0x8086 && 
				   (
					current->device_id == 0x27AE ||
					current->device_id == 0xA001 ||
					current->device_id == 0xA002 ||
					current->device_id == 0xA011 ||
					current->device_id == 0xA012

					)
				   )
				{
					patch_gma_deviceid = current->device_id;
				}
				break;
				
			case PCI_CLASS_NETWORK_OTHER:
				
				// Patch BCM43xx
				if(current->vendor_id == 0x14E4 && ((current->device_id & 0xFFD0) == 0x4300))
				{
					patch_bcm_deviceid = current->device_id;
				}
				else if(current->vendor_id == 0x168C && current->device_id == 0x002B)
				{
					patch_atheros_deviceid = current->device_id;
				}
				break;
		}
	}
}


bool patch_hda_controller(TagPtr plist, char* plistbuffer, void* start)
{
	return false;
	// change the PCI class code to match to. Note: A LegacyHDA plist should do this on it's own
	// As such, it's disabled

	// TODO: read class code
	TagPtr personality;
	personality =		XMLCastDict(XMLGetProperty(plist, kPropIOKitPersonalities));
	personality =		XMLGetProperty(personality, (const char*)"BuiltInHDA");	
	TagPtr match_class =XMLCastArray(XMLGetProperty(personality, (const char*)"IOPCIClassMatch"));
	
	
	char* new_str = malloc(sizeof("0xXXXX000&0xFFFE0000"));
	sprintf(new_str, "0x04030000&amp;0xFFFE0000"); // todo, pass in actual class id
	
	
	char* orig_string = "0x04020000&amp;0xFFFE0000"; //XMLCastString(match_class);
	
	printf("Attemting to replace '%s' with '%s'\n", orig_string, new_str);
	
	// TODO: verify string doesn't exist first.
	
	replace_string(orig_string, new_str, plistbuffer + XMLCastStringOffset(match_class), 1024);
	
	return true;
	
}


bool patch_hda_kext(TagPtr plist, char* plistbuffer, void* start)
{
	uint16_t find_codec	= 0;
	int full_size, compressed_size, executable_offset;
	void* compressed_data;
	mkext2_file_entry* kext;
	int zlib_result;
	z_stream       zstream;
	bool           zstream_inited = false;

	switch(patch_hda_codec & 0xFF00)
	{
		case 0x0200:
			find_codec = 0x0262;
			break;

		case 0x0800:
			find_codec = 0x0885;
			break;
			
		case 0x0600:	// specificaly the 662
			find_codec = 0x0885;
			break;
	}
	if(!find_codec) return false;	// notify caller that we aren't patching the kext
		
	executable_offset	= XMLCastInteger(XMLGetProperty(plist, kMKEXTExecutableKey));
	kext				= (void*)((char*)start + executable_offset);
	full_size			= MKEXT2_GET_ENTRY_FULLSIZE(kext);
	compressed_size		= MKEXT2_GET_ENTRY_COMPSIZE(kext);
	compressed_data		= MKEXT2_GET_ENTRY_DATA(kext);	
	executable_offset	= XMLCastInteger(XMLGetProperty(plist, kMKEXTExecutableKey));
	
	
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
		
	/*int times = */replace_word(0x10EC0000 | (find_codec), 0x10EC0000 | (patch_hda_codec), executable, zstream.total_out);
	if (zstream_inited) inflateEnd(&zstream);
	
	
	zstream.next_in   = (UInt8*)executable;
	zstream.next_out  = (UInt8*)compressed_data;
	
	zstream.avail_in  = full_size;
	zstream.avail_out = compressed_size<<1;
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
		DBG("Deflated result is %d, avail: %d bytes, out: %d bytes, full: %d\n", zlib_result, compressed_size, zstream.total_out, full_size);
	} 
	else if (zlib_result == Z_OK)
	{
		/* deflate filled output buffer, meaning the data doesn't compress.
		 */
		DBG("Buffer FULL: deflated result is %d, avail: %d bytes, out: %d bytes, full: %d\n", zlib_result, compressed_size, zstream.total_out, full_size);
		printf("Unable to patch AppleHDA\n");
		
	} 
	else if (zlib_result != Z_STREAM_ERROR)
	{
		printf("AppleHDA: ZLIB Deflate Error: %s\n", zstream.msg);
		getc();
	}
	
	if (zstream_inited) deflateEnd(&zstream);
	
	if(zstream.total_out < compressed_size) kext->compressed_size = MKEXT_SWAP(zstream.total_out);

	
	free(executable);
	
	return true;	

}


/** Patches an array element within a personality. NOTE: string sizes should match **/
bool patch_plist_entry(TagPtr plist, char* plistbuffer, const char* personalityName, const char* propertyName, const char* nameMatch)
{
	TagPtr personality	= XMLGetProperty(XMLCastDict(XMLGetProperty(plist, kPropIOKitPersonalities)), personalityName);	
	TagPtr match_names	= XMLCastArray(XMLGetProperty(personality, propertyName));
	TagPtr replace		= XMLGetElement(match_names, 0);	// Modify the first entry
	int count			= XMLTagCount(match_names);
	
	while(count)
	{
		char* orig_string	= XMLCastString(XMLGetElement(match_names, --count));
		if(strcmp(orig_string, nameMatch) == 0) return false;			// Entry already exists, no need tmo modify plist + recompress
	}

	char* orig_string	= XMLCastString(replace);
	verbose("Patching %s, replacing %s with %s\n", personalityName, orig_string, nameMatch);
	replace_string(orig_string, nameMatch, plistbuffer + XMLCastStringOffset(replace), 10240);
	
}

bool patch_bcm_kext(TagPtr plist, char* plistbuffer, void* start)
{
	char* new_str		= malloc(sizeof("pci14e4,xxxx"));	
	sprintf(new_str, "pci14e4,%02x", patch_bcm_deviceid);
	return patch_plist_entry(plist, plistbuffer, "Broadcom 802.11 PCI", "IONameMatch", new_str);
}

bool patch_atheros_kext(TagPtr plist, char* plistbuffer, void* start)
{
	char* new_str		= malloc(sizeof("pci14e4,xxxx"));	
	sprintf(new_str, "pci168c,%02x", patch_atheros_deviceid);
	return patch_plist_entry(plist, plistbuffer, "Atheros Wireless LAN PCI", "IONameMatch", new_str);
}

bool patch_gma_kexts(TagPtr plist, char* plistbuffer, void* start)
{
	TagPtr personality			= XMLCastDict(XMLGetProperty(plist, kPropIOKitPersonalities));
	int exeutable_offset		= XMLCastInteger(XMLGetProperty(plist, kMKEXTExecutableKey));
	mkext2_file_entry* kext		= (void*)((char*)start + exeutable_offset);
	int full_size				= MKEXT2_GET_ENTRY_FULLSIZE(kext);
	int compressed_size			= MKEXT2_GET_ENTRY_COMPSIZE(kext);
	void* compressed_data		= MKEXT2_GET_ENTRY_DATA(kext);
	char* executable			= malloc(full_size);
	int zlib_result				= Z_OK;
	long offset;
	z_stream zstream;

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
		zlib_result = inflate(&zstream, Z_FINISH);
		
		DBG("Inflated result is %d, in: %d bytes, out: %d bytes, full: %d\n", zlib_result, zstream.total_in, zstream.total_out, full_size);
		
		char* newstring = malloc(sizeof("0x00008086"));
		sprintf(newstring, "0x%04x", 0x8086 | (patch_gma_deviceid << 16));
		
		
		if(XMLGetProperty(personality, (const char*)"Intel915"))
		{
			if((patch_gma_deviceid & 0xFF00) != 0xA000)	// not GMA3150
			{
				verbose("Patching AppleIntelGMA950.kext\n");
				personality =	XMLGetProperty(personality, (const char*)"Intel915"); // IOAccelerator kext
				offset =		XMLCastStringOffset(XMLGetProperty(personality, (const char*)"IOPCIPrimaryMatch"));		
				
				replace_string("0x27A28086", newstring, plistbuffer + offset, 10240);
				replace_word(0x27A28086, 0x8086 | (patch_gma_deviceid << 16), executable, zstream.total_out);
			}
		}
		else if(XMLGetProperty(personality, (const char*)"AppleIntelIntegratedFramebuffer"))
		{
			verbose("Patching AppleIntelIntegratedFramebuffer\n");
			personality =	XMLGetProperty(personality, (const char*)"AppleIntelIntegratedFramebuffer");
			offset =		XMLCastStringOffset(XMLGetProperty(personality, (const char*)"IOPCIPrimaryMatch"));		
			
			if((patch_gma_deviceid & 0xFF00) == 0xA000)	// GMA3150
			{
				// Cursor corruption fix.
				// This patch changes the cursor address from
				// a physical address (used in the gma950) to an offset (used in the gma3150).
				//					{0x8b, 0x55, 0x08, 0x83, 0xba, 0xb0, 0x00, 0x00, 0x00, 0x01, 0x7e, 0x36, 0x89, 0x04, 0x24, 0xe8, 0x6b, 0xbc, 0xff, 0xff};
				char find_bytes[] = {0x8b, 0x55, 0x08, 0x83, 0xba, 0xb0, 0x00, 0x00, 0x00, 0x01, 0x7e, 0x36, 0x89, 0x04, 0x24, 0xe8/*, 0x32, 0xbb, 0xff, 0xff*/};	// getPhysicalAddress() and more
				char new_bytes[]  = {0xb8, 0x00, 0x00, 0x00, 0x02, 0xEB, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	// jump past getPhysicalAddress binding. NOTE: last six bytes are unusable, set to 0 for compression
				replace_bytes(find_bytes, sizeof(find_bytes), new_bytes, sizeof(new_bytes), executable, zstream.total_out);
			}
			replace_string("0x27A28086", newstring, plistbuffer + offset, 10240);
			replace_word(0x27A28086, 0x8086 | (patch_gma_deviceid << 16), executable, zstream.total_out);
			
		}
		else if(XMLGetProperty(personality, (const char*)"Intel965"))
		{
			verbose("Patching AppleIntelGMAX3100.kext\n");
			personality =	XMLGetProperty(personality, (const char*)"Intel965");
			offset =		XMLCastStringOffset(XMLGetProperty(personality, (const char*)"IOPCIPrimaryMatch"));		
			
			replace_string("0x2a028086", newstring, plistbuffer + offset, 10240);
		}
		else if(XMLGetProperty(personality, (const char*)"AppleIntelGMAX3100FB"))
		{
			verbose("Patching AppleIntelGMAX3100FB.kext\n");
			personality =	XMLGetProperty(personality, (const char*)"AppleIntelGMAX3100FB");
			
			offset =		XMLCastStringOffset(XMLGetProperty(personality, (const char*)"IOPCIPrimaryMatch"));		
			replace_string("0x2A028086", newstring, plistbuffer + offset, 10240);
			replace_word(0x2A028086, 0x8086 | (patch_gma_deviceid << 16), executable, zstream.total_out);
		}
		else
		{
			return false;
		}
		
		
		inflateEnd(&zstream);
		
		// Recompress the executable
		zstream.next_in   = (UInt8*)executable;
		zstream.next_out  = (UInt8*)compressed_data;
		
		zstream.avail_in  = full_size;
		zstream.avail_out = compressed_size;
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
			zlib_result = deflate(&zstream, Z_FINISH);
			
			if (zlib_result == Z_STREAM_END)
			{
				DBG("Deflated result is %d, avail: %d bytes, out: %d bytes, full: %d\n", zlib_result, compressed_size, zstream.total_out, full_size);
			} 
			else if (zlib_result == Z_OK)
			{
				/* deflate filled output buffer, meaning the data doesn't compress.
				 */
				printf("Deflated result is %d, in: %d bytes, out: %d bytes, full: %d\n", zlib_result, zstream.total_in, zstream.total_out, full_size);
				printf("ERROR: Unable to compress patched kext, not enough room.\n");
				pause();
				
			} 
			else if (zlib_result != Z_STREAM_ERROR)
			{
				printf("ZLIB Deflate Error: %s\n", zstream.msg);
				getc();
			}
			if(zstream.total_out < compressed_size) kext->compressed_size = MKEXT_SWAP(zstream.total_out);
			
			
			deflateEnd(&zstream);
		}
	}
	
	free(executable);
	
	return true;	
}

int chartohex(char c)
{
	if(c <= '9' && c >= '0')		return c - '0';	// c is between 0 and 9
	else if(c <= 'F' && c >= 'A')	return c - 'A' + 10; // c = 10 - 15;
	else if(c <= 'f' && c >= 'a')	return c - 'a' + 10; // c = 10 - 15;

	return 0;
}