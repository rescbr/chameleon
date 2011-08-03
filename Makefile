#	Makefile for kernel booter
ROOT = $(shell pwd)
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj
SYMROOT = $(ROOT)/sym
DSTROOT = $(ROOT)/dst
DOCROOT = $(ROOT)/doc

include Make.rules

ARCHS="i386 ppc arm"

THEME = default

REVISION = `svnversion -n | tr -d [:alpha:] > revision`
VERSION = `cat version`

SUBDIRS="src"

GENERIC_SUBDIRS = modules

#
# Currently builds for i386
#

all install: $(SYMROOT) $(OBJROOT) rebuild_config
	@echo ================= make $@ =================

	@for i in ${SUBDIRS}; 						  \
	do 											  \
	    if [ -d $$i ]; then						  \
		echo ================= make $@ for $$i =================; \
		( ROOT=$(ROOT);						  	  \
		  ARCHS=$(ARCHS)						  \
 		  cd $$i; ${MAKE}					  	  \
			$@			  						  \
		) || exit $$?; 						  	  \
	    else							  		  \
	    	echo "========= nothing to build for $$i =========";	  \
	    fi;								  		  \
	done