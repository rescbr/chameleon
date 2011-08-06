ROOT = $(shell pwd)/../../../../
DIR = cdboot
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj/${ARCH}/${DIR}
SYMROOT = $(ROOT)/sym/${ARCH}
DSTROOT = $(ROOT)/dst/usr/standalone/${ARCH}/
DOCROOT = $(ROOT)/doc

include ${ROOT}/Make.rules

DIRS_NEEDED = $(SYMROOT)

all: $(DIRS_NEEDED) $(SYMROOT)/cdboot

$(SYMROOT)/cdboot:
	@echo "\t[NASM] cdboot.s"	
	@$(NASM) cdboot.s -o $(SYMROOT)/cdboot
	@dd if=$(SYMROOT)/boot of=$(SYMROOT)/cdboot conv=sync bs=2k seek=1 &> /dev/null

	@# Update cdboot with boot file size info
	@stat -f %z $(SYMROOT)/boot \
		| perl -ane "print pack('V',@F[0]);" \
		| dd of=$(SYMROOT)/cdboot bs=1 count=4 seek=2044 conv=notrunc &> /dev/null

#dependencies
