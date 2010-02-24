/* Copied from 915 resolution created by steve tomljenovic
 * This source code is into the public domain.
 *
 * Included to Chameleon RC3 by meklort
 *
 * Included to RC4 and edited by deviato to match more intel chipsets
 *
 */

/* Copied from 915 resolution created by steve tomljenovic
 *
 * This code is based on the techniques used in :
 *
 *   - 855patch.  Many thanks to Christian Zietz (czietz gmx net)
 *     for demonstrating how to shadow the VBIOS into system RAM
 *     and then modify it.
 *
 *   - 1280patch by Andrew Tipton (andrewtipton null li).
 *
 *   - 855resolution by Alain Poirier
 *
 * This source code is into the public domain.
 */
#ifndef __915_RESOLUTION_H
#define __915_RESOLUTION_H

#define NEW(a) ((a *)(malloc(sizeof(a))))
#define FREE(a) (free(a))

#define FALSE 0
#define TRUE 1

#define VBIOS_START         0x0
#define VBIOS_SIZE          0x10000


//#define MODE_TABLE_OFFSET_845G 617


#define ATI_SIGNATURE1 "ATI MOBILITY RADEON"
#define ATI_SIGNATURE2 "ATI Technologies Inc"
#define NVIDIA_SIGNATURE "NVIDIA Corp"
#define INTEL_SIGNATURE "Intel Corp"




typedef enum {
	CT_UNKWN, CT_830, CT_845G, CT_855GM, CT_865G, CT_915G, CT_915GM, CT_945G, CT_945GM, CT_945GME,
	CT_946GZ, CT_G965, CT_Q965, CT_965GM, CT_GM45, CT_G41, CT_G31, CT_G45, CT_500
} chipset_type;


typedef enum {
	BT_UNKWN, BT_1, BT_2, BT_3
} bios_type;


typedef struct {
	UInt8 mode;
	UInt8 bits_per_pixel;
	UInt16 resolution;
	UInt8 unknown;
} __attribute__((packed)) vbios_mode;

typedef struct {
	UInt8 unknow1[2];
	UInt8 x1;
	UInt8 x_total;
	UInt8 x2;
	UInt8 y1;
	UInt8 y_total;
	UInt8 y2;
} __attribute__((packed)) vbios_resolution_type1;

typedef struct {
	unsigned long clock;
	
	UInt16 x1;
	UInt16 htotal;
	UInt16 x2;
	UInt16 hblank;
	UInt16 hsyncstart;
	UInt16 hsyncend;
	UInt16 y1;
    UInt16 vtotal;
    UInt16 y2;
	UInt16 vblank;
	UInt16 vsyncstart;
	UInt16 vsyncend;
} __attribute__((packed)) vbios_modeline_type2;

typedef struct {
	UInt8 xchars;
	UInt8 ychars;
	UInt8 unknown[4];
	
	vbios_modeline_type2 modelines[];
} __attribute__((packed)) vbios_resolution_type2;

typedef struct {
	unsigned long clock;
	
	UInt16 x1;
	UInt16 htotal;
	UInt16 x2;
	UInt16 hblank;
	UInt16 hsyncstart;
	UInt16 hsyncend;
	
	UInt16 y1;
	UInt16 vtotal;
	UInt16 y2;
	UInt16 vblank;
	UInt16 vsyncstart;
	UInt16 vsyncend;
	
	UInt16 timing_h;
	UInt16 timing_v;
	
	UInt8 unknown[6];
} __attribute__((packed)) vbios_modeline_type3;

typedef struct {
	unsigned char unknown[6];
	
    vbios_modeline_type3 modelines[];
} __attribute__((packed)) vbios_resolution_type3;

typedef struct {
	UInt32 chipset_id;
	chipset_type chipset;
	bios_type bios;
	
	UInt32 bios_fd;
	char* bios_ptr;
	
	vbios_mode * mode_table;
	UInt32 mode_table_size;
	UInt8 b1, b2;
	
	UInt8 unlocked;
} vbios_map;



void display_map_info(vbios_map*);
vbios_map * open_vbios(chipset_type);
void close_vbios (vbios_map*);
void unlock_vbios(vbios_map*);
void relock_vbios(vbios_map*);
void set_mode(vbios_map*, UInt32, UInt32, UInt32, UInt32, UInt32);
void list_modes(vbios_map *map, UInt32 raw);

#endif
