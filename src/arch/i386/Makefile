#	Makefile for i386 boot program

#
# these paths are only valid in subdirectories of this directory
#
ROOT = $(shell pwd)/../../../
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj/i386
SYMROOT = $(ROOT)/sym/i386
DSTROOT = $(ROOT)/dst/usr/standalone/i386
DOCROOT = $(ROOT)/doc

include ${ROOT}/Make.rules

# The order of building is important.
SUBDIRS = libsaio boot0 boot1 boot2 cdboot

all: ${OBJROOT} ${SYMROOT}
	@#cd modules; ${MAKE} BUILT_IN=yes $@


	@for i in ${SUBDIRS};						  \
	do								  \
		echo ================= make $@ for $$i =================; \
		( 							  \
			cd $$i; ${MAKE}					  \
			$@						  \
		) || exit $$?;						  \
	done