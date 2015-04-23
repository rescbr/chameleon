/*
 * Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 * Copyright © 2010 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

/*
	Original patch by Nawcom
	http://forum.voodooprojects.org/index.php/topic,1029.0.html

	Original Intel HDx000 code from valv
	Intel Ivy Bridge and Haswell code from ErmaC:
	- http://www.insanelymac.com/forum/topic/288241-intel-hd4000-inject-aaplig-platform-id/
*/

#include "libsa.h"
#include "saio_internal.h"
#include "bootstruct.h"
#include "pci.h"
#include "platform.h"
#include "device_inject.h"
#include "gma.h"
#include "vbe.h"
#include "graphics.h"

#ifndef DEBUG_GMA
#define DEBUG_GMA 0
#endif

#if DEBUG_GMA
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

uint8_t GMAX3100_vals[23][4] = {
	{ 0x01,0x00,0x00,0x00 },	//0 "AAPL,HasPanel"
	{ 0x01,0x00,0x00,0x00 },	//1 "AAPL,SelfRefreshSupported"
	{ 0x01,0x00,0x00,0x00 },	//2 "AAPL,aux-power-connected"
	{ 0x00,0x00,0x00,0x08 },	//3 "AAPL,backlight-control"
	{ 0x64,0x00,0x00,0x00 },	//4 "AAPL00,blackscreen-preferences"
	{ 0x00,0x00,0x00,0x08 },	//5 "AAPL01,BacklightIntensity"
	{ 0x01,0x00,0x00,0x00 },	//6 "AAPL01,blackscreen-preferences"
	{ 0x20,0x00,0x00,0x00 },	//7 "AAPL01,DataJustify"
	{ 0x00,0x00,0x00,0x00 },	//8 "AAPL01,Depth"
	{ 0x01,0x00,0x00,0x00 },	//9 "AAPL01,Dither"
	{ 0x20,0x03,0x00,0x00 },	//10 "AAPL01,DualLink"
	{ 0x00,0x00,0x00,0x00 },	//11 "AAPL01,Height"
	{ 0x00,0x00,0x00,0x00 },	//12 "AAPL01,Interlace"
	{ 0x00,0x00,0x00,0x00 },	//13 "AAPL01,Inverter"
	{ 0x08,0x52,0x00,0x00 },	//14 "AAPL01,InverterCurrent"
	{ 0x00,0x00,0x00,0x00 },	//15 "AAPL01,InverterCurrency"
	{ 0x00,0x00,0x00,0x00 },	//16 "AAPL01,LinkFormat"
	{ 0x01,0x00,0x00,0x00 },	//17 "AAPL01,LinkType"
	{ 0x01,0x00,0x00,0x00 },	//18 "AAPL01,Pipe"
	{ 0x3B,0x00,0x00,0x00 },	//19 "AAPL01,PixelFormat"
	{ 0x00,0x00,0x00,0x00 },	//20 "AAPL01,Refresh"
	{ 0x6B,0x10,0x00,0x00 },	//21 "AAPL01,Stretch"
	{ 0xc8,0x95,0x00,0x00 },	//22 "AAPL01,InverterFrequency"
};

uint8_t ivy_bridge_ig_vals[12][4] = {
	{ 0x00,0x00,0x66,0x01 },	//0 "AAPL,ig-platform-id" //FB: 96MB, Pipes: 3, Ports: 4, FBMem: 3
	{ 0x01,0x00,0x66,0x01 },	//1 "AAPL,ig-platform-id" //FB: 96MB, Pipes: 3, Ports: 4, FBMem: 3
	{ 0x02,0x00,0x66,0x01 },	//2 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 1, FBMem: 1
	{ 0x03,0x00,0x66,0x01 },	//3 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 2, Ports: 2, FBMem: 2
	{ 0x04,0x00,0x66,0x01 },	//4 "AAPL,ig-platform-id" //FB: 32MB, Pipes: 3, Ports: 1, FBMem: 1
	{ 0x05,0x00,0x62,0x01 },	//5 "AAPL,ig-platform-id" //FB: 32MB, Pipes: 2, Ports: 3, FBMem: 2
	{ 0x06,0x00,0x62,0x01 },	//6 "AAPL,ig-platform-id" //FB: 0MB, Pipes: 0, Ports: 0, FBMem: 0
	{ 0x07,0x00,0x62,0x01 },	//7 "AAPL,ig-platform-id" //FB: 0MB, Pipes: 0, Ports: 0, FBMem: 0
	{ 0x08,0x00,0x66,0x01 },	//8 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3
	{ 0x09,0x00,0x66,0x01 },	//9 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3
	{ 0x0a,0x00,0x66,0x01 },	//10 "AAPL,ig-platform-id" //FB: 32MB, Pipes: 2, Ports: 3, FBMem: 2
	{ 0x0b,0x00,0x66,0x01 }		//11 "AAPL,ig-platform-id" //FB: 32MB, Pipes: 2, Ports: 3, FBMem: 2
};

uint8_t haswell_ig_vals[16][4] = { /* - TESTING DATA --*/
	{ 0x00,0x00,0x06,0x04 },	// 0 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - mobile GT1
	{ 0x00,0x00,0x06,0x0c },	// 1 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - SDV mobile GT1
	{ 0x00,0x00,0x16,0x04 },	// 2 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - mobile GT2
	{ 0x00,0x00,0x16,0x0a },	// 3 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - ULT mobile GT2
	{ 0x00,0x00,0x16,0x0c },	// 4 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - SDV mobile GT2
	{ 0x00,0x00,0x26,0x04 },	// 5 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - mobile GT3
	{ 0x00,0x00,0x26,0x0a },	// 6 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - ULT mobile GT3
	{ 0x00,0x00,0x26,0x0c },	// 7 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - SDV mobile GT3
	{ 0x00,0x00,0x26,0x0d },    // 8 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - CRW mobile GT3
	{ 0x02,0x00,0x16,0x04 },	// 9 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 1, Ports: 1, FBMem: 1 - mobile GT2
	{ 0x03,0x00,0x22,0x0d },    // 10 "AAPL,ig-platform-id" //FB: 0MB, Pipes: 0, Ports: 0, FBMem: 0 - CRW Desktop GT3
//	{ 0x04,0x00,0x12,0x04 },	// ?? "AAPL,ig-platform-id" //FB: 32MB, Pipes: 3, Ports: 3, FBMem: 3 - ULT mobile GT3
	{ 0x05,0x00,0x26,0x0a },	// 11 "AAPL,ig-platform-id" //FB: 32MB, Pipes: 3, Ports: 3, FBMem: 3 - ULT mobile GT3
	{ 0x06,0x00,0x26,0x0a },	// 12 "AAPL,ig-platform-id" //FB: 32MB, Pipes: 3, Ports: 3, FBMem: 3 - ULT mobile GT3
	{ 0x07,0x00,0x26,0x0d },    // 13 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 4, FBMem: 3 - CRW mobile GT3
	{ 0x08,0x00,0x26,0x0a },	// 14 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - ULT mobile GT3
	{ 0x08,0x00,0x2e,0x0a },	// 15 "AAPL,ig-platform-id" //FB: 64MB, Pipes: 3, Ports: 3, FBMem: 3 - ULT reserved GT3
};

uint8_t HD2000_vals[16][4] = {
	{ 0x00,0x00,0x00,0x00 },    //0 "AAPL00,PixelFormat"
	{ 0x00,0x00,0x00,0x00 },    //1 "AAPL00,T1"
	{ 0x14,0x00,0x00,0x00 },    //2 "AAPL00,T2"
	{ 0xfa,0x00,0x00,0x00 },    //3 "AAPL00,T3"
	{ 0x2c,0x01,0x00,0x00 },    //4 "AAPL00,T4"
	{ 0x00,0x00,0x00,0x00 },    //5 "AAPL00,T5"
	{ 0x14,0x00,0x00,0x00 },    //6 "AAPL00,T6"
	{ 0xf4,0x01,0x00,0x00 },    //7 "AAPL00,T7"
	{ 0x00,0x00,0x00,0x00 },    //8 "AAPL00,LinkType"
	{ 0x00,0x00,0x00,0x00 },    //9 "AAPL00,LinkFormat"
	{ 0x00,0x00,0x00,0x00 },    //10 "AAPL00,DualLink"
	{ 0x00,0x00,0x00,0x00 },    //11 "AAPL00,Dither"
	{ 0x00,0x00,0x00,0x00 },    //12 "AAPL00,DataJustify"
	{ 0x00,0x00,0x00,0x00 },    //13 "graphic-options"
	{ 0x00,0x00,0x00,0x00 },    //14
	{ 0x01,0x00,0x00,0x00 }     //15
};

uint8_t HD3000_vals[17][4] = {
	{ 0x00,0x00,0x00,0x00 },    //0 "AAPL00,PixelFormat"
	{ 0x00,0x00,0x00,0x00 },    //1 "AAPL00,T1"
	{ 0x14,0x00,0x00,0x00 },    //2 "AAPL00,T2"
	{ 0xfa,0x00,0x00,0x00 },    //3 "AAPL00,T3"
	{ 0x2c,0x01,0x00,0x00 },    //4 "AAPL00,T4"
	{ 0x00,0x00,0x00,0x00 },    //5 "AAPL00,T5"
	{ 0x14,0x00,0x00,0x00 },    //6 "AAPL00,T6"
	{ 0xf4,0x01,0x00,0x00 },    //7 "AAPL00,T7"
	{ 0x00,0x00,0x00,0x00 },    //8 "AAPL00,LinkType"
	{ 0x00,0x00,0x00,0x00 },    //9 "AAPL00,LinkFormat"
	{ 0x00,0x00,0x00,0x00 },    //10 "AAPL00,DualLink"
	{ 0x00,0x00,0x00,0x00 },    //11 "AAPL00,Dither"
	{ 0x00,0x00,0x00,0x00 },    //12 "AAPL00,DataJustify"
	{ 0x00,0x00,0x00,0x00 },    //13 "graphic-options"
	{ 0x00,0x00,0x00,0x00 },    //14
	{ 0x01,0x00,0x00,0x00 },    //15
	{ 0x00,0x00,0x01,0x00 }     //16 AAPL,snb-platform-id
};

uint8_t HD4000_vals[15][4] = {
	{ 0x00,0x00,0x00,0x00 },    //0 "AAPL00,PixelFormat"
	{ 0x00,0x00,0x00,0x00 },    //1 "AAPL00,T1"
	{ 0x01,0x00,0x00,0x00 },    //2 "AAPL00,T2"
	{ 0xc8,0x00,0x00,0x00 },    //3 "AAPL00,T3"
	{ 0xc8,0x00,0x00,0x00 },    //4 "AAPL00,T4"
	{ 0x01,0x00,0x00,0x00 },    //5 "AAPL00,T5"
	{ 0x00,0x00,0x00,0x00 },    //6 "AAPL00,T6"
	{ 0x90,0x01,0x00,0x00 },    //7 "AAPL00,T7"
	{ 0x01,0x00,0x00,0x00 },    //8 "AAPL00,LinkType"
	{ 0x00,0x00,0x00,0x00 },    //9 "AAPL00,LinkFormat"
	{ 0x01,0x00,0x00,0x00 },    //10 "AAPL00,DualLink"
	{ 0x00,0x00,0x00,0x00 },    //11 "AAPL00,Dither"
	{ 0xc3,0x8c,0x64,0x00 },    //12 "AAPL,gray-value"
	{ 0x01,0x00,0x00,0x00 },    //13 "AAPL,gray-page"
	{ 0x0c,0x00,0x00,0x00 }     //14 "graphics-options"
};

// http://www.insanelymac.com/forum/topic/286092-guide-1st-generation-intel-hd-graphics-qeci/
uint8_t HDx000_os_info[20] = {
	0x30,0x49,0x01,0x11,0x01,0x10,0x08,0x00,0x00,0x01,
	0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF
};

uint8_t HD2000_tbl_info[18] = {
	0x30,0x44,0x02,0x02,0x02,0x02,0x00,0x00,0x00,
	0x00,0x01,0x02,0x02,0x02,0x00,0x01,0x02,0x02
};
uint8_t HD2000_os_info[20] = {
	0x30,0x49,0x01,0x11,0x11,0x11,0x08,0x00,0x00,0x01,
	0xf0,0x1f,0x01,0x00,0x00,0x00,0x10,0x07,0x00,0x00
};

// The following values came from a Sandy Bridge MacBook Air
uint8_t HD3000_tbl_info[18] = {
	0x30,0x44,0x02,0x02,0x02,0x02,0x00,0x00,0x00,
	0x00,0x02,0x02,0x02,0x02,0x01,0x01,0x01,0x01
};

// The following values came from a Sandy Bridge MacBook Air
uint8_t HD3000_os_info[20] = {
	0x30,0x49,0x01,0x12,0x12,0x12,0x08,0x00,0x00,0x01,
	0xf0,0x1f,0x01,0x00,0x00,0x00,0x10,0x07,0x00,0x00
};


uint8_t reg_TRUE[]	= { 0x01, 0x00, 0x00, 0x00 };
uint8_t reg_FALSE[] = { 0x00, 0x00, 0x00, 0x00 };

// https://en.wikipedia.org/wiki/Comparison_of_Intel_graphics_processing_units#Seventh_generation

/* http://cgit.freedesktop.org/xorg/driver/xf86-video-intel/tree/src/intel_module.c */

static intel_gfx_info_t intel_gfx_chipsets[] = {
	{GMA_I810,                      "i810"},
	{GMA_I810_DC100,                "i810-dc100"},
	{GMA_I810_E,                    "i810e"},
	{GMA_I815,                      "i815"},
	{GMA_I830_M,                    "i830M"},
	{GMA_845_G,                     "845G"},
	{GMA_I854,                      "854"},
	{GMA_I855_GM,                   "852GM/855GM"},
	{GMA_I865_G,                    "865G"},
	{GMA_I915_G,                    "915G"},
	{GMA_E7221_G,                   "E7221 (i915)"},
	{GMA_I915_GM,                   "915GM"},
	{GMA_I945_G,                    "945G"},
    // 2776 /* Desktop GMA950 */
    // 2782 /* GMA 915 */
    // 2792 /* Mobile GMA915 */
	{GMA_I945_GM,                   "945GM"},
	{GMA_I945_GME,                  "945GME"},
    // 27A6 /* Mobile GMA950 */
    // 29C3 /* Desktop GMA3100 */
	{GMA_PINEVIEW_M,                "Pineview GM"},
	{GMA_GMA3150_M,                 "Pineview GM"}, // A012 /* Mobile GMA3150 */
	{GMA_PINEVIEW_G,                "Pineview G"},
	{GMA_GMA3150_D,                 "Desktop GMA3150"}, // A002 /* Desktop GMA3150 */
	{GMA_I965_G,                    "965G"},
	{GMA_G35_G,                     "G35"},
	{GMA_I965_Q,                    "965Q"},
	{GMA_I946_GZ,                   "946GZ"},
	{GMA_I965_GM,                   "965GM"},
	{GMA_I965_GME,                  "965GME/GLE"},
	{GMA_G33_G,                     "G33"},
    // 2A13 /* GMAX3100 */
    // 2A43 /* GMAX3100 */
	{GMA_Q35_G,                     "Q35"},
	{GMA_Q33_G,                     "Q33"},
	{GMA_GM45_GM,                   "GM45"},
	{GMA_G45_E_G,                   "4 Series"},
	{GMA_G45_G,                     "G45/G43"},
	{GMA_Q45_G,                     "Q45/Q43"},
	{GMA_G41_G,                     "G41"},
	{GMA_B43_G,                     "B43"},
	{GMA_B43_G1,                    "B43"},
    /**/
	{GMA_IRONLAKE_D_G,              HD_GRAPHICS},
	{GMA_IRONLAKE_M_G,              HD_GRAPHICS},
    // 004A /* HD2000 */

    /* Sandy */
	{GMA_SANDYBRIDGE_GT1,           HD_GRAPHICS_2000 },
	{GMA_SANDYBRIDGE_GT2,           HD_GRAPHICS_3000 },
	{GMA_SANDYBRIDGE_GT2_PLUS,      HD_GRAPHICS_3000 },
	{GMA_SANDYBRIDGE_M_GT1,         HD_GRAPHICS_2000 },
	{GMA_SANDYBRIDGE_M_GT2,         HD_GRAPHICS_3000 },
	{GMA_SANDYBRIDGE_M_GT2_PLUS,    HD_GRAPHICS_3000 },
	{GMA_SANDYBRIDGE_S_GT,          HD_GRAPHICS },
    // 010B /* ??? */
    // 010E /* ??? */

    /* Ivy */
	{GMA_IVYBRIDGE_M_GT1,           HD_GRAPHICS_2500 },
	{GMA_IVYBRIDGE_M_GT2,           HD_GRAPHICS_4000 },
	{GMA_IVYBRIDGE_D_GT1,           HD_GRAPHICS_2500 },
	{GMA_IVYBRIDGE_D_GT2,           HD_GRAPHICS_4000 },
	{GMA_IVYBRIDGE_S_GT1,           HD_GRAPHICS },
	{GMA_IVYBRIDGE_S_GT2,           "HD Graphics P4000" },
	{GMA_IVYBRIDGE_S_GT3,           HD_GRAPHICS }, // 015e
 	{GMA_IVYBRIDGE_S_GT4,           HD_GRAPHICS_2500 },  // 0172 /* HD Graphics 2500 Mobile */
  	{GMA_IVYBRIDGE_S_GT5,           HD_GRAPHICS_2500 },  // 0176 /* HD Graphics 2500 Mobile */

    /* Haswell */
    // 0090 /* ??? */
    // 0091 /* ??? */
    // 0092 /* ??? */
	{GMA_HASWELL_D_GT1,             HD_GRAPHICS },
	{GMA_HASWELL_D_GT2,             HD_GRAPHICS_4600 },
	{GMA_HASWELL_D_GT3,             HD_GRAPHICS_5000 }, /* ??? */
	{GMA_HASWELL_M_GT1,             HD_GRAPHICS },
	{GMA_HASWELL_M_GT2,             HD_GRAPHICS_4600 },
	{GMA_HASWELL_M_GT3,             HD_GRAPHICS_5000 }, /* ??? */
	{GMA_HASWELL_S_GT1,             HD_GRAPHICS },
	{GMA_HASWELL_S_GT2,             "HD Graphics P4600/P4700" },
	{GMA_HASWELL_S_GT3,             HD_GRAPHICS_5000 }, /* ??? */
	{GMA_HASWELL_B_GT1,             HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_B_GT2,             HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_B_GT3,             HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_E_GT1,             HD_GRAPHICS },
	{GMA_HASWELL_E_GT2,             HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_E_GT3,             HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_ULT_D_GT1,         HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_ULT_D_GT2,         HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_ULT_D_GT3,         IRIS_5100 },
	{GMA_HASWELL_ULT_M_GT1,         HD_GRAPHICS },
	{GMA_HASWELL_ULT_M_GT2,         HD_GRAPHICS_4400 },
	{GMA_HASWELL_ULT_M_GT3,         HD_GRAPHICS_5000 },
	{GMA_HASWELL_ULT_S_GT1,         HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_ULT_S_GT2,         HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_ULT_S_GT3,         IRIS_5100 },
	{GMA_HASWELL_ULT_B_GT1,         HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_ULT_B_GT2,         HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_ULT_B_GT3,         IRIS_5100 },
	{GMA_HASWELL_ULT_E_GT1,         HD_GRAPHICS },
	{GMA_HASWELL_ULT_E_GT2,         HD_GRAPHICS_4200 },
	// 0A2A /* ??? */
	{GMA_HASWELL_ULT_E_GT3,         IRIS_5100 },
	// 0C02 /* Intel Haswell HD Graphics - GTL */
	// 0C04 /* ??? */
	// 0C06 /* Intel Haswell HD Graphics - GTL */
	// 0C12 /* Intel Haswell HD Graphics - GTM */
	// 0C16 /* Intel Haswell HD Graphics - GTH */
	// 0C22 /* Intel Haswell HD Graphics - GTH */
	// 0C26 /* Intel Haswell HD Graphics - GTH */
	{GMA_HASWELL_CRW_D_GT1,         HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_CRW_D_GT2,         HD_GRAPHICS_4600 },
	{GMA_HASWELL_CRW_D_GT3,         IRIS_5200 },
	{GMA_HASWELL_CRW_M_GT1,         HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_CRW_M_GT2,         HD_GRAPHICS_4600 },
	{GMA_HASWELL_CRW_M_GT3,         IRIS_5200 },
	{GMA_HASWELL_CRW_S_GT1,         HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_CRW_S_GT2,         HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_CRW_S_GT3,         IRIS_5200 },
	{GMA_HASWELL_CRW_B_GT1,         HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_CRW_B_GT2,         HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_CRW_B_GT3,         IRIS_5200 },
	{GMA_HASWELL_CRW_E_GT1,         HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_CRW_E_GT2,         HD_GRAPHICS }, /* ??? */
	{GMA_HASWELL_CRW_E_GT3,         IRIS_5200 },
	{GMA_HASWELL_CRW_M_GT2_PLUS_IG, HD_GRAPHICS },
    
    /* Brodwell */
    {GMA_BRODWELL_BDW_M_GT1,		HD_GRAPHICS },      /* 1602 */
    {GMA_BRODWELL_BDW_M_GT2,		HD_GRAPHICS_5600},	/* 1612 */
    {GMA_BRODWELL_BDW_M_GT3e,		IRIS_Pro_6200},     /* 1622 */
    {GMA_BRODWELL_BDW_M_1632,		HD_GRAPHICS },		/* 1632 */
    {GMA_BRODWELL_ULT_M_GT1,        HD_GRAPHICS },      /* 1606 */
    {GMA_BRODWELL_ULT_M_GT2,        HD_GRAPHICS_5500},  /* 1616 */
    {GMA_BRODWELL_ULT_M_GT3,		HD_GRAPHICS_6000},	/* 1626 */
    {GMA_BRODWELL_ULT_M_1636,		HD_GRAPHICS },		/* 1636 */
    {GMA_BRODWELL_BDW_S_GT1,        HD_GRAPHICS },      /* 160a */
    {GMA_BRODWELL_BDW_S_GT2,		HD_GRAPHICS },		/* 161a */
    {GMA_BRODWELL_BDW_S_GT3e,		IRIS_Pro_P6300},    /* 162a */
    {GMA_BRODWELL_BDW_S_163A,		HD_GRAPHICS },		/* 163a */
    {GMA_BRODWELL_LVT_M_GT1,        HD_GRAPHICS },      /* 160b */
    {GMA_BRODWELL_LVT_M_GT2,        HD_GRAPHICS },      /* 161b */
    {GMA_BRODWELL_LVT_M_GT3,		IRIS_6100 },		/* 162b */
    {GMA_BRODWELL_LVT_M_163B,		HD_GRAPHICS },		/* 163b */
    {GMA_BRODWELL_BDW_D_GT1,		HD_GRAPHICS },      /* 160d */
    {GMA_BRODWELL_BDW_D_GT2,		HD_GRAPHICS },		/* 161d */
    {GMA_BRODWELL_BDW_D_GT3,		HD_GRAPHICS },		/* 162d */
    {GMA_BRODWELL_BDW_D_163D,		HD_GRAPHICS },		/* 163d */
    {GMA_BRODWELL_ULX_M_GT1,        HD_GRAPHICS },      /* 160e */
    {GMA_BRODWELL_ULX_M_GT2,		HD_GRAPHICS_5300},	/* 161e */
    {GMA_BRODWELL_ULX_M_GT3,		HD_GRAPHICS },		/* 162e */
    {GMA_BRODWELL_ULX_M_163E,		HD_GRAPHICS }		/* 163e */
};

#define GFX_DEVICES_LEN (sizeof(intel_gfx_chipsets) / sizeof(intel_gfx_chipsets[0]))

/* END http://cgit.freedesktop.org/xorg/driver/xf86-video-intel/tree/src/intel_module.c */

/* Get Intel GFX device name */
static char *get_gma_controller_name(uint16_t device_id, uint16_t vendor_id)
{
	int i = 0;
	static char desc[128];

	for (i = 0; i < GFX_DEVICES_LEN; i++)
	{
		if (intel_gfx_chipsets[i].model == ((device_id << 16) | vendor_id))
		{
			snprintf(desc, sizeof(desc), "%s %s", INTEL_NAME, intel_gfx_chipsets[i].label_info);
			return desc;
		}
	}
	snprintf(desc, sizeof(desc), "Unknown %s Graphics card", INTEL_NAME);
	return desc;
}

bool setup_gma_devprop(pci_dt_t *gma_dev)
{
	char                    *devicepath = NULL;
	volatile uint8_t        *regs;
	uint32_t                bar[7];
	char                    *model = NULL;
	uint8_t BuiltIn =       0x00;
	int                     len;
	const char              *value;
	uint16_t                vendor_id = gma_dev->vendor_id;
	uint16_t                device_id = gma_dev->device_id;
	uint8_t ClassFix[4] =   { 0x00, 0x00, 0x03, 0x00 };
    
    uint8_t hd3k_kmobile_device_id[4] =  { 0x26, 0x01, 0x00, 0x00 }; // MacMan
    uint8_t hd4k_device_id[4] =          { 0x62, 0x01, 0x00, 0x00 }; // MacMan
    uint8_t hd4400_mobile_device_id[4] = { 0x26, 0x0A, 0x00, 0x00 }; // MacMan
    uint8_t hd4600_device_id[4] =        { 0x12, 0x04, 0x00, 0x00 }; // MacMan
    uint8_t hd4600_mobile_device_id[4] = { 0x16, 0x04, 0x00, 0x00 }; // MacMan
    uint8_t igp_device_id[4] =           { 0x00, 0x00, 0x00, 0x00 }; // MacMan
    
    uint8_t snb_id_3k[4] =          { 0x10, 0x00, 0x03, 0x00 }; // MacMan
    uint8_t snb_id_3k_mobile[4] =   { 0x00, 0x00, 0x01, 0x00 }; // MacMan
    uint8_t ig_id_4k[4] =           { 0x0A, 0x00, 0x66, 0x01 }; // MacMan
    uint8_t ig_id_4k_mobile[4] =    { 0x03, 0x00, 0x66, 0x01 }; // MacMan
    uint8_t ig_id_4600[4] =         { 0x03, 0x00, 0x22, 0x0D }; // MacMan
    uint8_t ig_id_4600_mobile[4] =  { 0x06, 0x00, 0x26, 0x0A }; // MacMan
    uint8_t ig_id_5500_mobile[4] =  { 0x00, 0x00, 0x16, 0x16 }; // MacMan
    uint8_t ig_platform_id[4] =     { 0x00, 0x00, 0x00, 0x00 }; // MacMan
    
	devicepath = get_pci_dev_path(gma_dev);

	bar[0] = pci_config_read32(gma_dev->dev.addr, 0x10);
	regs = (uint8_t *) (bar[0] & ~0x0f);

	model = get_gma_controller_name(device_id, vendor_id);

	verbose("---------------------------------------------\n");
   	verbose("------------ INTEL DEVICE INFO --------------\n");
	verbose("---------------------------------------------\n");
	verbose("Class code: [%04x]\n%s [%04x:%04x] (rev %02x)\nSubsystem: [%04x:%04x] :: %s\n",
			gma_dev->class_id, model, gma_dev->vendor_id, gma_dev->device_id, gma_dev->revision_id, gma_dev->subsys_id.subsys.vendor_id, gma_dev->subsys_id.subsys.device_id, devicepath);

	if (!string) {
		string = devprop_create_string();
	}

	struct DevPropDevice *device = devprop_add_device(string, devicepath);
	if (!device) {
		printf("Failed initializing dev-prop string dev-entry.\n");
		pause();
		return false;
	}

	devprop_add_value(device, "model", (uint8_t*)model, (strlen(model) + 1));
	devprop_add_value(device, "device_type", (uint8_t*)"display", 8);


	switch ((device_id << 16) | vendor_id)
	{
        case GMA_IRONLAKE_D_G: // 0042
        case GMA_IRONLAKE_M_G: // 0046
            devprop_add_value(device, "built-in", &BuiltIn, 1);
            devprop_add_value(device, "class-code", ClassFix, 4);
            devprop_add_value(device, "hda-gfx",	(uint8_t *)"onboard-1", 10);
            devprop_add_value(device, "AAPL,os-info", HDx000_os_info, 20);
            break;
        case GMA_I945_GM: // Mobile GMA950 Mobile GMA3150
        case GMA_I945_GME:
        //case GMA_945GM:
        case GMA_PINEVIEW_G:
        case GMA_PINEVIEW_M:
        case GMA_GMA3150_M:
            devprop_add_value(device, "AAPL,HasPanel", reg_TRUE, 4);
            devprop_add_value(device, "built-in", &BuiltIn, 1);
            devprop_add_value(device, "class-code", ClassFix, 4);
            break;

        case GMA_I945_G: // Desktop GMA950 Desktop GMA3150
        //case GMA_82945G:
        case GMA_GMA3150_D:
            BuiltIn = 0x01;
            devprop_add_value(device, "built-in", &BuiltIn, 1);
            devprop_add_value(device, "class-code", ClassFix, 4);
            break;

        case GMA_I965_GM: // GMAX3100
        case GMA_I965_GME:
        //case 0x80862A13:
        case GMA_GM45_GM:
        //case GMA_GM45_GM2:
            devprop_add_value(device, "AAPL,HasPanel",                  GMAX3100_vals[0], 4);
            devprop_add_value(device, "AAPL,SelfRefreshSupported",      GMAX3100_vals[1], 4);
            devprop_add_value(device, "AAPL,aux-power-connected",       GMAX3100_vals[2], 4);
            devprop_add_value(device, "AAPL,backlight-control",         GMAX3100_vals[3], 4);
            devprop_add_value(device, "AAPL00,blackscreen-preferences",	GMAX3100_vals[4], 4);
            devprop_add_value(device, "AAPL01,BacklightIntensity",      GMAX3100_vals[5], 4);
            devprop_add_value(device, "AAPL01,blackscreen-preferences",	GMAX3100_vals[6], 4);
            devprop_add_value(device, "AAPL01,DataJustify",             GMAX3100_vals[7], 4);
            devprop_add_value(device, "AAPL01,Depth",                   GMAX3100_vals[8], 4);
            devprop_add_value(device, "AAPL01,Dither",                  GMAX3100_vals[9], 4);
            devprop_add_value(device, "AAPL01,DualLink",                GMAX3100_vals[10], 4);
            devprop_add_value(device, "AAPL01,Height",                  GMAX3100_vals[11], 4);
            devprop_add_value(device, "AAPL01,Interlace",               GMAX3100_vals[12], 4);
            devprop_add_value(device, "AAPL01,Inverter",                GMAX3100_vals[13], 4);
            devprop_add_value(device, "AAPL01,InverterCurrent",         GMAX3100_vals[14], 4);
            devprop_add_value(device, "AAPL01,InverterCurrency",        GMAX3100_vals[15], 4);
            devprop_add_value(device, "AAPL01,LinkFormat",              GMAX3100_vals[16], 4);
            devprop_add_value(device, "AAPL01,LinkType",                GMAX3100_vals[17], 4);
            devprop_add_value(device, "AAPL01,Pipe",                    GMAX3100_vals[18], 4);
            devprop_add_value(device, "AAPL01,PixelFormat",             GMAX3100_vals[19], 4);
            devprop_add_value(device, "AAPL01,Refresh",                 GMAX3100_vals[20], 4);
            devprop_add_value(device, "AAPL01,Stretch",                 GMAX3100_vals[21], 4);
            //devprop_add_value(device, "AAPL01,InverterFrequency",	GMAX3100_vals[22], 4);
            devprop_add_value(device, "class-code",                     ClassFix, 4);
            break;
            
        case GMA_SANDYBRIDGE_GT1: // HD Graphics 2000, found in kext
            devprop_add_value(device, "built-in",			&BuiltIn, 1);
            devprop_add_value(device, "class-code",			ClassFix, 4);
            devprop_add_value(device, "device-id",			(uint8_t*)&device_id, sizeof(device_id));
            devprop_add_value(device, "hda-gfx",			(uint8_t *)"onboard-1", 10);
            devprop_add_value(device, "AAPL,tbl-info",		HD2000_tbl_info, 18);
            devprop_add_value(device, "AAPL,os-info",		HD2000_os_info, 20);
            if (getValueForKey(kIGPlatformID, &value, &len, &bootInfo->chameleonConfig))    // MacMan
            {
                verbose("Setting 0x%s for snb-platform-id\n", value);
                hex2devprop(value, ig_platform_id, 4);
                devprop_add_value(device, "AAPL,snb-platform-id", ig_platform_id, 4);
            }
            else
            {
                devprop_add_value(device, "AAPL,snb-platform-id", snb_id_3k, 4);
                verbose("Using default snb-platform-id of 0x00030010\n");
            }
            break;
            
        case GMA_SANDYBRIDGE_M_GT1: // HD Graphics 2000 Mobile, found in kext
            devprop_add_value(device, "class-code",         ClassFix, 4);
            devprop_add_value(device, "device-id",			(uint8_t*)&device_id, sizeof(device_id));
            devprop_add_value(device, "hda-gfx",			(uint8_t *)"onboard-1", 10);
            devprop_add_value(device, "AAPL00,PixelFormat",	HD2000_vals[0], 4);
            devprop_add_value(device, "AAPL00,T1",          HD2000_vals[1], 4);
            devprop_add_value(device, "AAPL00,T2",          HD2000_vals[2], 4);
            devprop_add_value(device, "AAPL00,T3",          HD2000_vals[3], 4);
            devprop_add_value(device, "AAPL00,T4",          HD2000_vals[4], 4);
            devprop_add_value(device, "AAPL00,T5",          HD2000_vals[5], 4);
            devprop_add_value(device, "AAPL00,T6",          HD2000_vals[6], 4);
            devprop_add_value(device, "AAPL00,T7",          HD2000_vals[7], 4);
            devprop_add_value(device, "AAPL00,LinkType",	HD2000_vals[8], 4);
            devprop_add_value(device, "AAPL00,LinkFormat",	HD2000_vals[9], 4);
            devprop_add_value(device, "AAPL00,DualLink",	HD2000_vals[10], 4);
            devprop_add_value(device, "AAPL00,Dither",		HD2000_vals[11], 4);
            devprop_add_value(device, "AAPL00,DataJustify",	HD2000_vals[12], 4);
            devprop_add_value(device, "graphic-options",	HD2000_vals[13], 4);
            devprop_add_value(device, "AAPL,tbl-info",		HD2000_tbl_info, 18);
            devprop_add_value(device, "AAPL,os-info",		HD2000_os_info, 20);
            if (getValueForKey(kIGPlatformID, &value, &len, &bootInfo->chameleonConfig))    // MacMan
            {
                verbose("Setting 0x%s for snb-platform-id\n", value);
                hex2devprop(value, ig_platform_id, 4);
                devprop_add_value(device, "AAPL,snb-platform-id", ig_platform_id, 4);
            }
            else
            {
                devprop_add_value(device, "AAPL,snb-platform-id", snb_id_3k_mobile, 4);
                verbose("Using default snb-platform-id of 0x00030010\n");
            }
            break;

        case GMA_SANDYBRIDGE_GT2:      // HD Graphics 3000
        case GMA_SANDYBRIDGE_GT2_PLUS: // HD Graphics 3000
            devprop_add_value(device, "built-in",			&BuiltIn, 1);
            devprop_add_value(device, "class-code",			ClassFix, 4);
            devprop_add_value(device, "hda-gfx",			(uint8_t *)"onboard-1", 10);
            devprop_add_value(device, "AAPL,tbl-info",		HD3000_tbl_info, 18);
            devprop_add_value(device, "AAPL,os-info",		HD3000_os_info, 20);
            if (getValueForKey(kIGPDeviceID, &value, &len, &bootInfo->chameleonConfig))    // MacMan
            {
                verbose("Using 0x%s for unsupported device id injection.\n", value);
                hex2devprop(value, igp_device_id, 2);
                devprop_add_value(device, "device-id",igp_device_id, 4);
            }
            else
            {
                devprop_add_value(device, "device-id", hd3k_kmobile_device_id, 4); // MacMan Inject Mobile ID Instead of Patching Kext
                verbose("Using default unsupported device id injection of 0x0126\n");
            }
            if (getValueForKey(kIGPlatformID, &value, &len, &bootInfo->chameleonConfig))    // MacMan
            {
                verbose("Using 0x%s for snb-platform-id\n", value);
                hex2devprop(value, ig_platform_id, 4);
                devprop_add_value(device, "AAPL,snb-platform-id", ig_platform_id, 4);
            }
            else
            {
                devprop_add_value(device, "AAPL,snb-platform-id", snb_id_3k, 4);
                verbose("Using default snb-platform-id of 0x00030010\n");
            }
            break;
            
        case GMA_SANDYBRIDGE_M_GT2:      // HD Graphics 3000 Mobile, found in kext
        case GMA_SANDYBRIDGE_M_GT2_PLUS: // HD Graphics 3000 Mobile, found in kext
            devprop_add_value(device, "class-code",         ClassFix, 4);
            devprop_add_value(device, "device-id",			(uint8_t*)&device_id, sizeof(device_id));
            devprop_add_value(device, "hda-gfx",			(uint8_t *)"onboard-1", 10);
            devprop_add_value(device, "AAPL00,PixelFormat",	HD3000_vals[0], 4);
            devprop_add_value(device, "AAPL00,T1",          HD3000_vals[1], 4);
            devprop_add_value(device, "AAPL00,T2",          HD3000_vals[2], 4);
            devprop_add_value(device, "AAPL00,T3",          HD3000_vals[3], 4);
            devprop_add_value(device, "AAPL00,T4",          HD3000_vals[4], 4);
            devprop_add_value(device, "AAPL00,T5",          HD3000_vals[5], 4);
            devprop_add_value(device, "AAPL00,T6",          HD3000_vals[6], 4);
            devprop_add_value(device, "AAPL00,T7",          HD3000_vals[7], 4);
            devprop_add_value(device, "AAPL00,LinkType",	HD3000_vals[8], 4);
            devprop_add_value(device, "AAPL00,LinkFormat",	HD3000_vals[9], 4);
            devprop_add_value(device, "AAPL00,DualLink",	HD3000_vals[10], 4);
            devprop_add_value(device, "AAPL00,Dither",		HD3000_vals[11], 4);
            devprop_add_value(device, "AAPL00,DataJustify",	HD3000_vals[12], 4);
            devprop_add_value(device, "graphic-options",	HD3000_vals[13], 4);
            devprop_add_value(device, "AAPL,tbl-info",		HD3000_tbl_info, 18);
            devprop_add_value(device, "AAPL,os-info",		HD3000_os_info, 20);
            if (getValueForKey(kIGPlatformID, &value, &len, &bootInfo->chameleonConfig))    // MacMan
            {
                verbose("Setting 0x%s for snb-platform-id\n", value);
                hex2devprop(value, ig_platform_id, 4);
                devprop_add_value(device, "AAPL,snb-platform-id", ig_platform_id, 4);
            }
            else
            {
                devprop_add_value(device, "AAPL,snb-platform-id", snb_id_3k_mobile, 4);
                verbose("Using default snb-platform-id of 0x00010000\n");
            }
            break;
            
            //
            // Supported Ivy Bridge Desktop Device IDs
            //
        case GMA_IVYBRIDGE_D_GT1: // HD Graphics 2500, found in kext
        case GMA_IVYBRIDGE_D_GT2: // HD Graphics 4000, found in kext
//            devprop_add_value(device, "AAPL00,DualLink",    HD4000_vals[10], 4);
            devprop_add_value(device, "built-in",           &BuiltIn, 1);
            devprop_add_value(device, "class-code",         ClassFix, 4);
            devprop_add_value(device, "device-id",			(uint8_t*)&device_id, sizeof(device_id));
            devprop_add_value(device, "hda-gfx",            (uint8_t *)"onboard-1", 10);
            if (getValueForKey(kIGPlatformID, &value, &len, &bootInfo->chameleonConfig))    // MacMan
            {
                verbose("Using 0x%s for ig-platform-id\n", value);
                hex2devprop(value, ig_platform_id, 4);
                devprop_add_value(device, "AAPL,ig-platform-id", ig_platform_id, 4);
            }
            else
            {
                devprop_add_value(device, "AAPL,ig-platform-id", ig_id_4k, 4);
                verbose("Using default ig-platform-id of 0x0166000a\n");
            }
            break;
            
            //
            // Supported Ivy Bridge Mobile Device IDs
            //
        case GMA_IVYBRIDGE_M_GT1: // HD Graphics 2500 Mobile, found in kext
        case GMA_IVYBRIDGE_M_GT2: // HD Graphics 4000 Mobile, found in kext
//            devprop_add_value(device, "AAPL00,DualLink",    HD4000_vals[10], 4);
            devprop_add_value(device, "built-in",           &BuiltIn, 1);
            devprop_add_value(device, "class-code",         ClassFix, 4);
            devprop_add_value(device, "device-id",			(uint8_t*)&device_id, sizeof(device_id));
            devprop_add_value(device, "hda-gfx",            (uint8_t *)"onboard-1", 10);
            if (getValueForKey(kIGPlatformID, &value, &len, &bootInfo->chameleonConfig))    // MacMan
            {
                verbose("Using 0x%s for ig-platform-id\n", value);
                hex2devprop(value, ig_platform_id, 4);
                devprop_add_value(device, "AAPL,ig-platform-id", ig_platform_id, 4);
            }
            else
            {
                devprop_add_value(device, "AAPL,ig-platform-id", ig_id_4k_mobile, 4);
                verbose("Using default ig-platform-id of 0x01660003\n");
            }
            break;

            //
            // Unsupported Ivy Bridge Desktop Device IDs
            //
        case GMA_IVYBRIDGE_S_GT1: // HD Graphics 4000
        case GMA_IVYBRIDGE_S_GT2: // HD Graphics P4000
        case GMA_IVYBRIDGE_S_GT3: // Xeon E3-1200 v2/3rd Gen Core processor Graphics Controller
        case GMA_IVYBRIDGE_S_GT4: // HD Graphics 2500 // Xeon E3-1200 v2/3rd Gen Core processor Graphics Controller
        case GMA_IVYBRIDGE_S_GT5: // HD Graphics 2500 // 3rd Gen Core processor Graphics Controller
//            devprop_add_value(device, "AAPL00,DualLink",    HD4000_vals[10], 4);
            devprop_add_value(device, "built-in",           &BuiltIn, 1);
            devprop_add_value(device, "class-code",         ClassFix, 4);
            devprop_add_value(device, "hda-gfx",            (uint8_t *)"onboard-1", 10);
            if (getValueForKey(kIGPDeviceID, &value, &len, &bootInfo->chameleonConfig))    // MacMan
            {
                verbose("Using %s for unsupported device id injection.\n", value);
                hex2devprop(value, igp_device_id, 2);
                devprop_add_value(device, "device-id",igp_device_id, 4);
            }
            else
            {
                devprop_add_value(device, "device-id", hd4k_device_id, 4); // MacMan Inject Supported ID Instead of Patching Kext
                verbose("Using default unsupported device id injection of 0162\n");
            }
            if (getValueForKey(kIGPlatformID, &value, &len, &bootInfo->chameleonConfig))
            {
                verbose("Using 0x%s for ig-platform-id\n", value);
                hex2devprop(value, ig_platform_id, 4);
                devprop_add_value(device, "AAPL,ig-platform-id", ig_platform_id, 4);
            }
            else
            {
                devprop_add_value(device, "AAPL,ig-platform-id", ig_id_4k, 4);
                verbose("Using default ig-platform-id of 0x0166000a\n");
            }
            break;
            
            //
            // Supported Haswell Desktop Device IDs
            //
        case GMA_HASWELL_D_GT2: // HD Graphics 4600, found in kext
        case GMA_HASWELL_CRW_D_GT3: // HD Graphics 5200 Iris Pro and found in kext
//            devprop_add_value(device, "AAPL00,DualLink",    HD4000_vals[10], 4);
            devprop_add_value(device, "built-in",           &BuiltIn, 1);
            devprop_add_value(device, "class-code",         ClassFix, 4);
            devprop_add_value(device, "device-id",			(uint8_t*)&device_id, sizeof(device_id));
            devprop_add_value(device, "hda-gfx",            (uint8_t *)"onboard-1", 10);
            if (getValueForKey(kIGPlatformID, &value, &len, &bootInfo->chameleonConfig))
            {
                verbose("Using 0x%s for ig-platform-id\n", value);
                hex2devprop(value, ig_platform_id, 4);
                devprop_add_value(device, "AAPL,ig-platform-id", ig_platform_id, 4);
            }
            else
            {
                devprop_add_value(device, "AAPL,ig-platform-id", ig_id_4600, 4);
                verbose("Using default ig-platform-id of 0x0d220003\n");
            }
            break;

            //
            // Supported Haswell Mobile Device IDs
            //
        case GMA_HASWELL_M_GT1: // HD Graphics Mobile, found in kext
        case GMA_HASWELL_M_GT2: // HD Graphics 4600 Mobile, found in kext
        case GMA_HASWELL_M_GT3: // HD Graphics 5000 Mobile, found in kext
        case GMA_HASWELL_ULT_M_GT2: // HD Graphics 4400 Mobile, found in kext
        case GMA_HASWELL_ULT_M_GT3: // HD Graphics 5000 Iris Mobile, found in kext
        case GMA_HASWELL_ULT_E_GT2: // HD Graphics 4200 Mobile, found in kext
        case GMA_HASWELL_ULT_E_GT3: // HD Graphics 5100 Iris Mobile, found in kext
        case GMA_HASWELL_CRW_M_GT3: // HD Graphics 5200 Iris Pro Mobile and found in kext
//            devprop_add_value(device, "AAPL00,DualLink",    HD4000_vals[10], 4);
            devprop_add_value(device, "built-in",           &BuiltIn, 1);
            devprop_add_value(device, "class-code",         ClassFix, 4);
            devprop_add_value(device, "device-id",			(uint8_t*)&device_id, sizeof(device_id));
            devprop_add_value(device, "hda-gfx",            (uint8_t *)"onboard-1", 10);
            if (getValueForKey(kIGPlatformID, &value, &len, &bootInfo->chameleonConfig))
            {
                verbose("Using 0x%s for ig-platform-id\n", value);
                hex2devprop(value, ig_platform_id, 4);
                devprop_add_value(device, "AAPL,ig-platform-id", ig_platform_id, 4);
            }
            else
            {
                devprop_add_value(device, "AAPL,ig-platform-id", ig_id_4600_mobile, 4);
                verbose("Using default ig-platform-id of 0x0a260006\n");
            }
            break;

            //
            // Unsupported Haswell Desktop Device IDs
            //
        //case 0x80860090:
        //case 0x80860091:
        //case 0x80860092:
        case GMA_HASWELL_D_GT1: // HD Graphics
        case GMA_HASWELL_D_GT3: // HD Graphics 5000
        case GMA_HASWELL_S_GT1: // HD Graphics
        case GMA_HASWELL_S_GT2: // HD Graphics
        case GMA_HASWELL_S_GT3: //
        case GMA_HASWELL_B_GT1: //
        case GMA_HASWELL_B_GT2: // HD Graphics
        case GMA_HASWELL_B_GT3: //
        case GMA_HASWELL_E_GT1: //
        case GMA_HASWELL_E_GT3: //
        case GMA_HASWELL_ULT_D_GT1: //
        case GMA_HASWELL_ULT_D_GT2: //
        case GMA_HASWELL_ULT_D_GT3: // HD Graphics 5000 Iris
        case GMA_HASWELL_ULT_S_GT1: //
        case GMA_HASWELL_ULT_S_GT2: //
        case GMA_HASWELL_ULT_S_GT3: //
        case GMA_HASWELL_ULT_B_GT1: //
        case GMA_HASWELL_ULT_B_GT2: //
        case GMA_HASWELL_ULT_B_GT3: //
        case GMA_HASWELL_ULT_E_GT1: // HD Graphics
        //case GMA_HASWELL_SDV_D_GT1_IG: // 
        //case GMA_HASWELL_SDV_D_GT2_IG: // 
        //case GMA_HASWELL_SDV_D_GT2_PLUS_IG: // 
        case GMA_HASWELL_CRW_D_GT1: // 
        case GMA_HASWELL_CRW_D_GT2: // HD Graphics
        case GMA_HASWELL_CRW_S_GT1: // 
        case GMA_HASWELL_CRW_S_GT2: // 
        case GMA_HASWELL_CRW_S_GT3: // HD Graphics Iris Pro
        case GMA_HASWELL_CRW_B_GT1: // 
        case GMA_HASWELL_CRW_B_GT2: // 
        case GMA_HASWELL_CRW_B_GT3: // 
        case GMA_HASWELL_CRW_E_GT1: // 
        case GMA_HASWELL_CRW_E_GT2: // 
        case GMA_HASWELL_CRW_E_GT3: //
//            devprop_add_value(device, "AAPL00,DualLink",    HD4000_vals[10], 4);
            devprop_add_value(device, "built-in",           &BuiltIn, 1);
            devprop_add_value(device, "class-code",         ClassFix, 4);
            devprop_add_value(device, "hda-gfx",            (uint8_t *)"onboard-1", 10);
            if (getValueForKey(kIGPDeviceID, &value, &len, &bootInfo->chameleonConfig))    // MacMan
            {
                verbose("Using 0x%s for unsupported device id injection.\n", value);
                hex2devprop(value, igp_device_id, 2);
                devprop_add_value(device, "device-id",igp_device_id, 4);
            }
            else
            {
                devprop_add_value(device, "device-id",  hd4600_device_id, 4); // MacMan Inject Supported ID Instead of Patching Kext
                verbose("Using default unsupported device id injection of 0x0412\n");
            }
            if (getValueForKey(kIGPlatformID, &value, &len, &bootInfo->chameleonConfig))
            {
                verbose("Using 0x%s for ig-platform-id\n", value);
                hex2devprop(value, ig_platform_id, 4);
                devprop_add_value(device, "AAPL,ig-platform-id", ig_platform_id, 4);
            }
            else
            {
                devprop_add_value(device, "AAPL,ig-platform-id", ig_id_4600, 4);
                verbose("Using default ig-platform-id of 0x0d220003\n");
            }
            break;
            
        case GMA_HASWELL_E_GT2: // HD Graphics 4400
            devprop_add_value(device, "built-in",           &BuiltIn, 1);
            devprop_add_value(device, "class-code",         ClassFix, 4);
            devprop_add_value(device, "hda-gfx",            (uint8_t *)"onboard-1", 10);
            if (getValueForKey(kIGPDeviceID, &value, &len, &bootInfo->chameleonConfig))    // MacMan
            {
                verbose("Using 0x%s for unsupported device id injection.\n", value);
                hex2devprop(value, igp_device_id, 2);
                devprop_add_value(device, "device-id",igp_device_id, 4);
            }
            else
            {
                devprop_add_value(device, "device-id", hd4400_mobile_device_id, 4); // MacMan Inject Supported ID Instead of Patching Kext
                verbose("Using default unsupported device id injection of 0a26\n");
            }
            if (getValueForKey(kIGPlatformID, &value, &len, &bootInfo->chameleonConfig))
            {
                verbose("Using 0x%s for ig-platform-id\n", value);
                hex2devprop(value, ig_platform_id, 4);
                devprop_add_value(device, "AAPL,ig-platform-id", ig_platform_id, 4);
            }
            else
            {
                devprop_add_value(device, "AAPL,ig-platform-id", ig_id_4600, 4);
                verbose("Using default ig-platform-id of 0x0d220003\n");
            }
            break;

            //
            // Unsupported Haswell Mobile Device IDs
            //
        case GMA_HASWELL_ULT_M_GT1: // HD Graphics Mobile
        case GMA_HASWELL_CRW_M_GT1: //
        case GMA_HASWELL_CRW_M_GT2: // HD Graphics 4600 Mobile
        case GMA_HASWELL_CRW_M_GT2_PLUS_IG: //
//            devprop_add_value(device, "AAPL00,DualLink",    HD4000_vals[10], 4);
            devprop_add_value(device, "built-in",           &BuiltIn, 1);
            devprop_add_value(device, "class-code",         ClassFix, 4);
            devprop_add_value(device, "hda-gfx",            (uint8_t *)"onboard-1", 10);
            if (getValueForKey(kIGPDeviceID, &value, &len, &bootInfo->chameleonConfig))    // MacMan
            {
                verbose("Using 0x%s for unsupported device id injection.\n", value);
                hex2devprop(value, igp_device_id, 2);
                devprop_add_value(device, "device-id",igp_device_id, 4);
            }
            else
            {
                devprop_add_value(device, "device-id", hd4600_mobile_device_id, 4); // MacMan Inject Supported ID Instead of Patching Kext
                verbose("Using default unsupported device id injection of 0x0416\n");
            }
            if (getValueForKey(kIGPlatformID, &value, &len, &bootInfo->chameleonConfig))
            {
                verbose("Using 0x%s for ig-platform-id\n", value);
                hex2devprop(value, ig_platform_id, 4);
                devprop_add_value(device, "AAPL,ig-platform-id", ig_platform_id, 4);
            }
            else
            {
                devprop_add_value(device, "AAPL,ig-platform-id", ig_id_4600_mobile, 4);
                verbose("Using default ig-platform-id of 0x0a260006\n");
            }
            break;
            
            //
            // Supported Broadwell Desktop Device IDs
            //
            
            //
            // Currently none available as of 2015-04-08
            //
            
            //
            // Supported Broadwell Mobile Device IDs
            //
        case GMA_BRODWELL_ULT_M_GT1:	// HD Graphics ??? found in kext
        case GMA_BRODWELL_ULX_M_GT1:	// HD Graphics ??? found in kext
        case GMA_BRODWELL_BDW_M_GT2:	// HD Graphics 5600 and found in kext
        case GMA_BRODWELL_ULT_M_GT2:	// HD Graphics 5500 and found in kext
        case GMA_BRODWELL_ULX_M_GT2:	// HD Graphics 5300 and found in kext
        case GMA_BRODWELL_ULT_M_GT3:	// HD Graphics 6000 and found in kext
        case GMA_BRODWELL_BDW_M_GT3e:	// Iris Pro 6200 and found in kext
        case GMA_BRODWELL_LVT_M_GT3:	// Iris 6100 and found in kext
            devprop_add_value(device, "built-in",           &BuiltIn, 1);
            devprop_add_value(device, "class-code",         ClassFix, 4);
            devprop_add_value(device, "device-id",			(uint8_t*)&device_id, sizeof(device_id));
            devprop_add_value(device, "hda-gfx",            (uint8_t *)"onboard-1", 10);
            if (getValueForKey(kIGPlatformID, &value, &len, &bootInfo->chameleonConfig))
            {
                verbose("Using 0x%s for ig-platform-id\n", value);
                hex2devprop(value, ig_platform_id, 4);
                devprop_add_value(device, "AAPL,ig-platform-id", ig_platform_id, 4);
            }
            else
            {
                devprop_add_value(device, "AAPL,ig-platform-id", ig_id_5500_mobile, 4);
                verbose("Using default ig-platform-id of 0x16160000\n");
            }
            break;

        default:
            break;
	}

	stringdata = malloc(sizeof(uint8_t) * string->length);
	if (!stringdata)
	{
		printf("No stringdata.\n");
		pause();
		return false;
	}

	verbose("---------------------------------------------\n");
	memcpy(stringdata, (uint8_t*)devprop_generate_string(string), string->length);
	stringlength = string->length;

	return true;
}
