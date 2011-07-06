/*
 *  smp.c
 *  Chameleon
 *
 *  Created by cparm on 17/06/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "smp.h"
#include "libsaio.h"

#define ACPI_RANGE_START    (0x0E0000)
#define ACPI_RANGE_END      (0x0FFFFF)

static struct mp_t *
bios_search_mp(char *base, int length);
static struct mp_t* getAddressOfMPSTable();

static int lapic_dummy = 0;
unsigned imps_lapic_addr = ((unsigned)(&lapic_dummy)) - LAPIC_ID;

struct mp_t {
	uint8_t sig[4];
	uint32_t config_ptr;
	uint8_t len;
	uint8_t ver;
	uint8_t checksum;
	uint8_t f1;
	uint8_t f2;
	uint8_t fr[3];
}__attribute__((packed)) mp_t;

static struct mp_t* getAddressOfMPSTable()
{
	struct mp_t *mp;
    uint16_t		*addr;
	
    /* EBDA is the 1 KB addressed by the 16 bit pointer at 0x40E. */
    addr = (uint16_t *)ptov(EBDA_SEG_ADDR);
    if ((mp = bios_search_mp((char *)(*addr << 4), EBDA_SEG_LEN)) != NULL)
		return (mp);
	
	unsigned mem_lower = ((CMOS_READ_BYTE(CMOS_BASE_MEMORY+1) << 8)
						  | CMOS_READ_BYTE(CMOS_BASE_MEMORY))       << 10;
	
	if ((mp = bios_search_mp((char *)mem_lower, EBDA_SEG_LEN)) != NULL)
		return (mp);
	
    if ((mp = bios_search_mp((char *)0x00F0000, ACPI_RANGE_END)) != NULL)
		return (mp);
	
    return (NULL);
	
}

static struct mp_t *
bios_search_mp(char *base, int length)
{
	struct mp_t *mp;
    int			ofs;
	
    /* search on 16-byte boundaries */
    for (ofs = 0; ofs < length; ofs += 16) {
		
		mp = (struct mp_t*)ptov(base + ofs);
		
		/* compare signature, validate checksum */
		if (!strncmp((char*)mp->sig, MP_SIGSTR, strlen(MP_SIGSTR))) {
			uint8_t csum = checksum8(mp, sizeof(struct mp_t));
			if(csum == 0) { 
				return mp;
			}
			
		}				
	}	
    return NULL;
}

void * getMPSTable()
{	
	struct mp_t *mps_p = getAddressOfMPSTable() ;
    
    if (mps_p)
    {		
        if (mps_p->config_ptr) {
			
            struct imps_cth *local_cth_ptr
            = (struct imps_cth *)ptov(mps_p->config_ptr);
			
            imps_lapic_addr = local_cth_ptr->lapic_addr;
			
        } else {
            imps_lapic_addr = LAPIC_ADDR_DEFAULT;
        }		
	}
	
	return (void *)mps_p;
}