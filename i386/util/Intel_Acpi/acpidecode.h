/*
Copyright (c) 2010, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name of Intel Corporation nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ACPI_DECODE_H
#define ACPI_DECODE_H

#include "datatype.h"
#include "intel_acpi.h"
#include "ppm.h"

/* acpidecode.h contains functions needed only for decoding ACPI to produce the
 * inputs to the PPM code, but not needed for encoding ACPI as done inside the
 * PPM code. */

#define ACPI_NAMESPACE_MAX_DEPTH 10

struct acpi_namespace {
    U32 nameseg[ACPI_NAMESPACE_MAX_DEPTH];
    U32 depth;
};

/* Globals used for retrieving ACPI processor structures from the DSDT */

#define CPU_MAP_LIMIT 256 /* Any bigger than 256 and we'd have to support the x2APIC structures, which we don't yet anyway. */

struct acpi_processor {
    struct acpi_namespace ns;
    U8 id;
    U32 pmbase;
};

#define ACPI_NAMESPATH_STR_MAX_LEN  2 /* \_ */ + (4 * ACPI_NAMESPACE_MAX_DEPTH ) + (ACPI_NAMESPACE_MAX_DEPTH - 1 /*max number of . */ ) + 1 /* eof */

#endif /* ACPI_DECODE_H */
