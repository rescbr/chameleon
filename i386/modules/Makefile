
#	Makefile for i386 boot program
#	define FLOPPY and SMALL using DEFINES macro as necessary

CFLAGS	= -O $(MORECPP) -arch i386 -g -static
DEFINES=
CONFIG = hd
LIBDIR = libsa
INC = -I. -I$(LIBDIR)
ifneq "" "$(wildcard /bin/mkdirs)"
  MKDIRS = /bin/mkdirs
else
  MKDIRS = /bin/mkdir -p
endif
AS = as
LD = ld

#
# these paths are only valid in subdirectories of this directory
#
OBJROOT=`pwd`/../../obj/i386/modules/
SYMROOT=`pwd`/../../sym/i386/modules/
DSTROOT=`pwd`/../../dst/i386/modules/
SRCROOT=/tmp

VPATH = $(OBJROOT):$(SYMROOT)

# The order of building is important.
SUBDIRS = Libc Freetype1 uClibcxx lua Libz Libpng HelloWorld KernelPatcher USBFix Networking HPET NetbookInstaller CPUfreq Keymapper YellowIconFixer ACPICodec ACPIPatcher GraphicsEnabler Memory RamDiskLoader SMBiosPatcher SMBiosGetters GUI HibernateEnabler 
SUBDIRS += libpixman cairo libwebcore # graphics lib replacement (solution 1)
SUBDIRS += libefi libeg tryme # graphics lib replacement (solution 2)

all embedtheme tags debug install installhdrs:	
	@rm -rf $(OBJROOT)
	@mkdir $(OBJROOT)
	@for i in ${SUBDIRS}; \
	do \
		echo ================= make $@ for $$i =================; \
		( if test -d $$i; then cd $$i ; $(MKDIRS) $(OBJROOT)/$$i; ${MAKE}					  \
			"OBJROOT=$(OBJROOT)/$$i"			  \
		  	"SYMROOT=$(SYMROOT)"				  \
			"DSTROOT=$(DSTROOT)"				  \
			"SRCROOT=$(SRCROOT)"				  \
			"RC_ARCHS=$(RC_ARCHS)"				  \
			"RC_KANJI=$(RC_KANJI)"				  \
			"JAPANESE=$(JAPANESE)"				  \
			"RC_CFLAGS=$(RC_CFLAGS)" $@;  		  \
			else echo "$$i is not a directory or do not exist"; fi 			  \
		) || exit $$?; 						  \
	done	

installsrc:
	tar cf - . | (cd ${SRCROOT}; tar xfBp -)
