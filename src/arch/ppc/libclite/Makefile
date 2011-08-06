ROOT = $(shell pwd)/../../../../
DIR = libclite
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

INC = -I. -I$(SYMROOT) -I$(INCLUDESDIR) -I${SRCROOT}/include

OBJECTS = bswap

LIBS  = libclite.a
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
	@ranlib $(SYMROOT)/$(@F)


# dependencies
-include $(OBJROOT)/Makedep
