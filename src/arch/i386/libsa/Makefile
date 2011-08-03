ROOT = $(shell pwd)/../../../../
DIR = libsa
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj/i386/${DIR}
SYMROOT = $(ROOT)/sym/i386
DSTROOT = $(ROOT)/dst/usr/standalone/i386/
DOCROOT = $(ROOT)/doc

LIBSAIODIR = ../libsaio


INC = -I. -I$(SYMROOT) -I$(LIBSAIODIR) -I${SRCROOT}/include

OBJECTS = prf printf zalloc \
	string strtol error \
	setjmp qsort efi_tables

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
