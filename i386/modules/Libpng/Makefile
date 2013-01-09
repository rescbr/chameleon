
MODULE_NAME = Libpng
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = _$(MODULE_NAME)_start
MODULE_DEPENDENCIES = 

BUNDLE_NAME = $(MODULE_NAME)
BUNDLE_EXEC = $(BUNDLE_NAME)
BUNDLE_LIBS = 
BUNDLE_INFO      = 
BUNDLE_ID      =  com.boot.LIBS.libpng
BUNDLE_COPYRIGHT = 
BUNDLE_LICENCE   = 
BUNDLE_VERSION   = 1.0
BUNDLE_ICON      =
BUNDLE_SHORT_VERSION = 1
BUNDLE_CLASS = LIBPNG
BUNDLE_CLASS_PROVIDER = LIBS

DIR = Libpng

include ../../MakePaths.dir

OBJROOT=../../../obj/i386/modules/$(DIR)
SYMROOT=../../../sym/i386/modules/
DSTROOT=../../../dst/i386/modules/


UTILDIR = ../../util
LIBSADIR = ../../libsa
LIBSAIODIR = ../../libsaio
BOOT2DIR = ../../boot2
INCLUDE = ../include

INSTALLDIR = $(DSTROOT)/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders/standalone

OPTIM = -Os -Oz
DEBUG = -DNOTHING
CFLAGS	= $(RC_CFLAGS) $(OPTIM) $(MORECPP) -arch i386 -g -Wall -fstack-protector-all \
    -D__ARCHITECTURE__=\"i386\"  \
    $(DEBUG) \
    -fno-builtin $(OMIT_FRAME_POINTER_CFLAG) \
    -mpreferred-stack-boundary=2 -fno-align-functions \
    -march=pentium4 -msse2 -mfpmath=sse -msoft-float -fno-common  

DEFINES=
CONFIG = hd
INC = -I. -I.. -I$(SYMROOT) -I$(UTILDIR) -I$(LIBSADIR) -I$(LIBSAIODIR) -I$(INCLUDE) -I$(BOOT2DIR)
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

PNG_OBJS = PngFile.o png.o pngerror.o pngget.o pngmem.o pngpread.o pngread.o pngrio.o pngrtran.o pngrutil.o pngset.o pngtrans.o pngwio.o pngwrite.o pngwtran.o pngwutil.o stack_protector.o


SFILES =
CFILES = 
HFILES = 
EXPORTED_HFILES =
INSTALLED_HFILES =
OTHERFILES = Makefile
ALLSRC =  $(SFILES) $(CFILES) \
	$(HFILES) $(OTHERFILES)
DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all embedtheme: ${PNG_OBJS} bundle


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
	$(OBJROOT)/*.o -o $(SYMROOT)/$(BUNDLE_EXEC)
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
	@defaults write $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/Info "OSBundleLibraries" -dict "com.boot.LIBS.libstdio" '0' "com.boot.LIBS.libz" '0'
	@plutil -convert xml1 $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/Info.plist

stack_protector.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "$(LIBSAIODIR)/stack_protector.c" $(INC) -o "$(OBJROOT)/stack_protector.o"
        

PngFile.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "PngFile.c" $(INC) -o "$(OBJROOT)/PngFile.o"

png.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "png.c" $(INC) -o "$(OBJROOT)/png.o"

pngerror.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "pngerror.c" $(INC) -o "$(OBJROOT)/pngerror.o"

pngget.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "pngget.c" $(INC) -o "$(OBJROOT)/pngget.o"

pngmem.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "pngmem.c" $(INC) -o "$(OBJROOT)/pngmem.o"

pngpread.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "pngpread.c" $(INC) -o "$(OBJROOT)/pngpread.o"

pngread.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "pngread.c" $(INC) -o "$(OBJROOT)/pngread.o"

pngrio.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "pngrio.c" $(INC) -o "$(OBJROOT)/pngrio.o"        

pngrtran.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "pngrtran.c" $(INC) -o "$(OBJROOT)/pngrtran.o"

pngrutil.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "pngrutil.c" $(INC) -o "$(OBJROOT)/pngrutil.o"

pngset.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "pngset.c" $(INC) -o "$(OBJROOT)/pngset.o"

pngtrans.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "pngtrans.c" $(INC) -o "$(OBJROOT)/pngtrans.o"

pngwio.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "pngwio.c" $(INC) -o "$(OBJROOT)/pngwio.o"

pngwrite.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "pngwrite.c" $(INC) -o "$(OBJROOT)/pngwrite.o"

pngwtran.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "pngwtran.c" $(INC) -o "$(OBJROOT)/pngwtran.o"

pngwutil.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "pngwutil.c" $(INC) -o "$(OBJROOT)/pngwutil.o"

include ../../MakeInc.dir

# dependencies
-include $(OBJROOT)/Makedep
