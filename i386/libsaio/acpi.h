#ifndef __LIBSAIO_ACPI_H
#define __LIBSAIO_ACPI_H

#include "acpi_tools.h"

#define ACPI_RANGE_START    (0x0E0000)
#define ACPI_RANGE_END      (0x0FFFFF)


#define Unspecified         0
#define Desktop             1
#define Mobile              2
#define Workstation         3
#define EnterpriseServer    4
#define SOHOServer          5
#define AppliancePC         6

#define MaxSupportedPMProfile     AppliancePC // currently max profile supported 
#define PMProfileError            MaxSupportedPMProfile + 1

#endif /* !__LIBSAIO_ACPI_H */
