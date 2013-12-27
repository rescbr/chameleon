/*
 * SMBIOS Table Patcher, part of the Chameleon Boot Loader Project
 *
 * Copyright 2010 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */


#include "boot.h"
#include "bootstruct.h"
#include "smbios_getters.h"
// Bungo
#include "convert.h"

#ifndef DEBUG_SMBIOS
#define DEBUG_SMBIOS 0
#endif

#if DEBUG_SMBIOS
#define DBG(x...)	printf(x)
#else
#define DBG(x...)	msglog(x)
#endif

#define SMBPlist			&bootInfo->smbiosConfig
/* ASSUMPTION: 16KB should be enough for the whole thing */
#define SMB_ALLOC_SIZE	16384


//-------------------------------------------------------------------------------------------------------------------------
// SMBIOS Plist Keys
//-------------------------------------------------------------------------------------------------------------------------

/* =======================
 BIOS Information (Type 0)
 ========================= */
#define kSMBBIOSInformationVendorKey		        "SMbiosvendor"         // Apple Inc.
#define kSMBBIOSInformationVersionKey		        "SMbiosversion"        // MP31.88Z.006C.B05.0802291410
#define kSMBBIOSInformationReleaseDateKey	        "SMbiosdate"           // 02/29/08
// Bungo
#define kSMBBIOSInformationReleaseKey               "SMbiosrelease"        // BIOS Revision
// example: BIOS Revision: 1.23 --> 2 bytes: Major=0x01, Minor=0x17 --> after swap: 0x1701hex = 5889dec (SMBIOS_spec_DSP0134_2.7.1)

/* =========================
 System Information (Type 1)
 =========================== */
#define kSMBSystemInformationManufacturerKey        "SMmanufacturer"       // Apple Inc.
#define kSMBSystemInformationProductNameKey         "SMproductname"        // MacPro3,1
#define kSMBSystemInformationVersionKey             "SMsystemversion"      // 1.0
#define kSMBSystemInformationSerialNumberKey        "SMserial"             // Serial number
//Bungo
#define kSMBSystemInformationUUIDKey                "SMsystemuuid"         // ioreg -rd1 -c IOPlatformExpertDevice | awk '/IOPlatformUUID/ { split($0, line, "\""); printf("%s\n", line[4]); }'
#define kSMBSystemInformationSKUNumberKey           "SMskunumber"          // System SKU#

#define kSMBSystemInformationFamilyKey              "SMfamily"             // MacPro

/* =========================================
 Base Board (or Module) Information (Type 2)
 =========================================== */
#define kSMBBaseBoardManufacturerKey                "SMboardmanufacturer"  // Apple Inc.
#define kSMBBaseBoardProductKey                     "SMboardproduct"       // Mac-F2268DC8
// Bungo
#define kSMBBaseBoardVersionKey                     "SMboardversion"       // MacPro3,1
#define kSMBBaseBoardSerialNumberKey                "SMboardserial"        // C02140302D5DMT31M
#define kSMBBaseBoardAssetTagNumberKey              "SMboardassettag"      // Base Board Asset Tag#
#define kSMBBaseBoardLocationInChassisKey           "SMboardlocation"      // Part Component
#define kSMBBaseBoardTypeKey                        "SMboardtype"          // 10 (Motherboard) all model, 11 (Processor+Memory Module) MacPro

// Bungo
/* =======================
 System Enclosure (Type 3)
 ========================= */
#define kSMBSystemEnclosureManufacturerKey          "SMchassismanufacturer" // Apple Inc.
#define kSMBSystemEnclosureTypeKey                  "SMchassistype"         // 7 Desktop
#define kSMBSystemEnclosureVersionKey               "SMchassisversion"      // Mac-F42C88C8
#define kSMBSystemEnclosureSerialNumberKey          "SMchassisserial"       // Serial number
#define kSMBSystemEnclosureAssetTagNumberKey        "SMchassisassettag"     // Pro-Enclosure

/* ============================
 Processor Information (Type 4)
 ============================== */
// Bungo
#define kSMBProcessorInformationSocketKey           "SMcpusocket"
#define kSMBProcessorInformationManufacturerKey     "SMcpumanufacturer"
#define kSMBProcessorInformationVersionKey          "SMcpuversion"
//
#define kSMBProcessorInformationExternalClockKey	"SMexternalclock"
#define kSMBProcessorInformationMaximumClockKey		"SMmaximalclock"
// Bungo
#define kSMBProcessorInformationCurrentClockKey     "SMcurrentclock"
#define kSMBProcessorInformationUpgradeKey          "SMcpuupgrade"
#define kSMBProcessorInformationSerialNumberKey     "SMcpuserial"
#define kSMBProcessorInformationAssetTagNumberKey   "SMcpuassettag"
#define kSMBProcessorInformationPartNumberKey       "SMcpupartnumber"

/* =====================
 Memory Device (Type 17)
 ======================= */
#define kSMBMemoryDeviceDeviceLocatorKey            "SMmemdevloc"           //
#define kSMBMemoryDeviceBankLocatorKey              "SMmembankloc"          //
#define kSMBMemoryDeviceMemoryTypeKey               "SMmemtype"             //
#define kSMBMemoryDeviceMemorySpeedKey              "SMmemspeed"            //
#define kSMBMemoryDeviceManufacturerKey             "SMmemmanufacturer"     //
#define kSMBMemoryDeviceSerialNumberKey             "SMmemserial"           //
#define kSMBMemoryDevicePartNumberKey               "SMmempart"             //

/* ===========================================
 Memory SPD Data   (Apple Specific - Type 130)
 ============================================= */

/* ============================================
 OEM Processor Type (Apple Specific - Type 131)
 ============================================== */
#define kSMBOemProcessorTypeKey                     "SMoemcputype" // Bungo: renamed from SMcputype

/* =================================================
 OEM Processor Bus Speed (Apple Specific - Type 132)
 =================================================== */
#define kSMBOemProcessorBusSpeedKey                 "SMoemcpubusspeed" // Bungo: renamed from SMbusspeed

//-------------------------------------------------------------------------------------------------------------------------
// Default SMBIOS Data
//-------------------------------------------------------------------------------------------------------------------------
/* Rewrite: use a struct */

#define kDefaultVendorManufacturer			"Apple Inc."
#define kDefaultBIOSReleaseDate				"11/06/2009"
#define kDefaultSerialNumber				"SOMESRLNMBR"
#define kDefaultskuNumber				"Default System SKU#"
#define kDefaultBoardProduct				"Mac-F4208DC8"
#define kDefaultBoardType				"10" // 0xA
#define kDefaultSystemVersion				"1.0"

//Bungo
/* 256 = 0x0100 -> swap bytes: 0x0001 -> Release: 0.1 (see SMBIOS spec. table Type 0) */
#define kDefaultBIOSRelease                         256

#define KDefaultBoardSerialNumber			"C02140302D5DMT31M" // new C07019501PLDCVHAD - C02032101R5DC771H
#define KDefaultBoardAssetTagNumber			"Pro-Enclosure" // ErmaC
#define kDefaultLocatioInChassis			"Part Component" // ErmaC

// defaults for a Mac mini
#define kDefaultMacminiFamily				"Macmini"
#define kDefaultMacmini					"Macmini1,1"
#define kDefaultMacminiBIOSVersion			"    MM21.88Z.009A.B00.0903051113"

// defaults for a MacBook
#define kDefaultMacBookFamily				"MacBook"
#define kDefaultMacBook					"MacBook4,1"
#define kDefaultMacBookBIOSVersion			"    MB41.88Z.0073.B00.0903051113"

// defaults for a MacBook Pro
#define kDefaultMacBookProFamily			"MacBookPro"
#define kDefaultMacBookPro				"MacBookPro4,1"
#define kDefaultMacBookProBIOSVersion			"    MBP41.88Z.0073.B00.0903051113"

// defaults for an iMac
#define kDefaultiMacFamily				"iMac"
#define kDefaultiMac					"iMac8,1"
#define kDefaultiMacBIOSVersion				"    IM81.88Z.00C1.B00.0903051113"
// defaults for an iMac11,1 core i3/i5/i7
#define kDefaultiMacNehalem				"iMac11,1"
#define kDefaultiMacNehalemBIOSVersion			"    IM111.88Z.0034.B00.0903051113"
// defaults for an iMac12,1
#define kDefaultiMacSandy				"iMac12,1"
#define kDefaultiMacSandyBIOSVersion			"    IM121.88Z.0047.B00.1102091756"

// defaults for a Mac Pro
#define kDefaultMacProFamily				"MacPro"
#define kDefaultMacPro					"MacPro3,1"
#define kDefaultMacProBIOSVersion			"    MP31.88Z.006C.B05.0903051113"
// defaults for a Mac Pro 4,1 core i7/Xeon
#define kDefaultMacProNehalem				"MacPro4,1"
#define kDefaultMacProNehalemBIOSVersion		"    MP41.88Z.0081.B04.0903051113"
// defaults for a Mac Pro 5,1 core i7/Xeon
#define kDefaultMacProWestmere				"MacPro5,1"
#define kDefaultMacProWestmereBIOSVersion		"    MP51.88Z.007F.B03.1010071432"
#define kDefaultMacProWestmereBIOSReleaseDate		"10/07/2010"
//-------------------------------------------------------------------------------------------------------------------------


#define getFieldOffset(struct, field)	((uint8_t)(uint32_t)&(((struct *)0)->field))

typedef struct
{
	SMBStructHeader *orig;
	SMBStructHeader *new;
} SMBStructPtrs;

/* =======================
 BIOS Information (Type 0)
 ========================= */
typedef struct
{
	char *vendor;
	char *version;
	char *releaseDate;
	uint16_t release;     // Bungo
} defaultBIOSInfo_t;

defaultBIOSInfo_t defaultBIOSInfo;

/* =========================
 System Information (Type 1)
 =========================== */
typedef struct
{
	char *manufacturer;
	char *productName;
	char *version;
	char *serialNumber;
	char *skuNumber;	// ErmaC
	char *family;
} defaultSystemInfo_t;

defaultSystemInfo_t defaultSystemInfo;

/* =========================================
 Base Board (or Module) Information (Type 2)
 =========================================== */
typedef struct
{
	char *manufacturer;
	char *product;
	char *productName;		// ErmaC
	char *serialNumber;		// ErmaC
	char *assetTagNumber;		// ErmaC
	char *locationInChassis;	// ErmaC
	char *boardType;		// ErmaC
} defaultBaseBoard_t;

defaultBaseBoard_t defaultBaseBoard;

typedef struct
{
	uint8_t			type;
	SMBValueType		valueType;
	uint8_t			fieldOffset;
	char			*keyString;
	bool			(*getSMBValue)(returnType *);
	// Bungo
	// char			**defaultValue;
	void			*defaultValue;
} SMBValueSetter;

SMBValueSetter SMBSetters[] = 
{
	/* =======================
	 BIOS Information (Type 0)
	 ========================= */
	{ kSMBTypeBIOSInformation, kSMBString, getFieldOffset(SMBBIOSInformation, vendor),
		kSMBBIOSInformationVendorKey, NULL, &defaultBIOSInfo.vendor }, // SMbiosvendor - Apple Inc.

	{ kSMBTypeBIOSInformation, kSMBString, getFieldOffset(SMBBIOSInformation, version),
		kSMBBIOSInformationVersionKey, NULL, &defaultBIOSInfo.version }, // SMbiosversion - MP31.88Z.006C.B05.0802291410

	{ kSMBTypeBIOSInformation, kSMBString, getFieldOffset(SMBBIOSInformation, releaseDate),
		kSMBBIOSInformationReleaseDateKey, NULL, &defaultBIOSInfo.releaseDate }, // SMbiosdate - 02/29/08

	// Bungo
	{ kSMBTypeBIOSInformation, kSMBWord, getFieldOffset(SMBBIOSInformation, releaseMajor),
		kSMBBIOSInformationReleaseKey, NULL,	&defaultBIOSInfo.release }, // SMbiosrelease - 256
	//

	/* =========================
	 System Information (Type 1)
	 =========================== */
	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, manufacturer),
		kSMBSystemInformationManufacturerKey, NULL,	&defaultSystemInfo.manufacturer	}, // SMmanufacturer - Apple Inc.

	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, productName),
		kSMBSystemInformationProductNameKey, NULL, &defaultSystemInfo.productName }, // SMproductname - MacPro3,1

	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, version),
		kSMBSystemInformationVersionKey, NULL, &defaultSystemInfo.version }, // SMsystemversion - 1.0

	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, serialNumber),
		kSMBSystemInformationSerialNumberKey, NULL, &defaultSystemInfo.serialNumber }, // SMserial - Serial number

	// Bungo
	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, skuNumber),
		kSMBSystemInformationSKUNumberKey, NULL, &defaultSystemInfo.skuNumber}, // SMskunumber - System SKU#

	//

	{kSMBTypeSystemInformation,	kSMBString,	getFieldOffset(SMBSystemInformation, family),
		kSMBSystemInformationFamilyKey,	NULL,	&defaultSystemInfo.family}, // SMfamily - MacPro


	/* =========================================
	 Base Board (or Module) Information (Type 2)
	 =========================================== */
	{kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, manufacturer),
		kSMBBaseBoardManufacturerKey, NULL, &defaultBaseBoard.manufacturer }, // SMboardmanufacturer - Apple Inc.

	{kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, product),
		kSMBBaseBoardProductKey, NULL, &defaultBaseBoard.product}, // SMboardproduct - Mac-F2268DC8

	// Bungo
	{kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, version),
		kSMBBaseBoardVersionKey, NULL, &defaultBaseBoard.productName}, // SMboardproductname - MacPro3,1

	{kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, serialNumber),
		kSMBBaseBoardSerialNumberKey, NULL, &defaultBaseBoard.serialNumber }, // SMboardserial - C02140302D5DMT31M

	{kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, assetTagNumber),
		kSMBBaseBoardAssetTagNumberKey, NULL, &defaultBaseBoard.assetTagNumber }, // SMboardassetag - Base Board Asset Tag#

	{kSMBTypeBaseBoard,	kSMBString,	getFieldOffset(SMBBaseBoard, locationInChassis),
		kSMBBaseBoardLocationInChassisKey, NULL, &defaultBaseBoard.locationInChassis }, // SMboardlocation - Part Component

	{kSMBTypeBaseBoard,	kSMBByte,getFieldOffset(SMBBaseBoard, boardType),
		kSMBBaseBoardTypeKey,NULL, &defaultBaseBoard.boardType }, // SMboardtype - 10 (Motherboard) all model, 11 (Processor+Memory Module) MacPro
	//

    // Bungo
	/* =======================
	 System Enclosure (Type 3)
	 ========================= */
	{kSMBTypeSystemEnclosure,	kSMBString,	getFieldOffset(SMBSystemEnclosure, manufacturer),
		kSMBSystemEnclosureManufacturerKey, NULL,	&defaultBaseBoard.manufacturer }, // SMchassismanufacturer - Apple Inc.

	{kSMBTypeSystemEnclosure, kSMBByte,	getFieldOffset(SMBSystemEnclosure, type),
		kSMBSystemEnclosureTypeKey, NULL, &defaultBaseBoard.boardType	}, // SMchassistype - 7

	{kSMBTypeSystemEnclosure, kSMBString, getFieldOffset(SMBSystemEnclosure, version),
		kSMBSystemEnclosureVersionKey, NULL, &defaultBaseBoard.product }, // SMchassisversion - Mac-F42C88C8

	{kSMBTypeSystemEnclosure, kSMBString, getFieldOffset(SMBSystemEnclosure, serialNumber),
		kSMBSystemEnclosureSerialNumberKey, NULL, &defaultSystemInfo.serialNumber }, // SMchassisserial

	{kSMBTypeSystemEnclosure, kSMBString, getFieldOffset(SMBSystemEnclosure, assetTagNumber),
		kSMBSystemEnclosureAssetTagNumberKey, NULL, &defaultBaseBoard.assetTagNumber }, // SMchassisassettag - Pro Enclosure


	/* ============================
	 Processor Information (Type 4)
	 ============================== */
	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, socketDesignation),
		kSMBProcessorInformationSocketKey, NULL, NULL}, // SMcpusocket -

	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, manufacturer),
		kSMBProcessorInformationManufacturerKey, NULL, NULL}, // SMcpumanufacturer - Intel(R) Corporation

	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, processorVersion),
		kSMBProcessorInformationVersionKey, NULL, NULL}, // SMcpuversion

	{kSMBTypeProcessorInformation,	kSMBWord, getFieldOffset(SMBProcessorInformation, externalClock),
		kSMBProcessorInformationExternalClockKey, getProcessorInformationExternalClock,	NULL}, // SMcpuexternalclock

	{kSMBTypeProcessorInformation,	kSMBWord, getFieldOffset(SMBProcessorInformation, maximumClock),
		kSMBProcessorInformationMaximumClockKey, getProcessorInformationMaximumClock,	NULL}, // SMcpumaxspeed

	// Bungo
	{kSMBTypeProcessorInformation,	kSMBWord,	getFieldOffset(SMBProcessorInformation, currentClock),
		kSMBProcessorInformationCurrentClockKey, NULL, NULL}, // SMcpucurrentspeed

	{kSMBTypeProcessorInformation,	kSMBByte,	getFieldOffset(SMBProcessorInformation, processorUpgrade),
		kSMBProcessorInformationUpgradeKey, NULL, NULL}, // SMcpuupgrade
	//

	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, serialNumber),
		NULL, NULL, NULL},

    // Bungo
	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, assetTag),
		kSMBProcessorInformationAssetTagNumberKey, NULL, NULL}, // SMcpuassettag

	//

	{kSMBTypeProcessorInformation,	kSMBString,	getFieldOffset(SMBProcessorInformation, partNumber),
		NULL, NULL, NULL},

	/* =====================
	 Memory Device (Type 17)
	 ======================= */
	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, deviceLocator),
		kSMBMemoryDeviceDeviceLocatorKey, NULL, NULL},

	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, bankLocator),
		kSMBMemoryDeviceBankLocatorKey, NULL, NULL},

	{kSMBTypeMemoryDevice,	kSMBByte,	getFieldOffset(SMBMemoryDevice, memoryType),
		kSMBMemoryDeviceMemoryTypeKey, getSMBMemoryDeviceMemoryType,	NULL},

	{kSMBTypeMemoryDevice,	kSMBWord,	getFieldOffset(SMBMemoryDevice, memorySpeed),
		kSMBMemoryDeviceMemorySpeedKey, getSMBMemoryDeviceMemorySpeed,	NULL},

	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, manufacturer),
		kSMBMemoryDeviceManufacturerKey, getSMBMemoryDeviceManufacturer, NULL},

	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, serialNumber),
		kSMBMemoryDeviceSerialNumberKey, getSMBMemoryDeviceSerialNumber, NULL},

	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, assetTag),
		NULL, NULL, NULL},

	{kSMBTypeMemoryDevice,	kSMBWord,	getFieldOffset(SMBMemoryDevice, errorHandle),
		NULL, getSMBMemoryDeviceMemoryErrorHandle, NULL},

	{kSMBTypeMemoryDevice,	kSMBString,	getFieldOffset(SMBMemoryDevice, partNumber),
		kSMBMemoryDevicePartNumberKey, getSMBMemoryDevicePartNumber, NULL},
	//

	//-------------------------------------------------------------------------------------------------------------------------
	// Apple Specific
	//-------------------------------------------------------------------------------------------------------------------------
	// OEM Processor Type (Apple Specific - Type 131)
	{kSMBTypeOemProcessorType,		kSMBWord,	getFieldOffset(SMBOemProcessorType, ProcessorType),			kSMBOemProcessorTypeKey,		
		getSMBOemProcessorType,			NULL},

	// OEM Processor Bus Speed (Apple Specific - Type 132)
	{kSMBTypeOemProcessorBusSpeed,	kSMBWord,	getFieldOffset(SMBOemProcessorBusSpeed, ProcessorBusSpeed),	kSMBOemProcessorBusSpeedKey,	
		getSMBOemProcessorBusSpeed,		NULL}
};

int numOfSetters = sizeof(SMBSetters) / sizeof(SMBValueSetter);


SMBEntryPoint *origeps	= 0;
SMBEntryPoint *neweps	= 0;

static uint8_t stringIndex;	// increament when a string is added and set the field value accordingly
static uint8_t stringsSize;	// add string size

static SMBWord tableLength	= 0;
static SMBWord handle		= 0;
static SMBWord maxStructSize	= 0;
static SMBWord structureCount	= 0;

bool   useSMBIOSdefaults        = true;  // Bungo

/* Rewrite this function */
void setDefaultSMBData(void)
{
	defaultBIOSInfo.vendor              = kDefaultVendorManufacturer;
	defaultBIOSInfo.releaseDate         = kDefaultBIOSReleaseDate;

	defaultBIOSInfo.release             = kDefaultBIOSRelease;    // Bungo

	defaultSystemInfo.manufacturer      = kDefaultVendorManufacturer;
	defaultSystemInfo.version           = kDefaultSystemVersion;
	defaultSystemInfo.serialNumber      = kDefaultSerialNumber;
	defaultSystemInfo.skuNumber         = kDefaultskuNumber;      // Bungo

	defaultBaseBoard.manufacturer       = kDefaultVendorManufacturer;
	defaultBaseBoard.product            = kDefaultBoardProduct;
	defaultBaseBoard.boardType          = kDefaultBoardType;		// ErmaC 
	defaultBaseBoard.serialNumber       = KDefaultBoardSerialNumber;	// ErmaC
	defaultBaseBoard.assetTagNumber     = KDefaultBoardAssetTagNumber;	// ErmaC
	defaultBaseBoard.locationInChassis  = kDefaultLocatioInChassis;		// ErmaC

	if (platformCPUFeature(CPU_FEATURE_MOBILE))
	{
		if (Platform.CPU.NoCores > 1)
		{
			defaultBIOSInfo.version			= kDefaultMacBookProBIOSVersion;
			defaultSystemInfo.productName	= kDefaultMacBookPro;
			defaultSystemInfo.family		= kDefaultMacBookProFamily;
		}
		else
		{
			defaultBIOSInfo.version			= kDefaultMacBookBIOSVersion;
			defaultSystemInfo.productName	= kDefaultMacBook;
			defaultSystemInfo.family		= kDefaultMacBookFamily;
		}
	}
	else
	{
		switch (Platform.CPU.NoCores)
		{
			case 1:
				defaultBIOSInfo.version			= kDefaultMacminiBIOSVersion;
				defaultSystemInfo.productName		= kDefaultMacmini;
				defaultSystemInfo.family		= kDefaultMacminiFamily;
				break;

			case 2:
				defaultBIOSInfo.version			= kDefaultiMacBIOSVersion;
				defaultSystemInfo.productName		= kDefaultiMac;
				defaultSystemInfo.family		= kDefaultiMacFamily;
				break;
			default:
			{
				switch (Platform.CPU.Family)
				{
					case 0x06:
					{
						switch (Platform.CPU.Model)
						{
							case CPU_MODEL_FIELDS:			// Intel Core i5, i7, Xeon X34xx LGA1156 (45nm)
							case CPU_MODEL_DALES:
							case CPU_MODEL_DALES_32NM:		// Intel Core i3, i5 LGA1156 (32nm)
								defaultBIOSInfo.version			= kDefaultiMacNehalemBIOSVersion;
								defaultSystemInfo.productName	= kDefaultiMacNehalem;
								defaultSystemInfo.family		= kDefaultiMacFamily;
								break;

							case CPU_MODEL_SANDYBRIDGE:			// Intel Core i3, i5, i7 LGA1155 (32nm)
							case CPU_MODEL_IVYBRIDGE:			// Intel Core i3, i5, i7 LGA1155 (22nm)
								defaultBIOSInfo.version			= kDefaultiMacSandyBIOSVersion;
								defaultSystemInfo.productName	= kDefaultiMacSandy;
								defaultSystemInfo.family		= kDefaultiMacFamily;
								break;
							case CPU_MODEL_NEHALEM:			// Intel Core i7, Xeon W35xx, Xeon X55xx, Xeon E55xx LGA1366 (45nm)
							case CPU_MODEL_NEHALEM_EX:		// Intel Xeon X75xx, Xeon X65xx, Xeon E75xx, Xeon E65x
								defaultBIOSInfo.version			= kDefaultMacProNehalemBIOSVersion;
								defaultSystemInfo.productName	= kDefaultMacProNehalem;
								defaultSystemInfo.family		= kDefaultMacProFamily;
								break;

							case CPU_MODEL_WESTMERE:		// Intel Core i7, Xeon X56xx, Xeon E56xx, Xeon W36xx LGA1366 (32nm) 6 Core
							case CPU_MODEL_WESTMERE_EX:		// Intel Xeon E7
							case CPU_MODEL_JAKETOWN:		// Intel Core i7, Xeon E5 LGA2011 (32nm)
								defaultBIOSInfo.version			= kDefaultMacProWestmereBIOSVersion;
								defaultBIOSInfo.releaseDate		= kDefaultMacProWestmereBIOSReleaseDate;
								defaultSystemInfo.productName	= kDefaultMacProWestmere;
								defaultSystemInfo.family		= kDefaultMacProFamily;
								break;

							default:
								defaultBIOSInfo.version			= kDefaultMacProBIOSVersion;
								defaultSystemInfo.productName	= kDefaultMacPro;
								defaultSystemInfo.family		= kDefaultMacProFamily;
								break;
						}
						break;
					}
					default:
						defaultBIOSInfo.version		= kDefaultMacProBIOSVersion;
						defaultSystemInfo.productName	= kDefaultMacPro;
						defaultSystemInfo.family	= kDefaultMacProFamily;
						break;
				}
				break;
			}
		}
	}
}

/* Used for SM*n smbios.plist keys */
bool getSMBValueForKey(SMBStructHeader *structHeader, const char *keyString, const char **string, returnType *value)
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

	sprintf(key, "%s%d", keyString, idx);

	if (value)
	{
		if (getIntForKey(key, (int *)&(value->dword), SMBPlist))
		{
			return true;
		}
	}
	else
	{
		if (getValueForKey(key, string, &len, SMBPlist))
		{
			return true;
		}
	}
	return false;
}

char *getSMBStringForField(SMBStructHeader *structHeader, uint8_t field)
{
	uint8_t *stringPtr = (uint8_t *)structHeader + structHeader->length;

	if (!field)
	{
		return NULL;
	}

	for (field--; field != 0 && strlen((char *)stringPtr) > 0;
		field--, stringPtr = (uint8_t *)((uint32_t)stringPtr + strlen((char *)stringPtr) + 1));

	return (char *)stringPtr;
}

void setSMBStringForField(SMBStructHeader *structHeader, const char *string, uint8_t *field)
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
	{
		strSize--;
	}

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

bool setSMBValue(SMBStructPtrs *structPtr, int idx, returnType *value)
{
	const char *string = 0;
	int len;
	bool parsed;
	int val;

	if (numOfSetters <= idx)
	{
		return false;
	}

	switch (SMBSetters[idx].valueType)
	{
		case kSMBString:
		{
			if (SMBSetters[idx].keyString)
			{
				if (getValueForKey(SMBSetters[idx].keyString, &string, &len, SMBPlist))
				{
					break;
				}
				else
				{
					if (structPtr->orig->type == kSMBTypeMemoryDevice)	// MemoryDevice only
					{
						if (getSMBValueForKey(structPtr->orig, SMBSetters[idx].keyString, &string, NULL))
						{
							break;
						}
					}
				}

			}
			if (SMBSetters[idx].getSMBValue)
			{
				if (SMBSetters[idx].getSMBValue((returnType *)&string))
				{
					break;
            // if ((SMBSetters[idx].defaultValue) && *(SMBSetters[idx].defaultValue))  Bungo
				}
			}
			if (useSMBIOSdefaults && (SMBSetters[idx].defaultValue) && *(char *)(SMBSetters[idx].defaultValue))
			{
                // string = *(SMBSetters[idx].defaultValue);  Bungo
				string = (char *)(SMBSetters[idx].defaultValue);
				break;
			}
			string = getSMBStringForField(structPtr->orig, *(uint8_t *)value);
			break;
		}
		case kSMBByte:
		case kSMBWord:
		case kSMBDWord:
		//case kSMBQWord:
			if (SMBSetters[idx].keyString)
			{
				parsed = getIntForKey(SMBSetters[idx].keyString, &val, SMBPlist);
				if (!parsed)
				{
					if (structPtr->orig->type == kSMBTypeMemoryDevice)	// MemoryDevice only
					{
						parsed = getSMBValueForKey(structPtr->orig, SMBSetters[idx].keyString, NULL, (returnType *)&val);
					}
				}
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
			{
				if (SMBSetters[idx].getSMBValue(value))
				{
					return true;
				}
			}
// #if 0  Bungo: enables code below
            // if (*(SMBSetters[idx].defaultValue))  Bungo
			if (useSMBIOSdefaults && (SMBSetters[idx].defaultValue))
			{
                // value->dword = *(uint32_t *)(SMBSetters[idx].defaultValue);  Bungo
                switch (SMBSetters[idx].valueType)
                {
                    case kSMBByte:
                        value->byte = *(uint8_t *)(SMBSetters[idx].defaultValue);
                        break;
                    case kSMBWord:
                        value->word = *(uint16_t *)(SMBSetters[idx].defaultValue);
                        break;
                    case kSMBDWord:
                    default:
                        value->dword = *(uint32_t *)(SMBSetters[idx].defaultValue);
                        break;
                }
                return true;
			}
// #endif  Bungo
			break;
	}

	// if (SMBSetters[idx].valueType == kSMBString && string)  Bungo: use null string too -> "Not Specified"
	if (SMBSetters[idx].valueType == kSMBString)
	{
		setSMBStringForField(structPtr->new, string, &value->byte);
	}
	return true;
}

//-------------------------------------------------------------------------------------------------------------------------
// Apple Specific
//-------------------------------------------------------------------------------------------------------------------------
void addSMBFirmwareVolume(SMBStructPtrs *structPtr)
{
	return;
}

void addSMBMemorySPD(SMBStructPtrs *structPtr)
{
	/* SPD data from Platform.RAM.spd */
	return;
}

void addSMBOemProcessorType(SMBStructPtrs *structPtr)
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

void addSMBOemProcessorBusSpeed(SMBStructPtrs *structPtr)
{
	SMBOemProcessorBusSpeed *p = (SMBOemProcessorBusSpeed *)structPtr->new;

	switch (Platform.CPU.Family) 
	{
		case 0x06:
		{
			switch (Platform.CPU.Model)
			{
				case 0x19:			// Intel Core i5 650 @3.20 Ghz
				case CPU_MODEL_FIELDS:		// Intel Core i5, i7, Xeon X34xx LGA1156 (45nm)
				case CPU_MODEL_DALES:
				case CPU_MODEL_DALES_32NM:	// Intel Core i3, i5 LGA1156 (32nm)
				case CPU_MODEL_NEHALEM:		// Intel Core i7, Xeon W35xx, Xeon X55xx, Xeon E55xx LGA1366 (45nm)
				case CPU_MODEL_NEHALEM_EX:	// Intel Xeon X75xx, Xeon X65xx, Xeon E75xx, Xeon E65x
				case CPU_MODEL_WESTMERE:	// Intel Core i7, Xeon X56xx, Xeon E56xx, Xeon W36xx LGA1366 (32nm) 6 Core
				case CPU_MODEL_WESTMERE_EX:	// Intel Xeon E7
				case CPU_MODEL_SANDYBRIDGE:	// Intel Core i3, i5, i7 LGA1155 (32nm)
				case CPU_MODEL_IVYBRIDGE:	// Intel Core i3, i5, i7 LGA1155 (22nm)
				case CPU_MODEL_IVYBRIDGE_XEON:
				case CPU_MODEL_JAKETOWN:	// Intel Core i7, Xeon E5 LGA2011 (32nm)
				case CPU_MODEL_HASWELL:
				case CPU_MODEL_HASWELL_MB:
				case CPU_MODEL_HASWELL_ULT:
				case CPU_MODEL_CRYSTALWELL:

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
void addSMBEndOfTable(SMBStructPtrs *structPtr)
{
	structPtr->new->type	= kSMBTypeEndOfTable;
	structPtr->new->length	= sizeof(SMBStructHeader);
	structPtr->new->handle	= handle++;

	structPtr->new = (SMBStructHeader *)((uint8_t *)structPtr->new + sizeof(SMBStructHeader) + 2);
	tableLength += sizeof(SMBStructHeader) + 2;
	structureCount++;
}

void setSMBStruct(SMBStructPtrs *structPtr)
{
	bool setterFound = false;

	uint8_t *ptr;
	SMBWord structSize;
	int i;

	/* http://forge.voodooprojects.org/p/chameleon/issues/361/ */
	bool forceFullMemInfo = false;

	if (structPtr->orig->type == kSMBTypeMemoryDevice)
	{
		getBoolForKey(kMemFullInfo, &forceFullMemInfo, &bootInfo->chameleonConfig);
		if (forceFullMemInfo)
		{
			structPtr->orig->length = 27;
		}
	}

	stringIndex = 1;
	stringsSize = 0;

	if (handle < structPtr->orig->handle)
	{
		handle = structPtr->orig->handle;
	}

	memcpy((void *)structPtr->new, structPtr->orig, structPtr->orig->length);

	for (i = 0; i < numOfSetters; i++)
	{
		if ((structPtr->orig->type == SMBSetters[i].type) && (SMBSetters[i].fieldOffset < structPtr->orig->length))
		{
			setterFound = true;
			setSMBValue(structPtr, i, (returnType *)((uint8_t *)structPtr->new + SMBSetters[i].fieldOffset));
		}
	}

	if (setterFound)
	{
		ptr = (uint8_t *)structPtr->new + structPtr->orig->length;
		for (; ((uint16_t *)ptr)[0] != 0; ptr++);

		if (((uint16_t *)ptr)[0] == 0)
		{
			ptr += 2;
		}
		structSize = ptr - (uint8_t *)structPtr->new;
	}
	else
	{
		ptr = (uint8_t *)structPtr->orig + structPtr->orig->length;
		for (; ((uint16_t *)ptr)[0] != 0; ptr++);

		if (((uint16_t *)ptr)[0] == 0)
		{
			ptr += 2;
		}

		structSize = ptr - (uint8_t *)structPtr->orig;
		memcpy((void *)structPtr->new, structPtr->orig, structSize);
	}

	structPtr->new = (SMBStructHeader *)((uint8_t *)structPtr->new + structSize);

	tableLength += structSize;

	if (structSize > maxStructSize)
	{
		maxStructSize = structSize;
	}

	structureCount++;
}

void setupNewSMBIOSTable(SMBEntryPoint *eps, SMBStructPtrs *structPtr)
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
		{
			ptr += 2;
		}

		structPtr->orig = (SMBStructHeader *)ptr;
	}

	addSMBFirmwareVolume(structPtr);
	addSMBMemorySPD(structPtr);
	addSMBOemProcessorType(structPtr);
	addSMBOemProcessorBusSpeed(structPtr);

	addSMBEndOfTable(structPtr);
}

// Bungo: does fix system uuid in SMBIOS istead of in EFI only
uint8_t *FixSystemUUID()
{
	uint8_t *ptr = (uint8_t *)neweps->dmi.tableAddress;
	SMBStructHeader *structHeader = (SMBStructHeader *)ptr;
	int i, isZero, isOnes;
	uint8_t FixedUUID[UUID_LEN] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
	const char *sysId = getStringForKey(kSMBSystemInformationUUIDKey, SMBPlist);
	uint8_t *ret = (uint8_t *)getUUIDFromString(sysId);

	for (;(structHeader->type != kSMBTypeSystemInformation);) // find System Information Table (Type 1) in patched SMBIOS
	{
		ptr = (uint8_t *)((uint32_t)structHeader + structHeader->length);
		for (; ((uint16_t *)ptr)[0] != 0; ptr++);

		if (((uint16_t *)ptr)[0] == 0)
		{
			ptr += 2;
		}

		structHeader = (SMBStructHeader *)ptr;
	}

	ptr = ((SMBSystemInformation *)structHeader)->uuid;

	if (!sysId || !ret) // no or bad custom UUID,...
	{
		sysId = 0;
        ret = ((SMBSystemInformation *)structHeader)->uuid; // ...try bios dmi info UUID extraction
	}

	for (i=0, isZero=1, isOnes=1; i<UUID_LEN; i++) // check if empty or setable, means: no uuid present
	{
		if (ret[i] != 0x00)
		{
			isZero = 0;
		}

		if (ret[i] != 0xff)
		{
			isOnes = 0;
		}
	}

	if (isZero || isOnes) // if empty or setable...
	{
		verbose("No UUID present in SMBIOS System Information Table\n");
		ret = FixedUUID; // ...set a fixed value for system-id = 000102030405060708090A0B0C0D0E0F
	}

	memcpy(ptr, ret, UUID_LEN); // fix uuid in the table
	return ptr;
}  // Bungo: end fix

void setupSMBIOSTable(void)
{
	SMBStructPtrs *structPtr;
	uint8_t *buffer;
	// bool setSMB = true; Bungo

	if (!origeps)
	{
		return;
	}

	neweps = origeps;

	structPtr = (SMBStructPtrs *)malloc(sizeof(SMBStructPtrs));
	if (!structPtr)
	{
		return;
	}
	
	buffer = (uint8_t *)malloc(SMB_ALLOC_SIZE);
	if (!buffer)
	{
		free(structPtr);
		return;
	}

	bzero(buffer, SMB_ALLOC_SIZE);
	structPtr->new = (SMBStructHeader *)buffer;

	// getBoolForKey(kSMBIOSdefaults, &setSMB, &bootInfo->chameleonConfig);  Bungo
	getBoolForKey(kSMBIOSdefaults, &useSMBIOSdefaults, &bootInfo->chameleonConfig);
	// if (setSMB)
           setDefaultSMBData();
	// Bungo
	
	setupNewSMBIOSTable(origeps, structPtr);

	neweps = (SMBEntryPoint *)AllocateKernelMemory(sizeof(SMBEntryPoint));
	if (!neweps)
	{
		free(buffer);
		free(structPtr);
		return;
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
		return;
	}

	memcpy((void *)neweps->dmi.tableAddress, buffer, tableLength);

	// Bungo
	Platform.UUID = FixSystemUUID(); // Fix System UUID

	neweps->dmi.checksum		= 0;
	neweps->dmi.checksum		= 0x100 - checksum8(&neweps->dmi, sizeof(DMIEntryPoint));

	neweps->checksum		= 0;
	neweps->checksum		= 0x100 - checksum8(neweps, sizeof(SMBEntryPoint));

	free(buffer);
	free(structPtr);

	decodeSMBIOSTable(neweps);
}

void *getSmbios(int which)
{
	switch (which)
	{
		case SMBIOS_ORIGINAL:
			if (!origeps)
			{
				origeps = getAddressOfSmbiosTable();
			}
			return origeps;
		case SMBIOS_PATCHED:
			return neweps;
	}

	return 0;
}

/* Collect any information needed later */
void readSMBIOSInfo(SMBEntryPoint *eps)
{
	uint8_t *structPtr = (uint8_t *)eps->dmi.tableAddress;
	SMBStructHeader *structHeader = (SMBStructHeader *)structPtr;

	int dimmnbr = 0;
	Platform.DMI.MaxMemorySlots	= 0;	// number of memory slots polulated by SMBIOS
	Platform.DMI.CntMemorySlots	= 0;	// number of memory slots counted
	Platform.DMI.MemoryModules	= 0;

	for (;((eps->dmi.tableAddress + eps->dmi.tableLength) > ((uint32_t)(uint8_t *)structHeader + sizeof(SMBStructHeader)));)
	{
		switch (structHeader->type)
		{
			/* Bungo
			case kSMBTypeSystemInformation:
				Platform.UUID = ((SMBSystemInformation *)structHeader)->uuid;
				break;
			*/

			case kSMBTypePhysicalMemoryArray:
				Platform.DMI.MaxMemorySlots += ((SMBPhysicalMemoryArray *)structHeader)->numMemoryDevices;
				break;

			case kSMBTypeMemoryDevice:
				Platform.DMI.CntMemorySlots++;
				if (((SMBMemoryDevice *)structHeader)->memorySize != 0)
				{
					Platform.DMI.MemoryModules++;
				}
				if (((SMBMemoryDevice *)structHeader)->memorySpeed > 0)
				{
					Platform.RAM.DIMM[dimmnbr].Frequency = ((SMBMemoryDevice *)structHeader)->memorySpeed;
				}
				dimmnbr++;
				break;
			default:
				break;
		}

		structPtr = (uint8_t *)((uint32_t)structHeader + structHeader->length);
		for (; ((uint16_t *)structPtr)[0] != 0; structPtr++);

		if (((uint16_t *)structPtr)[0] == 0)
		{
			structPtr += 2;
		}

		structHeader = (SMBStructHeader *)structPtr;
	}
}
