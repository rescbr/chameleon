ROOT = $(shell pwd)/../../../../
DIR = libsaio
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj/i386/${DIR}
SYMROOT = $(ROOT)/sym/i386
DSTROOT = $(ROOT)/dst/usr/standalone/i386/
DOCROOT = $(ROOT)/doc

UTILDIR = ../util
LIBSADIR = ../libsa
BOOT2DIR = ../boot2

DEFINES = -DNOTHING

INC = -I. -I$(SYMROOT) -I$(LIBSADIR) -I$(BOOT2DIR) -I${SRCROOT}/i386/include

OBJECTS = table asm bios biosfn \
			misc xml load console

LIBS  = libsaio.a
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
