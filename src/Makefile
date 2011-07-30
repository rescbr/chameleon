#	Makefile

SRCROOT = $(shell pwd)/../
OBJROOT = $(SRCROOT)/obj
SYMROOT = $(SRCROOT)/sym
DSTROOT = $(SRCROOT)/dst
DOCROOT = $(SRCROOT)/../doc

include ${SRCROOT}/Make.rules

config rebuild_config:
	@for i in config; \
	do \
		echo ================= make $@ for $$i =================; \
		( cd $$i; ${MAKE}					  \
			"OBJROOT=$(OBJROOT)/$$i"			  \
		  	"SYMROOT=$(SYMROOT)"				  \
			"DSTROOT=$(DSTROOT)"				  \
			"SRCROOT=$(SRCROOT)"				  \
			"RC_ARCHS=$(RC_ARCHS)"				  \
			"RC_KANJI=$(RC_KANJI)"				  \
			"JAPANESE=$(JAPANESE)"				  \
			"RC_CFLAGS=$(RC_CFLAGS)" $@ 			  \
		) || exit $$?; 						  \
	done


.PHONY: config
.PHONY: rebuild_config