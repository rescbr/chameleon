
MODULE_NAME = Symbols
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = `ls ../../.svn &> /dev/null && (svn info ../../ | grep Revision | awk '{print $$2}') || echo 0`
MODULE_START = _$(MODULE_NAME)_start
MODULE_DEPENDENCIES = 
DIR = Symbols

include ../../MakePaths.dir

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

OBJS = Symbols.o


SFILES =
CFILES = 
HFILES = 
EXPORTED_HFILES =
INSTALLED_HFILES =
OTHERFILES = Makefile
ALLSRC =  $(SFILES) $(CFILES) \
	$(HFILES) $(OTHERFILES)
DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all embedtheme: dylib


dylib: $(OBJS)
	ld -arch i386 \
	-undefined dynamic_lookup \
	-alias $(MODULE_START) start \
	-dylib -read_only_relocs suppress \
	-S -x  -dead_strip_dylibs \
	-no_uuid \
	-bind_at_load \
	-current_version $(MODULE_VERSION) -compatibility_version $(MODULE_COMPAT_VERSION) \
	-final_output $(MODULE_NAME) \
	$(OBJROOT)/*.o \
	-o $(SYMROOT)/$(MODULE_NAME).dylib



Symbols.o: #Symbols.h
	@rm -rf Symbols.h
	@echo "typedef struct {" >> Symbols.h
	@echo "	char*	symbol;" >> Symbols.h
	@echo "	unsigned int	addr;" >> Symbols.h
	@echo "} symbol_t;" >> Symbols.h 
	@echo "" >> Symbols.h
	
	@nm -g $(SYMROOT)/boot.sys | tr . _ | awk '{print "static char "$$3"_string[] = \""$$3"\";"}' >> Symbols.h

	@echo "symbol_t symbolList[] = {" >> Symbols.h
	@nm -g $(SYMROOT)/boot.sys | tr . _ | awk '{print "	{.symbol = "$$3"_string, .addr = 0x"$$1"},";}' >> Symbols.h
	@echo "};" >> Symbols.h

	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c Symbols.c $(INC) -o $(OBJROOT)/Symbols.o 

Symbols.h:
	@rm -rf Symbols.h
	@echo "typedef struct {" >> Symbols.h
	@echo "	char*	symbol;" >> Symbols.h
	@echo "	unsigned int	addr;" >> Symbols.h
	@echo "} symbol_t;" >> Symbols.h 
	@echo "" >> Symbols.h
	
	@nm -g $(SYMROOT)/boot.sys | tr . _ | awk '{print "static char "$$3"_string[] = \""$$3"\";"}' >> Symbols.h

	@echo "symbol_t symbolList[] = {" >> Symbols.h
	@nm -g $(SYMROOT)/boot.sys | tr . _ | awk '{print "	{.symbol = "$$3"_string, .addr = 0x"$$1"},";}' >> Symbols.h
	@echo "};" >> Symbols.h


include ../../MakeInc.dir

# dependencies
-include $(OBJROOT)/Makedep
