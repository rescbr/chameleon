
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

OBJS = kext_patcher.o inflate.o zutil.o inftrees.o inffast.o adler32.o


SFILES =
CFILES = 
HFILES = 
EXPORTED_HFILES =
INSTALLED_HFILES =
OTHERFILES = Makefile
ALLSRC =  $(SFILES) $(CFILES) \
	$(HFILES) $(OTHERFILES)
DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all embedtheme optionrom: ${OBJS} dylib


dylib: ${OBJS}
	ld -arch i386 \
	-undefined dynamic_lookup \
	-alias $(MODULE_START) start \
	-dylib -read_only_relocs suppress \
	-S -x  -dead_strip_dylibs \
	-no_uuid \
	-bind_at_load \
	-current_version $(MODULE_VERSION) -compatibility_version $(MODULE_COMPAT_VERSION) \
	-final_output $(MODULE_NAME) \
	-weak_library $(SYMROOT)/Symbols.dylib \
	-weak_library $(SYMROOT)/GUI.dylib \
	$(OBJROOT)/kext_patcher.o \
	$(OBJROOT)/inflate.o \
	$(OBJROOT)/inftrees.o \
	$(OBJROOT)/zutil.o \
	$(OBJROOT)/inffast.o \
	$(OBJROOT)/adler32.o \
	-o $(SYMROOT)/$(MODULE_NAME).dylib


kext_patcher.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "kext_patcher.c" $(INC) -o "$(OBJROOT)/kext_patcher.o"

inflate.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "inflate.c" $(INC) -o "$(OBJROOT)/inflate.o"

zutil.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "zutil.c" $(INC) -o "$(OBJROOT)/zutil.o"

inftrees.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "inftrees.c" $(INC) -o "$(OBJROOT)/inftrees.o"

inffast.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "inffast.c" $(INC) -o "$(OBJROOT)/inffast.o"

adler32.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "adler32.c" $(INC) -o "$(OBJROOT)/adler32.o"

include ../../MakeInc.dir

# dependencies
-include $(OBJROOT)/Makedep
