typedef struct {
	char*	symbol;
	unsigned int	addr;
} symbol_t;

static char _Adler32_string[] = "_Adler32";
static char _AllocateKernelMemory_string[] = "_AllocateKernelMemory";
static char _AllocateMemoryRange_string[] = "_AllocateMemoryRange";
static char _BinaryUnicodeCompare_string[] = "_BinaryUnicodeCompare";
static char _BootHelp_txt_string[] = "_BootHelp_txt";
static char _BootHelp_txt_len_string[] = "_BootHelp_txt_len";
static char _BootOrder_string[] = "_BootOrder";
static char _CacheInit_string[] = "_CacheInit";
static char _CacheRead_string[] = "_CacheRead";
static char _CacheReset_string[] = "_CacheReset";
static char _CreateUUIDString_string[] = "_CreateUUIDString";
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
static char _ExecKernel_string[] = "_ExecKernel";
static char _FastRelString_string[] = "_FastRelString";
static char _FastUnicodeCompare_string[] = "_FastUnicodeCompare";
static char _FileLoadDrivers_string[] = "_FileLoadDrivers";
static char _FindFirstDmiTableOfType_string[] = "_FindFirstDmiTableOfType";
static char _FindNextDmiTableOfType_string[] = "_FindNextDmiTableOfType";
static char _FourChar_string[] = "_FourChar";
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
static char _GetRefString_string[] = "_GetRefString";
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
static char _Idtr_prot_string[] = "_Idtr_prot";
static char _Idtr_real_string[] = "_Idtr_real";
static char _InitDriverSupport_string[] = "_InitDriverSupport";
static char _LoadDriverMKext_string[] = "_LoadDriverMKext";
static char _LoadDriverPList_string[] = "_LoadDriverPList";
static char _LoadDrivers_string[] = "_LoadDrivers";
static char _LoadExtraDrivers_p_string[] = "_LoadExtraDrivers_p";
static char _LoadFile_string[] = "_LoadFile";
static char _LoadMatchedModules_string[] = "_LoadMatchedModules";
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
static char _MacModel_string[] = "_MacModel";
static char _MatchLibraries_string[] = "_MatchLibraries";
static char _MatchPersonalities_string[] = "_MatchPersonalities";
static char _ModelLength_string[] = "_ModelLength";
static char _ModelRev_string[] = "_ModelRev";
static char _NTFSGetDescription_string[] = "_NTFSGetDescription";
static char _NTFSGetUUID_string[] = "_NTFSGetUUID";
static char _NTFSProbe_string[] = "_NTFSProbe";
static char _NetLoadDrivers_string[] = "_NetLoadDrivers";
static char _ParseXMLFile_string[] = "_ParseXMLFile";
static char _Platform_string[] = "_Platform";
static char _ReadFileAtOffset_string[] = "_ReadFileAtOffset";
static char _ReadPCIBusInfo_string[] = "_ReadPCIBusInfo";
static char _Round_string[] = "_Round";
static char _SYSTEM_ID_DEFAULT_string[] = "_SYSTEM_ID_DEFAULT";
static char _SaveRefString_string[] = "_SaveRefString";
static char _ThinFatFile_string[] = "_ThinFatFile";
static char _XMLCastArray_string[] = "_XMLCastArray";
static char _XMLCastBoolean_string[] = "_XMLCastBoolean";
static char _XMLCastDict_string[] = "_XMLCastDict";
static char _XMLCastInteger_string[] = "_XMLCastInteger";
static char _XMLCastString_string[] = "_XMLCastString";
static char _XMLCastStringOffset_string[] = "_XMLCastStringOffset";
static char _XMLDecode_string[] = "_XMLDecode";
static char _XMLFreeTag_string[] = "_XMLFreeTag";
static char _XMLGetElement_string[] = "_XMLGetElement";
static char _XMLGetProperty_string[] = "_XMLGetProperty";
static char _XMLIsType_string[] = "_XMLIsType";
static char _XMLParseFile_string[] = "_XMLParseFile";
static char _XMLParseNextTag_string[] = "_XMLParseNextTag";
static char _XMLTagCount_string[] = "_XMLTagCount";
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
static char _addBootArg_string[] = "_addBootArg";
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
static char _archCpuType_string[] = "_archCpuType";
static char _ascii_hex_to_int_string[] = "_ascii_hex_to_int";
static char _atoi_string[] = "_atoi";
static char _b_lseek_string[] = "_b_lseek";
static char _bcopy_string[] = "_bcopy";
static char _bgetc_string[] = "_bgetc";
static char _bind_location_string[] = "_bind_location";
static char _bind_macho_string[] = "_bind_macho";
static char _bios_string[] = "_bios";
static char _biosDevIsCDROM_string[] = "_biosDevIsCDROM";
static char _biosread_string[] = "_biosread";
static char _boot_string[] = "_boot";
static char _bootArgs_string[] = "_bootArgs";
static char _bootBanner_string[] = "_bootBanner";
static char _bootInfo_string[] = "_bootInfo";
static char _bootPrompt_string[] = "_bootPrompt";
static char _bootRescanPrompt_string[] = "_bootRescanPrompt";
static char _booterCommand_string[] = "_booterCommand";
static char _booterParam_string[] = "_booterParam";
static char _build_pci_dt_string[] = "_build_pci_dt";
static char _builtin_set_string[] = "_builtin_set";
static char _bvChain_string[] = "_bvChain";
static char _bvCount_string[] = "_bvCount";
static char _bvr_string[] = "_bvr";
static char _bzero_string[] = "_bzero";
static char _chainLoad_string[] = "_chainLoad";
static char _chainbootdev_string[] = "_chainbootdev";
static char _chainbootflag_string[] = "_chainbootflag";
static char _changeCursor_string[] = "_changeCursor";
static char _checksum8_string[] = "_checksum8";
static char _clearActivityIndicator_string[] = "_clearActivityIndicator";
static char _clearBootArgs_string[] = "_clearBootArgs";
static char _clearScreenRows_string[] = "_clearScreenRows";
static char _close_string[] = "_close";
static char _closedir_string[] = "_closedir";
static char _common_boot_string[] = "_common_boot";
static char _continue_at_low_address_string[] = "_continue_at_low_address";
static char _convertHexStr2Binary_string[] = "_convertHexStr2Binary";
static char _copyArgument_string[] = "_copyArgument";
static char _copyMultibootInfo_string[] = "_copyMultibootInfo";
static char _crc32_string[] = "_crc32";
static char _cursor_string[] = "_cursor";
static char _decodeRLE_string[] = "_decodeRLE";
static char _decompress_lzss_string[] = "_decompress_lzss";
static char _delay_string[] = "_delay";
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
static char _drawColorRectangle_string[] = "_drawColorRectangle";
static char _drawPreview_string[] = "_drawPreview";
static char _dump_pci_dt_string[] = "_dump_pci_dt";
static char _dyld_stub_binder_string[] = "_dyld_stub_binder";
static char _ebiosEjectMedia_string[] = "_ebiosEjectMedia";
static char _ebiosread_string[] = "_ebiosread";
static char _ebioswrite_string[] = "_ebioswrite";
static char _efi_guid_compare_string[] = "_efi_guid_compare";
static char _efi_guid_is_null_string[] = "_efi_guid_is_null";
static char _efi_guid_unparse_upper_string[] = "_efi_guid_unparse_upper";
static char _efi_inject_get_devprop_string_string[] = "_efi_inject_get_devprop_string";
static char _enableA20_string[] = "_enableA20";
static char _enable_pci_devs_string[] = "_enable_pci_devs";
static char _error_string[] = "_error";
static char _execute_hook_string[] = "_execute_hook";
static char _file_size_string[] = "_file_size";
static char _finalizeBootStruct_string[] = "_finalizeBootStruct";
static char _fix_restart_string[] = "_fix_restart";
static char _flushKeyboardBuffer_string[] = "_flushKeyboardBuffer";
static char _free_string[] = "_free";
static char _freeFilteredBVChain_string[] = "_freeFilteredBVChain";
static char _gBIOSBootVolume_string[] = "_gBIOSBootVolume";
static char _gBIOSDev_string[] = "_gBIOSDev";
static char _gBinaryAddress_string[] = "_gBinaryAddress";
static char _gBootArgs_string[] = "_gBootArgs";
static char _gBootArgsEnd_string[] = "_gBootArgsEnd";
static char _gBootArgsPtr_string[] = "_gBootArgsPtr";
static char _gBootFileType_string[] = "_gBootFileType";
static char _gBootMode_string[] = "_gBootMode";
static char _gBootOrder_string[] = "_gBootOrder";
static char _gBootVolume_string[] = "_gBootVolume";
static char _gCompareTable_string[] = "_gCompareTable";
static char _gCompareTableCompressed_string[] = "_gCompareTableCompressed";
static char _gDeviceCount_string[] = "_gDeviceCount";
static char _gDriverSpec_string[] = "_gDriverSpec";
static char _gDualLink_string[] = "_gDualLink";
static char _gEfiAcpi20TableGuid_string[] = "_gEfiAcpi20TableGuid";
static char _gEfiAcpiTableGuid_string[] = "_gEfiAcpiTableGuid";
static char _gEfiConfigurationTableNode_string[] = "_gEfiConfigurationTableNode";
static char _gEfiSmbiosTableGuid_string[] = "_gEfiSmbiosTableGuid";
static char _gEnableCDROMRescan_string[] = "_gEnableCDROMRescan";
static char _gErrors_string[] = "_gErrors";
static char _gExtensionsSpec_string[] = "_gExtensionsSpec";
static char _gFSLoadAddress_string[] = "_gFSLoadAddress";
static char _gFileName_string[] = "_gFileName";
static char _gFileSpec_string[] = "_gFileSpec";
static char _gHaveKernelCache_string[] = "_gHaveKernelCache";
static char _gLowerCaseTable_string[] = "_gLowerCaseTable";
static char _gLowerCaseTableCompressed_string[] = "_gLowerCaseTableCompressed";
static char _gMI_string[] = "_gMI";
static char _gMKextName_string[] = "_gMKextName";
static char _gMacOSVersion_string[] = "_gMacOSVersion";
static char _gMemoryMapNode_string[] = "_gMemoryMapNode";
static char _gMenuBottom_string[] = "_gMenuBottom";
static char _gMenuEnd_string[] = "_gMenuEnd";
static char _gMenuHeight_string[] = "_gMenuHeight";
static char _gMenuItemCount_string[] = "_gMenuItemCount";
static char _gMenuItems_string[] = "_gMenuItems";
static char _gMenuRow_string[] = "_gMenuRow";
static char _gMenuSelection_string[] = "_gMenuSelection";
static char _gMenuStart_string[] = "_gMenuStart";
static char _gMenuTop_string[] = "_gMenuTop";
static char _gOverrideKernel_string[] = "_gOverrideKernel";
static char _gPlatform_string[] = "_gPlatform";
static char _gPlatformName_string[] = "_gPlatformName";
static char _gRAMDiskBTAliased_string[] = "_gRAMDiskBTAliased";
static char _gRAMDiskFile_string[] = "_gRAMDiskFile";
static char _gRAMDiskMI_string[] = "_gRAMDiskMI";
static char _gRAMDiskVolume_string[] = "_gRAMDiskVolume";
static char _gRootDevice_string[] = "_gRootDevice";
static char _gRootPCIDev_string[] = "_gRootPCIDev";
static char _gSMBIOS_string[] = "_gSMBIOS";
static char _gSMBIOSBoardModel_string[] = "_gSMBIOSBoardModel";
static char _gST32_string[] = "_gST32";
static char _gST64_string[] = "_gST64";
static char _gScanSingleDrive_string[] = "_gScanSingleDrive";
static char _gTempSpec_string[] = "_gTempSpec";
static char _gVerboseMode_string[] = "_gVerboseMode";
static char _generate_cst_ssdt_string[] = "_generate_cst_ssdt";
static char _generate_pss_ssdt_string[] = "_generate_pss_ssdt";
static char _getBVChainForBIOSDev_string[] = "_getBVChainForBIOSDev";
static char _getBoolForKey_string[] = "_getBoolForKey";
static char _getBootOptions_string[] = "_getBootOptions";
static char _getBootVolumeDescription_string[] = "_getBootVolumeDescription";
static char _getBootVolumeRef_string[] = "_getBootVolumeRef";
static char _getColorForKey_string[] = "_getColorForKey";
static char _getConventionalMemorySize_string[] = "_getConventionalMemorySize";
static char _getCursorPositionAndType_string[] = "_getCursorPositionAndType";
static char _getDeviceDescription_string[] = "_getDeviceDescription";
static char _getDimensionForKey_string[] = "_getDimensionForKey";
static char _getEDID_string[] = "_getEDID";
static char _getExtendedMemorySize_string[] = "_getExtendedMemorySize";
static char _getIntForKey_string[] = "_getIntForKey";
static char _getMemoryInfoString_string[] = "_getMemoryInfoString";
static char _getMemoryMap_string[] = "_getMemoryMap";
static char _getNextArg_string[] = "_getNextArg";
static char _getNumberArrayFromProperty_string[] = "_getNumberArrayFromProperty";
static char _getPciRootUID_string[] = "_getPciRootUID";
static char _getPlatformName_string[] = "_getPlatformName";
static char _getSmbios_string[] = "_getSmbios";
static char _getSmbiosMacModel_string[] = "_getSmbiosMacModel";
static char _getSmbiosProductName_string[] = "_getSmbiosProductName";
static char _getStringForKey_string[] = "_getStringForKey";
static char _getStringFromUUID_string[] = "_getStringFromUUID";
static char _getSystemID_string[] = "_getSystemID";
static char _getUUIDFromString_string[] = "_getUUIDFromString";
static char _getVBEInfo_string[] = "_getVBEInfo";
static char _getVBEModeInfo_string[] = "_getVBEModeInfo";
static char _getVESAModeWithProperties_string[] = "_getVESAModeWithProperties";
static char _getValueForBootKey_string[] = "_getValueForBootKey";
static char _getValueForConfigTableKey_string[] = "_getValueForConfigTableKey";
static char _getValueForKey_string[] = "_getValueForKey";
static char _getVideoMode_string[] = "_getVideoMode";
static char _getVolumeLabelAlias_string[] = "_getVolumeLabelAlias";
static char _get_acpi_cpu_names_string[] = "_get_acpi_cpu_names";
static char _get_drive_info_string[] = "_get_drive_info";
static char _get_pci_dev_path_string[] = "_get_pci_dev_path";
static char _getc_string[] = "_getc";
static char _getchar_string[] = "_getchar";
static char _halt_string[] = "_halt";
static char _handle_symtable_string[] = "_handle_symtable";
static char _hi_multiboot_string[] = "_hi_multiboot";
static char _hook_exists_string[] = "_hook_exists";
static char _initBooterLog_string[] = "_initBooterLog";
static char _initGraphicsMode_string[] = "_initGraphicsMode";
static char _initKernBootStruct_string[] = "_initKernBootStruct";
static char _init_module_system_string[] = "_init_module_system";
static char _initialize_runtime_string[] = "_initialize_runtime";
static char _is_module_loaded_string[] = "_is_module_loaded";
static char _is_no_emulation_string[] = "_is_no_emulation";
static char _jump_to_chainbooter_string[] = "_jump_to_chainbooter";
static char _loadACPITable_string[] = "_loadACPITable";
static char _loadConfigFile_string[] = "_loadConfigFile";
static char _loadHelperConfig_string[] = "_loadHelperConfig";
static char _loadImageScale_string[] = "_loadImageScale";
static char _loadOverrideConfig_string[] = "_loadOverrideConfig";
static char _loadPrebootRAMDisk_string[] = "_loadPrebootRAMDisk";
static char _loadSystemConfig_string[] = "_loadSystemConfig";
static char _load_all_modules_string[] = "_load_all_modules";
static char _load_module_string[] = "_load_module";
static char _loadedModules_string[] = "_loadedModules";
static char _lookUpCLUTIndex_string[] = "_lookUpCLUTIndex";
static char _lookup_all_symbols_string[] = "_lookup_all_symbols";
static char _lookup_symbol_string[] = "_lookup_symbol";
static char _lspci_string[] = "_lspci";
static char _malloc_init_string[] = "_malloc_init";
static char _matchVolumeToString_string[] = "_matchVolumeToString";
static char _md0Ramdisk_string[] = "_md0Ramdisk";
static char _memcmp_string[] = "_memcmp";
static char _memcpy_string[] = "_memcpy";
static char _memset_string[] = "_memset";
static char _menuBVR_string[] = "_menuBVR";
static char _menuItems_string[] = "_menuItems";
static char _moduleCallbacks_string[] = "_moduleCallbacks";
static char _moduleSymbols_string[] = "_moduleSymbols";
static char _module_loaded_string[] = "_module_loaded";
static char _mountRAMDisk_string[] = "_mountRAMDisk";
static char _moveCursor_string[] = "_moveCursor";
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
static char _newFDiskBVRef_string[] = "_newFDiskBVRef";
static char _newFilteredBVChain_string[] = "_newFilteredBVChain";
static char _newGPTBVRef_string[] = "_newGPTBVRef";
static char _newString_string[] = "_newString";
static char _newStringForKey_string[] = "_newStringForKey";
static char _new_dsdt_string[] = "_new_dsdt";
static char _open_string[] = "_open";
static char _open_bvdev_string[] = "_open_bvdev";
static char _opendir_string[] = "_opendir";
static char _openmem_string[] = "_openmem";
static char _p_get_ramdisk_info_string[] = "_p_get_ramdisk_info";
static char _p_ramdiskReadBytes_string[] = "_p_ramdiskReadBytes";
static char _parse_mach_string[] = "_parse_mach";
static char _patch_fadt_string[] = "_patch_fadt";
static char _pause_string[] = "_pause";
static char _pci_config_read16_string[] = "_pci_config_read16";
static char _pci_config_read32_string[] = "_pci_config_read32";
static char _pci_config_read8_string[] = "_pci_config_read8";
static char _pci_config_write16_string[] = "_pci_config_write16";
static char _pci_config_write32_string[] = "_pci_config_write32";
static char _pci_config_write8_string[] = "_pci_config_write8";
static char _platformCPUFeature_string[] = "_platformCPUFeature";
static char _previewLoadedSectors_string[] = "_previewLoadedSectors";
static char _previewSaveunder_string[] = "_previewSaveunder";
static char _previewTotalSectors_string[] = "_previewTotalSectors";
static char _prf_string[] = "_prf";
static char _printMemoryInfo_string[] = "_printMemoryInfo";
static char _printMenuItem_string[] = "_printMenuItem";
static char _printf_string[] = "_printf";
static char _processBootArgument_string[] = "_processBootArgument";
static char _processBootOptions_string[] = "_processBootOptions";
static char _processRAMDiskCommand_string[] = "_processRAMDiskCommand";
static char _promptForRescanOption_string[] = "_promptForRescanOption";
static char _ptol_string[] = "_ptol";
static char _putc_string[] = "_putc";
static char _putca_string[] = "_putca";
static char _putchar_string[] = "_putchar";
static char _rawDiskRead_string[] = "_rawDiskRead";
static char _rawDiskWrite_string[] = "_rawDiskWrite";
static char _read_string[] = "_read";
static char _readBootSector_string[] = "_readBootSector";
static char _readKeyboardShiftFlags_string[] = "_readKeyboardShiftFlags";
static char _readKeyboardStatus_string[] = "_readKeyboardStatus";
static char _readdir_string[] = "_readdir";
static char _readdir_ext_string[] = "_readdir_ext";
static char _realloc_string[] = "_realloc";
static char _rebase_location_string[] = "_rebase_location";
static char _rebase_macho_string[] = "_rebase_macho";
static char _ref_strings_string[] = "_ref_strings";
static char _register_hook_callback_string[] = "_register_hook_callback";
static char _replace_function_string[] = "_replace_function";
static char _rescanBIOSDevice_string[] = "_rescanBIOSDevice";
static char _reserveKernBootStruct_string[] = "_reserveKernBootStruct";
static char _restoreCursor_string[] = "_restoreCursor";
static char _root_pci_dev_string[] = "_root_pci_dev";
static char _rsdplength_string[] = "_rsdplength";
static char _safe_malloc_string[] = "_safe_malloc";
static char _scanBootVolumes_string[] = "_scanBootVolumes";
static char _scanDMI_string[] = "_scanDMI";
static char _scanDisks_string[] = "_scanDisks";
static char _scan_cpu_string[] = "_scan_cpu";
static char _scan_cpu_DMI_string[] = "_scan_cpu_DMI";
static char _scan_mem_string[] = "_scan_mem";
static char _scan_pci_bus_string[] = "_scan_pci_bus";
static char _scan_platform_string[] = "_scan_platform";
static char _scollPage_string[] = "_scollPage";
static char _search_and_get_acpi_fd_string[] = "_search_and_get_acpi_fd";
static char _selectBootVolume_string[] = "_selectBootVolume";
static char _selectIndex_string[] = "_selectIndex";
static char _setActiveDisplayPage_string[] = "_setActiveDisplayPage";
static char _setBootGlobals_string[] = "_setBootGlobals";
static char _setCursorPosition_string[] = "_setCursorPosition";
static char _setCursorType_string[] = "_setCursorType";
static char _setRAMDiskBTHook_string[] = "_setRAMDiskBTHook";
static char _setRootVolume_string[] = "_setRootVolume";
static char _setVBEMode_string[] = "_setVBEMode";
static char _setVBEPalette_string[] = "_setVBEPalette";
static char _setVESAGraphicsMode_string[] = "_setVESAGraphicsMode";
static char _setVideoMode_string[] = "_setVideoMode";
static char _set_eth_builtin_string[] = "_set_eth_builtin";
static char _setupAcpi_string[] = "_setupAcpi";
static char _setupAcpiNoMod_string[] = "_setupAcpiNoMod";
static char _setupBooterLog_string[] = "_setupBooterLog";
static char _setupDeviceProperties_string[] = "_setupDeviceProperties";
static char _setupEfiDeviceTree_string[] = "_setupEfiDeviceTree";
static char _setupEfiTables32_string[] = "_setupEfiTables32";
static char _setupEfiTables64_string[] = "_setupEfiTables64";
static char _setupFakeEfi_string[] = "_setupFakeEfi";
static char _setupSystemType_string[] = "_setupSystemType";
static char _setup_pci_devs_string[] = "_setup_pci_devs";
static char _shouldboot_string[] = "_shouldboot";
static char _showHelp_string[] = "_showHelp";
static char _showInfoRAMDisk_string[] = "_showInfoRAMDisk";
static char _showTextBuffer_string[] = "_showTextBuffer";
static char _showTextFile_string[] = "_showTextFile";
static char _sleep_string[] = "_sleep";
static char _slvprintf_string[] = "_slvprintf";
static char _smbiosStringAtIndex_string[] = "_smbiosStringAtIndex";
static char _smbios_p_string[] = "_smbios_p";
static char _smbios_properties_string[] = "_smbios_properties";
static char _smbios_table_descriptions_string[] = "_smbios_table_descriptions";
static char _spinActivityIndicator_string[] = "_spinActivityIndicator";
static char _sprintf_string[] = "_sprintf";
static char _sputc_string[] = "_sputc";
static char _st_string[] = "_st";
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
static char _symbols_module_start_string[] = "_symbols_module_start";
static char _sysConfigValid_string[] = "_sysConfigValid";
static char _systemConfigDir_string[] = "_systemConfigDir";
static char _systemVersion_string[] = "_systemVersion";
static char _tableSign_string[] = "_tableSign";
static char _tell_string[] = "_tell";
static char _testBiosread_string[] = "_testBiosread";
static char _testFAT32EFIBootSector_string[] = "_testFAT32EFIBootSector";
static char _textAddress_string[] = "_textAddress";
static char _textSection_string[] = "_textSection";
static char _time18_string[] = "_time18";
static char _umountRAMDisk_string[] = "_umountRAMDisk";
static char _updateProgressBar_string[] = "_updateProgressBar";
static char _utf_decodestr_string[] = "_utf_decodestr";
static char _utf_encodestr_string[] = "_utf_encodestr";
static char _verbose_string[] = "_verbose";
static char _video_mode_string[] = "_video_mode";
static char _vol_opendir_string[] = "_vol_opendir";
static char _waitThenReload_string[] = "_waitThenReload";
static char _write_string[] = "_write";
static char _writebyte_string[] = "_writebyte";
static char _writeint_string[] = "_writeint";
static char boot2_string[] = "boot2";
static char dyld_stub_binder_string[] = "dyld_stub_binder";
symbol_t symbolList[] = {
	{.symbol = _Adler32_string, .addr = 0x00021edc},
	{.symbol = _AllocateKernelMemory_string, .addr = 0x0002c0a9},
	{.symbol = _AllocateMemoryRange_string, .addr = 0x0002c121},
	{.symbol = _BinaryUnicodeCompare_string, .addr = 0x00036917},
	{.symbol = _BootHelp_txt_string, .addr = 0x0003d214},
	{.symbol = _BootHelp_txt_len_string, .addr = 0x0003e804},
	{.symbol = _BootOrder_string, .addr = 0x000460f8},
	{.symbol = _CacheInit_string, .addr = 0x0002d6dd},
	{.symbol = _CacheRead_string, .addr = 0x0002d59b},
	{.symbol = _CacheReset_string, .addr = 0x0002d58c},
	{.symbol = _CreateUUIDString_string, .addr = 0x0002719e},
	{.symbol = _DT__AddChild_string, .addr = 0x0002d8d1},
	{.symbol = _DT__AddProperty_string, .addr = 0x0002d80a},
	{.symbol = _DT__Finalize_string, .addr = 0x0002daed},
	{.symbol = _DT__FindNode_string, .addr = 0x0002db76},
	{.symbol = _DT__FlattenDeviceTree_string, .addr = 0x0002da80},
	{.symbol = _DT__FreeNode_string, .addr = 0x0002d7c1},
	{.symbol = _DT__FreeProperty_string, .addr = 0x0002d7ab},
	{.symbol = _DT__GetName_string, .addr = 0x0002d7d7},
	{.symbol = _DT__Initialize_string, .addr = 0x0002d999},
	{.symbol = _DecodeKernel_string, .addr = 0x0002280d},
	{.symbol = _DecodeMachO_string, .addr = 0x000277cb},
	{.symbol = _DecompressData_string, .addr = 0x00025424},
	{.symbol = _EX2GetDescription_string, .addr = 0x0002e52b},
	{.symbol = _EX2Probe_string, .addr = 0x0002e512},
	{.symbol = _ExecKernel_string, .addr = 0x00020540},
	{.symbol = _FastRelString_string, .addr = 0x00036a3a},
	{.symbol = _FastUnicodeCompare_string, .addr = 0x00036aba},
	{.symbol = _FileLoadDrivers_string, .addr = 0x000225fc},
	{.symbol = _FindFirstDmiTableOfType_string, .addr = 0x00033661},
	{.symbol = _FindNextDmiTableOfType_string, .addr = 0x00033535},
	{.symbol = _FourChar_string, .addr = 0x00033dc4},
	{.symbol = _GPT_BASICDATA2_GUID_string, .addr = 0x0003c22c},
	{.symbol = _GPT_BASICDATA_GUID_string, .addr = 0x0003c21c},
	{.symbol = _GPT_BOOT_GUID_string, .addr = 0x0003c1fc},
	{.symbol = _GPT_EFISYS_GUID_string, .addr = 0x0003c20c},
	{.symbol = _GPT_HFS_GUID_string, .addr = 0x0003c1ec},
	{.symbol = _Gdt_string, .addr = 0x000204bc},
	{.symbol = _Gdtr_string, .addr = 0x000204f4},
	{.symbol = _GetDirEntry_string, .addr = 0x00026ecb},
	{.symbol = _GetFileBlock_string, .addr = 0x00026e8d},
	{.symbol = _GetFileInfo_string, .addr = 0x000270d6},
	{.symbol = _GetRefString_string, .addr = 0x0002c18f},
	{.symbol = _HFSFree_string, .addr = 0x0002e795},
	{.symbol = _HFSGetDescription_string, .addr = 0x0002fa99},
	{.symbol = _HFSGetDirEntry_string, .addr = 0x0002f811},
	{.symbol = _HFSGetFileBlock_string, .addr = 0x0002fb42},
	{.symbol = _HFSGetUUID_string, .addr = 0x0002f7dc},
	{.symbol = _HFSInitPartition_string, .addr = 0x0002f458},
	{.symbol = _HFSLoadFile_string, .addr = 0x0002fa4b},
	{.symbol = _HFSProbe_string, .addr = 0x0002fa69},
	{.symbol = _HFSReadFile_string, .addr = 0x0002f8f9},
	{.symbol = _HibernateBoot_string, .addr = 0x00025037},
	{.symbol = _Idtr_prot_string, .addr = 0x00020504},
	{.symbol = _Idtr_real_string, .addr = 0x000204fc},
	{.symbol = _InitDriverSupport_string, .addr = 0x0002275f},
	{.symbol = _LoadDriverMKext_string, .addr = 0x00022148},
	{.symbol = _LoadDriverPList_string, .addr = 0x00022260},
	{.symbol = _LoadDrivers_string, .addr = 0x00022929},
	{.symbol = _LoadExtraDrivers_p_string, .addr = 0x000452a8},
	{.symbol = _LoadFile_string, .addr = 0x00027797},
	{.symbol = _LoadMatchedModules_string, .addr = 0x00021fe8},
	{.symbol = _LoadThinFatFile_string, .addr = 0x00027282},
	{.symbol = _LoadVolumeFile_string, .addr = 0x00026a52},
	{.symbol = _MD5Final_string, .addr = 0x0002e4e8},
	{.symbol = _MD5Init_string, .addr = 0x0002dc65},
	{.symbol = _MD5Pad_string, .addr = 0x0002e492},
	{.symbol = _MD5Update_string, .addr = 0x0002e3d6},
	{.symbol = _MSDOSFree_string, .addr = 0x0002fc19},
	{.symbol = _MSDOSGetDescription_string, .addr = 0x00030ddf},
	{.symbol = _MSDOSGetDirEntry_string, .addr = 0x0003096a},
	{.symbol = _MSDOSGetFileBlock_string, .addr = 0x00030c64},
	{.symbol = _MSDOSGetUUID_string, .addr = 0x00030612},
	{.symbol = _MSDOSInitPartition_string, .addr = 0x00030439},
	{.symbol = _MSDOSLoadFile_string, .addr = 0x000308ad},
	{.symbol = _MSDOSProbe_string, .addr = 0x000308cb},
	{.symbol = _MSDOSReadFile_string, .addr = 0x000306a0},
	{.symbol = _MacModel_string, .addr = 0x0003e940},
	{.symbol = _MatchLibraries_string, .addr = 0x00021f4e},
	{.symbol = _MatchPersonalities_string, .addr = 0x00021f47},
	{.symbol = _ModelLength_string, .addr = 0x0003e93c},
	{.symbol = _ModelRev_string, .addr = 0x0003e948},
	{.symbol = _NTFSGetDescription_string, .addr = 0x0003102a},
	{.symbol = _NTFSGetUUID_string, .addr = 0x00030fa2},
	{.symbol = _NTFSProbe_string, .addr = 0x00030f7f},
	{.symbol = _NetLoadDrivers_string, .addr = 0x00022208},
	{.symbol = _ParseXMLFile_string, .addr = 0x00029ead},
	{.symbol = _Platform_string, .addr = 0x000460fc},
	{.symbol = _ReadFileAtOffset_string, .addr = 0x00026f06},
	{.symbol = _ReadPCIBusInfo_string, .addr = 0x0002b4aa},
	{.symbol = _Round_string, .addr = 0x0002bef8},
	{.symbol = _SYSTEM_ID_DEFAULT_string, .addr = 0x0003c38c},
	{.symbol = _SaveRefString_string, .addr = 0x0002c7dd},
	{.symbol = _ThinFatFile_string, .addr = 0x00027ac5},
	{.symbol = _XMLCastArray_string, .addr = 0x0002c256},
	{.symbol = _XMLCastBoolean_string, .addr = 0x0002c2c3},
	{.symbol = _XMLCastDict_string, .addr = 0x0002c26f},
	{.symbol = _XMLCastInteger_string, .addr = 0x0002c2d9},
	{.symbol = _XMLCastString_string, .addr = 0x0002c288},
	{.symbol = _XMLCastStringOffset_string, .addr = 0x0002c2a5},
	{.symbol = _XMLDecode_string, .addr = 0x0002c724},
	{.symbol = _XMLFreeTag_string, .addr = 0x0002c2fe},
	{.symbol = _XMLGetElement_string, .addr = 0x0002c1b3},
	{.symbol = _XMLGetProperty_string, .addr = 0x0002c401},
	{.symbol = _XMLIsType_string, .addr = 0x0002c23b},
	{.symbol = _XMLParseFile_string, .addr = 0x0002d134},
	{.symbol = _XMLParseNextTag_string, .addr = 0x0002c9e1},
	{.symbol = _XMLTagCount_string, .addr = 0x0002c448},
	{.symbol = __DATA__bss__begin_string, .addr = 0x000430d0},
	{.symbol = __DATA__bss__end_string, .addr = 0x00043e3c},
	{.symbol = __DATA__common__begin_string, .addr = 0x00043e40},
	{.symbol = __DATA__common__end_string, .addr = 0x000461cc},
	{.symbol = __bp_string, .addr = 0x000203aa},
	{.symbol = __hi_malloc_string, .addr = 0x00024430},
	{.symbol = __hi_strdup_string, .addr = 0x00024441},
	{.symbol = __prot_to_real_string, .addr = 0x0002032d},
	{.symbol = __real_to_prot_string, .addr = 0x000202df},
	{.symbol = __sp_string, .addr = 0x000203a7},
	{.symbol = __switch_stack_string, .addr = 0x000203ad},
	{.symbol = _acpi10_p_string, .addr = 0x00046110},
	{.symbol = _acpi20_p_string, .addr = 0x00046118},
	{.symbol = _acpi_cpu_count_string, .addr = 0x0003ec38},
	{.symbol = _acpi_cpu_name_string, .addr = 0x00046120},
	{.symbol = _addBootArg_string, .addr = 0x00023736},
	{.symbol = _addConfigurationTable_string, .addr = 0x0002b0cc},
	{.symbol = _add_symbol_string, .addr = 0x00025f74},
	{.symbol = _aml_add_alias_string, .addr = 0x000373cc},
	{.symbol = _aml_add_buffer_string, .addr = 0x000371ec},
	{.symbol = _aml_add_byte_string, .addr = 0x000371aa},
	{.symbol = _aml_add_dword_string, .addr = 0x00037103},
	{.symbol = _aml_add_name_string, .addr = 0x00037340},
	{.symbol = _aml_add_package_string, .addr = 0x00037028},
	{.symbol = _aml_add_qword_string, .addr = 0x00037063},
	{.symbol = _aml_add_scope_string, .addr = 0x00037368},
	{.symbol = _aml_add_to_parent_string, .addr = 0x00036f6f},
	{.symbol = _aml_add_word_string, .addr = 0x00037161},
	{.symbol = _aml_calculate_size_string, .addr = 0x00036d45},
	{.symbol = _aml_create_node_string, .addr = 0x00037004},
	{.symbol = _aml_destroy_node_string, .addr = 0x0003741f},
	{.symbol = _aml_fill_name_string, .addr = 0x00037231},
	{.symbol = _aml_fill_simple_name_string, .addr = 0x00037390},
	{.symbol = _aml_get_size_length_string, .addr = 0x00036d15},
	{.symbol = _aml_write_buffer_string, .addr = 0x00036fd9},
	{.symbol = _aml_write_byte_string, .addr = 0x00036e1c},
	{.symbol = _aml_write_dword_string, .addr = 0x00036e49},
	{.symbol = _aml_write_node_string, .addr = 0x00037468},
	{.symbol = _aml_write_qword_string, .addr = 0x00036e72},
	{.symbol = _aml_write_size_string, .addr = 0x00036ed8},
	{.symbol = _aml_write_word_string, .addr = 0x00036e2e},
	{.symbol = _archCpuType_string, .addr = 0x0003e8a8},
	{.symbol = _ascii_hex_to_int_string, .addr = 0x00033a93},
	{.symbol = _atoi_string, .addr = 0x00037724},
	{.symbol = _b_lseek_string, .addr = 0x0002736e},
	{.symbol = _bcopy_string, .addr = 0x000375ce},
	{.symbol = _bgetc_string, .addr = 0x0002be25},
	{.symbol = _bind_location_string, .addr = 0x00025e0f},
	{.symbol = _bind_macho_string, .addr = 0x00025ff9},
	{.symbol = _bios_string, .addr = 0x000203dd},
	{.symbol = _biosDevIsCDROM_string, .addr = 0x00027c4e},
	{.symbol = _biosread_string, .addr = 0x0002b8d6},
	{.symbol = _boot_string, .addr = 0x00020fd3},
	{.symbol = _bootArgs_string, .addr = 0x00043e40},
	{.symbol = _bootBanner_string, .addr = 0x0003cfd8},
	{.symbol = _bootInfo_string, .addr = 0x00043e44},
	{.symbol = _bootPrompt_string, .addr = 0x0003d03d},
	{.symbol = _bootRescanPrompt_string, .addr = 0x0003d105},
	{.symbol = _booterCommand_string, .addr = 0x000452c0},
	{.symbol = _booterParam_string, .addr = 0x000456c0},
	{.symbol = _build_pci_dt_string, .addr = 0x0002d514},
	{.symbol = _builtin_set_string, .addr = 0x0003ec40},
	{.symbol = _bvChain_string, .addr = 0x00043e48},
	{.symbol = _bvCount_string, .addr = 0x0003cfc4},
	{.symbol = _bvr_string, .addr = 0x00043e4c},
	{.symbol = _bzero_string, .addr = 0x000375ef},
	{.symbol = _chainLoad_string, .addr = 0x000249c4},
	{.symbol = _chainbootdev_string, .addr = 0x000204b0},
	{.symbol = _chainbootflag_string, .addr = 0x000204b1},
	{.symbol = _changeCursor_string, .addr = 0x000237d6},
	{.symbol = _checksum8_string, .addr = 0x000377a4},
	{.symbol = _clearActivityIndicator_string, .addr = 0x00021af5},
	{.symbol = _clearBootArgs_string, .addr = 0x0002378e},
	{.symbol = _clearScreenRows_string, .addr = 0x0002b59a},
	{.symbol = _close_string, .addr = 0x00026a68},
	{.symbol = _closedir_string, .addr = 0x00026ca0},
	{.symbol = _common_boot_string, .addr = 0x000206f3},
	{.symbol = _continue_at_low_address_string, .addr = 0x000202b4},
	{.symbol = _convertHexStr2Binary_string, .addr = 0x00033aee},
	{.symbol = _copyArgument_string, .addr = 0x00022d59},
	{.symbol = _copyMultibootInfo_string, .addr = 0x00024527},
	{.symbol = _crc32_string, .addr = 0x0003834f},
	{.symbol = _cursor_string, .addr = 0x0003e8c8},
	{.symbol = _decodeRLE_string, .addr = 0x00021433},
	{.symbol = _decompress_lzss_string, .addr = 0x00024290},
	{.symbol = _delay_string, .addr = 0x0002b474},
	{.symbol = _determine_safe_hi_addr_string, .addr = 0x000243a1},
	{.symbol = _devices_number_string, .addr = 0x0003ec3c},
	{.symbol = _devprop_add_device_string, .addr = 0x000363ec},
	{.symbol = _devprop_add_network_template_string, .addr = 0x00036310},
	{.symbol = _devprop_add_value_string, .addr = 0x000361a3},
	{.symbol = _devprop_create_string_string, .addr = 0x0003639f},
	{.symbol = _devprop_free_string_string, .addr = 0x00036130},
	{.symbol = _devprop_generate_string_string, .addr = 0x00035f39},
	{.symbol = _diskFreeMap_string, .addr = 0x00027cc5},
	{.symbol = _diskIsCDROM_string, .addr = 0x00027c72},
	{.symbol = _diskRead_string, .addr = 0x00028982},
	{.symbol = _diskResetBootVolumes_string, .addr = 0x0002818b},
	{.symbol = _diskScanBootVolumes_string, .addr = 0x00028ce8},
	{.symbol = _diskSeek_string, .addr = 0x00027b7f},
	{.symbol = _drawColorRectangle_string, .addr = 0x00021b18},
	{.symbol = _drawPreview_string, .addr = 0x00021c62},
	{.symbol = _dump_pci_dt_string, .addr = 0x0002d322},
	{.symbol = _dyld_stub_binder_string, .addr = 0x00025e36},
	{.symbol = _ebiosEjectMedia_string, .addr = 0x0002b665},
	{.symbol = _ebiosread_string, .addr = 0x0002b803},
	{.symbol = _ebioswrite_string, .addr = 0x0002b712},
	{.symbol = _efi_guid_compare_string, .addr = 0x000383af},
	{.symbol = _efi_guid_is_null_string, .addr = 0x00038380},
	{.symbol = _efi_guid_unparse_upper_string, .addr = 0x00038414},
	{.symbol = _efi_inject_get_devprop_string_string, .addr = 0x0003610b},
	{.symbol = _enableA20_string, .addr = 0x00029a25},
	{.symbol = _enable_pci_devs_string, .addr = 0x0002d256},
	{.symbol = _error_string, .addr = 0x0002a609},
	{.symbol = _execute_hook_string, .addr = 0x0002678a},
	{.symbol = _file_size_string, .addr = 0x0002739a},
	{.symbol = _finalizeBootStruct_string, .addr = 0x00029a8f},
	{.symbol = _fix_restart_string, .addr = 0x000461a0},
	{.symbol = _flushKeyboardBuffer_string, .addr = 0x0002342e},
	{.symbol = _free_string, .addr = 0x00037aa6},
	{.symbol = _freeFilteredBVChain_string, .addr = 0x00027c99},
	{.symbol = _gBIOSBootVolume_string, .addr = 0x0003e868},
	{.symbol = _gBIOSDev_string, .addr = 0x00043e50},
	{.symbol = _gBinaryAddress_string, .addr = 0x000460f0},
	{.symbol = _gBootArgs_string, .addr = 0x00045ac0},
	{.symbol = _gBootArgsEnd_string, .addr = 0x0003e818},
	{.symbol = _gBootArgsPtr_string, .addr = 0x0003e814},
	{.symbol = _gBootFileType_string, .addr = 0x00043e54},
	{.symbol = _gBootMode_string, .addr = 0x00043e58},
	{.symbol = _gBootOrder_string, .addr = 0x00043e5c},
	{.symbol = _gBootVolume_string, .addr = 0x00043e60},
	{.symbol = _gCompareTable_string, .addr = 0x000461c4},
	{.symbol = _gCompareTableCompressed_string, .addr = 0x0003ef40},
	{.symbol = _gDeviceCount_string, .addr = 0x0003cfc8},
	{.symbol = _gDriverSpec_string, .addr = 0x000452ac},
	{.symbol = _gDualLink_string, .addr = 0x00043e64},
	{.symbol = _gEfiAcpi20TableGuid_string, .addr = 0x0003e8e8},
	{.symbol = _gEfiAcpiTableGuid_string, .addr = 0x0003e8d8},
	{.symbol = _gEfiConfigurationTableNode_string, .addr = 0x0003e8d4},
	{.symbol = _gEfiSmbiosTableGuid_string, .addr = 0x0003c39c},
	{.symbol = _gEnableCDROMRescan_string, .addr = 0x00043e68},
	{.symbol = _gErrors_string, .addr = 0x00043e69},
	{.symbol = _gExtensionsSpec_string, .addr = 0x000452b0},
	{.symbol = _gFSLoadAddress_string, .addr = 0x0003e864},
	{.symbol = _gFileName_string, .addr = 0x000452b4},
	{.symbol = _gFileSpec_string, .addr = 0x000452b8},
	{.symbol = _gHaveKernelCache_string, .addr = 0x00043e6a},
	{.symbol = _gLowerCaseTable_string, .addr = 0x000461c8},
	{.symbol = _gLowerCaseTableCompressed_string, .addr = 0x0003ec68},
	{.symbol = _gMI_string, .addr = 0x00045ee4},
	{.symbol = _gMKextName_string, .addr = 0x00043e70},
	{.symbol = _gMacOSVersion_string, .addr = 0x00044070},
	{.symbol = _gMemoryMapNode_string, .addr = 0x000460f4},
	{.symbol = _gMenuBottom_string, .addr = 0x00045ec0},
	{.symbol = _gMenuEnd_string, .addr = 0x00045ec4},
	{.symbol = _gMenuHeight_string, .addr = 0x00045ec8},
	{.symbol = _gMenuItemCount_string, .addr = 0x00045ecc},
	{.symbol = _gMenuItems_string, .addr = 0x0003e81c},
	{.symbol = _gMenuRow_string, .addr = 0x00045ed0},
	{.symbol = _gMenuSelection_string, .addr = 0x00045ed4},
	{.symbol = _gMenuStart_string, .addr = 0x00045ed8},
	{.symbol = _gMenuTop_string, .addr = 0x00045edc},
	{.symbol = _gOverrideKernel_string, .addr = 0x00044078},
	{.symbol = _gPlatform_string, .addr = 0x0004407c},
	{.symbol = _gPlatformName_string, .addr = 0x0003cfc0},
	{.symbol = _gRAMDiskBTAliased_string, .addr = 0x0003e838},
	{.symbol = _gRAMDiskFile_string, .addr = 0x00045ef0},
	{.symbol = _gRAMDiskMI_string, .addr = 0x0003e830},
	{.symbol = _gRAMDiskVolume_string, .addr = 0x0003e834},
	{.symbol = _gRootDevice_string, .addr = 0x00044080},
	{.symbol = _gRootPCIDev_string, .addr = 0x00044280},
	{.symbol = _gSMBIOS_string, .addr = 0x00044284},
	{.symbol = _gSMBIOSBoardModel_string, .addr = 0x00046108},
	{.symbol = _gST32_string, .addr = 0x0003e8cc},
	{.symbol = _gST64_string, .addr = 0x0003e8d0},
	{.symbol = _gScanSingleDrive_string, .addr = 0x00044288},
	{.symbol = _gTempSpec_string, .addr = 0x000452bc},
	{.symbol = _gVerboseMode_string, .addr = 0x00044289},
	{.symbol = _generate_cst_ssdt_string, .addr = 0x00034ec7},
	{.symbol = _generate_pss_ssdt_string, .addr = 0x0003429f},
	{.symbol = _getBVChainForBIOSDev_string, .addr = 0x00027b60},
	{.symbol = _getBoolForKey_string, .addr = 0x0002a509},
	{.symbol = _getBootOptions_string, .addr = 0x00023888},
	{.symbol = _getBootVolumeDescription_string, .addr = 0x00027ef3},
	{.symbol = _getBootVolumeRef_string, .addr = 0x00026d73},
	{.symbol = _getColorForKey_string, .addr = 0x0002a34a},
	{.symbol = _getConventionalMemorySize_string, .addr = 0x0002b9af},
	{.symbol = _getCursorPositionAndType_string, .addr = 0x0002b5bd},
	{.symbol = _getDeviceDescription_string, .addr = 0x00026b3a},
	{.symbol = _getDimensionForKey_string, .addr = 0x0002a393},
	{.symbol = _getEDID_string, .addr = 0x0002c041},
	{.symbol = _getExtendedMemorySize_string, .addr = 0x0002b9cf},
	{.symbol = _getIntForKey_string, .addr = 0x0002a478},
	{.symbol = _getMemoryInfoString_string, .addr = 0x00023454},
	{.symbol = _getMemoryMap_string, .addr = 0x0002bc92},
	{.symbol = _getNextArg_string, .addr = 0x00029e11},
	{.symbol = _getNumberArrayFromProperty_string, .addr = 0x000213bd},
	{.symbol = _getPciRootUID_string, .addr = 0x00036826},
	{.symbol = _getPlatformName_string, .addr = 0x00029a11},
	{.symbol = _getSmbios_string, .addr = 0x000325cd},
	{.symbol = _getSmbiosMacModel_string, .addr = 0x00032523},
	{.symbol = _getSmbiosProductName_string, .addr = 0x000339b9},
	{.symbol = _getStringForKey_string, .addr = 0x0002a556},
	{.symbol = _getStringFromUUID_string, .addr = 0x00033bbd},
	{.symbol = _getSystemID_string, .addr = 0x0002aa53},
	{.symbol = _getUUIDFromString_string, .addr = 0x00033c34},
	{.symbol = _getVBEInfo_string, .addr = 0x0002c000},
	{.symbol = _getVBEModeInfo_string, .addr = 0x0002bfb6},
	{.symbol = _getVESAModeWithProperties_string, .addr = 0x0002148c},
	{.symbol = _getValueForBootKey_string, .addr = 0x0002a162},
	{.symbol = _getValueForConfigTableKey_string, .addr = 0x0002a20b},
	{.symbol = _getValueForKey_string, .addr = 0x0002a268},
	{.symbol = _getVideoMode_string, .addr = 0x000213ad},
	{.symbol = _getVolumeLabelAlias_string, .addr = 0x00027e01},
	{.symbol = _get_acpi_cpu_names_string, .addr = 0x00033dfc},
	{.symbol = _get_drive_info_string, .addr = 0x0002bb55},
	{.symbol = _get_pci_dev_path_string, .addr = 0x0002d285},
	{.symbol = _getc_string, .addr = 0x0002a7bf},
	{.symbol = _getchar_string, .addr = 0x0002a88b},
	{.symbol = _halt_string, .addr = 0x00020383},
	{.symbol = _handle_symtable_string, .addr = 0x00025a15},
	{.symbol = _hi_multiboot_string, .addr = 0x000246db},
	{.symbol = _hook_exists_string, .addr = 0x00025e4a},
	{.symbol = _initBooterLog_string, .addr = 0x0002a858},
	{.symbol = _initGraphicsMode_string, .addr = 0x00021867},
	{.symbol = _initKernBootStruct_string, .addr = 0x00029c13},
	{.symbol = _init_module_system_string, .addr = 0x00026993},
	{.symbol = _initialize_runtime_string, .addr = 0x00020f91},
	{.symbol = _is_module_loaded_string, .addr = 0x00025ec7},
	{.symbol = _is_no_emulation_string, .addr = 0x0002bad6},
	{.symbol = _jump_to_chainbooter_string, .addr = 0x000202ca},
	{.symbol = _loadACPITable_string, .addr = 0x00035326},
	{.symbol = _loadConfigFile_string, .addr = 0x0002a11a},
	{.symbol = _loadHelperConfig_string, .addr = 0x00029f3a},
	{.symbol = _loadImageScale_string, .addr = 0x00021098},
	{.symbol = _loadOverrideConfig_string, .addr = 0x0002a01b},
	{.symbol = _loadPrebootRAMDisk_string, .addr = 0x00024e82},
	{.symbol = _loadSystemConfig_string, .addr = 0x0002a097},
	{.symbol = _load_all_modules_string, .addr = 0x00026911},
	{.symbol = _load_module_string, .addr = 0x000267df},
	{.symbol = _loadedModules_string, .addr = 0x0003e858},
	{.symbol = _lookUpCLUTIndex_string, .addr = 0x0002100c},
	{.symbol = _lookup_all_symbols_string, .addr = 0x00025e78},
	{.symbol = _lookup_symbol_string, .addr = 0x0003e860},
	{.symbol = _lspci_string, .addr = 0x0002362f},
	{.symbol = _malloc_init_string, .addr = 0x00037940},
	{.symbol = _matchVolumeToString_string, .addr = 0x00027cf7},
	{.symbol = _md0Ramdisk_string, .addr = 0x00024eab},
	{.symbol = _memcmp_string, .addr = 0x00037620},
	{.symbol = _memcpy_string, .addr = 0x000375ab},
	{.symbol = _memset_string, .addr = 0x00037597},
	{.symbol = _menuBVR_string, .addr = 0x0004428c},
	{.symbol = _menuItems_string, .addr = 0x0003e810},
	{.symbol = _moduleCallbacks_string, .addr = 0x0003e854},
	{.symbol = _moduleSymbols_string, .addr = 0x0003e85c},
	{.symbol = _module_loaded_string, .addr = 0x00025f3a},
	{.symbol = _mountRAMDisk_string, .addr = 0x00024b96},
	{.symbol = _moveCursor_string, .addr = 0x00022d44},
	{.symbol = _msgbuf_string, .addr = 0x0003e8c4},
	{.symbol = _msglog_string, .addr = 0x0002a75f},
	{.symbol = _multibootRamdiskReadBytes_string, .addr = 0x000244d0},
	{.symbol = _multiboot_get_ramdisk_info_string, .addr = 0x00024469},
	{.symbol = _multiboot_partition_string, .addr = 0x0003e828},
	{.symbol = _multiboot_partition_set_string, .addr = 0x0003e82c},
	{.symbol = _multiboot_timeout_string, .addr = 0x0003e820},
	{.symbol = _multiboot_timeout_set_string, .addr = 0x0003e824},
	{.symbol = _multiboot_to_boot_string, .addr = 0x00024a09},
	{.symbol = _nbpScanBootVolumes_string, .addr = 0x0002a8d4},
	{.symbol = _nbpUnloadBaseCode_string, .addr = 0x0002a8db},
	{.symbol = _newAPMBVRef_string, .addr = 0x0002821f},
	{.symbol = _newFDiskBVRef_string, .addr = 0x00028bdd},
	{.symbol = _newFilteredBVChain_string, .addr = 0x0002800c},
	{.symbol = _newGPTBVRef_string, .addr = 0x00028a9e},
	{.symbol = _newString_string, .addr = 0x00029e7a},
	{.symbol = _newStringForKey_string, .addr = 0x0002a58e},
	{.symbol = _new_dsdt_string, .addr = 0x0003ec34},
	{.symbol = _open_string, .addr = 0x000275e8},
	{.symbol = _open_bvdev_string, .addr = 0x00027614},
	{.symbol = _opendir_string, .addr = 0x00026fd3},
	{.symbol = _openmem_string, .addr = 0x00027076},
	{.symbol = _p_get_ramdisk_info_string, .addr = 0x0003e8b0},
	{.symbol = _p_ramdiskReadBytes_string, .addr = 0x0003e8ac},
	{.symbol = _parse_mach_string, .addr = 0x0002642b},
	{.symbol = _patch_fadt_string, .addr = 0x00034086},
	{.symbol = _pause_string, .addr = 0x0002a8c0},
	{.symbol = _pci_config_read16_string, .addr = 0x0002d1e4},
	{.symbol = _pci_config_read32_string, .addr = 0x0002d20f},
	{.symbol = _pci_config_read8_string, .addr = 0x0002d1ba},
	{.symbol = _pci_config_write16_string, .addr = 0x0002d227},
	{.symbol = _pci_config_write32_string, .addr = 0x0002d4f9},
	{.symbol = _pci_config_write8_string, .addr = 0x0002d4cb},
	{.symbol = _platformCPUFeature_string, .addr = 0x0002a8e2},
	{.symbol = _previewLoadedSectors_string, .addr = 0x0003cfd0},
	{.symbol = _previewSaveunder_string, .addr = 0x0003cfd4},
	{.symbol = _previewTotalSectors_string, .addr = 0x0003cfcc},
	{.symbol = _prf_string, .addr = 0x00038460},
	{.symbol = _printMemoryInfo_string, .addr = 0x00023689},
	{.symbol = _printMenuItem_string, .addr = 0x000235da},
	{.symbol = _printf_string, .addr = 0x0002a6af},
	{.symbol = _processBootArgument_string, .addr = 0x00022de0},
	{.symbol = _processBootOptions_string, .addr = 0x00022e6f},
	{.symbol = _processRAMDiskCommand_string, .addr = 0x00024d32},
	{.symbol = _promptForRescanOption_string, .addr = 0x00022b51},
	{.symbol = _ptol_string, .addr = 0x00037702},
	{.symbol = _putc_string, .addr = 0x0002b6db},
	{.symbol = _putca_string, .addr = 0x0002b69b},
	{.symbol = _putchar_string, .addr = 0x0002a7d6},
	{.symbol = _rawDiskRead_string, .addr = 0x000283f5},
	{.symbol = _rawDiskWrite_string, .addr = 0x000282ef},
	{.symbol = _read_string, .addr = 0x000274bb},
	{.symbol = _readBootSector_string, .addr = 0x00028a35},
	{.symbol = _readKeyboardShiftFlags_string, .addr = 0x0002ba7c},
	{.symbol = _readKeyboardStatus_string, .addr = 0x0002baa3},
	{.symbol = _readdir_string, .addr = 0x00026ab8},
	{.symbol = _readdir_ext_string, .addr = 0x00026ada},
	{.symbol = _realloc_string, .addr = 0x00037dd3},
	{.symbol = _rebase_location_string, .addr = 0x00025df9},
	{.symbol = _rebase_macho_string, .addr = 0x00025b6f},
	{.symbol = _ref_strings_string, .addr = 0x0003e8f8},
	{.symbol = _register_hook_callback_string, .addr = 0x000266f0},
	{.symbol = _replace_function_string, .addr = 0x00025ef9},
	{.symbol = _rescanBIOSDevice_string, .addr = 0x000281f2},
	{.symbol = _reserveKernBootStruct_string, .addr = 0x00029be6},
	{.symbol = _restoreCursor_string, .addr = 0x000237b1},
	{.symbol = _root_pci_dev_string, .addr = 0x00045ee0},
	{.symbol = _rsdplength_string, .addr = 0x000461a4},
	{.symbol = _safe_malloc_string, .addr = 0x00037c97},
	{.symbol = _scanBootVolumes_string, .addr = 0x00026cc6},
	{.symbol = _scanDMI_string, .addr = 0x000336cb},
	{.symbol = _scanDisks_string, .addr = 0x00026f44},
	{.symbol = _scan_cpu_string, .addr = 0x000312fe},
	{.symbol = _scan_cpu_DMI_string, .addr = 0x0003378c},
	{.symbol = _scan_mem_string, .addr = 0x0002a960},
	{.symbol = _scan_pci_bus_string, .addr = 0x0002d389},
	{.symbol = _scan_platform_string, .addr = 0x0002a8f8},
	{.symbol = _scollPage_string, .addr = 0x0002b541},
	{.symbol = _search_and_get_acpi_fd_string, .addr = 0x000351ed},
	{.symbol = _selectBootVolume_string, .addr = 0x00026b99},
	{.symbol = _selectIndex_string, .addr = 0x0003e80c},
	{.symbol = _setActiveDisplayPage_string, .addr = 0x0002b518},
	{.symbol = _setBootGlobals_string, .addr = 0x00027756},
	{.symbol = _setCursorPosition_string, .addr = 0x0002b62c},
	{.symbol = _setCursorType_string, .addr = 0x0002b602},
	{.symbol = _setRAMDiskBTHook_string, .addr = 0x00024a49},
	{.symbol = _setRootVolume_string, .addr = 0x00026b1a},
	{.symbol = _setVBEMode_string, .addr = 0x0002bf68},
	{.symbol = _setVBEPalette_string, .addr = 0x0002bf0e},
	{.symbol = _setVESAGraphicsMode_string, .addr = 0x000216e8},
	{.symbol = _setVideoMode_string, .addr = 0x000218f4},
	{.symbol = _set_eth_builtin_string, .addr = 0x00036699},
	{.symbol = _setupAcpi_string, .addr = 0x000353c8},
	{.symbol = _setupAcpiNoMod_string, .addr = 0x00033f0b},
	{.symbol = _setupBooterLog_string, .addr = 0x0002a812},
	{.symbol = _setupDeviceProperties_string, .addr = 0x00036760},
	{.symbol = _setupEfiDeviceTree_string, .addr = 0x0002ab56},
	{.symbol = _setupEfiTables32_string, .addr = 0x0002af4e},
	{.symbol = _setupEfiTables64_string, .addr = 0x0002ad11},
	{.symbol = _setupFakeEfi_string, .addr = 0x0002b184},
	{.symbol = _setupSystemType_string, .addr = 0x0002a98a},
	{.symbol = _setup_pci_devs_string, .addr = 0x000367f5},
	{.symbol = _shouldboot_string, .addr = 0x0003e808},
	{.symbol = _showHelp_string, .addr = 0x00022d34},
	{.symbol = _showInfoRAMDisk_string, .addr = 0x00024a6e},
	{.symbol = _showTextBuffer_string, .addr = 0x00022b6f},
	{.symbol = _showTextFile_string, .addr = 0x00022ca7},
	{.symbol = _sleep_string, .addr = 0x0002be7b},
	{.symbol = _slvprintf_string, .addr = 0x00037e2d},
	{.symbol = _smbiosStringAtIndex_string, .addr = 0x000321c6},
	{.symbol = _smbios_p_string, .addr = 0x00046100},
	{.symbol = _smbios_properties_string, .addr = 0x0003e94c},
	{.symbol = _smbios_table_descriptions_string, .addr = 0x0003ebb4},
	{.symbol = _spinActivityIndicator_string, .addr = 0x00021a3a},
	{.symbol = _sprintf_string, .addr = 0x00037e67},
	{.symbol = _sputc_string, .addr = 0x0002a5e5},
	{.symbol = _st_string, .addr = 0x000461b0},
	{.symbol = _startprog_string, .addr = 0x0002038d},
	{.symbol = _stop_string, .addr = 0x0002a725},
	{.symbol = _stosl_string, .addr = 0x00021084},
	{.symbol = _strbreak_string, .addr = 0x000377be},
	{.symbol = _strcat_string, .addr = 0x0003788d},
	{.symbol = _strchr_string, .addr = 0x00037785},
	{.symbol = _strcmp_string, .addr = 0x00037647},
	{.symbol = _strcpy_string, .addr = 0x0003769c},
	{.symbol = _strdup_string, .addr = 0x00037853},
	{.symbol = _string_string, .addr = 0x0003ec44},
	{.symbol = _stringLength_string, .addr = 0x00029d8b},
	{.symbol = _stringdata_string, .addr = 0x0003ec48},
	{.symbol = _stringlength_string, .addr = 0x0003ec4c},
	{.symbol = _strlcpy_string, .addr = 0x000376d8},
	{.symbol = _strlen_string, .addr = 0x0003760d},
	{.symbol = _strncat_string, .addr = 0x00037755},
	{.symbol = _strncmp_string, .addr = 0x00037668},
	{.symbol = _strncpy_string, .addr = 0x000376b4},
	{.symbol = _strstr_string, .addr = 0x000378c7},
	{.symbol = _strtol_string, .addr = 0x00037ea3},
	{.symbol = _strtoul_string, .addr = 0x000381e0},
	{.symbol = _strtouq_string, .addr = 0x00038014},
	{.symbol = _symbols_module_start_string, .addr = 0x0003e850},
	{.symbol = _sysConfigValid_string, .addr = 0x00044290},
	{.symbol = _systemConfigDir_string, .addr = 0x00026afe},
	{.symbol = _systemVersion_string, .addr = 0x000442a0},
	{.symbol = _tableSign_string, .addr = 0x00033d8f},
	{.symbol = _tell_string, .addr = 0x00026a92},
	{.symbol = _testBiosread_string, .addr = 0x000299ff},
	{.symbol = _testFAT32EFIBootSector_string, .addr = 0x000289b1},
	{.symbol = _textAddress_string, .addr = 0x0003e840},
	{.symbol = _textSection_string, .addr = 0x0003e848},
	{.symbol = _time18_string, .addr = 0x0002ba42},
	{.symbol = _umountRAMDisk_string, .addr = 0x00024b00},
	{.symbol = _updateProgressBar_string, .addr = 0x000211da},
	{.symbol = _utf_decodestr_string, .addr = 0x00036b8d},
	{.symbol = _utf_encodestr_string, .addr = 0x00036c68},
	{.symbol = _verbose_string, .addr = 0x0002a630},
	{.symbol = _video_mode_string, .addr = 0x0002b986},
	{.symbol = _vol_opendir_string, .addr = 0x00027031},
	{.symbol = _waitThenReload_string, .addr = 0x0002465d},
	{.symbol = _write_string, .addr = 0x0002745c},
	{.symbol = _writebyte_string, .addr = 0x00027412},
	{.symbol = _writeint_string, .addr = 0x000273c0},
	{.symbol = boot2_string, .addr = 0x00020200},
	{.symbol = dyld_stub_binder_string, .addr = 0x00026a4d},
};
