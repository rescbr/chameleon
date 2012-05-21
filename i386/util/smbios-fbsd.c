/*-
 * Copyright (c) 2005-2009 Jung-uk Kim <jkim@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Detect SMBIOS and export information about the SMBIOS into the
 * environment.
 *
 * System Management BIOS Reference Specification, v2.6 Final
 * http://www.dmtf.org/standards/published_documents/DSP0134_2.6.0.pdf
 */

/*
 * 2.1.1 SMBIOS Structure Table Entry Point
 *
 * "On non-EFI systems, the SMBIOS Entry Point structure, described below, can
 * be located by application software by searching for the anchor-string on
 * paragraph (16-byte) boundaries within the physical memory address range
 * 000F0000h to 000FFFFFh. This entry point encapsulates an intermediate anchor
 * string that is used by some existing DMI browsers."
 */

#include "libsaio.h"

#define SMBIOS_START            0xf0000
#define SMBIOS_LENGTH           0x10000
#define SMBIOS_STEP             0x10
#define SMBIOS_SIG              "_SM_"
#define SMBIOS_DMI_SIG          "_DMI_"

#define SMBIOS_GET8(base, off)  (*(uint8_t *)((base) + (off)))
#define SMBIOS_GET16(base, off) (*(uint16_t *)((base) + (off)))
#define SMBIOS_GET32(base, off) (*(uint32_t *)((base) + (off)))

#define SMBIOS_GETLEN(base)     SMBIOS_GET8(base, 0x01)
#define SMBIOS_GETSTR(base)     ((base) + SMBIOS_GETLEN(base))

static uint32_t smbios_enabled_memory = 0;
static uint32_t smbios_old_enabled_memory = 0;
static uint8_t  smbios_enabled_sockets = 0;
static uint8_t  smbios_populated_sockets = 0;
typedef char* caddr_t;

static uint8_t
smbios_checksum(const caddr_t addr, const uint8_t len)
{
	uint8_t         sum;
	int             i;
	
	for (sum = 0, i = 0; i < len; i++)
		sum += SMBIOS_GET8(addr, i);
	return (sum);
}

static caddr_t
smbios_sigsearch(const caddr_t addr, const uint32_t len)
{
	caddr_t         cp;
	
	/* Search on 16-byte boundaries. */
	for (cp = addr; cp < addr + len; cp += SMBIOS_STEP)
		if (strncmp(cp, SMBIOS_SIG, 4) == 0 &&
			smbios_checksum(cp, SMBIOS_GET8(cp, 0x05)) == 0 &&
			strncmp(cp + 0x10, SMBIOS_DMI_SIG, 5) == 0 &&
			smbios_checksum(cp + 0x10, 0x0f) == 0)
			return (cp);
	return (NULL);
}

void
smbios_detect(void)
{
	char            buf[16];
	caddr_t         addr, dmi, smbios;
	size_t          count, length;
	uint32_t        paddr;
	int             i, major, minor, ver;
	
	/* Search signatures and validate checksums. */
	smbios = smbios_sigsearch(PTOV(SMBIOS_START), SMBIOS_LENGTH);
	if (smbios == NULL)
		return;
	
	length = SMBIOS_GET16(smbios, 0x16);    /* Structure Table Length */
	paddr = SMBIOS_GET32(smbios, 0x18);     /* Structure Table Address */
	count = SMBIOS_GET16(smbios, 0x1c);     /* No of SMBIOS Structures */
	ver = SMBIOS_GET8(smbios, 0x1e);        /* SMBIOS BCD Revision */
	
	if (ver != 0) {
		major = ver >> 4;
		minor = ver & 0x0f;
		if (major > 9 || minor > 9)
			ver = 0;
	}
	if (ver == 0) {
		major = SMBIOS_GET8(smbios, 0x06); /* SMBIOS Major Version */
		minor = SMBIOS_GET8(smbios, 0x07); /* SMBIOS Minor Version */
	}
	ver = (major << 8) | minor;
	
	addr = PTOV(paddr);
	for (dmi = addr, i = 0; dmi < addr + length && i < count; i++)
		dmi = smbios_parse_table(dmi, ver);
	
	sprintf(buf, "%d.%d", major, minor);
	setenv("smbios.version", buf, 1);
	if (smbios_enabled_memory > 0 || smbios_old_enabled_memory > 0) {
		sprintf(buf, "%u", smbios_enabled_memory > 0 ?
				336                     smbios_enabled_memory : smbios_old_enabled_memory);
		setenv("smbios.memory.enabled", buf, 1);
	}
	if (smbios_enabled_sockets > 0) {
		sprintf(buf, "%u", smbios_enabled_sockets);
		setenv("smbios.socket.enabled", buf, 1);
	}
	if (smbios_populated_sockets > 0) {
		sprintf(buf, "%u", smbios_populated_sockets);
		setenv("smbios.socket.populated", buf, 1);
	}
}
