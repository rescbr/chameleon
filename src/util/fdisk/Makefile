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

$(PROGRAM): $(ACTUAL_OBJECTS)
	@echo "\t[LD:${ARCH}] $(@F)"
	@$(TARGET_CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch ${ARCH} -o $(SYMROOT)/$(@F).${ARCH} $(ACTUAL_OBJECTS)

	@#echo "\t[LIPO] $(@F)"
	@#lipo -create -arch i386 $(SYMROOT)/$(@F).i386 -arch ppc $(SYMROOT)/$(@F).ppc -output $(SYMROOT)/$(@F)
	@#$(RM) $(SYMROOT)/$(@F).i386 $(SYMROOT)/$(@F).x86_64 $(SYMROOT)/$(@F).ppc

else
all:
endif


#dependencies
-include $(OBJROOT)/Makedep

