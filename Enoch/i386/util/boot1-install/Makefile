SRCROOT = $(abspath $(CURDIR)/../../..)
OBJROOT = $(SRCROOT)/obj/i386/util/boot1-install
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

DIR = boot1-install

include ${SRCROOT}/Make.rules

LDFLAGS := $(LDFLAGS) -mmacosx-version-min=10.5 \
-framework CoreFoundation \
-framework DiskArbitration \
-Wl,-no_source_version \
-Wl,-no_function_starts \
-Wl,-no_data_in_code_info \
-Wl,-no_version_load_command \
-Wl,-no_uuid \
-Wl,-no_dependent_dr_info

CFLAGS := $(CFLAGS) -mmacosx-version-min=10.5

OBJS =  boot1-install.o32 \
        boot1-install.o64

OBJS := $(addprefix $(OBJROOT)/, $(OBJS))

PROGRAM = boot1-install
PROGRAM:= $(addprefix $(SYMROOT)/, $(PROGRAM))

ifeq ($(CONFIG_BOOT1INSTALL),y)

all: $(SYMROOT) $(OBJROOT) $(PROGRAM)

$(PROGRAM): $(OBJS)
	@echo "\t[LD32] $(@F)_32"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch i386 -o $@_32 $(filter %.o32,$^)
	@echo "\t[LD64] $(@F)_64"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -arch x86_64 -o $@_64 $(filter %.o64,$^)
	@echo "\t[LIPO] $@"
	@lipo -create -arch i386 $@_32 -arch x86_64 $@_64 -output $@
	@strip $@
	@rm $@_32 $@_64

else

all:

endif


#dependencies
-include $(OBJROOT)/Makedep

clean-local:
	@for o in $(OBJS); do if [ -f "$${o}" ];then echo "	[RM] $${o}"; fi; done
	@for p in $(SYMPROG); do if [ -f "$${p}" ];then echo "	[RM] $${p}"; fi; done
	@rm -f $(SYMPROG) $(OBJS)
