/*
 *  Copyright 2008 mackerintel
 *
 *  state_generator.h
 *  Chameleon
 *
 *  Created by Mozodojo on 20/07/10.
 *  Copyright 2010 mozo. All rights reserved.
 *
 */

#ifndef __LIBSAIO_STATE_GENERATOR_H
#define __LIBSAIO_STATE_GENERATOR_H

#include "aml_generator.h"
#include "libsaio.h"

void	get_acpi_cpu_names(uint8_t *dsdt, uint32_t length);
struct	acpi_2_ssdt *generate_cst_ssdt(struct acpi_2_fadt *fadt);
struct	acpi_2_ssdt *generate_pss_ssdt(struct acpi_2_dsdt *dsdt);

#endif /* !__LIBSAIO_STATE_GENERATOR_H */
