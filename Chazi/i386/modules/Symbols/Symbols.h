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
static char _GMAX3100_vals_string[] = "_GMAX3100_vals";
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
static char _NTFSGetUUID_string[] = "_NTFSGetUUID";
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
static char _XMLDecode_string[] = "_XMLDecode";
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
static char _acpi_cpu_count_string[] = "_acpi_cpu_count";
static char _acpi_cpu_name_string[] = "_acpi_cpu_name";
static char _addConfigurationTable_string[] = "_addConfigurationTable";
static char _add_symbol_string[] = "_add_symbol";
static char _aml_add_alias_string[] = "_aml_add_alias";
static char _aml_add_buffer_string[] = "_aml_add_buffer";
static char _aml_add_byte_string[] = "_aml_add_byte";
static char _aml_add_dword_string[] = "_aml_add_dword";
static char _aml_add_name_string[] = "_aml_add_name";
static char _aml_add_package_string[] = "_aml_add_package";
static char _aml_add_qword_string[] = "_aml_add_qword";
static char _aml_add_scope_string[] = "_aml_add_scope";
static char _aml_add_to_parent_string[] = "_aml_add_to_parent";
static char _aml_add_word_string[] = "_aml_add_word";
static char _aml_calculate_size_string[] = "_aml_calculate_size";
static char _aml_create_node_string[] = "_aml_create_node";
static char _aml_destroy_node_string[] = "_aml_destroy_node";
static char _aml_fill_name_string[] = "_aml_fill_name";
static char _aml_fill_simple_name_string[] = "_aml_fill_simple_name";
static char _aml_get_size_length_string[] = "_aml_get_size_length";
static char _aml_write_buffer_string[] = "_aml_write_buffer";
static char _aml_write_byte_string[] = "_aml_write_byte";
static char _aml_write_dword_string[] = "_aml_write_dword";
static char _aml_write_node_string[] = "_aml_write_node";
static char _aml_write_qword_string[] = "_aml_write_qword";
static char _aml_write_size_string[] = "_aml_write_size";
static char _aml_write_word_string[] = "_aml_write_word";
static char _animateProgressBar_string[] = "_animateProgressBar";
static char _archCpuType_string[] = "_archCpuType";
static char _ascii_hex_to_int_string[] = "_ascii_hex_to_int";
static char _atiSetMode_1_string[] = "_atiSetMode_1";
static char _atiSetMode_2_string[] = "_atiSetMode_2";
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
static char _bind_macho_string[] = "_bind_macho";
static char _bios_string[] = "_bios";
static char _biosDevIsCDROM_string[] = "_biosDevIsCDROM";
static char _biosTypeNames_string[] = "_biosTypeNames";
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
static char _bpResolution_string[] = "_bpResolution";
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
static char _chipsetTypeNames_string[] = "_chipsetTypeNames";
static char _clearActivityIndicator_string[] = "_clearActivityIndicator";
static char _clearGraphicBootPrompt_string[] = "_clearGraphicBootPrompt";
static char _clearScreenRows_string[] = "_clearScreenRows";
static char _close_string[] = "_close";
static char _closeVbios_string[] = "_closeVbios";
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
static char _cursor_string[] = "_cursor";
static char _decodeRLE_string[] = "_decodeRLE";
static char _decompress_lzss_string[] = "_decompress_lzss";
static char _delay_string[] = "_delay";
static char _detectAtiBiosType_string[] = "_detectAtiBiosType";
static char _detectBiosType_string[] = "_detectBiosType";
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
static char _dram_controller_dev_string[] = "_dram_controller_dev";
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
static char _execute_hook_string[] = "_execute_hook";
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
static char _freeWindowBuffer_string[] = "_freeWindowBuffer";
static char _freqs_string[] = "_freqs";
static char _gAppleBootPictRLE_string[] = "_gAppleBootPictRLE";
static char _gAutoResolution_string[] = "_gAutoResolution";
static char _gBIOSBootVolume_string[] = "_gBIOSBootVolume";
static char _gBIOSDev_string[] = "_gBIOSDev";
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
static char _gRAMDiskBTAliased_string[] = "_gRAMDiskBTAliased";
static char _gRAMDiskFile_string[] = "_gRAMDiskFile";
static char _gRAMDiskMI_string[] = "_gRAMDiskMI";
static char _gRAMDiskVolume_string[] = "_gRAMDiskVolume";
static char _gRootDevice_string[] = "_gRootDevice";
static char _gST32_string[] = "_gST32";
static char _gST64_string[] = "_gST64";
static char _gScanSingleDrive_string[] = "_gScanSingleDrive";
static char _gVerboseMode_string[] = "_gVerboseMode";
static char _generateCRTCTiming_string[] = "_generateCRTCTiming";
static char _generate_cst_ssdt_string[] = "_generate_cst_ssdt";
static char _generate_pss_ssdt_string[] = "_generate_pss_ssdt";
static char _getAspectRatio_string[] = "_getAspectRatio";
static char _getBVChainForBIOSDev_string[] = "_getBVChainForBIOSDev";
static char _getBoolForKey_string[] = "_getBoolForKey";
static char _getBootOptions_string[] = "_getBootOptions";
static char _getBootVolumeDescription_string[] = "_getBootVolumeDescription";
static char _getBootVolumeRef_string[] = "_getBootVolumeRef";
static char _getChipset_string[] = "_getChipset";
static char _getChipsetId_string[] = "_getChipsetId";
static char _getColorForKey_string[] = "_getColorForKey";
static char _getConventionalMemorySize_string[] = "_getConventionalMemorySize";
static char _getCroppedPixmapAtPosition_string[] = "_getCroppedPixmapAtPosition";
static char _getCursorPositionAndType_string[] = "_getCursorPositionAndType";
static char _getDDRPartNum_string[] = "_getDDRPartNum";
static char _getDDRSerial_string[] = "_getDDRSerial";
static char _getDDRspeedMhz_string[] = "_getDDRspeedMhz";
static char _getDMIString_string[] = "_getDMIString";
static char _getDeviceDescription_string[] = "_getDeviceDescription";
static char _getDimensionForKey_string[] = "_getDimensionForKey";
static char _getEDID_string[] = "_getEDID";
static char _getExtendedMemorySize_string[] = "_getExtendedMemorySize";
static char _getGraphicModeParams_string[] = "_getGraphicModeParams";
static char _getIntForKey_string[] = "_getIntForKey";
static char _getMemoryInfoString_string[] = "_getMemoryInfoString";
static char _getMemoryMap_string[] = "_getMemoryMap";
static char _getNextArg_string[] = "_getNextArg";
static char _getNumberArrayFromProperty_string[] = "_getNumberArrayFromProperty";
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
static char _getVESAModeWithProperties_string[] = "_getVESAModeWithProperties";
static char _getValueForBootKey_string[] = "_getValueForBootKey";
static char _getValueForConfigTableKey_string[] = "_getValueForConfigTableKey";
static char _getValueForKey_string[] = "_getValueForKey";
static char _getVendorName_string[] = "_getVendorName";
static char _getVideoMode_string[] = "_getVideoMode";
static char _getVolumeLabelAlias_string[] = "_getVolumeLabelAlias";
static char _get_acpi_cpu_names_string[] = "_get_acpi_cpu_names";
static char _get_drive_info_string[] = "_get_drive_info";
static char _get_gma_model_string[] = "_get_gma_model";
static char _get_pci_dev_path_string[] = "_get_pci_dev_path";
static char _getc_string[] = "_getc";
static char _getchar_string[] = "_getchar";
static char _getvramsizekb_string[] = "_getvramsizekb";
static char _gprintf_string[] = "_gprintf";
static char _gtfTimings_string[] = "_gtfTimings";
static char _gui_string[] = "_gui";
static char _halt_string[] = "_halt";
static char _handle_symtable_string[] = "_handle_symtable";
static char _hex2bin_string[] = "_hex2bin";
static char _hi_multiboot_string[] = "_hi_multiboot";
static char _imageCnt_string[] = "_imageCnt";
static char _images_string[] = "_images";
static char _infoMenuItems_string[] = "_infoMenuItems";
static char _initBooterLog_string[] = "_initBooterLog";
static char _initFont_string[] = "_initFont";
static char _initGUI_string[] = "_initGUI";
static char _initGraphicsMode_string[] = "_initGraphicsMode";
static char _initKernBootStruct_string[] = "_initKernBootStruct";
static char _init_module_system_string[] = "_init_module_system";
static char _initialize_runtime_string[] = "_initialize_runtime";
static char _intelSetMode_1_string[] = "_intelSetMode_1";
static char _intelSetMode_2_string[] = "_intelSetMode_2";
static char _intelSetMode_3_string[] = "_intelSetMode_3";
static char _intializeTables_string[] = "_intializeTables";
static char _is_module_laoded_string[] = "_is_module_laoded";
static char _is_no_emulation_string[] = "_is_no_emulation";
static char _jump_to_chainbooter_string[] = "_jump_to_chainbooter";
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
static char _load_all_modules_string[] = "_load_all_modules";
static char _load_module_string[] = "_load_module";
static char _loadedModules_string[] = "_loadedModules";
static char _loader_string[] = "_loader";
static char _lookUpCLUTIndex_string[] = "_lookUpCLUTIndex";
static char _lookup_all_symbols_string[] = "_lookup_all_symbols";
static char _lookup_symbol_string[] = "_lookup_symbol";
static char _lspci_string[] = "_lspci";
static char _makeRoundedCorners_string[] = "_makeRoundedCorners";
static char _malloc_init_string[] = "_malloc_init";
static char _map_string[] = "_map";
static char _mapType1Resolution_string[] = "_mapType1Resolution";
static char _mapType2Resolution_string[] = "_mapType2Resolution";
static char _mapType3Resolution_string[] = "_mapType3Resolution";
static char _mapping_string[] = "_mapping";
static char _matchVolumeToString_string[] = "_matchVolumeToString";
static char _md0Ramdisk_string[] = "_md0Ramdisk";
static char _mem_detect_string[] = "_mem_detect";
static char _memcmp_string[] = "_memcmp";
static char _memcpy_string[] = "_memcpy";
static char _memset_string[] = "_memset";
static char _menuBVR_string[] = "_menuBVR";
static char _menuItems_string[] = "_menuItems";
static char _moduleCallbacks_string[] = "_moduleCallbacks";
static char _moduleSymbols_string[] = "_moduleSymbols";
static char _module_loaded_string[] = "_module_loaded";
static char _mountRAMDisk_string[] = "_mountRAMDisk";
static char _msgbuf_string[] = "_msgbuf";
static char _msglog_string[] = "_msglog";
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
static char _nhm_bus_string[] = "_nhm_bus";
static char _notify_usb_dev_string[] = "_notify_usb_dev";
static char _nvidiaSetMode_string[] = "_nvidiaSetMode";
static char _nvidia_compatible_0_string[] = "_nvidia_compatible_0";
static char _nvidia_compatible_1_string[] = "_nvidia_compatible_1";
static char _nvidia_device_type_string[] = "_nvidia_device_type";
static char _nvidia_device_type_0_string[] = "_nvidia_device_type_0";
static char _nvidia_device_type_1_string[] = "_nvidia_device_type_1";
static char _nvidia_name_0_string[] = "_nvidia_name_0";
static char _nvidia_name_1_string[] = "_nvidia_name_1";
static char _nvidia_slot_name_string[] = "_nvidia_slot_name";
static char _open_string[] = "_open";
static char _openAtiVbios_string[] = "_openAtiVbios";
static char _openIntelVbios_string[] = "_openIntelVbios";
static char _openNvidiaVbios_string[] = "_openNvidiaVbios";
static char _openVbios_string[] = "_openVbios";
static char _open_bvdev_string[] = "_open_bvdev";
static char _opendir_string[] = "_opendir";
static char _openmem_string[] = "_openmem";
static char _p_get_ramdisk_info_string[] = "_p_get_ramdisk_info";
static char _p_ramdiskReadBytes_string[] = "_p_ramdiskReadBytes";
static char _parse_mach_string[] = "_parse_mach";
static char _patchVbios_string[] = "_patchVbios";
static char _patch_fadt_string[] = "_patch_fadt";
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
static char _rebase_location_string[] = "_rebase_location";
static char _rebase_macho_string[] = "_rebase_macho";
static char _reg_FALSE_string[] = "_reg_FALSE";
static char _reg_TRUE_string[] = "_reg_TRUE";
static char _register_hook_string[] = "_register_hook";
static char _register_hook_callback_string[] = "_register_hook_callback";
static char _relockVbios_string[] = "_relockVbios";
static char _replace_function_string[] = "_replace_function";
static char _rescanBIOSDevice_string[] = "_rescanBIOSDevice";
static char _reserveKernBootStruct_string[] = "_reserveKernBootStruct";
static char _restoreTables_string[] = "_restoreTables";
static char _restoreVbios_string[] = "_restoreVbios";
static char _root_pci_dev_string[] = "_root_pci_dev";
static char _safe_malloc_string[] = "_safe_malloc";
static char _saveTables_string[] = "_saveTables";
static char _saveVbios_string[] = "_saveVbios";
static char _scanBootVolumes_string[] = "_scanBootVolumes";
static char _scanDisks_string[] = "_scanDisks";
static char _scan_cpu_string[] = "_scan_cpu";
static char _scan_dram_controller_string[] = "_scan_dram_controller";
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
static char _setupAcpi_string[] = "_setupAcpi";
static char _setupBooterLog_string[] = "_setupBooterLog";
static char _setupDeviceList_string[] = "_setupDeviceList";
static char _setupDeviceProperties_string[] = "_setupDeviceProperties";
static char _setupEfiDeviceTree_string[] = "_setupEfiDeviceTree";
static char _setupEfiTables32_string[] = "_setupEfiTables32";
static char _setupEfiTables64_string[] = "_setupEfiTables64";
static char _setupFakeEfi_string[] = "_setupFakeEfi";
static char _setupSystemType_string[] = "_setupSystemType";
static char _setup_ati_devprop_string[] = "_setup_ati_devprop";
static char _setup_gma_devprop_string[] = "_setup_gma_devprop";
static char _setup_nvidia_devprop_string[] = "_setup_nvidia_devprop";
static char _setup_pci_devs_string[] = "_setup_pci_devs";
static char _showBootBanner_string[] = "_showBootBanner";
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
static char _spd_indexes_string[] = "_spd_indexes";
static char _spinActivityIndicator_string[] = "_spinActivityIndicator";
static char _sprintf_string[] = "_sprintf";
static char _startprog_string[] = "_startprog";
static char _stop_string[] = "_stop";
static char _stosl_string[] = "_stosl";
static char _strbreak_string[] = "_strbreak";
static char _strcat_string[] = "_strcat";
static char _strchr_string[] = "_strchr";
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
static char _tableSign_string[] = "_tableSign";
static char _tell_string[] = "_tell";
static char _testBiosread_string[] = "_testBiosread";
static char _testFAT32EFIBootSector_string[] = "_testFAT32EFIBootSector";
static char _time18_string[] = "_time18";
static char _uhci_reset_string[] = "_uhci_reset";
static char _umountRAMDisk_string[] = "_umountRAMDisk";
static char _unlockVbios_string[] = "_unlockVbios";
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
static char _write_string[] = "_write";
static char _writebyte_string[] = "_writebyte";
static char _writeint_string[] = "_writeint";
static char _xResolution_string[] = "_xResolution";
static char _yResolution_string[] = "_yResolution";
static char boot2_string[] = "boot2";
symbol_t symbolList[] = {
	{.symbol = _AllocateKernelMemory_string, .addr = 0x00034890},
	{.symbol = _AllocateMemoryRange_string, .addr = 0x00034908},
	{.symbol = _BinaryUnicodeCompare_string, .addr = 0x0003f6e0},
	{.symbol = _BootHelp_txt_string, .addr = 0x0004fa34},
	{.symbol = _BootHelp_txt_len_string, .addr = 0x00051174},
	{.symbol = _CLCL_string, .addr = 0x0004d44c},
	{.symbol = _CacheInit_string, .addr = 0x00035590},
	{.symbol = _CacheRead_string, .addr = 0x0003544e},
	{.symbol = _CacheReset_string, .addr = 0x0003543f},
	{.symbol = _CreateUUIDString_string, .addr = 0x0002e1eb},
	{.symbol = _DISTBASE_string, .addr = 0x0004d35c},
	{.symbol = _DISTEXTRA_string, .addr = 0x0004d3d4},
	{.symbol = _DT__AddChild_string, .addr = 0x00035784},
	{.symbol = _DT__AddProperty_string, .addr = 0x000356bd},
	{.symbol = _DT__Finalize_string, .addr = 0x000359a0},
	{.symbol = _DT__FindNode_string, .addr = 0x00035a29},
	{.symbol = _DT__FlattenDeviceTree_string, .addr = 0x00035933},
	{.symbol = _DT__FreeNode_string, .addr = 0x00035674},
	{.symbol = _DT__FreeProperty_string, .addr = 0x0003565e},
	{.symbol = _DT__GetName_string, .addr = 0x0003568a},
	{.symbol = _DT__Initialize_string, .addr = 0x0003584c},
	{.symbol = _DecodeKernel_string, .addr = 0x00023762},
	{.symbol = _DecodeMachO_string, .addr = 0x0002e7f5},
	{.symbol = _DecompressData_string, .addr = 0x0002928a},
	{.symbol = _EX2GetDescription_string, .addr = 0x000363dc},
	{.symbol = _EX2Probe_string, .addr = 0x000363c3},
	{.symbol = _FastRelString_string, .addr = 0x0003f803},
	{.symbol = _FastUnicodeCompare_string, .addr = 0x0003f883},
	{.symbol = _FindFirstDmiTableOfType_string, .addr = 0x0003d13f},
	{.symbol = _FindNextDmiTableOfType_string, .addr = 0x0003cffa},
	{.symbol = _GMAX3100_vals_string, .addr = 0x00054314},
	{.symbol = _GPT_BASICDATA2_GUID_string, .addr = 0x0004db18},
	{.symbol = _GPT_BASICDATA_GUID_string, .addr = 0x0004db08},
	{.symbol = _GPT_BOOT_GUID_string, .addr = 0x0004dae8},
	{.symbol = _GPT_EFISYS_GUID_string, .addr = 0x0004daf8},
	{.symbol = _GPT_HFS_GUID_string, .addr = 0x0004dad8},
	{.symbol = _Gdt_string, .addr = 0x000204e8},
	{.symbol = _Gdtr_string, .addr = 0x00020520},
	{.symbol = _GetDirEntry_string, .addr = 0x0002defd},
	{.symbol = _GetFileBlock_string, .addr = 0x0002e0e5},
	{.symbol = _GetFileInfo_string, .addr = 0x0002e123},
	{.symbol = _HFSFree_string, .addr = 0x00036646},
	{.symbol = _HFSGetDescription_string, .addr = 0x00037979},
	{.symbol = _HFSGetDirEntry_string, .addr = 0x000376c2},
	{.symbol = _HFSGetFileBlock_string, .addr = 0x00037a22},
	{.symbol = _HFSGetUUID_string, .addr = 0x0003768d},
	{.symbol = _HFSInitPartition_string, .addr = 0x00037309},
	{.symbol = _HFSLoadFile_string, .addr = 0x0003792b},
	{.symbol = _HFSProbe_string, .addr = 0x00037949},
	{.symbol = _HFSReadFile_string, .addr = 0x000377aa},
	{.symbol = _HibernateBoot_string, .addr = 0x00028e58},
	{.symbol = _HuffmanTree_decode_string, .addr = 0x00026798},
	{.symbol = _HuffmanTree_makeFromLengths_string, .addr = 0x0002736f},
	{.symbol = _HuffmanTree_new_string, .addr = 0x00026c7c},
	{.symbol = _Idtr_prot_string, .addr = 0x00020530},
	{.symbol = _Idtr_real_string, .addr = 0x00020528},
	{.symbol = _Inflator_error_string, .addr = 0x00056af0},
	{.symbol = _Inflator_generateFixedTrees_string, .addr = 0x0002796f},
	{.symbol = _Inflator_getTreeInflateDynamic_string, .addr = 0x000274e4},
	{.symbol = _Inflator_huffmanDecodeSymbol_string, .addr = 0x00026d83},
	{.symbol = _Inflator_inflate_string, .addr = 0x00028285},
	{.symbol = _Inflator_inflateHuffmanBlock_string, .addr = 0x00027a44},
	{.symbol = _Inflator_inflateNoCompression_string, .addr = 0x00028174},
	{.symbol = _LENBASE_string, .addr = 0x0004d274},
	{.symbol = _LENEXTRA_string, .addr = 0x0004d2e8},
	{.symbol = _LoadDrivers_string, .addr = 0x000232e1},
	{.symbol = _LoadExtraDrivers_p_string, .addr = 0x000568e4},
	{.symbol = _LoadFile_string, .addr = 0x0002e7c1},
	{.symbol = _LoadThinFatFile_string, .addr = 0x0002e2cf},
	{.symbol = _LoadVolumeFile_string, .addr = 0x0002daa5},
	{.symbol = _MD5Final_string, .addr = 0x00036399},
	{.symbol = _MD5Init_string, .addr = 0x00035b18},
	{.symbol = _MD5Pad_string, .addr = 0x0003634a},
	{.symbol = _MD5Update_string, .addr = 0x0003628e},
	{.symbol = _MSDOSFree_string, .addr = 0x00037af9},
	{.symbol = _MSDOSGetDescription_string, .addr = 0x00038cf2},
	{.symbol = _MSDOSGetDirEntry_string, .addr = 0x0003887d},
	{.symbol = _MSDOSGetFileBlock_string, .addr = 0x00038b77},
	{.symbol = _MSDOSGetUUID_string, .addr = 0x000384f2},
	{.symbol = _MSDOSInitPartition_string, .addr = 0x00038319},
	{.symbol = _MSDOSLoadFile_string, .addr = 0x000387c0},
	{.symbol = _MSDOSProbe_string, .addr = 0x000387de},
	{.symbol = _MSDOSReadFile_string, .addr = 0x00038580},
	{.symbol = _NTFSGetDescription_string, .addr = 0x00038f3d},
	{.symbol = _NTFSGetUUID_string, .addr = 0x00038eb5},
	{.symbol = _NTFSProbe_string, .addr = 0x00038e92},
	{.symbol = _PNG_adam7Pass_string, .addr = 0x000270e9},
	{.symbol = _PNG_checkColorValidity_string, .addr = 0x000268f9},
	{.symbol = _PNG_convert_string, .addr = 0x00027cce},
	{.symbol = _PNG_decode_string, .addr = 0x00028531},
	{.symbol = _PNG_error_string, .addr = 0x000568e0},
	{.symbol = _PNG_getBpp_string, .addr = 0x00026953},
	{.symbol = _PNG_info_new_string, .addr = 0x000284c1},
	{.symbol = _PNG_paethPredictor_string, .addr = 0x00026a87},
	{.symbol = _PNG_read32bitInt_string, .addr = 0x000268d3},
	{.symbol = _PNG_readBitFromReversedStream_string, .addr = 0x00026845},
	{.symbol = _PNG_readBitsFromReversedStream_string, .addr = 0x0002686c},
	{.symbol = _PNG_readPngHeader_string, .addr = 0x00026978},
	{.symbol = _PNG_setBitOfReversedStream_string, .addr = 0x000268b1},
	{.symbol = _PNG_unFilterScanline_string, .addr = 0x00026e41},
	{.symbol = _ParseXMLFile_string, .addr = 0x00031abf},
	{.symbol = _Platform_string, .addr = 0x00056f10},
	{.symbol = _ReadFileAtOffset_string, .addr = 0x0002df38},
	{.symbol = _ReadPCIBusInfo_string, .addr = 0x0003321a},
	{.symbol = _Round_string, .addr = 0x00034162},
	{.symbol = _Sqrt_string, .addr = 0x00034178},
	{.symbol = _ThinFatFile_string, .addr = 0x0002eaef},
	{.symbol = _XMLDecode_string, .addr = 0x00034af2},
	{.symbol = _XMLFreeTag_string, .addr = 0x00034c30},
	{.symbol = _XMLGetProperty_string, .addr = 0x00034a39},
	{.symbol = _XMLParseNextTag_string, .addr = 0x00034d5f},
	{.symbol = _Zlib_decompress_string, .addr = 0x000283c9},
	{.symbol = _Zlib_readBitFromStream_string, .addr = 0x000267e3},
	{.symbol = _Zlib_readBitsFromStream_string, .addr = 0x00026808},
	{.symbol = __DATA__bss__begin_string, .addr = 0x000543d0},
	{.symbol = __DATA__bss__end_string, .addr = 0x00055acc},
	{.symbol = __DATA__common__begin_string, .addr = 0x00055ad0},
	{.symbol = __DATA__common__end_string, .addr = 0x000571b8},
	{.symbol = __bp_string, .addr = 0x000203aa},
	{.symbol = __hi_malloc_string, .addr = 0x00025b0e},
	{.symbol = __hi_strdup_string, .addr = 0x00025b1f},
	{.symbol = __prot_to_real_string, .addr = 0x0002032d},
	{.symbol = __real_to_prot_string, .addr = 0x000202df},
	{.symbol = __sp_string, .addr = 0x000203a7},
	{.symbol = __switch_stack_string, .addr = 0x000203ad},
	{.symbol = _acpi10_p_string, .addr = 0x00057118},
	{.symbol = _acpi20_p_string, .addr = 0x00057120},
	{.symbol = _acpi_cpu_count_string, .addr = 0x00053832},
	{.symbol = _acpi_cpu_name_string, .addr = 0x00057130},
	{.symbol = _addConfigurationTable_string, .addr = 0x00032f49},
	{.symbol = _add_symbol_string, .addr = 0x0002d198},
	{.symbol = _aml_add_alias_string, .addr = 0x00040195},
	{.symbol = _aml_add_buffer_string, .addr = 0x0003ffb5},
	{.symbol = _aml_add_byte_string, .addr = 0x0003ff73},
	{.symbol = _aml_add_dword_string, .addr = 0x0003fecc},
	{.symbol = _aml_add_name_string, .addr = 0x00040109},
	{.symbol = _aml_add_package_string, .addr = 0x0003fdf1},
	{.symbol = _aml_add_qword_string, .addr = 0x0003fe2c},
	{.symbol = _aml_add_scope_string, .addr = 0x00040131},
	{.symbol = _aml_add_to_parent_string, .addr = 0x0003fd38},
	{.symbol = _aml_add_word_string, .addr = 0x0003ff2a},
	{.symbol = _aml_calculate_size_string, .addr = 0x0003fb0e},
	{.symbol = _aml_create_node_string, .addr = 0x0003fdcd},
	{.symbol = _aml_destroy_node_string, .addr = 0x000401e8},
	{.symbol = _aml_fill_name_string, .addr = 0x0003fffa},
	{.symbol = _aml_fill_simple_name_string, .addr = 0x00040159},
	{.symbol = _aml_get_size_length_string, .addr = 0x0003fade},
	{.symbol = _aml_write_buffer_string, .addr = 0x0003fda2},
	{.symbol = _aml_write_byte_string, .addr = 0x0003fbe5},
	{.symbol = _aml_write_dword_string, .addr = 0x0003fc12},
	{.symbol = _aml_write_node_string, .addr = 0x00040231},
	{.symbol = _aml_write_qword_string, .addr = 0x0003fc3b},
	{.symbol = _aml_write_size_string, .addr = 0x0003fca1},
	{.symbol = _aml_write_word_string, .addr = 0x0003fbf7},
	{.symbol = _animateProgressBar_string, .addr = 0x0002a6e8},
	{.symbol = _archCpuType_string, .addr = 0x00051844},
	{.symbol = _ascii_hex_to_int_string, .addr = 0x0003d1a9},
	{.symbol = _atiSetMode_1_string, .addr = 0x000392ae},
	{.symbol = _atiSetMode_2_string, .addr = 0x00039223},
	{.symbol = _ati_aapl01_coher_string, .addr = 0x00053d4c},
	{.symbol = _ati_aapl_blackscr_prefs_0_n4_string, .addr = 0x000540d4},
	{.symbol = _ati_aapl_blackscr_prefs_1_n4_string, .addr = 0x000540e0},
	{.symbol = _ati_aapl_emc_disp_list_n4_string, .addr = 0x0005400c},
	{.symbol = _ati_aux_power_conn_string, .addr = 0x00053d34},
	{.symbol = _ati_backlight_ctrl_string, .addr = 0x00053d40},
	{.symbol = _ati_card_no_string, .addr = 0x00053d58},
	{.symbol = _ati_compatible_0_string, .addr = 0x00053cb8},
	{.symbol = _ati_compatible_1_string, .addr = 0x00053cc0},
	{.symbol = _ati_connector_type_0_string, .addr = 0x00053d00},
	{.symbol = _ati_connector_type_0_n4_string, .addr = 0x00053ff4},
	{.symbol = _ati_connector_type_1_string, .addr = 0x00053d0c},
	{.symbol = _ati_connector_type_1_n4_string, .addr = 0x00054000},
	{.symbol = _ati_copyright_string, .addr = 0x00053d60},
	{.symbol = _ati_device_type_string, .addr = 0x00053cd8},
	{.symbol = _ati_device_type_0_string, .addr = 0x00053cc8},
	{.symbol = _ati_device_type_1_string, .addr = 0x00053cd0},
	{.symbol = _ati_display_con_fl_type_0_string, .addr = 0x00053d18},
	{.symbol = _ati_display_type_0_string, .addr = 0x00053d24},
	{.symbol = _ati_display_type_1_string, .addr = 0x00053d2c},
	{.symbol = _ati_efi_compile_d_string, .addr = 0x00053d68},
	{.symbol = _ati_efi_disp_conf_string, .addr = 0x00053d70},
	{.symbol = _ati_efi_drv_type_string, .addr = 0x00053d80},
	{.symbol = _ati_efi_enbl_mode_string, .addr = 0x00053d8c},
	{.symbol = _ati_efi_init_stat_string, .addr = 0x00053d98},
	{.symbol = _ati_efi_orientation_string, .addr = 0x00053da4},
	{.symbol = _ati_efi_orientation_n4_string, .addr = 0x000540f8},
	{.symbol = _ati_efi_version_string, .addr = 0x00053db0},
	{.symbol = _ati_efi_versionB_string, .addr = 0x00053db8},
	{.symbol = _ati_efi_versionE_string, .addr = 0x00053dc0},
	{.symbol = _ati_efidisplay_0_string, .addr = 0x00053cf8},
	{.symbol = _ati_efidisplay_0_n4_string, .addr = 0x00053fec},
	{.symbol = _ati_fb_offset_n4_string, .addr = 0x00054054},
	{.symbol = _ati_hwgpio_n4_string, .addr = 0x00054064},
	{.symbol = _ati_iospace_offset_n4_string, .addr = 0x00054070},
	{.symbol = _ati_mclk_string, .addr = 0x00053dc8},
	{.symbol = _ati_mclk_n4_string, .addr = 0x00054080},
	{.symbol = _ati_mem_rev_id_string, .addr = 0x00053dd4},
	{.symbol = _ati_mem_vend_id_string, .addr = 0x00053de0},
	{.symbol = _ati_mrt_string, .addr = 0x00053dec},
	{.symbol = _ati_mvad_string, .addr = 0x00053e9c},
	{.symbol = _ati_mvad_n4_string, .addr = 0x00054104},
	{.symbol = _ati_name_string, .addr = 0x00053cf0},
	{.symbol = _ati_name_0_string, .addr = 0x00053ce0},
	{.symbol = _ati_name_1_string, .addr = 0x00053ce8},
	{.symbol = _ati_platform_info_string, .addr = 0x00053e14},
	{.symbol = _ati_refclk_n4_string, .addr = 0x00054098},
	{.symbol = _ati_regspace_offset_n4_string, .addr = 0x000540a4},
	{.symbol = _ati_romno_string, .addr = 0x00053df4},
	{.symbol = _ati_saved_config_string, .addr = 0x00053ee4},
	{.symbol = _ati_saved_config_n4_string, .addr = 0x0005420c},
	{.symbol = _ati_sclk_string, .addr = 0x00053dfc},
	{.symbol = _ati_sclk_n4_string, .addr = 0x0005408c},
	{.symbol = _ati_swgpio_info_n4_string, .addr = 0x000540ec},
	{.symbol = _ati_vendor_id_string, .addr = 0x00053e08},
	{.symbol = _ati_vram_memsize_0_string, .addr = 0x000540b4},
	{.symbol = _ati_vram_memsize_1_string, .addr = 0x000540c4},
	{.symbol = _atoi_string, .addr = 0x0004286a},
	{.symbol = _b_lseek_string, .addr = 0x0002e3bb},
	{.symbol = _bcopy_string, .addr = 0x00042714},
	{.symbol = _bgetc_string, .addr = 0x00033b95},
	{.symbol = _bind_macho_string, .addr = 0x0002d322},
	{.symbol = _bios_string, .addr = 0x0002040a},
	{.symbol = _biosDevIsCDROM_string, .addr = 0x0002ec78},
	{.symbol = _biosTypeNames_string, .addr = 0x00051950},
	{.symbol = _biosread_string, .addr = 0x00033646},
	{.symbol = _blend_string, .addr = 0x0002987b},
	{.symbol = _blendImage_string, .addr = 0x000212d0},
	{.symbol = _boot_string, .addr = 0x00021297},
	{.symbol = _bootArgs_string, .addr = 0x00055ad0},
	{.symbol = _bootBanner_string, .addr = 0x0004f7f4},
	{.symbol = _bootImageData_string, .addr = 0x000517d4},
	{.symbol = _bootImageHeight_string, .addr = 0x000517d2},
	{.symbol = _bootImageWidth_string, .addr = 0x000517d0},
	{.symbol = _bootInfo_string, .addr = 0x00055ad4},
	{.symbol = _bootPrompt_string, .addr = 0x0004f85f},
	{.symbol = _bootRescanPrompt_string, .addr = 0x0004f927},
	{.symbol = _bpResolution_string, .addr = 0x000518d0},
	{.symbol = _build_pci_dt_string, .addr = 0x000353e0},
	{.symbol = _builtin_set_string, .addr = 0x000538b8},
	{.symbol = _bvChain_string, .addr = 0x00055ad8},
	{.symbol = _bvCount_string, .addr = 0x0004f7e0},
	{.symbol = _bvr_string, .addr = 0x00055adc},
	{.symbol = _bzero_string, .addr = 0x00042735},
	{.symbol = _centeredAt_string, .addr = 0x00029a39},
	{.symbol = _centeredIn_string, .addr = 0x000299ff},
	{.symbol = _chainLoad_string, .addr = 0x000260d2},
	{.symbol = _chainbootdev_string, .addr = 0x000204dc},
	{.symbol = _chainbootflag_string, .addr = 0x000204dd},
	{.symbol = _checksum8_string, .addr = 0x000428ea},
	{.symbol = _chipsetTypeNames_string, .addr = 0x00051860},
	{.symbol = _clearActivityIndicator_string, .addr = 0x000227d6},
	{.symbol = _clearGraphicBootPrompt_string, .addr = 0x00029aee},
	{.symbol = _clearScreenRows_string, .addr = 0x0003330a},
	{.symbol = _close_string, .addr = 0x0002dabb},
	{.symbol = _closeVbios_string, .addr = 0x00031007},
	{.symbol = _closedir_string, .addr = 0x0002dd20},
	{.symbol = _colorFont_string, .addr = 0x00029b3f},
	{.symbol = _common_boot_string, .addr = 0x00020560},
	{.symbol = _continue_at_low_address_string, .addr = 0x000202b4},
	{.symbol = _convertHexStr2Binary_string, .addr = 0x0003d204},
	{.symbol = _convertImage_string, .addr = 0x00022bb4},
	{.symbol = _copyArgument_string, .addr = 0x00023c31},
	{.symbol = _copyMultibootInfo_string, .addr = 0x00025c05},
	{.symbol = _crc32_string, .addr = 0x0004365e},
	{.symbol = _createBackBuffer_string, .addr = 0x0002a56f},
	{.symbol = _createWindowBuffer_string, .addr = 0x0002a4e9},
	{.symbol = _cursor_string, .addr = 0x000518d8},
	{.symbol = _decodeRLE_string, .addr = 0x00021c50},
	{.symbol = _decompress_lzss_string, .addr = 0x0002596e},
	{.symbol = _delay_string, .addr = 0x000331e4},
	{.symbol = _detectAtiBiosType_string, .addr = 0x00039211},
	{.symbol = _detectBiosType_string, .addr = 0x00039438},
	{.symbol = _determine_safe_hi_addr_string, .addr = 0x00025a7f},
	{.symbol = _devices_number_string, .addr = 0x000538b4},
	{.symbol = _devprop_add_device_string, .addr = 0x0003efdf},
	{.symbol = _devprop_add_network_template_string, .addr = 0x0003ef38},
	{.symbol = _devprop_add_value_string, .addr = 0x0003edcb},
	{.symbol = _devprop_create_string_string, .addr = 0x0003ef92},
	{.symbol = _devprop_free_string_string, .addr = 0x0003eb56},
	{.symbol = _devprop_generate_string_string, .addr = 0x0003ebc9},
	{.symbol = _diskFreeMap_string, .addr = 0x0002ecef},
	{.symbol = _diskIsCDROM_string, .addr = 0x0002ec9c},
	{.symbol = _diskRead_string, .addr = 0x0002f9ac},
	{.symbol = _diskResetBootVolumes_string, .addr = 0x0002f1b5},
	{.symbol = _diskScanBootVolumes_string, .addr = 0x0002fd12},
	{.symbol = _diskSeek_string, .addr = 0x0002eba9},
	{.symbol = _dprintf_string, .addr = 0x0002aba7},
	{.symbol = _dram_controller_dev_string, .addr = 0x000518dc},
	{.symbol = _drawBackground_string, .addr = 0x0002aadd},
	{.symbol = _drawBootGraphics_string, .addr = 0x00029d34},
	{.symbol = _drawCheckerBoard_string, .addr = 0x00021538},
	{.symbol = _drawColorRectangle_string, .addr = 0x000227f9},
	{.symbol = _drawDataRectangle_string, .addr = 0x000219e6},
	{.symbol = _drawDeviceIcon_string, .addr = 0x0002ae8b},
	{.symbol = _drawDeviceList_string, .addr = 0x0002c996},
	{.symbol = _drawInfoMenu_string, .addr = 0x0002c211},
	{.symbol = _drawInfoMenuItems_string, .addr = 0x0002a19d},
	{.symbol = _drawPreview_string, .addr = 0x00022943},
	{.symbol = _drawProgressBar_string, .addr = 0x0002a78d},
	{.symbol = _drawStr_string, .addr = 0x0002a0ef},
	{.symbol = _drawStrCenteredAt_string, .addr = 0x0002a01e},
	{.symbol = _dumpAllTablesOfType_string, .addr = 0x0003b34d},
	{.symbol = _dumpPhysAddr_string, .addr = 0x0003aee1},
	{.symbol = _dump_pci_dt_string, .addr = 0x00035225},
	{.symbol = _ebiosEjectMedia_string, .addr = 0x000333d5},
	{.symbol = _ebiosread_string, .addr = 0x00033573},
	{.symbol = _ebioswrite_string, .addr = 0x00033482},
	{.symbol = _efi_guid_compare_string, .addr = 0x000436b7},
	{.symbol = _efi_guid_is_null_string, .addr = 0x0004368f},
	{.symbol = _efi_guid_unparse_upper_string, .addr = 0x00043708},
	{.symbol = _efi_inject_get_devprop_string_string, .addr = 0x0003ed9b},
	{.symbol = _ehci_acquire_string, .addr = 0x00033cf5},
	{.symbol = _enableA20_string, .addr = 0x00031637},
	{.symbol = _enable_pci_devs_string, .addr = 0x00035159},
	{.symbol = _error_string, .addr = 0x0003245a},
	{.symbol = _execute_hook_string, .addr = 0x0002d213},
	{.symbol = _file_size_string, .addr = 0x0002e3e7},
	{.symbol = _fillPixmapWithColor_string, .addr = 0x00029ab5},
	{.symbol = _finalizeBootStruct_string, .addr = 0x000316a1},
	{.symbol = _find_and_read_smbus_controller_string, .addr = 0x0003b49a},
	{.symbol = _flipRB_string, .addr = 0x00029a7a},
	{.symbol = _font_console_string, .addr = 0x00055ae0},
	{.symbol = _font_small_string, .addr = 0x00055e60},
	{.symbol = _force_enable_hpet_string, .addr = 0x00040360},
	{.symbol = _free_string, .addr = 0x00042bec},
	{.symbol = _freeFilteredBVChain_string, .addr = 0x0002ecc3},
	{.symbol = _freeWindowBuffer_string, .addr = 0x00029d01},
	{.symbol = _freqs_string, .addr = 0x00051960},
	{.symbol = _gAppleBootPictRLE_string, .addr = 0x0004d508},
	{.symbol = _gAutoResolution_string, .addr = 0x000561e0},
	{.symbol = _gBIOSBootVolume_string, .addr = 0x00051804},
	{.symbol = _gBIOSDev_string, .addr = 0x000561e4},
	{.symbol = _gBootFileType_string, .addr = 0x000561e8},
	{.symbol = _gBootFileType_t_string, .addr = 0x000561ec},
	{.symbol = _gBootMode_string, .addr = 0x000561f0},
	{.symbol = _gBootVolume_string, .addr = 0x000561f4},
	{.symbol = _gCompareTable_string, .addr = 0x000571b0},
	{.symbol = _gCompareTableCompressed_string, .addr = 0x00053bb8},
	{.symbol = _gDeviceCount_string, .addr = 0x0004f7e4},
	{.symbol = _gEfiAcpi20TableGuid_string, .addr = 0x000518fc},
	{.symbol = _gEfiAcpiTableGuid_string, .addr = 0x000518ec},
	{.symbol = _gEfiConfigurationTableNode_string, .addr = 0x000518e8},
	{.symbol = _gEfiSmbiosTableGuid_string, .addr = 0x0004dce8},
	{.symbol = _gEnableCDROMRescan_string, .addr = 0x000561f8},
	{.symbol = _gErrors_string, .addr = 0x000561f9},
	{.symbol = _gFSLoadAddress_string, .addr = 0x00051800},
	{.symbol = _gHaveKernelCache_string, .addr = 0x000561fa},
	{.symbol = _gLowerCaseTable_string, .addr = 0x000571b4},
	{.symbol = _gLowerCaseTableCompressed_string, .addr = 0x000538e0},
	{.symbol = _gMI_string, .addr = 0x000568ec},
	{.symbol = _gMKextName_string, .addr = 0x00056200},
	{.symbol = _gMacOSVersion_string, .addr = 0x00056400},
	{.symbol = _gMemoryMapNode_string, .addr = 0x00056f00},
	{.symbol = _gOverrideKernel_string, .addr = 0x00056408},
	{.symbol = _gRAMDiskBTAliased_string, .addr = 0x000511a0},
	{.symbol = _gRAMDiskFile_string, .addr = 0x000568f0},
	{.symbol = _gRAMDiskMI_string, .addr = 0x00051198},
	{.symbol = _gRAMDiskVolume_string, .addr = 0x0005119c},
	{.symbol = _gRootDevice_string, .addr = 0x00056410},
	{.symbol = _gST32_string, .addr = 0x000518e0},
	{.symbol = _gST64_string, .addr = 0x000518e4},
	{.symbol = _gScanSingleDrive_string, .addr = 0x00056610},
	{.symbol = _gVerboseMode_string, .addr = 0x00056611},
	{.symbol = _generateCRTCTiming_string, .addr = 0x0003452d},
	{.symbol = _generate_cst_ssdt_string, .addr = 0x0003dda5},
	{.symbol = _generate_pss_ssdt_string, .addr = 0x0003d70f},
	{.symbol = _getAspectRatio_string, .addr = 0x00030d72},
	{.symbol = _getBVChainForBIOSDev_string, .addr = 0x0002eb8a},
	{.symbol = _getBoolForKey_string, .addr = 0x000320bf},
	{.symbol = _getBootOptions_string, .addr = 0x00024913},
	{.symbol = _getBootVolumeDescription_string, .addr = 0x0002ef1d},
	{.symbol = _getBootVolumeRef_string, .addr = 0x0002ddf3},
	{.symbol = _getChipset_string, .addr = 0x00030a24},
	{.symbol = _getChipsetId_string, .addr = 0x00030a11},
	{.symbol = _getColorForKey_string, .addr = 0x00031f00},
	{.symbol = _getConventionalMemorySize_string, .addr = 0x0003371f},
	{.symbol = _getCroppedPixmapAtPosition_string, .addr = 0x0002a5fe},
	{.symbol = _getCursorPositionAndType_string, .addr = 0x0003332d},
	{.symbol = _getDDRPartNum_string, .addr = 0x0003b502},
	{.symbol = _getDDRSerial_string, .addr = 0x0003b5be},
	{.symbol = _getDDRspeedMhz_string, .addr = 0x0003b444},
	{.symbol = _getDMIString_string, .addr = 0x0003adda},
	{.symbol = _getDeviceDescription_string, .addr = 0x0002db8d},
	{.symbol = _getDimensionForKey_string, .addr = 0x00031f49},
	{.symbol = _getEDID_string, .addr = 0x000343de},
	{.symbol = _getExtendedMemorySize_string, .addr = 0x0003373f},
	{.symbol = _getGraphicModeParams_string, .addr = 0x00021f05},
	{.symbol = _getIntForKey_string, .addr = 0x0003202e},
	{.symbol = _getMemoryInfoString_string, .addr = 0x0002437c},
	{.symbol = _getMemoryMap_string, .addr = 0x00033a02},
	{.symbol = _getNextArg_string, .addr = 0x00031a23},
	{.symbol = _getNumberArrayFromProperty_string, .addr = 0x00021970},
	{.symbol = _getPciRootUID_string, .addr = 0x0003f517},
	{.symbol = _getPlatformName_string, .addr = 0x00031623},
	{.symbol = _getResolution_string, .addr = 0x00032372},
	{.symbol = _getSmbios_string, .addr = 0x0003be75},
	{.symbol = _getStringForKey_string, .addr = 0x000321a1},
	{.symbol = _getStringFromUUID_string, .addr = 0x0003d2d0},
	{.symbol = _getUUIDFromString_string, .addr = 0x0003d347},
	{.symbol = _getVBECurrentMode_string, .addr = 0x000342a8},
	{.symbol = _getVBEDACFormat_string, .addr = 0x00034468},
	{.symbol = _getVBEInfo_string, .addr = 0x000344ec},
	{.symbol = _getVBEInfoString_string, .addr = 0x00022493},
	{.symbol = _getVBEModeInfo_string, .addr = 0x000344a2},
	{.symbol = _getVBEModeInfoString_string, .addr = 0x00022329},
	{.symbol = _getVBEPalette_string, .addr = 0x000342dc},
	{.symbol = _getVBEPixelClock_string, .addr = 0x0003425a},
	{.symbol = _getVESAModeWithProperties_string, .addr = 0x00021ca9},
	{.symbol = _getValueForBootKey_string, .addr = 0x00031c0c},
	{.symbol = _getValueForConfigTableKey_string, .addr = 0x00031cb5},
	{.symbol = _getValueForKey_string, .addr = 0x00031d12},
	{.symbol = _getVendorName_string, .addr = 0x0003b654},
	{.symbol = _getVideoMode_string, .addr = 0x00021960},
	{.symbol = _getVolumeLabelAlias_string, .addr = 0x0002ee2b},
	{.symbol = _get_acpi_cpu_names_string, .addr = 0x0003d44f},
	{.symbol = _get_drive_info_string, .addr = 0x000338c5},
	{.symbol = _get_gma_model_string, .addr = 0x000416f5},
	{.symbol = _get_pci_dev_path_string, .addr = 0x00035188},
	{.symbol = _getc_string, .addr = 0x0003267c},
	{.symbol = _getchar_string, .addr = 0x00032748},
	{.symbol = _getvramsizekb_string, .addr = 0x000408af},
	{.symbol = _gprintf_string, .addr = 0x0002ad22},
	{.symbol = _gtfTimings_string, .addr = 0x00030bce},
	{.symbol = _gui_string, .addr = 0x00056620},
	{.symbol = _halt_string, .addr = 0x00020383},
	{.symbol = _handle_symtable_string, .addr = 0x0002d04f},
	{.symbol = _hex2bin_string, .addr = 0x00041e70},
	{.symbol = _hi_multiboot_string, .addr = 0x00025db9},
	{.symbol = _imageCnt_string, .addr = 0x00051750},
	{.symbol = _images_string, .addr = 0x000511b0},
	{.symbol = _infoMenuItems_string, .addr = 0x00051760},
	{.symbol = _initBooterLog_string, .addr = 0x00032715},
	{.symbol = _initFont_string, .addr = 0x0002a36a},
	{.symbol = _initGUI_string, .addr = 0x0002b98a},
	{.symbol = _initGraphicsMode_string, .addr = 0x0002212f},
	{.symbol = _initKernBootStruct_string, .addr = 0x00031825},
	{.symbol = _init_module_system_string, .addr = 0x0002d95d},
	{.symbol = _initialize_runtime_string, .addr = 0x00021255},
	{.symbol = _intelSetMode_1_string, .addr = 0x0003961c},
	{.symbol = _intelSetMode_2_string, .addr = 0x0003972e},
	{.symbol = _intelSetMode_3_string, .addr = 0x0003988a},
	{.symbol = _intializeTables_string, .addr = 0x00030f78},
	{.symbol = _is_module_laoded_string, .addr = 0x0002d166},
	{.symbol = _is_no_emulation_string, .addr = 0x00033846},
	{.symbol = _jump_to_chainbooter_string, .addr = 0x000202ca},
	{.symbol = _lasttime_string, .addr = 0x000511ac},
	{.symbol = _legacy_off_string, .addr = 0x00033f47},
	{.symbol = _loadACPITable_string, .addr = 0x0003e21e},
	{.symbol = _loadConfigFile_string, .addr = 0x00031bbe},
	{.symbol = _loadEmbeddedPngImage_string, .addr = 0x00021a92},
	{.symbol = _loadHelperConfig_string, .addr = 0x00031b4c},
	{.symbol = _loadImageScale_string, .addr = 0x0002164b},
	{.symbol = _loadOverrideConfig_string, .addr = 0x00031df4},
	{.symbol = _loadPngImage_string, .addr = 0x00021b35},
	{.symbol = _loadPrebootRAMDisk_string, .addr = 0x00026590},
	{.symbol = _loadSystemConfig_string, .addr = 0x0003210c},
	{.symbol = _loadThemeValues_string, .addr = 0x0002b3d7},
	{.symbol = _load_all_modules_string, .addr = 0x0002d8ed},
	{.symbol = _load_module_string, .addr = 0x0002d6a7},
	{.symbol = _loadedModules_string, .addr = 0x000517f4},
	{.symbol = _loader_string, .addr = 0x000203dd},
	{.symbol = _lookUpCLUTIndex_string, .addr = 0x000215bf},
	{.symbol = _lookup_all_symbols_string, .addr = 0x0002d26c},
	{.symbol = _lookup_symbol_string, .addr = 0x000517fc},
	{.symbol = _lspci_string, .addr = 0x000248b9},
	{.symbol = _makeRoundedCorners_string, .addr = 0x00029bd4},
	{.symbol = _malloc_init_string, .addr = 0x00042a86},
	{.symbol = _map_string, .addr = 0x000568d4},
	{.symbol = _mapType1Resolution_string, .addr = 0x0003940b},
	{.symbol = _mapType2Resolution_string, .addr = 0x0003941a},
	{.symbol = _mapType3Resolution_string, .addr = 0x00039429},
	{.symbol = _mapping_string, .addr = 0x0005350c},
	{.symbol = _matchVolumeToString_string, .addr = 0x0002ed21},
	{.symbol = _md0Ramdisk_string, .addr = 0x000265b9},
	{.symbol = _mem_detect_string, .addr = 0x00041a83},
	{.symbol = _memcmp_string, .addr = 0x00042766},
	{.symbol = _memcpy_string, .addr = 0x000426f1},
	{.symbol = _memset_string, .addr = 0x000426dd},
	{.symbol = _menuBVR_string, .addr = 0x000568d8},
	{.symbol = _menuItems_string, .addr = 0x00051180},
	{.symbol = _moduleCallbacks_string, .addr = 0x000517f0},
	{.symbol = _moduleSymbols_string, .addr = 0x000517f8},
	{.symbol = _module_loaded_string, .addr = 0x0002d0b4},
	{.symbol = _mountRAMDisk_string, .addr = 0x000262a4},
	{.symbol = _msgbuf_string, .addr = 0x000518d4},
	{.symbol = _msglog_string, .addr = 0x0003261c},
	{.symbol = _multibootRamdiskReadBytes_string, .addr = 0x00025bae},
	{.symbol = _multiboot_get_ramdisk_info_string, .addr = 0x00025b47},
	{.symbol = _multiboot_partition_string, .addr = 0x00051190},
	{.symbol = _multiboot_partition_set_string, .addr = 0x00051194},
	{.symbol = _multiboot_timeout_string, .addr = 0x00051188},
	{.symbol = _multiboot_timeout_set_string, .addr = 0x0005118c},
	{.symbol = _multiboot_to_boot_string, .addr = 0x00026117},
	{.symbol = _nbpScanBootVolumes_string, .addr = 0x00032791},
	{.symbol = _nbpUnloadBaseCode_string, .addr = 0x00032798},
	{.symbol = _newAPMBVRef_string, .addr = 0x0002f249},
	{.symbol = _newFilteredBVChain_string, .addr = 0x0002f036},
	{.symbol = _newGPTBVRef_string, .addr = 0x0002fac8},
	{.symbol = _newString_string, .addr = 0x00031a8c},
	{.symbol = _newStringForKey_string, .addr = 0x000321d9},
	{.symbol = _nhm_bus_string, .addr = 0x0005196c},
	{.symbol = _notify_usb_dev_string, .addr = 0x00034100},
	{.symbol = _nvidiaSetMode_string, .addr = 0x000399e8},
	{.symbol = _nvidia_compatible_0_string, .addr = 0x00054374},
	{.symbol = _nvidia_compatible_1_string, .addr = 0x0005437c},
	{.symbol = _nvidia_device_type_string, .addr = 0x00054394},
	{.symbol = _nvidia_device_type_0_string, .addr = 0x00054384},
	{.symbol = _nvidia_device_type_1_string, .addr = 0x0005438c},
	{.symbol = _nvidia_name_0_string, .addr = 0x0005439c},
	{.symbol = _nvidia_name_1_string, .addr = 0x000543a4},
	{.symbol = _nvidia_slot_name_string, .addr = 0x000543ac},
	{.symbol = _open_string, .addr = 0x0002e632},
	{.symbol = _openAtiVbios_string, .addr = 0x00039338},
	{.symbol = _openIntelVbios_string, .addr = 0x0003948e},
	{.symbol = _openNvidiaVbios_string, .addr = 0x00039bf7},
	{.symbol = _openVbios_string, .addr = 0x000310c2},
	{.symbol = _open_bvdev_string, .addr = 0x0002e65e},
	{.symbol = _opendir_string, .addr = 0x0002dfe5},
	{.symbol = _openmem_string, .addr = 0x0002e088},
	{.symbol = _p_get_ramdisk_info_string, .addr = 0x0005184c},
	{.symbol = _p_ramdiskReadBytes_string, .addr = 0x00051848},
	{.symbol = _parse_mach_string, .addr = 0x0002d781},
	{.symbol = _patchVbios_string, .addr = 0x00031417},
	{.symbol = _patch_fadt_string, .addr = 0x0003d559},
	{.symbol = _pause_string, .addr = 0x0003277d},
	{.symbol = _pci_config_read16_string, .addr = 0x000350e7},
	{.symbol = _pci_config_read32_string, .addr = 0x00035112},
	{.symbol = _pci_config_read8_string, .addr = 0x000350bd},
	{.symbol = _pci_config_write16_string, .addr = 0x0003512a},
	{.symbol = _pci_config_write32_string, .addr = 0x000353c5},
	{.symbol = _pci_config_write8_string, .addr = 0x00035397},
	{.symbol = _platformCPUFeature_string, .addr = 0x0003279f},
	{.symbol = _png_alloc_add_node_string, .addr = 0x00026be9},
	{.symbol = _png_alloc_find_node_string, .addr = 0x00026745},
	{.symbol = _png_alloc_free_string, .addr = 0x00026b5b},
	{.symbol = _png_alloc_free_all_string, .addr = 0x00026b36},
	{.symbol = _png_alloc_head_string, .addr = 0x000511a4},
	{.symbol = _png_alloc_malloc_string, .addr = 0x00026c55},
	{.symbol = _png_alloc_realloc_string, .addr = 0x00026c8e},
	{.symbol = _png_alloc_remove_node_string, .addr = 0x00026add},
	{.symbol = _png_alloc_tail_string, .addr = 0x000511a8},
	{.symbol = _pos_string, .addr = 0x00029a61},
	{.symbol = _previewLoadedSectors_string, .addr = 0x0004f7ec},
	{.symbol = _previewSaveunder_string, .addr = 0x0004f7f0},
	{.symbol = _previewTotalSectors_string, .addr = 0x0004f7e8},
	{.symbol = _prf_string, .addr = 0x00043495},
	{.symbol = _printVBEModeInfo_string, .addr = 0x000225a5},
	{.symbol = _printf_string, .addr = 0x00032536},
	{.symbol = _processBootArgument_string, .addr = 0x00023cb8},
	{.symbol = _processBootOptions_string, .addr = 0x00023d47},
	{.symbol = _processRAMDiskCommand_string, .addr = 0x00026440},
	{.symbol = _prompt_string, .addr = 0x00056b00},
	{.symbol = _promptForRescanOption_string, .addr = 0x00023879},
	{.symbol = _prompt_pos_string, .addr = 0x00051754},
	{.symbol = _prompt_text_string, .addr = 0x00051758},
	{.symbol = _ptol_string, .addr = 0x00042848},
	{.symbol = _putc_string, .addr = 0x0003344b},
	{.symbol = _putca_string, .addr = 0x0003340b},
	{.symbol = _putchar_string, .addr = 0x00032693},
	{.symbol = _rawDiskRead_string, .addr = 0x0002f41f},
	{.symbol = _rawDiskWrite_string, .addr = 0x0002f319},
	{.symbol = _read_string, .addr = 0x0002e508},
	{.symbol = _readBootSector_string, .addr = 0x0002fa5f},
	{.symbol = _readEDID_string, .addr = 0x00032230},
	{.symbol = _readKeyboardShiftFlags_string, .addr = 0x000337ec},
	{.symbol = _readKeyboardStatus_string, .addr = 0x00033813},
	{.symbol = _readdir_string, .addr = 0x0002db0b},
	{.symbol = _readdir_ext_string, .addr = 0x0002db2d},
	{.symbol = _realloc_string, .addr = 0x00042f19},
	{.symbol = _rebase_location_string, .addr = 0x0002d042},
	{.symbol = _rebase_macho_string, .addr = 0x0002ce0f},
	{.symbol = _reg_FALSE_string, .addr = 0x00054370},
	{.symbol = _reg_TRUE_string, .addr = 0x0005436c},
	{.symbol = _register_hook_string, .addr = 0x0002d11b},
	{.symbol = _register_hook_callback_string, .addr = 0x0002d98d},
	{.symbol = _relockVbios_string, .addr = 0x00030d20},
	{.symbol = _replace_function_string, .addr = 0x0002d2e1},
	{.symbol = _rescanBIOSDevice_string, .addr = 0x0002f21c},
	{.symbol = _reserveKernBootStruct_string, .addr = 0x000317f8},
	{.symbol = _restoreTables_string, .addr = 0x00030e0e},
	{.symbol = _restoreVbios_string, .addr = 0x00031530},
	{.symbol = _root_pci_dev_string, .addr = 0x000568e8},
	{.symbol = _safe_malloc_string, .addr = 0x00042ddd},
	{.symbol = _saveTables_string, .addr = 0x00030f0e},
	{.symbol = _saveVbios_string, .addr = 0x00030f46},
	{.symbol = _scanBootVolumes_string, .addr = 0x0002dd46},
	{.symbol = _scanDisks_string, .addr = 0x0002df76},
	{.symbol = _scan_cpu_string, .addr = 0x00039dd4},
	{.symbol = _scan_dram_controller_string, .addr = 0x0003a672},
	{.symbol = _scan_mem_string, .addr = 0x000327db},
	{.symbol = _scan_memory_string, .addr = 0x0003ae3b},
	{.symbol = _scan_pci_bus_string, .addr = 0x0003527d},
	{.symbol = _scan_platform_string, .addr = 0x000327b3},
	{.symbol = _scan_spd_string, .addr = 0x0003b4f1},
	{.symbol = _scollPage_string, .addr = 0x000332b1},
	{.symbol = _search_and_get_acpi_fd_string, .addr = 0x0003e061},
	{.symbol = _selectAlternateBootDevice_string, .addr = 0x00023897},
	{.symbol = _selectBootVolume_string, .addr = 0x0002dbec},
	{.symbol = _selectIndex_string, .addr = 0x0005117c},
	{.symbol = _setActiveDisplayPage_string, .addr = 0x00033288},
	{.symbol = _setBootGlobals_string, .addr = 0x0002e780},
	{.symbol = _setCursorPosition_string, .addr = 0x0003339c},
	{.symbol = _setCursorType_string, .addr = 0x00033372},
	{.symbol = _setRAMDiskBTHook_string, .addr = 0x00026157},
	{.symbol = _setRootVolume_string, .addr = 0x0002db6d},
	{.symbol = _setVBEDACFormat_string, .addr = 0x00034430},
	{.symbol = _setVBEMode_string, .addr = 0x00034390},
	{.symbol = _setVBEPalette_string, .addr = 0x00034336},
	{.symbol = _setVideoMode_string, .addr = 0x000221e3},
	{.symbol = _set_eth_builtin_string, .addr = 0x0003f28c},
	{.symbol = _setupAcpi_string, .addr = 0x0003e2c0},
	{.symbol = _setupBooterLog_string, .addr = 0x000326cf},
	{.symbol = _setupDeviceList_string, .addr = 0x0002b120},
	{.symbol = _setupDeviceProperties_string, .addr = 0x0003f353},
	{.symbol = _setupEfiDeviceTree_string, .addr = 0x00032902},
	{.symbol = _setupEfiTables32_string, .addr = 0x00032dcb},
	{.symbol = _setupEfiTables64_string, .addr = 0x00032b8e},
	{.symbol = _setupFakeEfi_string, .addr = 0x00033001},
	{.symbol = _setupSystemType_string, .addr = 0x0003283f},
	{.symbol = _setup_ati_devprop_string, .addr = 0x00040a4a},
	{.symbol = _setup_gma_devprop_string, .addr = 0x0004171c},
	{.symbol = _setup_nvidia_devprop_string, .addr = 0x00041f37},
	{.symbol = _setup_pci_devs_string, .addr = 0x0003f3ca},
	{.symbol = _showBootBanner_string, .addr = 0x00051178},
	{.symbol = _showHelp_string, .addr = 0x00023baa},
	{.symbol = _showInfoBox_string, .addr = 0x0002c22c},
	{.symbol = _showInfoRAMDisk_string, .addr = 0x0002617c},
	{.symbol = _showTextFile_string, .addr = 0x00023b1b},
	{.symbol = _sleep_string, .addr = 0x00033beb},
	{.symbol = _slvprintf_string, .addr = 0x00042f73},
	{.symbol = _smb_read_byte_intel_string, .addr = 0x0003b39e},
	{.symbol = _smbios_p_string, .addr = 0x00057128},
	{.symbol = _smbios_properties_string, .addr = 0x0005353c},
	{.symbol = _smbios_table_descriptions_string, .addr = 0x000537a4},
	{.symbol = _spd_indexes_string, .addr = 0x000534b8},
	{.symbol = _spinActivityIndicator_string, .addr = 0x0002271b},
	{.symbol = _sprintf_string, .addr = 0x00042fad},
	{.symbol = _startprog_string, .addr = 0x0002038d},
	{.symbol = _stop_string, .addr = 0x000325c5},
	{.symbol = _stosl_string, .addr = 0x00021637},
	{.symbol = _strbreak_string, .addr = 0x00042904},
	{.symbol = _strcat_string, .addr = 0x000429d3},
	{.symbol = _strchr_string, .addr = 0x000428cb},
	{.symbol = _strcmp_string, .addr = 0x0004278d},
	{.symbol = _strcpy_string, .addr = 0x000427e2},
	{.symbol = _strdup_string, .addr = 0x00042999},
	{.symbol = _string_string, .addr = 0x000538bc},
	{.symbol = _stringLength_string, .addr = 0x0003199d},
	{.symbol = _stringdata_string, .addr = 0x000538c0},
	{.symbol = _stringlength_string, .addr = 0x000538c4},
	{.symbol = _strlcpy_string, .addr = 0x0004281e},
	{.symbol = _strlen_string, .addr = 0x00042753},
	{.symbol = _strncat_string, .addr = 0x0004289b},
	{.symbol = _strncmp_string, .addr = 0x000427ae},
	{.symbol = _strncpy_string, .addr = 0x000427fa},
	{.symbol = _strstr_string, .addr = 0x00042a0d},
	{.symbol = _strtol_string, .addr = 0x00042fe9},
	{.symbol = _strtoul_string, .addr = 0x00043326},
	{.symbol = _strtouq_string, .addr = 0x0004315a},
	{.symbol = _sysConfigValid_string, .addr = 0x000568dc},
	{.symbol = _systemConfigDir_string, .addr = 0x0002db51},
	{.symbol = _tableSign_string, .addr = 0x0003d41a},
	{.symbol = _tell_string, .addr = 0x0002dae5},
	{.symbol = _testBiosread_string, .addr = 0x000309ff},
	{.symbol = _testFAT32EFIBootSector_string, .addr = 0x0002f9db},
	{.symbol = _time18_string, .addr = 0x000337b2},
	{.symbol = _uhci_reset_string, .addr = 0x00033c68},
	{.symbol = _umountRAMDisk_string, .addr = 0x0002620e},
	{.symbol = _unlockVbios_string, .addr = 0x00030ca8},
	{.symbol = _updateGraphicBootPrompt_string, .addr = 0x0002c832},
	{.symbol = _updateInfoMenu_string, .addr = 0x0002c57b},
	{.symbol = _updateProgressBar_string, .addr = 0x0002178d},
	{.symbol = _updateVRAM_string, .addr = 0x0002bfb4},
	{.symbol = _usbList_string, .addr = 0x0005190c},
	{.symbol = _usb_loop_string, .addr = 0x0003400b},
	{.symbol = _useGUI_string, .addr = 0x000568dd},
	{.symbol = _utf_decodestr_string, .addr = 0x0003f956},
	{.symbol = _utf_encodestr_string, .addr = 0x0003fa31},
	{.symbol = _vector32_cleanup_string, .addr = 0x00026bbc},
	{.symbol = _vector32_init_string, .addr = 0x00026760},
	{.symbol = _vector32_new_string, .addr = 0x00027303},
	{.symbol = _vector32_resize_string, .addr = 0x00026d36},
	{.symbol = _vector32_resizev_string, .addr = 0x00027299},
	{.symbol = _vector8_cleanup_string, .addr = 0x00026b8f},
	{.symbol = _vector8_copy_string, .addr = 0x000284fe},
	{.symbol = _vector8_init_string, .addr = 0x0002677c},
	{.symbol = _vector8_new_string, .addr = 0x0002844c},
	{.symbol = _vector8_resize_string, .addr = 0x00026ce9},
	{.symbol = _vector8_resizev_string, .addr = 0x000279dd},
	{.symbol = _vendorMap_string, .addr = 0x00051970},
	{.symbol = _verbose_string, .addr = 0x0003249e},
	{.symbol = _video_mode_string, .addr = 0x000336f6},
	{.symbol = _vol_opendir_string, .addr = 0x0002e043},
	{.symbol = _vprf_string, .addr = 0x0002c67b},
	{.symbol = _vramwrite_string, .addr = 0x0002a95f},
	{.symbol = _waitThenReload_string, .addr = 0x00025d3b},
	{.symbol = _write_string, .addr = 0x0002e4a9},
	{.symbol = _writebyte_string, .addr = 0x0002e45f},
	{.symbol = _writeint_string, .addr = 0x0002e40d},
	{.symbol = _xResolution_string, .addr = 0x000518c8},
	{.symbol = _yResolution_string, .addr = 0x000518cc},
	{.symbol = boot2_string, .addr = 0x00020200},
};
