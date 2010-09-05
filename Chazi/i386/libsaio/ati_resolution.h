/*
 *  ati_resolution.h
 *  
 *
 *  Created by Le Bidou on 19/03/10.
 *  Copyright 2010 ---. All rights reserved.
 *
 */

#ifndef _ATI_RESOLUTION_H_
#define _ATI_RESOLUTION_H_

//#include "libsaio.h"
#include "libsa.h"
#include "saio_internal.h"
//---
#include "autoresolution.h"

#define ATI_SIGNATURE1 "ATI MOBILITY RADEON"
#define ATI_SIGNATURE2 "ATI Technologies Inc"

/****************************************************************************/	
/*Shortatombios.h parts:                                                    */
/*Portion I: Definitions  shared between VBIOS and Driver                   */
/****************************************************************************/


#define ATOM_VERSION_MAJOR                   0x00020000
#define ATOM_VERSION_MINOR                   0x00000002

#define ATOM_HEADER_VERSION (ATOM_VERSION_MAJOR | ATOM_VERSION_MINOR)

#pragma pack(1)                                       /* BIOS data must use byte aligment */

/*  Define offset to location of ROM header. */

#define OFFSET_TO_POINTER_TO_ATOM_ROM_HEADER		0x00000048L
#define OFFSET_TO_ATOM_ROM_IMAGE_SIZE				    0x00000002L

typedef struct 
{
  uint16_t structureSize;
  uint8_t  TableFormatRevision;   /*Change it when the Parser is not backward compatible */
  uint8_t  TableContentRevision;  /*Change it only when the table needs to change but the firmware */
                                  /*Image can't be updated, while Driver needs to carry the new table! */
}atomCommonTableHeader;

typedef struct
{
  atomCommonTableHeader		sHeader;
  uint8_t	 firmWareSignature[4];    /*Signature to distinguish between Atombios and non-atombios, 
                                      atombios should init it as "ATOM", don't change the position */
  uint16_t biosRuntimeSegmentAddress;
  uint16_t protectedModeInfoOffset;
  uint16_t configFilenameOffset;
  uint16_t cRcBlockOffset;
  uint16_t BiosBootupMessageOffset;
  uint16_t int10Offset;
  uint16_t pciBusDevInitCode;
  uint16_t ioBaseAddress;
  uint16_t subsystemVendorID;
  uint16_t subsystemID;
  uint16_t pciInfoOffset; 
  uint16_t masterCommandTableOffset; /*Offset for SW to get all command table offsets, Don't change the position */
  uint16_t masterDataTableOffset;   /*Offset for SW to get all data table offsets, Don't change the position */
  uint8_t  extendedFunctionCode;
  uint8_t  reserved;
}atomRomHeader;

/****************************************************************************/	
// Structure used in Data.mtb
/****************************************************************************/	
typedef struct
{
  uint16_t        UtilityPipeLine;	        // Offest for the utility to get parser info,Don't change this position!
  uint16_t        MultimediaCapabilityInfo; // Only used by MM Lib,latest version 1.1, not configuable from Bios, need to include the table to build Bios 
  uint16_t        MultimediaConfigInfo;     // Only used by MM Lib,latest version 2.1, not configuable from Bios, need to include the table to build Bios
  uint16_t        StandardVESA_Timing;      // Only used by Bios
  uint16_t        FirmwareInfo;             // Shared by various SW components,latest version 1.4
  uint16_t        DAC_Info;                 // Will be obsolete from R600
  uint16_t        LVDS_Info;                // Shared by various SW components,latest version 1.1 
  uint16_t        TMDS_Info;                // Will be obsolete from R600
  uint16_t        AnalogTV_Info;            // Shared by various SW components,latest version 1.1 
  uint16_t        SupportedDevicesInfo;     // Will be obsolete from R600
  uint16_t        GPIO_I2C_Info;            // Shared by various SW components,latest version 1.2 will be used from R600           
  uint16_t        VRAM_UsageByFirmware;     // Shared by various SW components,latest version 1.3 will be used from R600
  uint16_t        GPIO_Pin_LUT;             // Shared by various SW components,latest version 1.1
  uint16_t        VESA_ToInternalModeLUT;   // Only used by Bios
  uint16_t        ComponentVideoInfo;       // Shared by various SW components,latest version 2.1 will be used from R600
  uint16_t        PowerPlayInfo;            // Shared by various SW components,latest version 2.1,new design from R600
  uint16_t        CompassionateData;        // Will be obsolete from R600
  uint16_t        SaveRestoreInfo;          // Only used by Bios
  uint16_t        PPLL_SS_Info;             // Shared by various SW components,latest version 1.2, used to call SS_Info, change to new name because of int ASIC SS info
  uint16_t        OemInfo;                  // Defined and used by external SW, should be obsolete soon
  uint16_t        XTMDS_Info;               // Will be obsolete from R600
  uint16_t        MclkSS_Info;              // Shared by various SW components,latest version 1.1, only enabled when ext SS chip is used
  uint16_t        Object_Header;            // Shared by various SW components,latest version 1.1
  uint16_t        IndirectIOAccess;         // Only used by Bios,this table position can't change at all!!
  uint16_t        MC_InitParameter;         // Only used by command table
  uint16_t        ASIC_VDDC_Info;						// Will be obsolete from R600
  uint16_t        ASIC_InternalSS_Info;			// New tabel name from R600, used to be called "ASIC_MVDDC_Info"
  uint16_t        TV_VideoMode;							// Only used by command table
  uint16_t        VRAM_Info;								// Only used by command table, latest version 1.3
  uint16_t        MemoryTrainingInfo;				// Used for VBIOS and Diag utility for memory training purpose since R600. the new table rev start from 2.1
  uint16_t        IntegratedSystemInfo;			// Shared by various SW components
  uint16_t        ASIC_ProfilingInfo;				// New table name from R600, used to be called "ASIC_VDDCI_Info" for pre-R600
  uint16_t        VoltageObjectInfo;				// Shared by various SW components, latest version 1.1
  uint16_t				PowerSourceInfo;					// Shared by various SW components, latest versoin 1.1
}atomMasterListOfDataTables;

typedef struct 
{ 
  atomCommonTableHeader			header;  
  atomMasterListOfDataTables	listOfDataTables;
}atomMasterDataTable;

typedef union
{ 
	uint16_t              usAccess;
}atomModeMiscInfoAccess;

/****************************************************************************/	
// Structure used in StandardVESA_TimingTable
//                   AnalogTV_InfoTable 
//                   ComponentVideoInfoTable
/****************************************************************************/	
typedef struct
{
  uint16_t  hTotal;
  uint16_t  hActive;
  uint16_t  hSyncStart;
  uint16_t  hSyncWidth;
  uint16_t  vTotal;
  uint16_t  vActive;
  uint16_t  vSyncStart;
  uint16_t  vSyncWidth;
  uint16_t  pixelClock;					                 //in 10Khz unit
  atomModeMiscInfoAccess  modeMiscInfo;
  uint16_t  overscanRight;
  uint16_t  overscanLeft;
  uint16_t  overscanBottom;
  uint16_t  overscanTop;
  uint16_t  teserve;
  uint8_t   internalModeNumber;
  uint8_t   refreshRate;
}atomModeTiming;

typedef struct
{
  uint16_t  pixelClock;
  uint16_t  hActive;
  uint16_t  hBlank;
  uint16_t  vActive;
  uint16_t  vBlank;			
  uint16_t  hSyncOffset;
  uint16_t  hSyncWidth;
  uint16_t  vSyncOffset;
  uint16_t  vSyncWidth;
  uint16_t  imageHSize;
  uint16_t  imageVSize;
  uint8_t   hBorder;
  uint8_t   vBorder;
  atomModeMiscInfoAccess modeMiscInfo;
  uint8_t   internalModeNumber;
  uint8_t   refreshRate;
}atomDtdFormat;


/****************************************************************************/	
/*Original ati_resolution.h parts:                                          */
/****************************************************************************/


typedef struct
{
  atomCommonTableHeader header;  
  uint8_t * 				 modeTimings;
}atomStandardVesaTiming;




typedef struct
{
    uint8_t         *base;
    atomRomHeader  *atomRomHeader;
    uint16_t         *masterCommandTables;
    uint16_t         *masterDataTables;
} atiBiosTables;

char detectAtiBiosType(sModeTable * table);

vBiosMap * openAtiVbios(vBiosMap * map, atiBiosTables atiTables);

bool atiSetMode_1(sModeTable* map, uint8_t idx, uint32_t* x, uint32_t* y);
bool atiSetMode_2(sModeTable* map, uint8_t idx, uint32_t* x, uint32_t* y);



#endif