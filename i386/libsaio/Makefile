
DIR = libsaio
include ../MakePaths.dir

UTILDIR = ../util
LIBSADIR = ../libsa
BOOT2DIR = ../boot2
INSTALLDIR = $(DSTROOT)/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders/standalone

OPTIM = -Os -Oz
DEBUG = 
CFLAGS	= $(RC_CFLAGS) $(OPTIM) $(MORECPP) -arch i386 -g -Wall -fno-stack-protector \
    -D__ARCHITECTURE__=\"i386\" \
    $(DEBUG) \
    -fno-builtin -static $(OMIT_FRAME_POINTER_CFLAG) -march=pentium4 -msse2 -msoft-float  -Qunused-arguments -ffreestanding -DBOOT_CORE -mpreferred-stack-boundary=2 -fno-align-functions -mfpmath=sse -Werror
GFLAGS	= 
DEFINES=
CONFIG = hd
INC = -I. -I$(SYMROOT) -I$(UTILDIR) -I$(LIBSADIR) -I$(BOOT2DIR)
ifneq "" "$(wildcard /bin/mkdirs)"
  MKDIRS = /bin/mkdirs
else
  MKDIRS = /bin/mkdir -p
endif
AS = as
LD = ld
# LIBS= -lc_static
LIBS=

VPATH = $(OBJROOT):$(SYMROOT)

SAIO_OBJS = table.o asm.o bios.o biosfn.o arc4random-fbsd.o   \
        disk.o sys.o cache.o bootstruct.o \
        stringTable.o load.o pci.o allocate.o misc.o \
		vbe.o hfs.o hfs_compare.o \
        xml.o md5.o device_tree.o modules.o \
        cpu.o platform.o \
        fake_efi.o \
        convert.o acpi_tools.o smbios.o uterror.o lzss.o lib.o rtc.o time.o gen_uuid-uuid.o pack.o unpack.o CException.o

# Options enabled by default:

#CFLAGS += -DNO_WIN_SUPPORT # -7744 bytes
SAIO_OBJS += ntfs.o msdos.o exfat.o

#CFLAGS += -DNO_LINUX_SUPPORT # -256 bytes
SAIO_OBJS += ext2fs.o

#CFLAGS += -DNO_OTHERS_BSD_SUPPORT # -512 bytes
SAIO_OBJS += freebsd.o openbsd.o

#CFLAGS += -DNO_HAIKU_SUPPORT # -256 bytes
SAIO_OBJS += befs.o

# Options disabled by default:

CFLAGS += -DNO_BOOT_IMG

CFLAGS += -DNO_SMP_SUPPORT 
#SAIO_OBJS += smp-imps.o

#CFLAGS += -DUFS_SUPPORT # +3616 bytes
#SAIO_OBJS += ufs.o ufs_byteorder.o

#CFLAGS += -DAPPLE_PARTITION_MAP_SUPPORT # +704 bytes
#CFLAGS += -DBOOT_HELPER_SUPPORT
# for npb, you must also activate loader function in asm.s, 
# and NBP_SUPPORT flag in boot2's makefile (it should compile it, but the result of this is untested)
#CFLAGS += -DNBP_SUPPORT
#SAIO_OBJS += nbp.o 


SAIO_EXTERN_OBJS = console.o

SFILES =
CFILES = 
HFILES = 
EXPORTED_HFILES =
INSTALLED_HFILES =
OTHERFILES = Makefile
ALLSRC =  $(SFILES) $(CFILES) \
	$(HFILES) $(OTHERFILES)
LIBS = libsaio.a
DIRS_NEEDED = $(OBJROOT) $(SYMROOT)

all embedtheme: $(DIRS_NEEDED) libsaio.h embedded.h $(LIBS)

fake_efi.o: vers.h
vers.h:
	@echo "#define I386BOOT_VERSION \"5.0.132\"" > $(SYMROOT)/vers.h
	@echo "#define I386BOOT_BUILDDATE \"`date \"+%m/%d/%Y %H:%M:%S\"`\"" >> $(SYMROOT)/vers.h
	@echo "#define I386BOOT_CHAMELEONVERSION \"`cat ../../version`\"" >> $(SYMROOT)/vers.h
	@echo "#define I386BOOT_CHAMELEONREVISION \"`svnversion -n | tr -d [:alpha:]`\"" >> $(SYMROOT)/vers.h            
	@echo "#define FIRMWARE_PUBLISHER \"`whoami`\"" >> $(SYMROOT)/vers.h
    
libsaio.a: $(SAIO_EXTERN_OBJS) $(SAIO_OBJS)
	rm -f $(SYMROOT)/$(@F)
	ar q $(SYMROOT)/$(@F) $^
	ranlib $(SYMROOT)/$(@F)

embedded.h:
	@cd $(SYMROOT)/../../doc && xxd -i BootHelp.txt > $(SYMROOT)/embedded.h

include ../MakeInc.dir

# dependencies
-include $(OBJROOT)/Makedep
