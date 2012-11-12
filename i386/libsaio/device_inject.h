/*
 *	Copyright 2009 Jasmin Fazlic All rights reserved.
 */
/*
 *	Cleaned and merged by iNDi
 */

#ifndef __LIBSAIO_DEVICE_INJECT_H
#define __LIBSAIO_DEVICE_INJECT_H

#include "pci.h"
#include "device_tree.h"

#define DP_ADD_TEMP_VAL(dev, val) devprop_add_value(dev, (char*)val[0], (uint8_t*)val[1], strlen(val[1]) + 1)
#define DP_ADD_TEMP_VAL_DATA(dev, val) devprop_add_value(dev, (char*)val.name, (uint8_t*)val.data, val.size)

struct	DevPropString	*devprop_create_string(void);
struct	DevPropDevice *devprop_add_device(struct DevPropString *string, pci_dt_t * pci_dt);
struct	DevPropDevice *devprop_make_device(pci_dt_t *pci_dt);
int		devprop_add_value(struct DevPropDevice *device, char *nm, uint8_t *vl, uint32_t len);
void	register_device_inject(void);
char	*devprop_generate_string(struct DevPropString *string);
void	devprop_free_string(struct DevPropString *string);

#endif /* !__LIBSAIO_DEVICE_INJECT_H */
