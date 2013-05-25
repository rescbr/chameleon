MODULE_NAME = NVIDIAGraphicsEnabler
MODULE_AUTHOR = Chameleon
MODULE_DESCRIPTION = to be added...
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = $(MODULE_NAME)_start
MODULE_DEPENDENCIES = 

DIR = NVIDIAGraphicsEnabler



include ../MakeInc.dir

$(SYMROOT)/modules/$(MODULE_NAME).dylib:
	@echo "\t[CP] $(MODULE_NAME).dylib"
	@cp $(MODULE_NAME).dylib $(SRCROOT)/sym/i386/modules/$(MODULE_NAME).dylib

$(SRCROOT)/sym/i386/boot_modules.c:
	@
