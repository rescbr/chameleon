MODULE_NAME = FileNVRAM
MODULE_DESCRIPTION = FileNVRAM module for preloading NVRAM values
MODULE_AUTHOR = xZenue LLC.
MODULE_VERSION = "1.1.3"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = $(MODULE_NAME)_start
MODULE_DEPENDENCIES = 

DIR = FileNVRAM



include ../MakeInc.dir

$(SYMROOT)/modules/$(MODULE_NAME).dylib:
	@echo "	[CP] $(MODULE_NAME).dylib"
	@cp $(MODULE_NAME).dylib $(SRCROOT)/sym/i386/modules/$(MODULE_NAME).dylib

$(SRCROOT)/sym/i386/boot_modules.c:
	@
