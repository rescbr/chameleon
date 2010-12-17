MODULE_NAME = GUI
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = _$(MODULE_NAME)_start
MODULE_DEPENDENCIES = 

DIR = GUI

MODULE_OBJS = GUI_module.o gui.o picopng.o graphic_utils.o

THEME = embed
THEMEDIR = ../../../artwork/themes/$(THEME)

OPTIM = -Os -Oz
DEBUG = -DNOTHING
#DEBUG = -DDEBUG_HELLO_WORLD=1
CFLAGS	= $(RC_CFLAGS) $(OPTIM) $(MORECPP) -arch i386 -g -Wmost \
    -D__ARCHITECTURE__=\"i386\" -DSAIO_INTERNAL_USER \
    -DRCZ_COMPRESSED_FILE_SUPPORT $(DEBUG) \
    -fno-builtin $(OMIT_FRAME_POINTER_CFLAG) \
    -mpreferred-stack-boundary=2 -fno-align-functions -fno-stack-protector \
    -march=pentium4 -msse2 -mfpmath=sse -msoft-float -fno-common

DEFINES=
CONFIG = hd
INC = -I. -I.. -I$(SYMROOT) -I$(UTILDIR) -I$(LIBSADIR) -I$(LIBSAIODIR) -I$(BOOT2DIR)
LIBS=

VPATH = $(OBJROOT):$(SYMROOT)

SFILES =
CFILES = 
HFILES = appleboot.h
EXPORTED_HFILES =
INSTALLED_HFILES =
OTHERFILES = Makefile
ALLSRC =  $(SFILES) $(CFILES) \
	$(HFILES) $(OTHERFILES)
DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all embedtheme optionrom: dylib

embedtheme: CFLAGS += -DEMBED_THEME
embedtheme: art.h all

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

include ../MakeInc.dir