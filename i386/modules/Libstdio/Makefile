
MODULE_NAME = Libstdio
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = _$(MODULE_NAME)_start
MODULE_DEPENDENCIES = 

BUNDLE_NAME = $(MODULE_NAME)
BUNDLE_EXEC = $(BUNDLE_NAME)
BUNDLE_LIBS = 
BUNDLE_INFO      = 
BUNDLE_ID      =  com.boot.LIBS.libstdio
BUNDLE_COPYRIGHT = 
BUNDLE_LICENCE   = 
BUNDLE_VERSION   = 1.0
BUNDLE_ICON      =
BUNDLE_SHORT_VERSION = 1
BUNDLE_CLASS = LIBSTDIO
BUNDLE_CLASS_PROVIDER = LIBS

DIR = Libstdio

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
    -march=pentium4 -msse2 -mfpmath=sse -msoft-float -fno-common -DNO_FLOATING_POINT

DEFINES=
CONFIG = hd
INC = -I. -I.. -I$(SYMROOT) -I$(UTILDIR) -I$(LIBSADIR) -I$(LIBSAIODIR) -I$(INCLUDE) 
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

OBJS = fclose.o fread.o fwrite.o fvwrite.o fflush.o  ungetc.o makebuf.o refill.o fwalk.o flags.o fopen.o findfp.o stdio.o fdopen.o wsetup.o puts.o stack_protector.o vfprintf.o fprintf.o printf-pos.o#fseek.o ftell.o #


SFILES =
CFILES = 
HFILES = 
EXPORTED_HFILES =
INSTALLED_HFILES =
OTHERFILES = Makefile
ALLSRC =  $(SFILES) $(CFILES) \
	$(HFILES) $(OTHERFILES)
DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all embedtheme: ${OBJS} bundle


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
	@plutil -convert xml1 $(SYMROOT)/$(BUNDLE_NAME).bundle/Contents/Info.plist

stack_protector.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "$(LIBSAIODIR)/stack_protector.c" $(INC) -o "$(OBJROOT)/stack_protector.o"        


fclose.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "fclose.c" $(INC) -o "$(OBJROOT)/fclose.o"

fread.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "fread.c" $(INC) -o "$(OBJROOT)/fread.o"

fwrite.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "fwrite.c" $(INC) -o "$(OBJROOT)/fwrite.o"        

fvwrite.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "fvwrite.c" $(INC) -o "$(OBJROOT)/fvwrite.o"

ungetc.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "ungetc.c" $(INC) -o "$(OBJROOT)/ungetc.o"

makebuf.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "makebuf.c" $(INC) -o "$(OBJROOT)/makebuf.o"

refill.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "refill.c" $(INC) -o "$(OBJROOT)/refill.o"

fwalk.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "fwalk.c" $(INC) -o "$(OBJROOT)/fwalk.o"

flags.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "flags.c" $(INC) -o "$(OBJROOT)/flags.o"

fopen.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "fopen.c" $(INC) -o "$(OBJROOT)/fopen.o"

findfp.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "findfp.c" $(INC) -o "$(OBJROOT)/findfp.o"

stdio.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio.c" $(INC) -o "$(OBJROOT)/stdio.o"        

fdopen.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "fdopen.c" $(INC) -o "$(OBJROOT)/fdopen.o"

wsetup.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "wsetup.c" $(INC) -o "$(OBJROOT)/wsetup.o"

puts.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "puts.c" $(INC) -o "$(OBJROOT)/puts.o"

ftell.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "ftell.c" $(INC) -o "$(OBJROOT)/ftell.o"

fseek.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "fseek.c" $(INC) -o "$(OBJROOT)/fseek.o"

fprintf.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "fprintf.c" $(INC) -o "$(OBJROOT)/fprintf.o"

vfprintf.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "vfprintf.c" $(INC) -o "$(OBJROOT)/vfprintf.o"

printf-pos.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "printf-pos.c" $(INC) -o "$(OBJROOT)/printf-pos.o"

include ../../MakeInc.dir

# dependencies
-include $(OBJROOT)/Makedep
