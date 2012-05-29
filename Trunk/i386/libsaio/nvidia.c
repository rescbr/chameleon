/*
 *	NVidia injector
 *
 *	Copyright (C) 2009	Jasmin Fazlic, iNDi
 *
 *	NVidia injector modified by Fabio (ErmaC) on May 2012,
 *	for allow the cosmetics injection also based on SubVendorID and SubDeviceID.
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
 *
 *	Alternatively you can choose to comply with APSL
 *
 *	DCB-Table parsing is based on software (nouveau driver) originally distributed under following license:
 *
 *
 *	Copyright 2005-2006 Erik Waling
 *	Copyright 2006 Stephane Marchesin
 *	Copyright 2007-2009 Stuart Bennett
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a
 *	copy of this software and associated documentation files (the "Software"),
 *	to deal in the Software without restriction, including without limitation
 *	the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *	and/or sell copies of the Software, and to permit persons to whom the
 *	Software is furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in
 *	all copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *	THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 *	OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
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
#define READ_BYTE(rom, offset) (*(u_char *)(rom + offset))
#define READ_LE_SHORT(rom, offset) (READ_BYTE(rom, offset+1) << 8 | READ_BYTE(rom, offset))
#define READ_LE_INT(rom, offset)   (READ_LE_SHORT(rom, offset+2) << 16 | READ_LE_SHORT(rom, offset))
#define WRITE_LE_SHORT(data)       (((data) << 8 & 0xff00) | ((data) >> 8 & 0x00ff ))
#define WRITE_LE_INT(data)         (WRITE_LE_SHORT(data) << 16 | WRITE_LE_SHORT(data >> 16))

extern uint32_t devices_number;

const char *nvidia_compatible_0[]       =	{ "@0,compatible",	"NVDA,NVMac"	 };
const char *nvidia_compatible_1[]       =	{ "@1,compatible",	"NVDA,NVMac"	 };
const char *nvidia_device_type_0[]      =	{ "@0,device_type",	"display"	 };
const char *nvidia_device_type_1[]      =	{ "@1,device_type",	"display"	 };
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

static nvidia_card_info_t nvidia_cards[] = {
    /* ========================================================================================
     * Layout is device(VendorId + DeviceId), subdev (SubvendorId + SubdeviceId), display name.
     * ========================================================================================
     */
	/*Unknown*/	{ 0x10DE0000,	NV_SUB_IDS,	"Unknown" },
    /* ------ Specific DeviceID and SubDevID. ------ */
	// 0000 - 0040
	// 0040 - 004F
	// 0050 - 005F
	// 0060 - 006F
	// 0070 - 007F
	// 0080 - 008F
	// 0090 - 009F
	// 00A0 - 00AF
	// 00B0 - 00BF
	// 00C0 - 00CF
	// 00D0 - 00DF
	// 00E0 - 00EF
	// 00F0 - 00FF
	// 0100 - 010F
	// 0110 - 011F
	// 0120 - 012F
	// 0130 - 013F
	// 0140 - 014F
	// 0150 - 015F
	// 0160 - 016F
	// 0170 - 017F
	// 0180 - 018F
	// 0190 - 019F
	{ 0x10DE0193,	0x10438234,	"Asus GeForce 8800 GTS" },
	{ 0x10DE0193,	0x1043823C,	"Asus GeForce 8800 GTS" },
	{ 0x10DE0193,	0x1043825F,	"Asus GeForce 8800 GTS" },
	{ 0x10DE0193,	0x10DE0420,	"Asus GeForce 8800 GTS" },
	{ 0x10DE0193,	0x10DE0421,	"Albatron GeForce 8800 GTS" },
	{ 0x10DE0193,	0x19F104A6,	"BFG GeForce 8800 GTS" },
	{ 0x10DE019D,	0x107D2A72,	"Leadtek Quadro FX 5600" },
	{ 0x10DE019D,	0x10DE0409,	"nVidia Quadro FX 5600" },
	{ 0x10DE019E,	0x107D2A72,	"Leadtek Quadro FX 4600" },
	{ 0x10DE019E,	0x10DE0408,	"nVidia Quadro FX 4600" }, // fwood
	// 01A0 - 01AF
	// 01B0 - 01BF
	// 01C0 - 01CF
	// 01D0 - 01DF
	{ 0x10DE01D8,	0x10250090,	"Acer GeForce Go 7400" },
	{ 0x10DE01D8,	0x102801C8,	"Dell GeForce Go 7400" },
	{ 0x10DE01D8,	0x102801CC,	"Dell Quadro NVS 120M" },
	{ 0x10DE01D8,	0x102801D7,	"Dell GeForce Go 7400" },
	{ 0x10DE01D8,	0x102801F3,	"Dell GeForce Go 7400" },
	{ 0x10DE01D8,	0x102801F9,	"Dell GeForce Go 7400" },
	{ 0x10DE01D8,	0x102801FE,	"Dell GeForce Go 7400" },
	{ 0x10DE01D8,	0x10280209,	"Dell GeForce Go 7400" },
	{ 0x10DE01D8,	0x10282003,	"Dell Quadro NVS 120M" },
	{ 0x10DE01D8,	0x103C30A5,	"HP GeForce Go 7400" },
	{ 0x10DE01D8,	0x103C30B6,	"HP GeForce Go 7400" },
	{ 0x10DE01D8,	0x103C30B7,	"HP GeForce Go 7400" },
	{ 0x10DE01D8,	0x103C30BB,	"HP GeForce Go 7400" },
	{ 0x10DE01D8,	0x10431211,	"Asus GeForce Go 7400" },
	{ 0x10DE01D8,	0x10431214,	"Asus GeForce Go 7400" },
	{ 0x10DE01D8,	0x104D81E6,	"Sony GeForce Go 7400" },
	{ 0x10DE01D8,	0x104D81EF,	"Sony GeForce Go 7400" },
	{ 0x10DE01D8,	0x104D81FD,	"Sony GeForce Go 7400" },
	{ 0x10DE01D8,	0x104D8205,	"Sony GeForce Go 7400" },
	{ 0x10DE01D8,	0x104D820F,	"Sony GeForce Go 7400" },
	{ 0x10DE01D8,	0x109F319C,	"Trigem GeForce Go 7400" },
	{ 0x10DE01D8,	0x109F319D,	"Trigem GeForce Go 7400" },
	{ 0x10DE01D8,	0x109F3C01,	"Trigem GeForce Go 7400" },
	{ 0x10DE01D8,	0x11790001,	"Toshiba GeForce Go 7400" },
	{ 0x10DE01D8,	0x1179FF00,	"Toshiba GeForce Go 7400" },
	{ 0x10DE01D8,	0x1179FF10,	"Toshiba GeForce Go 7400" },
	{ 0x10DE01D8,	0x1179FF31,	"Toshiba GeForce Go 7400" },
	{ 0x10DE01D8,	0x144D8062,	"Samsung GeForce Go 7400" },
	{ 0x10DE01D8,	0x144DB03C,	"Samsung GeForce Go 7400" },
	{ 0x10DE01D8,	0x144DC024,	"Samsung GeForce Go 7400" },
	{ 0x10DE01D8,	0x144DC026,	"Samsung GeForce Go 7400" },
	{ 0x10DE01D8,	0x14620511,	"MSi GeForce Go 7400" },
	{ 0x10DE01D8,	0x14623FCC,	"MSi GeForce Go 7400" },
	{ 0x10DE01D8,	0x14623FDF,	"MSi GeForce Go 7400" },
	{ 0x10DE01D8,	0x14624327,	"MSi GeForce Go 7400" },
 	{ 0x10DE01D8,	0x15092A30,	"GeForce Go 7400" }, // First International Computer Inc
	{ 0x10DE01D8,	0x152D0753,	"Quanta GeForce Go 7400" },
	{ 0x10DE01D8,	0x152D0763,	"Quante GeForce Go 7400" },
	{ 0x10DE01D8,	0x161F203D,	"Arima GeForce Go 7400" },
	{ 0x10DE01D8,	0x161F2052,	"Arima GeForce Go 7400" },
	{ 0x10DE01D8,	0x161F2054,	"Arima GeForce Go 7400" },
	{ 0x10DE01D8,	0x161F205D,	"Arima GeForce Go 7400" },
 	{ 0x10DE01D8,	0x1631C022,	"NEC GeForce Go 7400" },
	{ 0x10DE01D8,	0x173410D3,	"Fujitsu GeForce Go 7400" },
	{ 0x10DE01D8,	0x17AA2075,	"Lenovo GeForce Go 7400" },
	{ 0x10DE01D8,	0x17AA3833,	"Lenovo GeForce Go 7400" },
	{ 0x10DE01D8,	0x17AA39F5,	"Lenovo GeForce Go 7400" },
	{ 0x10DE01D8,	0x17AA6666,	"Lenovo GeForce Go 7400" },
	{ 0x10DE01D8,	0x17C0207F,	"Wistron GeForce Go 7400" },
 	{ 0x10DE01D8,	0x17C02083,	"Wistron GeForce Go 7400" },
	{ 0x10DE01D8,	0x17FF500E,	"Benq GeForce Go 7400" },
	{ 0x10DE01D8,	0x18940040,	"LG GeForce Go 7400" },
	{ 0x10DE01D8,	0x18640041,	"LG GeForce Go 7400" },
	{ 0x10DE01D8,	0x18640042,	"LG GeForce Go 7400" },
	{ 0x10DE01D8,	0x18640043,	"LG GeForce Go 7400" },
	{ 0x10DE01D8,	0x18640044,	"LG GeForce Go 7400" },
 	{ 0x10DE01D8,	0x18640045,	"LG GeForce Go 7400" },
	{ 0x10DE01D8,	0x18640046,	"LG GeForce Go 7400" },
	{ 0x10DE01D8,	0x18640047,	"LG GeForce Go 7400" },
	{ 0x10DE01D8,	0x1864007A,	"LG GeForce Go 7400" },
	{ 0x10DE01D8,	0x19614605,	"ESS GeForce Go 7400" },
	{ 0x10DE01D8,	0x19615607,	"ESS GeForce Go 7400" },
	{ 0x10DE01D8,	0x19915532,	"GeForce Go 7400" }, // Topstar Digital Technologies Co., Ltd.
 	{ 0x10DE01D8,	0x19DB2174,	"GeForce Go 7400" }, // ??
	{ 0x10DE01D8,	0xC0181631,	"GeForce Go 7400" }, // ??
	{ 0x10DE01DA,	0x1028017D,	"Dell Quadro NVS 110M" },
	{ 0x10DE01DA,	0x10280407,	"Dell GeForce 7300 LE" },
	{ 0x10DE01DA,	0x11790001,	"Toshiba Quadro NVS 110M" },
	{ 0x10DE01DA,	0x11790002,	"Toshiba Quadro NVS 110M" },
	{ 0x10DE01DA,	0x11790010,	"Toshiba Quadro NVS 110M" },
	{ 0x10DE01DA,	0x1179FF00,	"Toshiba Quadro NVS 110M" },
	{ 0x10DE01DA,	0x1179FF10,	"Toshiba Quadro NVS 110M" },
	// 01E0 - 01EF
	// 01F0 - 01FF
	// 0200 - 020F
	// 0210 - 021F
	// 0220 - 022F
	// 0230 - 023F
	// 0240 - 024F
	// 0250 - 025F
	// 0260 - 026F
	// 0270 - 027F
	// 0280 - 028F
	// 0290 - 029F
	// 02A0 - 02AF
	// 02B0 - 02BF
	// 02C0 - 02CF
	// 02D0 - 02DF
	// 02E0 - 02EF
	// 02F0 - 02FF
	// 0300 - 030F
	// 0310 - 031F
	// 0320 - 032F
	// 0330 - 033F
	// 0340 - 034F
	// 0350 - 035F
	// 0360 - 036F
	// 0370 - 037F
	// 0380 - 038F
	// 0390 - 039F
	// 03A0 - 03AF
	// 03B0 - 03BF
	// 03C0 - 03CF
	// 03D0 - 03DF
	// 03E0 - 03EF
	// 03F0 - 03FF
	// 0400 - 040F
	{ 0x10DE0402,	0x10DE0439,	"Galaxy 8600GT" }, // pianman
	{ 0x10DE0407,	0x101922D4,	"Elitegroup GeForce 8600M GT" },
	{ 0x10DE0407,	0x1025011D,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x1025011E,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250121,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250125,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250126,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250127,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250129,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x1025012B,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250136,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x1025013D,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x1025013F,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250142,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250143,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250145,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x10250146,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x1025015E,	"Acer GeForce 8600M GT" },
	{ 0x10DE0407,	0x1028019C,	"Dell GeForce 8600M GT" },
	{ 0x10DE0407,	0x102801F1,	"Dell GeForce 8600M GT" }, // LatinMcG
	{ 0x10DE0407,	0x102801F2,	"Dell GeForce 8600M GT" },
	{ 0x10DE0407,	0x10280228,	"Dell GeForce 8600M GT" },
	{ 0x10DE0407,	0x10280229,	"Dell GeForce 8600M GT" },
	{ 0x10DE0407,	0x1028022E,	"Dell GeForce 8600M GT" }, // DarwinX
	{ 0x10DE0407,	0x10431515,	"Asus GeForce 8600M GT" },
	{ 0x10DE0407,	0x10431588,	"Asus GeForce 8600M GT" },
	{ 0x10DE0407,	0x10431618,	"Asus GeForce 8600M GT" },
	{ 0x10DE0407,	0x10431632,	"Asus GeForce 8600M GT" },
	{ 0x10DE0407,	0x104314A2,	"Asus GeForce 8600M GT" },
	{ 0x10DE0407,	0x104381F7,	"Asus GeForce 8600M GT" },
	{ 0x10DE0407,	0x104D9005,	"Sony GeForce 8600M GT" },
	{ 0x10DE0407,	0x104D9016,	"Sony GeForce 8600M GT" },
	{ 0x10DE0407,	0x104D9018,	"Sony GeForce 8600M GT" },
	{ 0x10DE0407,	0x106B00A0,	"Apple GeForce 8600M GT" },
	{ 0x10DE0407,	0x106B00A3,	"Apple GeForce 8600M GT" },
	{ 0x10DE0407,	0x106B00A4,	"Apple GeForce 8600M GT" },
	// 0410 - 041F
	// 0420 - 042F
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
	// 04D0 - 04DF
	// 04E0 - 04EF
	// 04F0 - 04FF
	// 0500 - 050F
	// 0510 - 051F
	// 0520 - 052F
	// 0530 - 053F
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
	{ 0x10DE05E0,	0x10DE064E,	"nVidia GeForce GTX 295" },
	{ 0x10DE05E0,	0x38421291,	"EVGA GeForce GTX 295" },
	{ 0x10DE05E2,	0x10438291,	"Asus GTX 260" },
	{ 0x10DE05E2,	0x10438298,	"Asus GTX 260" },
	{ 0x10DE05E2,	0x104382C4,	"Asus GTX 260" },
	{ 0x10DE05E2,	0x104382CF,	"Asus GTX 260" },
	{ 0x10DE05E2,	0x104382E3,	"Asus GTX 260" },
	{ 0x10DE05E2,	0x10B00801,	"Gainward GTX 260" },
	{ 0x10DE05E2,	0x10DE0585,	"nVidia GTX 260" },
	{ 0x10DE05E2,	0x10DE0617,	"nVidia GTX 260" },
	{ 0x10DE05E2,	0x16822390,	"HFX GTX 260" },
	{ 0x10DE05E2,	0x17870000,	"HIS GTX 260" },
	{ 0x10DE05E2,	0x34421260,	"Bihl GTX 260" },
	{ 0x10DE05E2,	0x34421262,	"Bihl GTX 260" },
	{ 0x10DE05E2,	0x73770000,	"Colorful GTX 260" },
	{ 0x10DE05E3,	0x10438320,	"Asus GeForce GTX 285" }, // mauriziopasotti
	{ 0x10DE05E3,	0x106B0000,	"Apple GeForce GTX 285" },
	{ 0x10DE05E3,	0x10DE065B,	"nVidia GeForce GTX 285" },
	{ 0x10DE05E3,	0x38421080,	"EVGA GeForce GTX 285" },
	{ 0x10DE05E3,	0x38421187,	"EVGA GeForce GTX 285" },
	{ 0x10DE05E7,	0x10DE0595,	"nVidia Tesla T10 Processor" },
	{ 0x10DE05E7,	0x10DE066A,	"nVidia Tesla C1060" },
	{ 0x10DE05E7,	0x10DE068F,	"nVidia Tesla T10 Processor" },
	{ 0x10DE05E7,	0x10DE0697,	"nVidia Tesla M1060" },
	{ 0x10DE05E7,	0x10DE0714,	"nVidia Tesla M1060" },
	{ 0x10DE05E7,	0x10DE0743,	"nVidia Tesla M1060" },
	{ 0x10DE05EB,	0x10DE0705,	"nVidia GeForce GTX 295" },
	{ 0x10DE05EB,	0x19F110C2,	"BFG GeForce GTX 295" },
	// 05F0 - 05FF
	// 0600 - 060F
	{ 0x10DE0600,	0x10438268,	"Asus GeForce 8800 GTS" },
	{ 0x10DE0600,	0x1043826C,	"Asus GeForce 8800 GTS" },
	{ 0x10DE0600,	0x10DE0000,	"Abit GeForce 8800 GTS" },
	{ 0x10DE0600,	0x10DE0502,	"nVidia GeForce 8800 GTS" },
	{ 0x10DE0600,	0x19F10719,	"BFG GeForce 8800 GTS" },
	// 0610 - 061F
	{ 0x10DE0611,	0x104381F7,	"Asus GeForce 8800 GT" },
	{ 0x10DE0611,	0x10DE053C,	"nVidia GeForce 8800 GT" },
	{ 0x10DE0611,	0x14621171,	"MSi GeForce 8800 GT" },
	{ 0x10DE0611,	0x14621172,	"MSi GeForce 8800 GT" },
	{ 0x10DE0611,	0x174B9210,	"PC Partner GeForce 8800 GT" },
	{ 0x10DE0611,	0x1ACC8582,	"Point of View GeForce 8800 GT" }, // Alex//3oo1
	{ 0x10DE0611,	0x3842C802,	"EVGA GeForce 8800 GT" },
	{ 0x10DE0618,	0x1025028E,	"Acer GeForce GTX 260M" },
	{ 0x10DE0618,	0x102802A1,	"Dell GeForce GTX 260M" },
	{ 0x10DE0618,	0x102802A2,	"Dell GeForce GTX 260M" },
	{ 0x10DE0618,	0x10431A52,	"Asus GeForce GTX 260M" },
	{ 0x10DE0618,	0x10432028,	"Asus GeForce GTX 170M" },
	{ 0x10DE0618,	0x1043202B,	"Asus GTX 680" },
	{ 0x10DE0618,	0x10432033,	"Asus GeForce GTX 260M" },
	{ 0x10DE0618,	0x15580481,	"Clevo/Kapok GeForce GTX 260M" },
	{ 0x10DE0618,	0x15580577,	"Clevo/Kapok GeForce GTX 260M" },
	{ 0x10DE0618,	0x15580860,	"Clevo/Kapok GeForce GTX 260M" },
	// 0620 - 062F
	{ 0x10DE0622,	0x104382AC,	"Asus EN9600GT Magic" }, // Fabio71
	// 0630 - 063F
	// 0640 - 064F
	{ 0x10DE0640,	0x106B00AD,	"Apple GeForge 9500 GT" },
	{ 0x10DE0640,	0x106B00B3,	"Apple GeForge 9500 GT" },
	{ 0x10DE0640,	0x106B061B,	"Apple GeForge 9500 GT" },
	{ 0x10DE0640,	0x10DE077F,	"Inno3D GeForge 9500GT HDMI" }, // Fabio71
	{ 0x10DE0640,	0x14621290,	"MSi GeForge 9500 GT" },
	{ 0x10DE0640,	0x14621291,	"MSi GeForge 9500 GT" },
	{ 0x10DE0640,	0x16423796,	"Bitland GeForge 9500 GT" },
	// 0650 - 065F
	// 0660 - 066F
	// 0670 - 067F
	// 0680 - 068F
	// 0690 - 069F
	// 06A0 - 06AF
	// 06B0 - 06BF
	// 06C0 - 06CF
	{ 0x10DE06C0,	0x10DE075F,	"nVidia GeForce GTX 480" },
	// { 0x10DE06C0,	0x19DA0010,	"Zotac GTX 480 AMP" },
	{ 0x10DE06C0,	0x38421482,	"EVGA GTX 480" },
	{ 0x10DE06CD,	0x10DE079F,	"Point of View GeForce GTX 470" }, // Alex//3oo1
	{ 0x10DE06CD,	0x10DE979F,	"nVidia GeForce GTX 470" },
	{ 0x10DE06CD,	0x19DA0010,	"Zotac GTX 470" },
	{ 0x10DE06CD,	0x19DA1153,	"Zotac GeForce GTX 470" }, // magnifico10 and Fabio71 TODO: AMP???
	// 06D0 - 06DF
	{ 0x10DE06D1,	0x10DE0771,	"nVidia Tesla C2050" },
	{ 0x10DE06D1,	0x10DE0772,	"nVidia Tesla C2070" },
	{ 0x10DE06D2,	0x10DE0774,	"nVidia Tesla M2070" },
	{ 0x10DE06D2,	0x10DE0830,	"nVidia Tesla M2070" },
	{ 0x10DE06D2,	0x10DE0842,	"nVidia Tesla M2070" },
	{ 0x10DE06D2,	0x10DE088F,	"nVidia Tesla X2070" },
	{ 0x10DE06D2,	0x10DE0908,	"nVidia Tesla M2070" },
	{ 0x10DE06DD,	0x103C076F,	"HP Quadro 6000" },
	{ 0x10DE06DD,	0x10DE076F,	"nVidia Quadro 6000" },
	{ 0x10DE06D9,	0x103C0770,	"HP Quadro 5000" },
	{ 0x10DE06D9,	0x10DE0770,	"nVidia Quadro 5000" },
	{ 0x10DE06DD,	0x1028081A,	"Dell Quadro 5000M" },
	{ 0x10DE06DD,	0x103C1520,	"HP Quadro 5000M" },
	{ 0x10DE06DD,	0x103C0780,	"HP Quadro 4000" },
	{ 0x10DE06DD,	0x10DE0780,	"nVidia Quadro 4000" },
	{ 0x10DE06DE,	0x10DE0773,	"nVidia Tesla S2050" },
	{ 0x10DE06DE,	0x10DE077A,	"nVidia Tesla M2050" },
	{ 0x10DE06DE,	0x10DE082F,	"nVidia Tesla M2050" },
	{ 0x10DE06DE,	0x10DE0830,	"nVidia Tesla M2070" },
	{ 0x10DE06DE,	0x10DE0831,	"nVidia Tesla M2070" },
	{ 0x10DE06DE,	0x10DE0832,	"nVidia Tesla M2070" },
	{ 0x10DE06DE,	0x10DE0840,	"nVidia Tesla X2070" },
	{ 0x10DE06DE,	0x10DE0842,	"nVidia Tesla M2050" },
	{ 0x10DE06DE,	0x10DE0843,	"nVidia Tesla M2050" },
	{ 0x10DE06DE,	0x10DE0846,	"nVidia Tesla M2050" },
	{ 0x10DE06DE,	0x10DE0866,	"nVidia Tesla M2050" },
	{ 0x10DE06DE,	0x10DE0907,	"nVidia Tesla M2050" },
	{ 0x10DE06DE,	0x10DE091E,	"nVidia Tesla M2050" },
	{ 0x10DE06DF,	0x10DE0842,	"nVidia Tesla M2070-Q" },
	{ 0x10DE06DF,	0x10DE084D,	"nVidia Tesla M2070-Q" },
	{ 0x10DE06DF,	0x10DE087F,	"nVidia Tesla M2070-Q" },
	// 06E0 - 06EF
	{ 0x10DE06E4,	0x10438322,	"Asus EN8400GS" }, // Fabio71
	{ 0x10DE06E4,	0x14583475,	"GV-NX84S256HE [GeForce 8400 GS]" },
	{ 0x10DE06E8,	0x10280262,	"Dell GeForce 9200M GS" },
	{ 0x10DE06E8,	0x10280271,	"Dell GeForce 9200M GS" },
	{ 0x10DE06E8,	0x10280272,	"Dell GeForce 9200M GS" },
	{ 0x10DE06E8,	0x103C30F4,	"HP GeForce 9200M GS" },
	{ 0x10DE06E8,	0x103C30F7,	"HP GeForce 9200M GS" },
	{ 0x10DE06E8,	0x103C3603,	"HP GeForce 9200M GS" },
	{ 0x10DE06E8,	0x103C360B,	"HP GeForce 9200M GE" },
	{ 0x10DE06E8,	0x103C3621,	"HP GeForce 9200M GE" },
	{ 0x10DE06E8,	0x103C3629,	"HP GeForce 9200M GE" },
	{ 0x10DE06E8,	0x10432008,	"Asus GeForce 9200M GE" },
	{ 0x10DE06E8,	0x107B0900,	"Gateway GeForce 9200M GE" },
	{ 0x10DE06E8,	0x11790001,	"Toshiba GeForce 9200M GE" },
	{ 0x10DE06E8,	0x144DC041,	"Samsung GeForce 9200M GE" },
	{ 0x10DE06E8,	0x144DC042,	"Samsung GeForce 9200M GE" },
	{ 0x10DE06E8,	0x144DC048,	"Samsung GeForce 9200M GE" },
	{ 0x10DE06E8,	0x144DC04A,	"Samsung GeForce 9200M GE" },
	{ 0x10DE06E8,	0x144DC521,	"Samsung GeForce 9200M GE" },
	{ 0x10DE06E8,	0x144DC524,	"Samsung GeForce 9200M GE" },
	{ 0x10DE06E8,	0x152D0772,	"Quanta GeForce 9200M GE" },
	{ 0x10DE06E8,	0x152D0773,	"Quanta GeForce 9200M GE" },
	{ 0x10DE06E8,	0x152D0774,	"Quanta GeForce 9200M GE" },
	{ 0x10DE06E8,	0x152D0775,	"Quanta GeForce 9200M GE" },
	{ 0x10DE06E8,	0x17341146,	"Fujitsu GeForce 9200M GE" },
	{ 0x10DE06E8,	0x18541772,	"LG GeForce 9200M GE" },
	{ 0x10DE06E8,	0x18541773,	"LG GeForce 9200M GE" },
	{ 0x10DE06E8,	0x18541774,	"LG GeForce 9200M GE" },
	{ 0x10DE06E8,	0x18541775,	"LG GeForce 9200M GE" },
	{ 0x10DE06E8,	0x19614605,	"ESS GeForce 9200M GE" },
	{ 0x10DE06E8,	0x19615584,	"ESS GeForce 9200M GE" },
	{ 0x10DE06E8,	0x1B0A000E,	"Pegatron GeForce 9200M GE" },
	// 06F0 - 06FF
	// 0700 - 070F
	// 0710 - 071F
	// 0720 - 072F
	// 0730 - 073F
	// 0740 - 074F
	// 0750 - 075F
	// 0760 - 076F
	// 0770 - 077F
	// 0780 - 078F
	// 0790 - 079F
	// 07A0 - 07AF
	// 07B0 - 07BF
	// 07C0 - 07CF
	// 07D0 - 07DF
	// 07E0 - 07EF
	// 07F0 - 07FF
	// 0800 - 080F
	// 0810 - 081F
	// 0820 - 082F
	// 0830 - 083F
	// 0840 - 084F
	// 0850 - 085F
	// 0860 - 086F
	// 0870 - 087F
	{ 0x10DE0876,	0x103C3651,	"HP ION" },
	{ 0x10DE0876,	0x10438402,	"Asus ION" },
	// 0880 - 088F
	// 0890 - 089F
	// 08A0 - 08AF
	// 08B0 - 08BF
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
	// 0A10 - 0A1F
	// 0A20 - 0A2F
	{ 0x10DE0A20,	0x10438311,	"Asus GeForce GT 220" },
	// 0A30 - 0A3F
	// 0A40 - 0A4F
	// 0A50 - 0A5F
	// 0A60 - 0A6F
	{ 0x10DE0A64,	0x1025063C,	"Acer ION" },
	{ 0x10DE0A64,	0x103C2AAD,	"HP ION" },
	{ 0x10DE0A64,	0x10430010,	"Asus ION2" }, // buoo
	{ 0x10DE0A64,	0x1043841F,	"Asus ION" },
	{ 0x10DE0A64,	0x1043842F,	"Asus ION" },
	{ 0x10DE0A64,	0x10438455,	"Asus ION" },
	{ 0x10DE0A64,	0x1043845B,	"Asus ION" },
	{ 0x10DE0A64,	0x1043845E,	"Asus ION" },
	{ 0x10DE0A64,	0x17AA3605,	"Lenovo ION" },
	{ 0x10DE0A64,	0x18490A64,	"ASRock ION" },
	{ 0x10DE0A64,	0x1B0A00CE,	"Pegatron ION" },
   	{ 0x10DE0A64,	0x1B0A00D7,	"Pegatron ION" },
	{ 0x10DE0A65,	0x10438334,	"Asus GeForce 210" },
	{ 0x10DE0A65,	0x10438353,	"Asus GeForce 210" },
	{ 0x10DE0A65,	0x10438354,	"Asus GeForce 210" },
	{ 0x10DE0A65,	0x10DE0794,	"nVidia GeForce 210" },
	{ 0x10DE0A65,	0x10DE0847,	"nVidia GeForce 210" },
	{ 0x10DE0A65,	0x145834D5,	"GigaByte GeForce 210" },
	{ 0x10DE0A65,	0x145834EF,	"GigaByte GeForce 210" },
	{ 0x10DE0A65,	0x16822941,	"XFX GeForce 210" },
	{ 0x10DE0A6C,	0x1028040B,	"Dell NVS 3100M" },
	{ 0x10DE0A6C,	0x17AA2142,	"Lenovo NVS 3100M" },
	// 0A70 - 0A7F
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
	{ 0x10DE0CA3,	0x10438326,	"Asus GeForce GT 240" },
	{ 0x10DE0CA3,	0x10438328,	"Asus GeForce GT 240" },
	{ 0x10DE0CA3,	0x1043832A,	"Asus GeForce GT 240" },
	{ 0x10DE0CA3,	0x1043832E,	"Asus GeForce GT 240" },
	{ 0x10DE0CA3,	0x10438335,	"Asus GeForce GT 240" },
	{ 0x10DE0CA3,	0x145834E2,	"GigaByte GeForce GT 240" },
	{ 0x10DE0CA3,	0x145834E5,	"GigaByte GeForce GT 240" },
	{ 0x10DE0CA3,	0x145834E6,	"GigaByte GeForce GT 240" },
	{ 0x10DE0CA3,	0x14621900,	"MSi GeForce GT 230" },
	{ 0x10DE0CA3,	0x14621913,	"MSi GeForce GT 230" },
	{ 0x10DE0CA3,	0x14622070,	"MSi GeForce GT 240" },
	{ 0x10DE0CA3,	0x14622072,	"MSi GeForce GT 240" },
	{ 0x10DE0CA3,	0x14622073,	"MSi GeForce GT 240" },
	{ 0x10DE0CA3,	0x14628010,	"MSi GeForce GT 240" },
	{ 0x10DE0CA3,	0x14628041,	"MSi VN240GT-MD1G" }, //Fabio71
	{ 0x10DE0CA3,	0x16423926,	"Bitland GeForce GT 230" },
	{ 0x10DE0CA3,	0x196E0010,	"PNY GeForce GT 240" },
	{ 0x10DE0CA3,	0x196E069D,	"PNY GeForce GT 240" }, // ErmaC
	{ 0x10DE0CA3,	0x196E075B,	"PNY GeForce GT 240" },
	{ 0x10DE0CA3,	0x19DA1142,	"Zotac GeForce GT 240" },
	{ 0x10DE0CA3,	0x19DA1143,	"Zotac GeForce GT 240" },
	{ 0x10DE0CA3,	0x19DA1144,	"Zotac GeForce GT 240" },
	{ 0x10DE0CA3,	0x19DA2130,	"Zotac GeForce GT 240" },
	{ 0x10DE0CA3,	0x19DA2134,	"Zotac GeForce GT 240" },
	// 0CB0 - 0CBF
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
	{ 0x10DE0DD1,	0x102802A2,	"Dell GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1028048F,	"Dell GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10280490,	"Dell GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10280491,	"Dell GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x102804BA,	"Dell GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1043203D,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1043203E,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10432040,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10432041,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10432042,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10432043,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10432044,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10432045,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10432046,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10432047,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10432048,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1043204A,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1043204B,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10438465,	"Asus GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x10DE10DE,	"nVidia GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1179FC00,	"Toshiba GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1179FC01,	"Toshiba GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1179FC05,	"Toshiba GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1179FCB0,	"Toshiba GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1179FF50,	"Toshiba GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1179FFD6,	"Toshiba GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1179FFD7,	"Toshiba GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x1179FFD8,	"Toshiba GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x14621083,	"MSi GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x15585102,	"Clevo/Kapok GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x15587100,	"Clevo/Kapok GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x15587200,	"Clevo/Kapok GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x15588100,	"Clevo/Kapok GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x15588687,	"Clevo/Kapok GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x17AA3620,	"Lenovo GeForce GTX 460M" },
	{ 0x10DE0DD1,	0x17C010EA,	"Wistron GeForce GTX 460M" },
	{ 0x10DE0DD6,	0x10280010,	"Dell GeForce GT 550M" },
	{ 0x10DE0DD8,	0x103C084A,	"HP nVidia Quadro 2000" },
	{ 0x10DE0DD8,	0x10DE084A,	"nVidia Quadro 2000" }, // mauriziopasotti
	{ 0x10DE0DD8,	0x10DE0914,	"nVidia Quadro 2000 D" },
	// 0DE0 - 0DEF
	{ 0x10DE0DE1,	0x38421430,	"EVGA GeForce GT 430" },
	{ 0x10DE0DE9,	0x10250487,	"Acer GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10250488,	"Acer GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10250505,	"Acer GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10250507,	"Acer GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10250512,	"Acer GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10250573,	"Acer GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10250574,	"Acer GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10250575,	"Acer GeForce GT 630M" },
	{ 0x10DE0DE9,	0x1028055E,	"Dell GeForce GT 630M" },
	{ 0x10DE0DE9,	0x10280563,	"Dell GeForce GT 630M" },
	{ 0x10DE0DE9,	0x103C181A,	"HP GeForce GT 630M" },
	{ 0x10DE0DE9,	0x103C181B,	"HP GeForce GT 630M" },
	{ 0x10DE0DE9,	0x103C181D,	"HP GeForce GT 630M" },
	{ 0x10DE0DE9,	0x103C1837,	"HP GeForce GT 630M" },
	// 0DF0 - 0DFF
	// 0E00 - 0E0F
	// 0E10 - 0E1F
	// 0E20 - 0E2F
	{ 0x10DE0E22,	0x1043835D,	"Asus GeForce GTX 460" },
	{ 0x10DE0E22,	0x10B00401,	"Gainward GeForce GTX 460" },
	{ 0x10DE0E22,	0x10B00801,	"Gainward GeForce GTX 460" },
	{ 0x10DE0E22,	0x10DE0804,	"nVidia GeForce GTX 460" },
	{ 0x10DE0E22,	0x10DE0865,	"nVidia GeForce GTX 460" },
	{ 0x10DE0E22,	0x145834FA,	"GigaByte GeForce GTX 460" },
	{ 0x10DE0E22,	0x145834FC,	"GigaByte GeForce GTX 460" },
	{ 0x10DE0E22,	0x14583501,	"GigaByte GeForce GTX 460" },
	{ 0x10DE0E22,	0x14622321,	"MSi GeForce GTX 460" },
	{ 0x10DE0E22,	0x14622322,	"MSi GeForce GTX 460" },
	{ 0x10DE0E22,	0x14622381,	"MSi GeForce GTX 460" },
	{ 0x10DE0E22,	0x19DA1166,	"Zotac GeForce GTX 460" },
	{ 0x10DE0E22,	0x19DA2166,	"Zotac GeForce GTX 460" }, // Fabio71
	{ 0x10DE0E22,	0x38421370,	"GeForce GTX 460" },
	{ 0x10DE0E22,	0x38421373,	"GeForce GTX 460" },
	{ 0x10DE0E23,	0x10B00401,	"Gainward GeForce GTX 460" },
	// 0E30 - 0E3F
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
	// 0FE0 - 0FEF
	// 0FF0 - 0FFF
	// 1000 - 100F
	// 1010 - 101F
	// 1020 - 102F
	// 1030 - 103F
	// 1040 - 104F
	{ 0x10DE1040,	0x104383A0,	"Asus GeForce GT 520" },
	{ 0x10DE1040,	0x104383BD,	"Asus GeForce GT 520" },
	{ 0x10DE1040,	0x104383C1,	"Asus GeForce GT 520" },
	{ 0x10DE1040,	0x14622592,	"MSi GeForce GT 520" },
	{ 0x10DE1040,	0x14622593,	"MSi GeForce GT 520" },
	{ 0x10DE1040,	0x16423A98,	"Bitland GeForce GT 520" },
	{ 0x10DE1040,	0x16423B42,	"Bitland GeForce GT 520" },
	{ 0x10DE1040,	0x174B3214,	"PC Partner GeForce GT 520" },
	{ 0x10DE1040,	0x196E0915,	"PNY GeForce GT 520" },
	{ 0x10DE1040,	0x19DA1215,	"Zotac GeForce GT 520" },
	{ 0x10DE1040,	0x19DA1222,	"Zotac GeForce GT 520" }, // robertx
	{ 0x10DE1040,	0x1ACC5213,	"Point of View GeForce GT 520" },
	{ 0x10DE1040,	0x1ACC5214,	"Point of View GeForce GT 520" },
	{ 0x10DE1040,	0x1ACC522C,	"Point of View GeForce GT 520" },
	{ 0x10DE1040,	0x1B0A90AA,	"Pegatron GeForce GT 520" },
	{ 0x10DE1054,	0x10280511,	"Dell GeForce 410M" },
	{ 0x10DE1054,	0x10CF1656,	"Fujitsu GeForce 410M" },
	{ 0x10DE1054,	0x10CF1657,	"Fujitsu GeForce 410M" },
	{ 0x10DE1054,	0x1179FCC0,	"Toshiba GeForce 410M" },
	{ 0x10DE1054,	0x14581100,	"GigaByte GeForce 410M" },
	{ 0x10DE1054,	0x14581125,	"GigaByte GeForce 410M" },
	{ 0x10DE1055,	0x104D908A,	"Sony GeForce 410M" },
	{ 0x10DE1055,	0x104D908B,	"Sony GeForce 410M" }, // shulillo vedder
	// 1060 - 106F
	// 1070 - 107F
	// 1080 - 108F
	{ 0x10DE1081,	0x10DE087E,	"nVidia GeForce GTX 570" },
	{ 0x10DE1086,	0x10DE0871,	"Inno3D GeForce GTX 570" },
	{ 0x10DE1087,	0x104383D6,	"Asus ENGTX560Ti448 DCII" },
	{ 0x10DE1087,	0x19DA2207,	"Zotac GeForce GTX 560 Ti-448" },
	// 1090 - 109F
	{ 0x10DE1091,	0x10DE0887,	"nVidia Tesla M2090" },
	{ 0x10DE1091,	0x10DE088E,	"nVidia Tesla X2090" },
	{ 0x10DE1091,	0x10DE0891,	"nVidia Tesla X2090" },
	{ 0x10DE1094,	0x10DE0888,	"nVidia Tesla M2075" },
	// 10A0 - 10AF
	// 10B0 - 10BF
	// 10C0 - 10CF
	{ 0x10DE10C4,	0x17AA3605,	"Lenovo ION" },
	// 10D0 - 10DF
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
	{ 0x10DE1180,	0x104383F0,	"Asus GTX680-2GD5" },
	{ 0x10DE1180,	0x104383F7,	"Asus GTX 680 Direct CU II" },
	{ 0x10DE1180,	0x10DE0969,	"nVidia GTX 680" },
	{ 0x10DE1180,	0x118010B0,	"Gainward GTX 680" },
	{ 0x10DE1180,	0x1458353C,	"GV-N680OC-2GD WindForce GTX 680 OC" },
	{ 0x10DE1180,	0x15691180,	"Palit GTX 680 JetStream" },
	{ 0x10DE1180,	0x19DA1255,	"Zotac GTX 680" },
	{ 0x10DE1180,	0x38422680,	"EVGA GTX 680" },
	{ 0x10DE1180,	0x38422682,	"EVGA GTX 680 SC" },
	{ 0x10DE1189,	0x10438405,	"Asus GTX 670 Direct CU II TOP" },
	{ 0x10DE1180,	0x10DE097A,	"nVidia GTX 670" },
	{ 0x10DE1180,	0x15691189,	"Palit GTX 670 JetStream" },
	{ 0x10DE1180,	0x19DA1255,	"Zotac GTX 670" },
	{ 0x10DE1189,	0x38422672,	"EVGA GTX 670" },
	// 1190 - 119F
	// 11A0 - 11AF
	// 11B0 - 11BF
	// 11C0 - 11CF
	// 11D0 - 11DF
	// 11E0 - 11EF
	// 11F0 - 11FF
	// 1200 - 120F
	{ 0x10DE1210,	0x10431487,	"Asus GeForce GTX 570M" },
	{ 0x10DE1210,	0x10432104,	"Asus GeForce GTX 570M" },
	{ 0x10DE1210,	0x146210BD,	"MSi GeForce GTX 570M" },
	{ 0x10DE1211,	0x10280490,	"Dell GeForce GTX 580M" },
	{ 0x10DE1247,	0x10431407,	"Asus GeForce GT 555M" },
	{ 0x10DE1247,	0x10431752,	"Asus GeForce GT 635M" },
	{ 0x10DE1247,	0x10432050,	"Asus GeForce GT 555M" },
	{ 0x10DE1247,	0x10432051,	"Asus GeForce GT 555M" },
	{ 0x10DE1247,	0x1043212A,	"Asus GeForce GT 635M" },
	{ 0x10DE1247,	0x1043212B,	"Asus GeForce GT 635M" },
	{ 0x10DE1247,	0x1043212C,	"Asus GeForce GT 635M" },
	{ 0x10DE1247,	0x14581532,	"GigaByte GeForce GT 555M" },
	{ 0x10DE1247,	0x14586744,	"GigaByte GeForce GT 555M" },
	{ 0x10DE1247,	0x152D0930,	"Quanta GeForce GT 635M" },
	{ 0x10DE1251,	0x102802A2,	"Dell GeForce GTX 560M" },
	{ 0x10DE1251,	0x102802F8,	"Dell GeForce GTX 560M" },
	{ 0x10DE1251,	0x1028048F,	"Dell GeForce GTX 560M" },
	{ 0x10DE1251,	0x10280490,	"Dell GeForce GTX 560M" },
	{ 0x10DE1251,	0x102804BA,	"Dell GeForce GTX 560M" },
	{ 0x10DE1251,	0x104313B7,	"Asus GeForce GTX 560M" },
	{ 0x10DE1251,	0x1043204A,	"Asus GeForce GTX 560M" },
	{ 0x10DE1251,	0x1043204B,	"Asus GeForce GTX 560M" },
	{ 0x10DE1251,	0x10432100,	"Asus GeForce GTX 560M" },
	{ 0x10DE1251,	0x10432101,	"Asus GeForce GTX 560M" },
	{ 0x10DE1251,	0x104384BA,	"Asus GeForce GTX 560M" },
	{ 0x10DE1251,	0x1179FC00,	"Toshiba GeForce GTX 560M" },
	{ 0x10DE1251,	0x1179FC01,	"Toshiba GeForce GTX 560M" },
	{ 0x10DE1251,	0x1179FC05,	"Toshiba GeForce GTX 560M" },
	{ 0x10DE1251,	0x146210A9,	"MSi GeForce GTX 560M" },
	{ 0x10DE1251,	0x15585102,	"Clevo/Kapok GeForce GTX 560M" },
	{ 0x10DE1251,	0x15587100,	"Clevo/Kapok GeForce GTX 560M" },
	{ 0x10DE1251,	0x15587101,	"Clevo/Kapok GeForce GTX 560M" },
	{ 0x10DE1251,	0x15587200,	"Clevo/Kapok GeForce GTX 560M" },
	{ 0x10DE1251,	0x15588000,	"Clevo/Kapok GeForce GTX 560M" },
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
    /* ------ Specific DeviceID and Generic SubDevID. ------ */
	// 0000 - 0040	
	// 0040 - 004F	
	{ 0x10DE0040,	NV_SUB_IDS,	"GeForce 6800 Ultra" },
	{ 0x10DE0041,	NV_SUB_IDS,	"GeForce 6800" },
	{ 0x10DE0042,	NV_SUB_IDS,	"GeForce 6800 LE" },
	{ 0x10DE0043,	NV_SUB_IDS,	"GeForce 6800 XE" },
	{ 0x10DE0044,	NV_SUB_IDS,	"GeForce 6800 XT" },
	{ 0x10DE0045,	NV_SUB_IDS,	"GeForce 6800 GT" },
	{ 0x10DE0046,	NV_SUB_IDS,	"GeForce 6800 GT" },
	{ 0x10DE0047,	NV_SUB_IDS,	"GeForce 6800 GS" },
	{ 0x10DE0048,	NV_SUB_IDS,	"GeForce 6800 XT" },
	{ 0x10DE004D,	NV_SUB_IDS,	"Quadro FX 3400" },
	{ 0x10DE004E,	NV_SUB_IDS,	"Quadro FX 4000" },
	// 0050 - 005F
	// 0060 - 006F
	// 0070 - 007F
	// 0080 - 008F
	// 0090 - 009F
	{ 0x10DE0090,	NV_SUB_IDS,	"GeForce 7800 GTX" },
	{ 0x10DE0091,	NV_SUB_IDS,	"GeForce 7800 GTX" },
	{ 0x10DE0092,	NV_SUB_IDS,	"GeForce 7800 GT" },
	{ 0x10DE0093,	NV_SUB_IDS,	"GeForce 7800 GS" },
	{ 0x10DE0095,	NV_SUB_IDS,	"GeForce 7800 SLI" },
	{ 0x10DE0098,	NV_SUB_IDS,	"GeForce Go 7800" },
	{ 0x10DE0099,	NV_SUB_IDS,	"GeForce Go 7800 GTX" },
	{ 0x10DE009D,	NV_SUB_IDS,	"Quadro FX 4500" },
	// 00A0 - 00AF	
	// 00B0 - 00BF	
	// 00C0 - 00CF	
	{ 0x10DE00C0,	NV_SUB_IDS,	"GeForce 6800 GS" },
	{ 0x10DE00C1,	NV_SUB_IDS,	"GeForce 6800" },
	{ 0x10DE00C2,	NV_SUB_IDS,	"GeForce 6800 LE" },
	{ 0x10DE00C3,	NV_SUB_IDS,	"GeForce 6800 XT" },
	{ 0x10DE00C8,	NV_SUB_IDS,	"GeForce Go 6800" },
	{ 0x10DE00C9,	NV_SUB_IDS,	"GeForce Go 6800 Ultra" },
	{ 0x10DE00CC,	NV_SUB_IDS,	"Quadro FX Go1400" },
	{ 0x10DE00CD,	NV_SUB_IDS,	"Quadro FX 3450/4000 SDI" },
	{ 0x10DE00CE,	NV_SUB_IDS,	"Quadro FX 1400" },
	// 00D0 - 00DF	
	// 00E0 - 00EF	
	// 00F0 - 00FF	
	{ 0x10DE00F1,	NV_SUB_IDS,	"GeForce 6600 GT" },
	{ 0x10DE00F2,	NV_SUB_IDS,	"GeForce 6600" },
	{ 0x10DE00F3,	NV_SUB_IDS,	"GeForce 6200" },
	{ 0x10DE00F4,	NV_SUB_IDS,	"GeForce 6600 LE" },
	{ 0x10DE00F5,	NV_SUB_IDS,	"GeForce 7800 GS" },
	{ 0x10DE00F6,	NV_SUB_IDS,	"GeForce 6800 GS/XT" },
	{ 0x10DE00F8,	NV_SUB_IDS,	"Quadro FX 3400/4400" },
	{ 0x10DE00F9,	NV_SUB_IDS,	"GeForce 6800 Series GPU" },
	// 0100 - 010F	
	// 0110 - 011F	
	// 0120 - 012F	
	// 0130 - 013F	
	// 0140 - 014F	
	{ 0x10DE0140,	NV_SUB_IDS,	"GeForce 6600 GT" },
	{ 0x10DE0141,	NV_SUB_IDS,	"GeForce 6600" },
	{ 0x10DE0142,	NV_SUB_IDS,	"GeForce 6600 LE" },
	{ 0x10DE0143,	NV_SUB_IDS,	"GeForce 6600 VE" },
	{ 0x10DE0144,	NV_SUB_IDS,	"GeForce Go 6600" },
	{ 0x10DE0145,	NV_SUB_IDS,	"GeForce 6610 XL" },
	{ 0x10DE0146,	NV_SUB_IDS,	"GeForce Go 6600 TE/6200 TE" },
	{ 0x10DE0147,	NV_SUB_IDS,	"GeForce 6700 XL" },
	{ 0x10DE0148,	NV_SUB_IDS,	"GeForce Go 6600" },
	{ 0x10DE0149,	NV_SUB_IDS,	"GeForce Go 6600 GT" },
	{ 0x10DE014A,	NV_SUB_IDS,	"Quadro NVS 440" },
	{ 0x10DE014C,	NV_SUB_IDS,	"Quadro FX 550" },
	{ 0x10DE014D,	NV_SUB_IDS,	"Quadro FX 550" },
	{ 0x10DE014E,	NV_SUB_IDS,	"Quadro FX 540" },
	{ 0x10DE014F,	NV_SUB_IDS,	"GeForce 6200" },
	// 0150 - 015F	
	// 0160 - 016F	
	{ 0x10DE0160,	NV_SUB_IDS,	"GeForce 6500" },
	{ 0x10DE0161,	NV_SUB_IDS,	"GeForce 6200 TurboCache(TM)" },
	{ 0x10DE0162,	NV_SUB_IDS,	"GeForce 6200SE TurboCache(TM)" },
	{ 0x10DE0163,	NV_SUB_IDS,	"GeForce 6200 LE" },
	{ 0x10DE0164,	NV_SUB_IDS,	"GeForce Go 6200" },
	{ 0x10DE0165,	NV_SUB_IDS,	"Quadro NVS 285" },
	{ 0x10DE0166,	NV_SUB_IDS,	"GeForce Go 6400" },
	{ 0x10DE0167,	NV_SUB_IDS,	"GeForce Go 6200" },
	{ 0x10DE0168,	NV_SUB_IDS,	"GeForce Go 6400" },
	{ 0x10DE0169,	NV_SUB_IDS,	"GeForce 6250" },
	{ 0x10DE016A,	NV_SUB_IDS,	"GeForce 7100 GS" },
	{ 0x10DE016C,	NV_SUB_IDS,	"NVIDIA NV44GLM" },
	{ 0x10DE016D,	NV_SUB_IDS,	"NVIDIA NV44GLM" },
	// 0170 - 017F	
	// 0180 - 018F	
	// 0190 - 019F		
	{ 0x10DE0191,	NV_SUB_IDS,	"GeForce 8800 GTX" },
	{ 0x10DE0193,	NV_SUB_IDS,	"GeForce 8800 GTS" },
	{ 0x10DE0194,	NV_SUB_IDS,	"GeForce 8800 Ultra" },
	{ 0x10DE0197,	NV_SUB_IDS,	"Tesla C870" },
	{ 0x10DE019D,	NV_SUB_IDS,	"Quadro FX 5600" },
	{ 0x10DE019E,	NV_SUB_IDS,	"Quadro FX 4600" },
	// 01A0 - 01AF
	// 01B0 - 01BF
	// 01C0 - 01CF
	// 01D0 - 01DF
	{ 0x10DE01D0,	NV_SUB_IDS,	"GeForce 7350 LE" },
	{ 0x10DE01D1,	NV_SUB_IDS,	"GeForce 7300 LE" },
	{ 0x10DE01D2,	NV_SUB_IDS,	"GeForce 7550 LE" },
	{ 0x10DE01D3,	NV_SUB_IDS,	"GeForce 7300 SE/7200 GS" },
	{ 0x10DE01D6,	NV_SUB_IDS,	"GeForce Go 7200" },
	{ 0x10DE01D7,	NV_SUB_IDS,	"Quadro NVS 110M / GeForce Go 7300" }, // 71 SubID
	{ 0x10DE01D8,	NV_SUB_IDS,	"GeForce Go 7400" },
	{ 0x10DE01D9,	NV_SUB_IDS,	"GeForce Go 7450" },
	{ 0x10DE01DA,	NV_SUB_IDS,	"Quadro NVS 110M" },
	{ 0x10DE01DB,	NV_SUB_IDS,	"Quadro NVS 120M" },
	{ 0x10DE01DC,	NV_SUB_IDS,	"Quadro FX 350M" },
	{ 0x10DE01DD,	NV_SUB_IDS,	"GeForce 7500 LE" },
	{ 0x10DE01DE,	NV_SUB_IDS,	"Quadro FX 350" },
	{ 0x10DE01DF,	NV_SUB_IDS,	"GeForce 7300 GS" },
	// 01E0 - 01EF	
	// 01F0 - 01FF
	{ 0x10DE01F0,	NV_SUB_IDS,	"GeForce4 MX" },
	// 0200 - 020F	
	// 0210 - 021F	
	{ 0x10DE0211,	NV_SUB_IDS,	"GeForce 6800" },
	{ 0x10DE0212,	NV_SUB_IDS,	"GeForce 6800 LE" },
	{ 0x10DE0215,	NV_SUB_IDS,	"GeForce 6800 GT" },
	{ 0x10DE0218,	NV_SUB_IDS,	"GeForce 6800 XT" },
	// 0220 - 022F
	{ 0x10DE0221,	NV_SUB_IDS,	"GeForce 6200" },
	{ 0x10DE0222,	NV_SUB_IDS,	"GeForce 6200 A-LE" },
	{ 0x10DE0228,	NV_SUB_IDS,	"NVIDIA NV44M" },
	// 0230 - 023F
	// 0240 - 024F
	{ 0x10DE0240,	NV_SUB_IDS,	"GeForce 6150" },
	{ 0x10DE0241,	NV_SUB_IDS,	"GeForce 6150 LE" },
	{ 0x10DE0242,	NV_SUB_IDS,	"GeForce 6100" },
	{ 0x10DE0243,	NV_SUB_IDS,	"NVIDIA C51" },
	{ 0x10DE0244,	NV_SUB_IDS,	"GeForce Go 6150" },
	{ 0x10DE0245,	NV_SUB_IDS,	"Quadro NVS 210S / GeForce 6150LE" },
	{ 0x10DE0247,	NV_SUB_IDS,	"GeForce Go 6100" },
	// 0250 - 025F
	{ 0x10DE025B,	NV_SUB_IDS,	"Quadro4 700 XGL" },
	// 0260 - 026F
	// 0270 - 027F
	// 0280 - 028F
	// 0290 - 029F
	{ 0x10DE0290,	NV_SUB_IDS,	"GeForce 7900 GTX" },
	{ 0x10DE0291,	NV_SUB_IDS,	"GeForce 7900 GT/GTO" },
	{ 0x10DE0292,	NV_SUB_IDS,	"GeForce 7900 GS" },
	{ 0x10DE0293,	NV_SUB_IDS,	"GeForce 7950 GX2" },
	{ 0x10DE0294,	NV_SUB_IDS,	"GeForce 7950 GX2" },
	{ 0x10DE0295,	NV_SUB_IDS,	"GeForce 7950 GT" },
	{ 0x10DE0298,	NV_SUB_IDS,	"GeForce Go 7900 GS" },
	{ 0x10DE0299,	NV_SUB_IDS,	"GeForce Go 7900 GTX" },
	{ 0x10DE029A,	NV_SUB_IDS,	"Quadro FX 2500M" },
	{ 0x10DE029B,	NV_SUB_IDS,	"Quadro FX 1500M" },
	{ 0x10DE029C,	NV_SUB_IDS,	"Quadro FX 5500" },
	{ 0x10DE029D,	NV_SUB_IDS,	"Quadro FX 3500" },
	{ 0x10DE029E,	NV_SUB_IDS,	"Quadro FX 1500" },
	{ 0x10DE029F,	NV_SUB_IDS,	"Quadro FX 4500 X2" },
	// 02A0 - 02AF
	// 02B0 - 02BF
	// 02C0 - 02CF
	// 02D0 - 02DF
	// 02E0 - 02EF
	{ 0x10DE02E0,	NV_SUB_IDS,	"GeForce 7600 GT" },
	{ 0x10DE02E1,	NV_SUB_IDS,	"GeForce 7600 GS" },
	{ 0x10DE02E2,	NV_SUB_IDS,	"GeForce 7300 GT" },
	{ 0x10DE02E3,	NV_SUB_IDS,	"GeForce 7900 GS" },
	{ 0x10DE02E4,	NV_SUB_IDS,	"GeForce 7950 GT" },
	// 02F0 - 02FF
	// 0300 - 030F
	{ 0x10DE0301,	NV_SUB_IDS,	"GeForce FX 5800 Ultra" },
	{ 0x10DE0302,	NV_SUB_IDS,	"GeForce FX 5800" },
	{ 0x10DE0308,	NV_SUB_IDS,	"Quadro FX 2000" },
	{ 0x10DE0309,	NV_SUB_IDS,	"Quadro FX 1000" },
	// 0310 - 031F
	{ 0x10DE0311,	NV_SUB_IDS,	"GeForce FX 5600 Ultra" },
	{ 0x10DE0312,	NV_SUB_IDS,	"GeForce FX 5600" },
	{ 0x10DE0314,	NV_SUB_IDS,	"GeForce FX 5600XT" },
	{ 0x10DE031A,	NV_SUB_IDS,	"GeForce FX Go5600" },
	{ 0x10DE031B,	NV_SUB_IDS,	"GeForce FX Go5650" },
	{ 0x10DE031C,	NV_SUB_IDS,	"Quadro FX Go700" },
	// 0320 - 032F
	{ 0x10DE0320,	NV_SUB_IDS,	"GeForce FX 5200" },
	{ 0x10DE0321,	NV_SUB_IDS,	"GeForce FX 5200 Ultra" },
	{ 0x10DE0322,	NV_SUB_IDS,	"GeForce FX 5200" },
	{ 0x10DE0323,	NV_SUB_IDS,	"GeForce FX 5200 LE" },
	{ 0x10DE0324,	NV_SUB_IDS,	"GeForce FX Go5200" },
	{ 0x10DE0325,	NV_SUB_IDS,	"GeForce FX Go5250" },
	{ 0x10DE0326,	NV_SUB_IDS,	"GeForce FX 5500" },
	{ 0x10DE0328,	NV_SUB_IDS,	"GeForce FX Go5200 32M/64M" },
	{ 0x10DE0329,	NV_SUB_IDS,	"GeForce FX Go5200" },
	{ 0x10DE032A,	NV_SUB_IDS,	"Quadro NVS 55/280 PCI" },
	{ 0x10DE032B,	NV_SUB_IDS,	"Quadro FX 500/600 PCI" },
	{ 0x10DE032C,	NV_SUB_IDS,	"GeForce FX Go53xx Series" },
	{ 0x10DE032D,	NV_SUB_IDS,	"GeForce FX Go5100" },
	//  { 0x10DE032F,	NV_SUB_IDS,	"NVIDIA NV34GL" },
	// 0330 - 033F
	{ 0x10DE0330,	NV_SUB_IDS,	"GeForce FX 5900 Ultra" },
	{ 0x10DE0331,	NV_SUB_IDS,	"GeForce FX 5900" },
	{ 0x10DE0332,	NV_SUB_IDS,	"GeForce FX 5900XT" },
	{ 0x10DE0333,	NV_SUB_IDS,	"GeForce FX 5950 Ultra" },
	{ 0x10DE0334,	NV_SUB_IDS,	"GeForce FX 5900ZT" },
	{ 0x10DE0338,	NV_SUB_IDS,	"Quadro FX 3000" },
	{ 0x10DE033F,	NV_SUB_IDS,	"Quadro FX 700" },
	// 0340 - 034F
	{ 0x10DE0341,	NV_SUB_IDS,	"GeForce FX 5700 Ultra" },
	{ 0x10DE0342,	NV_SUB_IDS,	"GeForce FX 5700" },
	{ 0x10DE0343,	NV_SUB_IDS,	"GeForce FX 5700LE" },
	{ 0x10DE0344,	NV_SUB_IDS,	"GeForce FX 5700VE" },
	//  { 0x10DE0345,	NV_SUB_IDS,	"NVIDIA NV36.5" },
	{ 0x10DE0347,	NV_SUB_IDS,	"GeForce FX Go5700" },
	{ 0x10DE0348,	NV_SUB_IDS,	"GeForce FX Go5700" },
	//  { 0x10DE0349,	NV_SUB_IDS,	"NVIDIA NV36M Pro" },
	//  { 0x10DE034B,	NV_SUB_IDS,	"NVIDIA NV36MAP" },
	{ 0x10DE034C,	NV_SUB_IDS,	"Quadro FX Go1000" },
	{ 0x10DE034E,	NV_SUB_IDS,	"Quadro FX 1100" },
	//  { 0x10DE034F,	NV_SUB_IDS,	"NVIDIA NV36GL" },
	// 0350 - 035F
	// 0360 - 036F
	// 0370 - 037F
	// 0380 - 038F
	{ 0x10DE038B,	NV_SUB_IDS,	"GeForce 7650 GS" },
	// 0390 - 039F
	{ 0x10DE0390,	NV_SUB_IDS,	"GeForce 7650 GS" },
	{ 0x10DE0391,	NV_SUB_IDS,	"GeForce 7600 GT" },
	{ 0x10DE0392,	NV_SUB_IDS,	"GeForce 7600 GS" },
	{ 0x10DE0393,	NV_SUB_IDS,	"GeForce 7300 GT" },
	{ 0x10DE0394,	NV_SUB_IDS,	"GeForce 7600 LE" },
	{ 0x10DE0395,	NV_SUB_IDS,	"GeForce 7300 GT" },
	{ 0x10DE0397,	NV_SUB_IDS,	"GeForce Go 7700" },
	{ 0x10DE0398,	NV_SUB_IDS,	"GeForce Go 7600" },
	{ 0x10DE0399,	NV_SUB_IDS,	"GeForce Go 7600 GT"},
	{ 0x10DE039A,	NV_SUB_IDS,	"Quadro NVS 300M" },
	{ 0x10DE039B,	NV_SUB_IDS,	"GeForce Go 7900 SE" },
	{ 0x10DE039C,	NV_SUB_IDS,	"Quadro FX 560M" },
	{ 0x10DE039E,	NV_SUB_IDS,	"Quadro FX 560" },
	// 03A0 - 03AF
	// 03B0 - 03BF
	// 03C0 - 03CF
	// 03D0 - 03DF
	{ 0x10DE03D0,	NV_SUB_IDS,	"GeForce 6150SE nForce 430" },
	{ 0x10DE03D1,	NV_SUB_IDS,	"GeForce 6100 nForce 405" },
	{ 0x10DE03D2,	NV_SUB_IDS,	"GeForce 6100 nForce 400" },
	{ 0x10DE03D5,	NV_SUB_IDS,	"GeForce 6100 nForce 420" },
	{ 0x10DE03D6,	NV_SUB_IDS,	"GeForce 7025 / nForce 630a" },
	// 03E0 - 03EF
	// 03F0 - 03FF
	// 0400 - 040F
	{ 0x10DE0400,	NV_SUB_IDS,	"GeForce 8600 GTS" },
	{ 0x10DE0401,	NV_SUB_IDS,	"GeForce 8600 GT" },
	{ 0x10DE0402,	NV_SUB_IDS,	"GeForce 8600 GT" },
	{ 0x10DE0403,	NV_SUB_IDS,	"GeForce 8600 GS" },
	{ 0x10DE0404,	NV_SUB_IDS,	"GeForce 8400 GS" },
	{ 0x10DE0405,	NV_SUB_IDS,	"GeForce 9500M GS" },
	{ 0x10DE0406,	NV_SUB_IDS,	"GeForce 8300 GS" },
	{ 0x10DE0407,	NV_SUB_IDS,	"GeForce 8600M GT" },
	{ 0x10DE0408,	NV_SUB_IDS,	"GeForce 9650M GS" },
	{ 0x10DE0409,	NV_SUB_IDS,	"GeForce 8700M GT" },
	{ 0x10DE040A,	NV_SUB_IDS,	"Quadro FX 370" },
	{ 0x10DE040B,	NV_SUB_IDS,	"Quadro NVS 320M" },
	{ 0x10DE040C,	NV_SUB_IDS,	"Quadro FX 570M" },
	{ 0x10DE040D,	NV_SUB_IDS,	"Quadro FX 1600M" },
	{ 0x10DE040E,	NV_SUB_IDS,	"Quadro FX 570" },
	{ 0x10DE040F,	NV_SUB_IDS,	"Quadro FX 1700" },
	// 0410 - 041F
	{ 0x10DE0410,	NV_SUB_IDS,	"GeForce GT 330" },
	// 0420 - 042F
	{ 0x10DE0420,	NV_SUB_IDS,	"GeForce 8400 SE" },
	{ 0x10DE0421,	NV_SUB_IDS,	"GeForce 8500 GT" },
	{ 0x10DE0422,	NV_SUB_IDS,	"GeForce 8400 GS" },
	{ 0x10DE0423,	NV_SUB_IDS,	"GeForce 8300 GS" },
	{ 0x10DE0424,	NV_SUB_IDS,	"GeForce 8400 GS" },
	{ 0x10DE0425,	NV_SUB_IDS,	"GeForce 8600M GS" },
	{ 0x10DE0426,	NV_SUB_IDS,	"GeForce 8400M GT" },
	{ 0x10DE0427,	NV_SUB_IDS,	"GeForce 8400M GS" },
	{ 0x10DE0428,	NV_SUB_IDS,	"GeForce 8400M G" },
	{ 0x10DE0429,	NV_SUB_IDS,	"Quadro NVS 140M" },
	{ 0x10DE042A,	NV_SUB_IDS,	"Quadro NVS 130M" },
	{ 0x10DE042B,	NV_SUB_IDS,	"Quadro NVS 135M" },
	{ 0x10DE042C,	NV_SUB_IDS,	"GeForce 9400 GT" },
	{ 0x10DE042D,	NV_SUB_IDS,	"Quadro FX 360M" },
	{ 0x10DE042E,	NV_SUB_IDS,	"GeForce 9300M G" },
	{ 0x10DE042F,	NV_SUB_IDS,	"Quadro NVS 290" },
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
	{ 0x10DE04C0,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04C1,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04C2,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04C3,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04C4,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04C5,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04C6,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04C7,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04C8,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04C9,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04CA,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04CB,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04CC,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04CD,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04CE,	NV_SUB_IDS,	"NVIDIA G78" },
	{ 0x10DE04CF,	NV_SUB_IDS,	"NVIDIA G78" },
	// 04D0 - 04DF
	// 04E0 - 04EF
	// 04F0 - 04FF
	// 0500 - 050F
	// 0510 - 051F
	// 0520 - 052F
	// 0530 - 053F
	{ 0x10DE0530,	NV_SUB_IDS,	"GeForce 7190M / nForce 650M" },
	{ 0x10DE0531,	NV_SUB_IDS,	"GeForce 7150M / nForce 630M" },
	{ 0x10DE0533,	NV_SUB_IDS,	"GeForce 7000M / nForce 610M" },
	{ 0x10DE053A,	NV_SUB_IDS,	"GeForce 7050 PV / nForce 630a" },
	{ 0x10DE053B,	NV_SUB_IDS,	"GeForce 7050 PV / nForce 630a" },
	{ 0x10DE053E,	NV_SUB_IDS,	"GeForce 7025 / nForce 630a" },
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
	{ 0x10DE05E0,	NV_SUB_IDS,	"GeForce GTX 295" },
	{ 0x10DE05E1,	NV_SUB_IDS,	"GeForce GTX 280" },
	{ 0x10DE05E2,	NV_SUB_IDS,	"GeForce GTX 260" },
	{ 0x10DE05E3,	NV_SUB_IDS,	"GeForce GTX 285" },
	{ 0x10DE05E4,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05E5,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05E6,	NV_SUB_IDS,	"GeForce GTX 275" },
	{ 0x10DE05E7,	NV_SUB_IDS,	"nVidia Tesla C1060" },
	{ 0x10DE05E8,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05E9,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05EA,	NV_SUB_IDS,	"GeForce GTX 260" },
	{ 0x10DE05EB,	NV_SUB_IDS,	"GeForce GTX 295" },
	{ 0x10DE05EC,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05ED,	NV_SUB_IDS,	"Quadroplex 2200 D2" },
	{ 0x10DE05EE,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05EF,	NV_SUB_IDS,	"NVIDIA GT200" },
	// 05F0 - 05FF
	{ 0x10DE05F0,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05F1,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05F2,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05F3,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05F4,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05F5,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05F6,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05F7,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05F8,	NV_SUB_IDS,	"Quadroplex 2200 S4" },
	{ 0x10DE05F9,	NV_SUB_IDS,	"NVIDIA Quadro CX" },
	{ 0x10DE05FA,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05FB,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05FC,	NV_SUB_IDS,	"NVIDIA GT200" },
	{ 0x10DE05FD,	NV_SUB_IDS,	"Quadro FX 5800" },
	{ 0x10DE05FE,	NV_SUB_IDS,	"Quadro FX 4800" },
	{ 0x10DE05FF,	NV_SUB_IDS,	"Quadro FX 3800" },
	// 0600 - 060F
	{ 0x10DE0600,	NV_SUB_IDS,	"GeForce 8800 GTS 512" },
	{ 0x10DE0601,	NV_SUB_IDS,	"GeForce 9800 GT" },
	{ 0x10DE0602,	NV_SUB_IDS,	"GeForce 8800 GT" },
	{ 0x10DE0603,	NV_SUB_IDS,	"GeForce GT 230" },
	{ 0x10DE0604,	NV_SUB_IDS,	"GeForce 9800 GX2" },
	{ 0x10DE0605,	NV_SUB_IDS,	"GeForce 9800 GT" },
	{ 0x10DE0606,	NV_SUB_IDS,	"GeForce 8800 GS" },
	{ 0x10DE0607,	NV_SUB_IDS,	"GeForce GTS 240" },
	{ 0x10DE0608,	NV_SUB_IDS,	"GeForce 9800M GTX" },
	{ 0x10DE0609,	NV_SUB_IDS,	"GeForce 8800M GTS" },
	{ 0x10DE060A,	NV_SUB_IDS,	"GeForce GTX 280M" },
	{ 0x10DE060B,	NV_SUB_IDS,	"GeForce 9800M GT" },
	{ 0x10DE060C,	NV_SUB_IDS,	"GeForce 8800M GTX" },
	{ 0x10DE060D,	NV_SUB_IDS,	"GeForce 8800 GS" },
	{ 0x10DE060F,	NV_SUB_IDS,	"GeForce GTX 285M" },
	// 0610 - 061F
	{ 0x10DE0610,	NV_SUB_IDS,	"GeForce 9600 GSO" },
	{ 0x10DE0611,	NV_SUB_IDS,	"GeForce 8800 GT" },
	{ 0x10DE0612,	NV_SUB_IDS,	"GeForce 9800 GTX" },
	{ 0x10DE0613,	NV_SUB_IDS,	"GeForce 9800 GTX+" },
	{ 0x10DE0614,	NV_SUB_IDS,	"GeForce 9800 GT" },
	{ 0x10DE0615,	NV_SUB_IDS,	"GeForce GTS 250" },
	{ 0x10DE0617,	NV_SUB_IDS,	"GeForce 9800M GTX" },
	{ 0x10DE0618,	NV_SUB_IDS,	"GeForce GTX 170M" },
	{ 0x10DE0619,	NV_SUB_IDS,	"Quadro FX 4700 X2" },
	{ 0x10DE061A,	NV_SUB_IDS,	"Quadro FX 3700" },
	{ 0x10DE061B,	NV_SUB_IDS,	"Quadro VX 200" },
	{ 0x10DE061C,	NV_SUB_IDS,	"Quadro FX 3600M" },
	{ 0x10DE061D,	NV_SUB_IDS,	"Quadro FX 2800M" },
	{ 0x10DE061E,	NV_SUB_IDS,	"Quadro FX 3700M" },
	{ 0x10DE061F,	NV_SUB_IDS,	"Quadro FX 3800M" },
	// 0620 - 062F
	{ 0x10DE0620,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE0621,	NV_SUB_IDS,	"GeForce GT 230" },
	{ 0x10DE0622,	NV_SUB_IDS,	"GeForce 9600 GT" },
	{ 0x10DE0623,	NV_SUB_IDS,	"GeForce 9600 GS" },
	{ 0x10DE0624,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE0625,	NV_SUB_IDS,	"GeForce 9600 GSO 512"},
	{ 0x10DE0626,	NV_SUB_IDS,	"GeForce GT 130" },
	{ 0x10DE0627,	NV_SUB_IDS,	"GeForce GT 140" },
	{ 0x10DE0628,	NV_SUB_IDS,	"GeForce 9800M GTS" },
	{ 0x10DE0629,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE062A,	NV_SUB_IDS,	"GeForce 9700M GTS" },
	{ 0x10DE062B,	NV_SUB_IDS,	"GeForce 9800M GS" },
	{ 0x10DE062C,	NV_SUB_IDS,	"GeForce 9800M GTS" },
	{ 0x10DE062D,	NV_SUB_IDS,	"GeForce 9600 GT" },
	{ 0x10DE062E,	NV_SUB_IDS,	"GeForce 9600 GT" },
	{ 0x10DE062F,	NV_SUB_IDS,	"GeForce 9800 S" },
	// 0630 - 063F
	{ 0x10DE0630,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE0631,	NV_SUB_IDS,	"GeForce GTS 160M" },
	{ 0x10DE0632,	NV_SUB_IDS,	"GeForce GTS 150M" },
	{ 0x10DE0633,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE0634,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE0635,	NV_SUB_IDS,	"GeForce 9600 GSO" },
	{ 0x10DE0636,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE0637,	NV_SUB_IDS,	"GeForce 9600 GT" },
	{ 0x10DE0638,	NV_SUB_IDS,	"Quadro FX 1800" },
	{ 0x10DE0639,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE063A,	NV_SUB_IDS,	"Quadro FX 2700M" },
	{ 0x10DE063B,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE063C,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE063D,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE063E,	NV_SUB_IDS,	"NVIDIA G94" },
	{ 0x10DE063F,	NV_SUB_IDS,	"NVIDIA G94" },
	// 0640 - 064F
	{ 0x10DE0640,	NV_SUB_IDS,	"GeForce 9500 GT" },
	{ 0x10DE0641,	NV_SUB_IDS,	"GeForce 9400 GT" },
	{ 0x10DE0642,	NV_SUB_IDS,	"GeForce 8400 GS" },
	{ 0x10DE0643,	NV_SUB_IDS,	"GeForce 9500 GT" },
	{ 0x10DE0644,	NV_SUB_IDS,	"GeForce 9500 GS" },
	{ 0x10DE0645,	NV_SUB_IDS,	"GeForce 9500 GS" },
	{ 0x10DE0646,	NV_SUB_IDS,	"GeForce GT 120" },
	{ 0x10DE0647,	NV_SUB_IDS,	"GeForce 9600M GT" },
	{ 0x10DE0648,	NV_SUB_IDS,	"GeForce 9600M GS" },
	{ 0x10DE0649,	NV_SUB_IDS,	"GeForce 9600M GT" },
	{ 0x10DE064A,	NV_SUB_IDS,	"GeForce 9700M GT" },
	{ 0x10DE064B,	NV_SUB_IDS,	"GeForce 9500M G" },
	{ 0x10DE064C,	NV_SUB_IDS,	"GeForce 9650M GT" },
	// 0650 - 065F
	{ 0x10DE0650,	NV_SUB_IDS,	"NVIDIA G96-825" },
	{ 0x10DE0651,	NV_SUB_IDS,	"GeForce G 110M" },
	{ 0x10DE0652,	NV_SUB_IDS,	"GeForce GT 130M" },
	{ 0x10DE0653,	NV_SUB_IDS,	"GeForce GT 120M" },
	{ 0x10DE0654,	NV_SUB_IDS,	"GeForce GT 220M" },
	{ 0x10DE0655,	NV_SUB_IDS,	"GeForce GT 120" },
	{ 0x10DE0656,	NV_SUB_IDS,	"GeForce 9650 S" },
	{ 0x10DE0657,	NV_SUB_IDS,	"NVIDIA G96" },
	{ 0x10DE0658,	NV_SUB_IDS,	"Quadro FX 380" },
	{ 0x10DE0659,	NV_SUB_IDS,	"Quadro FX 580" },
	{ 0x10DE065A,	NV_SUB_IDS,	"Quadro FX 1700M" },
	{ 0x10DE065B,	NV_SUB_IDS,	"GeForce 9400 GT" },
	{ 0x10DE065C,	NV_SUB_IDS,	"Quadro FX 770M" },
	{ 0x10DE065D,	NV_SUB_IDS,	"NVIDIA G96" },
	{ 0x10DE065E,	NV_SUB_IDS,	"NVIDIA G96" },
	{ 0x10DE065F,	NV_SUB_IDS,	"GeForce G210" },
	// 0660 - 066F
	// 0670 - 067F
	// 0680 - 068F
	// 0690 - 069F
	// 06A0 - 06AF
	{ 0x10DE06A0,	NV_SUB_IDS,	"NVIDIA GT214" },
	// 06B0 - 06BF
	{ 0x10DE06B0,	NV_SUB_IDS,	"NVIDIA GT214" },
	// 06C0 - 06CF
	{ 0x10DE06C0,	NV_SUB_IDS,	"GeForce GTX 480" },
	{ 0x10DE06C3,	NV_SUB_IDS,	"GeForce GTX D12U" },
	{ 0x10DE06C4,	NV_SUB_IDS,	"GeForce GTX 465" },
	{ 0x10DE06CA,	NV_SUB_IDS,	"GeForce GTX 480M" },
	{ 0x10DE06CD,	NV_SUB_IDS,	"GeForce GTX 470" },
	// 06D0 - 06DF
	{ 0x10DE06D1,	NV_SUB_IDS,	"Tesla C2050 / C2070" },
	{ 0x10DE06D2,	NV_SUB_IDS,	"Tesla M2070 / X2070" },
	{ 0x10DE06D8,	NV_SUB_IDS,	"Quadro 6000" },
	{ 0x10DE06D9,	NV_SUB_IDS,	"Quadro 5000" },
	{ 0x10DE06DC,	NV_SUB_IDS,	"Quadro 6000" },
	{ 0x10DE06DD,	NV_SUB_IDS,	"nVidia Quadro 4000" },
	{ 0x10DE06DE,	NV_SUB_IDS,	"nVidia Tesla S2050" },
	{ 0x10DE06DF,	NV_SUB_IDS,	"Tesla M2070Q" },
	// 06E0 - 06EF
	{ 0x10DE06E0,	NV_SUB_IDS,	"GeForce 9300 GE" },
	{ 0x10DE06E1,	NV_SUB_IDS,	"GeForce 9300 GS" },
	{ 0x10DE06E2,	NV_SUB_IDS,	"GeForce 8400" },
	{ 0x10DE06E3,	NV_SUB_IDS,	"GeForce 8400 SE" },
	{ 0x10DE06E4,	NV_SUB_IDS,	"GeForce 8400 GS" },
	{ 0x10DE06E5,	NV_SUB_IDS,	"GeForce 9300M GS" },
	{ 0x10DE06E6,	NV_SUB_IDS,	"GeForce G100" },
	{ 0x10DE06E7,	NV_SUB_IDS,	"GeForce 9300 SE" },
	{ 0x10DE06E8,	NV_SUB_IDS,	"GeForce 9200M GS" },
	{ 0x10DE06E9,	NV_SUB_IDS,	"GeForce 9300M GS" },
	{ 0x10DE06EA,	NV_SUB_IDS,	"Quadro NVS 150M" },
	{ 0x10DE06EB,	NV_SUB_IDS,	"Quadro NVS 160M" },
	{ 0x10DE06EC,	NV_SUB_IDS,	"GeForce G 105M" },
	{ 0x10DE06ED,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06EF,	NV_SUB_IDS,	"GeForce G 103M" },
	// 06F0 - 06FF
	{ 0x10DE06F0,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06F1,	NV_SUB_IDS,	"GeForce G105M" },
	{ 0x10DE06F2,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06F3,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06F4,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06F5,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06F6,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06F7,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06F8,	NV_SUB_IDS,	"Quadro NVS 420" },
	{ 0x10DE06F9,	NV_SUB_IDS,	"Quadro FX 370 LP" },
	{ 0x10DE06FA,	NV_SUB_IDS,	"Quadro NVS 450" },
	{ 0x10DE06FB,	NV_SUB_IDS,	"Quadro FX 370M" },
	{ 0x10DE06FC,	NV_SUB_IDS,	"NVIDIA G98" },
	{ 0x10DE06FD,	NV_SUB_IDS,	"Quadro NVS 295" },
	{ 0x10DE06FE,	NV_SUB_IDS,	"NVIDIA G98" },
	// { 0x10DE06FF,	NV_SUB_IDS,	"HICx16 + Graphics" },
	// 0700 - 070F
	// 0710 - 071F
	// 0720 - 072F
	// 0730 - 073F
	// 0740 - 074F
	// 0750 - 075F
	// 0760 - 076F
	// 0770 - 077F
	// 0780 - 078F
	// 0790 - 079F
	// 07A0 - 07AF
	// 07B0 - 07BF
	// 07C0 - 07CF
	// 07D0 - 07DF
	// 07E0 - 07EF
	{ 0x10DE07E0,	NV_SUB_IDS,	"GeForce 7150 / nForce 630i" },
	{ 0x10DE07E1,	NV_SUB_IDS,	"GeForce 7100 / nForce 630i" },
	{ 0x10DE07E2,	NV_SUB_IDS,	"GeForce 7050 / nForce 630i" },
	{ 0x10DE07E3,	NV_SUB_IDS,	"GeForce 7050 / nForce 610i" },
	{ 0x10DE07E5,	NV_SUB_IDS,	"GeForce 7050 / nForce 620i" },
	// 07F0 - 07FF
	// 0800 - 080F
	// 0810 - 081F
	// 0820 - 082F
	// 0830 - 083F
	// 0840 - 084F
	{ 0x10DE0840,	NV_SUB_IDS,	"GeForce 8200M" },
	{ 0x10DE0844,	NV_SUB_IDS,	"GeForce 9100M G" },
	{ 0x10DE0845,	NV_SUB_IDS,	"GeForce 8200M G" },
	{ 0x10DE0846,	NV_SUB_IDS,	"GeForce 9200" },
	{ 0x10DE0847,	NV_SUB_IDS,	"GeForce 9100" },
	{ 0x10DE0848,	NV_SUB_IDS,	"GeForce 8300" },
	{ 0x10DE0849,	NV_SUB_IDS,	"GeForce 8200" },
	{ 0x10DE084A,	NV_SUB_IDS,	"nForce 730a" },
	{ 0x10DE084B,	NV_SUB_IDS,	"GeForce 9200" },
	{ 0x10DE084C,	NV_SUB_IDS,	"nForce 980a/780a SLI" },
	{ 0x10DE084D,	NV_SUB_IDS,	"nForce 750a SLI" },
	{ 0x10DE084F,	NV_SUB_IDS,	"GeForce 8100 / nForce 720a" },
	// 0850 - 085F
	// 0860 - 086F
	{ 0x10DE0860,	NV_SUB_IDS,	"GeForce 9300" },
	{ 0x10DE0861,	NV_SUB_IDS,	"GeForce 9400" },
	{ 0x10DE0862,	NV_SUB_IDS,	"GeForce 9400M G" },
	{ 0x10DE0863,	NV_SUB_IDS,	"GeForce 9400M" },
	{ 0x10DE0864,	NV_SUB_IDS,	"GeForce 9300" },
	{ 0x10DE0865,	NV_SUB_IDS,	"GeForce 9300" },
	{ 0x10DE0866,	NV_SUB_IDS,	"GeForce 9400M G" },
	{ 0x10DE0867,	NV_SUB_IDS,	"GeForce 9400" },
	{ 0x10DE0868,	NV_SUB_IDS,	"nForce 760i SLI" },
	{ 0x10DE0869,	NV_SUB_IDS,	"GeForce 9400" },
	{ 0x10DE086A,	NV_SUB_IDS,	"GeForce 9400" },
	{ 0x10DE086C,	NV_SUB_IDS,	"GeForce 9300 / nForce 730i" },
	{ 0x10DE086D,	NV_SUB_IDS,	"GeForce 9200" },
	{ 0x10DE086E,	NV_SUB_IDS,	"GeForce 9100M G" },
	{ 0x10DE086F,	NV_SUB_IDS,	"GeForce 8200M G" },
	// 0870 - 087F
	{ 0x10DE0870,	NV_SUB_IDS,	"GeForce 9400M" },
	{ 0x10DE0871,	NV_SUB_IDS,	"GeForce 9200" },
	{ 0x10DE0872,	NV_SUB_IDS,	"GeForce G102M" },
	{ 0x10DE0873,	NV_SUB_IDS,	"GeForce G102M" },
	{ 0x10DE0874,	NV_SUB_IDS,	"ION 9300M" },
	{ 0x10DE0876,	NV_SUB_IDS,	"ION 9400M" },
	{ 0x10DE087A,	NV_SUB_IDS,	"GeForce 9400" },
	{ 0x10DE087D,	NV_SUB_IDS,	"ION 9400M" },
	{ 0x10DE087E,	NV_SUB_IDS,	"ION LE" },
	{ 0x10DE087F,	NV_SUB_IDS,	"ION LE" },
	// 0880 - 088F
	// 0890 - 089F
	// 08A0 - 08AF
	{ 0x10DE08A0,	NV_SUB_IDS,	"GeForce 320M" },
	// { 0x10DE08A1,	NV_SUB_IDS,	"NVIDIA MCP89-MZT" },
	// { 0x10DE08A2,	NV_SUB_IDS,	"NVIDIA MCP89-EPT" },
	{ 0x10DE08A3,	NV_SUB_IDS,	"GeForce 320M" },
	{ 0x10DE08A4,	NV_SUB_IDS,	"GeForce 320M" },
	{ 0x10DE08A5,	NV_SUB_IDS,	"GeForce 320M" },
	// 08B0 - 08BF
	// { 0x10DE08B0,	NV_SUB_IDS,	"MCP83 MMD" },
	{ 0x10DE08B1,	NV_SUB_IDS,	"GeForce 300M" },
	// { 0x10DE08B2,	NV_SUB_IDS,	"NVIDIA MCP83-MJ" },
	// { 0x10DE08B3,	NV_SUB_IDS,	"NVIDIA MCP89 MM9" },
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
	// { 0x10DE0A00,	NV_SUB_IDS,	"NVIDIA GT212" },
	// 0A10 - 0A1F
	// { 0x10DE0A10,	NV_SUB_IDS,	"NVIDIA GT212" },
	// 0A20 - 0A2F
	{ 0x10DE0A20,	NV_SUB_IDS,	"GeForce GT 220" },
	// { 0x10DE0A21,	NV_SUB_IDS,	"NVIDIA D10M2-20" },
	{ 0x10DE0A22,	NV_SUB_IDS,	"GeForce 315" },
	{ 0x10DE0A23,	NV_SUB_IDS,	"GeForce 210" },
	{ 0x10DE0A26,	NV_SUB_IDS,	"GeForce 405" },
	{ 0x10DE0A27,	NV_SUB_IDS,	"GeForce 405" },
	{ 0x10DE0A28,	NV_SUB_IDS,	"GeForce GT 230M" },
	{ 0x10DE0A29,	NV_SUB_IDS,	"GeForce GT 330M" },
	{ 0x10DE0A2A,	NV_SUB_IDS,	"GeForce GT 230M" },
	{ 0x10DE0A2B,	NV_SUB_IDS,	"GeForce GT 330M" },
	{ 0x10DE0A2C,	NV_SUB_IDS,	"NVS 5100M" },
	{ 0x10DE0A2D,	NV_SUB_IDS,	"GeForce GT 320M" },	
	// 0A30 - 0A3F
	// { 0x10DE0A30,	NV_SUB_IDS,	"NVIDIA GT216" },
	{ 0x10DE0A32,	NV_SUB_IDS,	"GeForce GT 415" },
	{ 0x10DE0A34,	NV_SUB_IDS,	"GeForce GT 240M" },
	{ 0x10DE0A35,	NV_SUB_IDS,	"GeForce GT 325M" },
	{ 0x10DE0A38,	NV_SUB_IDS,	"Quadro 400" },
	{ 0x10DE0A3C,	NV_SUB_IDS,	"Quadro FX 880M" },
	// { 0x10DE0A3D,	NV_SUB_IDS,	"NVIDIA N10P-ES" },
	// { 0x10DE0A3F,	NV_SUB_IDS,	"NVIDIA GT216-INT" },
	// 0A40 - 0A4F
	// 0A50 - 0A5F
	// 0A60 - 0A6F
	{ 0x10DE0A60,	NV_SUB_IDS,	"GeForce G210" },
	// { 0x10DE0A61,	NV_SUB_IDS,	"NVIDIA NVS 2100" },
	{ 0x10DE0A62,	NV_SUB_IDS,	"GeForce 205" },
	{ 0x10DE0A63,	NV_SUB_IDS,	"GeForce 310" },
	{ 0x10DE0A64,	NV_SUB_IDS,	"ION" },
	{ 0x10DE0A65,	NV_SUB_IDS,	"GeForce 210" },
	{ 0x10DE0A66,	NV_SUB_IDS,	"GeForce 310" },
	{ 0x10DE0A67,	NV_SUB_IDS,	"GeForce 315" },
	{ 0x10DE0A68,	NV_SUB_IDS,	"GeForce G105M" },
	{ 0x10DE0A69,	NV_SUB_IDS,	"GeForce G105M" },
	{ 0x10DE0A6A,	NV_SUB_IDS,	"NVS 2100M" },
	{ 0x10DE0A6C,	NV_SUB_IDS,	"NVS 3100M" },
	{ 0x10DE0A6E,	NV_SUB_IDS,	"GeForce 305M" },
	{ 0x10DE0A6F,	NV_SUB_IDS,	"ION" },	
	// 0A70 - 0A7F
	{ 0x10DE0A70,	NV_SUB_IDS,	"GeForce 310M" },
	{ 0x10DE0A71,	NV_SUB_IDS,	"GeForce 305M" },
	{ 0x10DE0A72,	NV_SUB_IDS,	"GeForce 310M" },
	{ 0x10DE0A73,	NV_SUB_IDS,	"GeForce 305M" },
	{ 0x10DE0A74,	NV_SUB_IDS,	"GeForce G210M" },
	{ 0x10DE0A75,	NV_SUB_IDS,	"GeForce G310M" },
	{ 0x10DE0A76,	NV_SUB_IDS,	"ION" },
	{ 0x10DE0A78,	NV_SUB_IDS,	"Quadro FX 380 LP" },
	// { 0x10DE0A79,	NV_SUB_IDS,	"N12M-NS-S" },
	{ 0x10DE0A7A,	NV_SUB_IDS,	"GeForce 315M" },
	{ 0x10DE0A7B,	NV_SUB_IDS,	"GeForce 505" },
	{ 0x10DE0A7C,	NV_SUB_IDS,	"Quadro FX 380M" },
	// { 0x10DE0A7D,	NV_SUB_IDS,	"NVIDIA GT218-ES" },
	// { 0x10DE0A7E,	NV_SUB_IDS,	"NVIDIA GT218-INT-S" },
	// { 0x10DE0A7F,	NV_SUB_IDS,	"NVIDIA GT218-INT-B" },
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
	{ 0x10DE0CA0,	NV_SUB_IDS,	"GeForce GT 330 " },
	{ 0x10DE0CA2,	NV_SUB_IDS,	"GeForce GT 320" },
	{ 0x10DE0CA3,	NV_SUB_IDS,	"GeForce GT 240" },
	{ 0x10DE0CA4,	NV_SUB_IDS,	"GeForce GT 340" },
	{ 0x10DE0CA5,	NV_SUB_IDS,	"GeForce GT 220" },
	{ 0x10DE0CA7,	NV_SUB_IDS,	"GeForce GT 330" },
	{ 0x10DE0CA8,	NV_SUB_IDS,	"GeForce GTS 260M" },
	{ 0x10DE0CA9,	NV_SUB_IDS,	"GeForce GTS 250M" },
	{ 0x10DE0CAC,	NV_SUB_IDS,	"GeForce GT 220" },
	//  { 0x10DE0CAD,	NV_SUB_IDS,	"NVIDIA N10E-ES" },
	//  { 0x10DE0CAE,	NV_SUB_IDS,	"NVIDIA GT215-INT" },
	{ 0x10DE0CAF,	NV_SUB_IDS,	"GeForce GT 335M" },
	// 0CB0 - 0CBF	
	{ 0x10DE0CB0,	NV_SUB_IDS,	"GeForce GTS 350M" },
	{ 0x10DE0CB1,	NV_SUB_IDS,	"GeForce GTS 360M" },
	{ 0x10DE0CBC,	NV_SUB_IDS,	"Quadro FX 1800M" },
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
	{ 0x10DE0DC0,	NV_SUB_IDS,	"GeForce GT 440" },
	//  { 0x10DE0DC1,	NV_SUB_IDS,	"D12-P1-35" },
	//  { 0x10DE0DC2,	NV_SUB_IDS,	"D12-P1-35" },
	{ 0x10DE0DC4,	NV_SUB_IDS,	"GeForce GTS 450" },
	{ 0x10DE0DC5,	NV_SUB_IDS,	"GeForce GTS 450" },
	{ 0x10DE0DC6,	NV_SUB_IDS,	"GeForce GTS 450" },
	//  { 0x10DE0DCA,	NV_SUB_IDS,	"GF10x" },
	//  { 0x10DE0DCC,	NV_SUB_IDS,	"N12E-GS" },
	{ 0x10DE0DCD,	NV_SUB_IDS,	"GeForce GT 555M" },
	{ 0x10DE0DCE,	NV_SUB_IDS,	"GeForce GT 555M" },
	//  { 0x10DE0DCF,	NV_SUB_IDS,	"N12P-GT-B" },
	// 0DD0 - 0DDF	
	//  { 0x10DE0DD0,	NV_SUB_IDS,	"N11E-GT" },
	{ 0x10DE0DD1,	NV_SUB_IDS,	"GeForce GTX 460M" },
	{ 0x10DE0DD2,	NV_SUB_IDS,	"GeForce GT 445M" },
	{ 0x10DE0DD3,	NV_SUB_IDS,	"GeForce GT 435M" },
	{ 0x10DE0DD6,	NV_SUB_IDS,	"GeForce GT 550M" },
	{ 0x10DE0DD8,	NV_SUB_IDS,	"Quadro 2000" },
	{ 0x10DE0DDA,	NV_SUB_IDS,	"Quadro 2000M" },
	//  { 0x10DE0DDE,	NV_SUB_IDS,	"GF106-ES" },
	//  { 0x10DE0DDF,	NV_SUB_IDS,	"GF106-INT" },
	// 0DE0 - 0DEF
	{ 0x10DE0DE0,	NV_SUB_IDS,	"GeForce GT 440" },
	{ 0x10DE0DE1,	NV_SUB_IDS,	"GeForce GT 430" },
	{ 0x10DE0DE2,	NV_SUB_IDS,	"GeForce GT 420" },
	{ 0x10DE0DE4,	NV_SUB_IDS,	"GeForce GT 520" },
	{ 0x10DE0DE5,	NV_SUB_IDS,	"GeForce GT 530" },
	{ 0x10DE0DE8,	NV_SUB_IDS,	"GeForce GT 620M" },
	{ 0x10DE0DE9,	NV_SUB_IDS,	"GeForce GT 630M" },
	{ 0x10DE0DEA,	NV_SUB_IDS,	"GeForce GT 610M" },
	{ 0x10DE0DEB,	NV_SUB_IDS,	"GeForce GT 555M" },
	{ 0x10DE0DEC,	NV_SUB_IDS,	"GeForce GT 525M" },
	{ 0x10DE0DED,	NV_SUB_IDS,	"GeForce GT 520M" },
	{ 0x10DE0DEE,	NV_SUB_IDS,	"GeForce GT 415M" },
	//  { 0x10DE0DEF,	NV_SUB_IDS,	"N13P-NS1-A1" },
	// 0DF0 - 0DFF
	{ 0x10DE0DF0,	NV_SUB_IDS,	"GeForce GT 425M" },
	{ 0x10DE0DF1,	NV_SUB_IDS,	"GeForce GT 420M" },
	{ 0x10DE0DF2,	NV_SUB_IDS,	"GeForce GT 435M" },
	{ 0x10DE0DF3,	NV_SUB_IDS,	"GeForce GT 420M" },
	{ 0x10DE0DF4,	NV_SUB_IDS,	"GeForce GT 540M" },
	{ 0x10DE0DF5,	NV_SUB_IDS,	"GeForce GT 525M" },
	{ 0x10DE0DF6,	NV_SUB_IDS,	"GeForce GT 550M" },
	{ 0x10DE0DF7,	NV_SUB_IDS,	"GeForce GT 520M" },
	{ 0x10DE0DF8,	NV_SUB_IDS,	"Quadro 600" },
	{ 0x10DE0DF9,	NV_SUB_IDS,	"Quadro 500M" },
	{ 0x10DE0DFA,	NV_SUB_IDS,	"Quadro 1000M" },
	{ 0x10DE0DFC,	NV_SUB_IDS,	"NVS 5200M" },
	//  { 0x10DE0DFE,	NV_SUB_IDS,	"GF108 ES" },
	//  { 0x10DE0DFF,	NV_SUB_IDS,	"GF108 INT" },
	// 0E00 - 0E0F
	// 0E10 - 0E1F
	// 0E20 - 0E2F
	{ 0x10DE0E21,	NV_SUB_IDS,	"D12U-25" },
	{ 0x10DE0E22,	NV_SUB_IDS,	"GeForce GTX 460" },
	{ 0x10DE0E23,	NV_SUB_IDS,	"GeForce GTX 460 SE" },
	{ 0x10DE0E24,	NV_SUB_IDS,	"GeForce GTX 460" },
	//  { 0x10DE0E25,	NV_SUB_IDS,	"D12U-50" },
	{ 0x10DE0E28,	NV_SUB_IDS,	"GeForce GTX 460" },
	// 0E30 - 0E3F
	{ 0x10DE0E30,	NV_SUB_IDS,	"GeForce GTX 470M" },
	{ 0x10DE0E31,	NV_SUB_IDS,	"GeForce GTX 485M" },
	//  { 0x10DE0E32,	NV_SUB_IDS,	"N12E-GT" },
	{ 0x10DE0E38,	NV_SUB_IDS,	"GF104GL" },
	{ 0x10DE0E3A,	NV_SUB_IDS,	"Quadro 3000M" },
	{ 0x10DE0E3B,	NV_SUB_IDS,	"Quadro 4000M" },
	//  { 0x10DE0E3E,	NV_SUB_IDS,	"GF104-ES" },
	//  { 0x10DE0E3F,	NV_SUB_IDS,	"GF104-INT" },
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
	{ 0x10DE0FD1,	NV_SUB_IDS,	"GeForce GT 650M" },
	{ 0x10DE0FD2,	NV_SUB_IDS,	"GeForce GT 640M" },
	{ 0x10DE0FD4,	NV_SUB_IDS,	"GeForce GTX 660M" },
	//  { 0x10DE0FDB,	NV_SUB_IDS,	"GK107-ESP-A1" },
	// 0FE0 - 0FEF
	// 0FF0 - 0FFF
	// 1000 - 100F
	// 1010 - 101F
	// 1020 - 102F
	// 1030 - 103F
	// 1040 - 104F
	{ 0x10DE1040,	NV_SUB_IDS,	"GeForce GT 520" },
	//  { 0x10DE1041,	NV_SUB_IDS,	"D13M1-45" },
	{ 0x10DE1042,	NV_SUB_IDS,	"GeForce 510" },
	{ 0x10DE1048,	NV_SUB_IDS,	"GeForce 605" },
	{ 0x10DE1049,	NV_SUB_IDS,	"GeForce GT 620" },
	// 1050 - 105F
	{ 0x10DE1050,	NV_SUB_IDS,	"GeForce GT 520M" },
	{ 0x10DE1051,	NV_SUB_IDS,	"GeForce GT 520MX" },
	{ 0x10DE1052,	NV_SUB_IDS,	"GeForce GT 520M" },
	{ 0x10DE1054,	NV_SUB_IDS,	"GeForce GT 410M" },
	{ 0x10DE1055,	NV_SUB_IDS,	"GeForce 410M" },
	{ 0x10DE1056,	NV_SUB_IDS,	"Quadro NVS 4200M" },
	{ 0x10DE1057,	NV_SUB_IDS,	"Quadro NVS 4200M" },
	{ 0x10DE1058,	NV_SUB_IDS,	"GeForce 610M" },
	{ 0x10DE1059,	NV_SUB_IDS,	"GeForce 610M" },
	{ 0x10DE105A,	NV_SUB_IDS,	"GeForce 610M" },
	// 1060 - 106F
	// 1070 - 107F
	//  { 0x10DE107D,	NV_SUB_IDS,	"GF119" },
	//  { 0x10DE107E,	NV_SUB_IDS,	"GF119-INT" },
	//  { 0x10DE107F,	NV_SUB_IDS,	"GF119-ES" },
	// 1080 - 108F
	{ 0x10DE1080,	NV_SUB_IDS,	"GeForce GTX 580" },
	{ 0x10DE1081,	NV_SUB_IDS,	"GeForce GTX 570" },
	{ 0x10DE1082,	NV_SUB_IDS,	"GeForce GTX 560 Ti" },
	{ 0x10DE1083,	NV_SUB_IDS,	"D13U" },
	{ 0x10DE1084,	NV_SUB_IDS,	"GeForce GTX 560" },
	{ 0x10DE1086,	NV_SUB_IDS,	"GeForce GTX 570" },
	{ 0x10DE1087,	NV_SUB_IDS,	"GeForce GTX 560 Ti-448" },
	{ 0x10DE1088,	NV_SUB_IDS,	"GeForce GTX 590" },
	{ 0x10DE1089,	NV_SUB_IDS,	"GeForce GTX 580" },
	{ 0x10DE108B,	NV_SUB_IDS,	"GeForce GTX 590" },
	//  { 0x10DE108C,	NV_SUB_IDS,	"D13U" },
	{ 0x10DE108E,	NV_SUB_IDS,	"Tesla C2090" },
	// 1090 - 109F
	{ 0x10DE1091,	NV_SUB_IDS,	"nVidia Tesla M2090" },
	{ 0x10DE1094,	NV_SUB_IDS,	"Tesla M2075 Dual-Slot Computing Processor Module" },
	{ 0x10DE1096,	NV_SUB_IDS,	"Tesla C2075" },
	//  { 0x10DE1098,	NV_SUB_IDS,	"D13U" },
	{ 0x10DE109A,	NV_SUB_IDS,	"Quadro 5010M" },
	{ 0x10DE109B,	NV_SUB_IDS,	"Quadro 7000" },
	// 10A0 - 10AF
	// 10B0 - 10BF
	// 10C0 - 10CF
	{ 0x10DE10C0,	NV_SUB_IDS,	"GeForce 9300 GS" },
	{ 0x10DE10C3,	NV_SUB_IDS,	"GeForce 8400 GS" },
	{ 0x10DE10C4,	NV_SUB_IDS,	"NVIDIA ION" },
	{ 0x10DE10C5,	NV_SUB_IDS,	"GeForce 405" },
	// 10D0 - 10DF
	{ 0x10DE10D8,	NV_SUB_IDS,	"NVS 300" },
	// 10E0 - 10EF
	// 10F0 - 10FF
	// 1100 - 110F
	// 1110 - 111F
	// 1120 - 112F
	// 1130 - 113F
	// 1140 - 114F
	//  { 0x10DE1140,	NV_SUB_IDS,	"GF117" },
	{ 0x10DE1141,	NV_SUB_IDS,	"GeForce 610M" },
	{ 0x10DE1142,	NV_SUB_IDS,	"GeForce 620M" },
	//  { 0x10DE1143,	NV_SUB_IDS,	"N13P-GV" },
	//  { 0x10DE1144,	NV_SUB_IDS,	"GF117" },
	//  { 0x10DE1145,	NV_SUB_IDS,	"GF117" },
	//  { 0x10DE1146,	NV_SUB_IDS,	"GF117" },
	//  { 0x10DE1147,	NV_SUB_IDS,	"GF117" },
	//  { 0x10DE1149,	NV_SUB_IDS,	"GF117-ES" },
	//  { 0x10DE114A,	NV_SUB_IDS,	"GF117-INT" },
	//  { 0x10DE114B,	NV_SUB_IDS,	"PCI-GEN3-B" },
	// 1150 - 115F
	// 1160 - 116F
	// 1170 - 117F
	// 1180 - 118F
	{ 0x10DE1180,	NV_SUB_IDS,	"GeForce GTX 680" },
	{ 0x10DE1188,	NV_SUB_IDS,	"GeForce GTX 690" },
	{ 0x10DE1189,	NV_SUB_IDS,	"GeForce GTX 670" },
	// 1190 - 119F
	// 11A0 - 11AF
	// 11B0 - 11BF
	// 11C0 - 11CF
	// 11D0 - 11DF
	// 11E0 - 11EF
	// 11F0 - 11FF
	// 1200 - 120F
	{ 0x10DE1200,	NV_SUB_IDS,	"GeForce GTX 560 Ti" },
	{ 0x10DE1201,	NV_SUB_IDS,	"GeForce GTX 560" },
	{ 0x10DE1202,	NV_SUB_IDS,	"GeForce GTX 560 Ti" },
	{ 0x10DE1203,	NV_SUB_IDS,	"GeForce GTX 460 SE v2" },
	{ 0x10DE1205,	NV_SUB_IDS,	"GeForce GTX 460 v2" },
	{ 0x10DE1206,	NV_SUB_IDS,	"GeForce GTX 555" },
	{ 0x10DE1208,	NV_SUB_IDS,	"GeForce GTX 560 SE" },
	{ 0x10DE1210,	NV_SUB_IDS,	"GeForce GTX 570M" },
	{ 0x10DE1211,	NV_SUB_IDS,	"GeForce GTX 580M" },
	{ 0x10DE1212,	NV_SUB_IDS,	"GeForce GTX 675M" },
	{ 0x10DE1213,	NV_SUB_IDS,	"GeForce GTX 670M" },
	{ 0x10DE1240,	NV_SUB_IDS,	"GeForce GT 620M" },
	{ 0x10DE1241,	NV_SUB_IDS,	"GeForce GT 545" },
	{ 0x10DE1243,	NV_SUB_IDS,	"GeForce GT 545" },
	{ 0x10DE1244,	NV_SUB_IDS,	"GeForce GTX 550 Ti" },
	{ 0x10DE1245,	NV_SUB_IDS,	"GeForce GTS 450" },
	{ 0x10DE1246,	NV_SUB_IDS,	"GeForce GTX 550M" },
	{ 0x10DE1247,	NV_SUB_IDS,	"GeForce GT 635M" },
	{ 0x10DE1248,	NV_SUB_IDS,	"GeForce GTX 555M" },
	{ 0x10DE124B,	NV_SUB_IDS,	"GeForce GT 640" },
	{ 0x10DE124D,	NV_SUB_IDS,	"GeForce GTX 555M" },
	//  { 0x10DE1250,	NV_SUB_IDS,	"GF116-INT" },
	{ 0x10DE1251,	NV_SUB_IDS,	"GeForce GTX 560M" },
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
};

static int patch_nvidia_rom(uint8_t *rom)
{
	if (!rom || (rom[0] != 0x55 && rom[1] != 0xaa)) {
		printf("False ROM signature: 0x%02x%02x\n", rom[0], rom[1]);
		return PATCH_ROM_FAILED;
	}
	
	uint16_t dcbptr = READ_LE_SHORT(rom, 0x36);

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

			sig = READ_LE_INT(dcbtable, 6);
		}
		else
		{
			sig = READ_LE_INT(dcbtable, 4);
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
		connection = READ_LE_INT(dcbtable,headerlength + recordlength * i);

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

static char *get_nvidia_model(uint32_t device_id, uint32_t subsys_id)
{
	int i;
	for (i = 1; i < (sizeof(nvidia_cards) / sizeof(nvidia_cards[0])); i++) // size of NVKnowChipsets array for-loop
	{
		if ((nvidia_cards[i].device == device_id) && (nvidia_cards[i].subdev == subsys_id))
            {
                return nvidia_cards[i].name_model;
                break;
            }
            else if ((nvidia_cards[i].device == device_id) && (nvidia_cards[i].subdev == 0x00000000))
            {
                return nvidia_cards[i].name_model;
                break;
            }
	}
    return nvidia_cards[0].name_model;
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
		//case 0x0649: vram_size = 1024*1024*1024; break;	// 9600M GT 0649
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

static bool checkNvRomSig(uint8_t * aRom){
    return aRom != NULL && (aRom[0] == 0x55 && aRom[1] == 0xaa);
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
	model = get_nvidia_model(((nvda_dev->vendor_id << 16) | nvda_dev->device_id),((nvda_dev->subsys_id.subsys.vendor_id << 16) | nvda_dev->subsys_id.subsys.device_id));
	
	verbose("%s %dMB NV%02x [%04x:%04x]-[%04x:%04x] :: %s device number: %d\n",
			model, (uint32_t)(videoRam / 1024 / 1024),
			(REG32(0) >> 20) & 0x1ff, nvda_dev->vendor_id, nvda_dev->device_id,
			nvda_dev->subsys_id.subsys.vendor_id, nvda_dev->subsys_id.subsys.device_id,
			devicepath, devices_number);
	
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
		
        // PROM first
        // Enable PROM access
        (REG32(NV_PBUS_PCI_NV_20)) = NV_PBUS_PCI_NV_20_ROM_SHADOW_DISABLED;
        nvRom = (uint8_t*)&regs[NV_PROM_OFFSET];

        // Valid Signature ?
		if (checkNvRomSig(nvRom))
		{
            bcopy((uint8_t *)nvRom, rom, NVIDIA_ROM_SIZE);
            DBG("PROM Address 0x%x Signature 0x%02x%02x\n", nvRom, rom[0], rom[1]);
        }
        else
        {

            // disable PROM access
            (REG32(NV_PBUS_PCI_NV_20)) = NV_PBUS_PCI_NV_20_ROM_SHADOW_ENABLED;

	    //PRAM next
            nvRom = (uint8_t*)&regs[NV_PRAMIN_OFFSET];

            if(checkNvRomSig(nvRom))
            {
                bcopy((uint32_t *)nvRom, rom, NVIDIA_ROM_SIZE);
                DBG("PRAM Address 0x%x Signature 0x%02x%02x\n", nvRom, rom[0], rom[1]);
            }
            else
    		{
				// 0xC0000 last
				bcopy((char *)0xc0000, rom, NVIDIA_ROM_SIZE);
				
				// Valid Signature ?
				if (!checkNvRomSig(rom))
				{
					printf("ERROR: Unable to locate nVidia Video BIOS\n");
					return false;
				}
                else
                {
                    DBG("ROM Address 0x%x Signature 0x%02x%02x\n", nvRom, rom[0], rom[1]);
                }
            			}//end PRAM check
                }//end PROM check
    	}//end load rom from bios
	
	if ((nvPatch = patch_nvidia_rom(rom)) == PATCH_ROM_FAILED)
	{
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
			model = get_nvidia_model(((rom_pci_header->vendor_id << 16) | rom_pci_header->device_id), NV_SUB_IDS);
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
	devprop_add_value(device, "@0,display-cfg", default_dcfg_0, DCFG0_LEN);
	devprop_add_value(device, "@1,display-cfg", default_dcfg_1, DCFG1_LEN);
	

	if (getBoolForKey(kVBIOS, &doit, &bootInfo->chameleonConfig) && doit)
	{
		devprop_add_value(device, "vbios", rom, (nvBiosOveride > 0) ? nvBiosOveride : (rom[2] * 512));
	}

	//add HDMI Audio back to nvidia
	doit = false;
	//http://forge.voodooprojects.org/p/chameleon/issues/67/
	if(getBoolForKey(kEnableHDMIAudio, &doit, &bootInfo->chameleonConfig) && doit)
	{
        static uint8_t connector_type_1[]= {0x00, 0x08, 0x00, 0x00};
		devprop_add_value(device, "@1,connector-type",connector_type_1, 4);
	}
	//end Nvidia HDMI Audio

	stringdata = malloc(sizeof(uint8_t) * string->length);
	memcpy(stringdata, (uint8_t*)devprop_generate_string(string), string->length);
	stringlength = string->length;
	
	return true;
}
