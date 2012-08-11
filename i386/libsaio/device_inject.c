/*
 *	Copyright 2009 Jasmin Fazlic All rights reserved.
 */
/*
 *	Cleaned and merged by iNDi
 */

#include "libsaio.h"
#include "bootstruct.h"
#include "pci.h"
#include "device_inject.h"
#include "convert.h"
#include "platform.h"

#ifndef DEBUG_INJECT
#define DEBUG_INJECT 0
#endif

#if DEBUG_INJECT
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

char *efi_inject_get_devprop_string(uint32_t *len)
{
    struct DevPropString *string = (struct DevPropString *)(uint32_t)get_env(envEFIString);

	if(string)
	{
		*len = string->length;
		return devprop_generate_string(string);
	}
	return NULL;
}

void setupDeviceProperties(Node *node)
{
	const char *val;
	uint8_t *binStr;
    uint8_t *kbinStr;

	int cnt = 0, cnt2 = 0;
	
	static char DEVICE_PROPERTIES_PROP[] = "device-properties";
	
	/* Generate devprop string.
	 */
	uint32_t strlength;
	char *string = efi_inject_get_devprop_string(&strlength);
	if (string == NULL) {
		verbose("efi_inject_get_devprop_string NULL trying stringdata\n");
		return;
	}
	/* Use the static "device-properties" boot config key contents if available,
	 * otheriwse use the generated one.
	 */  
	if (!getValueForKey(kDeviceProperties, &val, &cnt, DEFAULT_BOOT_CONFIG))
	{
		val = (const char*)string;
		cnt = strlength * 2;
	} 
    
	if (cnt > 1)
	{
		binStr = convertHexStr2Binary(val, &cnt2);  
        
        if (cnt2 > 0)
        {
            kbinStr = (uint8_t*)AllocateKernelMemory(cnt2);        
            bcopy(binStr,kbinStr,cnt2);        
            DT__AddProperty(node, DEVICE_PROPERTIES_PROP, cnt2, kbinStr);
        }
	}
}

struct DevPropString *devprop_create_string(void)
{
	struct DevPropString *string = (struct DevPropString*)malloc(sizeof(struct DevPropString));
	
	if(string == NULL)
	{
		return NULL;
	}
	
	memset(string, 0, sizeof(struct DevPropString));
	string->length = 12;
	string->WHAT2 = 0x01000000;
	return string;
}

struct DevPropDevice *devprop_make_device(pci_dt_t *pci_dt)
{
	struct DevPropDevice	*device;
    int numpaths = 0;

	pci_dt_t	*current;
	pci_dt_t	*end;
	
	end = root_pci_dev;
    
	device = malloc(sizeof(struct DevPropDevice));
	if (!device) {
        return NULL;
    }
    memset(device, 0, sizeof(struct DevPropDevice));

	device->acpi_dev_path._UID = getPciRootUID();
	while (end != pci_dt)
	{
		current = pci_dt;
		while (current->parent != end)
			current = current->parent;			
		end = current;

		{
            device->pci_dev_path[numpaths].device =	(uint8_t)current->dev.bits.dev;
			device->pci_dev_path[numpaths].function = (uint8_t)current->dev.bits.func;
            numpaths++;
		} 
		
	}
			
	if(!numpaths)
	{
        printf("ERROR parsing device path\n");
        free(device);
		return NULL;
	}
	
	device->numentries = 0x00;
	
	device->acpi_dev_path.length = 0x0c;
	device->acpi_dev_path.type = 0x02;
	device->acpi_dev_path.subtype = 0x01;
	device->acpi_dev_path._HID = 0xd041030a;
	
	device->num_pci_devpaths = numpaths;
	device->length = 24 + (6*numpaths);
	
	int		i; 
	
	for(i = 0; i < numpaths; i++)
	{
		device->pci_dev_path[i].length = 0x06;
		device->pci_dev_path[i].type = 0x01;
		device->pci_dev_path[i].subtype = 0x01;
	}
	
	device->path_end.length = 0x04;
	device->path_end.type = 0x7f;
	device->path_end.subtype = 0xff;
	
	device->data = NULL;
			
	return device;
}

struct DevPropDevice *devprop_add_device(struct DevPropString *string, pci_dt_t * pci_dt)
{
	struct DevPropDevice	*device;
		
	if (string == NULL || pci_dt == NULL) {
		return NULL;
	}
	device = devprop_make_device(pci_dt);
	if (!device) {
        return NULL;
    }
		
	device->string = string;
	string->length += device->length;
	
	if(!string->entries)
	{
		if((string->entries = (struct DevPropDevice*)malloc(sizeof(struct DevPropDevice) * MAX_STRING_NUM_ENTRIES))== NULL)
		{
            printf("ERROR parsing device path 2\n");
            
            free(device);
			return NULL;
		}
	}
	struct DevPropDevice **string_entries_arrey = (struct DevPropDevice **) string->entries;
	
	if ((string->numentries+1) < MAX_STRING_NUM_ENTRIES)
    {
		string->numentries++;
		
    }
    else
    {
        free(string->entries);
        free(device);
        return NULL;
    }	
    
	string_entries_arrey[string->numentries-1] = device;
	
	return device;
}

#if 0
struct DevPropDevice *devprop_add_device(struct DevPropString *string, char *path)
{
	struct DevPropDevice	*device;
	const char		pciroot_string[] = "PciRoot(0x";
	const char		pci_device_string[] = "Pci(0x";
	
	if (string == NULL || path == NULL) {
		return NULL;
	}
	device = malloc(sizeof(struct DevPropDevice));
	if (!device) {
        return NULL;
    }
	if (strncmp(path, pciroot_string, strlen(pciroot_string))) {
        free(device);
		printf("ERROR parsing device path 1\n");
		return NULL;
	}
	
	memset(device, 0, sizeof(struct DevPropDevice));
	device->acpi_dev_path._UID = getPciRootUID();
	
	int numpaths = 0;
	int		x, curr = 0, w = 0;
    
	char	buff[16];
	
    
	for (x = 0; x < strlen(path); x++) 
	{
        
		if (!strncmp(&path[x], pci_device_string, strlen(pci_device_string)))
		{
			x+=strlen(pci_device_string);
			curr=x;
			while(path[++x] != ',');
            
            w = x-curr;
            
            if ((w > 4) || /*(w > sizeof(buff)) ||*/ (w == 0)) {
                printf("ERROR parsing device path 2\n");
                break;
            }
            
            snprintf(buff, x-curr, "%s",&path[curr]);
			
			device->pci_dev_path[numpaths].device =	(uint8_t)strtoul(buff, NULL, 16);
			
            bzero(buff, sizeof(buff));
            
			x += 3; // 0x
			curr = x;
			while(path[++x] != ')');
            
            w = x-curr;
            
            if ((w > 4) || /*(w > sizeof(buff)) ||*/ (w == 0)) {
                printf("ERROR parsing device path 3\n");
                break;
            }
            
            snprintf(buff, x-curr, "%s",&path[curr]);
            
			device->pci_dev_path[numpaths].function = (uint8_t)strtoul(buff, NULL, 16); // TODO: find dev from char *path
			
			numpaths++;
		}
	}
	
	if(!numpaths)
	{
        printf("ERROR parsing device path 4\n");
        free(device);
		return NULL;
	}
	
	device->numentries = 0x00;
	
	device->acpi_dev_path.length = 0x0c;
	device->acpi_dev_path.type = 0x02;
	device->acpi_dev_path.subtype = 0x01;
	device->acpi_dev_path._HID = 0xd041030a;
	
	device->num_pci_devpaths = numpaths;
	device->length = 24 + (6*numpaths);
	
	int		i; 
	
	for(i = 0; i < numpaths; i++)
	{
		device->pci_dev_path[i].length = 0x06;
		device->pci_dev_path[i].type = 0x01;
		device->pci_dev_path[i].subtype = 0x01;
	}
	
	device->path_end.length = 0x04;
	device->path_end.type = 0x7f;
	device->path_end.subtype = 0xff;
	
	device->string = string;
	device->data = NULL;
	string->length += device->length;
	
	if(!string->entries)
	{
		if((string->entries = (struct DevPropDevice*)malloc(sizeof(struct DevPropDevice) * MAX_STRING_NUM_ENTRIES))== NULL)
		{
            printf("ERROR parsing device path 6\n");
            
            free(device);
			return NULL;
		}
	}
	struct DevPropDevice **string_entries_arrey = (struct DevPropDevice **) string->entries;
	
	if ((string->numentries+1) < MAX_STRING_NUM_ENTRIES)
    {
		string->numentries++;
		
    }
    else
    {
        free(string->entries);
        free(device);
        return NULL;
    }	
    
	string_entries_arrey[string->numentries-1] = device;
	
	return device;
}
#endif

int devprop_add_value(struct DevPropDevice *device, char *nm, uint8_t *vl, uint32_t len)
{
	
	if(!nm || !vl || !len)
	{
		return 0;
	}
	
	uint32_t length = ((strlen(nm) * 2) + len + (2 * sizeof(uint32_t)) + 2);
	uint8_t *data = (uint8_t*)malloc(length);
	{
		if(!data)
		{
			return 0;
		}
		
		memset(data, 0, length);
		uint32_t off= 0;
		data[off+1] = ((strlen(nm) * 2) + 6) >> 8;
		data[off] =   ((strlen(nm) * 2) + 6) & 0x00FF;
		
		off += 4;
		uint32_t i=0, l = strlen(nm);
		for(i = 0 ; i < l ; i++, off += 2)
		{
			data[off] = *nm++;
		}
		
		off += 2;
		l = len;
		uint32_t *datalength = (uint32_t*)&data[off];
		*datalength = l + 4;
		off += 4;
		for(i = 0 ; i < l ; i++, off++)
		{
			data[off] = *vl++;
		}
	}	
	
	uint32_t offset = device->length - (24 + (6 * device->num_pci_devpaths));
	
	uint8_t *newdata = (uint8_t*)malloc((length + offset));
	if(!newdata)
	{
		return 0;
	}
	if(device->data)
	{
		if(offset > 1)
		{
			memcpy(newdata, device->data, offset);
		}
	}
	
	memcpy(newdata + offset, data, length);
	
	device->length += length;
	device->string->length += length;
	device->numentries++;
	
	if(!device->data)
	{
		//device->data = (uint8_t*)malloc(sizeof(uint8_t)); //IMHO this is useless
	}
	else
	{
		free(device->data);
	}
	
	free(data);
	device->data = newdata;
	
	return 1;
}

char *devprop_generate_string(struct DevPropString *string)
{
    int len = string->length * 2;
	char *buffer = (char*)malloc(len);
	char *ptr = buffer;
	
	if(!buffer)
	{
		return NULL;
	}
	
	snprintf(buffer, len, "%08x%08x%04x%04x", dp_swap32(string->length), string->WHAT2,
			dp_swap16(string->numentries), string->WHAT3);
	buffer += 24;
    len -= 24;
	int i = 0, x = 0;
	
	struct DevPropDevice **string_entries_arrey = (struct DevPropDevice **) string->entries;

	while(i < string->numentries)
	{
		if (!(i<MAX_STRING_NUM_ENTRIES)) 
        {
            break;
        }
        if (!len) {
            break;
        }
		snprintf(buffer, len, "%08x%04x%04x", dp_swap32(string_entries_arrey[i]->length),
				dp_swap16(string_entries_arrey[i]->numentries), string_entries_arrey[i]->WHAT2);
		
		buffer += 16;
        len -= 16;
		snprintf(buffer, len,"%02x%02x%04x%08x%08x", string_entries_arrey[i]->acpi_dev_path.type,
				string_entries_arrey[i]->acpi_dev_path.subtype,
				dp_swap16(string_entries_arrey[i]->acpi_dev_path.length),
				string_entries_arrey[i]->acpi_dev_path._HID,
				dp_swap32(string_entries_arrey[i]->acpi_dev_path._UID));
		
		buffer += 24;
        len -= 24;
		for(x=0;x < string_entries_arrey[i]->num_pci_devpaths; x++)
		{
            if (!len) {
                break;
            }
			snprintf(buffer, len,"%02x%02x%04x%02x%02x", string_entries_arrey[i]->pci_dev_path[x].type,
					string_entries_arrey[i]->pci_dev_path[x].subtype,
					dp_swap16(string_entries_arrey[i]->pci_dev_path[x].length),
					string_entries_arrey[i]->pci_dev_path[x].function,
					string_entries_arrey[i]->pci_dev_path[x].device);
			buffer += 12;
            len -= 12;
		}
		if (!len) {
            break;
        }
		snprintf(buffer, len,"%02x%02x%04x", string_entries_arrey[i]->path_end.type,
				string_entries_arrey[i]->path_end.subtype,
				dp_swap16(string_entries_arrey[i]->path_end.length));
		
		buffer += 8;
        len -= 8;
		uint8_t *dataptr = string_entries_arrey[i]->data;
		for(x = 0; (uint32_t)x < (string_entries_arrey[i]->length) - (24 + (6 * string_entries_arrey[i]->num_pci_devpaths)) ; x++)
		{
            if (!len) {
                break;
            }
			snprintf(buffer, len, "%02x", *dataptr++);
			buffer += 2;
            len -= 2;
		}
		i++;
	}
	return ptr;
}

void devprop_free_string(struct DevPropString *string)
{
	if(!string)
	{
		return;
	}
	
	int i;
	
	struct DevPropDevice **string_entries_arrey = (struct DevPropDevice **) string->entries;
	
	for(i = 0; i < string->numentries; i++)
	{
        if (!(i<MAX_STRING_NUM_ENTRIES)) 
        {
            break;
        }
		if(string_entries_arrey[i])
		{
			if(string_entries_arrey[i]->data)
			{
				free(string_entries_arrey[i]->data);
				string_entries_arrey[i]->data = NULL;
			}
		}
	}
    
    free(string->entries);
    string->entries = NULL;
    
	free(string);
	string = NULL;
}