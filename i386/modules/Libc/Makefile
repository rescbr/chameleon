
MODULE_NAME = Libc
MODULE_VERSION = "1.0.0"
MODULE_COMPAT_VERSION = "1.0.0"
MODULE_START = _$(MODULE_NAME)_start
MODULE_DEPENDENCIES = 

BUNDLE_NAME = $(MODULE_NAME)
BUNDLE_EXEC = $(BUNDLE_NAME)
BUNDLE_LIBS = 
BUNDLE_INFO      = 
BUNDLE_ID      =  com.boot.LIBS.libsystem_c
BUNDLE_COPYRIGHT = 
BUNDLE_LICENCE   = 
BUNDLE_VERSION   = 1.0
BUNDLE_ICON      =
BUNDLE_SHORT_VERSION = 1
BUNDLE_CLASS = LIBSYSTEM_C
BUNDLE_CLASS_PROVIDER = LIBS

DIR = Libc

include ../../MakePaths.dir

OBJROOT=../../../obj/i386/modules/$(DIR)
SYMROOT=../../../sym/i386/modules/
DSTROOT=../../../dst/i386/modules/


UTILDIR = ../../util
LIBSADIR = ../../libsa
LIBSAIODIR = ../../libsaio
BOOT2DIR = ../../boot2
INCLUDE = ../include
STDIO_INCLUDE = ./stdio
GC_INCLUDE = ./gc/include

INSTALLDIR = $(DSTROOT)/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders/standalone

OPTIM = -Os -Oz
DEBUG = -DNOTHING
CFLAGS	= $(RC_CFLAGS) $(OPTIM) $(MORECPP) -arch i386 -g -Wall -fstack-protector-all \
    -D__ARCHITECTURE__=\"i386\"  \
    $(DEBUG) \
    -fno-builtin $(OMIT_FRAME_POINTER_CFLAG) \
    -mpreferred-stack-boundary=2 -fno-align-functions \
    -march=pentium4 -msse2 -mfpmath=sse -msoft-float -fno-common -DNO_FLOATING_POINT -nostdinc

DEFINES=
CONFIG = hd
INC = -I. -I.. -I$(SYMROOT) -I$(UTILDIR) -I$(INCLUDE) -I$(STDIO_INCLUDE)
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

OBJS = __libc_init.o fclose.o fread.o fwrite.o fvwrite.o fflush.o fileno.o ungetc.o makebuf.o refill.o fwalk.o flags.o fopen.o findfp.o stdio.o fdopen.o wsetup.o puts.o stack_protector.o vfprintf.o fprintf.o printf-pos.o#fseek.o ftell.o #
OBJS += abort.o div.o bsearch.o exit.o abs.o atexit.o labs.o ldiv.o llabs.o lldiv.o reallocf.o
OBJS += errno.o

#OBJS += timeval_diff.o atol.o strtol.o sbrk.o

#OBJS += arena.o proto.o util.o malloc_zone.o zone_alloc.o 
#OBJS += allchblk.o alloc.o backgraph.o blacklst.o darwin_stop_world.o dbg_mlc.o finalize.o gc_dlopen.o gcj_mlc.o headers.o mach_dep.o malloc.o mallocx.o mark.o mark_rts.o misc.o new_hblk.o obj_map.o os_dep.o pcr_interface.o pthread_start.o pthread_stop_world.o \
		pthread_support.o ptr_chck.o real_malloc.o reclaim.o specific.o stubborn.o thread_local_alloc.o typd_mlc.o win32_threads.o

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
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/fclose.c" $(INC) -o "$(OBJROOT)/fclose.o"

fread.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/fread.c" $(INC) -o "$(OBJROOT)/fread.o"

fwrite.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/fwrite.c" $(INC) -o "$(OBJROOT)/fwrite.o"        

fvwrite.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/fvwrite.c" $(INC) -o "$(OBJROOT)/fvwrite.o"

ungetc.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/ungetc.c" $(INC) -o "$(OBJROOT)/ungetc.o"

makebuf.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/makebuf.c" $(INC) -o "$(OBJROOT)/makebuf.o"

refill.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/refill.c" $(INC) -o "$(OBJROOT)/refill.o"

fwalk.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/fwalk.c" $(INC) -o "$(OBJROOT)/fwalk.o"

flags.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/flags.c" $(INC) -o "$(OBJROOT)/flags.o"

fopen.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/fopen.c" $(INC) -o "$(OBJROOT)/fopen.o"

findfp.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/findfp.c" $(INC) -o "$(OBJROOT)/findfp.o"

stdio.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/stdio.c" $(INC) -o "$(OBJROOT)/stdio.o"        

fdopen.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/fdopen.c" $(INC) -o "$(OBJROOT)/fdopen.o"

wsetup.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/wsetup.c" $(INC) -o "$(OBJROOT)/wsetup.o"

puts.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/puts.c" $(INC) -o "$(OBJROOT)/puts.o"

ftell.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/ftell.c" $(INC) -o "$(OBJROOT)/ftell.o"

fseek.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/fseek.c" $(INC) -o "$(OBJROOT)/fseek.o"

fprintf.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/fprintf.c" $(INC) -o "$(OBJROOT)/fprintf.o"

vfprintf.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/vfprintf.c" $(INC) -o "$(OBJROOT)/vfprintf.o"

fflush.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/fflush.c" $(INC) -o "$(OBJROOT)/fflush.o"

fileno.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/fileno.c" $(INC) -o "$(OBJROOT)/fileno.o"

printf-pos.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdio/printf-pos.c" $(INC) -o "$(OBJROOT)/printf-pos.o"

abort.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdlib/abort.c" $(INC) -o "$(OBJROOT)/abort.o"

div.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdlib/div.c" $(INC) -o "$(OBJROOT)/div.o"

bsearch.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdlib/bsearch.c" $(INC) -o "$(OBJROOT)/bsearch.o"        

exit.o: 	
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdlib/exit.c" $(INC) -o "$(OBJROOT)/exit.o"

abs.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdlib/abs.c" $(INC) -o "$(OBJROOT)/abs.o"

atexit.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdlib/atexit.c" $(INC) -o "$(OBJROOT)/atexit.o"

labs.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdlib/labs.c" $(INC) -o "$(OBJROOT)/labs.o"

ldiv.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdlib/ldiv.c" $(INC) -o "$(OBJROOT)/ldiv.o"

llabs.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdlib/llabs.c" $(INC) -o "$(OBJROOT)/llabs.o"

lldiv.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdlib/lldiv.c" $(INC) -o "$(OBJROOT)/lldiv.o"

reallocf.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "stdlib/reallocf.c" $(INC) -o "$(OBJROOT)/reallocf.o"
	
errno.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "sys/errno.c" $(INC) -o "$(OBJROOT)/errno.o"

__libc_init.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "__libc_init.c" $(INC) -o "$(OBJROOT)/__libc_init.o"

include ../../MakeInc.dir

# dependencies
-include $(OBJROOT)/Makedep
