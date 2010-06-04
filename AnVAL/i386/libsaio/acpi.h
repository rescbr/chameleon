#ifndef __LIBSAIO_ACPI_H
#define __LIBSAIO_ACPI_H

#define ACPI_RANGE_START    (0x0E0000)
#define ACPI_RANGE_END      (0x0FFFFF)

#define UINT64_LE_FROM_CHARS(a,b,c,d,e,f,g,h) \
(   ((uint64_t)h << 56) \
|   ((uint64_t)g << 48) \
|   ((uint64_t)f << 40) \
|   ((uint64_t)e << 32) \
|   ((uint64_t)d << 24) \
|   ((uint64_t)c << 16) \
|   ((uint64_t)b <<  8) \
|   ((uint64_t)a <<  0) \
)

#define ACPI_SIGNATURE_UINT64_LE UINT64_LE_FROM_CHARS('R','S','D',' ','P','T','R',' ')

/* Per ACPI 3.0a spec */

// TODO Migrate
struct acpi_2_rsdp {
    char            Signature[8];
    uint8_t         Checksum;
    char            OEMID[6];
    uint8_t         Revision;
    uint32_t        RsdtAddress;
    uint32_t        Length;
    uint64_t        XsdtAddress;
    uint8_t         ExtendedChecksum;
    char            Reserved[3];
} __attribute__((packed));

// TODO Migrate
struct acpi_2_rsdt {
	char            Signature[4];
	uint32_t        Length;
	uint8_t         Revision;
	uint8_t         Checksum;
	char            OEMID[6];
	char            OEMTableId[8];
	uint32_t        OEMRevision;
	uint32_t        CreatorId;
	uint32_t        CreatorRevision;
} __attribute__((packed));

// TODO Migrate
struct acpi_2_xsdt {
	char            Signature[4];
	uint32_t        Length;
	uint8_t         Revision;
	uint8_t         Checksum;
	char            OEMID[6];
	char            OEMTableId[8];
	uint32_t        OEMRevision;
	uint32_t        CreatorId;
	uint32_t        CreatorRevision;
} __attribute__((packed));

// TODO Migrate
struct acpi_2_gas {
	uint8_t				Address_Space_ID;
	uint8_t				Register_Bit_Width;
	uint8_t				Register_Bit_Offset;
	uint8_t				Access_Size;
	uint64_t			Address;
} __attribute__((packed));

// TODO Migrate
struct acpi_2_fadt {
	char				Signature[4];
	uint32_t			Length;
	uint8_t				Revision;
	uint8_t				Checksum;
	char				OEMID[6];
	char				OEMTableId[8];
	uint32_t			OEMRevision;
	uint32_t			CreatorId;
	uint32_t			CreatorRevision;
	uint32_t			FIRMWARE_CTRL;
	uint32_t			DSDT;
	uint8_t				INT_MODEL; // JrCs
	uint8_t				Preferred_PM_Profile; // JrCs
	uint16_t			SCI_INT;
	uint32_t			SMI_CMD;
	uint8_t				ACPI_ENABLE;
	uint8_t				ACPI_DISABLE;
	uint8_t				S4BIOS_REQ;
	uint8_t				PSTATE_CNT;
	uint32_t			PM1a_EVT_BLK;
	uint32_t			PM1b_EVT_BLK;
	uint32_t			PM1a_CNT_BLK;
	uint32_t			PM1b_CNT_BLK;
	uint32_t			PM2_CNT_BLK;
	uint32_t			PM_TMR_BLK;
	uint32_t			GPE0_BLK;
	uint32_t			GPE1_BLK;
	uint8_t				PM1_EVT_LEN;
	uint8_t				PM1_CNT_LEN;
	uint8_t				PM2_CNT_LEN;
	uint8_t				PM_TMR_LEN;
	uint8_t				GPE0_BLK_LEN;
	uint8_t				GPE1_BLK_LEN;
	uint8_t				GPE1_BASE;
	uint8_t				CST_CNT;
	uint16_t			P_LVL2_LAT;
	uint16_t			P_LVL3_LAT;
	uint16_t			FLUSH_SIZE;
	uint16_t			FLUSH_STRIDE;
	uint8_t				DUTY_OFFSET;
	uint8_t				DUTY_WIDTH;
	uint8_t				DAY_ALRM;
	uint8_t				MON_ALRM;
	uint8_t				CENTURY;
	uint16_t			IAPC_BOOT_ARCH;
	uint8_t				Reserved1;
	uint32_t			Flags;
	struct acpi_2_gas	RESET_REG;
	uint8_t				RESET_VALUE;
	uint8_t				Reserved2[3];
	uint64_t			X_FIRMWARE_CTRL;
	uint64_t			X_DSDT;
	struct acpi_2_gas	X_PM1a_EVT_BLK;
	struct acpi_2_gas	X_PM1b_EVT_BLK;
	struct acpi_2_gas	X_PM1a_CNT_BLK;
	struct acpi_2_gas	X_PM1b_CNT_BLK;
	struct acpi_2_gas	X_PM2_CNT_BLK;
	struct acpi_2_gas	X_PM_TMR_BLK;
	struct acpi_2_gas	X_GPE0_BLK;
	struct acpi_2_gas	X_GPE1_BLK;
} __attribute__((packed));

#endif /* !__LIBSAIO_ACPI_H */
