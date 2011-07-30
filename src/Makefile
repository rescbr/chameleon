#	Makefile
ROOT = $(shell pwd)/../
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj
SYMROOT = $(ROOT)/sym
DSTROOT = $(ROOT)/dst
DOCROOT = $(ROOT)/doc

SUBDIRS=util modules i386 ppc arm

all:
	@echo ================= make $@ BUILT_IN=yes for modules =================
	@cd modules; ${MAKE} BUILT_IN=yes $@
	@for i in ${SUBDIRS};										  \
	do															  \
		echo ================= make $@ for $$i =================; \
		( ROOT=$(ROOT);										  	  \
			cd $$i; ${MAKE}										  \
			$@ 			  										  \
		) || exit $$?; 											  \
	done
	

include ${ROOT}/Make.rules
