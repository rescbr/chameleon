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

PROGRAMS = machOconv macho-to-xcoff dyldsymboltool 
OBJECTS = machOconv macho-to-xcoff dyldsymboltool

ifeq (${CONFIG_BDMESG}, y)
PROGRAMS += bdmesg
OBJECTS += bdmesg
endif

ifeq (${CONFIG_OPENUP}, y)
PROGRAMS += openUp
OBJECTS += openUp
endif


LDFLAGS = $(TARGET_LDFLAGS) -framework IOKit -framework CoreFoundation -mmacosx-version-min=10.4

SYMPROG = $(addsuffix $(addprefix $(SYMROOT)/, $(PROGRAMS)), .${ARCH})

DIRS_NEEDED = 

include ${ROOT}/Make.rules


all: $(OBJROOT) $(SYMROOT) $(SYMPROG) fdisk440



$(SYMPROG): ${NATIVE_OBJECTS}
	@echo "\t[LD:${ARCH}] $(@F)"
	@$(TARGET_CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -o $(SYMROOT)/$(@F) $(OBJROOT)/$(@F).o

.PHONY: fdisk440
fdisk440:
	@echo ================= make all for fdisk =================;
	@$(MAKE) ARCH=${ARCH} -C fdisk


#dependencies
-include $(OBJROOT)/Makedep

