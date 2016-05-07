/*
 * Copyright 2013 Intel Corporation
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
	Original patch by Nawcom
	http://forum.voodooprojects.org/index.php/topic,1029.0.html

	Original Intel HDx000 code from valv
	Intel Ivy Bridge, Haswell and Broadwell code from ErmaC:
	- http://www.insanelymac.com/forum/topic/288241-intel-hd4000-inject-aaplig-platform-id/
*/

#ifndef __LIBSAIO_GMA_H
#define __LIBSAIO_GMA_H

bool setup_gma_devprop(pci_dt_t *gma_dev);

struct intel_gfx_info_t;
typedef struct{
    uint32_t	model;
	char		*label_info;
}intel_gfx_info_t;

#define REG8(reg)	((volatile uint8_t *)regs)[(reg)]
#define REG16(reg)	((volatile uint16_t *)regs)[(reg) >> 1]
#define REG32(reg)	((volatile uint32_t *)regs)[(reg) >> 2]

/****************************************************************************
 * Miscellanious defines
 ****************************************************************************/

/* Intel gfx Controller models */
#define GFX_MODEL_CONSTRUCT(vendor, model) (((uint32_t)(model) << 16) | ((vendor##_VENDORID) & 0xffff))

/* Intel */
#define INTEL_NAME          "Intel"
#define HD_GRAPHICS         "HD Graphics"
#define HD_GRAPHICS_2000    "HD Graphics 2000"
#define HD_GRAPHICS_2500    "HD Graphics 2500"
#define HD_GRAPHICS_3000    "HD Graphics 3000"
#define HD_GRAPHICS_4000    "HD Graphics 4000"
#define HD_GRAPHICS_4200    "HD Graphics 4200"
#define HD_GRAPHICS_4400    "HD Graphics 4400"
#define HD_GRAPHICS_4600    "HD Graphics 4600"
#define HD_GRAPHICS_5000    "HD Graphics 5000"
#define IRIS_5100           "Iris(TM) Graphics 5100"
#define IRIS_5200           "Iris(TM) Pro Graphics 5200"
#define HD_GRAPHICS_5300    "HD Graphics 5300"
#define HD_GRAPHICS_5500    "HD Graphics 5500"
#define HD_GRAPHICS_5600    "HD Graphics 5600"
#define HD_GRAPHICS_6000    "HD Graphics 6000"
#define IRIS_6100           "Iris Graphics 6100"
#define IRIS_6200           "Iris Pro Graphics 6200"
#define IRIS_6300           "Iris Pro Graphics P6300"
#define HD_GRAPHICS_510     "HD Graphics 510"
#define HD_GRAPHICS_515     "HD Graphics 515"
#define HD_GRAPHICS_520     "HD Graphics 520"
#define HD_GRAPHICS_P530    "HD Graphics P530"
#define HD_GRAPHICS_530     "HD Graphics 530"
#define HD_GRAPHICS_535     "HD Graphics 535"
#define HD_GRAPHICS_550     "HD Graphics 550"
#define IRIS_540            "Iris(TM) Graphics 540"
#define IRIS_570_580        "Iris(TM) Pro Graphics 570/580"
#define IRIS_580            "Iris(TM) Pro Graphics 580"
#define IRIS                "Iris(TM) Graphics"
#define IRIS_P580           "Iris(TM) Pro Graphics P580"
#define INTEL_VENDORID      PCI_VENDOR_ID_INTEL

/* http://cgit.freedesktop.org/xorg/driver/xf86-video-intel/tree/src/intel_driver.h */
/* http://people.redhat.com/agk/patches/linux/patches-3.6/git-update1.patch */
            
#define GMA_I810                   GFX_MODEL_CONSTRUCT(INTEL, 0x7121)
#define GMA_I810_DC100             GFX_MODEL_CONSTRUCT(INTEL, 0x7123)
#define GMA_I810_E                 GFX_MODEL_CONSTRUCT(INTEL, 0x7125)
#define GMA_I815                   GFX_MODEL_CONSTRUCT(INTEL, 0x1132)
/* ==================================== */
// Cherryview (Braswell, Cherry Trail)
// #define GMA_ GFX_MODEL_CONSTRUCT(INTEL, 0x22B0) // Intel(R) HD Graphics
// #define GMA_ GFX_MODEL_CONSTRUCT(INTEL, 0x22B1) // Intel(R) HD Graphics
// #define GMA_ GFX_MODEL_CONSTRUCT(INTEL, 0x22B2) // Intel(R) HD Graphics
// #define GMA_ GFX_MODEL_CONSTRUCT(INTEL, 0x22B3) // Intel(R) HD Graphics

/* ==================================== */

#define GMA_I830_M                 GFX_MODEL_CONSTRUCT(INTEL, 0x3577)
#define GMA_845_G                  GFX_MODEL_CONSTRUCT(INTEL, 0x2562)
#define GMA_I854                   GFX_MODEL_CONSTRUCT(INTEL, 0x358E)
#define GMA_I855_GM                GFX_MODEL_CONSTRUCT(INTEL, 0x3582)
#define GMA_I865_G                 GFX_MODEL_CONSTRUCT(INTEL, 0x2572)
/* ==================================== */

#define GMA_I915_G                 GFX_MODEL_CONSTRUCT(INTEL, 0x2582) // GMA 915
#define GMA_I915_GM                GFX_MODEL_CONSTRUCT(INTEL, 0x2592) // GMA 915
#define GMA_E7221_G                GFX_MODEL_CONSTRUCT(INTEL, 0x258A)
#define GMA_I945_G                 GFX_MODEL_CONSTRUCT(INTEL, 0x2772) // Desktop GMA950
//#define GMA_82945G                 GFX_MODEL_CONSTRUCT(INTEL, 0x2776) // Desktop GMA950
//#define GMA_82915G                 GFX_MODEL_CONSTRUCT(INTEL, 0x2782) // GMA 915
//#define GMA_038000                 GFX_MODEL_CONSTRUCT(INTEL, 0x2792) // Mobile GMA915
#define GMA_I945_GM                GFX_MODEL_CONSTRUCT(INTEL, 0x27A2) // Mobile GMA950
#define GMA_I945_GME               GFX_MODEL_CONSTRUCT(INTEL, 0x27AE) // Mobile GMA950
//#define GMA_945GM               GFX_MODEL_CONSTRUCT(INTEL, 0x27A6) // Mobile GMA950
//#define GMA_PINEVIEW_M_HB             GFX_MODEL_CONSTRUCT(INTEL, 0xA010)
#define GMA_PINEVIEW_M             GFX_MODEL_CONSTRUCT(INTEL, 0xA011) // Mobile GMA3150
#define GMA_GMA3150_M              GFX_MODEL_CONSTRUCT(INTEL, 0xA012) // Mobile GMA3150
//#define GMA_PINEVIEW_HB             GFX_MODEL_CONSTRUCT(INTEL, 0xA000)
#define GMA_PINEVIEW_G             GFX_MODEL_CONSTRUCT(INTEL, 0xA001) // Mobile GMA3150
#define GMA_GMA3150_D              GFX_MODEL_CONSTRUCT(INTEL, 0xA002) // Desktop GMA3150
#define GMA_Q35_G                  GFX_MODEL_CONSTRUCT(INTEL, 0x29B2)
#define GMA_G33_G                  GFX_MODEL_CONSTRUCT(INTEL, 0x29C2) // Desktop GMA3100
// 29C3 // Desktop GMA3100
#define GMA_Q33_G                  GFX_MODEL_CONSTRUCT(INTEL, 0x29D2)
/* ==================================== */

#define GMA_G35_G                  GFX_MODEL_CONSTRUCT(INTEL, 0x2982)
#define GMA_I965_Q                 GFX_MODEL_CONSTRUCT(INTEL, 0x2992)
#define GMA_I965_G                 GFX_MODEL_CONSTRUCT(INTEL, 0x29A2)
#define GMA_I946_GZ                GFX_MODEL_CONSTRUCT(INTEL, 0x2972)
#define GMA_I965_GM                GFX_MODEL_CONSTRUCT(INTEL, 0x2A02) // GMAX3100
#define GMA_I965_GME               GFX_MODEL_CONSTRUCT(INTEL, 0x2A12) // GMAX3100
#define GMA_GM45_GM                GFX_MODEL_CONSTRUCT(INTEL, 0x2A42) // GMAX3100
//#define GMA_GM45_GM2                GFX_MODEL_CONSTRUCT(INTEL, 0x2A43) // GMAX3100
#define GMA_G45_E_G                GFX_MODEL_CONSTRUCT(INTEL, 0x2E02)
#define GMA_G45_G                  GFX_MODEL_CONSTRUCT(INTEL, 0x2E22)
#define GMA_Q45_G                  GFX_MODEL_CONSTRUCT(INTEL, 0x2E12)
#define GMA_G41_G                  GFX_MODEL_CONSTRUCT(INTEL, 0x2E32)
#define GMA_B43_G                  GFX_MODEL_CONSTRUCT(INTEL, 0x2E42)
#define GMA_B43_G1                 GFX_MODEL_CONSTRUCT(INTEL, 0x2E92)

#define GMA_IRONLAKE_D_G           GFX_MODEL_CONSTRUCT(INTEL, 0x0042) // HD2000
#define GMA_IRONLAKE_M_G           GFX_MODEL_CONSTRUCT(INTEL, 0x0046) // HD2000
/*
#define GMA_IRONLAKE_D_HB          GFX_MODEL_CONSTRUCT(INTEL, 0x0040)
#define GMA_IRONLAKE_D2_HB         GFX_MODEL_CONSTRUCT(INTEL, 0x0069)
#define GMA_IRONLAKE_M_HB          GFX_MODEL_CONSTRUCT(INTEL, 0x0044)
#define GMA_IRONLAKE_MA_HB         GFX_MODEL_CONSTRUCT(INTEL, 0x0062)
#define GMA_IRONLAKE_MC2_HB        GFX_MODEL_CONSTRUCT(INTEL, 0x006a)
*/
// 004A // HD2000
/* ==================================== */

/* ========== Sandy Bridge ============ */
//#define GMA_SANDYBRIDGE_HB        GFX_MODEL_CONSTRUCT(INTEL, 0x0100) /* Desktop */
#define GMA_SANDYBRIDGE_GT1        GFX_MODEL_CONSTRUCT(INTEL, 0x0102) // HD Graphics 2000
//#define GMA_SANDYBRIDGE_M_HB        GFX_MODEL_CONSTRUCT(INTEL, 0x0104) /* Mobile */
#define GMA_SANDYBRIDGE_GT2        GFX_MODEL_CONSTRUCT(INTEL, 0x0112) // HD Graphics 3000
#define GMA_SANDYBRIDGE_GT2_PLUS	GFX_MODEL_CONSTRUCT(INTEL, 0x0122) // HD Graphics 3000
#define GMA_SANDYBRIDGE_M_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0106) // HD Graphics 2000 Mobile
#define GMA_SANDYBRIDGE_M_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0116) // HD Graphics 3000 Mobile
#define GMA_SANDYBRIDGE_M_GT2_PLUS	GFX_MODEL_CONSTRUCT(INTEL, 0x0126) // HD Graphics 3000 Mobile
//#define GMA_SANDYBRIDGE_S_HB     GFX_MODEL_CONSTRUCT(INTEL, 0x0108) /* Server */
#define GMA_SANDYBRIDGE_S_GT       GFX_MODEL_CONSTRUCT(INTEL, 0x010A) // HD Graphics
// 010B // ??
// 010E // ??
/* ==================================== */

/* ========== Ivy Bridge ============== */
//#define GMA_IVYBRIDGE_HB        GFX_MODEL_CONSTRUCT(INTEL, 0x0150)  /* Desktop */
//#define GMA_IVYBRIDGE_M_HB        GFX_MODEL_CONSTRUCT(INTEL, 0x0154)  /* Mobile */
#define GMA_IVYBRIDGE_M_GT1        GFX_MODEL_CONSTRUCT(INTEL, 0x0156) // HD Graphics 2500 Mobile
#define GMA_IVYBRIDGE_M_GT2        GFX_MODEL_CONSTRUCT(INTEL, 0x0166) // HD Graphics 4000 Mobile
#define GMA_IVYBRIDGE_D_GT1        GFX_MODEL_CONSTRUCT(INTEL, 0x0152) // HD Graphics 2500
#define GMA_IVYBRIDGE_D_GT2        GFX_MODEL_CONSTRUCT(INTEL, 0x0162) // HD Graphics 4000
//#define GMA_IVYBRIDGE_S_HB        GFX_MODEL_CONSTRUCT(INTEL, 0x0158) /* Server */
#define GMA_IVYBRIDGE_S_GT1        GFX_MODEL_CONSTRUCT(INTEL, 0x015A) // HD Graphics 4000
#define GMA_IVYBRIDGE_S_GT2        GFX_MODEL_CONSTRUCT(INTEL, 0x016A) // HD Graphics P4000
#define GMA_IVYBRIDGE_S_GT3        GFX_MODEL_CONSTRUCT(INTEL, 0x015E) // Xeon E3-1200 v2/3rd Gen Core processor Graphics Controller
#define GMA_IVYBRIDGE_S_GT4        GFX_MODEL_CONSTRUCT(INTEL, 0x0172) // HD Graphics 2500 Mobile // Xeon E3-1200 v2/3rd Gen Core processor Graphics Controller
#define GMA_IVYBRIDGE_S_GT5        GFX_MODEL_CONSTRUCT(INTEL, 0x0176) // HD Graphics 2500 Mobile // 3rd Gen Core processor Graphics Controller
/* ==================================== */

/* ======  Valleyview (Bay Trail) ======= */

//#define GMA_VALLEYVIEW_0F00          GFX_MODEL_CONSTRUCT(INTEL, 0x0F00) /* VLV1 */
//#define GMA_VALLEYVIEW_0F30          GFX_MODEL_CONSTRUCT(INTEL, 0x0F30) /* "HD Graphics" */
//#define GMA_VALLEYVIEW_0F31          GFX_MODEL_CONSTRUCT(INTEL, 0x0F31) /* "HD Graphics" */
//#define GMA_VALLEYVIEW_0F32          GFX_MODEL_CONSTRUCT(INTEL, 0x0F32) /* "HD Graphics" */
//#define GMA_VALLEYVIEW_0F33          GFX_MODEL_CONSTRUCT(INTEL, 0x0F33) /* "HD Graphics" */
//#define GMA_VALLEYVIEW_0155          GFX_MODEL_CONSTRUCT(INTEL, 0x0155) /* "HD Graphics" */
//#define GMA_VALLEYVIEW_0157          GFX_MODEL_CONSTRUCT(INTEL, 0x0157) /* "HD Graphics" */
/* ==================================== */

/* ============ Haswell =============== */
// 0090 // AppleIntelHD5000Graphics.kext
// 0091 // AppleIntelHD5000Graphics.kext
// 0092 // AppleIntelHD5000Graphics.kext
//#define GMA_HASWELL_HB          GFX_MODEL_CONSTRUCT(INTEL, 0x0400) /* Desktop */
#define GMA_HASWELL_D_GT1          GFX_MODEL_CONSTRUCT(INTEL, 0x0402) // GT1 Desktop
#define GMA_HASWELL_D_GT2          GFX_MODEL_CONSTRUCT(INTEL, 0x0412) // Haswell GT2 Desktop
#define GMA_HASWELL_D_GT3          GFX_MODEL_CONSTRUCT(INTEL, 0x0422) // GT3 Desktop
//#define GMA_HASWELL_M_HB          GFX_MODEL_CONSTRUCT(INTEL, 0x0404) /* Mobile */
#define GMA_HASWELL_M_GT1          GFX_MODEL_CONSTRUCT(INTEL, 0x0406) // Haswell Mobile GT1
#define GMA_HASWELL_M_GT2          GFX_MODEL_CONSTRUCT(INTEL, 0x0416) // Haswell Mobile GT2
#define GMA_HASWELL_M_GT3          GFX_MODEL_CONSTRUCT(INTEL, 0x0426) // Haswell Mobile GT3
#define GMA_HASWELL_S_GT1          GFX_MODEL_CONSTRUCT(INTEL, 0x040A) // Intel(R) HD Graphics
//#define GMA_HASWELL_S_HB          GFX_MODEL_CONSTRUCT(INTEL, 0x0408) /* Server */
#define GMA_HASWELL_S_GT2          GFX_MODEL_CONSTRUCT(INTEL, 0x041A) // Intel(R) HD Graphics P4600/P4700
#define GMA_HASWELL_S_GT3          GFX_MODEL_CONSTRUCT(INTEL, 0x042A) // GT3 Server
#define GMA_HASWELL_B_GT1          GFX_MODEL_CONSTRUCT(INTEL, 0x040B)
#define GMA_HASWELL_B_GT2          GFX_MODEL_CONSTRUCT(INTEL, 0x041B)
#define GMA_HASWELL_B_GT3          GFX_MODEL_CONSTRUCT(INTEL, 0x042B)
#define GMA_HASWELL_E_GT1          GFX_MODEL_CONSTRUCT(INTEL, 0x040E)
#define GMA_HASWELL_E_GT2          GFX_MODEL_CONSTRUCT(INTEL, 0x041E)
#define GMA_HASWELL_E_GT3          GFX_MODEL_CONSTRUCT(INTEL, 0x042E)

#define GMA_HASWELL_ULT_D_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0A02)
#define GMA_HASWELL_ULT_D_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0A12)
#define GMA_HASWELL_ULT_D_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0A22) //
#define GMA_HASWELL_ULT_M_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0A06) // GT1 ULT
#define GMA_HASWELL_ULT_M_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0A16) // Haswell ULT Mobile GT2
#define GMA_HASWELL_ULT_M_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0A26) // Haswell ULT Mobile GT3 - Intel(R) HD Graphics 5000
#define GMA_HASWELL_ULT_S_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0A0A)
#define GMA_HASWELL_ULT_S_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0A1A)
#define GMA_HASWELL_ULT_S_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0A2A)
#define GMA_HASWELL_ULT_B_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0A0B)
#define GMA_HASWELL_ULT_B_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0A1B)
#define GMA_HASWELL_ULT_B_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0A2B)

#define GMA_HASWELL_ULT_E_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0A0E) // Intel(R) HD Graphics
#define GMA_HASWELL_ULT_E_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0A1E) // Intel(R) HD Graphics 4400
#define GMA_HASWELL_ULT_E_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0A2E) // Haswell ULT E GT3

#define GMA_HASWELL_SDV_D_GT1_IG GFX_MODEL_CONSTRUCT(INTEL, 0x0C02)
//#define GMA_HASWELL_E_HB         GFX_MODEL_CONSTRUCT(INTEL, 0x0C04) // DRAM Controller
#define GMA_HASWELL_SDV_M_GT1_IG GFX_MODEL_CONSTRUCT(INTEL, 0x0C06) // Haswell SDV Mobile GT1
#define GMA_HASWELL_SDV_D_GT2_IG GFX_MODEL_CONSTRUCT(INTEL, 0x0C12)
#define GMA_HASWELL_SDV_M_GT2_IG GFX_MODEL_CONSTRUCT(INTEL, 0x0C16) // Haswell SDV Mobile GT2
#define GMA_HASWELL_SDV_D_GT2_PLUS_IG GFX_MODEL_CONSTRUCT(INTEL, 0x0C22) // Haswell HD Graphics - GTH
#define GMA_HASWELL_SDV_M_GT2_PLUS_IG GFX_MODEL_CONSTRUCT(INTEL, 0x0C26) // Haswell SDV Mobile GT3
//#define GMA_HASWELL_SDV_S_GT1_IG	GFX_MODEL_CONSTRUCT(INTEL, 0x0C0A)
//#define GMA_HASWELL_SDV_S_GT2_IG	GFX_MODEL_CONSTRUCT(INTEL, 0x0C1A)
//#define GMA_HASWELL_SDV_S_GT2_PLUS_IG	GFX_MODEL_CONSTRUCT(INTEL, 0x0C2A)

#define GMA_HASWELL_CRW_D_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0D02)
#define GMA_HASWELL_CRW_D_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0D12) // Intel(R) HD Graphics 5200 Drivers
#define GMA_HASWELL_CRW_D_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0D22) // Haswell CRW GT3 - Intel(R) Iris(TM) Pro Graphics 5200
//#define GMA_HASWELL_CRW_D_GT2_PLUS_IG	GFX_MODEL_CONSTRUCT(INTEL, 0x0D32)
#define GMA_HASWELL_CRW_M_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0D06) // Intel(R) HD Graphics 5200
#define GMA_HASWELL_CRW_M_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0D16) // Intel(R) HD Graphics 5200
#define GMA_HASWELL_CRW_M_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0D26) // Haswell CRW Mobile GT3 - Intel(R) Iris(TM) Pro Graphics 5200 Drivers
#define GMA_HASWELL_CRW_S_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0D0A)
#define GMA_HASWELL_CRW_S_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0D1A)
#define GMA_HASWELL_CRW_S_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0D2A)
#define GMA_HASWELL_CRW_B_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0D0B)
#define GMA_HASWELL_CRW_B_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0D1B)
#define GMA_HASWELL_CRW_B_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0D2B)
#define GMA_HASWELL_CRW_E_GT1      GFX_MODEL_CONSTRUCT(INTEL, 0x0D0E)
#define GMA_HASWELL_CRW_E_GT2      GFX_MODEL_CONSTRUCT(INTEL, 0x0D1E)
#define GMA_HASWELL_CRW_E_GT3      GFX_MODEL_CONSTRUCT(INTEL, 0x0D2E)
#define GMA_HASWELL_CRW_M_GT2_PLUS_IG    GFX_MODEL_CONSTRUCT(INTEL, 0x0D36) // Crystal Well Integrated Graphics Controller
#define GMA_HASWELL_CRW_S_GT2_PLUS_IG    GFX_MODEL_CONSTRUCT(INTEL, 0x0D3A)

/* Broadwell */
#define GMA_BROADWELL_BDW_0bd0      GFX_MODEL_CONSTRUCT(INTEL, 0x0bd0) // Intel Broadwell HD Graphics HAS GT0 Drivers // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_0bd1      GFX_MODEL_CONSTRUCT(INTEL, 0x0bd1) // Intel Broadwell HD Graphics HAS GT1 Drivers // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_0bd2      GFX_MODEL_CONSTRUCT(INTEL, 0x0bd2) // Intel Broadwell HD Graphics HAS GT2 Drivers // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_0bd3      GFX_MODEL_CONSTRUCT(INTEL, 0x0bd3) // Intel Broadwell HD Graphics HAS GT3 Drivers
#define GMA_BROADWELL_BDW_0bd4      GFX_MODEL_CONSTRUCT(INTEL, 0x0bd4) // Intel Broadwell HD Graphics HAS GT4 Drivers
#define GMA_BROADWELL_BDW_1602      GFX_MODEL_CONSTRUCT(INTEL, 0x1602) // Intel(R) HD Graphics Drivers // Halo // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_U_GT1     GFX_MODEL_CONSTRUCT(INTEL, 0x1606) // BDW U GT1 // ULT // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_160A      GFX_MODEL_CONSTRUCT(INTEL, 0x160A) // Broadwell-U Integrated Graphics // Server // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_160B      GFX_MODEL_CONSTRUCT(INTEL, 0x160B) // Broadwell-U Integrated Graphics // ULT // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_160D      GFX_MODEL_CONSTRUCT(INTEL, 0x160D) // Broadwell-U Integrated Graphics // Workstation // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_160E      GFX_MODEL_CONSTRUCT(INTEL, 0x160E) // Intel(R) HD Graphics Drivers // ULX // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_1612      GFX_MODEL_CONSTRUCT(INTEL, 0x1612) // Intel(R) HD Graphics 5600 Drivers // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_U_GT2     GFX_MODEL_CONSTRUCT(INTEL, 0x1616) // BDW U GT2 Intel(R) HD Graphics 5500 Drivers // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_161B      GFX_MODEL_CONSTRUCT(INTEL, 0x161B) // Broadwell-U Integrated Graphics // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_161A      GFX_MODEL_CONSTRUCT(INTEL, 0x161A) // Broadwell-U Integrated Graphics // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_161D      GFX_MODEL_CONSTRUCT(INTEL, 0x161D) // Broadwell-U Integrated Graphics // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_Y_GT2     GFX_MODEL_CONSTRUCT(INTEL, 0x161E) // BDW Y GT2 Intel(R) HD Graphics 5300 Drivers // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_1622      GFX_MODEL_CONSTRUCT(INTEL, 0x1622) // Intel(R) Iris(TM) Pro Graphics 6200 Drivers // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_162A      GFX_MODEL_CONSTRUCT(INTEL, 0x162A) // Intel(R) Iris(TM) Pro Graphics 6300P Drivers // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_U_GT3     GFX_MODEL_CONSTRUCT(INTEL, 0x1626) // BDW U GT3 15W Intel(R) HD Graphics 6000 Drivers // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_U_GT3_2   GFX_MODEL_CONSTRUCT(INTEL, 0x162B) // BDW U GT3 28W Intel(R) Iris(TM) Pro Graphics 6100 Drivers // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_162D      GFX_MODEL_CONSTRUCT(INTEL, 0x162D) // Intel(R) Iris(TM) Pro Graphics 6300P Drivers  // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_162E      GFX_MODEL_CONSTRUCT(INTEL, 0x162E) // Broadwell-U Integrated Graphics // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_1632      GFX_MODEL_CONSTRUCT(INTEL, 0x1632) // Broadwell-U Integrated Graphics // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_1636      GFX_MODEL_CONSTRUCT(INTEL, 0x1636) // Broadwell-U Integrated Graphics // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_163A      GFX_MODEL_CONSTRUCT(INTEL, 0x163A) // Broadwell-U Integrated Graphics // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_163B      GFX_MODEL_CONSTRUCT(INTEL, 0x163B) // Broadwell-U Integrated Graphics // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_163D      GFX_MODEL_CONSTRUCT(INTEL, 0x163D) // Broadwell-U Integrated Graphics // AppleIntelBDWGraphics.kext
#define GMA_BROADWELL_BDW_163E      GFX_MODEL_CONSTRUCT(INTEL, 0x163E) // Broadwell-U Integrated Graphics // AppleIntelBDWGraphics.kext

/* Skylake */
#define GMA_SKYLAKE_ULT_GT1	GFX_MODEL_CONSTRUCT(INTEL, 0x1906) // Intel(R) HD Graphics 510
#define GMA_SKYLAKE_ULT_GT15	GFX_MODEL_CONSTRUCT(INTEL, 0x1913) // Intel(R) HD Graphics 510
#define GMA_SKYLAKE_ULT_GT2	GFX_MODEL_CONSTRUCT(INTEL, 0x1916) // Intel(R) HD Graphics 520
#define GMA_SKYLAKE_ULX_GT1	GFX_MODEL_CONSTRUCT(INTEL, 0x190E) // Intel(R) HD Graphics
#define GMA_SKYLAKE_ULX_GT2	GFX_MODEL_CONSTRUCT(INTEL, 0x191E) // Intel(R) HD Graphics 515
#define GMA_SKYLAKE_DT_GT2	GFX_MODEL_CONSTRUCT(INTEL, 0x1912) // Intel(R) HD Graphics 530
#define GMA_SKYLAKE_1921	GFX_MODEL_CONSTRUCT(INTEL, 0x1921) // Intel(R) HD Graphics 520
#define GMA_SKYLAKE_ULT_GT3_E	GFX_MODEL_CONSTRUCT(INTEL, 0x1926) // Intel(R) Iris(TM) Graphics 540
#define GMA_SKYLAKE_ULT_GT3	GFX_MODEL_CONSTRUCT(INTEL, 0x1923) // Intel(R) HD Graphics 535
#define GMA_SKYLAKE_ULT_GT3_28W	GFX_MODEL_CONSTRUCT(INTEL, 0x1927) // Intel(R) Iris(TM) Graphics 550
#define GMA_SKYLAKE_DT_GT15	GFX_MODEL_CONSTRUCT(INTEL, 0x1917) // Intel(R) HD Graphics 530
#define GMA_SKYLAKE_DT_GT1	GFX_MODEL_CONSTRUCT(INTEL, 0x1902) // Intel(R) HD Graphics 510
#define GMA_SKYLAKE_DT_GT4	GFX_MODEL_CONSTRUCT(INTEL, 0x1932) // Intel(R) Iris(TM) Pro Graphics 570/580
#define GMA_SKYLAKE_GT4		GFX_MODEL_CONSTRUCT(INTEL, 0x193B) // Intel(R) Iris(TM) Pro Graphics 580
#define GMA_SKYLAKE_GT3_FE	GFX_MODEL_CONSTRUCT(INTEL, 0x192B) // Intel(R) Iris(TM) Graphics
#define GMA_SKYLAKE_GT2		GFX_MODEL_CONSTRUCT(INTEL, 0x191B) // Intel(R) HD Graphics 530
#define GMA_SKYLAKE_192A	GFX_MODEL_CONSTRUCT(INTEL, 0x192A) // Intel(R) Iris(TM) Pro Graphics P580
#define GMA_SKYLAKE_SRW_GT4	GFX_MODEL_CONSTRUCT(INTEL, 0x193A) // Intel(R) Iris(TM) Pro Graphics P580
#define GMA_SKYLAKE_WS_GT2	GFX_MODEL_CONSTRUCT(INTEL, 0x191D) // Intel(R) HD Graphics P530
#define GMA_SKYLAKE_WS_GT4	GFX_MODEL_CONSTRUCT(INTEL, 0x193D) // Intel(R) Iris(TM) Pro Graphics P580

/* END */

#endif /* !__LIBSAIO_GMA_H */
