ROOT = $(shell pwd)/../../../
DIR = libsa
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj/i386/${DIR}
SYMROOT = $(ROOT)/sym/util
DSTROOT = $(ROOT)/dst/usr/standalone/i386/
DOCROOT = $(ROOT)/doc

LIBSAIODIR = ../libsaio


CPPFLAGS := $(CPPFLAGS) -nostdinc++

INC = -I. -I$(SYMROOT) -I$(LIBSAIODIR) -I${SRCROOT}/include

OBJECTS = prf printf zalloc \
	string strtol error \
	setjmp qsort efi_tables

LIBS = libsa.a
LIBS := $(addprefix $(SYMROOT)/, $(LIBS))

include ${SRCROOT}/i386/Make.rules



DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all embedtheme: $(DIRS_NEEDED) $(LIBS)
$(warning $(ACTUAL_OBJECTS))

$(LIBS): ${ACTUAL_OBJECTS}
	@echo "\t[RM] $@"
	@rm -f $@
	@echo "\t[AR] $(@F)"
	@ar q $@ $^ &> /dev/null
	@echo "\t[RANLIB] $(@F)"
	@ranlib $@

$(DIRS_NEEDED) $(INSTALLDIR) $(OBJROOT) $(SYMROOT):
	@echo "\t[MKDIR] $@"
	@$(MKDIRS) $@


# dependencies
-include $(OBJROOT)/Makedep
