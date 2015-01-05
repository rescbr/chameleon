MODULE_NAME = KernelPatcher
MODULE_DESCRIPTION = Kernel patcher
MODULE_AUTHOR = Evan Lojewski
MODULE_VERSION = "1.1.3"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = $(MODULE_NAME)_start
MODULE_DEPENDENCIES =

DIR = KernelPatcher



include ../MakeInc.dir

$(SYMROOT)/modules/$(MODULE_NAME).dylib:
	@echo "	[CP] $(MODULE_NAME).dylib"
	@cp $(MODULE_NAME).dylib $(SRCROOT)/sym/i386/modules/$(MODULE_NAME).dylib

$(SRCROOT)/sym/i386/boot_modules.c:
	@
