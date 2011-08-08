ROOT = $(shell pwd)/../../../../
DIR = boot2
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj/${ARCH}/${DIR}
SYMROOT = $(ROOT)/sym/${ARCH}
DSTROOT = $(ROOT)/dst/usr/standalone/${ARCH}/
DOCROOT = $(ROOT)/doc

INCLUDESDIR = ../include

CFLAGS	:= $(RC_CFLAGS) $(OPTIM) $(MORECPP) -static -g -Wmost -Werror \
		-fno-builtin -DSAIO_INTERNAL_USER $(OMIT_FRAME_POINTER_CFLAG) \
		-fno-align-functions -fno-stack-protector \
		-msoft-float -nostdinc -include $(SRCROOT)/autoconf.h
	      
CPPFLAGS := $(CPPFLAGS) -static -nostdinc++ -Wmost -Werror \
	-fno-builtin -msoft-float \
	-fno-rtti -fno-exceptions \
	-include $(SRCROOT)/autoconf.h


DEFINES = -DNOTHING

LIBSAIODIR = ../libclite

INCLUDESDIR = ../include
INC = -I. -I$(SYMROOT) -I$(INCLUDESDIR) -I${SRCROOT}/include


LIBS= -L$(SYMROOT)  -lclite
LIBDEP=  libclite.a

# enable boot2 to find boot_modules.c
VPATH = $(ROOT)/sym/modules/

OBJECTS = main boot ci_io ci Control2 MAC-PARTS sl_words boot_modules


DIRS_NEEDED = 

include ${ROOT}/Make.rules

all: $(OBJROOT) $(SYMROOT) ${SYMROOT}/boot

${SYMROOT}/boot: ${ACTUAL_OBJECTS}
	@echo "\t[LD] boot.sys"
	@$(CC)  -nostdlib -arch ${ARCH} -static -nostdlib -e _StartTVector -seg1addr 06600000 \
			-o $(SYMROOT)/boot.sys $(filter %.${ARCH}o,$^) `find $(ROOT)/obj/boot2_modules/ -name \*.${ARCH}o` $(LIBS)

	@# TODO: embed Symbols.dylib

	@$(SYMROOT)/../util/macho-to-xcoff $(SYMROOT)/boot.sys $(SYMROOT)/bootx.xcoff
	@cp bootinfo.hdr $(SYMROOT)/bootx.bootinfo
	@cat $(SYMROOT)/bootx.xcoff >> $(SYMROOT)/bootx.bootinfo

	@$(LD) -arch ${ARCH} \
	-undefined dynamic_lookup \
	-dylib -read_only_relocs suppress \
	-S -x -Z -dead_strip_dylibs \
	-no_uuid \
	$(filter %.${ARCH}o,$^) `find $(OBJROOT)/../../boot2_modules/ -name \*.${ARCH}o` $(LIBS) \
	-final_output Symbols \
	-macosx_version_min 10.6 \
	-o $(OBJROOT)/Symbols_LINKER_ONLY.dylib





install: ${DSTROOT} $(SYMROOT)/boot
	@echo "\t[CP] bootx.bootinfo"
	@cp $(SYMROOT)/bootx.bootinfo ${DSTROOT}
	@echo "\t[CP] bootx.xcoff"
	@cp $(SYMROOT)/bootx.xcoff ${DSTROOT}

	@# TODO: copy Symbols (linker) so that other can extend w/o having the source


# dependencies
-include $(OBJROOT)/Makedep
