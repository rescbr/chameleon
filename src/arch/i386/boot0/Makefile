ROOT = $(shell pwd)/../../../../
DIR = boot0
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj/i386/${DIR}
SYMROOT = $(ROOT)/sym/i386
DSTROOT = $(ROOT)/dst/usr/standalone/i386/
DOCROOT = $(ROOT)/doc

include ${ROOT}/Make.rules

DIRS_NEEDED = 


BOOT0_OBJS  = boot0 boot0hfs boot0md chain0
BOOT0_OBJS := $(addprefix $(SYMROOT)/, $(BOOT0_OBJS))

all: $(SYMROOT) $(BOOT0_OBJS)

$(BOOT0_OBJS):
	@echo "\t[NASM] $(@F)"
	@$(NASM) $(@F).s -o $@

#dependencies

