/*
 * ATI Graphics Card Enabler, part of the Chameleon Boot Loader Project
 *
 * Copyright 2010 by Islam M. Ahmed Zaid. All rights reserved.
 *
 */

#ifndef __LIBSAIO_ATI_H
#define __LIBSAIO_ATI_H

#define kUseAtiROM			"UseAtiROM"			/* ati.c */
#define kAtiConfig          "AtiConfig"         /* ati.c */
#define kAtiPorts           "AtiPorts"          /* ati.c */
#define kATYbinimage        "ATYbinimage"       /* ati.c */
#define kEnableHDMIAudio    "EnableHDMIAudio"   /* ati.c */

bool setup_ati_devprop(pci_dt_t *ati_dev);


#endif /* !__LIBSAIO_ATI_H */
