
MODULE_NAME = KextPatcher
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = _$(MODULE_NAME)_start
MODULE_DEPENDENCIES = 

DIR = KextPatcher

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

KEXT_OBJS = kext_patcher.o inflate.o deflate.o zutil.o inftrees.o inffast.o adler32.o hex_editor.o trees.o stack_protector.o


SFILES =
CFILES = 
HFILES = 
EXPORTED_HFILES =
INSTALLED_HFILES =
OTHERFILES = Makefile
ALLSRC =  $(SFILES) $(CFILES) \
	$(HFILES) $(OTHERFILES)
DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all embedtheme: ${KEXT_OBJS} dylib


dylib: 
	ld -arch i386 \
	-undefined dynamic_lookup \
	-alias $(MODULE_START) start \
	-dylib -read_only_relocs suppress \
	-S -x -Z -dead_strip_dylibs \
	-no_uuid \
	-current_version $(MODULE_VERSION) -compatibility_version $(MODULE_COMPAT_VERSION) \
	-final_output $(MODULE_NAME) \
	-macosx_version_min 10.6 \
	$(OBJROOT)/*.o \
	-weak_library $(SYMROOT)/Symbols.dylib \
	-o $(SYMROOT)/$(MODULE_NAME).dylib

	
stack_protector.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "$(LIBSAIODIR)/stack_protector.c" $(INC) -o "$(OBJROOT)/stack_protector.o"
        

kext_patcher.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "kext_patcher.c" $(INC) -o "$(OBJROOT)/kext_patcher.o"

inflate.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "inflate.c" $(INC) -o "$(OBJROOT)/inflate.o"

deflate.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "deflate.c" $(INC) -o "$(OBJROOT)/deflate.o"

zutil.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "zutil.c" $(INC) -o "$(OBJROOT)/zutil.o"

inftrees.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "inftrees.c" $(INC) -o "$(OBJROOT)/inftrees.o"

inffast.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "inffast.c" $(INC) -o "$(OBJROOT)/inffast.o"

adler32.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "adler32.c" $(INC) -o "$(OBJROOT)/adler32.o"

trees.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "trees.c" $(INC) -o "$(OBJROOT)/trees.o"

hex_editor.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "hex_editor.c" $(INC) -o "$(OBJROOT)/hex_editor.o"

include ../../MakeInc.dir

# dependencies
-include $(OBJROOT)/Makedep
