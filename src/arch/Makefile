#	Makefile for i386 boot program

#
# these paths are only valid in subdirectories of this directory
#
ROOT = $(shell pwd)/../../
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj/${ARCH}
SYMROOT = $(ROOT)/sym/${ARCH}
DSTROOT = $(ROOT)/dst/usr/standalone/${ARCH}
DOCROOT = $(ROOT)/doc

include ${ROOT}/Make.rules

# The order of building is important.
SUBDIRS = ${ARCH}

all install: ${OBJROOT} ${SYMROOT}
	@for i in ${SUBDIRS};						  \
	do								  \
		echo ================= make $@ for $$i =================; \
		(							  \
			cd $$i; ${MAKE}	ARCHS="${ARCHS}" ARCH=${ARCH}	  \
			$@						  \
		) || exit $$?;						  \
	done