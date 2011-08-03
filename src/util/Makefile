#
# Until I can remove the dependency on the appkit,
# we'll just keep the generated files in this directory
# and install them directly, rather than generating them again.
#

ROOT = $(shell pwd)/../../
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj/util
SYMROOT = $(ROOT)/sym/util
DSTROOT = $(ROOT)/dst/usr/bin/
DOCROOT = $(ROOT)/doc

DIR = util

PROGRAMS = machOconv dyldsymboltool
OBJECTS = machOconv dyldsymboltool

ifeq (${CONFIG_BDMESG}, y)
PROGRAMS += bdmesg
OBJECTS += bdmesg
endif

ifeq (${CONFIG_OPENUP}, y)
PROGRAMS += openUp
OBJECTS += openUp
endif


LDFLAGS := $(LDFALGS) -framework IOKit -framework CoreFoundation -mmacosx-version-min=10.4

SYMPROG = $(addpostfix $(addprefix $(SYMROOT)/, $(PROGRAMS)), ${ARCH})

DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

include ${ROOT}/Make.rules


all: $(DIRS_NEEDED) $(SYMPROG) fdisk440


$(SYMPROG).${ARCH}: ${ACTUAL_OBJECTS}
	@echo "\t[LD:${ARCH}] $(@F)"
	@$(TARGET_CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch ${ARCH} -o $(SYMROOT)/$(@F).${ARCH} $(OBJROOT)/$(@F).${ARCH}o

	@#echo "\t[LIPO] $(@F)"
	@#lipo -create -arch i386 $(SYMROOT)/$(@F).i386 -arch ppc $(SYMROOT)/$(@F).ppc -output $(SYMROOT)/$(@F)
	@#$(RM) $(SYMROOT)/$(@F).i386 $(SYMROOT)/$(@F).ppc

.PHONY: fdisk440
fdisk440:
	@echo ================= make all for fdisk =================;
	@$(MAKE) ARCH=${ARCH} -C fdisk


#dependencies
-include $(OBJROOT)/Makedep

