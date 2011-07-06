
MODULE_NAME = SMBiosGetters
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = _$(MODULE_NAME)_start
MODULE_DEPENDENCIES = 

DIR = SMBiosGetters

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
CFLAGS	= $(RC_CFLAGS) $(OPTIM) $(MORECPP) -arch i386 -g -Wmost \
    -D__ARCHITECTURE__=\"i386\" -DSAIO_INTERNAL_USER \
    -DRCZ_COMPRESSED_FILE_SUPPORT $(DEBUG) \
    -fno-builtin $(OMIT_FRAME_POINTER_CFLAG) \
    -mpreferred-stack-boundary=2 -fno-align-functions \
    -march=pentium4 -msse2 -mfpmath=sse -msoft-float -fno-common -fno-stack-protector #-lc_kext -lc_kext -libcc_kext -fstack-protector-all

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

SMBiosGetters_OBJS = SMBiosGetters.o mysmbios.o smbios_decode.o smbios_getters.o


SFILES =
CFILES = 
HFILES = 
EXPORTED_HFILES =
INSTALLED_HFILES =
OTHERFILES = Makefile
ALLSRC =  $(SFILES) $(CFILES) \
	$(HFILES) $(OTHERFILES)
DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all embedtheme: ${SMBiosGetters_OBJS} dylib


dylib: 
	ld -arch i386 \
	-undefined dynamic_lookup \
	-alias $(MODULE_START) start \
	-dylib -read_only_relocs suppress \
	-S -x -Z -dead_strip_dylibs \
	-no_uuid \
	-current_version $(MODULE_VERSION) -compatibility_version $(MODULE_COMPAT_VERSION) \
	-final_output $(MODULE_NAME) \
	"/usr/lib/gcc/i686-apple-darwin10/4.2.1/libcc_kext.a" \
	$(OBJROOT)/SMBiosGetters.o \
	$(OBJROOT)/mysmbios.o \
	$(OBJROOT)/smbios_getters.o \
	$(OBJROOT)/smbios_decode.o \
	-o $(SYMROOT)/$(MODULE_NAME).dylib



SMBiosGetters.o: 	
	gcc-4.2 $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "SMBiosGetters.c" $(INC) -o "$(OBJROOT)/SMBiosGetters.o"
	
mysmbios.o: 	
	gcc-4.2 $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "mysmbios.c" $(INC) -o "$(OBJROOT)/mysmbios.o"
	
smbios_decode.o: 	
	gcc-4.2 $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "smbios_decode.c" $(INC) -o "$(OBJROOT)/smbios_decode.o"
	
smbios_getters.o: 	
	gcc-4.2 $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "smbios_getters.c" $(INC) -o "$(OBJROOT)/smbios_getters.o"

include ../../MakeInc.dir

# dependencies
-include $(OBJROOT)/Makedep
