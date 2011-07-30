ROOT = $(shell pwd)/../../
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj/i386
SYMROOT = $(ROOT)/sym/i386
DSTROOT = $(ROOT)/dst/usr/standalone/i386
DOCROOT = $(ROOT)/doc

DIR = libsa
include ${ROOT}/Make.rules

LIBSAIODIR = ../libsaio

CFLAGS	:= $(CFLAGS) $(RC_CFLAGS) $(MORECPP) -arch i386 \
	      -fno-builtin -static $(OMIT_FRAME_POINTER_CFLAG) \
	      -mpreferred-stack-boundary=2 -fno-align-functions -fno-stack-protector \
	      -march=pentium4 -msse2 -mfpmath=sse -msoft-float \
	      -nostdinc -include $(SRCROOT)/autoconf.h

CPPFLAGS := $(CPPFLAGS) -nostdinc++

INC = -I. -I$(SYMROOT) -I$(LIBSAIODIR) -I${SRCROOT}/i386/include

SA_OBJS = prf.o printf.o zalloc.o \
	string.o strtol.o error.o \
	setjmp.o qsort.o efi_tables.o

LIBS = libsa.a
LIBS := $(addprefix $(SYMROOT)/, $(LIBS))

DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all embedtheme: $(DIRS_NEEDED) $(LIBS)


$(LIBS): $(addprefix $(OBJROOT)/, $(SA_OBJS))
	@echo "\t[RM] $@"
	@rm -f $@
	@echo "\t[AR] $(@F)"
	@ar q $@ $^ &> /dev/null
	@echo "\t[RANLIB] $(@F)"
	@ranlib $@

# dependencies
-include $(OBJROOT)/Makedep
