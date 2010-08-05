typedef struct {
	char*	symbol;
	unsigned int	addr;
} symbol_t;

static char _AllocateKernelMemory_string[] = "_AllocateKernelMemory";
static char _AllocateMemoryRange_string[] = "_AllocateMemoryRange";
static char _BinaryUnicodeCompare_string[] = "_BinaryUnicodeCompare";
static char _BootHelp_txt_string[] = "_BootHelp_txt";
static char _BootHelp_txt_len_string[] = "_BootHelp_txt_len";
static char _CLCL_string[] = "_CLCL";
static char _CacheInit_string[] = "_CacheInit";
static char _CacheRead_string[] = "_CacheRead";
static char _CacheReset_string[] = "_CacheReset";
static char _CreateUUIDString_string[] = "_CreateUUIDString";
static char _DISTBASE_string[] = "_DISTBASE";
static char _DISTEXTRA_string[] = "_DISTEXTRA";
static char _DT__AddChild_string[] = "_DT__AddChild";
static char _DT__AddProperty_string[] = "_DT__AddProperty";
static char _DT__Finalize_string[] = "_DT__Finalize";
static char _DT__FindNode_string[] = "_DT__FindNode";
static char _DT__FlattenDeviceTree_string[] = "_DT__FlattenDeviceTree";
static char _DT__FreeNode_string[] = "_DT__FreeNode";
static char _DT__FreeProperty_string[] = "_DT__FreeProperty";
static char _DT__GetName_string[] = "_DT__GetName";
static char _DT__Initialize_string[] = "_DT__Initialize";
static char _DecodeKernel_string[] = "_DecodeKernel";
static char _DecodeMachO_string[] = "_DecodeMachO";
static char _DecompressData_string[] = "_DecompressData";
static char _EX2GetDescription_string[] = "_EX2GetDescription";
static char _EX2Probe_string[] = "_EX2Probe";
static char _FastRelString_string[] = "_FastRelString";
static char _FastUnicodeCompare_string[] = "_FastUnicodeCompare";
static char _FindFirstDmiTableOfType_string[] = "_FindFirstDmiTableOfType";
static char _FindNextDmiTableOfType_string[] = "_FindNextDmiTableOfType";
static char _GPT_BASICDATA2_GUID_string[] = "_GPT_BASICDATA2_GUID";
static char _GPT_BASICDATA_GUID_string[] = "_GPT_BASICDATA_GUID";
static char _GPT_BOOT_GUID_string[] = "_GPT_BOOT_GUID";
static char _GPT_EFISYS_GUID_string[] = "_GPT_EFISYS_GUID";
static char _GPT_HFS_GUID_string[] = "_GPT_HFS_GUID";
static char _Gdt_string[] = "_Gdt";
static char _Gdtr_string[] = "_Gdtr";
static char _GetDirEntry_string[] = "_GetDirEntry";
static char _GetFileBlock_string[] = "_GetFileBlock";
static char _GetFileInfo_string[] = "_GetFileInfo";
static char _HFSFree_string[] = "_HFSFree";
static char _HFSGetDescription_string[] = "_HFSGetDescription";
static char _HFSGetDirEntry_string[] = "_HFSGetDirEntry";
static char _HFSGetFileBlock_string[] = "_HFSGetFileBlock";
static char _HFSGetUUID_string[] = "_HFSGetUUID";
static char _HFSInitPartition_string[] = "_HFSInitPartition";
static char _HFSLoadFile_string[] = "_HFSLoadFile";
static char _HFSProbe_string[] = "_HFSProbe";
static char _HFSReadFile_string[] = "_HFSReadFile";
static char _HibernateBoot_string[] = "_HibernateBoot";
static char _HuffmanTree_decode_string[] = "_HuffmanTree_decode";
static char _HuffmanTree_makeFromLengths_string[] = "_HuffmanTree_makeFromLengths";
static char _HuffmanTree_new_string[] = "_HuffmanTree_new";
static char _Idtr_prot_string[] = "_Idtr_prot";
static char _Idtr_real_string[] = "_Idtr_real";
static char _Inflator_error_string[] = "_Inflator_error";
static char _Inflator_generateFixedTrees_string[] = "_Inflator_generateFixedTrees";
static char _Inflator_getTreeInflateDynamic_string[] = "_Inflator_getTreeInflateDynamic";
static char _Inflator_huffmanDecodeSymbol_string[] = "_Inflator_huffmanDecodeSymbol";
static char _Inflator_inflate_string[] = "_Inflator_inflate";
static char _Inflator_inflateHuffmanBlock_string[] = "_Inflator_inflateHuffmanBlock";
static char _Inflator_inflateNoCompression_string[] = "_Inflator_inflateNoCompression";
static char _LENBASE_string[] = "_LENBASE";
static char _LENEXTRA_string[] = "_LENEXTRA";
static char _LoadDrivers_string[] = "_LoadDrivers";
static char _LoadExtraDrivers_p_string[] = "_LoadExtraDrivers_p";
static char _LoadFile_string[] = "_LoadFile";
static char _LoadThinFatFile_string[] = "_LoadThinFatFile";
static char _LoadVolumeFile_string[] = "_LoadVolumeFile";
static char _MD5Final_string[] = "_MD5Final";
static char _MD5Init_string[] = "_MD5Init";
static char _MD5Pad_string[] = "_MD5Pad";
static char _MD5Update_string[] = "_MD5Update";
static char _MSDOSFree_string[] = "_MSDOSFree";
static char _MSDOSGetDescription_string[] = "_MSDOSGetDescription";
static char _MSDOSGetDirEntry_string[] = "_MSDOSGetDirEntry";
static char _MSDOSGetFileBlock_string[] = "_MSDOSGetFileBlock";
static char _MSDOSGetUUID_string[] = "_MSDOSGetUUID";
static char _MSDOSInitPartition_string[] = "_MSDOSInitPartition";
static char _MSDOSLoadFile_string[] = "_MSDOSLoadFile";
static char _MSDOSProbe_string[] = "_MSDOSProbe";
static char _MSDOSReadFile_string[] = "_MSDOSReadFile";
static char _NTFSGetDescription_string[] = "_NTFSGetDescription";
static char _NTFSProbe_string[] = "_NTFSProbe";
static char _PNG_adam7Pass_string[] = "_PNG_adam7Pass";
static char _PNG_checkColorValidity_string[] = "_PNG_checkColorValidity";
static char _PNG_convert_string[] = "_PNG_convert";
static char _PNG_decode_string[] = "_PNG_decode";
static char _PNG_error_string[] = "_PNG_error";
static char _PNG_getBpp_string[] = "_PNG_getBpp";
static char _PNG_info_new_string[] = "_PNG_info_new";
static char _PNG_paethPredictor_string[] = "_PNG_paethPredictor";
static char _PNG_read32bitInt_string[] = "_PNG_read32bitInt";
static char _PNG_readBitFromReversedStream_string[] = "_PNG_readBitFromReversedStream";
static char _PNG_readBitsFromReversedStream_string[] = "_PNG_readBitsFromReversedStream";
static char _PNG_readPngHeader_string[] = "_PNG_readPngHeader";
static char _PNG_setBitOfReversedStream_string[] = "_PNG_setBitOfReversedStream";
static char _PNG_unFilterScanline_string[] = "_PNG_unFilterScanline";
static char _ParseXMLFile_string[] = "_ParseXMLFile";
static char _Platform_string[] = "_Platform";
static char _ReadFileAtOffset_string[] = "_ReadFileAtOffset";
static char _ReadPCIBusInfo_string[] = "_ReadPCIBusInfo";
static char _Round_string[] = "_Round";
static char _Sqrt_string[] = "_Sqrt";
static char _ThinFatFile_string[] = "_ThinFatFile";
static char _XMLFreeTag_string[] = "_XMLFreeTag";
static char _XMLGetProperty_string[] = "_XMLGetProperty";
static char _XMLParseNextTag_string[] = "_XMLParseNextTag";
static char _Zlib_decompress_string[] = "_Zlib_decompress";
static char _Zlib_readBitFromStream_string[] = "_Zlib_readBitFromStream";
static char _Zlib_readBitsFromStream_string[] = "_Zlib_readBitsFromStream";
static char __DATA__bss__begin_string[] = "__DATA__bss__begin";
static char __DATA__bss__end_string[] = "__DATA__bss__end";
static char __DATA__common__begin_string[] = "__DATA__common__begin";
static char __DATA__common__end_string[] = "__DATA__common__end";
static char __bp_string[] = "__bp";
static char __hi_malloc_string[] = "__hi_malloc";
static char __hi_strdup_string[] = "__hi_strdup";
static char __prot_to_real_string[] = "__prot_to_real";
static char __real_to_prot_string[] = "__real_to_prot";
static char __sp_string[] = "__sp";
static char __switch_stack_string[] = "__switch_stack";
static char _acpi10_p_string[] = "_acpi10_p";
static char _acpi20_p_string[] = "_acpi20_p";
static char _addConfigurationTable_string[] = "_addConfigurationTable";
static char _animateProgressBar_string[] = "_animateProgressBar";
static char _archCpuType_string[] = "_archCpuType";
static char _ascii_hex_to_int_string[] = "_ascii_hex_to_int";
static char _ati_aapl01_coher_string[] = "_ati_aapl01_coher";
static char _ati_aapl_blackscr_prefs_0_n4_string[] = "_ati_aapl_blackscr_prefs_0_n4";
static char _ati_aapl_blackscr_prefs_1_n4_string[] = "_ati_aapl_blackscr_prefs_1_n4";
static char _ati_aapl_emc_disp_list_n4_string[] = "_ati_aapl_emc_disp_list_n4";
static char _ati_aux_power_conn_string[] = "_ati_aux_power_conn";
static char _ati_backlight_ctrl_string[] = "_ati_backlight_ctrl";
static char _ati_card_no_string[] = "_ati_card_no";
static char _ati_compatible_0_string[] = "_ati_compatible_0";
static char _ati_compatible_1_string[] = "_ati_compatible_1";
static char _ati_connector_type_0_string[] = "_ati_connector_type_0";
static char _ati_connector_type_0_n4_string[] = "_ati_connector_type_0_n4";
static char _ati_connector_type_1_string[] = "_ati_connector_type_1";
static char _ati_connector_type_1_n4_string[] = "_ati_connector_type_1_n4";
static char _ati_copyright_string[] = "_ati_copyright";
static char _ati_device_type_string[] = "_ati_device_type";
static char _ati_device_type_0_string[] = "_ati_device_type_0";
static char _ati_device_type_1_string[] = "_ati_device_type_1";
static char _ati_display_con_fl_type_0_string[] = "_ati_display_con_fl_type_0";
static char _ati_display_type_0_string[] = "_ati_display_type_0";
static char _ati_display_type_1_string[] = "_ati_display_type_1";
static char _ati_efi_compile_d_string[] = "_ati_efi_compile_d";
static char _ati_efi_disp_conf_string[] = "_ati_efi_disp_conf";
static char _ati_efi_drv_type_string[] = "_ati_efi_drv_type";
static char _ati_efi_enbl_mode_string[] = "_ati_efi_enbl_mode";
static char _ati_efi_init_stat_string[] = "_ati_efi_init_stat";
static char _ati_efi_orientation_string[] = "_ati_efi_orientation";
static char _ati_efi_orientation_n4_string[] = "_ati_efi_orientation_n4";
static char _ati_efi_version_string[] = "_ati_efi_version";
static char _ati_efi_versionB_string[] = "_ati_efi_versionB";
static char _ati_efi_versionE_string[] = "_ati_efi_versionE";
static char _ati_efidisplay_0_string[] = "_ati_efidisplay_0";
static char _ati_efidisplay_0_n4_string[] = "_ati_efidisplay_0_n4";
static char _ati_fb_offset_n4_string[] = "_ati_fb_offset_n4";
static char _ati_hwgpio_n4_string[] = "_ati_hwgpio_n4";
static char _ati_iospace_offset_n4_string[] = "_ati_iospace_offset_n4";
static char _ati_mclk_string[] = "_ati_mclk";
static char _ati_mclk_n4_string[] = "_ati_mclk_n4";
static char _ati_mem_rev_id_string[] = "_ati_mem_rev_id";
static char _ati_mem_vend_id_string[] = "_ati_mem_vend_id";
static char _ati_mrt_string[] = "_ati_mrt";
static char _ati_mvad_string[] = "_ati_mvad";
static char _ati_mvad_n4_string[] = "_ati_mvad_n4";
static char _ati_name_string[] = "_ati_name";
static char _ati_name_0_string[] = "_ati_name_0";
static char _ati_name_1_string[] = "_ati_name_1";
static char _ati_platform_info_string[] = "_ati_platform_info";
static char _ati_refclk_n4_string[] = "_ati_refclk_n4";
static char _ati_regspace_offset_n4_string[] = "_ati_regspace_offset_n4";
static char _ati_romno_string[] = "_ati_romno";
static char _ati_saved_config_string[] = "_ati_saved_config";
static char _ati_saved_config_n4_string[] = "_ati_saved_config_n4";
static char _ati_sclk_string[] = "_ati_sclk";
static char _ati_sclk_n4_string[] = "_ati_sclk_n4";
static char _ati_swgpio_info_n4_string[] = "_ati_swgpio_info_n4";
static char _ati_vendor_id_string[] = "_ati_vendor_id";
static char _ati_vram_memsize_0_string[] = "_ati_vram_memsize_0";
static char _ati_vram_memsize_1_string[] = "_ati_vram_memsize_1";
static char _atoi_string[] = "_atoi";
static char _b_lseek_string[] = "_b_lseek";
static char _bcopy_string[] = "_bcopy";
static char _bgetc_string[] = "_bgetc";
static char _bios_string[] = "_bios";
static char _biosDevIsCDROM_string[] = "_biosDevIsCDROM";
static char _biosread_string[] = "_biosread";
static char _blend_string[] = "_blend";
static char _blendImage_string[] = "_blendImage";
static char _boot_string[] = "_boot";
static char _bootArgs_string[] = "_bootArgs";
static char _bootBanner_string[] = "_bootBanner";
static char _bootImageData_string[] = "_bootImageData";
static char _bootImageHeight_string[] = "_bootImageHeight";
static char _bootImageWidth_string[] = "_bootImageWidth";
static char _bootInfo_string[] = "_bootInfo";
static char _bootPrompt_string[] = "_bootPrompt";
static char _bootRescanPrompt_string[] = "_bootRescanPrompt";
static char _build_pci_dt_string[] = "_build_pci_dt";
static char _builtin_set_string[] = "_builtin_set";
static char _bvChain_string[] = "_bvChain";
static char _bvCount_string[] = "_bvCount";
static char _bvr_string[] = "_bvr";
static char _bzero_string[] = "_bzero";
static char _centeredAt_string[] = "_centeredAt";
static char _centeredIn_string[] = "_centeredIn";
static char _chainLoad_string[] = "_chainLoad";
static char _chainbootdev_string[] = "_chainbootdev";
static char _chainbootflag_string[] = "_chainbootflag";
static char _checksum8_string[] = "_checksum8";
static char _clearActivityIndicator_string[] = "_clearActivityIndicator";
static char _clearGraphicBootPrompt_string[] = "_clearGraphicBootPrompt";
static char _clearScreenRows_string[] = "_clearScreenRows";
static char _close_string[] = "_close";
static char _close_vbios_string[] = "_close_vbios";
static char _closedir_string[] = "_closedir";
static char _colorFont_string[] = "_colorFont";
static char _common_boot_string[] = "_common_boot";
static char _continue_at_low_address_string[] = "_continue_at_low_address";
static char _convertHexStr2Binary_string[] = "_convertHexStr2Binary";
static char _convertImage_string[] = "_convertImage";
static char _copyArgument_string[] = "_copyArgument";
static char _copyMultibootInfo_string[] = "_copyMultibootInfo";
static char _crc32_string[] = "_crc32";
static char _createBackBuffer_string[] = "_createBackBuffer";
static char _createWindowBuffer_string[] = "_createWindowBuffer";
static char _decodeRLE_string[] = "_decodeRLE";
static char _decompress_lzss_string[] = "_decompress_lzss";
static char _delay_string[] = "_delay";
static char _detect_ati_bios_type_string[] = "_detect_ati_bios_type";
static char _detect_bios_type_string[] = "_detect_bios_type";
static char _determine_safe_hi_addr_string[] = "_determine_safe_hi_addr";
static char _devices_number_string[] = "_devices_number";
static char _devprop_add_device_string[] = "_devprop_add_device";
static char _devprop_add_network_template_string[] = "_devprop_add_network_template";
static char _devprop_add_value_string[] = "_devprop_add_value";
static char _devprop_create_string_string[] = "_devprop_create_string";
static char _devprop_free_string_string[] = "_devprop_free_string";
static char _devprop_generate_string_string[] = "_devprop_generate_string";
static char _diskFreeMap_string[] = "_diskFreeMap";
static char _diskIsCDROM_string[] = "_diskIsCDROM";
static char _diskRead_string[] = "_diskRead";
static char _diskResetBootVolumes_string[] = "_diskResetBootVolumes";
static char _diskScanBootVolumes_string[] = "_diskScanBootVolumes";
static char _diskSeek_string[] = "_diskSeek";
static char _dprintf_string[] = "_dprintf";
static char _drawBackground_string[] = "_drawBackground";
static char _drawBootGraphics_string[] = "_drawBootGraphics";
static char _drawCheckerBoard_string[] = "_drawCheckerBoard";
static char _drawColorRectangle_string[] = "_drawColorRectangle";
static char _drawDataRectangle_string[] = "_drawDataRectangle";
static char _drawDeviceIcon_string[] = "_drawDeviceIcon";
static char _drawDeviceList_string[] = "_drawDeviceList";
static char _drawInfoMenu_string[] = "_drawInfoMenu";
static char _drawInfoMenuItems_string[] = "_drawInfoMenuItems";
static char _drawPreview_string[] = "_drawPreview";
static char _drawProgressBar_string[] = "_drawProgressBar";
static char _drawStr_string[] = "_drawStr";
static char _drawStrCenteredAt_string[] = "_drawStrCenteredAt";
static char _dumpAllTablesOfType_string[] = "_dumpAllTablesOfType";
static char _dumpPhysAddr_string[] = "_dumpPhysAddr";
static char _dump_pci_dt_string[] = "_dump_pci_dt";
static char _ebiosEjectMedia_string[] = "_ebiosEjectMedia";
static char _ebiosread_string[] = "_ebiosread";
static char _ebioswrite_string[] = "_ebioswrite";
static char _efi_guid_compare_string[] = "_efi_guid_compare";
static char _efi_guid_is_null_string[] = "_efi_guid_is_null";
static char _efi_guid_unparse_upper_string[] = "_efi_guid_unparse_upper";
static char _efi_inject_get_devprop_string_string[] = "_efi_inject_get_devprop_string";
static char _ehci_acquire_string[] = "_ehci_acquire";
static char _enableA20_string[] = "_enableA20";
static char _enable_pci_devs_string[] = "_enable_pci_devs";
static char _error_string[] = "_error";
static char _file_size_string[] = "_file_size";
static char _fillPixmapWithColor_string[] = "_fillPixmapWithColor";
static char _finalizeBootStruct_string[] = "_finalizeBootStruct";
static char _find_and_read_smbus_controller_string[] = "_find_and_read_smbus_controller";
static char _flipRB_string[] = "_flipRB";
static char _font_console_string[] = "_font_console";
static char _font_small_string[] = "_font_small";
static char _force_enable_hpet_string[] = "_force_enable_hpet";
static char _free_string[] = "_free";
static char _freeFilteredBVChain_string[] = "_freeFilteredBVChain";
static char _freqs_string[] = "_freqs";
static char _gAppleBootPictRLE_string[] = "_gAppleBootPictRLE";
static char _gBIOSBootVolume_string[] = "_gBIOSBootVolume";
static char _gBIOSDev_string[] = "_gBIOSDev";
static char _gBinaryAddress_string[] = "_gBinaryAddress";
static char _gBootFileType_string[] = "_gBootFileType";
static char _gBootFileType_t_string[] = "_gBootFileType_t";
static char _gBootMode_string[] = "_gBootMode";
static char _gBootVolume_string[] = "_gBootVolume";
static char _gCompareTable_string[] = "_gCompareTable";
static char _gCompareTableCompressed_string[] = "_gCompareTableCompressed";
static char _gDeviceCount_string[] = "_gDeviceCount";
static char _gEfiAcpi20TableGuid_string[] = "_gEfiAcpi20TableGuid";
static char _gEfiAcpiTableGuid_string[] = "_gEfiAcpiTableGuid";
static char _gEfiConfigurationTableNode_string[] = "_gEfiConfigurationTableNode";
static char _gEfiSmbiosTableGuid_string[] = "_gEfiSmbiosTableGuid";
static char _gEnableCDROMRescan_string[] = "_gEnableCDROMRescan";
static char _gErrors_string[] = "_gErrors";
static char _gFSLoadAddress_string[] = "_gFSLoadAddress";
static char _gHaveKernelCache_string[] = "_gHaveKernelCache";
static char _gLowerCaseTable_string[] = "_gLowerCaseTable";
static char _gLowerCaseTableCompressed_string[] = "_gLowerCaseTableCompressed";
static char _gMI_string[] = "_gMI";
static char _gMKextName_string[] = "_gMKextName";
static char _gMacOSVersion_string[] = "_gMacOSVersion";
static char _gMemoryMapNode_string[] = "_gMemoryMapNode";
static char _gOverrideKernel_string[] = "_gOverrideKernel";
static char _gPlatformName_string[] = "_gPlatformName";
static char _gRAMDiskBTAliased_string[] = "_gRAMDiskBTAliased";
static char _gRAMDiskFile_string[] = "_gRAMDiskFile";
static char _gRAMDiskMI_string[] = "_gRAMDiskMI";
static char _gRAMDiskVolume_string[] = "_gRAMDiskVolume";
static char _gRootDevice_string[] = "_gRootDevice";
static char _gST_string[] = "_gST";
static char _gScanSingleDrive_string[] = "_gScanSingleDrive";
static char _gVerboseMode_string[] = "_gVerboseMode";
static char _generateCRTCTiming_string[] = "_generateCRTCTiming";
static char _getBVChainForBIOSDev_string[] = "_getBVChainForBIOSDev";
static char _getBoolForKey_string[] = "_getBoolForKey";
static char _getBootOptions_string[] = "_getBootOptions";
static char _getBootVolumeDescription_string[] = "_getBootVolumeDescription";
static char _getBootVolumeRef_string[] = "_getBootVolumeRef";
static char _getColorForKey_string[] = "_getColorForKey";
static char _getConventionalMemorySize_string[] = "_getConventionalMemorySize";
static char _getCroppedPixmapAtPosition_string[] = "_getCroppedPixmapAtPosition";
static char _getCursorPositionAndType_string[] = "_getCursorPositionAndType";
static char _getDDRPartNum_string[] = "_getDDRPartNum";
static char _getDDRSerial_string[] = "_getDDRSerial";
static char _getDDRspeedMhz_string[] = "_getDDRspeedMhz";
static char _getDMIString_string[] = "_getDMIString";
static char _getDimensionForKey_string[] = "_getDimensionForKey";
static char _getEDID_string[] = "_getEDID";
static char _getExtendedMemorySize_string[] = "_getExtendedMemorySize";
static char _getGraphicModeParams_string[] = "_getGraphicModeParams";
static char _getIntForKey_string[] = "_getIntForKey";
static char _getMemoryInfoString_string[] = "_getMemoryInfoString";
static char _getMemoryMap_string[] = "_getMemoryMap";
static char _getMode_string[] = "_getMode";
static char _getNextArg_string[] = "_getNextArg";
static char _getPciRootUID_string[] = "_getPciRootUID";
static char _getPlatformName_string[] = "_getPlatformName";
static char _getResolution_string[] = "_getResolution";
static char _getSmbios_string[] = "_getSmbios";
static char _getStringForKey_string[] = "_getStringForKey";
static char _getStringFromUUID_string[] = "_getStringFromUUID";
static char _getUUIDFromString_string[] = "_getUUIDFromString";
static char _getVBECurrentMode_string[] = "_getVBECurrentMode";
static char _getVBEDACFormat_string[] = "_getVBEDACFormat";
static char _getVBEInfo_string[] = "_getVBEInfo";
static char _getVBEInfoString_string[] = "_getVBEInfoString";
static char _getVBEModeInfo_string[] = "_getVBEModeInfo";
static char _getVBEModeInfoString_string[] = "_getVBEModeInfoString";
static char _getVBEPalette_string[] = "_getVBEPalette";
static char _getVBEPixelClock_string[] = "_getVBEPixelClock";
static char _getValueForBootKey_string[] = "_getValueForBootKey";
static char _getValueForConfigTableKey_string[] = "_getValueForConfigTableKey";
static char _getValueForKey_string[] = "_getValueForKey";
static char _getVendorName_string[] = "_getVendorName";
static char _getVideoMode_string[] = "_getVideoMode";
static char _get_chipset_string[] = "_get_chipset";
static char _get_chipset_id_string[] = "_get_chipset_id";
static char _get_drive_info_string[] = "_get_drive_info";
static char _get_pci_dev_path_string[] = "_get_pci_dev_path";
static char _getc_string[] = "_getc";
static char _getchar_string[] = "_getchar";
static char _getvramsizekb_string[] = "_getvramsizekb";
static char _gprintf_string[] = "_gprintf";
static char _gui_string[] = "_gui";
static char _halt_string[] = "_halt";
static char _hex2bin_string[] = "_hex2bin";
static char _hi_multiboot_string[] = "_hi_multiboot";
static char _imageCnt_string[] = "_imageCnt";
static char _images_string[] = "_images";
static char _infoMenuItems_string[] = "_infoMenuItems";
static char _initFont_string[] = "_initFont";
static char _initGUI_string[] = "_initGUI";
static char _initGraphicsMode_string[] = "_initGraphicsMode";
static char _initKernBootStruct_string[] = "_initKernBootStruct";
static char _initialize_runtime_string[] = "_initialize_runtime";
static char _is_no_emulation_string[] = "_is_no_emulation";
static char _jump_to_chainbooter_string[] = "_jump_to_chainbooter";
static char _kernelSymbolAddresses_string[] = "_kernelSymbolAddresses";
static char _kernelSymbols_string[] = "_kernelSymbols";
static char _lasttime_string[] = "_lasttime";
static char _legacy_off_string[] = "_legacy_off";
static char _loadACPITable_string[] = "_loadACPITable";
static char _loadConfigFile_string[] = "_loadConfigFile";
static char _loadEmbeddedPngImage_string[] = "_loadEmbeddedPngImage";
static char _loadHelperConfig_string[] = "_loadHelperConfig";
static char _loadImageScale_string[] = "_loadImageScale";
static char _loadOverrideConfig_string[] = "_loadOverrideConfig";
static char _loadPngImage_string[] = "_loadPngImage";
static char _loadPrebootRAMDisk_string[] = "_loadPrebootRAMDisk";
static char _loadSystemConfig_string[] = "_loadSystemConfig";
static char _loadThemeValues_string[] = "_loadThemeValues";
static char _loader_string[] = "_loader";
static char _locate_symbols_string[] = "_locate_symbols";
static char _lookUpCLUTIndex_string[] = "_lookUpCLUTIndex";
static char _lspci_string[] = "_lspci";
static char _makeRoundedCorners_string[] = "_makeRoundedCorners";
static char _malloc_init_string[] = "_malloc_init";
static char _map_type1_resolution_string[] = "_map_type1_resolution";
static char _map_type2_resolution_string[] = "_map_type2_resolution";
static char _map_type3_resolution_string[] = "_map_type3_resolution";
static char _mapping_string[] = "_mapping";
static char _md0Ramdisk_string[] = "_md0Ramdisk";
static char _memcmp_string[] = "_memcmp";
static char _memcpy_string[] = "_memcpy";
static char _memset_string[] = "_memset";
static char _menuBVR_string[] = "_menuBVR";
static char _menuItems_string[] = "_menuItems";
static char _mountRAMDisk_string[] = "_mountRAMDisk";
static char _multibootRamdiskReadBytes_string[] = "_multibootRamdiskReadBytes";
static char _multiboot_get_ramdisk_info_string[] = "_multiboot_get_ramdisk_info";
static char _multiboot_partition_string[] = "_multiboot_partition";
static char _multiboot_partition_set_string[] = "_multiboot_partition_set";
static char _multiboot_timeout_string[] = "_multiboot_timeout";
static char _multiboot_timeout_set_string[] = "_multiboot_timeout_set";
static char _multiboot_to_boot_string[] = "_multiboot_to_boot";
static char _nbpScanBootVolumes_string[] = "_nbpScanBootVolumes";
static char _nbpUnloadBaseCode_string[] = "_nbpUnloadBaseCode";
static char _newAPMBVRef_string[] = "_newAPMBVRef";
static char _newFilteredBVChain_string[] = "_newFilteredBVChain";
static char _newGPTBVRef_string[] = "_newGPTBVRef";
static char _newString_string[] = "_newString";
static char _newStringForKey_string[] = "_newStringForKey";
static char _notify_usb_dev_string[] = "_notify_usb_dev";
static char _nvidia_compatible_0_string[] = "_nvidia_compatible_0";
static char _nvidia_compatible_1_string[] = "_nvidia_compatible_1";
static char _nvidia_device_type_string[] = "_nvidia_device_type";
static char _nvidia_device_type_0_string[] = "_nvidia_device_type_0";
static char _nvidia_device_type_1_string[] = "_nvidia_device_type_1";
static char _nvidia_name_0_string[] = "_nvidia_name_0";
static char _nvidia_name_1_string[] = "_nvidia_name_1";
static char _nvidia_slot_name_string[] = "_nvidia_slot_name";
static char _open_string[] = "_open";
static char _open_bvdev_string[] = "_open_bvdev";
static char _open_vbios_string[] = "_open_vbios";
static char _opendir_string[] = "_opendir";
static char _p_get_ramdisk_info_string[] = "_p_get_ramdisk_info";
static char _p_ramdiskReadBytes_string[] = "_p_ramdiskReadBytes";
static char _patchVideoBios_string[] = "_patchVideoBios";
static char _patch_commpage_stuff_routine_string[] = "_patch_commpage_stuff_routine";
static char _patch_cpuid_set_info_string[] = "_patch_cpuid_set_info";
static char _patch_fadt_string[] = "_patch_fadt";
static char _patch_kernel_string[] = "_patch_kernel";
static char _patch_kernel_32_string[] = "_patch_kernel_32";
static char _patch_kernel_64_string[] = "_patch_kernel_64";
static char _patch_lapic_init_string[] = "_patch_lapic_init";
static char _patch_pmCPUExitHaltToOff_string[] = "_patch_pmCPUExitHaltToOff";
static char _pause_string[] = "_pause";
static char _pci_config_read16_string[] = "_pci_config_read16";
static char _pci_config_read32_string[] = "_pci_config_read32";
static char _pci_config_read8_string[] = "_pci_config_read8";
static char _pci_config_write16_string[] = "_pci_config_write16";
static char _pci_config_write32_string[] = "_pci_config_write32";
static char _pci_config_write8_string[] = "_pci_config_write8";
static char _platformCPUFeature_string[] = "_platformCPUFeature";
static char _png_alloc_add_node_string[] = "_png_alloc_add_node";
static char _png_alloc_find_node_string[] = "_png_alloc_find_node";
static char _png_alloc_free_string[] = "_png_alloc_free";
static char _png_alloc_free_all_string[] = "_png_alloc_free_all";
static char _png_alloc_head_string[] = "_png_alloc_head";
static char _png_alloc_malloc_string[] = "_png_alloc_malloc";
static char _png_alloc_realloc_string[] = "_png_alloc_realloc";
static char _png_alloc_remove_node_string[] = "_png_alloc_remove_node";
static char _png_alloc_tail_string[] = "_png_alloc_tail";
static char _pos_string[] = "_pos";
static char _previewLoadedSectors_string[] = "_previewLoadedSectors";
static char _previewSaveunder_string[] = "_previewSaveunder";
static char _previewTotalSectors_string[] = "_previewTotalSectors";
static char _prf_string[] = "_prf";
static char _printVBEModeInfo_string[] = "_printVBEModeInfo";
static char _printf_string[] = "_printf";
static char _processBootArgument_string[] = "_processBootArgument";
static char _processBootOptions_string[] = "_processBootOptions";
static char _processRAMDiskCommand_string[] = "_processRAMDiskCommand";
static char _prompt_string[] = "_prompt";
static char _promptForRescanOption_string[] = "_promptForRescanOption";
static char _prompt_pos_string[] = "_prompt_pos";
static char _prompt_text_string[] = "_prompt_text";
static char _ptol_string[] = "_ptol";
static char _putc_string[] = "_putc";
static char _putca_string[] = "_putca";
static char _putchar_string[] = "_putchar";
static char _rawDiskRead_string[] = "_rawDiskRead";
static char _rawDiskWrite_string[] = "_rawDiskWrite";
static char _read_string[] = "_read";
static char _readBootSector_string[] = "_readBootSector";
static char _readEDID_string[] = "_readEDID";
static char _readKeyboardShiftFlags_string[] = "_readKeyboardShiftFlags";
static char _readKeyboardStatus_string[] = "_readKeyboardStatus";
static char _readdir_string[] = "_readdir";
static char _readdir_ext_string[] = "_readdir_ext";
static char _realloc_string[] = "_realloc";
static char _recoveryMode_string[] = "_recoveryMode";
static char _relock_vbios_string[] = "_relock_vbios";
static char _rescanBIOSDevice_string[] = "_rescanBIOSDevice";
static char _reserveKernBootStruct_string[] = "_reserveKernBootStruct";
static char _root_pci_dev_string[] = "_root_pci_dev";
static char _safe_malloc_string[] = "_safe_malloc";
static char _scanBootVolumes_string[] = "_scanBootVolumes";
static char _scanDisks_string[] = "_scanDisks";
static char _scan_cpu_string[] = "_scan_cpu";
static char _scan_mem_string[] = "_scan_mem";
static char _scan_memory_string[] = "_scan_memory";
static char _scan_pci_bus_string[] = "_scan_pci_bus";
static char _scan_platform_string[] = "_scan_platform";
static char _scan_spd_string[] = "_scan_spd";
static char _scollPage_string[] = "_scollPage";
static char _search_and_get_acpi_fd_string[] = "_search_and_get_acpi_fd";
static char _selectAlternateBootDevice_string[] = "_selectAlternateBootDevice";
static char _selectBootVolume_string[] = "_selectBootVolume";
static char _selectIndex_string[] = "_selectIndex";
static char _setActiveDisplayPage_string[] = "_setActiveDisplayPage";
static char _setBootGlobals_string[] = "_setBootGlobals";
static char _setCursorPosition_string[] = "_setCursorPosition";
static char _setCursorType_string[] = "_setCursorType";
static char _setRAMDiskBTHook_string[] = "_setRAMDiskBTHook";
static char _setRootVolume_string[] = "_setRootVolume";
static char _setVBEDACFormat_string[] = "_setVBEDACFormat";
static char _setVBEMode_string[] = "_setVBEMode";
static char _setVBEPalette_string[] = "_setVBEPalette";
static char _setVideoMode_string[] = "_setVideoMode";
static char _set_eth_builtin_string[] = "_set_eth_builtin";
static char _set_mode_string[] = "_set_mode";
static char _setupAcpi_string[] = "_setupAcpi";
static char _setupAcpiNoMod_string[] = "_setupAcpiNoMod";
static char _setupDeviceProperties_string[] = "_setupDeviceProperties";
static char _setupEfiDeviceTree_string[] = "_setupEfiDeviceTree";
static char _setupEfiTables_string[] = "_setupEfiTables";
static char _setupFakeEfi_string[] = "_setupFakeEfi";
static char _setupSystemType_string[] = "_setupSystemType";
static char _setup_ati_devprop_string[] = "_setup_ati_devprop";
static char _setup_nvidia_devprop_string[] = "_setup_nvidia_devprop";
static char _setup_pci_devs_string[] = "_setup_pci_devs";
static char _showHelp_string[] = "_showHelp";
static char _showInfoBox_string[] = "_showInfoBox";
static char _showInfoRAMDisk_string[] = "_showInfoRAMDisk";
static char _showTextFile_string[] = "_showTextFile";
static char _sleep_string[] = "_sleep";
static char _slvprintf_string[] = "_slvprintf";
static char _smb_read_byte_intel_string[] = "_smb_read_byte_intel";
static char _smbios_p_string[] = "_smbios_p";
static char _smbios_properties_string[] = "_smbios_properties";
static char _smbios_table_descriptions_string[] = "_smbios_table_descriptions";
static char _spinActivityIndicator_string[] = "_spinActivityIndicator";
static char _sprintf_string[] = "_sprintf";
static char _start_string[] = "_start";
static char _startprog_string[] = "_startprog";
static char _stop_string[] = "_stop";
static char _stosl_string[] = "_stosl";
static char _strcat_string[] = "_strcat";
static char _strcmp_string[] = "_strcmp";
static char _strcpy_string[] = "_strcpy";
static char _strdup_string[] = "_strdup";
static char _string_string[] = "_string";
static char _stringLength_string[] = "_stringLength";
static char _stringdata_string[] = "_stringdata";
static char _stringlength_string[] = "_stringlength";
static char _strlcpy_string[] = "_strlcpy";
static char _strlen_string[] = "_strlen";
static char _strncat_string[] = "_strncat";
static char _strncmp_string[] = "_strncmp";
static char _strncpy_string[] = "_strncpy";
static char _strstr_string[] = "_strstr";
static char _strtol_string[] = "_strtol";
static char _strtoul_string[] = "_strtoul";
static char _strtouq_string[] = "_strtouq";
static char _sysConfigValid_string[] = "_sysConfigValid";
static char _systemConfigDir_string[] = "_systemConfigDir";
static char _tell_string[] = "_tell";
static char _testBiosread_string[] = "_testBiosread";
static char _testFAT32EFIBootSector_string[] = "_testFAT32EFIBootSector";
static char _textAddress_string[] = "_textAddress";
static char _textSection_string[] = "_textSection";
static char _time18_string[] = "_time18";
static char _uhci_reset_string[] = "_uhci_reset";
static char _umountRAMDisk_string[] = "_umountRAMDisk";
static char _unlock_vbios_string[] = "_unlock_vbios";
static char _updateGraphicBootPrompt_string[] = "_updateGraphicBootPrompt";
static char _updateInfoMenu_string[] = "_updateInfoMenu";
static char _updateProgressBar_string[] = "_updateProgressBar";
static char _updateVRAM_string[] = "_updateVRAM";
static char _usbList_string[] = "_usbList";
static char _usb_loop_string[] = "_usb_loop";
static char _useGUI_string[] = "_useGUI";
static char _utf_decodestr_string[] = "_utf_decodestr";
static char _utf_encodestr_string[] = "_utf_encodestr";
static char _vector32_cleanup_string[] = "_vector32_cleanup";
static char _vector32_init_string[] = "_vector32_init";
static char _vector32_new_string[] = "_vector32_new";
static char _vector32_resize_string[] = "_vector32_resize";
static char _vector32_resizev_string[] = "_vector32_resizev";
static char _vector8_cleanup_string[] = "_vector8_cleanup";
static char _vector8_copy_string[] = "_vector8_copy";
static char _vector8_init_string[] = "_vector8_init";
static char _vector8_new_string[] = "_vector8_new";
static char _vector8_resize_string[] = "_vector8_resize";
static char _vector8_resizev_string[] = "_vector8_resizev";
static char _vendorMap_string[] = "_vendorMap";
static char _verbose_string[] = "_verbose";
static char _video_mode_string[] = "_video_mode";
static char _vol_opendir_string[] = "_vol_opendir";
static char _vprf_string[] = "_vprf";
static char _vramwrite_string[] = "_vramwrite";
static char _waitThenReload_string[] = "_waitThenReload";
static char boot2_string[] = "boot2";
symbol_t symbolList[] = {
	{.symbol = _AllocateKernelMemory_string, .addr = 0x000331bd},
	{.symbol = _AllocateMemoryRange_string, .addr = 0x00033235},
	{.symbol = _BinaryUnicodeCompare_string, .addr = 0x0003b966},
	{.symbol = _BootHelp_txt_string, .addr = 0x000497d8},
	{.symbol = _BootHelp_txt_len_string, .addr = 0x0004aa9c},
	{.symbol = _CLCL_string, .addr = 0x000476d8},
	{.symbol = _CacheInit_string, .addr = 0x00033e04},
	{.symbol = _CacheRead_string, .addr = 0x00033cc2},
	{.symbol = _CacheReset_string, .addr = 0x00033cb3},
	{.symbol = _CreateUUIDString_string, .addr = 0x0002d33c},
	{.symbol = _DISTBASE_string, .addr = 0x000475e8},
	{.symbol = _DISTEXTRA_string, .addr = 0x00047660},
	{.symbol = _DT__AddChild_string, .addr = 0x00033ff8},
	{.symbol = _DT__AddProperty_string, .addr = 0x00033f31},
	{.symbol = _DT__Finalize_string, .addr = 0x00034214},
	{.symbol = _DT__FindNode_string, .addr = 0x0003429d},
	{.symbol = _DT__FlattenDeviceTree_string, .addr = 0x000341a7},
	{.symbol = _DT__FreeNode_string, .addr = 0x00033ee8},
	{.symbol = _DT__FreeProperty_string, .addr = 0x00033ed2},
	{.symbol = _DT__GetName_string, .addr = 0x00033efe},
	{.symbol = _DT__Initialize_string, .addr = 0x000340c0},
	{.symbol = _DecodeKernel_string, .addr = 0x0002367c},
	{.symbol = _DecodeMachO_string, .addr = 0x0002d774},
	{.symbol = _DecompressData_string, .addr = 0x00028f2e},
	{.symbol = _EX2GetDescription_string, .addr = 0x00034e9b},
	{.symbol = _EX2Probe_string, .addr = 0x00034e82},
	{.symbol = _FastRelString_string, .addr = 0x0003ba89},
	{.symbol = _FastUnicodeCompare_string, .addr = 0x0003bb09},
	{.symbol = _FindFirstDmiTableOfType_string, .addr = 0x0003a0b8},
	{.symbol = _FindNextDmiTableOfType_string, .addr = 0x0003a058},
	{.symbol = _GPT_BASICDATA2_GUID_string, .addr = 0x00047da4},
	{.symbol = _GPT_BASICDATA_GUID_string, .addr = 0x00047d94},
	{.symbol = _GPT_BOOT_GUID_string, .addr = 0x00047d74},
	{.symbol = _GPT_EFISYS_GUID_string, .addr = 0x00047d84},
	{.symbol = _GPT_HFS_GUID_string, .addr = 0x00047d64},
	{.symbol = _Gdt_string, .addr = 0x000204e8},
	{.symbol = _Gdtr_string, .addr = 0x00020520},
	{.symbol = _GetDirEntry_string, .addr = 0x0002cf95},
	{.symbol = _GetFileBlock_string, .addr = 0x0002d23d},
	{.symbol = _GetFileInfo_string, .addr = 0x0002d27b},
	{.symbol = _HFSFree_string, .addr = 0x00035105},
	{.symbol = _HFSGetDescription_string, .addr = 0x0003634b},
	{.symbol = _HFSGetDirEntry_string, .addr = 0x000360b2},
	{.symbol = _HFSGetFileBlock_string, .addr = 0x000363dc},
	{.symbol = _HFSGetUUID_string, .addr = 0x0003607d},
	{.symbol = _HFSInitPartition_string, .addr = 0x00035cf9},
	{.symbol = _HFSLoadFile_string, .addr = 0x000362fd},
	{.symbol = _HFSProbe_string, .addr = 0x0003631b},
	{.symbol = _HFSReadFile_string, .addr = 0x0003617d},
	{.symbol = _HibernateBoot_string, .addr = 0x00028afc},
	{.symbol = _HuffmanTree_decode_string, .addr = 0x0002643c},
	{.symbol = _HuffmanTree_makeFromLengths_string, .addr = 0x00027013},
	{.symbol = _HuffmanTree_new_string, .addr = 0x00026920},
	{.symbol = _Idtr_prot_string, .addr = 0x00020530},
	{.symbol = _Idtr_real_string, .addr = 0x00020528},
	{.symbol = _Inflator_error_string, .addr = 0x00050240},
	{.symbol = _Inflator_generateFixedTrees_string, .addr = 0x00027613},
	{.symbol = _Inflator_getTreeInflateDynamic_string, .addr = 0x00027188},
	{.symbol = _Inflator_huffmanDecodeSymbol_string, .addr = 0x00026a27},
	{.symbol = _Inflator_inflate_string, .addr = 0x00027f29},
	{.symbol = _Inflator_inflateHuffmanBlock_string, .addr = 0x000276e8},
	{.symbol = _Inflator_inflateNoCompression_string, .addr = 0x00027e18},
	{.symbol = _LENBASE_string, .addr = 0x00047500},
	{.symbol = _LENEXTRA_string, .addr = 0x00047574},
	{.symbol = _LoadDrivers_string, .addr = 0x0002318e},
	{.symbol = _LoadExtraDrivers_p_string, .addr = 0x00050030},
	{.symbol = _LoadFile_string, .addr = 0x0002d740},
	{.symbol = _LoadThinFatFile_string, .addr = 0x0002d420},
	{.symbol = _LoadVolumeFile_string, .addr = 0x0002cbbf},
	{.symbol = _MD5Final_string, .addr = 0x00034e58},
	{.symbol = _MD5Init_string, .addr = 0x000345d7},
	{.symbol = _MD5Pad_string, .addr = 0x00034e09},
	{.symbol = _MD5Update_string, .addr = 0x00034d4d},
	{.symbol = _MSDOSFree_string, .addr = 0x00036696},
	{.symbol = _MSDOSGetDescription_string, .addr = 0x00037637},
	{.symbol = _MSDOSGetDirEntry_string, .addr = 0x000371c8},
	{.symbol = _MSDOSGetFileBlock_string, .addr = 0x000374bc},
	{.symbol = _MSDOSGetUUID_string, .addr = 0x00036e55},
	{.symbol = _MSDOSInitPartition_string, .addr = 0x00036c7f},
	{.symbol = _MSDOSLoadFile_string, .addr = 0x0003710b},
	{.symbol = _MSDOSProbe_string, .addr = 0x00037129},
	{.symbol = _MSDOSReadFile_string, .addr = 0x00036ee3},
	{.symbol = _NTFSGetDescription_string, .addr = 0x000377fa},
	{.symbol = _NTFSProbe_string, .addr = 0x000377d7},
	{.symbol = _PNG_adam7Pass_string, .addr = 0x00026d8d},
	{.symbol = _PNG_checkColorValidity_string, .addr = 0x0002659d},
	{.symbol = _PNG_convert_string, .addr = 0x00027972},
	{.symbol = _PNG_decode_string, .addr = 0x000281d5},
	{.symbol = _PNG_error_string, .addr = 0x0005002c},
	{.symbol = _PNG_getBpp_string, .addr = 0x000265f7},
	{.symbol = _PNG_info_new_string, .addr = 0x00028165},
	{.symbol = _PNG_paethPredictor_string, .addr = 0x0002672b},
	{.symbol = _PNG_read32bitInt_string, .addr = 0x00026577},
	{.symbol = _PNG_readBitFromReversedStream_string, .addr = 0x000264e9},
	{.symbol = _PNG_readBitsFromReversedStream_string, .addr = 0x00026510},
	{.symbol = _PNG_readPngHeader_string, .addr = 0x0002661c},
	{.symbol = _PNG_setBitOfReversedStream_string, .addr = 0x00026555},
	{.symbol = _PNG_unFilterScanline_string, .addr = 0x00026ae5},
	{.symbol = _ParseXMLFile_string, .addr = 0x0002fc42},
	{.symbol = _Platform_string, .addr = 0x00050650},
	{.symbol = _ReadFileAtOffset_string, .addr = 0x0002cfd0},
	{.symbol = _ReadPCIBusInfo_string, .addr = 0x00031b2b},
	{.symbol = _Round_string, .addr = 0x00032a79},
	{.symbol = _Sqrt_string, .addr = 0x00032a8f},
	{.symbol = _ThinFatFile_string, .addr = 0x0002da6e},
	{.symbol = _XMLFreeTag_string, .addr = 0x000334a4},
	{.symbol = _XMLGetProperty_string, .addr = 0x00033366},
	{.symbol = _XMLParseNextTag_string, .addr = 0x000335d3},
	{.symbol = _Zlib_decompress_string, .addr = 0x0002806d},
	{.symbol = _Zlib_readBitFromStream_string, .addr = 0x00026487},
	{.symbol = _Zlib_readBitsFromStream_string, .addr = 0x000264ac},
	{.symbol = __DATA__bss__begin_string, .addr = 0x0004dbc0},
	{.symbol = __DATA__bss__end_string, .addr = 0x0004f2c0},
	{.symbol = __DATA__common__begin_string, .addr = 0x0004f2c0},
	{.symbol = __DATA__common__end_string, .addr = 0x00050878},
	{.symbol = __bp_string, .addr = 0x000203aa},
	{.symbol = __hi_malloc_string, .addr = 0x000257c0},
	{.symbol = __hi_strdup_string, .addr = 0x000257d1},
	{.symbol = __prot_to_real_string, .addr = 0x0002032d},
	{.symbol = __real_to_prot_string, .addr = 0x000202df},
	{.symbol = __sp_string, .addr = 0x000203a7},
	{.symbol = __switch_stack_string, .addr = 0x000203ad},
	{.symbol = _acpi10_p_string, .addr = 0x00050858},
	{.symbol = _acpi20_p_string, .addr = 0x00050860},
	{.symbol = _addConfigurationTable_string, .addr = 0x00031984},
	{.symbol = _animateProgressBar_string, .addr = 0x0002a2bc},
	{.symbol = _archCpuType_string, .addr = 0x0004b004},
	{.symbol = _ascii_hex_to_int_string, .addr = 0x0003a122},
	{.symbol = _ati_aapl01_coher_string, .addr = 0x0004d5a0},
	{.symbol = _ati_aapl_blackscr_prefs_0_n4_string, .addr = 0x0004d928},
	{.symbol = _ati_aapl_blackscr_prefs_1_n4_string, .addr = 0x0004d934},
	{.symbol = _ati_aapl_emc_disp_list_n4_string, .addr = 0x0004d860},
	{.symbol = _ati_aux_power_conn_string, .addr = 0x0004d588},
	{.symbol = _ati_backlight_ctrl_string, .addr = 0x0004d594},
	{.symbol = _ati_card_no_string, .addr = 0x0004d5ac},
	{.symbol = _ati_compatible_0_string, .addr = 0x0004d50c},
	{.symbol = _ati_compatible_1_string, .addr = 0x0004d514},
	{.symbol = _ati_connector_type_0_string, .addr = 0x0004d554},
	{.symbol = _ati_connector_type_0_n4_string, .addr = 0x0004d848},
	{.symbol = _ati_connector_type_1_string, .addr = 0x0004d560},
	{.symbol = _ati_connector_type_1_n4_string, .addr = 0x0004d854},
	{.symbol = _ati_copyright_string, .addr = 0x0004d5b4},
	{.symbol = _ati_device_type_string, .addr = 0x0004d52c},
	{.symbol = _ati_device_type_0_string, .addr = 0x0004d51c},
	{.symbol = _ati_device_type_1_string, .addr = 0x0004d524},
	{.symbol = _ati_display_con_fl_type_0_string, .addr = 0x0004d56c},
	{.symbol = _ati_display_type_0_string, .addr = 0x0004d578},
	{.symbol = _ati_display_type_1_string, .addr = 0x0004d580},
	{.symbol = _ati_efi_compile_d_string, .addr = 0x0004d5bc},
	{.symbol = _ati_efi_disp_conf_string, .addr = 0x0004d5c4},
	{.symbol = _ati_efi_drv_type_string, .addr = 0x0004d5d4},
	{.symbol = _ati_efi_enbl_mode_string, .addr = 0x0004d5e0},
	{.symbol = _ati_efi_init_stat_string, .addr = 0x0004d5ec},
	{.symbol = _ati_efi_orientation_string, .addr = 0x0004d5f8},
	{.symbol = _ati_efi_orientation_n4_string, .addr = 0x0004d94c},
	{.symbol = _ati_efi_version_string, .addr = 0x0004d604},
	{.symbol = _ati_efi_versionB_string, .addr = 0x0004d60c},
	{.symbol = _ati_efi_versionE_string, .addr = 0x0004d614},
	{.symbol = _ati_efidisplay_0_string, .addr = 0x0004d54c},
	{.symbol = _ati_efidisplay_0_n4_string, .addr = 0x0004d840},
	{.symbol = _ati_fb_offset_n4_string, .addr = 0x0004d8a8},
	{.symbol = _ati_hwgpio_n4_string, .addr = 0x0004d8b8},
	{.symbol = _ati_iospace_offset_n4_string, .addr = 0x0004d8c4},
	{.symbol = _ati_mclk_string, .addr = 0x0004d61c},
	{.symbol = _ati_mclk_n4_string, .addr = 0x0004d8d4},
	{.symbol = _ati_mem_rev_id_string, .addr = 0x0004d628},
	{.symbol = _ati_mem_vend_id_string, .addr = 0x0004d634},
	{.symbol = _ati_mrt_string, .addr = 0x0004d640},
	{.symbol = _ati_mvad_string, .addr = 0x0004d6f0},
	{.symbol = _ati_mvad_n4_string, .addr = 0x0004d958},
	{.symbol = _ati_name_string, .addr = 0x0004d544},
	{.symbol = _ati_name_0_string, .addr = 0x0004d534},
	{.symbol = _ati_name_1_string, .addr = 0x0004d53c},
	{.symbol = _ati_platform_info_string, .addr = 0x0004d668},
	{.symbol = _ati_refclk_n4_string, .addr = 0x0004d8ec},
	{.symbol = _ati_regspace_offset_n4_string, .addr = 0x0004d8f8},
	{.symbol = _ati_romno_string, .addr = 0x0004d648},
	{.symbol = _ati_saved_config_string, .addr = 0x0004d738},
	{.symbol = _ati_saved_config_n4_string, .addr = 0x0004da60},
	{.symbol = _ati_sclk_string, .addr = 0x0004d650},
	{.symbol = _ati_sclk_n4_string, .addr = 0x0004d8e0},
	{.symbol = _ati_swgpio_info_n4_string, .addr = 0x0004d940},
	{.symbol = _ati_vendor_id_string, .addr = 0x0004d65c},
	{.symbol = _ati_vram_memsize_0_string, .addr = 0x0004d908},
	{.symbol = _ati_vram_memsize_1_string, .addr = 0x0004d918},
	{.symbol = _atoi_string, .addr = 0x0003dcd9},
	{.symbol = _b_lseek_string, .addr = 0x0002d50c},
	{.symbol = _bcopy_string, .addr = 0x0003db83},
	{.symbol = _bgetc_string, .addr = 0x000324a6},
	{.symbol = _bios_string, .addr = 0x0002040a},
	{.symbol = _biosDevIsCDROM_string, .addr = 0x0002dbf7},
	{.symbol = _biosread_string, .addr = 0x00031f57},
	{.symbol = _blend_string, .addr = 0x0002951f},
	{.symbol = _blendImage_string, .addr = 0x00021126},
	{.symbol = _boot_string, .addr = 0x000210ed},
	{.symbol = _bootArgs_string, .addr = 0x0004f2c0},
	{.symbol = _bootBanner_string, .addr = 0x0004959c},
	{.symbol = _bootImageData_string, .addr = 0x0004af6c},
	{.symbol = _bootImageHeight_string, .addr = 0x0004af6a},
	{.symbol = _bootImageWidth_string, .addr = 0x0004af68},
	{.symbol = _bootInfo_string, .addr = 0x0004f2c4},
	{.symbol = _bootPrompt_string, .addr = 0x00049602},
	{.symbol = _bootRescanPrompt_string, .addr = 0x000496ca},
	{.symbol = _build_pci_dt_string, .addr = 0x00033c54},
	{.symbol = _builtin_set_string, .addr = 0x0004d10c},
	{.symbol = _bvChain_string, .addr = 0x0004f2c8},
	{.symbol = _bvCount_string, .addr = 0x00049584},
	{.symbol = _bvr_string, .addr = 0x0004f2cc},
	{.symbol = _bzero_string, .addr = 0x0003dba4},
	{.symbol = _centeredAt_string, .addr = 0x000296dd},
	{.symbol = _centeredIn_string, .addr = 0x000296a3},
	{.symbol = _chainLoad_string, .addr = 0x00025d84},
	{.symbol = _chainbootdev_string, .addr = 0x000204dc},
	{.symbol = _chainbootflag_string, .addr = 0x000204dd},
	{.symbol = _checksum8_string, .addr = 0x0003dd3a},
	{.symbol = _clearActivityIndicator_string, .addr = 0x000225da},
	{.symbol = _clearGraphicBootPrompt_string, .addr = 0x00029792},
	{.symbol = _clearScreenRows_string, .addr = 0x00031c1b},
	{.symbol = _close_string, .addr = 0x0002cbd5},
	{.symbol = _close_vbios_string, .addr = 0x00030e03},
	{.symbol = _closedir_string, .addr = 0x0002d07d},
	{.symbol = _colorFont_string, .addr = 0x000297e3},
	{.symbol = _common_boot_string, .addr = 0x00020560},
	{.symbol = _continue_at_low_address_string, .addr = 0x000202b4},
	{.symbol = _convertHexStr2Binary_string, .addr = 0x0003a17d},
	{.symbol = _convertImage_string, .addr = 0x000229b3},
	{.symbol = _copyArgument_string, .addr = 0x00023b5a},
	{.symbol = _copyMultibootInfo_string, .addr = 0x000258b7},
	{.symbol = _crc32_string, .addr = 0x0003ea1d},
	{.symbol = _createBackBuffer_string, .addr = 0x0002a143},
	{.symbol = _createWindowBuffer_string, .addr = 0x0002a0bd},
	{.symbol = _decodeRLE_string, .addr = 0x000220dd},
	{.symbol = _decompress_lzss_string, .addr = 0x00025620},
	{.symbol = _delay_string, .addr = 0x00031af5},
	{.symbol = _detect_ati_bios_type_string, .addr = 0x00030701},
	{.symbol = _detect_bios_type_string, .addr = 0x000306aa},
	{.symbol = _determine_safe_hi_addr_string, .addr = 0x00025731},
	{.symbol = _devices_number_string, .addr = 0x0004d108},
	{.symbol = _devprop_add_device_string, .addr = 0x0003b20c},
	{.symbol = _devprop_add_network_template_string, .addr = 0x0003b165},
	{.symbol = _devprop_add_value_string, .addr = 0x0003aff8},
	{.symbol = _devprop_create_string_string, .addr = 0x0003b1bf},
	{.symbol = _devprop_free_string_string, .addr = 0x0003ad83},
	{.symbol = _devprop_generate_string_string, .addr = 0x0003adf6},
	{.symbol = _diskFreeMap_string, .addr = 0x0002ddd4},
	{.symbol = _diskIsCDROM_string, .addr = 0x0002dc1b},
	{.symbol = _diskRead_string, .addr = 0x0002e742},
	{.symbol = _diskResetBootVolumes_string, .addr = 0x0002df4b},
	{.symbol = _diskScanBootVolumes_string, .addr = 0x0002eaa8},
	{.symbol = _diskSeek_string, .addr = 0x0002db28},
	{.symbol = _dprintf_string, .addr = 0x0002a76f},
	{.symbol = _drawBackground_string, .addr = 0x0002a6b1},
	{.symbol = _drawBootGraphics_string, .addr = 0x000299a5},
	{.symbol = _drawCheckerBoard_string, .addr = 0x0002138e},
	{.symbol = _drawColorRectangle_string, .addr = 0x000225f8},
	{.symbol = _drawDataRectangle_string, .addr = 0x00021842},
	{.symbol = _drawDeviceIcon_string, .addr = 0x0002aa53},
	{.symbol = _drawDeviceList_string, .addr = 0x0002c252},
	{.symbol = _drawInfoMenu_string, .addr = 0x0002bacd},
	{.symbol = _drawInfoMenuItems_string, .addr = 0x00029d71},
	{.symbol = _drawPreview_string, .addr = 0x00022742},
	{.symbol = _drawProgressBar_string, .addr = 0x0002a361},
	{.symbol = _drawStr_string, .addr = 0x00029cc3},
	{.symbol = _drawStrCenteredAt_string, .addr = 0x00029bf2},
	{.symbol = _dumpAllTablesOfType_string, .addr = 0x00038597},
	{.symbol = _dumpPhysAddr_string, .addr = 0x0003812b},
	{.symbol = _dump_pci_dt_string, .addr = 0x00033a99},
	{.symbol = _ebiosEjectMedia_string, .addr = 0x00031ce6},
	{.symbol = _ebiosread_string, .addr = 0x00031e84},
	{.symbol = _ebioswrite_string, .addr = 0x00031d93},
	{.symbol = _efi_guid_compare_string, .addr = 0x0003ea76},
	{.symbol = _efi_guid_is_null_string, .addr = 0x0003ea4e},
	{.symbol = _efi_guid_unparse_upper_string, .addr = 0x0003eac7},
	{.symbol = _efi_inject_get_devprop_string_string, .addr = 0x0003afc8},
	{.symbol = _ehci_acquire_string, .addr = 0x00032606},
	{.symbol = _enableA20_string, .addr = 0x0002f7ba},
	{.symbol = _enable_pci_devs_string, .addr = 0x000339cd},
	{.symbol = _error_string, .addr = 0x000302fd},
	{.symbol = _file_size_string, .addr = 0x0002d538},
	{.symbol = _fillPixmapWithColor_string, .addr = 0x00029759},
	{.symbol = _finalizeBootStruct_string, .addr = 0x0002f824},
	{.symbol = _find_and_read_smbus_controller_string, .addr = 0x0003873a},
	{.symbol = _flipRB_string, .addr = 0x0002971e},
	{.symbol = _font_console_string, .addr = 0x0004f2d0},
	{.symbol = _font_small_string, .addr = 0x0004f650},
	{.symbol = _force_enable_hpet_string, .addr = 0x0003bd64},
	{.symbol = _free_string, .addr = 0x0003dfa7},
	{.symbol = _freeFilteredBVChain_string, .addr = 0x0002dda8},
	{.symbol = _freqs_string, .addr = 0x0004b020},
	{.symbol = _gAppleBootPictRLE_string, .addr = 0x00047794},
	{.symbol = _gBIOSBootVolume_string, .addr = 0x0004afac},
	{.symbol = _gBIOSDev_string, .addr = 0x0004f9d0},
	{.symbol = _gBinaryAddress_string, .addr = 0x0005084c},
	{.symbol = _gBootFileType_string, .addr = 0x0004f9d4},
	{.symbol = _gBootFileType_t_string, .addr = 0x0004f9d8},
	{.symbol = _gBootMode_string, .addr = 0x0004f9dc},
	{.symbol = _gBootVolume_string, .addr = 0x0004f9e0},
	{.symbol = _gCompareTable_string, .addr = 0x00050870},
	{.symbol = _gCompareTableCompressed_string, .addr = 0x0004d40c},
	{.symbol = _gDeviceCount_string, .addr = 0x00049588},
	{.symbol = _gEfiAcpi20TableGuid_string, .addr = 0x0004b044},
	{.symbol = _gEfiAcpiTableGuid_string, .addr = 0x0004b034},
	{.symbol = _gEfiConfigurationTableNode_string, .addr = 0x0004b030},
	{.symbol = _gEfiSmbiosTableGuid_string, .addr = 0x00047ea8},
	{.symbol = _gEnableCDROMRescan_string, .addr = 0x0004f9e4},
	{.symbol = _gErrors_string, .addr = 0x0004f9e5},
	{.symbol = _gFSLoadAddress_string, .addr = 0x0004afa8},
	{.symbol = _gHaveKernelCache_string, .addr = 0x0004f9e6},
	{.symbol = _gLowerCaseTable_string, .addr = 0x00050874},
	{.symbol = _gLowerCaseTableCompressed_string, .addr = 0x0004d134},
	{.symbol = _gMI_string, .addr = 0x00050038},
	{.symbol = _gMKextName_string, .addr = 0x0004f9f0},
	{.symbol = _gMacOSVersion_string, .addr = 0x0004fbf0},
	{.symbol = _gMemoryMapNode_string, .addr = 0x00050850},
	{.symbol = _gOverrideKernel_string, .addr = 0x0004fbf8},
	{.symbol = _gPlatformName_string, .addr = 0x00049580},
	{.symbol = _gRAMDiskBTAliased_string, .addr = 0x0004aac4},
	{.symbol = _gRAMDiskFile_string, .addr = 0x00050040},
	{.symbol = _gRAMDiskMI_string, .addr = 0x0004aabc},
	{.symbol = _gRAMDiskVolume_string, .addr = 0x0004aac0},
	{.symbol = _gRootDevice_string, .addr = 0x0004fc00},
	{.symbol = _gST_string, .addr = 0x0004b02c},
	{.symbol = _gScanSingleDrive_string, .addr = 0x0004fe00},
	{.symbol = _gVerboseMode_string, .addr = 0x0004fe01},
	{.symbol = _generateCRTCTiming_string, .addr = 0x00032e5a},
	{.symbol = _getBVChainForBIOSDev_string, .addr = 0x0002db09},
	{.symbol = _getBoolForKey_string, .addr = 0x00030221},
	{.symbol = _getBootOptions_string, .addr = 0x0002478d},
	{.symbol = _getBootVolumeDescription_string, .addr = 0x0002dc42},
	{.symbol = _getBootVolumeRef_string, .addr = 0x0002ce7b},
	{.symbol = _getColorForKey_string, .addr = 0x00030062},
	{.symbol = _getConventionalMemorySize_string, .addr = 0x00032030},
	{.symbol = _getCroppedPixmapAtPosition_string, .addr = 0x0002a1d2},
	{.symbol = _getCursorPositionAndType_string, .addr = 0x00031c3e},
	{.symbol = _getDDRPartNum_string, .addr = 0x000386e4},
	{.symbol = _getDDRSerial_string, .addr = 0x000387a2},
	{.symbol = _getDDRspeedMhz_string, .addr = 0x0003868e},
	{.symbol = _getDMIString_string, .addr = 0x00038024},
	{.symbol = _getDimensionForKey_string, .addr = 0x000300ab},
	{.symbol = _getEDID_string, .addr = 0x00032df2},
	{.symbol = _getExtendedMemorySize_string, .addr = 0x00032050},
	{.symbol = _getGraphicModeParams_string, .addr = 0x00021b41},
	{.symbol = _getIntForKey_string, .addr = 0x00030190},
	{.symbol = _getMemoryInfoString_string, .addr = 0x0002424f},
	{.symbol = _getMemoryMap_string, .addr = 0x00032313},
	{.symbol = _getMode_string, .addr = 0x00030956},
	{.symbol = _getNextArg_string, .addr = 0x0002fba6},
	{.symbol = _getPciRootUID_string, .addr = 0x0003b72b},
	{.symbol = _getPlatformName_string, .addr = 0x0002f7a6},
	{.symbol = _getResolution_string, .addr = 0x000344cd},
	{.symbol = _getSmbios_string, .addr = 0x00038ee3},
	{.symbol = _getStringForKey_string, .addr = 0x0003026e},
	{.symbol = _getStringFromUUID_string, .addr = 0x0003a249},
	{.symbol = _getUUIDFromString_string, .addr = 0x0003a2c0},
	{.symbol = _getVBECurrentMode_string, .addr = 0x00032bbf},
	{.symbol = _getVBEDACFormat_string, .addr = 0x00032d2d},
	{.symbol = _getVBEInfo_string, .addr = 0x00032db1},
	{.symbol = _getVBEInfoString_string, .addr = 0x000222a0},
	{.symbol = _getVBEModeInfo_string, .addr = 0x00032d67},
	{.symbol = _getVBEModeInfoString_string, .addr = 0x00022136},
	{.symbol = _getVBEPalette_string, .addr = 0x00032bf3},
	{.symbol = _getVBEPixelClock_string, .addr = 0x00032b71},
	{.symbol = _getValueForBootKey_string, .addr = 0x0002fe7a},
	{.symbol = _getValueForConfigTableKey_string, .addr = 0x0002ff23},
	{.symbol = _getValueForKey_string, .addr = 0x0002ff80},
	{.symbol = _getVendorName_string, .addr = 0x00038828},
	{.symbol = _getVideoMode_string, .addr = 0x000217b6},
	{.symbol = _get_chipset_string, .addr = 0x000304d8},
	{.symbol = _get_chipset_id_string, .addr = 0x000304c5},
	{.symbol = _get_drive_info_string, .addr = 0x000321d6},
	{.symbol = _get_pci_dev_path_string, .addr = 0x000339fc},
	{.symbol = _getc_string, .addr = 0x0003041b},
	{.symbol = _getchar_string, .addr = 0x0003046e},
	{.symbol = _getvramsizekb_string, .addr = 0x0003c2b3},
	{.symbol = _gprintf_string, .addr = 0x0002a8ea},
	{.symbol = _gui_string, .addr = 0x0004fe10},
	{.symbol = _halt_string, .addr = 0x00020383},
	{.symbol = _hex2bin_string, .addr = 0x0003d485},
	{.symbol = _hi_multiboot_string, .addr = 0x00025a6b},
	{.symbol = _imageCnt_string, .addr = 0x0004aee8},
	{.symbol = _images_string, .addr = 0x0004aad4},
	{.symbol = _infoMenuItems_string, .addr = 0x0004aef8},
	{.symbol = _initFont_string, .addr = 0x00029f3e},
	{.symbol = _initGUI_string, .addr = 0x0002b4ac},
	{.symbol = _initGraphicsMode_string, .addr = 0x00021d26},
	{.symbol = _initKernBootStruct_string, .addr = 0x0002f9a8},
	{.symbol = _initialize_runtime_string, .addr = 0x000210ab},
	{.symbol = _is_no_emulation_string, .addr = 0x00032157},
	{.symbol = _jump_to_chainbooter_string, .addr = 0x000202ca},
	{.symbol = _kernelSymbolAddresses_string, .addr = 0x0004af8c},
	{.symbol = _kernelSymbols_string, .addr = 0x0004af78},
	{.symbol = _lasttime_string, .addr = 0x0004aad0},
	{.symbol = _legacy_off_string, .addr = 0x00032858},
	{.symbol = _loadACPITable_string, .addr = 0x0003a791},
	{.symbol = _loadConfigFile_string, .addr = 0x0002fe2c},
	{.symbol = _loadEmbeddedPngImage_string, .addr = 0x00021f1f},
	{.symbol = _loadHelperConfig_string, .addr = 0x0002fccf},
	{.symbol = _loadImageScale_string, .addr = 0x000214a1},
	{.symbol = _loadOverrideConfig_string, .addr = 0x0002fd41},
	{.symbol = _loadPngImage_string, .addr = 0x00021fc2},
	{.symbol = _loadPrebootRAMDisk_string, .addr = 0x00026234},
	{.symbol = _loadSystemConfig_string, .addr = 0x0002fdb3},
	{.symbol = _loadThemeValues_string, .addr = 0x0002ac43},
	{.symbol = _loader_string, .addr = 0x000203dd},
	{.symbol = _locate_symbols_string, .addr = 0x0002ca0e},
	{.symbol = _lookUpCLUTIndex_string, .addr = 0x00021415},
	{.symbol = _lspci_string, .addr = 0x00024733},
	{.symbol = _makeRoundedCorners_string, .addr = 0x00029878},
	{.symbol = _malloc_init_string, .addr = 0x0003de41},
	{.symbol = _map_type1_resolution_string, .addr = 0x0003067d},
	{.symbol = _map_type2_resolution_string, .addr = 0x0003068c},
	{.symbol = _map_type3_resolution_string, .addr = 0x0003069b},
	{.symbol = _mapping_string, .addr = 0x0004cbe0},
	{.symbol = _md0Ramdisk_string, .addr = 0x0002625d},
	{.symbol = _memcmp_string, .addr = 0x0003dbd5},
	{.symbol = _memcpy_string, .addr = 0x0003db60},
	{.symbol = _memset_string, .addr = 0x0003db4c},
	{.symbol = _menuBVR_string, .addr = 0x00050024},
	{.symbol = _menuItems_string, .addr = 0x0004aaa4},
	{.symbol = _mountRAMDisk_string, .addr = 0x00025f48},
	{.symbol = _multibootRamdiskReadBytes_string, .addr = 0x00025860},
	{.symbol = _multiboot_get_ramdisk_info_string, .addr = 0x000257f9},
	{.symbol = _multiboot_partition_string, .addr = 0x0004aab4},
	{.symbol = _multiboot_partition_set_string, .addr = 0x0004aab8},
	{.symbol = _multiboot_timeout_string, .addr = 0x0004aaac},
	{.symbol = _multiboot_timeout_set_string, .addr = 0x0004aab0},
	{.symbol = _multiboot_to_boot_string, .addr = 0x00025dc9},
	{.symbol = _nbpScanBootVolumes_string, .addr = 0x000304b7},
	{.symbol = _nbpUnloadBaseCode_string, .addr = 0x000304be},
	{.symbol = _newAPMBVRef_string, .addr = 0x0002dfdf},
	{.symbol = _newFilteredBVChain_string, .addr = 0x0002de06},
	{.symbol = _newGPTBVRef_string, .addr = 0x0002e85e},
	{.symbol = _newString_string, .addr = 0x0002fc0f},
	{.symbol = _newStringForKey_string, .addr = 0x000302a6},
	{.symbol = _notify_usb_dev_string, .addr = 0x00032a17},
	{.symbol = _nvidia_compatible_0_string, .addr = 0x0004db68},
	{.symbol = _nvidia_compatible_1_string, .addr = 0x0004db70},
	{.symbol = _nvidia_device_type_string, .addr = 0x0004db88},
	{.symbol = _nvidia_device_type_0_string, .addr = 0x0004db78},
	{.symbol = _nvidia_device_type_1_string, .addr = 0x0004db80},
	{.symbol = _nvidia_name_0_string, .addr = 0x0004db90},
	{.symbol = _nvidia_name_1_string, .addr = 0x0004db98},
	{.symbol = _nvidia_slot_name_string, .addr = 0x0004dba0},
	{.symbol = _open_string, .addr = 0x0002d211},
	{.symbol = _open_bvdev_string, .addr = 0x0002d5bd},
	{.symbol = _open_vbios_string, .addr = 0x00030e0c},
	{.symbol = _opendir_string, .addr = 0x0002d0a3},
	{.symbol = _p_get_ramdisk_info_string, .addr = 0x0004b00c},
	{.symbol = _p_ramdiskReadBytes_string, .addr = 0x0004b008},
	{.symbol = _patchVideoBios_string, .addr = 0x00031254},
	{.symbol = _patch_commpage_stuff_routine_string, .addr = 0x0002c570},
	{.symbol = _patch_cpuid_set_info_string, .addr = 0x0002c71e},
	{.symbol = _patch_fadt_string, .addr = 0x0003a3e9},
	{.symbol = _patch_kernel_string, .addr = 0x0002cb83},
	{.symbol = _patch_kernel_32_string, .addr = 0x0002c99b},
	{.symbol = _patch_kernel_64_string, .addr = 0x0002c9fe},
	{.symbol = _patch_lapic_init_string, .addr = 0x0002c616},
	{.symbol = _patch_pmCPUExitHaltToOff_string, .addr = 0x0002c6c7},
	{.symbol = _pause_string, .addr = 0x000304a3},
	{.symbol = _pci_config_read16_string, .addr = 0x0003395b},
	{.symbol = _pci_config_read32_string, .addr = 0x00033986},
	{.symbol = _pci_config_read8_string, .addr = 0x00033931},
	{.symbol = _pci_config_write16_string, .addr = 0x0003399e},
	{.symbol = _pci_config_write32_string, .addr = 0x00033c39},
	{.symbol = _pci_config_write8_string, .addr = 0x00033c0b},
	{.symbol = _platformCPUFeature_string, .addr = 0x0003138d},
	{.symbol = _png_alloc_add_node_string, .addr = 0x0002688d},
	{.symbol = _png_alloc_find_node_string, .addr = 0x000263e9},
	{.symbol = _png_alloc_free_string, .addr = 0x000267ff},
	{.symbol = _png_alloc_free_all_string, .addr = 0x000267da},
	{.symbol = _png_alloc_head_string, .addr = 0x0004aac8},
	{.symbol = _png_alloc_malloc_string, .addr = 0x000268f9},
	{.symbol = _png_alloc_realloc_string, .addr = 0x00026932},
	{.symbol = _png_alloc_remove_node_string, .addr = 0x00026781},
	{.symbol = _png_alloc_tail_string, .addr = 0x0004aacc},
	{.symbol = _pos_string, .addr = 0x00029705},
	{.symbol = _previewLoadedSectors_string, .addr = 0x00049594},
	{.symbol = _previewSaveunder_string, .addr = 0x00049598},
	{.symbol = _previewTotalSectors_string, .addr = 0x00049590},
	{.symbol = _prf_string, .addr = 0x0003e854},
	{.symbol = _printVBEModeInfo_string, .addr = 0x000223b2},
	{.symbol = _printf_string, .addr = 0x00030387},
	{.symbol = _processBootArgument_string, .addr = 0x00023be1},
	{.symbol = _processBootOptions_string, .addr = 0x00023c70},
	{.symbol = _processRAMDiskCommand_string, .addr = 0x000260e4},
	{.symbol = _prompt_string, .addr = 0x00050250},
	{.symbol = _promptForRescanOption_string, .addr = 0x000237a2},
	{.symbol = _prompt_pos_string, .addr = 0x0004aeec},
	{.symbol = _prompt_text_string, .addr = 0x0004aef0},
	{.symbol = _ptol_string, .addr = 0x0003dcb7},
	{.symbol = _putc_string, .addr = 0x00031d5c},
	{.symbol = _putca_string, .addr = 0x00031d1c},
	{.symbol = _putchar_string, .addr = 0x00030432},
	{.symbol = _rawDiskRead_string, .addr = 0x0002e1b5},
	{.symbol = _rawDiskWrite_string, .addr = 0x0002e0af},
	{.symbol = _read_string, .addr = 0x0002d55e},
	{.symbol = _readBootSector_string, .addr = 0x0002e7f5},
	{.symbol = _readEDID_string, .addr = 0x0003438c},
	{.symbol = _readKeyboardShiftFlags_string, .addr = 0x000320fd},
	{.symbol = _readKeyboardStatus_string, .addr = 0x00032124},
	{.symbol = _readdir_string, .addr = 0x0002cc25},
	{.symbol = _readdir_ext_string, .addr = 0x0002cc47},
	{.symbol = _realloc_string, .addr = 0x0003e2d8},
	{.symbol = _recoveryMode_string, .addr = 0x0004958c},
	{.symbol = _relock_vbios_string, .addr = 0x0003078b},
	{.symbol = _rescanBIOSDevice_string, .addr = 0x0002dfb2},
	{.symbol = _reserveKernBootStruct_string, .addr = 0x0002f97b},
	{.symbol = _root_pci_dev_string, .addr = 0x00050034},
	{.symbol = _safe_malloc_string, .addr = 0x0003e198},
	{.symbol = _scanBootVolumes_string, .addr = 0x0002cdff},
	{.symbol = _scanDisks_string, .addr = 0x0002d00e},
	{.symbol = _scan_cpu_string, .addr = 0x00037ace},
	{.symbol = _scan_mem_string, .addr = 0x000313c9},
	{.symbol = _scan_memory_string, .addr = 0x00038085},
	{.symbol = _scan_pci_bus_string, .addr = 0x00033af1},
	{.symbol = _scan_platform_string, .addr = 0x000313a1},
	{.symbol = _scan_spd_string, .addr = 0x00038791},
	{.symbol = _scollPage_string, .addr = 0x00031bc2},
	{.symbol = _search_and_get_acpi_fd_string, .addr = 0x0003a5c6},
	{.symbol = _selectAlternateBootDevice_string, .addr = 0x000237c0},
	{.symbol = _selectBootVolume_string, .addr = 0x0002cca7},
	{.symbol = _selectIndex_string, .addr = 0x0004aaa0},
	{.symbol = _setActiveDisplayPage_string, .addr = 0x00031b99},
	{.symbol = _setBootGlobals_string, .addr = 0x0002d6ff},
	{.symbol = _setCursorPosition_string, .addr = 0x00031cad},
	{.symbol = _setCursorType_string, .addr = 0x00031c83},
	{.symbol = _setRAMDiskBTHook_string, .addr = 0x00025e09},
	{.symbol = _setRootVolume_string, .addr = 0x0002cc87},
	{.symbol = _setVBEDACFormat_string, .addr = 0x00032cf5},
	{.symbol = _setVBEMode_string, .addr = 0x00032ca7},
	{.symbol = _setVBEPalette_string, .addr = 0x00032c4d},
	{.symbol = _setVideoMode_string, .addr = 0x00021dd9},
	{.symbol = _set_eth_builtin_string, .addr = 0x0003b4b9},
	{.symbol = _set_mode_string, .addr = 0x00030a4d},
	{.symbol = _setupAcpi_string, .addr = 0x0003a8ea},
	{.symbol = _setupAcpiNoMod_string, .addr = 0x0003a83f},
	{.symbol = _setupDeviceProperties_string, .addr = 0x0003b580},
	{.symbol = _setupEfiDeviceTree_string, .addr = 0x000314d0},
	{.symbol = _setupEfiTables_string, .addr = 0x00031746},
	{.symbol = _setupFakeEfi_string, .addr = 0x00031a27},
	{.symbol = _setupSystemType_string, .addr = 0x0003140d},
	{.symbol = _setup_ati_devprop_string, .addr = 0x0003c44e},
	{.symbol = _setup_nvidia_devprop_string, .addr = 0x0003d54c},
	{.symbol = _setup_pci_devs_string, .addr = 0x0003b5f7},
	{.symbol = _showHelp_string, .addr = 0x00023ad3},
	{.symbol = _showInfoBox_string, .addr = 0x0002bae8},
	{.symbol = _showInfoRAMDisk_string, .addr = 0x00025e2e},
	{.symbol = _showTextFile_string, .addr = 0x00023a44},
	{.symbol = _sleep_string, .addr = 0x000324fc},
	{.symbol = _slvprintf_string, .addr = 0x0003e332},
	{.symbol = _smb_read_byte_intel_string, .addr = 0x000385e8},
	{.symbol = _smbios_p_string, .addr = 0x00050868},
	{.symbol = _smbios_properties_string, .addr = 0x0004cc10},
	{.symbol = _smbios_table_descriptions_string, .addr = 0x0004ce78},
	{.symbol = _spinActivityIndicator_string, .addr = 0x00022528},
	{.symbol = _sprintf_string, .addr = 0x0003e36c},
	{.symbol = _start_string, .addr = 0x0002cbb2},
	{.symbol = _startprog_string, .addr = 0x0002038d},
	{.symbol = _stop_string, .addr = 0x000303c4},
	{.symbol = _stosl_string, .addr = 0x0002148d},
	{.symbol = _strcat_string, .addr = 0x0003dd8e},
	{.symbol = _strcmp_string, .addr = 0x0003dbfc},
	{.symbol = _strcpy_string, .addr = 0x0003dc51},
	{.symbol = _strdup_string, .addr = 0x0003dd54},
	{.symbol = _string_string, .addr = 0x0004d110},
	{.symbol = _stringLength_string, .addr = 0x0002fb20},
	{.symbol = _stringdata_string, .addr = 0x0004d114},
	{.symbol = _stringlength_string, .addr = 0x0004d118},
	{.symbol = _strlcpy_string, .addr = 0x0003dc8d},
	{.symbol = _strlen_string, .addr = 0x0003dbc2},
	{.symbol = _strncat_string, .addr = 0x0003dd0a},
	{.symbol = _strncmp_string, .addr = 0x0003dc1d},
	{.symbol = _strncpy_string, .addr = 0x0003dc69},
	{.symbol = _strstr_string, .addr = 0x0003ddc8},
	{.symbol = _strtol_string, .addr = 0x0003e3a8},
	{.symbol = _strtoul_string, .addr = 0x0003e6e5},
	{.symbol = _strtouq_string, .addr = 0x0003e519},
	{.symbol = _sysConfigValid_string, .addr = 0x00050028},
	{.symbol = _systemConfigDir_string, .addr = 0x0002cc6b},
	{.symbol = _tell_string, .addr = 0x0002cbff},
	{.symbol = _testBiosread_string, .addr = 0x0002f794},
	{.symbol = _testFAT32EFIBootSector_string, .addr = 0x0002e771},
	{.symbol = _textAddress_string, .addr = 0x0004afa4},
	{.symbol = _textSection_string, .addr = 0x0004afa0},
	{.symbol = _time18_string, .addr = 0x000320c3},
	{.symbol = _uhci_reset_string, .addr = 0x00032579},
	{.symbol = _umountRAMDisk_string, .addr = 0x00025ec0},
	{.symbol = _unlock_vbios_string, .addr = 0x00030713},
	{.symbol = _updateGraphicBootPrompt_string, .addr = 0x0002c0ee},
	{.symbol = _updateInfoMenu_string, .addr = 0x0002be37},
	{.symbol = _updateProgressBar_string, .addr = 0x000215e3},
	{.symbol = _updateVRAM_string, .addr = 0x0002b870},
	{.symbol = _usbList_string, .addr = 0x0004b054},
	{.symbol = _usb_loop_string, .addr = 0x0003291c},
	{.symbol = _useGUI_string, .addr = 0x00050029},
	{.symbol = _utf_decodestr_string, .addr = 0x0003bbdc},
	{.symbol = _utf_encodestr_string, .addr = 0x0003bcb7},
	{.symbol = _vector32_cleanup_string, .addr = 0x00026860},
	{.symbol = _vector32_init_string, .addr = 0x00026404},
	{.symbol = _vector32_new_string, .addr = 0x00026fa7},
	{.symbol = _vector32_resize_string, .addr = 0x000269da},
	{.symbol = _vector32_resizev_string, .addr = 0x00026f3d},
	{.symbol = _vector8_cleanup_string, .addr = 0x00026833},
	{.symbol = _vector8_copy_string, .addr = 0x000281a2},
	{.symbol = _vector8_init_string, .addr = 0x00026420},
	{.symbol = _vector8_new_string, .addr = 0x000280f0},
	{.symbol = _vector8_resize_string, .addr = 0x0002698d},
	{.symbol = _vector8_resizev_string, .addr = 0x00027681},
	{.symbol = _vendorMap_string, .addr = 0x0004b098},
	{.symbol = _verbose_string, .addr = 0x00030341},
	{.symbol = _video_mode_string, .addr = 0x00032007},
	{.symbol = _vol_opendir_string, .addr = 0x0002d101},
	{.symbol = _vprf_string, .addr = 0x0002bf37},
	{.symbol = _vramwrite_string, .addr = 0x0002a533},
	{.symbol = _waitThenReload_string, .addr = 0x000259ed},
	{.symbol = boot2_string, .addr = 0x00020200},
};
