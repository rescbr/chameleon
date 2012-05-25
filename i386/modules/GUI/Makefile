
MODULE_NAME = GUI
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = _$(MODULE_NAME)_start
MODULE_DEPENDENCIES = 

BUNDLE_NAME = $(MODULE_NAME)
BUNDLE_EXEC = $(BUNDLE_NAME)
BUNDLE_LIBS = 
BUNDLE_INFO      = 
BUNDLE_ID      =  com.boot.UI.GUI.gui
BUNDLE_COPYRIGHT = 
BUNDLE_LICENCE   = 
BUNDLE_VERSION   = 1.0
BUNDLE_ICON      =
BUNDLE_SHORT_VERSION = 1
BUNDLE_CLASS = GUI
BUNDLE_CLASS_PROVIDER = UI

DIR = GUI

include ../../MakePaths.dir

OBJROOT=../../../obj/i386/modules/$(DIR)
SYMROOT=../../../sym/i386/modules/
DSTROOT=../../../dst/i386/modules/


UTILDIR = ../../util
LIBSADIR = ../../libsa
LIBSAIODIR = ../../libsaio
BOOT2DIR = ../../boot2

INSTALLDIR = $(DSTROOT)/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders/standalone

OPTIM = -Os -Oz
DEBUG = -DNOTHING
CFLAGS	= $(RC_CFLAGS) $(OPTIM) $(MORECPP) -arch i386 -g -Wmost -fstack-protector-all \
    -D__ARCHITECTURE__=\"i386\"  \
    $(DEBUG) \
    -fno-builtin $(OMIT_FRAME_POINTER_CFLAG) \
    -mpreferred-stack-boundary=2 -fno-align-functions \
    -march=pentium4 -msse2 -mfpmath=sse -msoft-float -fno-common #-fstack-protector-all

DEFINES=
CONFIG = hd
INC = -I. -I.. -I$(SYMROOT) -I$(UTILDIR) -I$(LIBSADIR) -I$(LIBSAIODIR) -I$(BOOT2DIR)
ifneq "" "$(wildcard /bin/mkdirs)"
  MKDIRS = /bin/mkdirs
else
  MKDIRS = /bin/mkdir -p
endif
AS = as
LD = ld
# LIBS= -lc_static
LIBS=

THEME = embed
THEMEDIR = ../../../artwork/themes/$(THEME)


VPATH = $(OBJROOT):$(SYMROOT)

GUI_OBJS = GUI_module.o gui.o picopng.o graphic_utils.o stack_protector.o


SFILES =
CFILES = 
HFILES = appleboot.h
EXPORTED_HFILES =
INSTALLED_HFILES =
OTHERFILES = Makefile
ALLSRC =  $(SFILES) $(CFILES) \
	$(HFILES) $(OTHERFILES)
DIRS_NEEDED = $(OBJROOT) $(SYMROOT)



embedtheme: CFLAGS += -DEMBED_THEME
embedtheme: art.h all

#CFLAGS += -DBOOT_HELPER_SUPPORT

all: ${GUI_OBJS} bundle


bundle: 
	ld -arch i386 \
	-undefined dynamic_lookup \
	-alias $(MODULE_START) start \
	-dylib -read_only_relocs suppress \
	-S -x -Z -dead_strip_dylibs \
	-no_uuid \
	-current_version $(MODULE_VERSION) -compatibility_version $(MODULE_COMPAT_VERSION) \
	-final_output $(MODULE_NAME) \
	-macosx_version_min 10.6 \
    $(OBJROOT)/stack_protector.o \
    $(OBJROOT)/GUI_module.o \
	$(OBJROOT)/gui.o \
	$(OBJROOT)/picopng.o \
	$(OBJROOT)/graphic_utils.o \
	 -o $(SYMROOT)/$(BUNDLE_EXEC)
	@rm -rf $(SYMROOT)/$(BUNDLE_NAME).bundle	
	@mkdir $(SYMROOT)/$(BUNDLE_NAME).bundle	
	@mkdir $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents	
	@mkdir $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/MacOS	
	@mkdir $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/Resources	
	@mkdir $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/PlugIns	
	@mv $(SYMROOT)/$(BUNDLE_EXEC) $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/MacOS	
	@#mv $(SYMROOT)/$(SYMBOLS_BUNDLE_LIBS) $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/PlugIns	
	
	@defaults write $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/Info "CFBundleGetInfoString" '$(BUNDLE_INFO)'	
	@defaults write $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/Info "NSHumanReadableCopyright" '$(BUNDLE_COPYRIGHT)'	
	@defaults write $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/Info "CFBundleShortVersionString" '$(BUNDLE_SHORT_VERSION)'	
	@defaults write $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/Info "CFBundleIconFile" '$(BUNDLE_ICON)'	
	@defaults write $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/Info "CFBundleVersion" '$(BUNDLE_VERSION)'	
	@defaults write $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/Info "CFBundlePackageType" 'BNDL'	
	@defaults write $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/Info "CFBundleInfoDictionaryVersion" '6.0'	
	@defaults write $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/Info "CFBundleName" '$(BUNDLE_NAME)'	
	@defaults write $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/Info "CFBundleIdentifier" '$(BUNDLE_ID)'
	@defaults write $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/Info "NSPrincipalClass" '$(BUNDLE_CLASS)'

	@defaults write $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/Info "CFBundleExecutable" '$(BUNDLE_EXEC)'	
	@defaults write $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/Info "IOClass" '$(BUNDLE_CLASS)'	
	@defaults write $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/Info "IOProviderClass" '$(BUNDLE_CLASS_PROVIDER)'	
	@plutil -convert xml1 $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/Info.plist

stack_protector.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "$(LIBSAIODIR)/stack_protector.c" $(INC) -o "$(OBJROOT)/stack_protector.o"

GUI_module.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "GUI_module.c" $(INC) -o "$(OBJROOT)/GUI_module.o"

gui.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "gui.c" $(INC) -o "$(OBJROOT)/gui.o"


picopng.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "picopng.c" $(INC) -o "$(OBJROOT)/picopng.o"

graphic_utils.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "graphic_utils.c" $(INC) -o "$(OBJROOT)/graphic_utils.o"
	
art.h:
	@if [ "$(PNGCRUSH)" ]; then	\
		echo "optimizing art files ...\n$(PNGCRUSH) $(PNGOPTIONS) artwork/$(THEME)"; \
		cd $(THEMEDIR) && find . -name '*.png' -exec $(PNGCRUSH) -q $(PNGOPTIONS) -d $(SYMROOT)/embed {} \;; \
		cp theme.plist $(SYMROOT)/embed; \
	else \
		cp -R $(THEMEDIR) $(SYMROOT)/embed; \
	fi;
	@echo "#ifndef __BOOT2_ART_H" >> $(SYMROOT)/art.h
	@echo "#define __BOOT2_ART_H" >> $(SYMROOT)/art.h
	@cd $(SYMROOT)/embed && find . -name 'theme.plist' -exec xxd -i >> $(SYMROOT)/art.h {} \;
	@cd $(SYMROOT)/embed && find . -name '*.png' -exec xxd -i >> $(SYMROOT)/art.h {} \;

	@echo "typedef struct {" >> $(SYMROOT)/art.h
	@echo "	char			name[32];" >> $(SYMROOT)/art.h
	@echo "	unsigned char	*pngdata;" >> $(SYMROOT)/art.h
	@echo "	unsigned int	*length;" >> $(SYMROOT)/art.h
	@echo "} embeddedpng_t;" >> $(SYMROOT)/art.h
	@echo "" >> $(SYMROOT)/art.h
	@echo "embeddedpng_t embeddedImages[] = {" >> $(SYMROOT)/art.h
	@cd $(SYMROOT)/embed && find . -name '*.png' | sort | cut -f 2 -d '/' | cut -f 1 -d '.' | \
		awk '{ printf "\t{.name = \"%s\", .pngdata = __%s_png, .length = &__%s_png_len},\n", $$1, $$1, $$1 }' >> $(SYMROOT)/art.h
	@echo "};" >> $(SYMROOT)/art.h

	@echo "#endif /* !__BOOT2_ART_H */"  >> $(SYMROOT)/art.h ;\





include ../../MakeInc.dir

# dependencies
-include $(OBJROOT)/Makedep
