/*
 *	NVidia injector
 *
 *	Copyright (C) 2009	Jasmin Fazlic, iNDi
 *
 *	NVidia injector is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	NVidia driver and injector is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with NVidia injector.	 If not, see <http://www.gnu.org/licenses/>.
 */ 
/*
 * Alternatively you can choose to comply with APSL
 */
 
 
/*
 * DCB-Table parsing is based on software (nouveau driver) originally distributed under following license:
 *
 *
 * Copyright 2005-2006 Erik Waling
 * Copyright 2006 Stephane Marchesin
 * Copyright 2007-2009 Stuart Bennett
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
#include "platform.h"
#include "device_inject.h"
#include "nvidia.h"

#ifndef DEBUG_NVIDIA
#define DEBUG_NVIDIA 0
#endif

#if DEBUG_NVIDIA
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

#define NVIDIA_ROM_SIZE				0x10000
#define PATCH_ROM_SUCCESS			1
#define PATCH_ROM_SUCCESS_HAS_LVDS	2
#define PATCH_ROM_FAILED			0
#define MAX_NUM_DCB_ENTRIES			16
#define TYPE_GROUPED				0xff

extern uint32_t devices_number;

const char *nvidia_compatible_0[]       =	{ "@0,compatible",	"NVDA,NVMac"	 };
const char *nvidia_compatible_1[]       =	{ "@1,compatible",	"NVDA,NVMac"	 };
const char *nvidia_device_type_0[]      =	{ "@0,device_type", "display"		 };
const char *nvidia_device_type_1[]      =	{ "@1,device_type", "display"		 };
const char *nvidia_device_type[]        =	{ "device_type",	"NVDA,Parent"	 };
const char *nvidia_device_type_child[]	=	{ "device_type",	"NVDA,Child"	 };
const char *nvidia_name_0[]             =	{ "@0,name",		"NVDA,Display-A" };
const char *nvidia_name_1[]             =	{ "@1,name",		"NVDA,Display-B" };
const char *nvidia_slot_name[]          =	{ "AAPL,slot-name", "Slot-1"		 };

static uint8_t default_NVCAP[]= {
	0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a,
	0x00, 0x00, 0x00, 0x00
};

#define NVCAP_LEN ( sizeof(default_NVCAP) / sizeof(uint8_t) )

static uint8_t default_dcfg_0[]		=	{0x03, 0x01, 0x03, 0x00};
static uint8_t default_dcfg_1[]		=	{0xff, 0xff, 0x00, 0x01};

// uint8_t connector_type_1[]		=	{0x00, 0x08, 0x00, 0x00};

#define DCFG0_LEN ( sizeof(default_dcfg_0) / sizeof(uint8_t) )
#define DCFG1_LEN ( sizeof(default_dcfg_1) / sizeof(uint8_t) )

static uint8_t default_NVPM[]= {
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

#define NVPM_LEN ( sizeof(default_NVPM) / sizeof(uint8_t) )

static struct nv_chipsets_t NVKnownChipsets[] = {
	{ 0x00000000,	/*0x00000000,*/	"Unknown" },
//========================================
	// 0000 - 0040	
    //	{ 0x10DE0001,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE0002,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE0003,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE0005,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE0006,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE0007,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE0008,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE0009,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE000A,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE000B,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE000C,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE000D,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE0010,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE0011,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE0012,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE0014,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE0018,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
	// 0040 - 004F	
	{ 0x10DE0040,	/*0x00000000,*/	"GeForce 6800 Ultra" },
	{ 0x10DE0041,	/*0x00000000,*/	"GeForce 6800" },
	{ 0x10DE0042,	/*0x00000000,*/	"GeForce 6800 LE" },
	{ 0x10DE0043,	/*0x00000000,*/	"GeForce 6800 XE" },
	{ 0x10DE0044,	/*0x00000000,*/	"GeForce 6800 XT" },
	{ 0x10DE0045,	/*0x00000000,*/	"GeForce 6800 GT" },
	{ 0x10DE0046,	/*0x00000000,*/	"GeForce 6800 GT" },
	{ 0x10DE0047,	/*0x00000000,*/	"GeForce 6800 GS" },
	{ 0x10DE0048,	/*0x00000000,*/	"GeForce 6800 XT" },
	{ 0x10DE004D,	/*0x00000000,*/	"Quadro FX 3400" },
	{ 0x10DE004E,	/*0x00000000,*/	"Quadro FX 4000" },
	// 0050 - 005F
    //	{ 0x10DE0059,	/*0x00000000,*/	"CK804 AC'97 Audio Controller" },
	// 0060 - 006F
    //	{ 0x10DE006A,	/*0x00000000,*/	"nForce2 AC97 Audio Controler (MCP)" },
    //	{ 0x10DE0067,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE0073,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
	// 0070 - 007F
	// 0080 - 008F
	// 0090 - 009F
	{ 0x10DE0090,	/*0x00000000,*/	"GeForce 7800 GTX" },
	{ 0x10DE0091,	/*0x00000000,*/	"GeForce 7800 GTX" },
	{ 0x10DE0092,	/*0x00000000,*/	"GeForce 7800 GT" },
	{ 0x10DE0093,	/*0x00000000,*/	"GeForce 7800 GS" },
	{ 0x10DE0095,	/*0x00000000,*/	"GeForce 7800 SLI" },
	{ 0x10DE0098,	/*0x00000000,*/	"GeForce Go 7800" },
	{ 0x10DE0099,	/*0x00000000,*/	"GeForce Go 7800 GTX" },
	{ 0x10DE009D,	/*0x00000000,*/	"Quadro FX 4500" },
	// 00A0 - 00AF	
	// 00B0 - 00BF	
	// 00C0 - 00CF	
	{ 0x10DE00C0,	/*0x00000000,*/	"GeForce 6800 GS" },
	{ 0x10DE00C1,	/*0x00000000,*/	"GeForce 6800" },
	{ 0x10DE00C2,	/*0x00000000,*/	"GeForce 6800 LE" },
	{ 0x10DE00C3,	/*0x00000000,*/	"GeForce 6800 XT" },
	{ 0x10DE00C8,	/*0x00000000,*/	"GeForce Go 6800" },
	{ 0x10DE00C9,	/*0x00000000,*/	"GeForce Go 6800 Ultra" },
	{ 0x10DE00CC,	/*0x00000000,*/	"Quadro FX Go1400" },
	{ 0x10DE00CD,	/*0x00000000,*/	"Quadro FX 3450/4000 SDI" },
	{ 0x10DE00CE,	/*0x00000000,*/	"Quadro FX 1400" },
    //	{ 0x10DE00DA,	/*0x00000000,*/	"nForce3 Audio" },
	// 00D0 - 00DF	
	// 00E0 - 00EF	
	// 00F0 - 00FF	
	{ 0x10DE00F1,	/*0x00000000,*/	"GeForce 6600 GT" },
	{ 0x10DE00F2,	/*0x00000000,*/	"GeForce 6600" },
	{ 0x10DE00F3,	/*0x00000000,*/	"GeForce 6200" },
	{ 0x10DE00F4,	/*0x00000000,*/	"GeForce 6600 LE" },
	{ 0x10DE00F5,	/*0x00000000,*/	"GeForce 7800 GS" },
	{ 0x10DE00F6,	/*0x00000000,*/	"GeForce 6800 GS/XT" },
	{ 0x10DE00F8,	/*0x00000000,*/	"Quadro FX 3400/4400" },
	{ 0x10DE00F9,	/*0x00000000,*/	"GeForce 6800 Series GPU" },
	// 0100 - 010F	
	// 0110 - 011F	
	// 0120 - 012F	
	// 0130 - 013F	
	// 0140 - 014F	
	{ 0x10DE0140,	/*0x00000000,*/	"GeForce 6600 GT" },
	{ 0x10DE0141,	/*0x00000000,*/	"GeForce 6600" },
	{ 0x10DE0142,	/*0x00000000,*/	"GeForce 6600 LE" },
	{ 0x10DE0143,	/*0x00000000,*/	"GeForce 6600 VE" },
	{ 0x10DE0144,	/*0x00000000,*/	"GeForce Go 6600" },
	{ 0x10DE0145,	/*0x00000000,*/	"GeForce 6610 XL" },
	{ 0x10DE0146,	/*0x00000000,*/	"GeForce Go 6600 TE/6200 TE" },
	{ 0x10DE0147,	/*0x00000000,*/	"GeForce 6700 XL" },
	{ 0x10DE0148,	/*0x00000000,*/	"GeForce Go 6600" },
	{ 0x10DE0149,	/*0x00000000,*/	"GeForce Go 6600 GT" },
	{ 0x10DE014A,	/*0x00000000,*/	"Quadro NVS 440" },
	{ 0x10DE014C,	/*0x00000000,*/	"Quadro FX 550" },
	{ 0x10DE014D,	/*0x00000000,*/	"Quadro FX 550" },
	{ 0x10DE014E,	/*0x00000000,*/	"Quadro FX 540" },
	{ 0x10DE014F,	/*0x00000000,*/	"GeForce 6200" },
	// 0150 - 015F	
	// 0160 - 016F	
	{ 0x10DE0160,	/*0x00000000,*/	"GeForce 6500" },
	{ 0x10DE0161,	/*0x00000000,*/	"GeForce 6200 TurboCache(TM)" },
	{ 0x10DE0162,	/*0x00000000,*/	"GeForce 6200SE TurboCache(TM)" },
	{ 0x10DE0163,	/*0x00000000,*/	"GeForce 6200 LE" },
	{ 0x10DE0164,	/*0x00000000,*/	"GeForce Go 6200" },
	{ 0x10DE0165,	/*0x00000000,*/	"Quadro NVS 285" },
	{ 0x10DE0166,	/*0x00000000,*/	"GeForce Go 6400" },
	{ 0x10DE0167,	/*0x00000000,*/	"GeForce Go 6200" },
	{ 0x10DE0168,	/*0x00000000,*/	"GeForce Go 6400" },
	{ 0x10DE0169,	/*0x00000000,*/	"GeForce 6250" },
	{ 0x10DE016A,	/*0x00000000,*/	"GeForce 7100 GS" },
	{ 0x10DE016C,	/*0x00000000,*/	"NVIDIA NV44GLM" }, //
	{ 0x10DE016D,	/*0x00000000,*/	"NVIDIA NV44GLM" }, //
	// 0170 - 017F	
	// 0180 - 018F	
	// 0190 - 019F		
	{ 0x10DE0191,	/*0x00000000,*/	"GeForce 8800 GTX" },
	{ 0x10DE0193,	/*0x00000000,*/	"GeForce 8800 GTS" },
	{ 0x10DE0194,	/*0x00000000,*/	"GeForce 8800 Ultra" },
	{ 0x10DE0197,	/*0x00000000,*/	"Tesla C870" },
	{ 0x10DE019D,	/*0x00000000,*/	"Quadro FX 5600" },
	{ 0x10DE019E,	/*0x00000000,*/	"Quadro FX 4600" },
	// 01A0 - 01AF
	// 01B0 - 01BF
    //	{ 0x10DE01B1,	/*0x00000000,*/	"nForce AC'97 Audio Controller" },
	// 01C0 - 01CF
	// 01D0 - 01DF
	{ 0x10DE01D0,	/*0x00000000,*/	"GeForce 7350 LE" },
	{ 0x10DE01D1,	/*0x00000000,*/	"GeForce 7300 LE" },
	{ 0x10DE01D2,	/*0x00000000,*/	"GeForce 7550 LE" },
	{ 0x10DE01D3,	/*0x00000000,*/	"GeForce 7300 SE/7200 GS" },
	{ 0x10DE01D6,	/*0x00000000,*/	"GeForce Go 7200" },
	{ 0x10DE01D7,	/*0x00000000,*/	"GeForce Go 7300" },
	{ 0x10DE01D8,	/*0x00000000,*/	"GeForce Go 7400" },
	{ 0x10DE01D9,	/*0x00000000,*/	"GeForce Go 7400 GS" },
	{ 0x10DE01DA,	/*0x00000000,*/	"Quadro NVS 110M" },
	{ 0x10DE01DB,	/*0x00000000,*/	"Quadro NVS 120M" },
	{ 0x10DE01DC,	/*0x00000000,*/	"Quadro FX 350M" },
	{ 0x10DE01DD,	/*0x00000000,*/	"GeForce 7500 LE" },
	{ 0x10DE01DE,	/*0x00000000,*/	"Quadro FX 350" },
	{ 0x10DE01DF,	/*0x00000000,*/	"GeForce 7300 GS" },
	// 01E0 - 01EF	
	// 01F0 - 01FF
	{ 0x10DE01F0,	/*0x00000000,*/	"GeForce4 MX" }, //
	// 0200 - 020F	
	// 0210 - 021F	
	{ 0x10DE0211,	/*0x00000000,*/	"GeForce 6800" },
	{ 0x10DE0212,	/*0x00000000,*/	"GeForce 6800 LE" },
	{ 0x10DE0215,	/*0x00000000,*/	"GeForce 6800 GT" },
	{ 0x10DE0218,	/*0x00000000,*/	"GeForce 6800 XT" },
	// 0220 - 022F
	{ 0x10DE0221,	/*0x00000000,*/	"GeForce 6200" },
	{ 0x10DE0222,	/*0x00000000,*/	"GeForce 6200 A-LE" },
	{ 0x10DE0228,	/*0x00000000,*/	"NVIDIA NV44M" }, // 
	// 0230 - 023F
	// 0240 - 024F
	{ 0x10DE0240,	/*0x00000000,*/	"GeForce 6150" },
	{ 0x10DE0241,	/*0x00000000,*/	"GeForce 6150 LE" },
	{ 0x10DE0242,	/*0x00000000,*/	"GeForce 6100" },
	{ 0x10DE0243,	/*0x00000000,*/	"NVIDIA C51" }, //
	{ 0x10DE0244,	/*0x00000000,*/	"GeForce Go 6150" },
	{ 0x10DE0245,	/*0x00000000,*/	"Quadro NVS 210S / GeForce 6150LE" },
	{ 0x10DE0247,	/*0x00000000,*/	"GeForce Go 6100" },
	// 0250 - 025F
	{ 0x10DE025B,	/*0x00000000,*/	"Quadro4 700 XGL" }, //
	// 0260 - 026F
	// 0270 - 027F
	// 0280 - 028F
	// 0290 - 029F
	{ 0x10DE0290,	/*0x00000000,*/	"GeForce 7900 GTX" },
	{ 0x10DE0291,	/*0x00000000,*/	"GeForce 7900 GT/GTO" },
	{ 0x10DE0292,	/*0x00000000,*/	"GeForce 7900 GS" },
	{ 0x10DE0293,	/*0x00000000,*/	"GeForce 7950 GX2" },
	{ 0x10DE0294,	/*0x00000000,*/	"GeForce 7950 GX2" },
	{ 0x10DE0295,	/*0x00000000,*/	"GeForce 7950 GT" },
	{ 0x10DE0298,	/*0x00000000,*/	"GeForce Go 7900 GS" },
	{ 0x10DE0299,	/*0x00000000,*/	"GeForce Go 7900 GTX" },
	{ 0x10DE029A,	/*0x00000000,*/	"Quadro FX 2500M" },
	{ 0x10DE029B,	/*0x00000000,*/	"Quadro FX 1500M" },
	{ 0x10DE029C,	/*0x00000000,*/	"Quadro FX 5500" },
	{ 0x10DE029D,	/*0x00000000,*/	"Quadro FX 3500" },
	{ 0x10DE029E,	/*0x00000000,*/	"Quadro FX 1500" },
	{ 0x10DE029F,	/*0x00000000,*/	"Quadro FX 4500 X2" },
	// 02A0 - 02AF
	// 02B0 - 02BF
	// 02C0 - 02CF
	// 02D0 - 02DF
	// 02E0 - 02EF
	{ 0x10DE02E0,	/*0x00000000,*/	"GeForce 7600 GT" },
	{ 0x10DE02E1,	/*0x00000000,*/	"GeForce 7600 GS" },
	{ 0x10DE02E2,	/*0x00000000,*/	"GeForce 7300 GT" },
	{ 0x10DE02E3,	/*0x00000000,*/	"GeForce 7900 GS" },
	{ 0x10DE02E4,	/*0x00000000,*/	"GeForce 7950 GT" },
	// 02F0 - 02FF
	// 0300 - 030F
	{ 0x10DE0301,	/*0x00000000,*/	"GeForce FX 5800 Ultra" },
	{ 0x10DE0302,	/*0x00000000,*/	"GeForce FX 5800" },
	{ 0x10DE0308,	/*0x00000000,*/	"Quadro FX 2000" },
	{ 0x10DE0309,	/*0x00000000,*/	"Quadro FX 1000" },
	// 0310 - 031F
	{ 0x10DE0311,	/*0x00000000,*/	"GeForce FX 5600 Ultra" },
	{ 0x10DE0312,	/*0x00000000,*/	"GeForce FX 5600" },
	{ 0x10DE0314,	/*0x00000000,*/	"GeForce FX 5600XT" },
	{ 0x10DE031A,	/*0x00000000,*/	"GeForce FX Go5600" },
	{ 0x10DE031B,	/*0x00000000,*/	"GeForce FX Go5650" },
	{ 0x10DE031C,	/*0x00000000,*/	"Quadro FX Go700" },
	// 0320 - 032F
	{ 0x10DE0320,	/*0x00000000,*/	"GeForce FX 5200" }, //
	{ 0x10DE0321,	/*0x00000000,*/	"GeForce FX 5200 Ultra" }, //
	{ 0x10DE0322,	/*0x00000000,*/	"GeForce FX 5200" }, //
	{ 0x10DE0323,	/*0x00000000,*/	"GeForce FX 5200 LE" }, //
	{ 0x10DE0324,	/*0x00000000,*/	"GeForce FX Go5200" },
	{ 0x10DE0325,	/*0x00000000,*/	"GeForce FX Go5250" },
	{ 0x10DE0326,	/*0x00000000,*/	"GeForce FX 5500" },
	{ 0x10DE0328,	/*0x00000000,*/	"GeForce FX Go5200 32M/64M" },
	{ 0x10DE0329,	/*0x00000000,*/	"GeForce FX Go5200" }, //
	{ 0x10DE032A,	/*0x00000000,*/	"Quadro NVS 55/280 PCI" },
	{ 0x10DE032B,	/*0x00000000,*/	"Quadro FX 500/600 PCI" },
	{ 0x10DE032C,	/*0x00000000,*/	"GeForce FX Go53xx Series" },
	{ 0x10DE032D,	/*0x00000000,*/	"GeForce FX Go5100" },
    //	{ 0x10DE032F,	/*0x00000000,*/	"NVIDIA NV34GL" },//
	// 0330 - 033F
	{ 0x10DE0330,	/*0x00000000,*/	"GeForce FX 5900 Ultra" },
	{ 0x10DE0331,	/*0x00000000,*/	"GeForce FX 5900" },
	{ 0x10DE0332,	/*0x00000000,*/	"GeForce FX 5900XT" },
	{ 0x10DE0333,	/*0x00000000,*/	"GeForce FX 5950 Ultra" },
	{ 0x10DE0334,	/*0x00000000,*/	"GeForce FX 5900ZT" },
	{ 0x10DE0338,	/*0x00000000,*/	"Quadro FX 3000" },
	{ 0x10DE033F,	/*0x00000000,*/	"Quadro FX 700" },
	// 0340 - 034F
	{ 0x10DE0341,	/*0x00000000,*/	"GeForce FX 5700 Ultra" },
	{ 0x10DE0342,	/*0x00000000,*/	"GeForce FX 5700" },
	{ 0x10DE0343,	/*0x00000000,*/	"GeForce FX 5700LE" },
	{ 0x10DE0344,	/*0x00000000,*/	"GeForce FX 5700VE" },
    //	{ 0x10DE0345,	/*0x00000000,*/	"NVIDIA NV36.5" }, //
	{ 0x10DE0347,	/*0x00000000,*/	"GeForce FX Go5700" },
	{ 0x10DE0348,	/*0x00000000,*/	"GeForce FX Go5700" },
    //	{ 0x10DE0349,	/*0x00000000,*/	"NVIDIA NV36M Pro" }, //
    //	{ 0x10DE034B,	/*0x00000000,*/	"NVIDIA NV36MAP" }, //
	{ 0x10DE034C,	/*0x00000000,*/	"Quadro FX Go1000" },
	{ 0x10DE034E,	/*0x00000000,*/	"Quadro FX 1100" },
    //	{ 0x10DE034F,	/*0x00000000,*/	"NVIDIA NV36GL" }, //
	// 0350 - 035F
	// 0360 - 036F
	// 0370 - 037F
	// 0380 - 038F
	{ 0x10DE038B,	/*0x00000000,*/	"GeForce 7650 GS" },
	// 0390 - 039F
	{ 0x10DE0390,	/*0x00000000,*/	"GeForce 7650 GS" },
	{ 0x10DE0391,	/*0x00000000,*/	"GeForce 7600 GT" },
	{ 0x10DE0392,	/*0x00000000,*/	"GeForce 7600 GS" },
	{ 0x10DE0393,	/*0x00000000,*/	"GeForce 7300 GT" },
	{ 0x10DE0394,	/*0x00000000,*/	"GeForce 7600 LE" },
	{ 0x10DE0395,	/*0x00000000,*/	"GeForce 7300 GT" },
	{ 0x10DE0397,	/*0x00000000,*/	"GeForce Go 7700" },
	{ 0x10DE0398,	/*0x00000000,*/	"GeForce Go 7600" },
	{ 0x10DE0399,	/*0x00000000,*/	"GeForce Go 7600 GT"},
	{ 0x10DE039A,	/*0x00000000,*/	"Quadro NVS 300M" },
	{ 0x10DE039B,	/*0x00000000,*/	"GeForce Go 7900 SE" },
	{ 0x10DE039C,	/*0x00000000,*/	"Quadro FX 560M" },
	{ 0x10DE039E,	/*0x00000000,*/	"Quadro FX 560" },
	// 03A0 - 03AF
	// 03B0 - 03BF
	// 03C0 - 03CF
	// 03D0 - 03DF
	{ 0x10DE03D0,	/*0x00000000,*/	"GeForce 6150SE nForce 430" },
	{ 0x10DE03D1,	/*0x00000000,*/	"GeForce 6100 nForce 405" },
	{ 0x10DE03D2,	/*0x00000000,*/	"GeForce 6100 nForce 400" },
	{ 0x10DE03D5,	/*0x00000000,*/	"GeForce 6100 nForce 420" },
	{ 0x10DE03D6,	/*0x00000000,*/	"GeForce 7025 / nForce 630a" },
	// 03E0 - 03EF
	// 03F0 - 03FF
	// 0400 - 040F
	{ 0x10DE0400,	/*0x00000000,*/	"GeForce 8600 GTS" },
	{ 0x10DE0401,	/*0x00000000,*/	"GeForce 8600 GT" },
	{ 0x10DE0402,	/*0x00000000,*/	"GeForce 8600 GT" },
	{ 0x10DE0403,	/*0x00000000,*/	"GeForce 8600 GS" },
	{ 0x10DE0404,	/*0x00000000,*/	"GeForce 8400 GS" },
	{ 0x10DE0405,	/*0x00000000,*/	"GeForce 9500M GS" },
	{ 0x10DE0406,	/*0x00000000,*/	"GeForce 8300 GS" },
	{ 0x10DE0407,	/*0x00000000,*/	"GeForce 8600M GT" },
	{ 0x10DE0408,	/*0x00000000,*/	"GeForce 9650M GS" },
	{ 0x10DE0409,	/*0x00000000,*/	"GeForce 8700M GT" },
	{ 0x10DE040A,	/*0x00000000,*/	"Quadro FX 370" },
	{ 0x10DE040B,	/*0x00000000,*/	"Quadro NVS 320M" },
	{ 0x10DE040C,	/*0x00000000,*/	"Quadro FX 570M" },
	{ 0x10DE040D,	/*0x00000000,*/	"Quadro FX 1600M" },
	{ 0x10DE040E,	/*0x00000000,*/	"Quadro FX 570" },
	{ 0x10DE040F,	/*0x00000000,*/	"Quadro FX 1700" },
	// 0410 - 041F
	{ 0x10DE0410,	/*0x00000000,*/	"GeForce GT 330" },
	// 0420 - 042F
	{ 0x10DE0420,	/*0x00000000,*/	"GeForce 8400 SE" },
	{ 0x10DE0421,	/*0x00000000,*/	"GeForce 8500 GT" },
	{ 0x10DE0422,	/*0x00000000,*/	"GeForce 8400 GS" },
	{ 0x10DE0423,	/*0x00000000,*/	"GeForce 8300 GS" },
	{ 0x10DE0424,	/*0x00000000,*/	"GeForce 8400 GS" },
	{ 0x10DE0425,	/*0x00000000,*/	"GeForce 8600M GS" },
	{ 0x10DE0426,	/*0x00000000,*/	"GeForce 8400M GT" },
	{ 0x10DE0427,	/*0x00000000,*/	"GeForce 8400M GS" },
	{ 0x10DE0428,	/*0x00000000,*/	"GeForce 8400M G" },
	{ 0x10DE0429,	/*0x00000000,*/	"Quadro NVS 140M" },
	{ 0x10DE042A,	/*0x00000000,*/	"Quadro NVS 130M" },
	{ 0x10DE042B,	/*0x00000000,*/	"Quadro NVS 135M" },
	{ 0x10DE042C,	/*0x00000000,*/	"GeForce 9400 GT" },
	{ 0x10DE042D,	/*0x00000000,*/	"Quadro FX 360M" },
	{ 0x10DE042E,	/*0x00000000,*/	"GeForce 9300M G" },
	{ 0x10DE042F,	/*0x00000000,*/	"Quadro NVS 290" },
	// 0430 - 043F
	// 0440 - 044F
	// 0450 - 045F
	// 0460 - 046F
	// 0470 - 047F
	// 0480 - 048F
	// 0490 - 049F
	// 04A0 - 04AF
	// 04B0 - 04BF
	// 04C0 - 04CF
	{ 0x10DE04C0,	/*0x00000000,*/	"NVIDIA G78" }, //
	{ 0x10DE04C1,	/*0x00000000,*/	"NVIDIA G78" }, //
	{ 0x10DE04C2,	/*0x00000000,*/	"NVIDIA G78" }, //
	{ 0x10DE04C3,	/*0x00000000,*/	"NVIDIA G78" }, //
	{ 0x10DE04C4,	/*0x00000000,*/	"NVIDIA G78" }, //
	{ 0x10DE04C5,	/*0x00000000,*/	"NVIDIA G78" }, //
	{ 0x10DE04C6,	/*0x00000000,*/	"NVIDIA G78" }, //
	{ 0x10DE04C7,	/*0x00000000,*/	"NVIDIA G78" }, //
	{ 0x10DE04C8,	/*0x00000000,*/	"NVIDIA G78" }, //
	{ 0x10DE04C9,	/*0x00000000,*/	"NVIDIA G78" }, //
	{ 0x10DE04CA,	/*0x00000000,*/	"NVIDIA G78" }, //
	{ 0x10DE04CB,	/*0x00000000,*/	"NVIDIA G78" }, //
	{ 0x10DE04CC,	/*0x00000000,*/	"NVIDIA G78" }, //
	{ 0x10DE04CD,	/*0x00000000,*/	"NVIDIA G78" }, //
	{ 0x10DE04CE,	/*0x00000000,*/	"NVIDIA G78" }, //
	{ 0x10DE04CF,	/*0x00000000,*/	"NVIDIA G78" }, //
	// 04D0 - 04DF
	// 04E0 - 04EF
	// 04F0 - 04FF
	// 0500 - 050F
	// 0510 - 051F
	// 0520 - 052F
	// 0530 - 053F
	{ 0x10DE0530,	/*0x00000000,*/	"GeForce 7190M / nForce 650M" },
	{ 0x10DE0531,	/*0x00000000,*/	"GeForce 7150M / nForce 630M" },
	{ 0x10DE0533,	/*0x00000000,*/	"GeForce 7000M / nForce 610M" },
	{ 0x10DE053A,	/*0x00000000,*/	"GeForce 7050 PV / nForce 630a" },
	{ 0x10DE053B,	/*0x00000000,*/	"GeForce 7050 PV / nForce 630a" },
	{ 0x10DE053E,	/*0x00000000,*/	"GeForce 7025 / nForce 630a" },
	// 0540 - 054F
	// 0550 - 055F
	// 0560 - 056F
	// 0570 - 057F
	// 0580 - 058F
	// 0590 - 059F
	// 05A0 - 05AF
	// 05B0 - 05BF
	// 05C0 - 05CF
	// 05D0 - 05DF
	// 05E0 - 05EF
	{ 0x10DE05E0,	/*0x00000000,*/	"GeForce GTX 295" },
	{ 0x10DE05E1,	/*0x00000000,*/	"GeForce GTX 280" },
	{ 0x10DE05E2,	/*0x00000000,*/	"GeForce GTX 260" },
	{ 0x10DE05E3,	/*0x00000000,*/	"GeForce GTX 285" },
	{ 0x10DE05E4,	/*0x00000000,*/	"NVIDIA GT200" }, //
	{ 0x10DE05E5,	/*0x00000000,*/	"NVIDIA GT200" }, //
	{ 0x10DE05E6,	/*0x00000000,*/	"GeForce GTX 275" },
	{ 0x10DE05E7,	/*0x00000000,*/	"Tesla C1060" },
	{ 0x10DE05E8,	/*0x00000000,*/	"NVIDIA GT200" }, //
	{ 0x10DE05E9,	/*0x00000000,*/	"NVIDIA GT200" }, //
	{ 0x10DE05EA,	/*0x00000000,*/	"GeForce GTX 260" },
	{ 0x10DE05EB,	/*0x00000000,*/	"GeForce GTX 295" },
	{ 0x10DE05EC,	/*0x00000000,*/	"NVIDIA GT200" }, //
	{ 0x10DE05ED,	/*0x00000000,*/	"Quadroplex 2200 D2" },
	{ 0x10DE05EE,	/*0x00000000,*/	"NVIDIA GT200" }, //
	{ 0x10DE05EF,	/*0x00000000,*/	"NVIDIA GT200" }, //
	// 05F0 - 05FF
	{ 0x10DE05F0,	/*0x00000000,*/	"NVIDIA GT200" }, //
	{ 0x10DE05F1,	/*0x00000000,*/	"NVIDIA GT200" }, //
	{ 0x10DE05F2,	/*0x00000000,*/	"NVIDIA GT200" }, //
	{ 0x10DE05F3,	/*0x00000000,*/	"NVIDIA GT200" }, //
	{ 0x10DE05F4,	/*0x00000000,*/	"NVIDIA GT200" }, //
	{ 0x10DE05F5,	/*0x00000000,*/	"NVIDIA GT200" }, //
	{ 0x10DE05F6,	/*0x00000000,*/	"NVIDIA GT200" }, //
	{ 0x10DE05F7,	/*0x00000000,*/	"NVIDIA GT200" }, //
	{ 0x10DE05F8,	/*0x00000000,*/	"Quadroplex 2200 S4" },
	{ 0x10DE05F9,	/*0x00000000,*/	"Quadro CX" },
	{ 0x10DE05FA,	/*0x00000000,*/	"NVIDIA GT200" }, //
	{ 0x10DE05FB,	/*0x00000000,*/	"NVIDIA GT200" }, //
	{ 0x10DE05FC,	/*0x00000000,*/	"NVIDIA GT200" }, //
	{ 0x10DE05FD,	/*0x00000000,*/	"Quadro FX 5800" },
	{ 0x10DE05FE,	/*0x00000000,*/	"Quadro FX 4800" },
	{ 0x10DE05FF,	/*0x00000000,*/	"Quadro FX 3800" },
	// 0600 - 060F
	{ 0x10DE0600,	/*0x00000000,*/	"GeForce 8800 GTS 512" },
	{ 0x10DE0601,	/*0x00000000,*/	"GeForce 9800 GT" },
	{ 0x10DE0602,	/*0x00000000,*/	"GeForce 8800 GT" },
	{ 0x10DE0603,	/*0x00000000,*/	"GeForce GT 230" },
	{ 0x10DE0604,	/*0x00000000,*/	"GeForce 9800 GX2" },
	{ 0x10DE0605,	/*0x00000000,*/	"GeForce 9800 GT" },
	{ 0x10DE0606,	/*0x00000000,*/	"GeForce 8800 GS" },
	{ 0x10DE0607,	/*0x00000000,*/	"GeForce GTS 240" },
	{ 0x10DE0608,	/*0x00000000,*/	"GeForce 9800M GTX" },
	{ 0x10DE0609,	/*0x00000000,*/	"GeForce 8800M GTS" },
	{ 0x10DE060A,	/*0x00000000,*/	"GeForce GTX 280M" },
	{ 0x10DE060B,	/*0x00000000,*/	"GeForce 9800M GT" },
	{ 0x10DE060C,	/*0x00000000,*/	"GeForce 8800M GTX" },
	{ 0x10DE060D,	/*0x00000000,*/	"GeForce 8800 GS" },
	{ 0x10DE060F,	/*0x00000000,*/	"GeForce GTX 285M" },
	// 0610 - 061F
	{ 0x10DE0610,	/*0x00000000,*/	"GeForce 9600 GSO" },
	{ 0x10DE0611,	/*0x00000000,*/	"GeForce 8800 GT" },
	{ 0x10DE0612,	/*0x00000000,*/	"GeForce 9800 GTX" },
	{ 0x10DE0613,	/*0x00000000,*/	"GeForce 9800 GTX+" },
	{ 0x10DE0614,	/*0x00000000,*/	"GeForce 9800 GT" },
	{ 0x10DE0615,	/*0x00000000,*/	"GeForce GTS 250" },
	{ 0x10DE0617,	/*0x00000000,*/	"GeForce 9800M GTX" },
	{ 0x10DE0618,	/*0x00000000,*/	"GeForce GTX 260M" }, // Subsystem Id: 1043 202B Asus GTX 680
	{ 0x10DE0619,	/*0x00000000,*/	"Quadro FX 4700 X2" },
	{ 0x10DE061A,	/*0x00000000,*/	"Quadro FX 3700" },
	{ 0x10DE061B,	/*0x00000000,*/	"Quadro VX 200" },
	{ 0x10DE061C,	/*0x00000000,*/	"Quadro FX 3600M" },
	{ 0x10DE061D,	/*0x00000000,*/	"Quadro FX 2800M" },
	{ 0x10DE061E,	/*0x00000000,*/	"Quadro FX 3700M" },
	{ 0x10DE061F,	/*0x00000000,*/	"Quadro FX 3800M" },
	// 0620 - 062F
	{ 0x10DE0620,	/*0x00000000,*/	"NVIDIA G94" }, // GeForce 8100/8200/8300
	{ 0x10DE0621,	/*0x00000000,*/	"GeForce GT 230" },
	{ 0x10DE0622,	/*0x00000000,*/	"GeForce 9600 GT" },
	{ 0x10DE0623,	/*0x00000000,*/	"GeForce 9600 GS" },
	{ 0x10DE0624,	/*0x00000000,*/	"NVIDIA G94" }, //
	{ 0x10DE0625,	/*0x00000000,*/	"GeForce 9600 GSO 512"},
	{ 0x10DE0626,	/*0x00000000,*/	"GeForce GT 130" },
	{ 0x10DE0627,	/*0x00000000,*/	"GeForce GT 140" },
	{ 0x10DE0628,	/*0x00000000,*/	"GeForce 9800M GTS" },
	{ 0x10DE0629,	/*0x00000000,*/	"NVIDIA G94" }, //
	{ 0x10DE062A,	/*0x00000000,*/	"GeForce 9700M GTS" },
	{ 0x10DE062B,	/*0x00000000,*/	"GeForce 9800M GS" },
	{ 0x10DE062C,	/*0x00000000,*/	"GeForce 9800M GTS" },
	{ 0x10DE062D,	/*0x00000000,*/	"GeForce 9600 GT" },
	{ 0x10DE062E,	/*0x00000000,*/	"GeForce 9600 GT" },
	{ 0x10DE062F,	/*0x00000000,*/	"GeForce 9800 S" }, //
	// 0630 - 063F
	{ 0x10DE0630,	/*0x00000000,*/	"NVIDIA G94" }, //
	{ 0x10DE0631,	/*0x00000000,*/	"GeForce GTS 160M" },
	{ 0x10DE0632,	/*0x00000000,*/	"GeForce GTS 150M" },
	{ 0x10DE0633,	/*0x00000000,*/	"NVIDIA G94" }, //
	{ 0x10DE0634,	/*0x00000000,*/	"NVIDIA G94" }, //
	{ 0x10DE0635,	/*0x00000000,*/	"GeForce 9600 GSO" },
	{ 0x10DE0636,	/*0x00000000,*/	"NVIDIA G94" }, //
	{ 0x10DE0637,	/*0x00000000,*/	"GeForce 9600 GT" },
	{ 0x10DE0638,	/*0x00000000,*/	"Quadro FX 1800" },
	{ 0x10DE0639,	/*0x00000000,*/	"NVIDIA G94" }, //
	{ 0x10DE063A,	/*0x00000000,*/	"Quadro FX 2700M" },
	{ 0x10DE063B,	/*0x00000000,*/	"NVIDIA G94" }, //
	{ 0x10DE063C,	/*0x00000000,*/	"NVIDIA G94" }, //
	{ 0x10DE063D,	/*0x00000000,*/	"NVIDIA G94" }, //
	{ 0x10DE063E,	/*0x00000000,*/	"NVIDIA G94" }, //
	{ 0x10DE063F,	/*0x00000000,*/	"NVIDIA G94" }, //
	// 0640 - 064F
	{ 0x10DE0640,	/*0x00000000,*/	"GeForce 9500 GT" },
	{ 0x10DE0641,	/*0x00000000,*/	"GeForce 9400 GT" },
	{ 0x10DE0642,	/*0x00000000,*/	"GeForce 8400 GS" },
	{ 0x10DE0643,	/*0x00000000,*/	"GeForce 9500 GT" },
	{ 0x10DE0644,	/*0x00000000,*/	"GeForce 9500 GS" },
	{ 0x10DE0645,	/*0x00000000,*/	"GeForce 9500 GS" },
	{ 0x10DE0646,	/*0x00000000,*/	"GeForce GT 120" },
	{ 0x10DE0647,	/*0x00000000,*/	"GeForce 9600M GT" },
	{ 0x10DE0648,	/*0x00000000,*/	"GeForce 9600M GS" },
	{ 0x10DE0649,	/*0x00000000,*/	"GeForce 9600M GT" },
	{ 0x10DE064A,	/*0x00000000,*/	"GeForce 9700M GT" },
	{ 0x10DE064B,	/*0x00000000,*/	"GeForce 9500M G" },
	{ 0x10DE064C,	/*0x00000000,*/	"GeForce 9650M GT" },
	// 0650 - 065F
	{ 0x10DE0650,	/*0x00000000,*/	"NVIDIA G96-825" }, //
	{ 0x10DE0651,	/*0x00000000,*/	"GeForce G 110M" },
	{ 0x10DE0652,	/*0x00000000,*/	"GeForce GT 130M" },
	{ 0x10DE0653,	/*0x00000000,*/	"GeForce GT 120M" },
	{ 0x10DE0654,	/*0x00000000,*/	"GeForce GT 220M" },
	{ 0x10DE0655,	/*0x00000000,*/	"GeForce GT 120" },
	{ 0x10DE0656,	/*0x00000000,*/	"GeForce 9650 S" },
	{ 0x10DE0657,	/*0x00000000,*/	"NVIDIA G96" }, //
	{ 0x10DE0658,	/*0x00000000,*/	"Quadro FX 380" },
	{ 0x10DE0659,	/*0x00000000,*/	"Quadro FX 580" },
	{ 0x10DE065A,	/*0x00000000,*/	"Quadro FX 1700M" },
	{ 0x10DE065B,	/*0x00000000,*/	"GeForce 9400 GT" },
	{ 0x10DE065C,	/*0x00000000,*/	"Quadro FX 770M" },
	{ 0x10DE065D,	/*0x00000000,*/	"NVIDIA G96" }, //
	{ 0x10DE065E,	/*0x00000000,*/	"NVIDIA G96" }, //
	{ 0x10DE065F,	/*0x00000000,*/	"GeForce G210" },
	// 0660 - 066F
	// 0670 - 067F
	// 0680 - 068F
	// 0690 - 069F
	// 06A0 - 06AF
	{ 0x10DE06A0,	/*0x00000000,*/	"NVIDIA GT214" }, //
	// 06B0 - 06BF
	{ 0x10DE06B0,	/*0x00000000,*/	"NVIDIA GT214" }, //
	// 06C0 - 06CF
	{ 0x10DE06C0,	/*0x00000000,*/	"GeForce GTX 480" },
	{ 0x10DE06C3,	/*0x00000000,*/	"GeForce GTX D12U" },
	{ 0x10DE06C4,	/*0x00000000,*/	"GeForce GTX 465" },
	{ 0x10DE06CA,	/*0x00000000,*/	"GeForce GTX 480M" },
	{ 0x10DE06CD,	/*0x00000000,*/	"GeForce GTX 470" },
	// 06D0 - 06DF
	{ 0x10DE06D1,	/*0x00000000,*/	"Tesla C2050" },	// TODO: sub-device id: 0x0771
	{ 0x10DE06D1,	/*0x00000000,*/	"Tesla C2070" },	// TODO: sub-device id: 0x0772
	{ 0x10DE06D2,	/*0x00000000,*/	"Tesla M2070" },
	{ 0x10DE06D8,	/*0x00000000,*/	"Quadro 6000" },
	{ 0x10DE06D9,	/*0x00000000,*/	"Quadro 5000" },
	{ 0x10DE06DA,	/*0x00000000,*/	"Quadro 5000M" },
	{ 0x10DE06DC,	/*0x00000000,*/	"Quadro 6000" },
	{ 0x10DE06DD,	/*0x00000000,*/	"Quadro 4000" },
	{ 0x10DE06DE,	/*0x00000000,*/	"Tesla M2050" },	// TODO: sub-device id: 0x0846
	{ 0x10DE06DE,	/*0x00000000,*/	"Tesla M2070" },	// TODO: sub-device id: ?	
	{ 0x10DE06DF,	/*0x00000000,*/	"Tesla M2070-Q" },
	// 0x10DE06DE also applies to misc S2050, X2070, M2050, M2070
	// 06E0 - 06EF
	{ 0x10DE06E0,	/*0x00000000,*/	"GeForce 9300 GE" },
	{ 0x10DE06E1,	/*0x00000000,*/	"GeForce 9300 GS" },
	{ 0x10DE06E2,	/*0x00000000,*/	"GeForce 8400" },
	{ 0x10DE06E3,	/*0x00000000,*/	"GeForce 8400 SE" },
	{ 0x10DE06E4,	/*0x00000000,*/	"GeForce 8400 GS" },
	{ 0x10DE06E5,	/*0x00000000,*/	"GeForce 9300M GS" },
	{ 0x10DE06E6,	/*0x00000000,*/	"GeForce G100" },
	{ 0x10DE06E7,	/*0x00000000,*/	"GeForce 9300 SE" },
	{ 0x10DE06E8,	/*0x00000000,*/	"GeForce 9200M GS" },
	{ 0x10DE06E9,	/*0x00000000,*/	"GeForce 9300M GS" },
	{ 0x10DE06EA,	/*0x00000000,*/	"Quadro NVS 150M" },
	{ 0x10DE06EB,	/*0x00000000,*/	"Quadro NVS 160M" },
	{ 0x10DE06EC,	/*0x00000000,*/	"GeForce G 105M" },
	{ 0x10DE06ED,	/*0x00000000,*/	"NVIDIA G98" }, //
	{ 0x10DE06EF,	/*0x00000000,*/	"GeForce G 103M" },
	// 06F0 - 06FF
	{ 0x10DE06F0,	/*0x00000000,*/	"NVIDIA G98" }, //
	{ 0x10DE06F1,	/*0x00000000,*/	"GeForce G105M" },
	{ 0x10DE06F2,	/*0x00000000,*/	"NVIDIA G98" }, //
	{ 0x10DE06F3,	/*0x00000000,*/	"NVIDIA G98" }, //
	{ 0x10DE06F4,	/*0x00000000,*/	"NVIDIA G98" }, //
	{ 0x10DE06F5,	/*0x00000000,*/	"NVIDIA G98" }, //
	{ 0x10DE06F6,	/*0x00000000,*/	"NVIDIA G98" }, //
	{ 0x10DE06F7,	/*0x00000000,*/	"NVIDIA G98" }, //
	{ 0x10DE06F8,	/*0x00000000,*/	"Quadro NVS 420" },
	{ 0x10DE06F9,	/*0x00000000,*/	"Quadro FX 370 LP" },
	{ 0x10DE06FA,	/*0x00000000,*/	"Quadro NVS 450" },
	{ 0x10DE06FB,	/*0x00000000,*/	"Quadro FX 370M" },
	{ 0x10DE06FC,	/*0x00000000,*/	"NVIDIA G98" }, //
	{ 0x10DE06FD,	/*0x00000000,*/	"Quadro NVS 295" },
	{ 0x10DE06FE,	/*0x00000000,*/	"NVIDIA G98" }, //
	{ 0x10DE06FF,	/*0x00000000,*/	"HICx16 + Graphics" },
	// 0700 - 070F
	// 0710 - 071F
	// 0720 - 072F
	// 0730 - 073F
	// 0740 - 074F
	// 0750 - 075F
    //	{ 0x10DE0759,	/*0x00000000,*/	"nVidia Standard Dual Channel PCI IDE Controller" },
	// 0760 - 076F
	// 0770 - 077F
    //	{ 0x10DE0774,	/*0x00000000,*/	"nVidia Microsoft UAA Bus Driver for High Definition Audio" },
    //	{ 0x10DE077B,	/*0x00000000,*/	"nVidia Standard OpenHCD USB Host Controller" },
    //	{ 0x10DE077C,	/*0x00000000,*/	"nVidia Standard Enhanced PCI to USB Host Controller" },
    //	{ 0x10DE077D,	/*0x00000000,*/	"nVidia Standard OpenHCD USB Host Controller" },
    //	{ 0x10DE077E,	/*0x00000000,*/	"nVidia Standard Enhanced PCI to USB Host Controller" },
	// 0780 - 078F
	// 0790 - 079F
	// 07A0 - 07AF
	// 07B0 - 07BF
	// 07C0 - 07CF
	// 07D0 - 07DF
	// 07E0 - 07EF
	{ 0x10DE07E0,	/*0x00000000,*/	"GeForce 7150 / nForce 630i" },
	{ 0x10DE07E1,	/*0x00000000,*/	"GeForce 7100 / nForce 630i" },
	{ 0x10DE07E2,	/*0x00000000,*/	"GeForce 7050 / nForce 630i" },
	{ 0x10DE07E3,	/*0x00000000,*/	"GeForce 7050 / nForce 610i" },
    //	{ 0x10DE07E4,	/*0x00000000,*/	"NVIDIA MCP73" },
	{ 0x10DE07E5,	/*0x00000000,*/	"GeForce 7050 / nForce 620i" },
    //	{ 0x10DE07E6,	/*0x00000000,*/	"NVIDIA MCP73" },
    //	{ 0x10DE07E7,	/*0x00000000,*/	"NVIDIA MCP73" },
    //	{ 0x10DE07E8,	/*0x00000000,*/	"NVIDIA MCP73" },
    //	{ 0x10DE07E9,	/*0x00000000,*/	"NVIDIA MCP73" },
    //	{ 0x10DE07EA,	/*0x00000000,*/	"NVIDIA MCP73" },
    //	{ 0x10DE07EB,	/*0x00000000,*/	"NVIDIA MCP73" },
    //	{ 0x10DE07ED,	/*0x00000000,*/	"NVIDIA MCP73" },
    //	{ 0x10DE07EE,	/*0x00000000,*/	"NVIDIA MCP73" },
    //	{ 0x10DE07EF,	/*0x00000000,*/	"NVIDIA MCP73" },
	// 07F0 - 07FF
	// 0800 - 080F
	// 0810 - 081F
	// 0820 - 082F
	// 0830 - 083F
	// 0840 - 084F
	{ 0x10DE0840,	/*0x00000000,*/	"GeForce 8200M" },
    //	{ 0x10DE0841,	/*0x00000000,*/	"NVIDIA MCP77/78" },
    //	{ 0x10DE0842,	/*0x00000000,*/	"NVIDIA MCP77/78" },
	{ 0x10DE0844,	/*0x00000000,*/	"GeForce 9100M G" },
	{ 0x10DE0845,	/*0x00000000,*/	"GeForce 8200M G" },
	{ 0x10DE0846,	/*0x00000000,*/	"GeForce 9200" },
	{ 0x10DE0847,	/*0x00000000,*/	"GeForce 9100" },
	{ 0x10DE0848,	/*0x00000000,*/	"GeForce 8300" },
	{ 0x10DE0849,	/*0x00000000,*/	"GeForce 8200" },
	{ 0x10DE084A,	/*0x00000000,*/	"nForce 730a" },
	{ 0x10DE084B,	/*0x00000000,*/	"GeForce 9200" },
	{ 0x10DE084C,	/*0x00000000,*/	"nForce 980a/780a SLI" },
	{ 0x10DE084D,	/*0x00000000,*/	"nForce 750a SLI" },
	{ 0x10DE084F,	/*0x00000000,*/	"GeForce 8100 / nForce 720a" },
	// 0850 - 085F
    //	{ 0x10DE0850,	/*0x00000000,*/	"NVIDIA MCP77/78" },
    //	{ 0x10DE0851,	/*0x00000000,*/	"NVIDIA MCP77/78" },
    //	{ 0x10DE0852,	/*0x00000000,*/	"NVIDIA MCP77/78" },
    //	{ 0x10DE0853,	/*0x00000000,*/	"NVIDIA MCP77/78" },
    //	{ 0x10DE0854,	/*0x00000000,*/	"NVIDIA MCP77/78" },
    //	{ 0x10DE0855,	/*0x00000000,*/	"NVIDIA MCP77/78" },
    //	{ 0x10DE0856,	/*0x00000000,*/	"NVIDIA MCP77/78" },
    //	{ 0x10DE0857,	/*0x00000000,*/	"NVIDIA MCP77/78" },
    //	{ 0x10DE0858,	/*0x00000000,*/	"NVIDIA MCP77/78" },
    //	{ 0x10DE0859,	/*0x00000000,*/	"NVIDIA MCP77/78" },
    //	{ 0x10DE085A,	/*0x00000000,*/	"NVIDIA MCP77/78" },
    //	{ 0x10DE085B,	/*0x00000000,*/	"NVIDIA MCP77/78" },
    //	{ 0x10DE085C,	/*0x00000000,*/	"NVIDIA MCP77/78" },
    //	{ 0x10DE085D,	/*0x00000000,*/	"NVIDIA MCP77/78" },
    //	{ 0x10DE085E,	/*0x00000000,*/	"NVIDIA MCP77/78" },
    //	{ 0x10DE085F,	/*0x00000000,*/	"NVIDIA MCP77/78" },
	// 0860 - 086F
	{ 0x10DE0860,	/*0x00000000,*/	"GeForce 9300" }, //
	{ 0x10DE0861,	/*0x00000000,*/	"GeForce 9400" },
	{ 0x10DE0862,	/*0x00000000,*/	"GeForce 9400M G" },
	{ 0x10DE0863,	/*0x00000000,*/	"GeForce 9400M" },
	{ 0x10DE0864,	/*0x00000000,*/	"GeForce 9300" },
	{ 0x10DE0865,	/*0x00000000,*/	"GeForce 9300" }, //
	{ 0x10DE0866,	/*0x00000000,*/	"GeForce 9400M G" },
	{ 0x10DE0867,	/*0x00000000,*/	"GeForce 9400" },
	{ 0x10DE0868,	/*0x00000000,*/	"nForce 760i SLI" },
	{ 0x10DE0869,	/*0x00000000,*/	"GeForce 9400" },
	{ 0x10DE086A,	/*0x00000000,*/	"GeForce 9400" },
	{ 0x10DE086C,	/*0x00000000,*/	"GeForce 9300 / nForce 730i" },
	{ 0x10DE086D,	/*0x00000000,*/	"GeForce 9200" },
	{ 0x10DE086E,	/*0x00000000,*/	"GeForce 9100M G" },
	{ 0x10DE086F,	/*0x00000000,*/	"GeForce 8200M G" },
	// 0870 - 087F
	{ 0x10DE0870,	/*0x00000000,*/	"GeForce 9400M" },
	{ 0x10DE0871,	/*0x00000000,*/	"GeForce 9200" },
	{ 0x10DE0872,	/*0x00000000,*/	"GeForce G102M" },
	{ 0x10DE0873,	/*0x00000000,*/	"GeForce G102M" },
	{ 0x10DE0874,	/*0x00000000,*/	"ION 9300M" },	
	{ 0x10DE0876,	/*0x00000000,*/	"ION 9400M" }, //
	{ 0x10DE087A,	/*0x00000000,*/	"GeForce 9400" },
	{ 0x10DE087D,	/*0x00000000,*/	"ION 9400M" },
	{ 0x10DE087E,	/*0x00000000,*/	"ION LE" },
	{ 0x10DE087F,	/*0x00000000,*/	"ION LE" },
	// 0880 - 088F
	// 0890 - 089F
	// 08A0 - 08AF
	{ 0x10DE08A0,	/*0x00000000,*/	"GeForce 320M" },
    //	{ 0x10DE08A1,	/*0x00000000,*/	"NVIDIA MCP89-MZT" },
    //	{ 0x10DE08A2,	/*0x00000000,*/	"NVIDIA MCP89-EPT" },
	{ 0x10DE08A3,	/*0x00000000,*/	"GeForce 320M" },
	{ 0x10DE08A4,	/*0x00000000,*/	"GeForce 320M" },
	{ 0x10DE08A5,	/*0x00000000,*/	"GeForce 320M" },
	// 08B0 - 08BF
    //	{ 0x10DE08B0,	/*0x00000000,*/	"Unknown" },
	{ 0x10DE08B1,	/*0x00000000,*/	"GeForce 300M" },
    //	{ 0x10DE08B2,	/*0x00000000,*/	"NVIDIA MCP83-MJ" },
    //	{ 0x10DE08B3,	/*0x00000000,*/	"NVIDIA MCP89 MM9" },
	// 08C0 - 08CF
	// 08D0 - 08DF
	// 08E0 - 08EF
	// 08F0 - 08FF
	// 0900 - 090F
	// 0910 - 091F
	// 0920 - 092F
	// 0930 - 093F
	// 0940 - 094F
	// 0950 - 095F
	// 0960 - 096F
	// 0970 - 097F
	// 0980 - 098F
	// 0990 - 099F
	// 09A0 - 09AF
	// 09B0 - 09BF
	// 09C0 - 09CF
	// 09D0 - 09DF
	// 09E0 - 09EF
	// 09F0 - 09FF
	// 0A00 - 0A0F
    //	{ 0x10DE0A00,	/*0x00000000,*/	"NVIDIA GT212" },
	// 0A10 - 0A1F
    //	{ 0x10DE0A10,	/*0x00000000,*/	"NVIDIA GT212" },
	// 0A20 - 0A2F
	{ 0x10DE0A20,	/*0x00000000,*/	"GeForce GT 220" }, // subsystem 10de:0a20:1043:8311
    //	{ 0x10DE0A21,	/*0x00000000,*/	"NVIDIA D10M2-20" },
	{ 0x10DE0A22,	/*0x00000000,*/	"GeForce 315" },
	{ 0x10DE0A23,	/*0x00000000,*/	"GeForce 210" },
	{ 0x10DE0A26,	/*0x00000000,*/	"GeForce 405" },
	{ 0x10DE0A27,	/*0x00000000,*/	"GeForce 405" },
	{ 0x10DE0A28,	/*0x00000000,*/	"GeForce GT 230M" },
	{ 0x10DE0A29,	/*0x00000000,*/	"GeForce GT 330M" },
	{ 0x10DE0A2A,	/*0x00000000,*/	"GeForce GT 230M" },
	{ 0x10DE0A2B,	/*0x00000000,*/	"GeForce GT 330M" },
	{ 0x10DE0A2C,	/*0x00000000,*/	"NVS 5100M" },
	{ 0x10DE0A2D,	/*0x00000000,*/	"GeForce GT 320M" },	
	// 0A30 - 0A3F
    //	{ 0x10DE0A30,	/*0x00000000,*/	"NVIDIA GT216" },
	{ 0x10DE0A34,	/*0x00000000,*/	"GeForce GT 240M" },
	{ 0x10DE0A35,	/*0x00000000,*/	"GeForce GT 325M" },
	{ 0x10DE0A38,	/*0x00000000,*/	"Quadro 400" },
	{ 0x10DE0A3C,	/*0x00000000,*/	"Quadro FX 880M" },
    //	{ 0x10DE0A3D,	/*0x00000000,*/	"NVIDIA N10P-ES" },
    //	{ 0x10DE0A3F,	/*0x00000000,*/	"NVIDIA GT216-INT" },
	// 0A40 - 0A4F
	// 0A50 - 0A5F
	// 0A60 - 0A6F
	{ 0x10DE0A60,	/*0x00000000,*/	"GeForce G210" },
    //	{ 0x10DE0A61,	/*0x00000000,*/	"NVIDIA NVS 2100" },
	{ 0x10DE0A62,	/*0x00000000,*/	"GeForce 205" },
	{ 0x10DE0A63,	/*0x00000000,*/	"GeForce 310" },
	{ 0x10DE0A64,	/*0x00000000,*/	"ION" },
	{ 0x10DE0A65,	/*0x00000000,*/	"GeForce 210" }, // subsystem 10de:0a65:1043:8334
	{ 0x10DE0A66,	/*0x00000000,*/	"GeForce 310" },
	{ 0x10DE0A67,	/*0x00000000,*/	"GeForce 315" },
	{ 0x10DE0A68,	/*0x00000000,*/	"GeForce G105M" },
	{ 0x10DE0A69,	/*0x00000000,*/	"GeForce G105M" },
	{ 0x10DE0A6A,	/*0x00000000,*/	"NVS 2100M" },
	{ 0x10DE0A6C,	/*0x00000000,*/	"NVS 3100M" }, // subsystem 10de:0a6c:1028:040b & 10de:0a6c:17aa:2142
	{ 0x10DE0A6E,	/*0x00000000,*/	"GeForce 305M" },
	{ 0x10DE0A6F,	/*0x00000000,*/	"ION" },	
	// 0A70 - 0A7F
	{ 0x10DE0A70,	/*0x00000000,*/	"GeForce 310M" },
	{ 0x10DE0A71,	/*0x00000000,*/	"GeForce 305M" },
	{ 0x10DE0A72,	/*0x00000000,*/	"GeForce 310M" },
	{ 0x10DE0A73,	/*0x00000000,*/	"GeForce 305M" },
	{ 0x10DE0A74,	/*0x00000000,*/	"GeForce G210M" },
	{ 0x10DE0A75,	/*0x00000000,*/	"GeForce G310M" },
	{ 0x10DE0A76,	/*0x00000000,*/	"ION" },
	{ 0x10DE0A78,	/*0x00000000,*/	"Quadro FX 380 LP" },
    //	{ 0x10DE0A79,	/*0x00000000,*/	"Unknown" },
	{ 0x10DE0A7A,	/*0x00000000,*/	"GeForce 315M" },
	{ 0x10DE0A7C,	/*0x00000000,*/	"Quadro FX 380M" },
    //	{ 0x10DE0A7D,	/*0x00000000,*/	"NVIDIA GT218-ES" },
    //	{ 0x10DE0A7E,	/*0x00000000,*/	"NVIDIA GT218-INT-S" },
    //	{ 0x10DE0A7F,	/*0x00000000,*/	"NVIDIA GT218-INT-B" },
	// 0A80 - 0A8F
	// 0A90 - 0A9F
	// 0AA0 - 0AAF
	// 0AB0 - 0ABF
	// 0AC0 - 0ACF
	// 0AD0 - 0ADF
	// 0AE0 - 0AEF
	// 0AF0 - 0AFF
	// 0B00 - 0B0F
	// 0B10 - 0B1F
	// 0B20 - 0B2F
	// 0B30 - 0B3F
	// 0B40 - 0B4F
	// 0B50 - 0B5F
	// 0B60 - 0B6F
	// 0B70 - 0B7F
	// 0B80 - 0B8F
	// 0B90 - 0B9F
	// 0BA0 - 0BAF
	// 0BB0 - 0BBF
	// 0BC0 - 0BCF
	// 0BD0 - 0BDF
	// 0BE0 - 0BEF
    //	{ 0x10DE0BE4,	/*0x00000000,*/	"nVidia High Definition Audio Controller" },
    //	{ 0x10DE0BE9,	/*0x00000000,*/	"nVidia High Definition Audio Controller" },
	// 0BF0 - 0BFF
	// 0C00 - 0C0F
	// 0C10 - 0C1F
	// 0C20 - 0C2F
	// 0C30 - 0C3F
	// 0C40 - 0C4F
	// 0C50 - 0C5F
	// 0C60 - 0C6F
	// 0C70 - 0C7F
	// 0C80 - 0C8F
	// 0C90 - 0C9F
	// 0CA0 - 0CAF
	{ 0x10DE0CA0,	/*0x00000000,*/	"GeForce GT 330 " },
	{ 0x10DE0CA2,	/*0x00000000,*/	"GeForce GT 320" },
	{ 0x10DE0CA3,	/*0x00000000,*/	"GeForce GT 240" },
	{ 0x10DE0CA4,	/*0x00000000,*/	"GeForce GT 340" },
	{ 0x10DE0CA5,	/*0x00000000,*/	"GeForce GT 220" },
	{ 0x10DE0CA7,	/*0x00000000,*/	"GeForce GT 330" },
	{ 0x10DE0CA8,	/*0x00000000,*/	"GeForce GTS 260M" },
	{ 0x10DE0CA9,	/*0x00000000,*/	"GeForce GTS 250M" },
	{ 0x10DE0CAC,	/*0x00000000,*/	"GeForce GT 220" },
    //	{ 0x10DE0CAD,	/*0x00000000,*/	"NVIDIA N10E-ES" },
    //	{ 0x10DE0CAE,	/*0x00000000,*/	"NVIDIA GT215-INT" },
	{ 0x10DE0CAF,	/*0x00000000,*/	"GeForce GT 335M" },
	// 0CB0 - 0CBF	
	{ 0x10DE0CB0,	/*0x00000000,*/	"GeForce GTS 350M" },
	{ 0x10DE0CB1,	/*0x00000000,*/	"GeForce GTS 360M" },
	{ 0x10DE0CBC,	/*0x00000000,*/	"Quadro FX 1800M" },
	// 0CC0 - 0CCF
	// 0CD0 - 0CDF
	// 0CE0 - 0CEF
	// 0CF0 - 0CFF
	// 0D00 - 0D0F
	// 0D10 - 0D1F
	// 0D20 - 0D2F
	// 0D30 - 0D3F
	// 0D40 - 0D4F
	// 0D50 - 0D5F
	// 0D60 - 0D6F
	// 0D70 - 0D7F
	// 0D80 - 0D8F
	// 0D90 - 0D9F
	// 0DA0 - 0DAF
	// 0DB0 - 0DBF
	// 0DC0 - 0DCF
	{ 0x10DE0DC0,	/*0x00000000,*/	"GeForce GT 440" },
	{ 0x10DE0DC1,	/*0x00000000,*/	"D12-P1-35" },
	{ 0x10DE0DC2,	/*0x00000000,*/	"D12-P1-35" },
	{ 0x10DE0DC4,	/*0x00000000,*/	"GeForce GTS 450" },
	{ 0x10DE0DC5,	/*0x00000000,*/	"GeForce GTS 450" },
	{ 0x10DE0DC6,	/*0x00000000,*/	"GeForce GTS 450" },
	{ 0x10DE0DCA,	/*0x00000000,*/	"GF10x" },
    //	{ 0x10DE0DCC,	/*0x00000000,*/	"Unknown" },
	{ 0x10DE0DCD,	/*0x00000000,*/	"GeForce GT 555M" },
	{ 0x10DE0DCE,	/*0x00000000,*/	"GeForce GT 555M" },
    //	{ 0x10DE0DCF,	/*0x00000000,*/	"Unknown" },
	// 0DD0 - 0DDF	
    //	{ 0x10DE0DD0,	/*0x00000000,*/	"Unknown" },
	{ 0x10DE0DD1,	/*0x00000000,*/	"GeForce GTX 460M" }, // subsystem 10de:0dd1:1558:8687
	{ 0x10DE0DD2,	/*0x00000000,*/	"GeForce GT 445M" },
	{ 0x10DE0DD3,	/*0x00000000,*/	"GeForce GT 435M" },
	{ 0x10DE0DD6,	/*0x00000000,*/	"GeForce GT 550M" },
	{ 0x10DE0DD8,	/*0x00000000,*/	"Quadro 2000" },
	{ 0x10DE0DDA,	/*0x00000000,*/	"Quadro 2000M" },
	{ 0x10DE0DDE,	/*0x00000000,*/	"GF106-ES" },
	{ 0x10DE0DDF,	/*0x00000000,*/	"GF106-INT" },
	// 0DE0 - 0DEF
	{ 0x10DE0DE0,	/*0x00000000,*/	"GeForce GT 440" },
	{ 0x10DE0DE1,	/*0x00000000,*/	"GeForce GT 430" }, // subsystem 10de:0de1:3842:1430
	{ 0x10DE0DE2,	/*0x00000000,*/	"GeForce GT 420" },
	{ 0x10DE0DE4,	/*0x00000000,*/	"GeForce GT 520" },
	{ 0x10DE0DE5,	/*0x00000000,*/	"GeForce GT 530" },
	{ 0x10DE0DE8,	/*0x00000000,*/	"GeForce GT 620M" },
	{ 0x10DE0DE9,	/*0x00000000,*/	"GeForce GT 630M" },
	{ 0x10DE0DEA,	/*0x00000000,*/	"GeForce GT 610M" },
	{ 0x10DE0DEB,	/*0x00000000,*/	"GeForce GT 555M" },
	{ 0x10DE0DEC,	/*0x00000000,*/	"GeForce GT 525M" },
	{ 0x10DE0DED,	/*0x00000000,*/	"GeForce GT 520M" },
	{ 0x10DE0DEE,	/*0x00000000,*/	"GeForce GT 415M" },
    //	{ 0x10DE0DEF,	/*0x00000000,*/	"Unknown" },
	// 0DF0 - 0DFF	
	{ 0x10DE0DF0,	/*0x00000000,*/	"GeForce GT 425M" },
	{ 0x10DE0DF1,	/*0x00000000,*/	"GeForce GT 420M" },
	{ 0x10DE0DF2,	/*0x00000000,*/	"GeForce GT 435M" },
	{ 0x10DE0DF3,	/*0x00000000,*/	"GeForce GT 420M" },
	{ 0x10DE0DF4,	/*0x00000000,*/	"GeForce GT 540M" },
	{ 0x10DE0DF5,	/*0x00000000,*/	"GeForce GT 525M" },
	{ 0x10DE0DF6,	/*0x00000000,*/	"GeForce GT 550M" },
	{ 0x10DE0DF7,	/*0x00000000,*/	"GeForce GT 520M" },
	{ 0x10DE0DF8,	/*0x00000000,*/	"Quadro 600" },
    //	{ 0x10DE0DF9,	/*0x00000000,*/	"Unknown" },
	{ 0x10DE0DFA,	/*0x00000000,*/	"Quadro 1000M" },
	{ 0x10DE0DFC,	/*0x00000000,*/	"NVS 5200M" },
	{ 0x10DE0DFE,	/*0x00000000,*/	"GF108 ES" },
	{ 0x10DE0DFF,	/*0x00000000,*/	"GF108 INT" },
	// 0E00 - 0E0F
	// 0E10 - 0E1F
	// 0E20 - 0E2F
	{ 0x10DE0E21,	/*0x00000000,*/	"D12U-25" },
	{ 0x10DE0E22,	/*0x00000000,*/	"GeForce GTX 460" }, // subsystem 10de:0e22:1462:2322
	{ 0x10DE0E23,	/*0x00000000,*/	"GeForce GTX 460 SE" },
	{ 0x10DE0E24,	/*0x00000000,*/	"GeForce GTX 460" },
	{ 0x10DE0E25,	/*0x00000000,*/	"D12U-50" },
	// 0E30 - 0E3F
	{ 0x10DE0E30,	/*0x00000000,*/	"GeForce GTX 470M" },
	{ 0x10DE0E31,	/*0x00000000,*/	"GeForce GTX 485M" },
    //	{ 0x10DE0E32,	/*0x00000000,*/	"Unknown" },
	{ 0x10DE0E38,	/*0x00000000,*/	"GF104GL" },
	{ 0x10DE0E3A,	/*0x00000000,*/	"Quadro 3000M" },
	{ 0x10DE0E3B,	/*0x00000000,*/	"Quadro 4000M" },
	{ 0x10DE0E3E,	/*0x00000000,*/	"GF104-ES" },
	{ 0x10DE0E3F,	/*0x00000000,*/	"GF104-INT" },
	// 0E40 - 0E4F
	// 0E50 - 0E5F
	// 0E60 - 0E6F
	// 0E70 - 0E7F
	// 0E80 - 0E8F
	// 0E90 - 0E9F
	// 0EA0 - 0EAF
	// 0EB0 - 0EBF
	// 0EC0 - 0ECF
	// 0ED0 - 0EDF
	// 0EE0 - 0EEF
	// 0EF0 - 0EFF
	// 0F00 - 0F0F
	// 0F10 - 0F1F
	// 0F20 - 0F2F
	// 0F30 - 0F3F
	// 0F40 - 0F4F
	// 0F50 - 0F5F
	// 0F60 - 0F6F
	// 0F70 - 0F7F
	// 0F80 - 0F8F
	// 0F90 - 0F9F
	// 0FA0 - 0FAF
	// 0FB0 - 0FBF
	// 0FC0 - 0FCF
	// 0FD0 - 0FDF
	{ 0x10DE0FD1,	/*0x00000000,*/	"GeForce GT 650M" },
	{ 0x10DE0FD2,	/*0x00000000,*/	"GeForce GT 640M" },
	{ 0x10DE0FD4,	/*0x00000000,*/	"GeForce GTX 660M" },
	// 0FE0 - 0FEF
	// 0FF0 - 0FFF
	// 1000 - 100F
	// 1010 - 101F
	// 1020 - 102F
	// 1030 - 103F
	// 1040 - 104F
	{ 0x10DE1040,	/*0x00000000,*/	"GeForce GT 520" },
	{ 0x10DE1042,	/*0x00000000,*/	"GeForce 510" },
	{ 0x10DE1049,	/*0x00000000,*/	"GeForce GT 620" },
	// 1050 - 105F
	{ 0x10DE1050,	/*0x00000000,*/	"GeForce GT 520M" },
	{ 0x10DE1051,	/*0x00000000,*/	"GeForce GT 520MX" },
    //	{ 0x10DE1052,	/*0x00000000,*/	"Unknown" },
	{ 0x10DE1054,	/*0x00000000,*/	"GeForce GT 410M" },
	{ 0x10DE1055,	/*0x00000000,*/	"GeForce 410M" },
	{ 0x10DE1056,	/*0x00000000,*/	"Quadro NVS 4200M" },
	{ 0x10DE1057,	/*0x00000000,*/	"Quadro NVS 4200M" },
	{ 0x10DE1058,	/*0x00000000,*/	"GeForce 610M" },
    //	{ 0x10DE1059,	/*0x00000000,*/	"AUDIO" },
	{ 0x10DE105A,	/*0x00000000,*/	"GeForce 610M" },
	// 1060 - 106F
	// 1070 - 107F
    //	{ 0x10DE107D,	/*0x00000000,*/	"Unknown" },
    //	{ 0x10DE107E,	/*0x00000000,*/	"Unknown" },
	{ 0x10DE107F,	/*0x00000000,*/	"NVIDIA GF119-ES" },
	// 1080 - 108F
	{ 0x10DE1080,	/*0x00000000,*/	"GeForce GTX 580" },
	{ 0x10DE1081,	/*0x00000000,*/	"GeForce GTX 570" }, // subsystem 10de:1081:10de:087e
	{ 0x10DE1082,	/*0x00000000,*/	"GeForce GTX 560 Ti" },
	{ 0x10DE1083,	/*0x00000000,*/	"D13U" },
	{ 0x10DE1084,	/*0x00000000,*/	"GeForce GTX 560" },
	{ 0x10DE1086,	/*0x00000000,*/	"GeForce GTX 570" },
	{ 0x10DE1087,	/*0x00000000,*/	"GeForce GTX 560 Ti-448" },
	{ 0x10DE1088,	/*0x00000000,*/	"GeForce GTX 590" },
	{ 0x10DE1089,	/*0x00000000,*/	"GeForce GTX 580" },
	{ 0x10DE108B,	/*0x00000000,*/	"GeForce GTX 590" },
	// 1090 - 109F
	{ 0x10DE1091,	/*0x00000000,*/	"Tesla M2090" },
	{ 0x10DE1094,	/*0x00000000,*/	"Tesla M2075 Dual-Slot Computing Processor Module" },
	{ 0x10DE1096,	/*0x00000000,*/	"Tesla C2075" },
	{ 0x10DE1098,	/*0x00000000,*/	"D13U" },
	{ 0x10DE109A,	/*0x00000000,*/	"Quadro 5010M" },
	{ 0x10DE109B,	/*0x00000000,*/	"Quadro 7000" },
	// 10A0 - 10AF
	// 10B0 - 10BF
	// 10C0 - 10CF
	{ 0x10DE10C0,	/*0x00000000,*/	"GeForce 9300 GS" },
	{ 0x10DE10C3,	/*0x00000000,*/	"GeForce 8400 GS" },
    //	{ 0x10DE10C4,	/*0x00000000,*/	"NVIDIA ION" },
	{ 0x10DE10C5,	/*0x00000000,*/	"GeForce 405" },
	// 10D0 - 10DF
	{ 0x10DE10D8,	/*0x00000000,*/	"NVS 300" },
	// 10E0 - 10EF
	// 10F0 - 10FF
	// 1100 - 110F
	// 1110 - 111F
	// 1120 - 112F
	// 1130 - 113F
	// 1140 - 114F
	// 1150 - 115F
	// 1160 - 116F
	// 1170 - 117F
	// 1180 - 118F
	{ 0x10DE1180,	/*0x00000000,*/	"GeForce GTX 680" },
	// 1190 - 119F
	// 11A0 - 11AF
	// 11B0 - 11BF
	// 11C0 - 11CF
	// 11D0 - 11DF
	// 11E0 - 11EF
	// 11F0 - 11FF
	// 1200 - 120F
	{ 0x10DE1200,	/*0x00000000,*/	"GeForce GTX 560 Ti" },
	{ 0x10DE1201,	/*0x00000000,*/	"GeForce GTX 560" },
	{ 0x10DE1203,	/*0x00000000,*/	"GeForce GTX 460 SE v2" },
	{ 0x10DE1205,	/*0x00000000,*/	"GeForce GTX 460 v2" },
	{ 0x10DE1208,	/*0x00000000,*/	"GeForce GTX 560 SE" },
	{ 0x10DE1210,	/*0x00000000,*/	"GeForce GTX 570M" },
	{ 0x10DE1211,	/*0x00000000,*/	"GeForce GTX 580M" },
	{ 0x10DE1212,	/*0x00000000,*/	"GeForce GTX 675M" },
	{ 0x10DE1213,	/*0x00000000,*/	"GeForce GTX 670M" },
	{ 0x10DE1240,	/*0x00000000,*/	"GeForce GT 620M" },
	{ 0x10DE1241,	/*0x00000000,*/	"GeForce GT 545" },
	{ 0x10DE1243,	/*0x00000000,*/	"GeForce GT 545" },
	{ 0x10DE1244,	/*0x00000000,*/	"GeForce GTX 550 Ti" },
	{ 0x10DE1245,	/*0x00000000,*/	"GeForce GTS 450" },
	{ 0x10DE1246,	/*0x00000000,*/	"GeForce GTX 550M" },
	{ 0x10DE1247,	/*0x00000000,*/	"GeForce GT 635M" }, // Subsystem Id: 1043 212C Asus GeForce GT 635M
	{ 0x10DE1248,	/*0x00000000,*/	"GeForce GTX 555M" },
	{ 0x10DE124D,	/*0x00000000,*/	"GeForce GTX 555M" },
    //	{ 0x10DE1250,	/*0x00000000,*/	"Unknown" },
	{ 0x10DE1251,	/*0x00000000,*/	"GeForce GTX 560M" },
	// 1260 - 126F
	// 1270 - 127F
	// 1280 - 128F
	// 1290 - 129F
	// 12A0 - 12AF
	// 12B0 - 12BF
	// 12C0 - 12CF
	// 12D0 - 12DF
	// 12E0 - 12EF
	// 12F0 - 12FF
    //	{ 0x10DE8001,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE8067,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
    //	{ 0x10DE8073,	/*0x00000000,*/	"NVIDIA HDMI Audio" },
};

static uint16_t swap16(uint16_t x)
{
	return (((x & 0x00FF) << 8) | ((x & 0xFF00) >> 8));
}

static uint16_t read16(uint8_t *ptr, uint16_t offset)
{
	uint8_t ret[2];

	ret[0] = ptr[offset+1];
	ret[1] = ptr[offset];

	return *((uint16_t*)&ret);
}

#if 0
static uint32_t swap32(uint32_t x)
{
	return ((x & 0x000000FF) << 24) | ((x & 0x0000FF00) << 8 ) | ((x & 0x00FF0000) >> 8 ) | ((x & 0xFF000000) >> 24);
}

static uint8_t	read8(uint8_t *ptr, uint16_t offset)
{ 
	return ptr[offset];
}

static uint32_t read32(uint8_t *ptr, uint16_t offset)
{
	uint8_t ret[4];

	ret[0] = ptr[offset+3];
	ret[1] = ptr[offset+2];
	ret[2] = ptr[offset+1];
	ret[3] = ptr[offset];

	return *((uint32_t*)&ret);
}
#endif

static int patch_nvidia_rom(uint8_t *rom)
{
	if (!rom || (rom[0] != 0x55 && rom[1] != 0xaa)) {
		printf("False ROM signature: 0x%02x%02x\n", rom[0], rom[1]);
		return PATCH_ROM_FAILED;
	}
	
	uint16_t dcbptr = swap16(read16(rom, 0x36));

	if (!dcbptr) {
		printf("no dcb table found\n");
		return PATCH_ROM_FAILED;
	}
//	else
//		printf("dcb table at offset 0x%04x\n", dcbptr);
	 
	uint8_t *dcbtable		 = &rom[dcbptr];
	uint8_t dcbtable_version = dcbtable[0];
	uint8_t headerlength	 = 0;
	uint8_t numentries		 = 0;
	uint8_t recordlength	 = 0;
	
	if (dcbtable_version >= 0x20)
	{
		uint32_t sig;
		
		if (dcbtable_version >= 0x30)
		{
			headerlength = dcbtable[1];
			numentries	 = dcbtable[2];
			recordlength = dcbtable[3];

			sig = *(uint32_t *)&dcbtable[6];
		}
		else
		{
			sig = *(uint32_t *)&dcbtable[4];
			headerlength = 8;
		}
		
		if (sig != 0x4edcbdcb)
		{
			printf("Bad display config block signature (0x%8x)\n", sig); //Azi: issue #48
			return PATCH_ROM_FAILED;
		}
	}
	else if (dcbtable_version >= 0x14) /* some NV15/16, and NV11+ */
	{
		char sig[8] = { 0 };
		
		strncpy(sig, (char *)&dcbtable[-7], 7);
		recordlength = 10;
		
		if (strcmp(sig, "DEV_REC"))
		{
			printf("Bad Display Configuration Block signature (%s)\n", sig);
			return PATCH_ROM_FAILED;
		}
	}
	else
	{
		printf("ERROR: dcbtable_version is 0x%X\n", dcbtable_version);
		return PATCH_ROM_FAILED;
	}
	
	if (numentries >= MAX_NUM_DCB_ENTRIES)
		numentries = MAX_NUM_DCB_ENTRIES;
	
	uint8_t num_outputs = 0, i = 0;
	
	struct dcbentry
	{
		uint8_t type;
		uint8_t index;
		uint8_t *heads;
	} entries[numentries];
	
	for (i = 0; i < numentries; i++)
	{
		uint32_t connection;
		connection = *(uint32_t *)&dcbtable[headerlength + recordlength * i];

		/* Should we allow discontinuous DCBs? Certainly DCB I2C tables can be discontinuous */
		if ((connection & 0x0000000f) == 0x0000000f) /* end of records */ 
			continue;
		if (connection == 0x00000000) /* seen on an NV11 with DCB v1.5 */ 
			continue;
		if ((connection & 0xf) == 0x6) /* we skip type 6 as it doesnt appear on macbook nvcaps */
			continue;
		
		entries[num_outputs].type = connection & 0xf;
		entries[num_outputs].index = num_outputs;
		entries[num_outputs++].heads = (uint8_t*)&(dcbtable[(headerlength + recordlength * i) + 1]);
	}
	
	int has_lvds = false;
	uint8_t channel1 = 0, channel2 = 0;
	
	for (i = 0; i < num_outputs; i++)
	{
		if (entries[i].type == 3)
		{
			has_lvds = true;
			//printf("found LVDS\n");
			channel1 |= ( 0x1 << entries[i].index);
			entries[i].type = TYPE_GROUPED;
		}
	}
	
	// if we have a LVDS output, we group the rest to the second channel
	if (has_lvds)
	{
		for (i = 0; i < num_outputs; i++)
		{
			if (entries[i].type == TYPE_GROUPED)
				continue;
			
			channel2 |= ( 0x1 << entries[i].index);
			entries[i].type = TYPE_GROUPED;
		}
	}
	else
	{
		int x;
		// we loop twice as we need to generate two channels
		for (x = 0; x <= 1; x++)
		{
			for (i=0; i<num_outputs; i++)
			{
				if (entries[i].type == TYPE_GROUPED)
					continue;
				// if type is TMDS, the prior output is ANALOG
				// we always group ANALOG and TMDS
				// if there is a TV output after TMDS, we group it to that channel as well
				if (i && entries[i].type == 0x2)
				{
					switch (x)
					{
						case 0:
							//printf("group channel 1\n");
							channel1 |= ( 0x1 << entries[i].index);
							entries[i].type = TYPE_GROUPED;
							
							if ((entries[i-1].type == 0x0))
							{
								channel1 |= ( 0x1 << entries[i-1].index);
								entries[i-1].type = TYPE_GROUPED;
							}
							// group TV as well if there is one
							if ( ((i+1) < num_outputs) && (entries[i+1].type == 0x1) )
							{
								//	printf("group tv1\n");
								channel1 |= ( 0x1 << entries[i+1].index);
								entries[i+1].type = TYPE_GROUPED;
							}
							break;
						
						case 1:
							//printf("group channel 2 : %d\n", i);
							channel2 |= ( 0x1 << entries[i].index);
							entries[i].type = TYPE_GROUPED;
							
							if ((entries[i - 1].type == 0x0))
							{
								channel2 |= ( 0x1 << entries[i-1].index);
								entries[i-1].type = TYPE_GROUPED;
							}
							// group TV as well if there is one
							if ( ((i+1) < num_outputs) && (entries[i+1].type == 0x1) )
							{
								//	printf("group tv2\n");
								channel2 |= ( 0x1 << entries[i+1].index);
								entries[i+1].type = TYPE_GROUPED;
							}
							break;
					}
					break;
				}
			}
		}
	}
	
	// if we have left ungrouped outputs merge them to the empty channel
	uint8_t *togroup;// = (channel1 ? (channel2 ? NULL : &channel2) : &channel1);
	togroup = &channel2;
	
	for (i = 0; i < num_outputs; i++)
	{
		if (entries[i].type != TYPE_GROUPED)
		{
			//printf("%d not grouped\n", i);
			if (togroup)
			{
				*togroup |= ( 0x1 << entries[i].index);
			}
			entries[i].type = TYPE_GROUPED;
		}
	}
	
	if (channel1 > channel2)
	{
		uint8_t buff = channel1;
		channel1 = channel2;
		channel2 = buff;
	}
	
	default_NVCAP[6] = channel1;
	default_NVCAP[8] = channel2;
	
	// patching HEADS
	for (i = 0; i < num_outputs; i++)
	{
		if (channel1 & (1 << i))
		{
			*entries[i].heads = 1;
		}
		else if(channel2 & (1 << i))
		{
			*entries[i].heads = 2;
		}
	}
	return (has_lvds ? PATCH_ROM_SUCCESS_HAS_LVDS : PATCH_ROM_SUCCESS);
}

static char *get_nvidia_model(uint32_t id)
{
	int i;
	
	for (i = 1; i < (sizeof(NVKnownChipsets) / sizeof(NVKnownChipsets[0])); i++) {
		if (NVKnownChipsets[i].device == id)
		{
			return NVKnownChipsets[i].name;
		}
	}
	return NVKnownChipsets[0].name;
}

static uint32_t load_nvidia_bios_file(const char *filename, uint8_t *buf, int bufsize)
{
	int fd;
	int size;
	
	if ((fd = open_bvdev("bt(0,0)", filename, 0)) < 0)
	{
		return 0;
	}
	
	size = file_size(fd);
	
	if (size > bufsize)
	{
		printf("Filesize of %s is bigger than expected! Truncating to 0x%x Bytes!\n",
				filename, bufsize);
		size = bufsize;
	}
	size = read(fd, (char *)buf, size);
	close(fd);
	
	return size > 0 ? size : 0;
}

static int devprop_add_nvidia_template(struct DevPropDevice *device)
{
	char tmp[16];
	
	if (!device)
		return 0;
	
	if (!DP_ADD_TEMP_VAL(device, nvidia_compatible_0))
		return 0;
	if (!DP_ADD_TEMP_VAL(device, nvidia_device_type_0))
		return 0;
	if (!DP_ADD_TEMP_VAL(device, nvidia_name_0))
		return 0;
	if (!DP_ADD_TEMP_VAL(device, nvidia_compatible_1))
		return 0;
	if (!DP_ADD_TEMP_VAL(device, nvidia_device_type_1))
		return 0;
	if (!DP_ADD_TEMP_VAL(device, nvidia_name_1))
		return 0;
	if (devices_number == 1)
	{
	  if (!DP_ADD_TEMP_VAL(device, nvidia_device_type))
		  return 0;
	}
	else
	{
	  if (!DP_ADD_TEMP_VAL(device, nvidia_device_type_child))
		  return 0;
	}
	
	// Rek : Dont use sprintf return, it does not WORK !! our custom sprintf() always return 0!
	// len = sprintf(tmp, "Slot-%x", devices_number);
	sprintf(tmp, "Slot-%x",devices_number);
	devprop_add_value(device, "AAPL,slot-name", (uint8_t *) tmp, strlen(tmp));
	devices_number++;
	
	return 1;
}

int hex2bin(const char *hex, uint8_t *bin, int len)
{
	char	*p;
	int		i;
	char	buf[3];
	
	if (hex == NULL || bin == NULL || len <= 0 || strlen(hex) != len * 2) {
		printf("[ERROR] bin2hex input error\n");
		return -1;
	}
	
	buf[2] = '\0';
	p = (char *) hex;
	
	for (i = 0; i < len; i++)
	{
		if (p[0] == '\0' || p[1] == '\0' || !isxdigit(p[0]) || !isxdigit(p[1])) {
			printf("[ERROR] bin2hex '%s' syntax error\n", hex);
			return -2;
		}
		buf[0] = *p++;
		buf[1] = *p++;
		bin[i] = (unsigned char) strtoul(buf, NULL, 16);
	}
	return 0;
}

unsigned long long mem_detect(volatile uint8_t *regs, uint8_t nvCardType, pci_dt_t *nvda_dev)
{
	unsigned long long vram_size = 0;
	
	if (nvCardType < NV_ARCH_50)
	{
		vram_size  = REG32(NV04_PFB_FIFO_DATA);
		vram_size &= NV10_PFB_FIFO_DATA_RAM_AMOUNT_MB_MASK;
	}
	else if (nvCardType < NV_ARCH_C0)
	{
		vram_size = REG32(NV04_PFB_FIFO_DATA);
		vram_size |= (vram_size & 0xff) << 32;
		vram_size &= 0xffffffff00ll;
	}
	else // >= NV_ARCH_C0
	{
		vram_size = REG32(NVC0_MEM_CTRLR_RAM_AMOUNT) << 20;
		vram_size *= REG32(NVC0_MEM_CTRLR_COUNT);
	}
	
	// Workaround for 9600M GT, GT 210/420/430/440/525M/540M & GTX 560M
	switch (nvda_dev->device_id)
	{
		case 0x0647: vram_size = 512*1024*1024; break;	// 9600M GT 0647
		case 0x0649: vram_size = 512*1024*1024; break;	// 9600M GT 0649
		case 0x0A65: vram_size = 1024*1024*1024; break; // GT 210
		case 0x0DE0: vram_size = 1024*1024*1024; break; // GT 440
		case 0x0DE1: vram_size = 1024*1024*1024; break; // GT 430
		case 0x0DE2: vram_size = 1024*1024*1024; break; // GT 420
		case 0x0DEC: vram_size = 1024*1024*1024; break; // GT 525M 0DEC
		case 0x0DF4: vram_size = 1024*1024*1024; break; // GT 540M
		case 0x0DF5: vram_size = 1024*1024*1024; break; // GT 525M 0DF5
		case 0x1251: vram_size = 1536*1024*1024; break; // GTX 560M
		default: break;
	}
	
	return vram_size;
}

bool setup_nvidia_devprop(pci_dt_t *nvda_dev)
{
	struct DevPropDevice	*device;
	char					*devicepath;
	option_rom_pci_header_t *rom_pci_header;
	volatile uint8_t		*regs;
	uint8_t					*rom;
	uint8_t					*nvRom;
	uint8_t					nvCardType;
	unsigned long long		videoRam;
	uint32_t				nvBiosOveride;
	uint32_t				bar[7];
	uint32_t				boot_display;
	int						nvPatch;
	int						len;
	char					biosVersion[32];
	char					nvFilename[32];
	char					kNVCAP[12];
	char					*model;
	const char				*value;
	bool					doit;
	
	devicepath = get_pci_dev_path(nvda_dev);
	bar[0] = pci_config_read32(nvda_dev->dev.addr, 0x10 );
	regs = (uint8_t *) (bar[0] & ~0x0f);
	
	// get card type
	nvCardType = (REG32(0) >> 20) & 0x1ff;
	
	// Amount of VRAM in kilobytes
	videoRam = mem_detect(regs, nvCardType, nvda_dev);
	model = get_nvidia_model((nvda_dev->vendor_id << 16) | nvda_dev->device_id);
	
	verbose("nVidia %s %dMB NV%02x [%04x:%04x] :: %s\n",
			model, (uint32_t)(videoRam / 1024 / 1024),
			(REG32(0) >> 20) & 0x1ff, nvda_dev->vendor_id, nvda_dev->device_id,
			devicepath);
	
	rom = malloc(NVIDIA_ROM_SIZE);
	sprintf(nvFilename, "/Extra/%04x_%04x.rom", (uint16_t)nvda_dev->vendor_id,
			(uint16_t)nvda_dev->device_id);
	
	if (getBoolForKey(kUseNvidiaROM, &doit, &bootInfo->chameleonConfig) && doit)
	{
		verbose("Looking for nvidia video bios file %s\n", nvFilename);
		nvBiosOveride = load_nvidia_bios_file(nvFilename, rom, NVIDIA_ROM_SIZE);
		
		if (nvBiosOveride > 0)
		{
			verbose("Using nVidia Video BIOS File %s (%d Bytes)\n", nvFilename, nvBiosOveride);
			DBG("%s Signature 0x%02x%02x %d bytes\n", nvFilename, rom[0], rom[1], nvBiosOveride);
		}
		else
		{
			printf("ERROR: unable to open nVidia Video BIOS File %s\n", nvFilename);
			return false;
		}
	}
	else
	{
		// Otherwise read bios from card
		nvBiosOveride = 0;
		
		// TODO: we should really check for the signature before copying the rom, i think.
		
		// PRAMIN first
		nvRom = (uint8_t*)&regs[NV_PRAMIN_OFFSET];
		bcopy((uint32_t *)nvRom, rom, NVIDIA_ROM_SIZE);
		
		// Valid Signature ?
		if (rom[0] != 0x55 && rom[1] != 0xaa)
		{
			// PROM next
			// Enable PROM access
			(REG32(NV_PBUS_PCI_NV_20)) = NV_PBUS_PCI_NV_20_ROM_SHADOW_DISABLED;
			
			nvRom = (uint8_t*)&regs[NV_PROM_OFFSET];
			bcopy((uint8_t *)nvRom, rom, NVIDIA_ROM_SIZE);
			
			// disable PROM access
			(REG32(NV_PBUS_PCI_NV_20)) = NV_PBUS_PCI_NV_20_ROM_SHADOW_ENABLED;
			
			// Valid Signature ?
			if (rom[0] != 0x55 && rom[1] != 0xaa)
			{
				// 0xC0000 last
				bcopy((char *)0xc0000, rom, NVIDIA_ROM_SIZE);
				
				// Valid Signature ?
				if (rom[0] != 0x55 && rom[1] != 0xaa)
				{
					printf("ERROR: Unable to locate nVidia Video BIOS\n");
					return false;
				}
				else
				{
					DBG("ROM Address 0x%x Signature 0x%02x%02x\n", nvRom, rom[0], rom[1]);
				}
			}
			else
			{
				DBG("PROM Address 0x%x Signature 0x%02x%02x\n", nvRom, rom[0], rom[1]);
			}
		}
		else
		{
			DBG("PRAM Address 0x%x Signature 0x%02x%02x\n", nvRom, rom[0], rom[1]);
		}
	}
	
	if ((nvPatch = patch_nvidia_rom(rom)) == PATCH_ROM_FAILED) {
		printf("ERROR: nVidia ROM Patching Failed!\n");
		//return false;
	}
	
	rom_pci_header = (option_rom_pci_header_t*)(rom + *(uint16_t *)&rom[24]);
	
	// check for 'PCIR' sig
	if (rom_pci_header->signature == 0x50434952)
	{
		if (rom_pci_header->device_id != nvda_dev->device_id)
		{
			// Get Model from the OpROM
			model = get_nvidia_model((rom_pci_header->vendor_id << 16) | rom_pci_header->device_id);
		}
		else
		{
			printf("nVidia incorrect PCI ROM signature: 0x%x\n", rom_pci_header->signature);
		}
	}
	
	if (!string) {
		string = devprop_create_string();
	}
	device = devprop_add_device(string, devicepath);
	
	/* FIXME: for primary graphics card only */
	boot_display = 1;
	if (devices_number == 1)
	{
	  devprop_add_value(device, "@0,AAPL,boot-display", (uint8_t*)&boot_display, 4);
	}
	
	if (nvPatch == PATCH_ROM_SUCCESS_HAS_LVDS) {
		uint8_t built_in = 0x01;
		devprop_add_value(device, "@0,built-in", &built_in, 1);
	}

	// get bios version
	const int MAX_BIOS_VERSION_LENGTH = 32;
	char* version_str = (char*)malloc(MAX_BIOS_VERSION_LENGTH);
	
	memset(version_str, 0, MAX_BIOS_VERSION_LENGTH);
	
	int i, version_start;
	int crlf_count = 0;
	
	// only search the first 384 bytes
	for (i = 0; i < 0x180; i++)
	{
		if (rom[i] == 0x0D && rom[i+1] == 0x0A)
		{
			crlf_count++;
			// second 0x0D0A was found, extract bios version
			if (crlf_count == 2)
			{
				if (rom[i-1] == 0x20) i--; // strip last " "
				
				for (version_start = i; version_start > (i-MAX_BIOS_VERSION_LENGTH); version_start--)
				{
					// find start
					if (rom[version_start] == 0x00)
					{
						version_start++;
						
						// strip "Version "
						if (strncmp((const char*)rom+version_start, "Version ", 8) == 0)
						{
							version_start += 8;
						}
						
						strncpy(version_str, (const char*)rom+version_start, i-version_start);
						break;
					}
				}
				break;
			}
		}
	}
	
	sprintf(biosVersion, "%s", (nvBiosOveride > 0) ? nvFilename : version_str);
	sprintf(kNVCAP, "NVCAP_%04x", nvda_dev->device_id);
	
	if (getValueForKey(kNVCAP, &value, &len, &bootInfo->chameleonConfig) && len == NVCAP_LEN * 2)
	{
		uint8_t new_NVCAP[NVCAP_LEN];
		
		if (hex2bin(value, new_NVCAP, NVCAP_LEN) == 0)
		{
			verbose("Using user supplied NVCAP for %s :: %s\n", model, devicepath);
			memcpy(default_NVCAP, new_NVCAP, NVCAP_LEN);
		}
	}
	
	if (getValueForKey(kDcfg0, &value, &len, &bootInfo->chameleonConfig) && len == DCFG0_LEN * 2)
	{
		uint8_t new_dcfg0[DCFG0_LEN];
		
		if (hex2bin(value, new_dcfg0, DCFG0_LEN) == 0)
		{
			memcpy(default_dcfg_0, new_dcfg0, DCFG0_LEN);
			
			verbose("Using user supplied @0,display-cfg\n");
			printf("@0,display-cfg: %02x%02x%02x%02x\n",
				   default_dcfg_0[0], default_dcfg_0[1], default_dcfg_0[2], default_dcfg_0[3]);
		}
	}
	
	if (getValueForKey(kDcfg1, &value, &len, &bootInfo->chameleonConfig) && len == DCFG1_LEN * 2)
	{
		uint8_t new_dcfg1[DCFG1_LEN];
		
		if (hex2bin(value, new_dcfg1, DCFG1_LEN) == 0)
		{
			memcpy(default_dcfg_1, new_dcfg1, DCFG1_LEN);
			
			verbose("Using user supplied @1,display-cfg\n");
			printf("@1,display-cfg: %02x%02x%02x%02x\n",
				   default_dcfg_1[0], default_dcfg_1[1], default_dcfg_1[2], default_dcfg_1[3]);
		}
	}
	
#if DEBUG_NVCAP
	printf("NVCAP: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x\n",
	default_NVCAP[0], default_NVCAP[1], default_NVCAP[2], default_NVCAP[3],
	default_NVCAP[4], default_NVCAP[5], default_NVCAP[6], default_NVCAP[7],
	default_NVCAP[8], default_NVCAP[9], default_NVCAP[10], default_NVCAP[11],
	default_NVCAP[12], default_NVCAP[13], default_NVCAP[14], default_NVCAP[15],
	default_NVCAP[16], default_NVCAP[17], default_NVCAP[18], default_NVCAP[19]);
#endif
	
	devprop_add_nvidia_template(device);
	devprop_add_value(device, "NVCAP", default_NVCAP, NVCAP_LEN);
	devprop_add_value(device, "NVPM", default_NVPM, NVPM_LEN);
	devprop_add_value(device, "VRAM,totalsize", (uint8_t*)&videoRam, 4);
	devprop_add_value(device, "model", (uint8_t*)model, strlen(model) + 1);
	devprop_add_value(device, "rom-revision", (uint8_t*)biosVersion, strlen(biosVersion) + 1);
    //	devprop_add_value(device, "@1,connector-type", connector_type_1, 4); // fixme
    //	devprop_add_value(device, "@0,display-cfg", display_cfg_0, 4);
    //	devprop_add_value(device, "@1,display-cfg", display_cfg_1, 4);
	devprop_add_value(device, "@0,display-cfg", default_dcfg_0, DCFG0_LEN);
	devprop_add_value(device, "@1,display-cfg", default_dcfg_1, DCFG1_LEN);
	
	//add HDMI Audio back to nvidia
	//http://forge.voodooprojects.org/p/chameleon/issues/67/
//	uint8_t connector_type_1[]= {0x00, 0x08, 0x00, 0x00};
//	devprop_add_value(device, "@1,connector-type",connector_type_1, 4);
	//end Nvidia HDMI Audio
	
	if (getBoolForKey(kVBIOS, &doit, &bootInfo->chameleonConfig) && doit)
	{
		devprop_add_value(device, "vbios", rom, (nvBiosOveride > 0) ? nvBiosOveride : (rom[2] * 512));
	}
	
	stringdata = malloc(sizeof(uint8_t) * string->length);
	memcpy(stringdata, (uint8_t*)devprop_generate_string(string), string->length);
	stringlength = string->length;
	
	return true;
}
