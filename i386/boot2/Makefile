
#	Makefile for i386 boot program
#	define FLOPPY and SMALL using DEFINES macro as necessary

PNGCRUSH = `which pngcrush`
PNGOPTIONS = -rem cHRM -rem gAMA -rem iCCP -rem sRGB

DIR = boot2
include ../MakePaths.dir

OPTIM = -Os -Oz
CFLAGS	= $(RC_CFLAGS) $(OPTIM) $(MORECPP) -arch i386 -g -Wmost -Werror -fno-stack-protector \
		-fno-builtin -DSAIO_INTERNAL_USER -static $(OMIT_FRAME_POINTER_CFLAG) \
		-mpreferred-stack-boundary=2 -fno-align-functions \
		-march=pentium4 -msse2 -mfpmath=sse -msoft-float  

DEFINES=
CONFIG = hd
SYMDIR = $(SYMROOT)
LIBSADIR = ../libsa
LIBSAIODIR = ../libsaio
UTILDIR = ../util
INC = -I. -I.. -I$(SYMDIR) -I$(LIBSADIR) -I$(LIBSAIODIR) -I$(UTILDIR)
ifneq "" "$(wildcard /bin/mkdirs)"
  MKDIRS = /bin/mkdirs
else
  MKDIRS = /bin/mkdir -p
endif
AS = as
LD = gcc
# LIBS= -lc_static
LIBS= -L$(SYMDIR) -lsaio -lsa
LIBDEP= $(SYMDIR)/libsaio.a $(SYMDIR)/libsa.a

OTHER_FILES =

INSTALLDIR = $(DSTROOT)/usr/standalone/i386
VPATH = $(OBJROOT):$(SYMROOT)

# The ordering is important;
# boot2.o must be first.
OBJS = boot2.o boot.o graphics.o drivers.o prompt.o options.o mboot.o 	
# button.o browser.o scrollbar.o == NOTYET	

UTILDIR = ../util
SFILES = boot2.s
CFILES = boot.c graphics.c drivers.c prompt.c options.c mboot.c
HFILES = boot.h appleClut8.h mboot.h multiboot.h
OTHERFILES = Makefile
ALLSRC = $(FOREIGNSRC) $(FOREIGNBIN) $(SFILES) $(CFILES) \
	$(HFILES) $(OTHERFILES)
DIRS_NEEDED = $(OBJROOT) $(SYMROOT)
BOOT2ADDR = 20200
MAXBOOTSIZE = 458240


#
# Strings used to retrieve the start location for the Symbols.dylib module
#
SYMBOLS_MODULE = Symbols.dylib
SYMBOL_START= _symbols_module_start
SYMBOL_ADDR = $(shell printf "%d" 0x`nm -s __DATA __data $(SYMROOT)/boot_embeded.sys | grep " $(SYMBOL_START)$$" | cut -f 1 -d " "`)
DATA_OFFSET = $(shell otool -l $(SYMROOT)/boot_embeded.sys | grep __data -A 4  | grep __DATA -A 3 | tail -n 1 | cut -f 6 -d " ")
DATA_ADDR   = $(shell printf "%d" `otool -l $(SYMROOT)/boot_embeded.sys | grep __data -A 4  | grep __DATA -A 3 | head -n 2 | tail -n 1 | cut -f 8 -d " "`)
PATCH_ADDR  = $(shell echo ${SYMBOL_ADDR}-${DATA_ADDR}+${DATA_OFFSET} | bc)

#CFLAGS += -DSAFE_MALLOC 

# CFLAGS += -DBOOT_HELPER_SUPPORT # +992 bytes

# CFLAGS += -DNBP_SUPPORT

all embedtheme: $(DIRS_NEEDED) boot

boot: embedded.h machOconv $(OBJS) $(LIBDEP) 
	@echo "\t[LD] boot.sys"
	@$(LD)   -static -Wl,-preload -Wl,-segaddr,__INIT,$(BOOT2ADDR) \
	        -nostdlib -arch i386 -Wl,-segalign,20 \
		-o $(SYMROOT)/boot.sys $(filter %.o,$^) $(LIBS) #-e boot2		
	@echo "\t[MACHOCONV] boot"
	@$(SYMROOT)/machOconv $(SYMROOT)/boot.sys $(SYMROOT)/boot &> /dev/null
	
	@make Symbols.dylib	

	@##echo "\t[LD] boot_embeded.sys"
	@##$(LD) -static -Wl,-preload -Wl,-segaddr,__INIT,$(BOOT2ADDR) \
	        -nostdlib -arch i386 -Wl,-segalign,20 \
		-Wl,-sectcreate,__DATA,__Symbols,$(SYMROOT)/Symbols.dylib \
		-o $(SYMROOT)/boot_embeded.sys $(filter %.o,$^) $(LIBS) -lcc_kext	
	@##make embed_symbols		# this is done in a sub process after boot.sys exists so the strings are populated correctly

	@##${RM} $(SYMROOT)/${SYMBOLS_MODULE}

	@##size $(SYMROOT)/boot.sys
	@ls -l $(SYMROOT)/boot
	@( size=`ls -l $(SYMROOT)/boot | awk '{ print $$5}'` ; \
	  if expr "$$size" ">" "$(MAXBOOTSIZE)" > /dev/null ;\
	  then \
	    echo "Booter executable larger than $(MAXBOOTSIZE) bytes" ;\
	    rm $(SYMROOT)/boot ;\
	    exit 1;\
	  fi)

embed_symbols: machOconv
	@echo ================= Embedding Symbols.dylib =================	
	@echo ******* Patching at $(PATCH_ADDR) ******
	@stat -f%z $(SYMROOT)/boot | perl -ane "print pack('V',@F[0]);" | dd conv=notrunc of=${SYMROOT}/boot_embeded.sys bs=1 count=4 seek=$(PATCH_ADDR)	 &> /dev/null
	@echo "\t[MACHOCONV] boot_embeded"
	@$(SYMROOT)/machOconv $(SYMROOT)/boot_embeded.sys $(SYMROOT)/boot_embeded

embedded.h:
	@cd $(SYMROOT)/../../doc && xxd -i BootHelp.txt > $(SYMROOT)/embedded.h

Symbols.dylib: Symbols.o
	@echo ================= Compiling ${SYMBOLS_MODULE} =================
	@echo "start" >>  ${OBJROOT}/Symbols.save
	@echo "_lookup_symbol" >>  ${OBJROOT}/Symbols.save

	@echo "\t[LD] $@"
	@ld -arch i386 \
	-undefined dynamic_lookup \
	-alias _Symbols_start start \
	-dylib -read_only_relocs suppress \
	-S -x  -dead_strip_dylibs \
	-no_uuid \
	-bind_at_load \
	-current_version 1.0.0 \
	-compatibility_version 1.0.0 \
	-final_output Symbols \
	-macosx_version_min 10.6 \
	-exported_symbols_list ${OBJROOT}/Symbols.save \
	${OBJROOT}/Symbols.o \
	-o $(SYMROOT)/${SYMBOLS_MODULE} 

	@##size $(SYMROOT)/${SYMBOLS_MODULE} 

Symbols.o: Symbols.plist
	@rm -rf $(SYMROOT)/Symbols.h
	@echo "typedef struct {" >> $(SYMROOT)/Symbols.h
	@echo "	char*	symbol;" >> $(SYMROOT)/Symbols.h
	@echo "	unsigned int	addr;" >> $(SYMROOT)/Symbols.h
	@echo "} symbol_t;" >> $(SYMROOT)/Symbols.h 
	@echo "" >> $(SYMROOT)/Symbols.h
	
	@nm -g $(SYMROOT)/boot.sys | tr . _ | awk '{print "static char "$$3"_string[] = \""$$3"\";"}' >> $(SYMROOT)/Symbols.h

	@echo "symbol_t symbolList[] = {" >> $(SYMROOT)/Symbols.h
	@nm -g $(SYMROOT)/boot.sys | tr . _ | awk '{print "	{.symbol = "$$3"_string, .addr = 0x"$$1"},";}' >> $(SYMROOT)/Symbols.h
	@echo "};" >> $(SYMROOT)/Symbols.h
	@echo "\t[CC] $@"
	@$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c $(UTILDIR)/Symbols.c $(INC) -o $(OBJROOT)/Symbols.o 

Symbols.plist: 
	@rm -rf $(SYMROOT)/Symbols.plist
	@echo "<?xml version="1.0" encoding="UTF-8"?>" >> $(SYMROOT)/Symbols.plist
	@echo "<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd"> " >> $(SYMROOT)/Symbols.plist
	@echo "<plist version="1.0">" >> $(SYMROOT)/Symbols.plist
	@echo "<dict>" >> $(SYMROOT)/Symbols.plist 
	@echo "<key>IOKitPersonalities</key>" >> $(SYMROOT)/Symbols.plist
	@echo "<dict>" >> $(SYMROOT)/Symbols.plist
	@nm -g $(SYMROOT)/boot.sys | tr . _ | awk '{print "<key>"$$3"</key><string>"$$1"</string>"}' >> $(SYMROOT)/Symbols.plist
	@echo "</dict>" >> $(SYMROOT)/Symbols.plist
	@echo "</dict>" >> $(SYMROOT)/Symbols.plist
	@echo "</plist>" >> $(SYMROOT)/Symbols.plist
	@plutil -convert xml1 $(SYMROOT)/Symbols.plist 
    

install_i386:: all $(INSTALLDIR)
	cp $(SYMROOT)/boot $(OTHER_FILES) $(INSTALLDIR)
	cd $(INSTALLDIR); chmod u+w boot $(OTHER_FILES)

include ../MakeInc.dir

#dependencies
-include $(OBJROOT)/Makedep
