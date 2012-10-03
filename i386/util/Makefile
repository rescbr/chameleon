#
# Until I can remove the dependency on the appkit,
# we'll just keep the generated files in this directory
# and install them directly, rather than generating them again.
#

DIR = util
include ../MakePaths.dir

VPATH = $(OBJROOT):$(SYMROOT)

INSTALLDIR = $(DSTROOT)/usr/standalone/i386
LOCALBIN = $(DSTROOT)/usr/local/bin

OPTIM = -Os -Oz
CFLAGS = $(RC_CFLAGS) $(OPTIM) -Wmost -Werror -g
LDFLAGS =
CFILES = machOconv.c
ALLSRC = $(CFILES) $(MFILES) $(HFILES) $(EXPORT_HFILES)
PROGRAMS = md.o machOconv bdmesg amlsgn fdisk440

OUTFILES = $(PROGRAMS)

DIRS_NEEDED = $(OBJROOT) $(SYMROOT) $(LANGDIR)

all embedtheme: $(DIRS_NEEDED) $(PROGRAMS)

machOconv: machOconv.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(DEFINES) -o $(SYMROOT)/$(@F) machOconv.o

bdmesg: bdmesg.o
	$(CC) $(CFLAGS) $(LDFLAGS) -framework IOKit -framework CoreFoundation -mmacosx-version-min=10.5 -o $(SYMROOT)/$(@F) bdmesg.o

amlsgn: 
	$(CC) -arch i386 -framework IOKit -framework Foundation -mmacosx-version-min=10.5 amlsgn.m -o $(SYMROOT)/amlsgn

fdisk440: 
	$(CC) $(LDFALGS) -mmacosx-version-min=10.5 -arch i386 -o $(SYMROOT)/fdisk440 fdisk/*.c

md.o: 
	$(CC) -mdynamic-no-pic -Wall -dead_strip -arch i386 -mmacosx-version-min=10.5 md.c -o $(SYMROOT)/md

include ../MakeInc.dir


#dependencies
-include $(OBJROOT)/Makedep

