#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "acpi.h"

#ifndef DEBUG_ACPI
#define DEBUG_ACPI 1
#endif

#if DEBUG_ACPI!=0
#define DBG(x...)  printf(x)
#else
#define DBG(x...)
#endif

/** Search and try opening an acpi table matching a spec defined by key, should be reused anywhere needed:*/
int acpiSearchAndGetFd(const char * key, const char ** outDirspec)
{
    int fd=0;
    const char * overriden_pathname=NULL;
    char filename[64];
    static char dirspec[512]="";
    int len=0,i=0;
    
    sprintf(filename, "%s.aml", key);
    
    /// Take in account user overriding for 'DSDT'.aml table
    // and try to find <key>.aml at the same place
    if (getValueForKey("DSDT", &overriden_pathname, &len,  
                       &bootInfo->bootConfig) && len>0)
    {
        strcpy(dirspec, overriden_pathname);
        for (i=len-1; i>=0; i--)
        {
            if (dirspec[i]=='/')
            {
                dirspec[i+1]='\0';
                strcat(dirspec, filename);
                break;
            }
            else if (i==0) // no '/' found copy filename and seek in current directory
            {
                strcpy(dirspec, filename);
                break;
            }
        }
        fd=open (dirspec,0);
        if (fd>=0) goto success_fd;
    }

    // Start searching any potential location for ACPI Table
    sprintf(dirspec, "/%s", filename); // start searching root
    fd=open (dirspec,0);
    if (fd>=0) goto success_fd;

    sprintf(dirspec, "%s", filename); // start current dir
    fd=open (dirspec,0);
    if (fd>=0) goto success_fd;

    sprintf(dirspec,"/Extra/%s",filename);
    fd=open (dirspec,0);
    if (fd>=0) goto success_fd;

    sprintf(dirspec,"bt(0,0)/Extra/%s",filename);
    fd=open (dirspec,0);
    if (fd>=0) goto success_fd;

    // NOT FOUND:
    verbose("ACPI Table not found: %s\n", filename);
    if (outDirspec) *outDirspec = "";
    return -1;
    // FOUND
  success_fd:
    if (outDirspec) *outDirspec = dirspec; 
    return fd;
}

/** Load a table in kernel memory and return a pointer to it if found, NULL otherwise */
void *acpiLoadTable (const char * key)
{
    void *tableAddr;
    const char * dirspec=NULL;
	
    int fd = acpiSearchAndGetFd(key, &dirspec);

    if (fd>=0)
    {
        tableAddr=(void*)AllocateKernelMemory(file_size (fd));
        if (tableAddr)
        {
            if (read (fd, tableAddr, file_size (fd))!=file_size (fd))
            {
                printf("Couldn't read table %s\n",dirspec);
                free (tableAddr);
                close (fd);
                return NULL;
            }
	      
            DBG("Table %s read and stored at: %x\n", dirspec, tableAddr);
            close (fd);
            return tableAddr;
        }
        close (fd);
    }  
    printf("Couldn't allocate memory for table %s\n", dirspec);
    return NULL;
}

/** Load an SSDTx into memory, code could be factorized here using acpiSearchAndGetFd() */
void * acpiLoadSSDTTable(int ssdt_number)
{
    char filename[64];

    sprintf(filename, "SSDT-%d", ssdt_number);
    return acpiLoadTable(filename);
}

/** Gets the ACPI 1.0 RSDP address */
struct acpi_2_rsdp* acpiGetAddressOfTable10()
{
    /* TODO: Before searching the BIOS space we are supposed to search the first 1K of the EBDA */
	
    void *acpi_addr = (void*)ACPI_RANGE_START;
    for(; acpi_addr <= (void*)ACPI_RANGE_END; acpi_addr += 16)
    {
        if(*(uint64_t *)acpi_addr == ACPI_SIGNATURE_UINT64_LE)
        {
            uint8_t csum = checksum8(acpi_addr, 20);
            if(csum == 0)
            {
                // Only return the table if it is a true version 1.0 table (Revision 0)
                if(((struct acpi_2_rsdp*)acpi_addr)->Revision == 0)
                    return acpi_addr;
            }
        }
    }
    return NULL;
}

/** Gets the ACPI 2.0 RSDP address */
struct acpi_2_rsdp* acpiGetAddressOfTable20()
{
    /* TODO: Before searching the BIOS space we are supposed to search the first 1K of the EBDA */
	
    void *acpi_addr = (void*)ACPI_RANGE_START;
    for(; acpi_addr <= (void*)ACPI_RANGE_END; acpi_addr += 16)
    {
        if(*(uint64_t *)acpi_addr == ACPI_SIGNATURE_UINT64_LE)
        {
            uint8_t csum = checksum8(acpi_addr, 20);

            /* Only assume this is a 2.0 or better table if the revision is greater than 0
             * NOTE: ACPI 3.0 spec only seems to say that 1.0 tables have revision 1
             * and that the current revision is 2.. I am going to assume that rev > 0 is 2.0.
             */

            if(csum == 0 && (((struct acpi_2_rsdp*)acpi_addr)->Revision > 0))
            {
                uint8_t csum2 = checksum8(acpi_addr, sizeof(struct acpi_2_rsdp));
                if(csum2 == 0)
                    return acpi_addr;
            }
        }
    }
    return NULL;
}

/** Fills an ACPI 2.0 GAS structure */
struct acpi_2_gas acpiFillGASStruct(uint32_t Address, uint8_t Length)
{
    struct acpi_2_gas TmpGAS;

    TmpGAS.Address_Space_ID = 1; /* I/O Address */

    if (Address == 0)
    {
        TmpGAS.Register_Bit_Width = 0;
    } else {
        TmpGAS.Register_Bit_Width = Length * 8;
    }

    TmpGAS.Register_Bit_Offset = 0;
    TmpGAS.Access_Size = 0; /* Not set for Legacy reasons... */
    TmpGAS.Address = (uint64_t)Address;

    return(TmpGAS);
}

/* Handling acpi dropped tables enumeration */

static struct ACPIDropTableEntry sACPIKeyDropTable[16];
const int sACPIKeyDropTableSize = sizeof(sACPIKeyDropTableSize) / sizeof(char);
static int sACPIKeyCount = 0;
static bool bACPIDropTableInitialized=false;

/* initialize the the dropped table array from user specification, if not done already*/
static void acpiInitializeDropTables()
{
    if (bACPIDropTableInitialized) return;
    const char * dropTablesEnum = NULL;
    int len = 0;
    bool bOverride = getValueForKey(kDrop, &dropTablesEnum, &len, &bootInfo->bootConfig);
    bACPIDropTableInitialized = true;
    bzero(sACPIKeyDropTable, sACPIKeyDropTableSize);

    if (!bOverride || !dropTablesEnum || len==0) return;

    // Now transform the whitespace delimited enum into an array of keys:
    char buffer[sACPIKeyDropTableSize*8];
    int i,j;
    // skip beginning whitespaces
    for (i=0; i < len; i++) 
        if (dropTablesEnum[i]!=' ' && dropTablesEnum[i]!='\t')
            break;
    for (;i < len; i++) 
    {
        j=0;
        *buffer = '\0';
        while(i<len && dropTablesEnum[i]!=' ' && dropTablesEnum[i]!='\t')
            buffer[j++] = dropTablesEnum[i++];
        buffer[j] = '\0';
        if(j>0)
        {
            DBG("ACPI: Adding Key[%d]: %s\n", sACPIKeyCount, buffer);
            strncpy(sACPIKeyDropTable[sACPIKeyCount++].key,buffer, ACPI_KEY_MAX_SIZE);
        }
    }
}

static int _current = 0;
/** Get first acpi drop key*/
struct ACPIDropTableEntry* acpiGetFirstDropTable()
{
    acpiInitializeDropTables();
    _current = 0;
    return sACPIKeyCount>0 ? &sACPIKeyDropTable[0] : NULL;
}

/** Get Next acpi drop key*/
struct ACPIDropTableEntry* acpiGetNextDropTable()
{
    acpiInitializeDropTables();
    return (_current<sACPIKeyCount) ?  &sACPIKeyDropTable[_current++] : NULL;
}
  
/* Gets and aml table name and in input and return ture if it is in the DROP list*/
bool acpiIsTableDropped(const char * key)
{
// Note: input key can be non zero terminated
    struct ACPIDropTableEntry* p;
    for (p = acpiGetFirstDropTable(); p; p = acpiGetNextDropTable())
    {
        if (p->key[0]==key[0] && p->key[1]==key[1] && 
            p->key[2]==key[2] && p->key[3]==key[3])
        {
            DBG("ACPI: Key: %s found in the drop list\n", p);
            return true;
        }
    }
    DBG("ACPI: Key: %c%c%c%c NOT found in the drop list\n", p);
    return false;
}
/* Return the content of user acpi table if it has been loaded sucessfully, NULL otherwise*/
void * acpiTableUserContent(const char * key)
{
// Note: input key can be non zero terminated
    struct ACPIDropTableEntry* p;
    for (p = acpiGetFirstDropTable(); p; p = acpiGetNextDropTable())
    {
        if (p->key[0]==key[0] && p->key[1]==key[1] && 
            p->key[2]==key[2] && p->key[3]==key[3])
        {
            return p->content;
        }
    }
    return NULL;
}

/** For each table in the drop list, try to load an equivalent user table */
void acpiLoadUserTables()
{
    struct ACPIDropTableEntry* p;
    for (p = acpiGetFirstDropTable(); p; p = acpiGetNextDropTable())
    {
        p->content = acpiLoadTable(p->key);

        if (p->content)
        {
            DBG("ACPI: User Table %s loaded in Memory\n", p->key);
        }
    }
}
