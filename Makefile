#	Makefile for kernel booter
SRCROOT = $(shell pwd)
OBJROOT = $(SRCROOT)/obj
SYMROOT = $(SRCROOT)/sym
DSTROOT = $(SRCROOT)/dst
DOCROOT = $(SRCROOT)/../doc

include ${SRCROOT}/Make.rules

THEME = default

VERSION = `cat version`
REVISION = `cat revision`

SUBDIRS="src"

GENERIC_SUBDIRS = modules

#
# Currently builds for i386
#

all config rebuild_config: $(SYMROOT) $(OBJROOT) $(SRCROOT)/auto.conf $(SRCROOT)/autoconf.h $(SRCROOT)/autoconf.inc $(SRCROOT)/.config $(SYMROOT)/vers.h
	for i in $$SUBDIRS; 						  \
	do \
	    if [ -d $$i ]; then						  \
		echo ================= make $@ for $$i =================; \
		( OBJROOT=$(OBJROOT)/$${i};				  \
		  SYMROOT=$(SYMROOT)/$${i};				  \
		  DSTROOT=$(DSTROOT);					  \
		  SRCROOT=$(SRCROOT);					  \
		    cd $$i; ${MAKE}					  \
			"OBJROOT=$$OBJROOT"		 	  	  \
		  	"SYMROOT=$$SYMROOT"				  \
			"DSTROOT=$$DSTROOT"				  \
			"SRCROOT=$$SRCROOT"				  \
			"ARCHS=$$ARCHS"				  \
			"TARGET=$$i"					  \
			$@			  \
		) || exit $$?; 						  \
	    else							  \
	    	echo "========= nothing to build for $$i =========";	  \
	    fi;								  \
	done

$(SYMROOT)/vers.h: version
	@if [ -e ".svn" ]; then svnversion -n | tr -d [:alpha:] > revision; fi
	@echo "#define BOOT_VERSION \"5.0.132\"" > $@
	@echo "#define BOOT_BUILDDATE \"`date \"+%Y-%m-%d %H:%M:%S\"`\"" >> $@
	@echo "#define BOOT_CHAMELEONVERSION \"$$VERSION\"" >> $@
	@echo "#define BOOT_CHAMELEONREVISION \"$$REVISION\"" >> $@


.PHONY: $(SYMROOT)/vers.h
.PHONY: config

