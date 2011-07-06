
MODULE_NAME = HibernateEnabler
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = _$(MODULE_NAME)_start
MODULE_DEPENDENCIES = 

DIR = HibernateEnabler

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
    -mpreferred-stack-boundary=2 -fno-align-functions -fno-stack-protector \
    -march=pentium4 -msse2 -mfpmath=sse -msoft-float -fno-common  #-mdynamic-no-pic

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

H_ENAB_OBJS = bmdecompress.o graphic_utils.o resume.o HibernateEnabler.o

SFILES =
CFILES = 
HFILES = 
EXPORTED_HFILES =
INSTALLED_HFILES =
OTHERFILES = Makefile
ALLSRC =  $(SFILES) $(CFILES) \
	$(HFILES) $(OTHERFILES)
DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all embedtheme: ${H_ENAB_OBJS} dylib


dylib: 
	ld -arch i386 \
	-undefined dynamic_lookup \
	-alias $(MODULE_START) start \
	-dylib -read_only_relocs suppress \
	-S -x -Z -dead_strip_dylibs \
	-no_uuid \
	-current_version $(MODULE_VERSION) -compatibility_version $(MODULE_COMPAT_VERSION) \
	-final_output $(MODULE_NAME) \
	$(OBJROOT)/bmdecompress.o \
	$(OBJROOT)/graphic_utils.o \
	$(OBJROOT)/resume.o \
	$(OBJROOT)/HibernateEnabler.o \
	-o $(SYMROOT)/$(MODULE_NAME).dylib



HibernateEnabler.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "HibernateEnabler.c" $(INC) -o "$(OBJROOT)/HibernateEnabler.o"
	
resume.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "resume.c" $(INC) -o "$(OBJROOT)/resume.o"
	
graphic_utils.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "graphic_utils.c" $(INC) -o "$(OBJROOT)/graphic_utils.o"
			
bmdecompress.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "bmdecompress.c" $(INC) -o "$(OBJROOT)/bmdecompress.o"
	
include ../../MakeInc.dir

# dependencies
-include $(OBJROOT)/Makedep
