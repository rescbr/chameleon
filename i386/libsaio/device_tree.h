/*
 * Copyright (c) 2005 Apple Computer, Inc.  All Rights Reserved.
 */

#ifndef __DEVICE_TREE_H
#define __DEVICE_TREE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct _Property {
    char *             name;
    uint32_t           length;
    void *             value;

    struct _Property * next;
} Property;

typedef struct _Node {
    struct _Property * properties;
    struct _Property * last_prop;
    
    struct _Node *     children;

    struct _Node *     next;
} Node;

#define MAX_PCI_DEV_PATHS 4
#define MAX_STRING_NUM_ENTRIES 100

#define DEV_PATH_HEADER		\
uint8_t		type;		\
uint8_t		subtype;	\
uint16_t	length;

struct ACPIDevPath {
	//uint8_t		type;		// = 2 ACPI device-path
	//uint8_t		subtype;	// = 1 ACPI Device-path
	//uint16_t	    length;		// = 0x0c
	DEV_PATH_HEADER
	uint32_t	_HID;		// = 0xD041030A ?
	uint32_t	_UID;		// = 0x00000000 PCI ROOT
};

struct PCIDevPath {
	//uint8_t		type;		// = 1 Hardware device-path
	//uint8_t		subtype;	// = 1 PCI
	//uint16_t	    length;		// = 6
	DEV_PATH_HEADER
	uint8_t		function;	// pci func number
	uint8_t		device;		// pci dev number
};

struct DevicePathEnd {
	//uint8_t		type;		// = 0x7f
	//uint8_t		subtype;	// = 0xff
	//uint16_t		length;		// = 4;
	DEV_PATH_HEADER
};

struct DevPropDevice {
	uint32_t length;
	uint16_t numentries;
	uint16_t WHAT2;										// 0x0000 ?
	struct ACPIDevPath acpi_dev_path;					// = 0x02010c00 0xd041030a
	struct PCIDevPath  pci_dev_path[MAX_PCI_DEV_PATHS]; // = 0x01010600 func dev
	struct DevicePathEnd path_end;						// = 0x7fff0400
	uint8_t *data;
	
	// ------------------------
	uint8_t	 num_pci_devpaths;
	struct DevPropString *string;
	// ------------------------
};

struct DevPropString {
	uint32_t length;
	uint32_t WHAT2;			// 0x01000000 ?
	uint16_t numentries;
	uint16_t WHAT3;			// 0x0000     ?
	struct DevPropDevice *entries;
};


extern Property *
DT__AddProperty(Node *node, const char *name, uint32_t length, void *value);

extern Node *
DT__AddChild(Node *parent, const char *name);

Node *
DT__FindNode(const char *path, bool createIfMissing);

extern void
DT__FreeProperty(Property *prop);

extern void
DT__FreeNode(Node *node);

extern char *
DT__GetName(Node *node);

void
DT__Initialize(void);

/*
 * Free up memory used by in-memory representation
 * of device tree.
 */
extern void
DT__Finalize(void);

void
DT__FlattenDeviceTree(void **result, uint32_t *length);

#endif /* __DEVICE_TREE_H */
