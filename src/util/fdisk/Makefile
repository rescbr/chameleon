ROOT = $(shell pwd)/../../../
DIR = fdisk
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj/util/${DIR}
SYMROOT = $(ROOT)/sym/util
DSTROOT = $(ROOT)/dst/usr/bin/
DOCROOT = $(ROOT)/doc

OBJECTS =  cmd disk fdisk getrawpartition mbr misc opendev part user auto

LDFLAGS := $(LDFALGS) -mmacosx-version-min=10.5
PROGRAM = fdisk440
PROGRAM:= $(addprefix $(SYMROOT)/, $(PROGRAM))


include ${ROOT}/Make.rules

ifeq ($(CONFIG_FDISK440),y)

all: $(SYMROOT) $(OBJROOT) $(PROGRAM)

$(PROGRAM): $(NATIVE_OBJECTS)
	@echo "\t[LD:${ARCH}] $(@F)"
	@$(NATIVE_CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -o $(SYMROOT)/$(@F) $(NATIVE_OBJECTS)

else
all:
endif


#dependencies
-include $(OBJROOT)/Makedep

