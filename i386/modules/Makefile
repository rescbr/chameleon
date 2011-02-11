#	Makefile for i386 modules

# The order of building is important.
SUBDIRS = klibc uClibc++ Resolution KernelPatcher GUI KextPatcher GraphicsEnabler HPET USBFix Memory Networking NetbookInstaller ACPIPatcher
#SUBDIRS = klibc uClibc++ ACPIPatcher PCI HDAC
#SUBDIRS = HelloWorld


CFLAGS= -O $(MORECPP) -arch i386 -g -static
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
OBJROOT=`pwd`/../../obj/i386/
SYMROOT=`pwd`/../../sym/i386/
DSTROOT=`pwd`/../../dst/i386/
SRCROOT=/tmp

VPATH = $(OBJROOT):$(SYMROOT)


all embedtheme optionrom tags debug install installhdrs: objroot_dirs lazydylib1.o
	@for i in ${SUBDIRS}; \
	do \
		echo ================= make $@ for $$i =================; \
		( cd $$i; $(MKDIRS) $(OBJROOT)/$$i; ${MAKE}					  \
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

objroot_dirs:
	@rm -rf "$(OBJROOT)" 
	@mkdir "$(OBJROOT)" &> /dev/null
	@rm -rf module_includes
	@mkdir module_includes
	@rm -rf "$(SYMROOT)/modules/"
	@mkdir "$(SYMROOT)/modules/"


%.o: %.c
	@echo "\t[CC] $<"
	@$(CC) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "$<" $(INC) -o "$(OBJROOT)/$@"

%.o: %.cpp
	@echo "\t[CPP] $<"
	@$(CPP) $(CPPFLAGS) $(CFLAGS) $(DEFINES) -c "$<" $(INC) -o "$(OBJROOT)/$@"

installsrc:
	tar cf - . | (cd ${SRCROOT}; tar xfBp -)
