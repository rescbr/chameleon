/*
 * Template by (c) 2009 Evan Lojewski. All rights reserved.
 *
 * NVRAM module by Slice 2010.
 */

#include "libsaio.h"
#include "modules.h"
#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
#include "efi.h"
#include "smbios_getters.h"
#include "xml.h"

#ifndef DEBUG_NVRAM
#define DEBUG_NVRAM 0
#endif

#if DEBUG_NVRAM
#define DBG(x...) printf(x)
#else
#define DBG(x...) msglog(x)
#endif

/*static const char const FIRMWARE_REVISION_PROP[] = "firmware-revision";
static const char const FIRMWARE_ABI_PROP[] = "firmware-abi";
static const char const FIRMWARE_VENDOR_PROP[] = "firmware-vendor";
static const char const FIRMWARE_ABI_32_PROP_VALUE[] = "EFI32";
static const char const FIRMWARE_ABI_64_PROP_VALUE[] = "EFI64";
static const char const SYSTEM_ID_PROP[] = "system-id";
static const char const SYSTEM_SERIAL_PROP[] = "SystemSerialNumber";
static const char const SYSTEM_TYPE_PROP[] = "system-type";
static const char const MODEL_PROP[] = "Model";
 */
const char PLATFORM_UUID[] = "platform-uuid"; 
//EFI_UINT32 const FIRMWARE_FEATURE_MASK = 0x000003FF;
EFI_UINT32 const STATIC_ZERO = 0;

#define kBL_GLOBAL_NVRAM_GUID "8BE4DF61-93CA-11D2-AA0D-00E098032B8C"

// Check if a system supports CSM legacy mode
#define kBL_APPLE_VENDOR_NVRAM_GUID "4D1EDE05-38C7-4A6A-9CC6-4BCCA8B38C14"
#define UUID_LEN	16

//extern EFI_GUID* getSystemID();
void NVRAM_hook(void* arg1, void* arg2, void* arg3, void* arg4);
void NVRAM_start(void);

typedef struct {
	char Name[32];
	char Value[512];
} variables;
int readNVRAM(variables* v);


void NVRAM_hook(void* arg1, void* arg2, void* arg3, void* arg4);
/*
void NVRAM_start_hook(void* arg1, void* arg2, void* arg3, void* arg4)
{
	msglog("NVRAM started with ExecKernel\n");

}
*/

void NVRAM_start()
{
//	register_hook_callback("ExecKernel", &NVRAM_start_hook); 
	//	register_hook_callback("Kernel Start", &NVRAM_hook);
	register_hook_callback("ModulesLoaded", &NVRAM_hook);
}

void NVRAM_hook(void* arg1, void* arg2, void* arg3, void* arg4)
{
	char		bootName[128];
	variables*	var;
	char*		ffName;
	uint8_t *	FirmwareFeatures;
	EFI_UINT32*	FirmwareFeaturesMask;
	char*		ffmName;
	char*		boName;
	char*		bnName;
//	EFI_GUID*	ret = 0;
	uint16_t	bootOptionNumber = 0;
	int i, j;
	
//	BLESS_EFI_LOAD_OPTION* NextBoot =(BLESS_EFI_LOAD_OPTION*)gBootOrder;
	
	DBG("NVRAM started with ModulesLoaded\n");
	
	//Slice create /options node -> /fakenvram
	// I need to use fakenvram until I know what is happen
//	bool UseNVRAM = FALSE;
	bool ClearNVRAM = FALSE;
	const char* buff;
//	TagPtr	dictionary;
	int cnt;
	var = malloc(sizeof(variables)+1);
	ClearNVRAM = getValueForKey("-c", &buff, &cnt, &bootInfo->bootConfig);
	if (!ClearNVRAM) {		
		readNVRAM(var);
	}

	for (i=0; i<32; i++) {
		if (var[i].Name[0] == 0) {
			break;
		}
		if (strcmp(var[i].Name, "efi-boot-device")==0) {
			//XMLParseFile(var[i].Value, &dictionary);
			//buff = XMLGetProperty(dictionary, "BLLastBSDName")->string;
			verbose("Required boot device is %s\n", var[i].Value);
			//NextBoot->FilePathListLength = 1;
			//strcpy(NextBoot->Description, buff);
			break;
		}
	}
	
	Node* optionsNode = DT__FindNode("/options", true);  //"/fakenvram"
	ffName = malloc(sizeof(PLATFORM_UUID)+1);
	strcpy(ffName, PLATFORM_UUID);
//	ret = getSystemID();
//	DT__AddProperty(optionsNode, ffName, UUID_LEN, (EFI_UINT32*) ret);		
		
		// this information can be obtained from DMI Type 0
		SMBByte* p = (SMBByte*)FindFirstDmiTableOfType(0, 0x18);
		FirmwareFeatures = malloc(sizeof(SMBByte));
		//TODO - the bufferFF must be composed from bits p[0x12] and [0x13]. Datasheet needed
		*FirmwareFeatures = ((p[19] >> 1) & 1)  //USB Legacy is supported
			| ((p[18] >> 14) & 2) //Boot from CD is supported
			| 0x14; //default for bless (GUID partition)
		
		sprintf(bootName, "%s:FirmwareFeatures", kBL_APPLE_VENDOR_NVRAM_GUID);
		ffName = malloc(sizeof(bootName)+1);
		strcpy(ffName, bootName);
		DT__AddProperty(optionsNode, ffName, sizeof(uint32_t), (char *)FirmwareFeatures); //legacy support
		
		sprintf(bootName, "%s:FirmwareFeaturesMask", kBL_APPLE_VENDOR_NVRAM_GUID);
	FirmwareFeaturesMask = malloc(sizeof(EFI_UINT32));
	*FirmwareFeaturesMask = (EFI_UINT32)0x3ff;
		ffmName = malloc(sizeof(bootName)+1);
		strcpy(ffmName, bootName);	
		DT__AddProperty(optionsNode, ffmName, sizeof(uint32_t), (EFI_UINT32*)FirmwareFeaturesMask); 	
		
		//TODO - check, validate and fill by bvr structure.
		//here I am not sure what is BootOrder: node or property?
		//Node* bootNode = DT__AddChild(optionsNode, kBL_GLOBAL_NVRAM_GUID ":BootOrder");
		sprintf(bootName, "%s:BootOrder", kBL_GLOBAL_NVRAM_GUID);
		boName = malloc(sizeof(bootName)+1);
		strcpy(boName, bootName);		
		DT__AddProperty(optionsNode, boName, sizeof(uint32_t), (EFI_UINT32*)&STATIC_ZERO);	
		
		sprintf(bootName, "%s:Boot%04hx", kBL_GLOBAL_NVRAM_GUID, bootOptionNumber);
		bnName = malloc(sizeof(bootName)+1);
		strcpy(bnName, bootName);			
		DT__AddProperty(optionsNode, bnName, sizeof(uint32_t), (EFI_UINT32*)&STATIC_ZERO); 
		
		
		//can we add here boot-properties?
		//	optionsNode = DT__FindNode("chosen", true);
#if NOTYET
		int lbC = 0;
		while(((char*)&bootInfo->bootConfig)[lbC++]);
		if (lbC > sizeof(bootInfo->bootConfig)) lbC = sizeof(bootInfo->bootConfig);
		DT__AddProperty(optionsNode, "boot-args", lbC, (char*)&bootInfo->bootConfig);	
#endif
		//TODO - BootCamp emulation?
		/*
		 romNode = DT__FindNode("/rom", true);
		 DT__AddProperty(romNode, "fv-main-address"...  //provided by AppleSMBIOS
		 DT__AddProperty(romNode, "fv-main-size"...
		 "IOEFIDevicePathType" -> "MediaFirmwareVolumeFilePath"
		 "Guid" -> "2B0585EB-D8B8-49A9-8B8C-E21B01AEF2B7"
		 
		 */
		//

		for (i=0; i<32; i++) {
			DBG("NVRAM get a name %s\n", var[i].Name);
			if (var[i].Name[0]) {
				if (isdigit(var[i].Name[0])) {
					msglog(" ...it is digit...\n");
					continue;
				}
				j=0; 
				while (var[i].Value[j++]);
				DT__AddProperty(optionsNode, var[i].Name, j,&var[i].Value);
#if 1 //DEBUG_NVRAM
				DBG("NVRAM add name=%s value=%s length=%d\n", var[i].Name, var[i].Value, j);
#endif
			} else {
				break;
			}

		}
	//Serialization?
	bnName[0] = bnName[1] = 0;
	DT__AddProperty(optionsNode, bnName, sizeof(uint32_t), (EFI_UINT32*)&STATIC_ZERO);
}

const char NVRAM_INF[] = "nvram.inf";
int readNVRAM(variables* var)
{
	int fd, fsize;
	char* nvr = 0;
	DBG("Start NVRAM reading\n");
	if ((fd = open(NVRAM_INF, 0)) < 0) {
		DBG("[ERROR] open NVRAM failed\n");
		return -1;
	}
	fsize = file_size(fd);
	if (!fsize) {
		DBG(" zero NVRAM file\n");
		close (fd);
		return -1;
	}
	if ((nvr = malloc(fsize)) == NULL) {
		DBG("[ERROR] alloc NVRAM memory failed\n");
		close (fd);
		return -1;
	}
	if (read (fd, nvr, fsize) != fsize) {
		DBG("[ERROR] read %s failed\n", NVRAM_INF);
		close (fd);
		return -1;
	}
	close (fd);
/*	if ((var = malloc(fsize)) == NULL) {
		DBG("[ERROR] alloc VAR memory failed\n");
		return -1;
	}*/
	int i = 0;
	char * onvr = nvr;
	bool skipGUID;
	while (*nvr) {
		int j = 0;
		skipGUID = false;
		DBG("Name[%d]:", i);
		while (*nvr != 9) {
//			DBG("%c", *nvr);
			if (*nvr == 4) {
				skipGUID = true; //TODO this is GUID
				nvr++;
			} else 
				var[i].Name[j++] = *nvr++; 
		}
		DBG("\n");
		nvr++; //skip \09
		var[i].Name[j] = 0; //end of c-string
		if (skipGUID) {
			//TODO this is GUID
			DBG("skip GUID\n");
			while (*nvr++ != 0x0A) {}
			i--;
		} else {
			
			j = 0;
			unsigned char c;
			while (*nvr != 0x0A) {
				c = *nvr++;
				if (c == 0x25) { //TODO this is hex 
					int k1=*nvr++;
					if ((k1 >= 0x30) && (k1 <= 0x39)) {
						k1 = k1 - 0x30;
					}
					if ((k1 > 0x60) && (k1 <= 0x66)) {
						k1 = k1 - 0x60 + 10;
					}
					int k2=*nvr++;
					if ((k2 >= 0x30) && (k2 <= 0x39)) {
						k2 = k2 - 0x30;
					}
					if ((k2 > 0x60) && (k2 <= 0x66)) {
						k2 = k2 - 0x60 + 10;
					}
					c = (k1 << 4) + k2;
					//break; 
				}
				var[i].Value[j++] = c; 
			}
			nvr++;
			DBG("Value[%d]:", i);
			int m;
			for (m=0; m<j; m++) {
//				DBG("%02x ", var[i].Value[m]);
			}
			//	DBG("Value[%d]:%s\n", i, var[i].Value);
			DBG("\n");
		}
		i++;
		if ((int)(nvr - onvr) > fsize) {
			DBG("end of buffer\n");
			break;
		}
	}
	var[i].Name[0]=0;
	var[i].Name[1]=0;

	return 0;
}