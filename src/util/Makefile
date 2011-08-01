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
OBJECTS = dyldsymboltool machOconv

ifeq (${CONFIG_BDMESG}, y)
PROGRAMS += bdmesg
OBJECTS += bdmesg
endif

ifeq (${CONFIG_OPENUP}, y)
PROGRAMS += openUp
OBJECTS += openUp
endif


LDFLAGS := $(LDFALGS) -framework IOKit -framework CoreFoundation -mmacosx-version-min=10.4

SYMPROG = $(addprefix $(SYMROOT)/, $(PROGRAMS))

DIRS_NEEDED = $(OBJROOT) $(SYMROOT)


include ${ROOT}/Make.rules



all: $(DIRS_NEEDED) $(SYMPROG) fdisk440



$(SYMPROG): ${ACTUAL_OBJECTS}
	@echo "\t[LD:I386] $(@F)"
	@$(X86_CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch i386 -o $(SYMROOT)/$(@F).i386 $(OBJROOT)/$(@F).i386o
	@echo "\t[LD:X86_64] $(@F)"
	@$(X86_CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch x86_64 -o $(SYMROOT)/$(@F).x86_64 $(OBJROOT)/$(@F).x86_64o
	@echo "\t[LD:PPC] $(@F)"
	@$(PPC_CC) $(CFLAGS) $(PPC_LDFLAGS) $(LDFLAGS) $(DEFINES) -arch ppc -o $(SYMROOT)/$(@F).ppc $(OBJROOT)/$(@F).ppco
	@#echo "\t[LD:ARM] $(@F)"
	@#$(ARM_CC) $(CFLAGS) $(ARM_LDFLAGS) $(LDFLAGS) $(DEFINES) -arch arm -o $(SYMROOT)/$(@F).arm $(OBJROOT)/$(@F).armo


	@echo "\t[LIPO] $(@F)"
	@lipo -create -arch i386 $(SYMROOT)/$(@F).i386 -arch x86_64 $(SYMROOT)/$(@F).x86_64 -arch ppc $(SYMROOT)/$(@F).ppc -output $(SYMROOT)/$(@F)
	@$(RM) $(SYMROOT)/$(@F).i386 $(SYMROOT)/$(@F).x86_64 $(SYMROOT)/$(@F).ppc

.PHONY: fdisk440
fdisk440:
	@echo ================= make all for fdisk =================;
	@$(MAKE) -C fdisk


#dependencies
-include $(OBJROOT)/Makedep

