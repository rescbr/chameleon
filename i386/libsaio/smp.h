/*
 *  <Insert copyright here : it must be BSD-like so everyone can use it>
 *
 *  Author:  Erich Boleyn  <erich@uruk.org>   http://www.uruk.org/~erich/
 *
 *  Header file implementing Intel MultiProcessor Specification (MPS)
 *  version 1.1 and 1.4 SMP hardware control for Intel Architecture CPUs,
 *  with hooks for running correctly on a standard PC without the hardware.
 *
 *  This file was created from information in the Intel MPS version 1.4
 *  document, order number 242016-004, which can be ordered from the
 *  Intel literature center.
 */
#ifndef _SMP_H
#define _SMP_H

#include "libsaio.h"

//#define MP_SIGL 0x5f504d5f
#define MP_SIGSTR "_MP_"

/*
 *  MP Configuration Table Header  (cth)
 *
 *  Look at page 4-5 of the MP spec for the starting definitions of
 *  this structure.
 */
struct imps_cth
{
	unsigned sig;
	unsigned short base_length;
	unsigned char spec_rev;
	unsigned char checksum;
	char oem_id[8];
	char prod_id[12];
	unsigned oem_table_ptr;
	unsigned short oem_table_size;
	unsigned short entry_count;
	unsigned lapic_addr;
	unsigned short extended_length;
	unsigned char extended_checksum;
	char reserved[1];
};

/*
 *  Defines that are here so as not to be in the global header file.
 */
#define EBDA_SEG_ADDR			0x40E
#define EBDA_SEG_LEN			0x400
#define BIOS_RESET_VECTOR		0x467
#define LAPIC_ADDR_DEFAULT		0xFEE00000uL
#define IOAPIC_ADDR_DEFAULT		0xFEC00000uL
#define CMOS_RESET_CODE			0xF
#define	CMOS_RESET_JUMP         0xa
#define CMOS_BASE_MEMORY		0x15
#define LAPIC_ID				0x20

/*
 *  This contains the local APIC hardware address.
 */
extern unsigned imps_lapic_addr;
/*
 *  Defines that use variables
 */

#define IMPS_LAPIC_READ(x)  (*((volatile unsigned *) (imps_lapic_addr+(x))))
#define IMPS_LAPIC_WRITE(x, y)   \
(*((volatile unsigned *) (imps_lapic_addr+(x))) = (y))

#endif  /* !_SMP_H */