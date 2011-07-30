#
# Until I can remove the dependency on the appkit,
# we'll just keep the generated files in this directory
# and install them directly, rather than generating them again.
#
SRCROOT = $(shell pwd)/../../
OBJROOT = $(SRCROOT)/obj/config
SYMROOT = $(SRCROOT)/sym/
DSTROOT = $(SRCROOT)/dst/
DOCROOT = $(SRCROOT)/../doc
IMGROOT = $(SRCROOT)/sym/cache

DIR = config

include ${SRCROOT}/Make.rules

OBJECTS = cconfig.o zconf.tab.o \
		  yesno.o textbox.o menubox.o \
		  checklist.o inputbox.o

DEFINES = -DKBUILD_NO_NLS -DCURSES_LOC=\<ncurses.h\> -DPATH_MAX=256 -DPACKAGE=\"chameleon\" 
LDFLAGS = -lncurses -lmenu

PROGRAMS = cconfig

SYMPROG = $(addprefix $(SYMROOT)/, $(PROGRAMS))

DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all: $(DIRS_NEEDED) $(SYMPROG)

$(SYMPROG): $(addprefix $(OBJROOT)/, $(OBJECTS))
	@echo "\t[LD] $(@F)"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -o $(SYMROOT)/$(@F) $(OBJROOT)/*.o

config: $(DIRS_NEEDED) $(SYMPROG)
	@cd ${SRCROOT} && $(SYMPROG) $(SRCROOT)/Cconfig

rebuild_config: $(DIRS_NEEDED) $(SYMPROG)
	@cd ${SRCROOT} && $(SYMPROG) $(SRCROOT)/Cconfig rebuild


.PHONY: config

#dependencies
-include $(OBJROOT)/Makedep