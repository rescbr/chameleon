


#ifndef __LIBSAIO_SMBIOS_GETTERS_H
#define __LIBSAIO_SMBIOS_GETTERS_H

#include "libsaio.h"
#include "mysmbios.h"
#include "platform.h"
#include "pci.h"

typedef enum {
	kSMBString,
	kSMBByte,
	kSMBWord,
	kSMBDWord
//	kSMBQWord
} SMBValueType;

typedef union {
	const char	*string;
	uint8_t		byte;
	uint16_t	word;
	uint32_t	dword;
//	uint64_t	qword;
} returnType;

extern bool getProcessorInformationExternalClock(returnType *value);
extern bool getProcessorInformationMaximumClock(returnType *value);
extern bool getSMBOemProcessorBusSpeed(returnType *value);
extern bool getSMBOemProcessorType(returnType *value);
extern bool getSMBMemoryDeviceMemoryType(returnType *value);
extern bool getSMBMemoryDeviceMemorySpeed(returnType *value);
extern bool getSMBMemoryDeviceManufacturer(returnType *value);
extern bool getSMBMemoryDeviceSerialNumber(returnType *value);
extern bool getSMBMemoryDevicePartNumber(returnType *value);

#endif /* !__LIBSAIO_SMBIOS_GETTERS_H */
