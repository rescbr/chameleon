#
# Until I can remove the dependency on the appkit,
# we'll just keep the generated files in this directory
# and install them directly, rather than generating them again.
#
ROOT = $(shell pwd)/../../
SRCROOT = ${ROOT}/src
OBJROOT = $(ROOT)/obj/config
SYMROOT = $(ROOT)/sym
DSTROOT = $(ROOT)/dst
DOCROOT = $(ROOT)/doc

DIR = config

include ${ROOT}/Make.rules

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

#dependencies
-include $(OBJROOT)/Makedep