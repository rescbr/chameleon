ROOT = $(shell pwd)/../../../../
DIR = libsa
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj/i386/${DIR}
SYMROOT = $(ROOT)/sym/i386
DSTROOT = $(ROOT)/dst/usr/standalone/i386/
DOCROOT = $(ROOT)/doc

LIBSAIODIR = ../libsaio

CFLAGS	:= $(RC_CFLAGS) $(OPTIM) $(MORECPP) -static -g -Wmost -Werror \
		-fno-builtin -DSAIO_INTERNAL_USER $(OMIT_FRAME_POINTER_CFLAG) \
		-fno-align-functions -fno-stack-protector \
		-msoft-float -nostdinc -include $(SRCROOT)/autoconf.h
	      
CPPFLAGS := $(CPPFLAGS) -static -nostdinc++ -Wmost -Werror \
	-fno-builtin -msoft-float \
	-fno-rtti -fno-exceptions \
	-include $(SRCROOT)/autoconf.h


INC = -I. -I$(SYMROOT) -I$(LIBSAIODIR) -I${SRCROOT}/include

OBJECTS = prf \
	error \
	setjmp efi_tables

LIBS = libsa.a
LIBS := $(addprefix $(SYMROOT)/, $(LIBS))


DIRS_NEEDED = 

include ${ROOT}/Make.rules


all: $(OBJROOT) $(SYMROOT) $(LIBS)

$(LIBS): ${ACTUAL_OBJECTS}
	@echo "\t[RM] $@"
	@rm -f $@
	@echo "\t[AR] $(@F)"
	@ar q $@ $^ &> /dev/null
	@echo "\t[RANLIB] $(@F)"
	@ranlib $@

# dependencies
-include $(OBJROOT)/Makedep
