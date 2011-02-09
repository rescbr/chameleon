/*
 * Copyright (c) 2009 Evan Lojewski. All rights reserved.
 *
 */
#ifndef __BOOT2_KERNEL_PATCHER_H
#define __BOOT2_KERNEL_PATCHER_H

#define kHDACodec				"HDACodec"

unsigned long Adler32( unsigned char * buffer, long length );

void KextPatcher_start();
void KextPatcher_hook(void* current, void* arg2, void* arg3, void* arg4);


/** Patch Routines **/
bool patch_plist_entry(TagPtr plist, char* plistbuffer, const char* personalityName, const char* propertyName, const char* nameMatch);

bool patch_kext(TagPtr plist, char* plistbuffer, void* start);
bool patch_gma_kexts(TagPtr plist, char* plistbuffer, void* start);
bool patch_bcm_kext(TagPtr plist, char* plistbuffer, void* start);
bool patch_atheros_kext(TagPtr plist, char* plistbuffer, void* start);
bool patch_hda_kext(TagPtr plist, char* plistbuffer, void* start);
bool patch_hda_controller(TagPtr plist, char* plistbuffer, void* start);


/** zlib support **/
static void * z_alloc(void *, u_int items, u_int size);
static void   z_free(void *, void *ptr);

typedef struct z_mem {
    uint32_t alloc_size;
    uint8_t  data[0];
} z_mem;



int chartohex(char c);

void kext_loaded(void* module, void* length, void* executableAddr, void* arg3);
void mkext_loaded(void* filespec, void* package, void* lenght, void* arg3);


/** Global patch variables **/
extern uint16_t patch_gma_deviceid;
extern uint16_t patch_bcm_deviceid;
extern uint16_t patch_atheros_deviceid;
extern uint16_t patch_hda_codec;		// TODO; detect proper codec

#define NEEDS_PATCHING		(patch_bcm_deviceid || patch_gma_deviceid || patch_hda_codec || patch_atheros_deviceid)



#endif /* !__BOOT2_KERNEL_PATCHER_H */
