SRCROOT = $(abspath $(CURDIR)/../..)
OBJROOT = $(SRCROOT)/obj/i386/boot0
SYMROOT = $(SRCROOT)/sym/i386
DSTROOT = $(SRCROOT)/dst/i386
DOCROOT = $(SRCROOT)/doc
IMGROOT = $(SRCROOT)/sym/cache
IMGSKELROOT = $(SRCROOT)/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

DIR = boot0
include ${SRCROOT}/Make.rules

DIRS_NEEDED = $(SYMROOT)

OBJS  = boot0 boot0hfs boot0md chain0
OBJS := $(addprefix $(SYMROOT)/, $(OBJS))

all: $(DIRS_NEEDED) $(OBJS)

#To rebuild boot0 if boot0.s is touched
$(SYMROOT)/boot0: boot0.s

$(OBJS): $(SRCROOT)/autoconf.inc
	@echo "	[NASM] $(@F)"
	@$(NASM) $(@F).s -o $@ -O3

clean-local:
	@for o in $(OBJS); do if [ -f "$${o}" ];then echo "	[RM] $${o}"; fi; done
	@rm -f $(OBJS)
