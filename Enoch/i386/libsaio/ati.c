/*
 * ATI Graphics Card Enabler, part of the Chameleon Boot Loader Project
 *
 * Copyright 2010 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */

#include "ati.h"

static value_t aty_name;
static value_t aty_nameparent;
static bool	doit	= false;
card_t *card;

card_config_t card_configs[] = {
	{NULL,		0},
	/* OLDController */
	{"Wormy",	2},
	{"Alopias",	2},
	{"Caretta",	1},
	{"Kakapo",	3},
	{"Kipunji",	4},
	{"Peregrine",	2},
	{"Raven",	3},
	{"Sphyrna",	1},
	/* AMD2400Controller */
	{"Iago",	2},
	/* AMD2600Controller */
	{"Hypoprion",	2},
	{"Lamna",	2},
	/* AMD3800Controller */
	{"Megalodon",	3},
	{"Triakis",	2},
	/* AMD4600Controller */
	{"Flicker",	3},
	{"Gliff",	3},
	{"Shrike",	3},
	/* AMD4800Controller */
	{"Cardinal",	2},
	{"Motmot",	2},
	{"Quail",	3},
	/* AMD5000Controller */
	{"Douc",	2},
	{"Langur",	3},
	{"Uakari",	4},
	{"Zonalis",	6},
	{"Alouatta",	4},
	{"Hoolock",	1},
	{"Vervet",	4},
	{"Baboon",	3},
	{"Eulemur",	3},
	{"Galago",	2},
	{"Colobus",	2},
	{"Mangabey",	2},
	{"Nomascus",	5},
	{"Orangutan",	2},
	/* AMD6000Controller */
	{"Pithecia",	3},
	{"Bulrushes",	6},
	{"Cattail",	4},
	{"Hydrilla",	5},
	{"Duckweed",	4},
	{"Fanwort",	4},
	{"Elodea",	5},
	{"Kudzu",	2},
	{"Gibba",	5},
	{"Lotus",	3},
	{"Ipomoea",	3},
	{"Muskgrass",	4},
	{"Juncus",	4},
	{"Osmunda",     4}, 
	{"Pondweed",	3},
	{"Spikerush",   4},
	{"Typha",       5},
	/* AMD7000Controller */
	{"Aji",		4},
	{"Buri",	4},
	{"Chutoro",	5},
	{"Dashimaki",	4},
	{"Ebi",		5},
	{"Gari",	5},
	{"Futomaki",	5},
	{"Hamachi",	4},
	{"OPM",         6},
	{"Ikura",       6},
	{"IkuraS",      1},
	/* AMD8000Controller */
	{"Baladi",      5},	//desktop
	{"Exmoor",      4},	//mobile
	{"Basset",      4}

};

radeon_card_info_t radeon_cards[] = {
	
	// Earlier cards are not supported
	//
	// Layout is device_id, subsys_id (subsystem id plus vendor id), chip_family_name, display name, frame buffer
	// Cards are grouped by device id and vendor id then sorted by subsystem id to make it easier to add new cards
	//

	/* Old series */

	// R423
	{ 0x5D48, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Mobile ",	kNull		},
	{ 0x5D49, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Mobile ",	kNull		},
	{ 0x5D4A, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Mobile ",	kNull		},
	{ 0x5D4C, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5D4D, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5D4E, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5D4F, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5D50, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5D52, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5D57, 0x00000000, CHIP_FAMILY_R423,  "ATI Radeon HD Desktop ",	kNull		},

	// RV410
	{ 0x5E48, 0x00000000, CHIP_FAMILY_RV410, "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5E4A, 0x00000000, CHIP_FAMILY_RV410, "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5E4B, 0x00000000, CHIP_FAMILY_RV410, "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5E4C, 0x00000000, CHIP_FAMILY_RV410, "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5E4D, 0x00000000, CHIP_FAMILY_RV410, "ATI Radeon HD Desktop ",	kNull		},
	{ 0x5E4F, 0x00000000, CHIP_FAMILY_RV410, "ATI Radeon HD Desktop ",	kNull		},

	// R520
	{ 0x7100, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x7101, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Mobile ",	kNull		},
	{ 0x7102, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Mobile ",	kNull		},
	{ 0x7103, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Mobile ",	kNull		},
	{ 0x7104, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x7105, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x7106, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Mobile ",	kNull		},
	{ 0x7108, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x7109, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x710A, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x710B, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x710C, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x710E, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},
	{ 0x710F, 0x00000000, CHIP_FAMILY_R520,  "ATI Radeon HD Desktop ",	kNull		},

	// RV515
	{ 0x7140, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7141, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7142, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7143, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7144, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x7145, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x7146, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7147, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7149, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x714A, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x714B, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x714C, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x714D, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x714E, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x714F, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7151, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7152, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7153, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x715E, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x715F, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7180, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7181, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7183, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7186, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x7187, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7188, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD2300 Mobile ",	kCaretta	},
	{ 0x718A, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x718B, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x718C, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x718D, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x718F, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7193, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x7196, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kCaretta	},
	{ 0x719B, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},
	{ 0x719F, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kCaretta	},

	// RV530
	{ 0x71C0, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71C1, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71C2, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71C3, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71C4, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",	kWormy		},
	{ 0x71C5, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD1600 Mobile ",	kWormy		},
	{ 0x71C6, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71C7, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71CD, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71CE, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71D2, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71D4, 0x00000000, CHIP_FAMILY_RV530, "ATI Mobility FireGL V5250",	kWormy		},
	{ 0x71D5, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",	kWormy		},
	{ 0x71D6, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Mobile ",	kWormy		},
	{ 0x71DA, 0x00000000, CHIP_FAMILY_RV530, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x71DE, 0x00000000, CHIP_FAMILY_RV530, "ASUS M66 ATI Radeon Mobile ",	kWormy		},

	// RV515
	{ 0x7200, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Desktop ",	kWormy		},
	{ 0x7210, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kWormy		},
	{ 0x7211, 0x00000000, CHIP_FAMILY_RV515, "ATI Radeon HD Mobile ",	kWormy		},

	// R580
	{ 0x7240, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7243, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7244, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7245, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7246, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7247, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7248, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7249, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x724A, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x724B, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x724C, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x724D, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x724E, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x724F, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Desktop ",	kAlopias	},

	// RV570
	{ 0x7280, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon X1950 Pro ",	kAlopias	},

	// RV560
	{ 0x7281, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7283, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",	kAlopias	},

	// R580
	{ 0x7284, 0x00000000, CHIP_FAMILY_R580,  "ATI Radeon HD Mobile ",	kAlopias	},

	// RV560
	{ 0x7287, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",	kAlopias	},

	// RV570
	{ 0x7288, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7289, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x728B, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x728C, 0x00000000, CHIP_FAMILY_RV570, "ATI Radeon HD Desktop ",	kAlopias	},

	// RV560
	{ 0x7290, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7291, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7293, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",	kAlopias	},
	{ 0x7297, 0x00000000, CHIP_FAMILY_RV560, "ATI Radeon HD Desktop ",	kAlopias	},

	/* IGP */

	// RS690
	{ 0x791E,	0x00000000, CHIP_FAMILY_RS690,	"ATI Radeon IGP ",	kNull			},
	{ 0x791F,	0x00000000, CHIP_FAMILY_RS690,	"ATI Radeon IGP ",	kNull			},
	{ 0x793F,	0x00000000, CHIP_FAMILY_RS690,	"ATI Radeon IGP ",	kNull			},
	{ 0x7941,	0x00000000, CHIP_FAMILY_RS690,	"ATI Radeon IGP ",	kNull			},
	{ 0x7942,	0x00000000, CHIP_FAMILY_RS690,	"ATI Radeon IGP ",	kNull			},


	// RS740
	{ 0x796C,	0x00000000, CHIP_FAMILY_RS740,	"ATI Radeon IGP ",	kNull			},
	{ 0x796D,	0x00000000, CHIP_FAMILY_RS740,	"ATI Radeon IGP ",	kNull			},
	{ 0x796E,	0x00000000, CHIP_FAMILY_RS740,	"ATI Radeon IGP ",	kNull			},
	{ 0x796F,	0x00000000, CHIP_FAMILY_RS740,	"ATI Radeon IGP ",	kNull			},

	/* standard/default models */

    // RS600
	{ 0x9400,	0x00000000, CHIP_FAMILY_R600,	"ATI Radeon HD 2900 XT",	kNull		},
	{ 0x9401,	0x00000000, CHIP_FAMILY_R600,	"ATI Radeon HD 2900 GT",	kNull		},
	{ 0x9402,	0x00000000, CHIP_FAMILY_R600,	"ATI Radeon HD 2900 GT",	kNull		},
	{ 0x9403,	0x00000000, CHIP_FAMILY_R600,	"ATI Radeon HD 2900 GT",	kNull		},
	{ 0x9405,	0x00000000, CHIP_FAMILY_R600,	"ATI Radeon HD 2900 GT",	kNull		},
	{ 0x940A,	0x00000000, CHIP_FAMILY_R600,	"ATI FireGL V8650",	kNull		},
	{ 0x940B,	0x00000000, CHIP_FAMILY_R600,	"ATI FireGL V8600",	kNull		},
	{ 0x940F,	0x00000000, CHIP_FAMILY_R600,	"ATI FireGL V7600",	kNull		},

	// RV740
	{ 0x94A0,	0x00000000, CHIP_FAMILY_RV740,	"ATI Radeon HD 4830M",	kFlicker	},
	{ 0x94A1,	0x00000000, CHIP_FAMILY_RV740,	"ATI Radeon HD 4860M",	kFlicker	},
	{ 0x94A3,	0x00000000, CHIP_FAMILY_RV740,	"ATI FirePro M7740",	kFlicker	},
	{ 0x94B1,	0x00000000, CHIP_FAMILY_RV740,	"ATI Radeon HD",	kFlicker	},
	{ 0x94B3,	0x00000000, CHIP_FAMILY_RV740,	"ATI Radeon HD 4770",	kFlicker	},
	{ 0x94B4,	0x00000000, CHIP_FAMILY_RV740,	"ATI Radeon HD 4700 Series",	kFlicker	},
	{ 0x94B5,	0x00000000, CHIP_FAMILY_RV740,	"ATI Radeon HD 4770",	kFlicker	},
	{ 0x94B9,	0x00000000, CHIP_FAMILY_RV740,	"ATI Radeon HD",	kFlicker	},

	// RV770
	{ 0x9440,	0x00000000, CHIP_FAMILY_RV770,	"ATI Radeon HD 4870 ",	kMotmot		},
	{ 0x9441,	0x00000000, CHIP_FAMILY_RV770,	"ATI Radeon HD 4870 X2",	kMotmot		},
	{ 0x9442,	0x00000000, CHIP_FAMILY_RV770,	"ATI Radeon HD 4850 Series",	kMotmot		},
	{ 0x9443,	0x00000000, CHIP_FAMILY_RV770,	"ATI Radeon HD 4850 X2",	kMotmot		},
	{ 0x9444,	0x00000000, CHIP_FAMILY_RV770,	"ATI FirePro V8750 (FireGL)",           kMotmot		},
	{ 0x9446,	0x00000000, CHIP_FAMILY_RV770,	"ATI FirePro V7770 (FireGL)",	kMotmot		},
	{ 0x9447,	0x00000000, CHIP_FAMILY_RV770,	"ATI FirePro V8700 Duo (FireGL)",	kMotmot		},
	{ 0x944A,	0x00000000, CHIP_FAMILY_RV770,	"ATI Mobility Radeon HD 4850",	kMotmot		},//iMac - Quail
	{ 0x944B,	0x00000000, CHIP_FAMILY_RV770,	"ATI Mobility Radeon HD 4850 X2",	kMotmot		},//iMac - Quail
	{ 0x944C,	0x00000000, CHIP_FAMILY_RV770,	"ATI Radeon HD 4830 Series",	kMotmot		},
	{ 0x944E,	0x00000000, CHIP_FAMILY_RV770,	"ATI Radeon HD 4810 Series",	kMotmot		},
	{ 0x9450,	0x00000000, CHIP_FAMILY_RV770,	"AMD FireStream 9270",	kMotmot		},
	{ 0x9452,	0x00000000, CHIP_FAMILY_RV770,	"AMD FireStream 9250",	kMotmot		},
	{ 0x9456,	0x00000000, CHIP_FAMILY_RV770,	"ATI FirePro V8700 (FireGL)",	kMotmot		},
	{ 0x945A,	0x00000000, CHIP_FAMILY_RV770,	"ATI Mobility Radeon HD 4870",	kMotmot		},
	{ 0x9460,	0x00000000, CHIP_FAMILY_RV770,	"ATI Radeon HD 4800 Series",	kMotmot		},
	{ 0x9462,	0x00000000, CHIP_FAMILY_RV770,	"ATI Radeon HD 4800 Series",	kMotmot		},
//	{ 0x946A,	0x00000000, CHIP_FAMILY_RV770,	"ATI Mobility Radeon",	kMotmot		},
//	{ 0x946B,	0x00000000, CHIP_FAMILY_RV770,	"ATI Mobility Radeon",	kMotmot		},
//	{ 0x947A,	0x00000000, CHIP_FAMILY_RV770,	"ATI Mobility Radeon",	kMotmot		},
//	{ 0x947B,	0x00000000, CHIP_FAMILY_RV770,	"ATI Mobility Radeon",	kMotmot		},

	// RV730
	{ 0x9480,	0x00000000, CHIP_FAMILY_RV730,	"ATI Mobility Radeon HD 550v",	kGliff		},
	{ 0x9487,	0x00000000, CHIP_FAMILY_RV730,	"ATI Radeon HD Series",	kGliff		},
	{ 0x9488,	0x00000000, CHIP_FAMILY_RV730,	"ATI Radeon HD 4650 Series",	kGliff		},
	{ 0x9489,	0x00000000, CHIP_FAMILY_RV730,	"ATI Radeon HD Series",	kGliff		},
	{ 0x948A,	0x00000000, CHIP_FAMILY_RV730,	"ATI Radeon HD Series",	kGliff		},
	{ 0x948F,	0x00000000, CHIP_FAMILY_RV730,	"ATI Radeon HD Series",	kGliff		},
	{ 0x9490,	0x00000000, CHIP_FAMILY_RV730,	"ATI Radeon HD 4670 Series",	kGliff		},
	{ 0x9491,	0x00000000, CHIP_FAMILY_RV730,	"ATI Radeon HD 4600 Series",	kGliff		},
	{ 0x9495,	0x00000000, CHIP_FAMILY_RV730,	"ATI Radeon HD 4650 Series",	kGliff		},
	{ 0x9498,	0x00000000, CHIP_FAMILY_RV730,	"ATI Radeon HD 4710 Series",	kGliff		},
	{ 0x949C,	0x00000000, CHIP_FAMILY_RV730,	"ATI FirePro V7750 (FireGL)",	kGliff		},
	{ 0x949E,	0x00000000, CHIP_FAMILY_RV730,	"ATI FirePro V5700 (FireGL)",	kGliff		},
	{ 0x949F,	0x00000000, CHIP_FAMILY_RV730,	"ATI FirePro V3750 (FireGL)",	kGliff		},

	// RV610
	{ 0x94C0,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD Series",	kIago		},
	{ 0x94C1,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2400 Series",	kIago		},
	{ 0x94C3,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2350 Series",	kIago		},
	{ 0x94C4,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2400 Series",	kIago		},
	{ 0x94C5,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2400 Series",	kIago		},
	{ 0x94C6,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2400 Series",	kIago		},
	{ 0x94C7,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2350",	kIago		},
	{ 0x94C8,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2400 Series",	kIago		},
	{ 0x94C9,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2400 Series",	kIago		},
	{ 0x94CB,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2400 Series",	kIago		},
	{ 0x94CC,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2400 Series",	kIago		},
	{ 0x94CD,	0x00000000, CHIP_FAMILY_RV610,	"ATI Radeon HD 2400 PRO Series",	kIago	},

	// RV670
	{ 0x9500,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3800 Series",	kMegalodon	},
	{ 0x9501,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3690 Series",	kMegalodon	},
	{ 0x9504,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3850M Series",	kMegalodon	},
	{ 0x9505,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3800 Series",	kMegalodon	},
	{ 0x9506,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3850 X2 M Series",	kMegalodon	},
	{ 0x9507,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3830",	kMegalodon	},
	{ 0x9508,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3870M Series",	kMegalodon	},
	{ 0x9509,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3870 X2 MSeries",	kMegalodon	},
	{ 0x950F,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3870 X2",	kMegalodon	},
	{ 0x9511,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3850 X2",	kMegalodon	},
	{ 0x9513,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3850 X2",	kMegalodon	},
	{ 0x9515,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD 3850 Series",	kMegalodon	},
	{ 0x9517,	0x00000000, CHIP_FAMILY_RV670,	"ATI Radeon HD Series",	kMegalodon	},
	{ 0x9519,	0x00000000, CHIP_FAMILY_RV670,	"AMD FireStream 9170",	kMegalodon	},

	// RV710
	{ 0x9540,	0x00000000, CHIP_FAMILY_RV710,	"ATI Radeon HD 4550",	kFlicker	},
	{ 0x9541,	0x00000000, CHIP_FAMILY_RV710,	"ATI Radeon HD",	kFlicker	},
	{ 0x9542,	0x00000000, CHIP_FAMILY_RV710,	"ATI Radeon HD",	kFlicker	},
	{ 0x954E,	0x00000000, CHIP_FAMILY_RV710,	"ATI Radeon HD",	kFlicker	},
	{ 0x954F,	0x00000000, CHIP_FAMILY_RV710,	"ATI Radeon HD 4350",	kFlicker	},
	{ 0x9552,	0x00000000, CHIP_FAMILY_RV710,	"ATI Mobility Radeon HD 4330", 	kShrike     },
	{ 0x9553,	0x00000000, CHIP_FAMILY_RV710,	"ATI Mobility Radeon HD 4570", 	kShrike     },
	{ 0x9555,	0x00000000, CHIP_FAMILY_RV710,	"ATI Mobility Radeon HD 4550", 	kShrike     },
	{ 0x9557,	0x00000000, CHIP_FAMILY_RV710,	"ATI FirePro RG220",	kFlicker	},
	{ 0x955F,	0x00000000, CHIP_FAMILY_RV710,	"ATI Radeon HD 4330M series",	kFlicker	},

	// RV630
	{ 0x9580,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD Series",	kHypoprion	},
	{ 0x9581,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 XT",	kHypoprion  },
	{ 0x9583,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 XT",	kHypoprion  },
	{ 0x9586,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 XT Series",	kHypoprion	},
	{ 0x9587,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 Pro Series",	kHypoprion	},
	{ 0x9588,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 XT",        kHypoprion  },
	{ 0x9589,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 PRO",       kHypoprion  },
	{ 0x958A,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 X2 Series",	kLamna      },
	{ 0x958B,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 X2 Series",	kLamna      },
	{ 0x958C,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 X2 Series",	kLamna      },
	{ 0x958D,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 X2 Series",	kLamna      },
	{ 0x958E,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD 2600 X2 Series",	kLamna      },
	{ 0x958F,	0x00000000, CHIP_FAMILY_RV630,	"ATI Radeon HD Series",	kHypoprion	},

	// RV635
//	{ 0x9590,	0x00000000, CHIP_FAMILY_RV635,	"ATI Radeon HD",	kMegalodon  },
	{ 0x9591,	0x00000000, CHIP_FAMILY_RV635,	"ATI Radeon HD 3600 Series",	kMegalodon  }, // Mobile
//	{ 0x9593,	0x00000000, CHIP_FAMILY_RV635,	"ATI Radeon HD",	kMegalodon  }, // Mobile
//	{ 0x9595,	0x00000000, CHIP_FAMILY_RV635,	"ATI Radeon HD",	kMegalodon  }, // Mobile
//	{ 0x9596,	0x00000000, CHIP_FAMILY_RV635,	"ATI Radeon HD",	kMegalodon  },
//	{ 0x9597,	0x00000000, CHIP_FAMILY_RV635,	"ATI Radeon HD",	kMegalodon  },
	{ 0x9598,	0x00000000, CHIP_FAMILY_RV635,	"ATI Radeon HD 3600 Series",	kMegalodon  },
//	{ 0x9599,	0x00000000, CHIP_FAMILY_RV635,	"ATI Radeon HD",	kMegalodon  },
//	{ 0x959B,	0x00000000, CHIP_FAMILY_RV635,	"ATI Radeon HD",	kMegalodon  }, // Mobile

	// RV620
	{ 0x95C0,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD 3550 Series",	kIago       },
//	{ 0x95C2,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD",	kIago       }, // Mobile
	{ 0x95C4,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD 3470 Series",	kIago       }, // Mobile
	{ 0x95C5,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD 3450 Series",	kIago       },
	{ 0x95C6,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD 3450 AGP",	kIago       },
//	{ 0x95C7,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD",	kIago       },
//	{ 0x95C9,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD",	kIago       },
//	{ 0x95CC,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD",	kIago       },
//	{ 0x95CD,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD",	kIago       },
//	{ 0x95CE,	0x00000000, CHIP_FAMILY_RV620,	"ATI Radeon HD",	kIago       },
	{ 0x95CF,	0x00000000, CHIP_FAMILY_RV620,	"ATI FirePro 2260",	kIago       },

	/* IGP */

	// RS780
	{ 0x9610,	0x00000000, CHIP_FAMILY_RS780,	"ATI Radeon HD 3200 Graphics",	kNull       },
	{ 0x9611,	0x00000000, CHIP_FAMILY_RS780,	"ATI Radeon HD 3100 Graphics",	kNull       },
//	{ 0x9612,	0x00000000, CHIP_FAMILY_RS780,	"ATI Radeon HD",	kNull       },
//	{ 0x9613,	0x00000000, CHIP_FAMILY_RS780,	"ATI Radeon HD",	kNull       },
	{ 0x9614,	0x00000000, CHIP_FAMILY_RS780,	"ATI Radeon HD 3300 Graphics",	kNull       },
//	{ 0x9615,	0x00000000, CHIP_FAMILY_RS780,	"ATI Radeon HD",	kNull       },
	{ 0x9616,	0x00000000, CHIP_FAMILY_RS780,	"AMD 760G",                     kNull       },

	// SUMO
	{ 0x9640,	0x00000000, CHIP_FAMILY_SUMO,	"AMD Radeon HD 6550D",	kNull       },
	{ 0x9641,	0x00000000, CHIP_FAMILY_SUMO,	"AMD Radeon HD 6620G",	kNull       }, // Mobile

    // SUMO2
	{ 0x9642,	0x00000000, CHIP_FAMILY_SUMO2,	"AMD Radeon HD 6370D",	kNull       },
	{ 0x9643,	0x00000000, CHIP_FAMILY_SUMO2,	"AMD Radeon HD 6380G",	kNull       }, // Mobile
	{ 0x9644,	0x00000000, CHIP_FAMILY_SUMO2,	"AMD Radeon HD 6410D",	kNull       },
	{ 0x9645,	0x00000000, CHIP_FAMILY_SUMO2,	"AMD Radeon HD 6410D",	kNull       }, // Mobile

    // SUMO
	{ 0x9647,	0x00000000, CHIP_FAMILY_SUMO,	"AMD Radeon HD 6520G",	kNull       }, // Mobile
	{ 0x9648,	0x00000000, CHIP_FAMILY_SUMO,	"AMD Radeon HD 6480G",	kNull       }, // Mobile

    // SUMO2
	{ 0x9649,	0x00000000, CHIP_FAMILY_SUMO2,	"AMD Radeon(TM) HD 6480G",	kNull       }, // Mobile

    // SUMO
	{ 0x964A,	0x00000000, CHIP_FAMILY_SUMO,	"AMD Radeon HD 6530D",	kNull       },
//	{ 0x964B,	0x00000000, CHIP_FAMILY_SUMO,	"AMD Radeon HD",	kNull       },
//	{ 0x964C,	0x00000000, CHIP_FAMILY_SUMO,	"AMD Radeon HD",	kNull       },
//	{ 0x964E,	0x00000000, CHIP_FAMILY_SUMO,	"AMD Radeon HD",	kNull       }, // Mobile
//	{ 0x964F,	0x00000000, CHIP_FAMILY_SUMO,	"AMD Radeon HD",	kNull       }, // Mobile

	// RS880
	{ 0x9710,	0x00000000, CHIP_FAMILY_RS880,	"ATI Radeon HD 4200 Series",	kNull	},
//	{ 0x9711,	0x00000000, CHIP_FAMILY_RS880,	"ATI Radeon HD",	kNull	},
	{ 0x9712,	0x00000000, CHIP_FAMILY_RS880,	"ATI Radeon HD 4200 Series",	kNull	}, // Mobile
//	{ 0x9713,	0x00000000, CHIP_FAMILY_RS880,	"ATI Radeon HD",	kNull	}, // Mobile
	{ 0x9714,	0x00000000, CHIP_FAMILY_RS880,	"ATI Radeon HD 4290",	kNull	},
	{ 0x9715,	0x00000000, CHIP_FAMILY_RS880,	"ATI Radeon HD 4250",	kNull	},
	{ 0x9723,	0x00000000, CHIP_FAMILY_RS880,	"ATI Radeon HD 5450 Series",	kNull	},

	// PALM
	{ 0x9802,	0x00000000, CHIP_FAMILY_PALM,	"AMD Radeon HD 6310 Graphics",  kNull       },
	{ 0x9803,	0x00000000, CHIP_FAMILY_PALM,	"AMD Radeon HD 6250 Graphics",  kNull       },
	{ 0x9804,	0x00000000, CHIP_FAMILY_PALM,	"AMD Radeon HD 6250 Graphics",  kNull       },
	{ 0x9805,	0x00000000, CHIP_FAMILY_PALM,	"AMD Radeon HD 6250 Graphics",  kNull       },
	{ 0x9806,	0x00000000, CHIP_FAMILY_PALM,	"AMD Radeon HD 6320 Graphics",  kNull       },
	{ 0x9807,	0x00000000, CHIP_FAMILY_PALM,	"AMD Radeon HD 6290 Graphics",  kNull       },
	{ 0x9808,	0x00000000, CHIP_FAMILY_PALM,	"AMD Radeon HD 7340 Graphics",  kNull       },
	{ 0x9809,	0x00000000, CHIP_FAMILY_PALM,	"AMD Radeon HD 7310 Graphics",  kNull       },
//	{ 0x980A,	0x00000000, CHIP_FAMILY_PALM,	"AMD Radeon HD",  kNull       },

	// KABINI
//	{ 0x9830,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       }, // Mobile
//	{ 0x9831,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       },
//	{ 0x9832,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       }, // Mobile
//	{ 0x9833,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       },
//	{ 0x9834,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       }, // Mobile
//	{ 0x9835,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       },
//	{ 0x9836,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       }, // Mobile
//	{ 0x9837,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       },
//	{ 0x9838,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       }, // Mobile
//	{ 0x9839,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       }, // Mobile
//	{ 0x983A,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       },
//	{ 0x983B,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       }, // Mobile
//	{ 0x983C,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       },
//	{ 0x983D,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       },
//	{ 0x983E,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       },
//	{ 0x983F,	0x00000000, CHIP_FAMILY_KABINI,	"AMD Radeon HD",  kNull       },

	// MULLINS
	{ 0x9850,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
	{ 0x9851,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
	{ 0x9852,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
	{ 0x9853,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
	{ 0x9854,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
	{ 0x9855,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
	{ 0x9856,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
	{ 0x9857,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
	{ 0x9858,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
	{ 0x9859,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
	{ 0x985A,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
	{ 0x985B,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
	{ 0x985C,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
	{ 0x985D,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
	{ 0x985E,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile
	{ 0x985F,	0x00000000, CHIP_FAMILY_MULLINS,	"AMD Radeon HD",  kNull       }, // Mobile

	// ARUBA
	{ 0x9900,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7660G",      kNull       }, // Mobile
	{ 0x9901,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7660D",      kNull       },
	{ 0x9903,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7640G",      kNull       }, // Mobile
	{ 0x9904,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7560D",      kNull       },
//	{ 0x9905,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       },
	{ 0x9906,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD FirePro A300 Series",  kNull       },
	{ 0x9907,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7620G",      kNull       }, // Mobile
	{ 0x9908,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7600G",      kNull       }, // Mobile
//	{ 0x9909,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       }, // Mobile
//	{ 0x990A,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       }, // Mobile
//	{ 0x990B,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       }, // Mobile
//	{ 0x990C,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       },
//	{ 0x990D,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       }, // Mobile
//	{ 0x990E,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       },
//	{ 0x990F,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       }, // Mobile
	{ 0x9910,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7660G",      kNull       }, // Mobile
	{ 0x9913,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7640G",      kNull       }, // Mobile
//	{ 0x9917,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon",      kNull       },
//	{ 0x9918,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon",      kNull       },
//	{ 0x9919,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon",      kNull       },
	{ 0x9990,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7520G",      kNull       }, // Mobile
	{ 0x9991,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7540D",      kNull       },
	{ 0x9992,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7420G",      kNull       }, // Mobile
//	{ 0x9993,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       },
	{ 0x9994,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD 7400G",      kNull       }, // Mobile
//	{ 0x9995,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       }, // Mobile
//	{ 0x9996,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       },
//	{ 0x9997,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       }, // Mobile
//	{ 0x9998,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       },
//	{ 0x9999,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       }, // Mobile
//	{ 0x999A,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       }, // Mobile
//	{ 0x999B,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       }, // Mobile
//	{ 0x999C,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       },
//	{ 0x999D,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       },
//	{ 0x99A0,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       }, // Mobile
//	{ 0x99A2,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       }, // Mobile
//	{ 0x99A4,	0x00000000, CHIP_FAMILY_ARUBA,	"AMD Radeon HD",      kNull       },

	{ 0x6610,	0x00000000, CHIP_FAMILY_OLAND,		"AMD Radeon R7 250",	kFutomaki		},
	{ 0x6613,	0x00000000, CHIP_FAMILY_OLAND,		"AMD Radeon R7 240",	kFutomaki		},
	{ 0x665C,	0x00000000, CHIP_FAMILY_BONAIRE,	"AMD Radeon HD 7790",	kFutomaki		},
	{ 0x665D,	0x00000000, CHIP_FAMILY_BONAIRE,	"AMD Radeon R9 260",	kFutomaki		},

	/* Evergreen */

	// CYPRESS
//	{ 0x6880,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD",	kNull	}, // Mobile
//	{ 0x6888,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD",	kNull	},
//	{ 0x6889,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD",	kNull	},
//	{ 0x688A,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD",	kNull	},
//	{ 0x688C,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD",	kNull	},
	{ 0x688D,	0x00000000, CHIP_FAMILY_CYPRESS,	"AMD FireStream 9350",	kZonalis	},
	{ 0x6898,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5870 Series",	kUakari	},
	{ 0x6899,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5850 Series",	kUakari	},
	{ 0x689B,	0x00000000, CHIP_FAMILY_CYPRESS,	"AMD Radeon HD 6800 Series",	kNull	},

	// HEMLOCK
	{ 0x689C,	0x00000000, CHIP_FAMILY_HEMLOCK,	"ATI Radeon HD 5970 Series",	kUakari	},
	{ 0x689D,	0x00000000, CHIP_FAMILY_HEMLOCK,	"ATI Radeon HD 5900 Series",	kUakari	},

	// CYPRESS
	{ 0x689E,	0x00000000, CHIP_FAMILY_CYPRESS,	"ATI Radeon HD 5800 Series",	kUakari	},

	// JUNIPER
	{ 0x68A0,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",	kHoolock	}, // Mobile
	{ 0x68A1,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5850 Series",	kHoolock	}, // Mobile
	{ 0x68A8,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6850M",		kHoolock	},
	{ 0x68A9,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI FirePro V5800 (FireGL)",	kHoolock	},
	{ 0x68B0,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",	kHoolock	}, // Mobile
	{ 0x68B1,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",	kHoolock	},
	{ 0x68B8,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5770 Series",	kVervet	},
	{ 0x68B9,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5700 Series",	kHoolock	},
	{ 0x68BA,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6770 Series",	kHoolock	},
	{ 0x68BC,	0x00000000, CHIP_FAMILY_JUNIPER,	"AMD FireStream 9370",		kHoolock	},
	{ 0x68BD,	0x00000000, CHIP_FAMILY_JUNIPER,	"AMD FireStream 9350",		kHoolock	},
	{ 0x68BE,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 5750 Series",	kHoolock	},
	{ 0x68BF,	0x00000000, CHIP_FAMILY_JUNIPER,	"ATI Radeon HD 6750 Series",	kHoolock	},

	// REDWOOD
	{ 0x68C0,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5730 Series",	kBaboon	}, // Mobile
	{ 0x68C1,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5650 Series",	kBaboon	}, // Mobile
	{ 0x68C7,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Mobility Radeon HD 5570",	kEulemur	}, // Mobile
	{ 0x68C8,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI FirePro v4800",    kBaboon	},	
	{ 0x68C9,	0x00000000, CHIP_FAMILY_REDWOOD,	"FirePro 3D V3800",	kBaboon	},
	{ 0x68D8,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5670 Series",	kBaboon	},
	{ 0x68D9,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5570 Series",	kBaboon	},
	{ 0x68DA,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5500 Series",	kBaboon	},
	{ 0x68DE,	0x00000000, CHIP_FAMILY_REDWOOD,	"ATI Radeon HD 5000 Series",	kNull		},

	// CEDAR
	{ 0x68E0,	0x00000000, CHIP_FAMILY_CEDAR,	"ATI Radeon HD 5470 Series",	kEulemur	},
	{ 0x68E1,	0x00000000, CHIP_FAMILY_CEDAR,	"AMD Radeon HD 6230",		kEulemur	},
	{ 0x68E4,	0x00000000, CHIP_FAMILY_CEDAR,	"ATI Radeon HD 6370M Series",	kEulemur	},
	{ 0x68E5,	0x00000000, CHIP_FAMILY_CEDAR,	"ATI Radeon HD 6300M Series",	kEulemur	},
//	{ 0x68E8,	0x00000000, CHIP_FAMILY_CEDAR,	"ATI Radeon HD ??? Series",	kNull		},
//	{ 0x68E9,	0x00000000, CHIP_FAMILY_CEDAR,	"ATI Radeon HD ??? Series",	kNull		},
	{ 0x68F1,	0x00000000, CHIP_FAMILY_CEDAR,	"AMD FirePro 2460",	kEulemur	},
	{ 0x68F2,	0x00000000, CHIP_FAMILY_CEDAR,	"AMD FirePro 2270",	kEulemur	},
//	{ 0x68F8,	0x00000000, CHIP_FAMILY_CEDAR,	"ATI Radeon HD ??? Series",	kNull		},
	{ 0x68F9,	0x00000000, CHIP_FAMILY_CEDAR,	"ATI Radeon HD 5450 Series",	kEulemur	},
	{ 0x68FA,	0x00000000, CHIP_FAMILY_CEDAR,	"ATI Radeon HD 7300 Series",	kEulemur	},
//	{ 0x68FE,	0x00000000, CHIP_FAMILY_CEDAR,	"ATI Radeon HD ??? Series",	kNull		},

	/* Northen Islands */

	// CAYMAN
	{ 0x6701,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6xxx Series",	kLotus		},
	{ 0x6702,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6xxx Series",	kLotus		},
	{ 0x6703,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6xxx Series",	kLotus		},
	{ 0x6704,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD FirePro V7900",	kLotus		},
	{ 0x6705,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6xxx Series",	kLotus		},
	{ 0x6706,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6xxx Series",	kLotus		},
	{ 0x6707,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6xxx Series",	kLotus		},
	{ 0x6708,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD FirePro V5900",	kLotus		},
	{ 0x6709,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6xxx Series",	kLotus		},
	{ 0x6718,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6970 Series",	kLotus		},
	{ 0x6719,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6950 Series",	kLotus		},
	{ 0x671C,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6970 Series",	kLotus		},
	{ 0x671D,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6950 Series",	kLotus		},
	{ 0x671F,	0x00000000, CHIP_FAMILY_CAYMAN,	"AMD Radeon HD 6930 Series",	kLotus		},

	// BARTS
	{ 0x6720,	0x00000000, CHIP_FAMILY_BARTS,	"AMD Radeon HD 6900M Series",	kFanwort	},
	{ 0x6722,	0x00000000, CHIP_FAMILY_BARTS,	"AMD Radeon HD 6900M Series",	kFanwort	},
	{ 0x6729,	0x00000000, CHIP_FAMILY_BARTS,	"AMD Radeon HD 6900M Series",	kFanwort	},
	{ 0x6738,	0x00000000, CHIP_FAMILY_BARTS,	"AMD Radeon HD 6870 Series",	kDuckweed	},
	{ 0x6739,	0x00000000, CHIP_FAMILY_BARTS,	"AMD Radeon HD 6850 X2",	kDuckweed	},
	{ 0x673E,	0x00000000, CHIP_FAMILY_BARTS,	"AMD Radeon HD 6790 Series",	kDuckweed	},

	// TURKS
	{ 0x6740,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 6770M Series",	kCattail		},
	{ 0x6741,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 6750M",	kCattail	},
	{ 0x6742,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7500/7600 Series",	kCattail	},
	{ 0x6745,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 6600M Series",	kCattail	},
	{ 0x6749,	0x00000000, CHIP_FAMILY_TURKS,	"ATI Radeon FirePro V4900",	kPithecia	},
	{ 0x674A,	0x00000000, CHIP_FAMILY_TURKS,	"AMD FirePro V3900",	kPithecia	},
	{ 0x6750,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 6670 Series",	kPithecia	},
	{ 0x6758,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 6670 Series",	kPithecia	},
	{ 0x6759,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 6570 Series",	kPithecia	},
	{ 0x675B,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7600 Series",	kPithecia	},
	{ 0x675D,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7570M Series",	kCattail	},
	{ 0x675F,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 6510 Series",	kPithecia	},

	// CAICOS
	{ 0x6760,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon HD 6470M Series",	kHydrilla	},
	{ 0x6761,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon HD 6430M Series",	kHydrilla	},
	{ 0x6763,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon E6460 Series",	kHydrilla	},
	{ 0x6768,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon HD 6400M Series",	kHydrilla	},
	{ 0x6770,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon HD 6400 Series",	kBulrushes	},
	{ 0x6772,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon HD 7400A Series",	kBulrushes	},
	{ 0x6778,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon HD 7000 Series",	kBulrushes	},
	{ 0x6779,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon HD 6450/7450/8450/R5 230",	kBulrushes	},
	{ 0x677B,	0x00000000, CHIP_FAMILY_CAICOS,	"AMD Radeon HD 7400 Series",	kBulrushes	},

	/* Southen Islands */

	// TAHITI
	//Framebuffers: Aji - 4 Desktop, Buri - 4 Mobile, Chutoro - 5 Mobile,  Dashimaki - 4, IkuraS - HMDI
	// Ebi - 5 Mobile, Gari - 5 M, Futomaki - 4 D, Hamachi - 4 D, OPM - 6 Server, Ikura - 6
	{ 0x6780,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7900 Series",	kFutomaki	},
	{ 0x6784,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7900 Series",	kFutomaki	},
	{ 0x6788,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7900 Series",	kFutomaki	},
	{ 0x678A,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7900 Series",	kFutomaki	},
	{ 0x6790,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7900 Series",	kFutomaki	},
	{ 0x6791,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7900 Series",	kFutomaki	},
	{ 0x6792,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7900 Series",	kFutomaki	},
	{ 0x6798,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7970X/8970/R9 280X",	kFutomaki	},
	{ 0x6799,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7990 Series",	kAji		},
	{ 0x679A,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7950/8950/R9 280",	kFutomaki	},
	{ 0x679B,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7900 Series",	kFutomaki	},
	{ 0x679E,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7870 XT",	kFutomaki	},
	{ 0x679F,	0x00000000, CHIP_FAMILY_TAHITI,	"AMD Radeon HD 7950 Series",	kFutomaki	},

	// HAWAII
//	{ 0x67A0,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",            kFutomaki	},
//	{ 0x67A1,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",            kFutomaki	},
//	{ 0x67A2,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",            kFutomaki	},
//	{ 0x67A8,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",            kFutomaki	},
//	{ 0x67A9,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",            kFutomaki	},
//	{ 0x67AA,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",            kFutomaki	},
	{ 0x67B0,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon R9 290X",           kBaladi	},
	{ 0x67B1,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon R9 290",            kBaladi	}, // CHIP_FAMILY_HAWAII
//	{ 0x67B8,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",            kFutomaki	},
//	{ 0x67B9,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",            kFutomaki	},
//	{ 0x67BA,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",            kFutomaki	},
//	{ 0x67BE,	0x00000000, CHIP_FAMILY_HAWAII,	"AMD Radeon",            kFutomaki	},

	// PITCAIRN
	{ 0x6800,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD 7970M",	kBuri	}, // Mobile
//	{ 0x6801,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD 8970M Series",	kFutomaki		}, // Mobile
//	{ 0x6802,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD ???M Series",	kFutomaki		}, // Mobile
	{ 0x6806,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD 7600 Series",	kFutomaki	},
	{ 0x6808,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD 7600 Series",	kFutomaki	},
//	{ 0x6809,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD ??? Series",	kNull		},
	{ 0x6810,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon R9 270X",	kFutomaki		},
	{ 0x6811,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon R9 270",	kFutomaki		},
//	{ 0x6816,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon",	kFutomaki		},
//	{ 0x6817,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon",	kFutomaki		},
	{ 0x6818,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD 7870 Series",	kFutomaki	},
	{ 0x6819,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD 7850 Series",	kFutomaki	},

	// VERDE
	{ 0x6820,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700 Series",	kBuri	}, // Mobile
	{ 0x6821,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700 Series",	kBuri	}, // Mobile
//	{ 0x6822,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD",	kBuri	}, // Mobile
//	{ 0x6823,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 8800M Series",	kBuri	}, // Mobile
//	{ 0x6824,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700M Series",	kBuri	}, // Mobile
	{ 0x6825,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7800M Series",	kPondweed	}, // Mobile
	{ 0x6826,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700 Series",	kBuri	}, // Mobile
	{ 0x6827,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7800M Series",	kPondweed	}, // Mobile
//	{ 0x6828,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD ??? Series",	kBuri	},
//	{ 0x6829,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD ??? Series",	kBuri	},
//	{ 0x682A,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD",	kBuri	}, // Mobile
	{ 0x682B,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 8800M Series",	kBuri	}, // Mobile
	{ 0x682D,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700 Series",	kBuri	}, // Mobile
	{ 0x682F,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7730 Series",	kBuri	}, // Mobile
	{ 0x6830,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7800M Series",	kBuri	}, // Mobile
	{ 0x6831,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700 Series",	kBuri	}, // Mobile
//	{ 0x6835,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD",	kBuri	},
	{ 0x6837,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700 Series",	kBuri	},
	{ 0x6838,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700 Series",	kBuri	},
	{ 0x6839,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700 Series",	kBuri	},
	{ 0x683B,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7700 Series",	kBuri	},
	{ 0x683D,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7770 Series",	kBuri	},
	{ 0x683F,	0x00000000, CHIP_FAMILY_VERDE,	"AMD Radeon HD 7750 Series",	kBuri	},

	// TURKS
	{ 0x6840,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7670M Series",	kPondweed	}, // Mobile
	{ 0x6841,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7650M Series",	kPondweed	}, // Mobile
	{ 0x6842,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7600M Series",	kPondweed	}, // Mobile
	{ 0x6843,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7670M Series",	kPondweed	}, // Mobile
	{ 0x6849,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7600M Series",	kPondweed	},

	// PITCAIRN
//	{ 0x684C,	0x00000000, CHIP_FAMILY_PITCAIRN,	"AMD Radeon HD",	kNull	},

	// TURKS
	{ 0x6850,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7600M Series",	kPondweed   },
	{ 0x6858,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7400 Series",	kPondweed   },
	{ 0x6859,	0x00000000, CHIP_FAMILY_TURKS,	"AMD Radeon HD 7600M Series",	kPondweed   },

	{ 0x0000,	0x00000000, CHIP_FAMILY_UNKNOW,	"AMD Unknown",			kNull		}
};

const char *chip_family_name[] = {
	"UNKNOW",
	"R420",
	"RV410",
	"RV515",
	"R520",
	"RV530",
	"RV560",
	"RV570",
	"R580",
	/* IGP */
	"RS600",
	"RS690",
	"RS740",
	"RS780",
	"RS880",
	/* R600 */
	"R600",
	"RV610",
	"RV620",
	"RV630",
	"RV635",
	"RV670",
	/* R700 */
	"RV710",
	"RV730",
	"RV740",
	"RV772",
	"RV770",
	"RV790",
	/* Evergreen */
	"Cedar",
	"Cypress",
	"Hemlock",
	"Juniper",
	"Redwood",
	"Broadway",
	/* Northern Islands */
	"Barts",
	"Caicos",
	"Cayman",
	"Turks",
	/* Southern Islands */
	"Tahiti",
	"Pitcairn",
	"Verde",
	"Oland",
	"Hainan",
	"Bonaire",
	"Kaveri",
	"Abini",
	"Hawaii",
	/* ... */
	"Mullins",
	""
};

AtiDevProp ati_devprop_list[] = {
	{FLAGTRUE,	false,	"@0,AAPL,boot-display",		get_bootdisplay_val,	NULVAL				},
//	{FLAGTRUE,	false,	"@0,ATY,EFIDisplay",		NULL,			STRVAL("TMDSA")			},

//	{FLAGTRUE,	true,	"@0,AAPL,vram-memory",		get_vrammemory_val,	NULVAL				},
	{FLAGTRUE,	true,	"@0,compatible",		get_name_val,		NULVAL				},
	{FLAGTRUE,	true,	"@0,connector-type",		get_conntype_val,	NULVAL				},
	{FLAGTRUE,	true,	"@0,device_type",		NULL,			STRVAL("display")		},
//	{FLAGTRUE,	false,	"@0,display-connect-flags",	NULL,			DWRVAL(0)			},

//	{FLAGTRUE,	true,	"@0,display-type",		NULL,			STRVAL("NONE")			},
	{FLAGTRUE,	true,	"@0,name",			get_name_val,		NULVAL				},
	{FLAGTRUE,	true,	"@0,VRAM,memsize",		get_vrammemsize_val,	NULVAL				},
//	{FLAGTRUE,	true,	"@0,ATY,memsize",		get_vrammemsize_val,	NULVAL				},

	{FLAGTRUE,	false,	"AAPL,aux-power-connected",	NULL,			DWRVAL(1)			},
	{FLAGTRUE,	false,	"AAPL00,DualLink",		get_dual_link_val,	NULVAL				},

//	{FLAGTRUE,	false,	"AAPL,backlight-control",	NULL,			DWRVAL(1)			},

	{FLAGTRUE,	false,	"ATY,bin_image",		get_binimage_val,	NULVAL				},
	{FLAGTRUE,	false,	"ATY,Copyright",		NULL,			STRVAL("Copyright AMD Inc. All Rights Reserved. 2005-2011") },
	{FLAGTRUE,	false,	"ATY,EFIVersion",		NULL,			STRVAL("01.00.3180")		},
	{FLAGTRUE,	false,	"ATY,Card#",			get_romrevision_val,	NULVAL				},
//	{FLAGTRUE,	false,	"ATY,Rom#",			NULL,			STRVAL("www.amd.com")		},
	{FLAGTRUE,	false,	"ATY,VendorID",			NULL,			WRDVAL(0x1002)			},
	{FLAGTRUE,	false,	"ATY,DeviceID",			get_deviceid_val,	NULVAL				},

//	{FLAGTRUE,	false,	"ATY,MCLK",			get_mclk_val,		NULVAL				},
//	{FLAGTRUE,	false,	"ATY,SCLK",			get_sclk_val,		NULVAL				},
	{FLAGTRUE,	false,	"ATY,RefCLK",			get_refclk_val,		DWRVAL(0x0a8c)			},
	
	{FLAGTRUE,	false,	"ATY,PlatformInfo",		get_platforminfo_val,	NULVAL				},

	{FLAGTRUE,	false,	"name",				get_nameparent_val,	NULVAL				},
	{FLAGTRUE,	false,	"device_type",			get_nameparent_val,	NULVAL				},
	{FLAGTRUE,	false,	"model",			get_model_val,		STRVAL("ATI Radeon")		},
//	{FLAGTRUE,	false,	"VRAM,totalsize",		get_vramtotalsize_val,	NULVAL				},
	{FLAGTRUE,	false,	"hda-gfx",			get_hdmiaudio,		NULVAL				},

	{FLAGTRUE,	false,	NULL,				NULL,			NULVAL				}
};

bool get_bootdisplay_val(value_t *val)
{
	static uint32_t v = 0;
	
	if (v) {
		return false;
	}
	if (!card->posted) {
		return false;
	}
	v = 1;
	val->type = kCst;
	val->size = 4;
	val->data = (uint8_t *)&v;
	
	return true;
}

bool get_dual_link_val(value_t *val)
{
	bool doit = false;
	if(getBoolForKey(kEnableDualLink, &doit, &bootInfo->chameleonConfig) && doit) {
		uint8_t AAPL00_value[] = {0x01, 0x00, 0x00, 0x00};
		val->type = kStr;
		val->size = strlen("AAPL00,DualLink") + 1;
		val->data = (uint8_t *)AAPL00_value;
		return true;
	}
	return false;
}

bool get_hdmiaudio(value_t * val)
{
	bool doit = false;
	if(getBoolForKey(kEnableHDMIAudio, &doit, &bootInfo->chameleonConfig) && doit) {
		val->type = kStr;
		val->size = strlen("onboard-1") + 1;
		val->data = (uint8_t *)"onboard-1";

		return true;
	}
	return false;
}

bool get_vrammemory_val(value_t *val)
{
	return false;
}

static const char* dtyp[] = {"LCD", "CRT", "DVI", "NONE"};
static uint32_t dti = 0;

bool get_display_type(value_t *val)
{
	dti++;
	if (dti > 3)
	{
		dti = 0;
	}
	val->type = kStr;
	val->size = 4;
	val->data = (uint8_t *)dtyp[dti];

	return true;
}

bool get_name_val(value_t *val)
{
	val->type = aty_name.type;
	val->size = aty_name.size;
	val->data = aty_name.data;
	
	return true;
}

bool get_nameparent_val(value_t *val)
{
	val->type = aty_nameparent.type;
	val->size = aty_nameparent.size;
	val->data = aty_nameparent.data;
	
	return true;
}

bool get_model_val(value_t *val)
{
	if (!card->info->model_name)
		return false;
	
	val->type = kStr;
	val->size = (uint32_t)strlen(card->info->model_name);
	val->data = (uint8_t *)card->info->model_name;
	
	return true;
}

bool get_conntype_val(value_t *val)
{
//Connector types:
//0x00000010: VGA
//0x00000004: DL DVI-I
//0x00000200: SL DVI-I
//0x00000080: S-V
//0x00000800: HDMI
//0x00000400: DisplayPort
//0x00000002: LVDS

	return false;
}

bool get_vrammemsize_val(value_t *val)
{
	static int idx = -1;
	static uint64_t memsize;
	
	idx++;
	memsize = ((uint64_t)card->vram_size << 32);
	if (idx == 0)
	{
		memsize = memsize | (uint64_t)card->vram_size;
	}
	val->type = kCst;
	val->size = 8;
	val->data = (uint8_t *)&memsize;
	
	return true;
}

bool get_binimage_val(value_t *val)
{
	if (!card->rom)
	{
		return false;
	}
	val->type = kPtr;
	val->size = card->rom_size;
	val->data = card->rom;

	return true;
}

bool get_romrevision_val(value_t *val)
{
	char *cRev="109-B77101-00";
	uint8_t *rev;
	if (!card->rom)
	{
		val->type = kPtr;
		val->size = 13;
		val->data = malloc(val->size);
		if (!val->data) {
			return false;
		}
		memcpy(val->data, cRev, val->size);

		return true;
	}

	rev = card->rom + *(uint8_t *)(card->rom + OFFSET_TO_GET_ATOMBIOS_STRINGS_START);

	val->type = kPtr;
	val->size = (uint32_t)strlen((char *)rev);
	if ((val->size < 3) || (val->size > 30)) { //fool proof. Real value 13
		rev = (uint8_t *)cRev;
		val->size = 13;
	}
	val->data = malloc(val->size);

	if (!val->data)
	{
		return false;
	}

	memcpy(val->data, rev, val->size);
	
	return true;
}

bool get_deviceid_val(value_t *val)
{
	val->type = kCst;
	val->size = 2;
	val->data = (uint8_t *)&card->pci_dev->device_id;
	
	return true;
}

bool get_mclk_val(value_t *val)
{
	return false;
}

bool get_sclk_val(value_t *val)
{
	return false;
}

bool get_refclk_val(value_t *val)
{
	return false;
}

bool get_platforminfo_val(value_t *val)
{
	val->data = malloc(0x80);
	if (!val->data)
	{
		return false;
	}
	bzero(val->data, 0x80);
	
	val->type		= kPtr;
	val->size		= 0x80;
	val->data[0]	= 1;
	
	return true;
}

bool get_vramtotalsize_val(value_t *val)
{

	val->type = kCst;
	val->size = 4;
	val->data = (uint8_t *)&card->vram_size;
	
	return true;
}

void free_val(value_t *val)
{
	if (val->type == kPtr)
	{
		free(val->data);
	}
	
	bzero(val, sizeof(value_t));
}

void devprop_add_list(AtiDevProp devprop_list[])
{
	int i, pnum;
	value_t *val = malloc(sizeof(value_t));
	
	for (i = 0; devprop_list[i].name != NULL; i++)
	{
		if ((devprop_list[i].flags == FLAGTRUE) || (devprop_list[i].flags & card->flags))
		{
			if (devprop_list[i].get_value && devprop_list[i].get_value(val))
			{
				devprop_add_value(card->device, devprop_list[i].name, val->data, val->size);
				free_val(val);
				
				if (devprop_list[i].all_ports)
				{
					for (pnum = 1; pnum < card->ports; pnum++)
					{
						if (devprop_list[i].get_value(val))
						{
							devprop_list[i].name[1] = (uint8_t)(0x30 + pnum); // convert to ascii
							devprop_add_value(card->device, devprop_list[i].name, val->data, val->size);
							free_val(val);
						}
					}
					devprop_list[i].name[1] = 0x30; // write back our "@0," for a next possible card
				}
			}
			else
			{
				if (devprop_list[i].default_val.type != kNul)
				{
					devprop_add_value(card->device, devprop_list[i].name,
						devprop_list[i].default_val.type == kCst ?
						(uint8_t *)&(devprop_list[i].default_val.data) : devprop_list[i].default_val.data,
						devprop_list[i].default_val.size);
				}
				
				if (devprop_list[i].all_ports)
				{
					for (pnum = 1; pnum < card->ports; pnum++)
					{
						if (devprop_list[i].default_val.type != kNul)
						{
							devprop_list[i].name[1] = (uint8_t)(0x30 + pnum); // convert to ascii
							devprop_add_value(card->device, devprop_list[i].name,
								devprop_list[i].default_val.type == kCst ?
								(uint8_t *)&(devprop_list[i].default_val.data) : devprop_list[i].default_val.data,
								devprop_list[i].default_val.size);
						}
					}
					devprop_list[i].name[1] = 0x30; // write back our "@0," for a next possible card
				}
			}
		}
	}

	free(val);
}

bool validate_rom(option_rom_header_t *rom_header, pci_dt_t *pci_dev)
{
	option_rom_pci_header_t *rom_pci_header;
	
	if (rom_header->signature != 0xaa55)
	{
		//verbose("invalid ROM signature %x\n", rom_header->signature);
		return false;
	}

	rom_pci_header = (option_rom_pci_header_t *)((uint8_t *)rom_header + rom_header->pci_header_offset);
	
	if (rom_pci_header->signature != 0x52494350)
	{
		//verbose("invalid ROM header %x\n", rom_pci_header->signature);
		return false;
	}
	
	if (rom_pci_header->vendor_id != pci_dev->vendor_id || rom_pci_header->device_id != pci_dev->device_id)
	{
		//verbose("invalid ROM vendor=%x deviceID=%d\n", rom_pci_header->vendor_id, rom_pci_header->device_id);
		return false;
	}
	
	return true;
}

bool load_vbios_file(const char *key, uint16_t vendor_id, uint16_t device_id, uint32_t subsys_id)
{
	int fd;
	char file_name[64];
	bool do_load = false;

	getBoolForKey(key, &do_load, &bootInfo->chameleonConfig);
	if (!do_load)
	{
		return false;
	}

	sprintf(file_name, "/Extra/%04x_%04x_%08x.rom", vendor_id, device_id, subsys_id);
	if ((fd = open_bvdev("bt(0,0)", file_name, 0)) < 0)
	{
		return false;
	}

	card->rom_size = file_size(fd);
	card->rom = malloc(card->rom_size);
	if (!card->rom)
	{
		return false;
	}

	read(fd, (char *)card->rom, card->rom_size);

	if (!validate_rom((option_rom_header_t *)card->rom, card->pci_dev))
	{
		verbose("validate_rom fails\n");
		card->rom_size = 0;
		card->rom = 0;
		return false;
	}
	
	card->rom_size = ((option_rom_header_t *)card->rom)->rom_size * 512;

	close(fd);

	return true;
}

void get_vram_size(void)
{
	ati_chip_family_t chip_family = card->info->chip_family;
	
	card->vram_size = 0;

	if (chip_family >= CHIP_FAMILY_CEDAR)
	{
		// size in MB on evergreen
		// XXX watch for overflow!!!
		card->vram_size = RegRead32(R600_CONFIG_MEMSIZE) * 1024 * 1024;
	}
	else
	{
		if (chip_family >= CHIP_FAMILY_R600)
		{
			card->vram_size = RegRead32(R600_CONFIG_MEMSIZE);
		}
	}
}

bool read_vbios(bool from_pci)
{
	option_rom_header_t *rom_addr;
	
	if (from_pci)
	{
		rom_addr = (option_rom_header_t *)(pci_config_read32(card->pci_dev->dev.addr, PCI_ROM_ADDRESS) & ~0x7ff);
		verbose(" @0x%x\n", rom_addr);
	}
	else
	{
		rom_addr = (option_rom_header_t *)0xc0000;
	}
	
	if (!validate_rom(rom_addr, card->pci_dev))
	{
		verbose("There is no ROM @0x%x\n", rom_addr);
		return false;
	}
	card->rom_size = rom_addr->rom_size * 512;
	if (!card->rom_size)
	{
		return false;
	}
	
	card->rom = malloc(card->rom_size);
	if (!card->rom)
	{
		return false;
	}
	
	memcpy(card->rom, (void *)rom_addr, card->rom_size);
	
	return true;
}

bool read_disabled_vbios(void)
{
	bool ret = false;
	ati_chip_family_t chip_family = card->info->chip_family;
	
	if (chip_family >= CHIP_FAMILY_RV770)
	{
		uint32_t viph_control		= RegRead32(RADEON_VIPH_CONTROL);
		uint32_t bus_cntl		= RegRead32(RADEON_BUS_CNTL);
		uint32_t d1vga_control		= RegRead32(AVIVO_D1VGA_CONTROL);
		uint32_t d2vga_control		= RegRead32(AVIVO_D2VGA_CONTROL);
		uint32_t vga_render_control	= RegRead32(AVIVO_VGA_RENDER_CONTROL);
		uint32_t rom_cntl		= RegRead32(R600_ROM_CNTL);
		uint32_t cg_spll_func_cntl	= 0;
		uint32_t cg_spll_status;
		
		// disable VIP
		RegWrite32(RADEON_VIPH_CONTROL, (viph_control & ~RADEON_VIPH_EN));
		
		// enable the rom
		RegWrite32(RADEON_BUS_CNTL, (bus_cntl & ~RADEON_BUS_BIOS_DIS_ROM));
		
		// Disable VGA mode
		RegWrite32(AVIVO_D1VGA_CONTROL, (d1vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
		RegWrite32(AVIVO_D2VGA_CONTROL, (d2vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
		RegWrite32(AVIVO_VGA_RENDER_CONTROL, (vga_render_control & ~AVIVO_VGA_VSTATUS_CNTL_MASK));
		
		if (chip_family == CHIP_FAMILY_RV730)
		{
			cg_spll_func_cntl = RegRead32(R600_CG_SPLL_FUNC_CNTL);
			
			// enable bypass mode
			RegWrite32(R600_CG_SPLL_FUNC_CNTL, (cg_spll_func_cntl | R600_SPLL_BYPASS_EN));
			
			// wait for SPLL_CHG_STATUS to change to 1
			cg_spll_status = 0;
			while (!(cg_spll_status & R600_SPLL_CHG_STATUS))
			{
				cg_spll_status = RegRead32(R600_CG_SPLL_STATUS);
			}
			
			RegWrite32(R600_ROM_CNTL, (rom_cntl & ~R600_SCK_OVERWRITE));
		}
		else
		{
			RegWrite32(R600_ROM_CNTL, (rom_cntl | R600_SCK_OVERWRITE));
		}

		ret = read_vbios(true);
		
		// restore regs
		if (chip_family == CHIP_FAMILY_RV730)
		{
			RegWrite32(R600_CG_SPLL_FUNC_CNTL, cg_spll_func_cntl);
			
			// wait for SPLL_CHG_STATUS to change to 1
			cg_spll_status = 0;
			while (!(cg_spll_status & R600_SPLL_CHG_STATUS))
			cg_spll_status = RegRead32(R600_CG_SPLL_STATUS);
		}
		RegWrite32(RADEON_VIPH_CONTROL, viph_control);
		RegWrite32(RADEON_BUS_CNTL, bus_cntl);
		RegWrite32(AVIVO_D1VGA_CONTROL, d1vga_control);
		RegWrite32(AVIVO_D2VGA_CONTROL, d2vga_control);
		RegWrite32(AVIVO_VGA_RENDER_CONTROL, vga_render_control);
		RegWrite32(R600_ROM_CNTL, rom_cntl);
	}
	else
		if (chip_family >= CHIP_FAMILY_R600)
		{
			uint32_t viph_control				= RegRead32(RADEON_VIPH_CONTROL);
			uint32_t bus_cntl				= RegRead32(RADEON_BUS_CNTL);
			uint32_t d1vga_control				= RegRead32(AVIVO_D1VGA_CONTROL);
			uint32_t d2vga_control				= RegRead32(AVIVO_D2VGA_CONTROL);
			uint32_t vga_render_control			= RegRead32(AVIVO_VGA_RENDER_CONTROL);
			uint32_t rom_cntl				= RegRead32(R600_ROM_CNTL);
			uint32_t general_pwrmgt				= RegRead32(R600_GENERAL_PWRMGT);
			uint32_t low_vid_lower_gpio_cntl		= RegRead32(R600_LOW_VID_LOWER_GPIO_CNTL);
			uint32_t medium_vid_lower_gpio_cntl		= RegRead32(R600_MEDIUM_VID_LOWER_GPIO_CNTL);
			uint32_t high_vid_lower_gpio_cntl		= RegRead32(R600_HIGH_VID_LOWER_GPIO_CNTL);
			uint32_t ctxsw_vid_lower_gpio_cntl		= RegRead32(R600_CTXSW_VID_LOWER_GPIO_CNTL);
			uint32_t lower_gpio_enable			= RegRead32(R600_LOWER_GPIO_ENABLE);
			
			// disable VIP
			RegWrite32(RADEON_VIPH_CONTROL, (viph_control & ~RADEON_VIPH_EN));
			
			// enable the rom
			RegWrite32(RADEON_BUS_CNTL, (bus_cntl & ~RADEON_BUS_BIOS_DIS_ROM));
			
			// Disable VGA mode
			RegWrite32(AVIVO_D1VGA_CONTROL, (d1vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
			RegWrite32(AVIVO_D2VGA_CONTROL, (d2vga_control & ~(AVIVO_DVGA_CONTROL_MODE_ENABLE | AVIVO_DVGA_CONTROL_TIMING_SELECT)));
			RegWrite32(AVIVO_VGA_RENDER_CONTROL, (vga_render_control & ~AVIVO_VGA_VSTATUS_CNTL_MASK));
			RegWrite32(R600_ROM_CNTL, ((rom_cntl & ~R600_SCK_PRESCALE_CRYSTAL_CLK_MASK) | (1 << R600_SCK_PRESCALE_CRYSTAL_CLK_SHIFT) | R600_SCK_OVERWRITE));
			RegWrite32(R600_GENERAL_PWRMGT, (general_pwrmgt & ~R600_OPEN_DRAIN_PADS));
			RegWrite32(R600_LOW_VID_LOWER_GPIO_CNTL, (low_vid_lower_gpio_cntl & ~0x400));
			RegWrite32(R600_MEDIUM_VID_LOWER_GPIO_CNTL, (medium_vid_lower_gpio_cntl & ~0x400));
			RegWrite32(R600_HIGH_VID_LOWER_GPIO_CNTL, (high_vid_lower_gpio_cntl & ~0x400));
			RegWrite32(R600_CTXSW_VID_LOWER_GPIO_CNTL, (ctxsw_vid_lower_gpio_cntl & ~0x400));
			RegWrite32(R600_LOWER_GPIO_ENABLE, (lower_gpio_enable | 0x400));
			
			ret = read_vbios(true);
			
			// restore regs
			RegWrite32(RADEON_VIPH_CONTROL, viph_control);
			RegWrite32(RADEON_BUS_CNTL, bus_cntl);
			RegWrite32(AVIVO_D1VGA_CONTROL, d1vga_control);
			RegWrite32(AVIVO_D2VGA_CONTROL, d2vga_control);
			RegWrite32(AVIVO_VGA_RENDER_CONTROL, vga_render_control);
			RegWrite32(R600_ROM_CNTL, rom_cntl);
			RegWrite32(R600_GENERAL_PWRMGT, general_pwrmgt);
			RegWrite32(R600_LOW_VID_LOWER_GPIO_CNTL, low_vid_lower_gpio_cntl);
			RegWrite32(R600_MEDIUM_VID_LOWER_GPIO_CNTL, medium_vid_lower_gpio_cntl);
			RegWrite32(R600_HIGH_VID_LOWER_GPIO_CNTL, high_vid_lower_gpio_cntl);
			RegWrite32(R600_CTXSW_VID_LOWER_GPIO_CNTL, ctxsw_vid_lower_gpio_cntl);
			RegWrite32(R600_LOWER_GPIO_ENABLE, lower_gpio_enable);
		}

	return ret;
}

bool radeon_card_posted(void)
{
	uint32_t reg;
	
	// first check CRTCs
	reg = RegRead32(RADEON_CRTC_GEN_CNTL) | RegRead32(RADEON_CRTC2_GEN_CNTL);
	if (reg & RADEON_CRTC_EN)
	{
		return true;
	}
	
	// then check MEM_SIZE, in case something turned the crtcs off
	reg = RegRead32(R600_CONFIG_MEMSIZE);
	if (reg)
	{
		return true;
	}
	
	return false;
}

#if 0
bool devprop_add_pci_config_space(void)
{
	int offset;
	
	uint8_t *config_space = malloc(0x100);
	if (!config_space)
	{
		return false;
	}
	
	for (offset = 0; offset < 0x100; offset += 4)
	{
		config_space[offset / 4] = pci_config_read32(card->pci_dev->dev.addr, offset);
	}
	
	devprop_add_value(card->device, "ATY,PCIConfigSpace", config_space, 0x100);
	free(config_space);
	
	return true;
}
#endif

static bool init_card(pci_dt_t *pci_dev)
{
	bool	add_vbios = true;
	char	name[24];
	char	name_parent[24];
	int		i;
	int		n_ports = 0;

	card = malloc(sizeof(card_t));
	if (!card)
	{
		return false;
	}
	bzero(card, sizeof(card_t));

	card->pci_dev = pci_dev;
	
	for (i = 0; radeon_cards[i].device_id ; i++)
	{
		if (radeon_cards[i].device_id == pci_dev->device_id)
		{
			if ((radeon_cards[i].subsys_id == 0x00000000) || (radeon_cards[i].subsys_id == pci_dev->subsys_id.subsys_id))
			{
				card->info = &radeon_cards[i];
				break;
			}
		}
	}

	if (card->info == NULL) // Jief
	{
		verbose("Unsupported ATI card! Device ID: [%04x:%04x] Subsystem ID: [%04x:%04x] \n", 
				pci_dev->vendor_id, pci_dev->device_id, pci_dev->subsys_id.subsys.vendor_id, pci_dev->subsys_id.subsys.device_id);
		return false;
	}
   	verbose("Found ATI card! Device ID:[%04x:%04x] Subsystem ID:[%08x] - Radeon [%04x:%08x] %s\n", 
		pci_dev->vendor_id, pci_dev->device_id, pci_dev->subsys_id, card->info->device_id, card->info->subsys_id, card->info->model_name);
	
	card->fb		= (uint8_t *)(pci_config_read32(pci_dev->dev.addr, PCI_BASE_ADDRESS_0) & ~0x0f);
	card->mmio		= (uint8_t *)(pci_config_read32(pci_dev->dev.addr, PCI_BASE_ADDRESS_2) & ~0x0f);
	card->io		= (uint8_t *)(pci_config_read32(pci_dev->dev.addr, PCI_BASE_ADDRESS_4) & ~0x03);

	verbose("Framebuffer @0x%08X  MMIO @0x%08X	I/O Port @0x%08X ROM Addr @0x%08X\n",
		card->fb, card->mmio, card->io, pci_config_read32(pci_dev->dev.addr, PCI_ROM_ADDRESS));
	
	card->posted = radeon_card_posted();
	verbose("ATI card %s, ", card->posted ? "POSTed" : "non-POSTed");
	verbose("\n");
	get_vram_size();
	
	getBoolForKey(kATYbinimage, &add_vbios, &bootInfo->chameleonConfig);
	
	if (add_vbios)
	{
		if (!load_vbios_file(kUseAtiROM, pci_dev->vendor_id, pci_dev->device_id, pci_dev->subsys_id.subsys_id))
		{
			verbose("reading Video BIOS from %s", card->posted ? "legacy space" : "PCI ROM");
			if (card->posted)
			{
				read_vbios(false);
			}
			else
			{
				read_disabled_vbios();
			}
			verbose("Video BIOS read from file\n");
		}
	}


	if (card->info->chip_family >= CHIP_FAMILY_CEDAR)
	{
		verbose("ATI Radeon EVERGREEN family\n");
		card->flags |= EVERGREEN;
	}


	// Check AtiConfig key for a framebuffer name,
	card->cfg_name = getStringForKey(kAtiConfig, &bootInfo->chameleonConfig);

	// if none,
	if (!card->cfg_name)
	{
		// use cfg_name on radeon_cards, to retrive the default name from card_configs,
		card->cfg_name = card_configs[card->info->cfg_name].name;
		
		// which means one of the fb's or kNull
		verbose("Framebuffer set to device's default: %s\n", card->cfg_name);
	}
	else
	{
		// else, use the fb name returned by AtiConfig.
		verbose("(AtiConfig) Framebuffer set to: %s\n", card->cfg_name);
	}

	// Check AtiPorts key for nr of ports,
	card->ports = getIntForKey(kAtiPorts, &n_ports, &bootInfo->chameleonConfig);
	// if a value bigger than 0 ?? is found, (do we need >= 0 ?? that's null FB on card_configs)
	if (n_ports > 0)
	{
		card->ports = (uint8_t)n_ports; // use it.
		verbose("(AtiPorts) # of ports set to: %d\n", card->ports);
	}
	else
	{
		// else, match cfg_name with card_configs list and retrive default nr of ports.
		for (i = 0; i < kCfgEnd; i++)
		{
			if (strcmp(card->cfg_name, card_configs[i].name) == 0)
			{
				card->ports = card_configs[i].ports; // default
			}
		}

		verbose("# of ports set to framebuffer's default: %d\n", card->ports);
	}


	sprintf(name, "ATY,%s", card->cfg_name);
	aty_name.type = kStr;
	aty_name.size = (uint32_t)strlen(name);
	aty_name.data = (uint8_t *)name;

	sprintf(name_parent, "ATY,%sParent", card->cfg_name);
	aty_nameparent.type = kStr;
	aty_nameparent.size = (uint32_t)strlen(name_parent);
	aty_nameparent.data = (uint8_t *)name_parent;
	
	return true;
}

bool setup_ati_devprop(pci_dt_t *ati_dev)
{
	char *devicepath;

	if (!init_card(ati_dev))
	{
		return false;
	}

	// -------------------------------------------------
	// Find a better way to do this (in device_inject.c)
	if (!string)
	{
		string = devprop_create_string();
	}
	devicepath = get_pci_dev_path(ati_dev);
	card->device = devprop_add_device(string, devicepath);
	if (!card->device)
	{
		return false;
	}
	// -------------------------------------------------

	if (getBoolForKey(kUseIntelHDMI, &doit, &bootInfo->chameleonConfig) && doit)
	{
		devprop_add_value(card->device, "hda-gfx", (uint8_t *)"onboard-2", 10);
	}
	else
	{
		devprop_add_value(card->device, "hda-gfx", (uint8_t *)"onboard-1", 10);
	}

#if 0
	uint64_t fb	= (uint32_t)card->fb;
	uint64_t mmio	= (uint32_t)card->mmio;
	uint64_t io	= (uint32_t)card->io;
	devprop_add_value(card->device, "ATY,FrameBufferOffset", &fb, 8);
	devprop_add_value(card->device, "ATY,RegisterSpaceOffset", &mmio, 8);
	devprop_add_value(card->device, "ATY,IOSpaceOffset", &io, 8);
#endif

	devprop_add_list(ati_devprop_list);

	// -------------------------------------------------
	// Find a better way to do this (in device_inject.c)
	//Azi: XXX tried to fix a malloc error in vain; this is related to XCode 4 compilation!
	stringdata = malloc(sizeof(uint8_t) * string->length);
	memcpy(stringdata, (uint8_t*)devprop_generate_string(string), string->length);
	stringlength = string->length;
	// -------------------------------------------------

	verbose("ATI %s %s %dMB (%s) [%04x:%04x] (subsys [%04x:%04x]):: %s\n",
			chip_family_name[card->info->chip_family], card->info->model_name,
			(uint32_t)(card->vram_size / (1024 * 1024)), card->cfg_name,
			ati_dev->vendor_id, ati_dev->device_id,
			ati_dev->subsys_id.subsys.vendor_id, ati_dev->subsys_id.subsys.device_id,
			devicepath);

	verbose("---------------------------------------------\n");

	free(card);

	return true;
}
