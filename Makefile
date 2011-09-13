export USE_APPLE_PB_SUPPORT = all

#	Makefile for kernel booter

# CFLAGS	= -O $(MORECPP) -arch i386 -g 
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
PAX = /bin/pax

OBJROOT = `pwd`/obj
SYMROOT = `pwd`/sym
DSTROOT = `pwd`/dst
SRCROOT = `pwd`
DOCROOT = `pwd`/doc
IMGROOT = `pwd`/sym/cache
IMGSKELROOT = `pwd`/imgskel
CDBOOT = ${IMGROOT}/usr/standalone/i386/cdboot

THEME = default

VERSION = `cat version`
#REVISION = `cat revision`
PRODUCT = Chameleon-$(VERSION)#-r$(REVISION)
CDLABEL = ${PRODUCT}
ISOIMAGE = ${SYMROOT}/${CDLABEL}.iso

EXCLUDE = --exclude=.svn --exclude=.git --exclude=.DS_Store --exclude=sym --exclude=obj --exclude=build \
		--exclude=package --exclude=archive --exclude=User_Guide_src --exclude=*.sh

ARCHLESS_RC_CFLAGS=`echo $(RC_CFLAGS) | sed 's/-arch [a-z0-9]*//g'`

VPATH = $(OBJROOT):$(SYMROOT)

GENERIC_SUBDIRS =

#
# Currently builds for i386
#

all embedtheme tags debug install installhdrs modules: $(SYMROOT) $(OBJROOT)
	@if [ -e ".svn" ]; then svnversion -n | tr -d [:alpha:] > revision; fi
	@if [ -z "$(RC_ARCHS)" ]; then					  \
		RC_ARCHS="i386";					  \
	fi;								  \
	SUBDIRS="$(GENERIC_SUBDIRS) $$RC_ARCHS";			  \
	for i in $$SUBDIRS; 						  \
	do \
	    if [ -d $$i ]; then						  \
		echo ================= make $@ for $$i =================; \
		( OBJROOT=$(OBJROOT)/$${i};				  \
		  SYMROOT=$(SYMROOT)/$${i};				  \
		  DSTROOT=$(DSTROOT);					  \
	          XCFLAGS=$(ARCHLESS_RC_CFLAGS);			  \
	          GENSUBDIRS="$(GENERIC_SUBDIRS)";			  \
	          for x in $$GENSUBDIRS;				  \
	          do							  \
	              if [ "$$x" == "$$i" ]; then			  \
	                  XCFLAGS="$(RC_CFLAGS)";			  \
	                  break;					  \
	              fi						  \
	          done;							  \
		  echo "$$OBJROOT $$SYMROOT $$DSTROOT"; \
		    cd $$i; ${MAKE}					  \
			"OBJROOT=$$OBJROOT"		 	  	  \
		  	"SYMROOT=$$SYMROOT"				  \
			"DSTROOT=$$DSTROOT"				  \
			"SRCROOT=$$SRCROOT"				  \
			"RC_ARCHS=$$RC_ARCHS"				  \
			"TARGET=$$i"					  \
			"RC_KANJI=$(RC_KANJI)"				  \
			"JAPANESE=$(JAPANESE)"				  \
			"RC_CFLAGS=$$XCFLAGS" $@			  \
		) || exit $$?; 						  \
	    else							  \
	    	echo "========= nothing to build for $$i =========";	  \
	    fi;								  \
	done

image:
	@rm -rf ${IMGROOT}	
	@mkdir -p ${IMGROOT}/usr/standalone/i386
	@mkdir -p ${IMGROOT}/Extra/modules				
	@mkdir -p ${IMGROOT}/Extra/Themes/Default				
	@mkdir -p ${IMGROOT}/usr/bin
	@if [ -e "$(IMGSKELROOT)" ]; then				\
		@echo "\t[CP] ${IMGROOTSKEL} ${IMGROOT}"		\
		@cp -R -f "${IMGSKELROOT}"/* "${IMGROOT}";		\
	fi;								  
	@cp -f ${SYMROOT}/i386/cdboot ${CDBOOT}
	
	@#cp -f ${SYMROOT}/i386/Symbols.dylib ${IMGROOT}/Extra/modules
	@#cp -f ${SYMROOT}/i386/AcpiCodec.dylib ${IMGROOT}/Extra/modules
	@#cp -f ${SYMROOT}/i386/SmbiosGetters.dylib ${IMGROOT}/Extra/modules
	@#cp -f ${SYMROOT}/i386/Memory.dylib ${IMGROOT}/Extra/modules
	@#cp -f ${SYMROOT}/i386/keymapper.dylib ${IMGROOT}/Extra/modules
	@#cp -f ${SYMROOT}/i386/Usbfix.dylib ${IMGROOT}/Extra/modules
	@#cp -f ${SYMROOT}/i386/GraphicsEnabler.dylib ${IMGROOT}/Extra/modules
	@#cp -f ${SYMROOT}/i386/GUI.dylib ${IMGROOT}/Extra/modules
	
	@#cp -f ${SRCROOT}/artwork/themes/default/* ${IMGROOT}/Extra/Themes/Default

	@cp -f ${SYMROOT}/i386/boot ${IMGROOT}/usr/standalone/i386
	@cp -f ${SYMROOT}/i386/boot ${IMGROOT}/usr/standalone/i386
	@cp -f ${SYMROOT}/i386/boot0 ${IMGROOT}/usr/standalone/i386
	@cp -f ${SYMROOT}/i386/boot0hfs ${IMGROOT}/usr/standalone/i386
	@cp -f ${SYMROOT}/i386/boot1h ${IMGROOT}/usr/standalone/i386
	@cp -f ${SYMROOT}/i386/boot1f32 ${IMGROOT}/usr/standalone/i386

	@hdiutil makehybrid -iso -joliet -hfs -hfs-volume-name \
		${CDLABEL} -eltorito-boot ${CDBOOT} -no-emul-boot -ov -o   \
		"${ISOIMAGE}" ${IMGROOT} -quiet

pkg installer: embedtheme
	@if [ -e "$(SYMROOT)" ]; then					  \
	    sudo `pwd`/package/buildpkg `pwd`/sym/package;		  \
	fi;

release: $(SYMROOT)
	@if [ -e ".svn" ]; then svnversion -n | tr -d [:alpha:] > revision; fi
	@if [ -e "$(SYMROOT)" ]; then					  \
	    sudo `pwd`/package/buildpkg `pwd`/sym/package;		  \
	fi;
	@tar -czf $(SYMROOT)/$(PRODUCT)-src.tar.gz ${EXCLUDE} .
	@tar -cjf $(SYMROOT)/$(PRODUCT)-src.tar.bz2 ${EXCLUDE} .


clean:
	rm -rf sym obj dst

#distclean: clean
#	@rm -f $(SYMROOT)/$(PRODUCT)-src.*
#	@rm -f $(SYMROOT)/$(PRODUCT).pkg

installsrc:
	gnutar cf - . | (cd ${SRCROOT}; gnutar xpf -)

$(SYMROOT) $(OBJROOT) $(DSTROOT):
	@$(MKDIRS) $@
