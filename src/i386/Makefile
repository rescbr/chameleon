#	Makefile for i386 boot program

#
# these paths are only valid in subdirectories of this directory
#
ROOT = $(shell pwd)/../../
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj/i386
SYMROOT = $(ROOT)/sym/i386
DSTROOT = $(ROOT)/dst/usr/standalone/i386
DOCROOT = $(ROOT)/doc

include ${ROOT}/Make.rules

# The order of building is important.
SUBDIRS = libsa libsaio boot0 boot1 boot2 cdboot modules

all:
	@#cd modules; ${MAKE} BUILT_IN=yes $@


	@for i in ${SUBDIRS};										  \
	do															  \
		echo ================= make $@ for $$i =================; \
		( ROOT=$(ROOT);										  	  \
			cd $$i; ${MAKE}										  \
			$@ 			  										  \
		) || exit $$?; 											  \
	done

config rebuild_config:
	@for i in config; 											  \
	do															  \
		echo ================= make $@ for $$i =================; \
		( ROOT=$(ROOT);										  	  \
			cd $$i; ${MAKE}										  \
			$@ 			  										  \
		) || exit $$?; 											  \
	done


.PHONY: config