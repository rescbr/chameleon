ROOT = $(shell pwd)/../../../../
DIR = boot2
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj/i386/${DIR}
SYMROOT = $(ROOT)/sym/i386
DSTROOT = $(ROOT)/dst/usr/standalone/i386/
DOCROOT = $(ROOT)/doc

include ${ROOT}/Make.rules

OPTIM =  -Oz

LIBSADIR = ../libsa
LIBSAIODIR = ../libsaio

INC = -I. -I$(SYMDIR) -I$(LIBSADIR) -I$(LIBSAIODIR) -I${SRCROOT}/i386/include

LIBS= -L$(SYMDIR) -lsaio -lsa
LIBDEP= libsaio.a libsa.a

OTHER_FILES =

OBJECTS = boot2 boot mboot boot_modules

UTILDIR = ../util
DIRS_NEEDED = $(OBJROOT) $(SYMROOT)
BOOT2ADDR = 20200
MAXBOOTSIZE = 458240


#
# Strings used to retrieve the start location for the Symbols.dylib module
#
SYMBOLS_MODULE = Symbols.dylib
SYMBOL_START= _symbols_module_start
SYMBOL_ADDR = $(shell printf "%d" 0x`nm -s __DATA __data $(SYMROOT)/boot.sys | grep " $(SYMBOL_START)$$" | cut -f 1 -d " "`)
DATA_OFFSET = $(shell otool -l $(SYMROOT)/boot.sys | grep __data -A 4  | grep __DATA -A 3 | tail -n 1 | cut -f 6 -d " ")
DATA_ADDR   = $(shell printf "%d" `otool -l $(SYMROOT)/boot.sys | grep __data -A 4  | grep __DATA -A 3 | head -n 2 | tail -n 1 | cut -f 8 -d " "`)
PATCH_ADDR  = $(shell echo ${SYMBOL_ADDR}-${DATA_ADDR}+${DATA_OFFSET} | bc)


all: $(DIRS_NEEDED) boot 


boot: $(SYMROOT)/embedded.h $(ACTUAL_OBJECTS) $(addprefix $(SYMROOT)/, $(LIBDEP))


	@${MKDIRS} $(OBJROOT)/../boot2_modules/
	@echo "\t[LD] boot.sys"
	@$(CC) -Wl,-segaddr,__INIT,$(BOOT2ADDR) -Wl,-segalign,20 -Wl,-preload \
			-nostdlib -arch i386 -static \
			-o $(SYMROOT)/boot.sys $(filter %.o,$^) `find $(OBJROOT)/../boot2_modules/ -name \*.o`  $(LIBS) -lcc_kext

ifeq (${CONFIG_MODULES}, y)

	@cp  $(SYMROOT)/boot.sys  $(SYMROOT)/boot2.sys

	@# Generate the Symbols.dylib file
	@echo "\t[dyldsymboltool] Symbols.dylib"
	@$(SYMROOT)/dyldsymboltool $(SYMROOT)/boot.sys $(SYMROOT)/${SYMBOLS_MODULE}

	@echo "\t[LD] boot.sys"
	@$(CC) -static -Wl,-preload -Wl,-segaddr,__INIT,$(BOOT2ADDR) \
	        -nostdlib -arch i386 -Wl,-segalign,20 \
		-Wl,-sectcreate,__DATA,__Symbols,$(SYMROOT)/Symbols.dylib \
		-o $(SYMROOT)/boot.sys $(filter %.o,$^) `find $(OBJROOT)/../boot2_modules/ -name \*.o` $(LIBS)  -lcc_kext

	@# Second pass, fixup global var locations
	@${RM} $(SYMROOT)/${SYMBOLS_MODULE}

	@# Generate the Symbols.dylib file
	@echo "\t[dyldsymboltool] Symbols.dylib"
	@$(SYMROOT)/dyldsymboltool $(SYMROOT)/boot.sys $(SYMROOT)/${SYMBOLS_MODULE}

	@${RM} $(SYMROOT)/boot.sys
	@echo "\t[LD] boot.sys"
	@$(CC) -static -Wl,-preload -Wl,-segaddr,__INIT,$(BOOT2ADDR) \
	        -nostdlib -arch i386 -Wl,-segalign,20 \
		-Wl,-sectcreate,__DATA,__Symbols,$(SYMROOT)/Symbols.dylib \
		-o $(SYMROOT)/boot.sys $(filter %.o,$^) `find $(OBJROOT)/../boot2_modules/ -name \*.o` $(LIBS)  -lcc_kext

	@${RM} $(SYMROOT)/${SYMBOLS_MODULE}

	@$(LD) -arch i386 \
	-undefined dynamic_lookup \
	-dylib -read_only_relocs suppress \
	-S -x -Z -dead_strip_dylibs \
	-no_uuid \
	$(filter %.o,$^) $(LIBS) \
	-final_output Symbols \
	-macosx_version_min 10.6 \
	-o $(OBJROOT)/Symbols_LINKER_ONLY.dylib

endif

	@# this is done in a sub process after boot.sys exists so the strings are populated correctly
	@make embed_symbols

	@${RM} $(SYMROOT)/boot2.sys

	@( size=`ls -l $(SYMROOT)/boot | awk '{ print $$5}'` ; \
	  if expr "$$size" ">" "$(MAXBOOTSIZE)" > /dev/null ;\
	  then \
	   	echo "\t******* boot is $$size bytes *******"; \
	    echo "Booter executable larger than $(MAXBOOTSIZE) bytes" ;\
	    rm $(SYMROOT)/boot ;\
	    exit 1;\
	  else \
	  	echo "\t******* boot is $$size bytes *******"; \
	  fi)

embed_symbols:
ifeq (${CONFIG_MODULES}, y)
	@echo ================= Embedding Symbols.dylib =================
	@echo "\t[MACHOCONV] boot"
	@$(SYMROOT)/machOconv $(SYMROOT)/boot2.sys $(SYMROOT)/boot &> /dev/null

	@echo "\t******* Patching at $(PATCH_ADDR) ******"
	@stat -f%z $(SYMROOT)/boot | perl -ane "print pack('V',@F[0]);" | dd conv=notrunc of=${SYMROOT}/boot.sys bs=1 count=4 seek=$(PATCH_ADDR) &> /dev/null
endif
	@echo "\t[MACHOCONV] boot"
	@$(SYMROOT)/machOconv $(SYMROOT)/boot.sys $(SYMROOT)/boot


$(SYMROOT)/embedded.h:
	@cd $(SYMROOT)/../../doc && xxd -i BootHelp.txt > $(SYMROOT)/embedded.h


#dependencies
-include $(OBJROOT)/Makedep
