/*
 * SMBIOS Table Patcher, part of the Chameleon Boot Loader Project
 *
 * Copyright 2010 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */


#include "libsaio.h"
#include "bootstruct.h"
#include "smbios_getters.h"

#ifndef DEBUG_SMBIOS
#define DEBUG_SMBIOS 0
#endif

#if DEBUG_SMBIOS==2
#define DBG(x...)	printf(x)
#elif DEBUG_SMBIOS==1
#define DBG(x...) msglog(x)
#else
#define DBG(x...)	
#endif

/* ASSUMPTION: 16KB should be enough for the whole thing */
#define SMB_ALLOC_SIZE	16384


//-------------------------------------------------------------------------------------------------------------------------
// SMBIOS Plist Keys
//-------------------------------------------------------------------------------------------------------------------------
/* BIOS Information */
#define kSMBBIOSInformationVendorKey				"SMbiosvendor"
#define kSMBBIOSInformationVersionKey				"SMbiosversion"
#define kSMBBIOSInformationReleaseDateKey			"SMbiosdate"

/* System Information */
#define kSMBSystemInformationManufacturerKey		"SMmanufacturer"
#define kSMBSystemInformationProductNameKey			"SMproductname"
#define kSMBSystemInformationVersionKey				"SMsystemversion"
#define kSMBSystemInformationSerialNumberKey		"SMserial"
#define kSMBSystemInformationFamilyKey				"SMfamily"

/* Base Board */
#define kSMBBaseBoardManufacturerKey				"SMboardmanufacturer"
#define kSMBBaseBoardProductKey						"SMboardproduct"

/* Processor Information */
#define kSMBProcessorInformationExternalClockKey	"SMexternalclock"
#define kSMBProcessorInformationMaximumClockKey		"SMmaximalclock"
#define kSMBProcessorInformationCurrentClockKey		"SMcurrentclock"

/* Memory Device */
#define kSMBMemoryDeviceDeviceLocatorKey			"SMmemdevloc"
#define kSMBMemoryDeviceBankLocatorKey				"SMmembankloc"
#define kSMBMemoryDeviceMemoryTypeKey				"SMmemtype"
#define kSMBMemoryDeviceMemorySpeedKey				"SMmemspeed"
#define kSMBMemoryDeviceManufacturerKey				"SMmemmanufacturer"
#define kSMBMemoryDeviceSerialNumberKey				"SMmemserial"
#define kSMBMemoryDevicePartNumberKey				"SMmempart"

/* Apple Specific */
#define kSMBOemProcessorTypeKey						"SMcputype"
#define kSMBOemProcessorBusSpeedKey					"SMbusspeed"

//-------------------------------------------------------------------------------------------------------------------------
// Default SMBIOS Data
//-------------------------------------------------------------------------------------------------------------------------
static char fake_serial[11];

static char const sn_gen_pn_str[36] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"; 

typedef struct {
    const char* key;
    const char* value;
} SMStrEntryPair;

// defaults for a MacBook
static const SMStrEntryPair const sm_macbook_defaults[]={
	{"SMbiosvendor",            "Apple Inc."                    },
	{"SMbiosversion",           "MB41.88Z.00C1.B00.0802091535"	},
	{"SMbiosdate",              "02/09/2008"                    },
	{"SMmanufacter",            "Apple Inc."                    },
	{"SMproductname",           "MacBook4,1"                    },
	{"SMsystemversion",         "1.0"                           },
	{"SMserial",                "RM83064H0P1"                   },
    {"SMserialProductCountry",	"RM"                            },
    {"SMserialYear",            "8"                             },
	{"SMserialWeek",            "30"                            },
	{"SMserialProductNumber",	"64H"                           },
	{"SMserialModel",			"0P1"                           },
	{"SMfamily",                "MacBook"                       },
	{"SMboardmanufacter",       "Apple Inc."                    },
	{"SMboardproduct",          "Mac-F22788A9"                  },
	{ "",""	}
};

// defaults for a MacBook Pro
static const SMStrEntryPair const sm_macbookpro_defaults[]={
	{"SMbiosvendor",            "Apple Inc."                    },
	{"SMbiosversion",           "MBP41.88Z.00C1.B03.0802271651"	},
	{"SMbiosdate",              "02/27/2008"                    },
	{"SMmanufacter",            "Apple Inc."                    },
	{"SMproductname",           "MacBookPro4,1"                 },
	{"SMsystemversion",         "1.0"                           },
	{"SMserial",                "W88198N6YJX"                   },
    {"SMserialProductCountry",	"W8"                            },
    {"SMserialYear",            "8"                             },
	{"SMserialWeek",            "19"                            },
	{"SMserialProductNumber",	"8N6"                           },
	{"SMserialModel",			"YJX"                           },
	{"SMfamily",                "MacBookPro"                    },
	{"SMboardmanufacter",       "Apple Inc."                    },
	{"SMboardproduct",          "Mac-F42C89C8"                  },
	{ "",""	}
};

// defaults for a Mac mini 
static const SMStrEntryPair const sm_macmini_defaults[]={
	{"SMbiosvendor",            "Apple Inc."                    },
	{"SMbiosversion",           "MM21.88Z.009A.B00.0706281359"	},
	{"SMbiosdate",              "06/28/2007"                    },
	{"SMmanufacter",            "Apple Inc."                    },
	{"SMproductname",           "Macmini2,1"                    },
	{"SMsystemversion",         "1.0"                           },
	{"SMserial",                "YM8054BYYL2"                   },
    {"SMserialProductCountry",	"YM"                            },
    {"SMserialYear",            "8"                             },
	{"SMserialWeek",            "05"                            },
	{"SMserialProductNumber",	"4BY"                           },
	{"SMserialModel",			"YL2"                           },
	{"SMfamily",                "Napa Mac"                      },
	{"SMboardmanufacter",       "Apple Inc."                    },
	{"SMboardproduct",          "Mac-F4208EAA"                  },
	{ "",""	}
};

// defaults for an iMac
static const SMStrEntryPair const sm_imac_defaults[]={
	{"SMbiosvendor",            "Apple Inc."                    },
	{"SMbiosversion",           "IM71.88Z.007A.B03.0803051705"	},
	{"SMbiosdate",              "03/05/2008"                    },
	{"SMmanufacter",            "Apple Inc."                    },
	{"SMproductname",           "iMac7,1"                       },	
	{"SMsystemversion",         "1.0"                           },
	{"SMserial",                "W87410PWX87"                   },
    {"SMserialProductCountry",	"W8"                            },
    {"SMserialYear",            "7"                             },
	{"SMserialWeek",            "41"                            },
	{"SMserialProductNumber",	"0PW"                           },
	{"SMserialModel",			"X87"                           },
	{"SMfamily",                "Mac"                           },
	{"SMboardmanufacter",       "Apple Inc."                    },
	{"SMboardproduct",          "Mac-F4238CC8"                  },
	{ "",""	}
};

// defaults for a Mac Pro
static const SMStrEntryPair const sm_macpro_defaults[]={
	{"SMbiosvendor",            "Apple Computer, Inc."			},
	{"SMbiosversion",           "MP31.88Z.006C.B02.0801021250"	},
	{"SMbiosdate",              "01/02/2008"					},
	{"SMmanufacter",            "Apple Computer, Inc."			},
	{"SMproductname",           "MacPro3,1"						},
	{"SMsystemversion",         "1.0"							},
	{"SMserial",                "G88014V4XYK"					},
    {"SMserialProductCountry",	"G8"                            },
    {"SMserialYear",            "8"                             },
	{"SMserialWeek",            "01"                            },
	{"SMserialProductNumber",	"4V4"                           },
	{"SMserialModel",			"XYK"                           },
	{"SMfamily",                "MacPro"						},
	{"SMboardmanufacter",       "Apple Computer, Inc."			},
	{"SMboardproduct",          "Mac-F42C88C8"					},
	{ "",""	}
};

// defaults for an iMac11,1 core i3/i5/i7
static const SMStrEntryPair const sm_imac_core_defaults[]={
	{"SMbiosvendor",            "Apple Inc."					},
	{"SMbiosversion",           "IM111.88Z.0034.B00.0910301727"	},
	{"SMbiosdate",              "10/30/2009"					},
	{"SMmanufacter",            "Apple Inc."					},
	{"SMproductname",           "iMac11,1"						},	
	{"SMsystemversion",         "1.0"							},
	{"SMserial",                "W89470DZ5RU"					},
    {"SMserialProductCountry",	 "W8"                           },
    {"SMserialYear",             "9"                            },
	{"SMserialWeek",             "47"                           },
	{"SMserialProductNumber",	 "0DZ"                          },
	{"SMserialModel",            "5RU"                          },
	{"SMfamily",                "iMac"							},
	{"SMboardmanufacter",       "Apple Inc."                    },
	{"SMboardproduct",          "Mac-F2268DAE"					},
	{ "",""	}
};

// defaults for an iMac12,1 : todo: populate correctly 
static const SMStrEntryPair const sm_imac_sandy_defaults[]={
	{"SMbiosvendor",             "Apple Inc."					},
	{"SMbiosversion",            "IM121.88Z.0047.B00.1102091756"},
	{"SMbiosdate",               "10/30/2011"					},
	{"SMmanufacter",             "Apple Inc."					},
	{"SMproductname",            "iMac12,1"						},	
	{"SMsystemversion",          "1.0"							},
	{"SMserial",                 "W89470DZ5RU"					},
    {"SMserialProductCountry",	 "W8"                           },
    {"SMserialYear",             "9"                            },
	{"SMserialWeek",             "47"                           },
	{"SMserialProductNumber",	 "0DZ"                          },
	{"SMserialModel",            "5RU"                          },
	{"SMfamily",                 "iMac"							},
	{"SMboardmanufacter",        "Apple Inc."                   },
	{"SMboardproduct",           "Mac-F2268DAE"					},
	{ "",""	}
};

// defaults for a Mac Pro 4,1 core i7/Xeon
static const SMStrEntryPair const sm_macpro_core_defaults[]={
	{"SMbiosvendor",            "Apple Computer, Inc."			},
	{"SMbiosversion",           "MP41.88Z.0081.B03.0902231259"	},
	{"SMbiosdate",              "02/23/2009"					},
	{"SMmanufacter",            "Apple Inc."                    },
	{"SMproductname",           "MacPro4,1"						},
	{"SMsystemversion",         "1.0"							},
	{"SMserial",                "CK91601V8Q0"					},
    {"SMserialProductCountry",	"CK"                            },
    {"SMserialYear",            "9"                             },
	{"SMserialWeek",            "16"                            },
	{"SMserialProductNumber",	"01V"                           },
	{"SMserialModel",			"8Q0"                           },
	{"SMfamily",                "MacPro"						},
	{"SMboardmanufacter",       "Apple Computer, Inc."			},
	{"SMboardproduct",          "Mac-F221BEC8"					},
	{ "",""	}
};

// defaults for a Mac Pro 5,1 Westmere
static const SMStrEntryPair const sm_macpro_westmere_defaults[]={
	{"SMbiosvendor",            "Apple Computer, Inc."			},
	{"SMbiosversion",           "MP51.88Z.007F.B00.1008031144"	},
	{"SMbiosdate",              "08/03/2010"					},
	{"SMmanufacter",            "Apple Inc."                    },
	{"SMproductname",           "MacPro5,1"						},
	{"SMsystemversion",         "0.0"							},
	{"SMserial",                "YM0330U7EUH"					},
    {"SMserialProductCountry",	"YM"                            },
    {"SMserialYear",            "0"                             },
	{"SMserialWeek",            "33"                            },
	{"SMserialProductNumber",	"0U7"                           },
	{"SMserialModel",			"EUH"                           },
	{"SMfamily",                "MacPro"						},
	{"SMboardmanufacter",       "Apple Computer, Inc."			},
	{"SMboardproduct",          "Mac-F221BEC8"					},
	{ "",""	}
};

// default for a Xserve
static const SMStrEntryPair const sm_xserve_defaults[]={
    {"SMbiosvendor",            "Apple Inc."					},
    {"SMbiosversion",           "XS21.88Z.006C.B06.0804011317"	},
    {"SMbiosdate",              "04/01/2008"					},
    {"SMmanufacter",            "Apple Inc."					},
    {"SMproductname",           "Xserve2,1"						},
    {"SMsystemversion",         "1.0"							},
    {"SMserial",                "CK816033X8S"					},
    {"SMserialProductCountry",	"CK"                            },
    {"SMserialYear",            "8"                             },
	{"SMserialWeek",            "16"                            },
	{"SMserialProductNumber",	"033"                           },
	{"SMserialModel",			"X8S"                           },
    {"SMfamily",                "Xserve"						},
    {"SMboardmanufacter",       "Apple Inc."					},
    {"SMboardproduct",          "Mac-F42289C8"					},
 	{ "",""	}
};

typedef struct {
    const char* code;
    const char* info;
} SMProductCountry;

static const SMProductCountry const sm_country_list[]={
    {"1C",		"China"                                 },
    {"2Z",		"Refurbished"                           },
    {"4H",		"China"                                 },
    {"5K",		"China"                                 },
    {"8H",		"China"                                 },
    {"5D",		"China"                                 },
    {"7J",		"China "                                },
    {"CK",		"Cork "                                 },
    /*{"E",		"Singapur"                              },*/
    {"EE",		"Taiwan"                                },
    /*{"F",		"Fremont "                              },*/
    {"FC",		"Fountain "                             },
    {"G8",		"USA"                                   },
    {"GQ",		"Refurbished"                           },
    {"PT",		"Korea"                                 },
    {"CY",		"Korea"                                 },
    {"QT",		"Taiwan"                                },
    {"QP",		"China"                                 },
    {"RN",		"Mexico"                                },
    {"RM",		"Refurbished/Remanufactured"			},
    {"SG",		"Singapore"                             },
    {"UV",		"Taiwan"                                },
    {"U2",		"Taiwan"                                },
    {"V7",		"Taiwan"                                },
    {"VM",		"China"                                 },
    {"W8",		"Shanghai"                              },
    {"WQ",		"China"                                 },
    {"XA",		"Elk Grove Sacramento"					},
    {"XB",		"Elk Grove Sacramento"					},
    {"YM",		"China /Konfiguriert"					}
};

#define getFieldOffset(struct, field)	((uint8_t)(uint32_t)&(((struct *)0)->field))

typedef struct {
	SMBStructHeader *orig;
	SMBStructHeader *new;
} SMBStructPtrs;

typedef struct {
	const char *vendor;
	const char *version;
	const char *releaseDate;
} defaultBIOSInfo_t;

defaultBIOSInfo_t defaultBIOSInfo;

typedef struct {
	const char *manufacturer;
	const char *productName;
	const char *version;
	const char *serialNumber;
	const char *family;
} defaultSystemInfo_t;

defaultSystemInfo_t defaultSystemInfo;

typedef struct {
	const char *manufacturer;
	const char *product;
} defaultBaseBoard_t;

defaultBaseBoard_t defaultBaseBoard;

typedef struct {
	uint8_t			type;
	SMBValueType	valueType;
	uint8_t			fieldOffset;
	char			*keyString;
	bool			(*getSMBValue)(returnType *);
	const char			**defaultValue;
} SMBValueSetter;

SMBValueSetter SMBSetters[] =                           
{                                                           
	//-------------------------------------------------------------------------------------------------------------------------
	// BIOSInformation
	//-------------------------------------------------------------------------------------------------------------------------
	{kSMBTypeBIOSInformation,	kSMBString,	getFieldOffset(SMBBIOSInformation, vendor),			kSMBBIOSInformationVendorKey,		
		NULL,	&defaultBIOSInfo.vendor			},
    
	{kSMBTypeBIOSInformation,	kSMBString,	getFieldOffset(SMBBIOSInformation, version),		kSMBBIOSInformationVersionKey,		
		NULL,	&defaultBIOSInfo.version		},
    
	{kSMBTypeBIOSInformation,	kSMBString,	getFieldOffset(SMBBIOSInformation, releaseDate),	kSMBBIOSInformationReleaseDateKey,	
		NULL,	&defaultBIOSInfo.releaseDate	},
    
	//-------------------------------------------------------------------------------------------------------------------------
	// SystemInformation
	//-------------------------------------------------------------------------------------------------------------------------
	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, manufacturer),	kSMBSystemInformationManufacturerKey,	
		NULL,	&defaultSystemInfo.manufacturer	},
    
	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, productName),	kSMBSystemInformationProductNameKey,	
		NULL,	&defaultSystemInfo.productName	},
    
	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, version),		kSMBSystemInformationVersionKey,		
		NULL,	&defaultSystemInfo.version		},
    
	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, serialNumber),	kSMBSystemInformationSerialNumberKey,	
		NULL,	&defaultSystemInfo.serialNumber	},
    
	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, skuNumber),	NULL,									
		NULL,	NULL							},
    
	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, family),		kSMBSystemInformationFamilyKey,			
		NULL,	&defaultSystemInfo.family		},
    
    
	//-------------------------------------------------------------------------------------------------------------------------
	// BaseBoard
	//-------------------------------------------------------------------------------------------------------------------------
	{kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, manufacturer),			kSMBBaseBoardManufacturerKey,	
		NULL,	&defaultBaseBoard.manufacturer	},
    
	{kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, product),				kSMBBaseBoardProductKey,		
		NULL,	&defaultBaseBoard.product		},
    
    {kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, version),				NULL,	NULL,	NULL},
    
    {kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, serialNumber),			NULL,	NULL,	NULL},
    
    {kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, assetTagNumber),		NULL,	NULL,	NULL},
    
    {kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, locationInChassis),	NULL,	NULL,	NULL},
    
    
	//-------------------------------------------------------------------------------------------------------------------------
	// ProcessorInformation
	//-------------------------------------------------------------------------------------------------------------------------
	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, socketDesignation),	NULL,	NULL,	NULL},
    
	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, manufacturer),		NULL,	NULL,	NULL},
    
	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, processorVersion),	NULL,	NULL,	NULL},
    
	{kSMBTypeProcessorInformation,	kSMBWord,	getFieldOffset(SMBProcessorInformation, externalClock),		kSMBProcessorInformationExternalClockKey,	
		getProcessorInformationExternalClock,	NULL},
    
	{kSMBTypeProcessorInformation,	kSMBWord,	getFieldOffset(SMBProcessorInformation, maximumClock),		kSMBProcessorInformationMaximumClockKey,	
		getProcessorInformationMaximumClock,	NULL},
	
	{kSMBTypeProcessorInformation,	kSMBWord,	getFieldOffset(SMBProcessorInformation, currentClock),		kSMBProcessorInformationCurrentClockKey,	
		getProcessorInformationCurrentClock,	NULL},
    
	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, serialNumber),		NULL,	NULL,	NULL},
    
	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, assetTag),			NULL,	NULL,	NULL},
    
	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, partNumber),		NULL,	NULL,	NULL},
    
	//-------------------------------------------------------------------------------------------------------------------------
	// Memory Device
	//-------------------------------------------------------------------------------------------------------------------------
	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, deviceLocator),	kSMBMemoryDeviceDeviceLocatorKey,	
		NULL,							NULL},
    
	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, bankLocator),	kSMBMemoryDeviceBankLocatorKey,		
		NULL,							NULL},
    
	{kSMBTypeMemoryDevice,	kSMBByte,	getFieldOffset(SMBMemoryDevice, memoryType),	kSMBMemoryDeviceMemoryTypeKey,		
		getSMBMemoryDeviceMemoryType,	NULL},
    
	{kSMBTypeMemoryDevice,	kSMBWord,	getFieldOffset(SMBMemoryDevice, memorySpeed),	kSMBMemoryDeviceMemorySpeedKey,		
		getSMBMemoryDeviceMemorySpeed,	NULL},
    
	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, manufacturer),	kSMBMemoryDeviceManufacturerKey,	
		getSMBMemoryDeviceManufacturer,	NULL},
    
	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, serialNumber),	kSMBMemoryDeviceSerialNumberKey,	
		getSMBMemoryDeviceSerialNumber,	NULL},
    
	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, assetTag),		NULL,	NULL,	NULL},
    
	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, partNumber),	kSMBMemoryDevicePartNumberKey,		
		getSMBMemoryDevicePartNumber,	NULL},
    
    
	//-------------------------------------------------------------------------------------------------------------------------
	// Apple Specific
	//-------------------------------------------------------------------------------------------------------------------------
	{kSMBTypeOemProcessorType,		kSMBWord,	getFieldOffset(SMBOemProcessorType, ProcessorType),			kSMBOemProcessorTypeKey,		
		getSMBOemProcessorType,			NULL},
    
	{kSMBTypeOemProcessorBusSpeed,	kSMBWord,	getFieldOffset(SMBOemProcessorBusSpeed, ProcessorBusSpeed),	kSMBOemProcessorBusSpeedKey,	
		getSMBOemProcessorBusSpeed,		NULL}
};

int numOfSetters = sizeof(SMBSetters) / sizeof(SMBValueSetter);

SMBEntryPoint *neweps	= 0;

static uint8_t stringIndex;	// increament when a string is added and set the field value accordingly
static uint8_t stringsSize;	// add string size

static SMBWord tableLength		= 0;
static SMBWord handle			= 0;
static SMBWord maxStructSize	= 0;
static SMBWord structureCount	= 0;

static void setDefaultSMBData(void);
static bool getSMBValueForKey(SMBStructHeader *structHeader, const char *keyString, const char **string, returnType *value);
static void setSMBStringForField(SMBStructHeader *structHeader, const char *string, uint8_t *field);
static bool setSMBValue(SMBStructPtrs *structPtr, int idx, returnType *value);
static void addSMBFirmwareVolume(SMBStructPtrs *structPtr);
static void addSMBMemorySPD(SMBStructPtrs *structPtr);
static void addSMBOemProcessorType(SMBStructPtrs *structPtr);
static void addSMBOemProcessorBusSpeed(SMBStructPtrs *structPtr);
static void addSMBEndOfTable(SMBStructPtrs *structPtr);
static void setSMBStruct(SMBStructPtrs *structPtr);
static void setupNewSMBIOSTable(SMBEntryPoint *eps, SMBStructPtrs *structPtr);
const char* sm_search_str(const SMStrEntryPair*	sm_defaults, const char * key);
const char* sm_get_random_productNumber(void);
const char* sm_get_random_week(void);
const char* sm_get_random_year(void);
const char* sm_get_random_country(void);

/* COPYRIGHT NOTICE: checksum8 from AppleSMBIOS */
static uint8_t checksum8( void * start, unsigned int length )
{
    uint8_t   csum = 0;
    uint8_t * cp = (uint8_t *) start;
    unsigned int i;
	
    for ( i = 0; i < length; i++)
        csum += *cp++;
	
    return csum;
}

const char *getDefaultSMBproductName(void)
{
	setDefaultSMBData();
	return defaultSystemInfo.productName;
}

const char *getDefaultSMBBoardProduct(void)
{
	setDefaultSMBData();
	return defaultBaseBoard.product;
}

const char* sm_search_str(const SMStrEntryPair*	sm_defaults, const char * key)
{
    int i;
    
    for (i=0; sm_defaults[i].key[0]; i++) {
		if (!strcmp (sm_defaults[i].key, key)) {
			return sm_defaults[i].value;
		}
	}
    
    // Shouldn't happen
    printf ("Error: no default for %s known\n", key);
    sleep (2);
    return "";
}

const char* sm_get_random_productNumber(void)
{
    static char str[4] = {0x00,0x00,0x00,0x00};
    if(str[0]  == 0)
    {           
        // Get randomized characters
        int rand_sn1 ;
        int rand_sn2 ;
        int rand_sn3 ;
        
        rand_sn1 = arc4random_unirange(0,35);
        rand_sn2 = arc4random_unirange(0,35);
        rand_sn3 = arc4random_unirange(0,35);
        
        // Append all charaters to the string 
        char tmp[2];
        bzero(tmp,sizeof(tmp));
        snprintf(tmp, sizeof(tmp),"%c",sn_gen_pn_str[rand_sn1]);
        strlcpy (str, tmp, sizeof(str));
        
        snprintf(tmp, sizeof(tmp),"%c",sn_gen_pn_str[rand_sn2]);
        strcat (str, tmp);
        
        snprintf(tmp, sizeof(tmp),"%c",sn_gen_pn_str[rand_sn3]);
        strcat (str, tmp);
        
        DBG ("fake_productNumber: %s\n",str);
        
    }
    return str;
}

const char* sm_get_random_week(void)
{
    static char str[4] = {0x00,0x00,0x00,0x00};
    if(str[0]  == 0)
    {           
        // Get randomized characters
        int rand_week ;
        rand_week = arc4random_unirange(0,47);
        
        // Append all charaters to the string 
        char tmp[3];
        bzero(tmp,sizeof(tmp));
        
        if (rand_week < 10) {
            snprintf(tmp, sizeof(tmp),"0%d",rand_week);
            strlcpy (str, tmp, sizeof(str));
        } else if (rand_week < 100) { // avoid overflow in case random return a number >= 100
            snprintf(tmp, sizeof(tmp),"%d",rand_week);
            strlcpy (str, tmp, sizeof(str));
        }      
        
        DBG ("fake_week: %s\n",str);
        
    }
    return str;
}

const char* sm_get_random_year(void)
{
    static char str[2] = {0x00,0x00};
    if(str[0]  == 0)
    {           
        // Get randomized characters
        int rand_year ;
        
        rand_year = arc4random_unirange(0,9);
		
        // Append all charaters to the string 
        char tmp[2];
        bzero(tmp,sizeof(tmp));
        
        if (rand_year < 10) {
            snprintf(tmp, sizeof(tmp),"%d",rand_year);
            strlcpy (str, tmp, sizeof(str));
        }
        
        DBG ("fake_year: %s\n",str);
        
    }
    return str;
}

const char* sm_get_random_country(void)
{
    static char str[3] = {0x00,0x00,0x00};
    if(str[0] == 0)
    {      
        
        // Get randomized characters
        int rand_country ;
        
        rand_country = arc4random_unirange(0,(sizeof(sm_country_list) / sizeof(sm_country_list[0]))-1);
		
        strlcpy (str, sm_country_list[rand_country].code,strlen(sm_country_list[rand_country].code)+1);
        
        DBG ("fake_country: %s (%s)\n",str,sm_country_list[rand_country].info);
        
    }
    return str;
}

static void setDefaultSMBData(void)
{
	static bool setDefSMB = true;
	
	if (setDefSMB) {		
		
        const SMStrEntryPair*	sm_defaults;
        const SMStrEntryPair*	sm_chosen;
        
        if (get_env(envIsServer))
        {
            sm_defaults=sm_xserve_defaults;
        } else if (get_env(envIsMobile)) {
            if (get_env(envNoCores) > 1) {
                sm_defaults=sm_macbookpro_defaults;
            } else {
                sm_defaults=sm_macbook_defaults;
            }
        } else {
            switch (get_env(envNoCores)) 
            {
                case 1: 
                    sm_defaults=sm_macmini_defaults; 
                    break;
                case 2:
                    sm_defaults=sm_imac_defaults;
                    break;
                default:
                {
                    switch (get_env(envFamily)) 
                    {
                        case 0x06:
                        {
                            switch (get_env(envModel))
                            {
                                case CPUID_MODEL_FIELDS:        // Intel Core i5, i7 LGA1156 (45nm)
                                case CPUID_MODEL_DALES:         // Intel Core i5, i7 LGA1156 (45nm) ???
                                case CPUID_MODEL_DALES_32NM:    // Intel Core i3, i5, i7 LGA1156 (32nm) (Clarkdale, Arrandale)
                                case 0x19:                      // Intel Core i5 650 @3.20 Ghz 
                                    sm_defaults=sm_imac_core_defaults; 
                                    break;
                                    
                                case CPUID_MODEL_SANDYBRIDGE:
                                case CPUID_MODEL_JAKETOWN:
                                    sm_defaults=sm_imac_sandy_defaults;
                                    break;
                                    
                                case CPUID_MODEL_NEHALEM: 
                                case CPUID_MODEL_NEHALEM_EX:
                                    sm_defaults=sm_macpro_core_defaults; 
                                    break;
                                    
                                case CPUID_MODEL_WESTMERE: 
                                case CPUID_MODEL_WESTMERE_EX:
                                    sm_defaults=sm_macpro_westmere_defaults; 
                                    break;
                                    
                                default:
                                    sm_defaults=sm_macpro_defaults; 
                                    break;
                            }
                            break;
                        }
                        default:
                            sm_defaults=sm_macpro_defaults; 
                            break;
                    }
                    break;
                }
            }
        }
        
        {
            const char	*str;
            int		size;
            
            if (getValueForKey("SMproductname", &str, &size, DEFAULT_SMBIOS_CONFIG))
            {              
                if (strstr (str, "MacPro5"))
                {
                    sm_chosen = sm_macpro_westmere_defaults ;
                }
                else if (strstr (str, "MacPro4"))
                {
                    sm_chosen = sm_macpro_core_defaults ;
                }
                else if (strstr (str, "MacPro"))
                {
                    sm_chosen = sm_macpro_defaults ;
                }
                else if (strstr (str,"MacBookPro"))
                {
                    sm_chosen = sm_macbookpro_defaults ;
                }
                else if (strstr (str, "MacBook"))
                {
                    sm_chosen = sm_macbook_defaults ;
                }
                else if (!strcmp ("iMac12,1", str))
                {
                    sm_chosen = sm_imac_sandy_defaults ;
                }
                else if (!strcmp ("iMac11,1", str))
                {
                    sm_chosen = sm_imac_core_defaults ;
                }
                else if (strstr (str, "iMac"))
                {
                    sm_chosen = sm_imac_defaults ;
                }
                else if (strstr (str, "Macmini"))
                {
                    sm_chosen = sm_macmini_defaults ;
                }
                else if (strstr (str, "Xserve"))
                {
                    sm_chosen = sm_xserve_defaults ;
                }
                else 
                {
                    sm_chosen = sm_defaults ;
                }    
            } 
            else 
                sm_chosen = sm_defaults;     
        }             
        
        bzero  (fake_serial,sizeof(fake_serial));      
        
        bool randomSerial = false;
        getBoolForKey(kSMBIOSRandomSerial, &randomSerial, DEFAULT_BOOT_CONFIG) ;
        
        if ( randomSerial ) // useless
            strlcpy (fake_serial,sm_get_random_country(), strlen(sm_get_random_country())+1);
        else
            strlcpy (fake_serial,sm_search_str(sm_chosen, "SMserialProductCountry"), strlen(sm_search_str(sm_chosen, "SMserialProductCountry"))+1);
        
        if ( randomSerial ) // useless
            strcat (fake_serial,sm_get_random_year());
        else
            strcat (fake_serial,sm_search_str(sm_chosen, "SMserialYear"));
        
        if ( randomSerial ) // useless
            strcat (fake_serial,sm_get_random_week());
        else
            strcat (fake_serial,sm_search_str(sm_chosen, "SMserialWeek"));            
        
        if ( randomSerial )
            strcat (fake_serial,sm_get_random_productNumber());
        else
            strcat (fake_serial,sm_search_str(sm_chosen, "SMserialProductNumber"));
        
        strcat (fake_serial,sm_search_str(sm_chosen, "SMserialModel"));
        
        if ( randomSerial )
            msglog ("fake_serial: %s\n",fake_serial);           
        
        defaultBIOSInfo.version			= sm_search_str(sm_chosen, "SMbiosversion");
        defaultBIOSInfo.releaseDate		= sm_search_str(sm_chosen, "SMbiosdate");        
        defaultBIOSInfo.vendor			= sm_search_str(sm_chosen, "SMbiosvendor");
		
        defaultSystemInfo.productName	= sm_search_str(sm_chosen, "SMproductname");
        defaultSystemInfo.family		= sm_search_str(sm_chosen, "SMfamily");
        defaultSystemInfo.manufacturer	= sm_search_str(sm_chosen, "SMboardmanufacter");
        defaultSystemInfo.version		= sm_search_str(sm_chosen, "SMsystemversion");
        defaultSystemInfo.serialNumber	= fake_serial;
		
        defaultBaseBoard.manufacturer	= sm_search_str(sm_chosen, "SMboardmanufacter");
        defaultBaseBoard.product		= sm_search_str(sm_chosen, "SMboardproduct");
        
		setDefSMB = false;
	}	
}

/* Used for SM*_N smbios.plist keys */
static bool getSMBValueForKey(SMBStructHeader *structHeader, const char *keyString, const char **string, returnType *value)
{
	static int idx = -1;
	static int current = -1;
	int len;
	char key[24];
    
	if (current != structHeader->handle)
	{
		idx++;
		current = structHeader->handle;
	}
    
	snprintf(key, sizeof(key),"%s%d", keyString, idx);
    
	if (value)
	{
		if (getIntForKey(key, (int *)&(value->dword), DEFAULT_SMBIOS_CONFIG))
			return true;
	}
	else
	{
		if (getValueForKey(key, string, &len, DEFAULT_SMBIOS_CONFIG))
			return true;
	}
	
	return false;
}

const char *getSMBStringForField(SMBStructHeader *structHeader, uint8_t field)
{
	uint8_t *stringPtr = (uint8_t *)structHeader + structHeader->length;
    
	if (!field)
    {
		//return (char *)0;
        return NULL;
    }
    
	for (field--; field != 0 && strlen((char *)stringPtr) > 0; 
         field--, stringPtr = (uint8_t *)((uint32_t)stringPtr + strlen((char *)stringPtr) + 1));
    
	return (char *)stringPtr;
}

static void setSMBStringForField(SMBStructHeader *structHeader, const char *string, uint8_t *field)
{
	int strSize;
    
	if (!field)
    {
        
		return;
    }
	if (!string)
	{
		*field = 0;
		return;
	}
	
	strSize = strlen(string);
	
	// remove any spaces found at the end
    while ((strSize != 0) && (string[strSize - 1] == ' '))
		strSize--;
	
    
	if (strSize == 0)
	{
		*field = 0;
		return;
	}
	
	memcpy((uint8_t *)structHeader + structHeader->length + stringsSize, string, strSize);
    
	*field = stringIndex;
	
	stringIndex++;
	stringsSize += strSize + 1;
}

static bool setSMBValue(SMBStructPtrs *structPtr, int idx, returnType *value)
{
	const char *string = 0;
	int len;
	bool parsed;
	int val;
    
	if (numOfSetters <= idx)
		return false;
	
	switch (SMBSetters[idx].valueType)
	{
		case kSMBString:
        {
            bool randomSerial = false;                 
            getBoolForKey(kSMBIOSRandomSerial, &randomSerial, DEFAULT_BOOT_CONFIG);
            
			if (SMBSetters[idx].keyString)
			{
                if ((SMBSetters[idx].defaultValue) && *(SMBSetters[idx].defaultValue) && randomSerial && (!strcmp ("SMserial", SMBSetters[idx].keyString)))
                {
                    string = *(SMBSetters[idx].defaultValue);
                    break;
                }
                else if (getValueForKey(SMBSetters[idx].keyString, &string, &len, DEFAULT_SMBIOS_CONFIG))
					break;
				else
					if (structPtr->orig->type == kSMBTypeMemoryDevice)	// MemoryDevice only
						if (getSMBValueForKey(structPtr->orig, SMBSetters[idx].keyString, &string, NULL))
							break;
			}
			if (SMBSetters[idx].getSMBValue)
				if (SMBSetters[idx].getSMBValue((returnType *)&string))
					break;
			if ((SMBSetters[idx].defaultValue) && *(SMBSetters[idx].defaultValue))
			{
				string = *(SMBSetters[idx].defaultValue);
				break;
			}
			string = getSMBStringForField(structPtr->orig, *(uint8_t *)value);
			break;
		}	
		case kSMBByte:
		case kSMBWord:
		case kSMBDWord:
			//case kSMBQWord:
			/*if (SMBSetters[idx].keyString)
             {
             if (getIntForKey(SMBSetters[idx].keyString, (int *)&(value->dword), DEFAULT_SMBIOS_CONFIG))
             return true;
             else
             if (structPtr->orig->type == kSMBTypeMemoryDevice)	// MemoryDevice only
             if (getSMBValueForKey(structPtr->orig, SMBSetters[idx].keyString, NULL, value))
             return true;
             }*/
            if (SMBSetters[idx].keyString)
			{
				parsed = getIntForKey(SMBSetters[idx].keyString, &val, DEFAULT_SMBIOS_CONFIG);
				if (!parsed)
					if (structPtr->orig->type == kSMBTypeMemoryDevice)	// MemoryDevice only
						parsed = getSMBValueForKey(structPtr->orig, SMBSetters[idx].keyString, NULL, (returnType *)&val);
				if (parsed)
				{
					switch (SMBSetters[idx].valueType)
					{
						case kSMBByte:
							value->byte = (uint8_t)val;
							break;
						case kSMBWord:
							value->word = (uint16_t)val;
							break;
						case kSMBDWord:
						default:
							value->dword = (uint32_t)val;
							break;
					}
					return true;
				}
            }
			if (SMBSetters[idx].getSMBValue)
				if (SMBSetters[idx].getSMBValue(value))
					return true;
#if 0
			if (*(SMBSetters[idx].defaultValue))
			{
				value->dword = *(uint32_t *)(SMBSetters[idx].defaultValue);
				return true;
			}
#endif
			break;
	}
	
	if (SMBSetters[idx].valueType == kSMBString && string)
		setSMBStringForField(structPtr->new, string, &value->byte);
    
	return true;
}
//-------------------------------------------------------------------------------------------------------------------------
// Apple Specific
//-------------------------------------------------------------------------------------------------------------------------
static void addSMBFirmwareVolume(SMBStructPtrs *structPtr)
{
	return;
}

static void addSMBMemorySPD(SMBStructPtrs *structPtr)
{
	/* SPD data from Platform->RAM.spd */
	return;
}

static void addSMBOemProcessorType(SMBStructPtrs *structPtr)
{
	SMBOemProcessorType *p = (SMBOemProcessorType *)structPtr->new;
	
	p->header.type		= kSMBTypeOemProcessorType;
	p->header.length	= sizeof(SMBOemProcessorType);
	p->header.handle	= handle++;
    
	setSMBValue(structPtr, numOfSetters - 2 , (returnType *)&(p->ProcessorType));
    
	structPtr->new = (SMBStructHeader *)((uint8_t *)structPtr->new + sizeof(SMBOemProcessorType) + 2);
	tableLength += sizeof(SMBOemProcessorType) + 2;
	structureCount++;
}

static void addSMBOemProcessorBusSpeed(SMBStructPtrs *structPtr)
{
	SMBOemProcessorBusSpeed *p = (SMBOemProcessorBusSpeed *)structPtr->new;
    
	switch (get_env(envFamily)) 
	{
		case 0x06:
		{
			switch (get_env(envModel))
			{
				case 0x19:					// Intel Core i5 650 @3.20 Ghz
				case CPUID_MODEL_FIELDS:		// Intel Core i5, i7 LGA1156 (45nm)
				case CPUID_MODEL_DALES:		// Intel Core i5, i7 LGA1156 (45nm) ???
				case CPUID_MODEL_DALES_32NM:	// Intel Core i3, i5, i7 LGA1156 (32nm)
				case CPUID_MODEL_NEHALEM:		// Intel Core i7 LGA1366 (45nm)
				case CPUID_MODEL_NEHALEM_EX:	// Intel Core i7 LGA1366 (45nm) 6 Core ???
				case CPUID_MODEL_WESTMERE:	// Intel Core i7 LGA1366 (32nm) 6 Core
				case CPUID_MODEL_WESTMERE_EX:	// Intel Core i7 LGA1366 (45nm) 6 Core ???
				case CPUID_MODEL_SANDYBRIDGE:
				case CPUID_MODEL_JAKETOWN: 
					break;
					
				default:
					return;
			}
		}
	}
	
	p->header.type		= kSMBTypeOemProcessorBusSpeed;
	p->header.length	= sizeof(SMBOemProcessorBusSpeed);
	p->header.handle	= handle++;
    
	setSMBValue(structPtr, numOfSetters -1, (returnType *)&(p->ProcessorBusSpeed));
    
	structPtr->new = (SMBStructHeader *)((uint8_t *)structPtr->new + sizeof(SMBOemProcessorBusSpeed) + 2);
	tableLength += sizeof(SMBOemProcessorBusSpeed) + 2;
	structureCount++;
}

//-------------------------------------------------------------------------------------------------------------------------
// EndOfTable
//-------------------------------------------------------------------------------------------------------------------------
static void addSMBEndOfTable(SMBStructPtrs *structPtr)
{
	structPtr->new->type	= kSMBTypeEndOfTable;
	structPtr->new->length	= sizeof(SMBStructHeader);
	structPtr->new->handle	= handle++;
    
	structPtr->new = (SMBStructHeader *)((uint8_t *)structPtr->new + sizeof(SMBStructHeader) + 2);
	tableLength += sizeof(SMBStructHeader) + 2;
	structureCount++;
}

static void setSMBStruct(SMBStructPtrs *structPtr)
{
	bool setterFound = false;
    
	uint8_t *ptr;
	SMBWord structSize;
	int i;
    
	stringIndex = 1;
	stringsSize = 0;
    
	if (handle < structPtr->orig->handle)
		handle = structPtr->orig->handle;
    
	memcpy((void *)structPtr->new, structPtr->orig, structPtr->orig->length);
    
	for (i = 0; i < numOfSetters; i++)
    /*if (structPtr->orig->type == SMBSetters[i].type)
     {
     if (SMBSetters[i].fieldOffset > structPtr->orig->length)
     continue;*/
        if ((structPtr->orig->type == SMBSetters[i].type) && (SMBSetters[i].fieldOffset < structPtr->orig->length))
        {
			setterFound = true;
			setSMBValue(structPtr, i, (returnType *)((uint8_t *)structPtr->new + SMBSetters[i].fieldOffset));
		}
    
	if (setterFound)
	{
		ptr = (uint8_t *)structPtr->new + structPtr->orig->length;
		for (; ((uint16_t *)ptr)[0] != 0; ptr++);
        
		if (((uint16_t *)ptr)[0] == 0)
			ptr += 2;
        
		structSize = ptr - (uint8_t *)structPtr->new;
	}
	else
	{
		ptr = (uint8_t *)structPtr->orig + structPtr->orig->length;
		for (; ((uint16_t *)ptr)[0] != 0; ptr++);
        
		if (((uint16_t *)ptr)[0] == 0)
			ptr += 2;
		
		structSize = ptr - (uint8_t *)structPtr->orig;
		memcpy((void *)structPtr->new, structPtr->orig, structSize);
	}
    
	structPtr->new = (SMBStructHeader *)((uint8_t *)structPtr->new + structSize);
    
	tableLength += structSize;
    
	if (structSize >  maxStructSize)
		maxStructSize = structSize;
    
	structureCount++;
    
}

static void setupNewSMBIOSTable(SMBEntryPoint *eps, SMBStructPtrs *structPtr)
{
	uint8_t *ptr = (uint8_t *)eps->dmi.tableAddress;
	structPtr->orig = (SMBStructHeader *)ptr;
    
	for (;((eps->dmi.tableAddress + eps->dmi.tableLength) > ((uint32_t)(uint8_t *)structPtr->orig + sizeof(SMBStructHeader)));)
	{
		switch (structPtr->orig->type)
		{
                /* Skip all Apple Specific Structures */
			case kSMBTypeFirmwareVolume:
			case kSMBTypeMemorySPD:
			case kSMBTypeOemProcessorType:
			case kSMBTypeOemProcessorBusSpeed:
				/* And this one too, to be added at the end */
			case kSMBTypeEndOfTable:
				break;
                
			default:
            {
				/* Add */
				setSMBStruct(structPtr);
				break;
            }
		}
        
		ptr = (uint8_t *)((uint32_t)structPtr->orig + structPtr->orig->length);
        
		for (; ((uint16_t *)ptr)[0] != 0; ptr++);
        
		if (((uint16_t *)ptr)[0] == 0)
			ptr += 2;
        
		structPtr->orig = (SMBStructHeader *)ptr;
	}
    
    
	addSMBFirmwareVolume(structPtr);
    
	addSMBMemorySPD(structPtr);
    
	addSMBOemProcessorType(structPtr);
    
	addSMBOemProcessorBusSpeed(structPtr);
    
	addSMBEndOfTable(structPtr);
    
    
}

SMBEntryPoint * setupSMBIOSTable(SMBEntryPoint *origeps)
{
	SMBStructPtrs *structPtr;
	uint8_t *buffer;
	bool setSMB = true;
    
	if (!origeps)
		return NULL;
    
	structPtr = (SMBStructPtrs *)malloc(sizeof(SMBStructPtrs));
	if (!structPtr)
		return NULL;
	
	buffer = (uint8_t *)malloc(SMB_ALLOC_SIZE);
	if (!buffer)
    {
        free(structPtr);
		return NULL;
    }
	
	bzero(buffer, SMB_ALLOC_SIZE);
	structPtr->new = (SMBStructHeader *)buffer;
    
	getBoolForKey(kSMBIOSdefaults, &setSMB, DEFAULT_BOOT_CONFIG);
	if (setSMB)
		setDefaultSMBData();
    
	setupNewSMBIOSTable(origeps, structPtr);
	
	SMBEntryPoint *neweps = (SMBEntryPoint *)AllocateKernelMemory(sizeof(SMBEntryPoint));
	if (!neweps)
    {
        free(buffer);
        free(structPtr);
		return NULL;
    }
	bzero(neweps, sizeof(SMBEntryPoint));
    
	neweps->anchor[0]			= '_';
	neweps->anchor[1]			= 'S';
	neweps->anchor[2]			= 'M';
	neweps->anchor[3]			= '_';
	neweps->entryPointLength	= sizeof(SMBEntryPoint);
	neweps->majorVersion		= 2;
	neweps->minorVersion		= 4;
	neweps->maxStructureSize	= maxStructSize;
	neweps->entryPointRevision	= 0;
    
	neweps->dmi.anchor[0]		= '_';
	neweps->dmi.anchor[1]		= 'D';
	neweps->dmi.anchor[2]		= 'M';
	neweps->dmi.anchor[3]		= 'I';
	neweps->dmi.anchor[4]		= '_';
	neweps->dmi.tableLength		= tableLength;
	neweps->dmi.tableAddress	= AllocateKernelMemory(tableLength);
	neweps->dmi.structureCount	= structureCount;
	neweps->dmi.bcdRevision		= 0x24;
    
	if (!neweps->dmi.tableAddress)
    {
        free(buffer);
        free(structPtr);
		return NULL;
    }    
	memcpy((void *)neweps->dmi.tableAddress, buffer, tableLength);
    
	neweps->dmi.checksum		= 0;
	neweps->dmi.checksum		= 0x100 - checksum8(&neweps->dmi, sizeof(DMIEntryPoint));
    
	neweps->checksum			= 0;
	neweps->checksum			= 0x100 - checksum8(neweps, sizeof(SMBEntryPoint));
    
	free(buffer);
    free(structPtr);

	decodeSMBIOSTable(neweps);
    
    return neweps;
}

/* Collect any information needed later */
void readSMBIOSInfo(SMBEntryPoint *eps)
{
	uint8_t *structPtr = (uint8_t *)eps->dmi.tableAddress;
	SMBStructHeader *structHeader = (SMBStructHeader *)structPtr;
    
	int dimmnbr = 0;
    int			MaxMemorySlots = 0;		// number of memory slots polulated by SMBIOS
    int			CntMemorySlots = 0;		// number of memory slots counted
    int			MemoryModules = 0;
    
    
    static RamSlotInfo_t RamDimm[MAX_RAM_SLOTS];
    
	for (;((eps->dmi.tableAddress + eps->dmi.tableLength) > ((uint32_t)(uint8_t *)structHeader + sizeof(SMBStructHeader)));)
	{
		switch (structHeader->type)
		{
			case kSMBTypeSystemInformation:
                safe_set_env(envUUID,(uint32_t)((SMBSystemInformation *)structHeader)->uuid);
				break;
                
			case kSMBTypePhysicalMemoryArray:
				MaxMemorySlots += ((SMBPhysicalMemoryArray *)structHeader)->numMemoryDevices;
				break;
                
			case kSMBTypeMemoryDevice:
				CntMemorySlots++;
        		if (((SMBMemoryDevice *)structHeader)->memorySize != 0)
					MemoryModules++;
        		if (((SMBMemoryDevice *)structHeader)->memorySpeed > 0)
                    
					RamDimm[dimmnbr].Frequency = ((SMBMemoryDevice *)structHeader)->memorySpeed;
				dimmnbr++;
				break;
			default:
				break;
		}
        
		structPtr = (uint8_t *)((uint32_t)structHeader + structHeader->length);
		for (; ((uint16_t *)structPtr)[0] != 0; structPtr++);
        
		if (((uint16_t *)structPtr)[0] == 0)
			structPtr += 2;
        
		structHeader = (SMBStructHeader *)structPtr;
	}
    safe_set_env(envDMIMaxMemorySlots, MaxMemorySlots);
    safe_set_env(envDMICntMemorySlots, CntMemorySlots);
    safe_set_env(envDMIMemModules, MemoryModules);
    safe_set_env_copy(envRamDimm, RamDimm, sizeof(RamDimm));
    
    
}

