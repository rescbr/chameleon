#	Makefile
ROOT = $(shell pwd)/../
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj
SYMROOT = $(ROOT)/sym
DSTROOT = $(ROOT)/dst
DOCROOT = $(ROOT)/doc

SUBDIRS=util modules arch/${ARCH}

all:
	@echo ================= make $@ BUILT_IN=yes for modules arch $(ARCH) =================
	@cd modules; ${MAKE} ARCH=${ARCH} BUILT_IN=yes $@
	@for i in ${SUBDIRS};										  \
	do															  \
		echo ================= make $@ for $$i arch $(ARCH) =================; \
		( ROOT=$(ROOT);										  	  \
			cd $$i; ${MAKE}	ARCH=${ARCH}							  \
			$@ 			  										  \
		) || exit $$?; 											  \
	done
	

include ${ROOT}/Make.rules
