
MODULE_NAME = GraphicsEnabler
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = _$(MODULE_NAME)_start
MODULE_DEPENDENCIES = 

DIR = GraphicsEnabler

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
    -D__ARCHITECTURE__=\"i386\" -DSAIO_INTERNAL_USER \
    -DRCZ_COMPRESSED_FILE_SUPPORT $(DEBUG) \
    -fno-builtin $(OMIT_FRAME_POINTER_CFLAG) \
    -mpreferred-stack-boundary=2 -fno-align-functions \
    -march=pentium4 -msse2 -mfpmath=sse -msoft-float -fno-common 

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

VPATH = $(OBJROOT):$(SYMROOT)

GRAPHICS_ENABLER_OBJS = nvidia.o ati.o gma.o GraphicsEnabler.o stack_protector.o


SFILES =
CFILES = 
HFILES = 
EXPORTED_HFILES =
INSTALLED_HFILES =
OTHERFILES = Makefile
ALLSRC =  $(SFILES) $(CFILES) \
	$(HFILES) $(OTHERFILES)
DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all embedtheme: ${GRAPHICS_ENABLER_OBJS} dylib


dylib: 
	ld -arch i386 \
	-undefined dynamic_lookup \
	-alias $(MODULE_START) start \
	-dylib -read_only_relocs suppress \
	-S -x -Z -dead_strip_dylibs \
	-no_uuid \
	-current_version $(MODULE_VERSION) -compatibility_version $(MODULE_COMPAT_VERSION) \
	-final_output $(MODULE_NAME) \
    $(OBJROOT)/stack_protector.o \
    $(OBJROOT)/nvidia.o \
	$(OBJROOT)/ati.o \
	$(OBJROOT)/gma.o \
	$(OBJROOT)/GraphicsEnabler.o \
	-macosx_version_min 10.6 \
	-o $(SYMROOT)/$(MODULE_NAME).dylib

	
stack_protector.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "$(LIBSAIODIR)/stack_protector.c" $(INC) -o "$(OBJROOT)/stack_protector.o"
        
GraphicsEnabler.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "GraphicsEnabler.c" $(INC) -o "$(OBJROOT)/GraphicsEnabler.o"

ati.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "ati.c" $(INC) -o "$(OBJROOT)/ati.o"

nvidia.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "nvidia.c" $(INC) -o "$(OBJROOT)/nvidia.o"

gma.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "gma.c" $(INC) -o "$(OBJROOT)/gma.o"

include ../../MakeInc.dir

# dependencies
-include $(OBJROOT)/Makedep
