ROOT = $(shell pwd)/../../../../
DIR = boot1
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj/${ARCH}/${DIR}
SYMROOT = $(ROOT)/sym/${ARCH}
DSTROOT = $(ROOT)/dst/usr/standalone/${ARCH}/
DOCROOT = $(ROOT)/doc

include ${ROOT}/Make.rules

DIRS_NEEDED =

PROGRAMS  =  boot1hp boot1f32

ifeq (${CONFIG_BOOT1_HFS}, y)
PROGRAMS += boot1h
endif

ifeq (${CONFIG_BOOT1_HFS_ACTIVE}, y)
PROGRAMS += boot1he
endif


PROGRAMS := $(addprefix $(SYMROOT)/, $(PROGRAMS))

all:  $(SYMROOT) $(PROGRAMS)


$(PROGRAMS):
	@echo "\t[NASM] $(@F)" 
	@$(NASM) $(@F).s -o $@
