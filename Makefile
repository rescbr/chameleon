#	Makefile for kernel booter
ROOT = $(shell pwd)
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj
SYMROOT = $(ROOT)/sym
DSTROOT = $(ROOT)/dst
DOCROOT = $(ROOT)/doc

#default architechtures to compile.
ARCHS=i386 ppc armv5

THEME = default

SUBDIRS="src"


all install: $(SYMROOT) $(OBJROOT) rebuild_config
	@echo ================= make $@ =================
	@for a in ${ARCHS}; 						  	  \
	do												  \
		for i in ${SUBDIRS}; 						  \
		do 											  \
	    	if [ -d $$i ]; then						  \
			echo "================= make $@ for $$i arch $$a================="; \
			( 						  	  \
 		  	cd $$i; ${MAKE}	ROOT=$(ROOT) ARCHS="${ARCHS}" ARCH=$$a		  	  	  \
				$@			  						  \
			) || exit $$?; 						  	  \
	    	else							  		  \
	    		echo "========= nothing to build for $$i arch $$a=========";	  \
	    	fi;								  		  \
		done										  \
	done
	
	
include Make.rules
