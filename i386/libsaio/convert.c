/*
 * Convert.c
 *   Implement conversion utility functions
 *   Create UUID parsing functions and gather other conversion routines
 *   --Rek
 */

#include "convert.h"

/** convert URL to a string */
char * newStringFromURL(char* string)
{    
	char * buffer = newString(string);
	
	int maxlen = strlen(buffer);	
	
	// replace "%20" by spaces.
	int len = 0;
	while (buffer[len] != '\0') {
		if (buffer[len] == '%' && buffer[len+1] == '2' && buffer[len+2] == '0')
		{
			buffer[len] = ' ';
			
			strlcpy(&buffer[len+1], &buffer[len+3], maxlen - (len+1)); 			
		}
		len++;
	}
	
	//DBG("%s maxlen : %d, newlen : %lu\n",buffer, maxlen, strlen(buffer));
	// This will leak a little bit, i mean as you can see the final string will be slightly smaller than the allocated string buffer, 
	// to fix this you can realloc the buffer, or do another newString(xxx) then free the first buffer, i choose to do nothing.
	
    return buffer ;
}

/** Transform a 16 bytes hexadecimal value UUID to a string */
const char * getStringFromUUID(const EFI_CHAR8* eUUID)
{
    static char msg[UUID_STR_LEN] = "";
    if (!eUUID) return "";
    const unsigned char * uuid = (unsigned char*) eUUID;
    snprintf(msg, sizeof(msg), "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		    uuid[0], uuid[1], uuid[2], uuid[3], 
		    uuid[4], uuid[5], uuid[6], uuid[7],
		    uuid[8], uuid[9], uuid[10],uuid[11],
		    uuid[12],uuid[13],uuid[14],uuid[15]);
    return msg ;
}

/** Parse an UUID string into an (EFI_CHAR8*) buffer */
EFI_CHAR8*  getUUIDFromString(const char *source)
{
    if (!source) return 0;
    
	char	*p = (char *)source;
	int	i;
	char	buf[3];
	static EFI_CHAR8 uuid[UUID_LEN+1]="";
    
	buf[2] = '\0';
	for (i=0; i<UUID_LEN; i++) {
		if (p[0] == '\0' || p[1] == '\0' || !isxdigit(p[0]) || !isxdigit(p[1])) {
			verbose("[ERROR] UUID='%s' syntax error\n", source);
			return 0;
		}
		buf[0] = *p++;
		buf[1] = *p++;
		uuid[i] = (unsigned char) strtoul(buf, NULL, 16);
		if (*p == '-' && (i % 2) == 1 && i < UUID_LEN - 1) {
			p++;
		}
	}
	uuid[UUID_LEN]='\0';
    
	if (*p != '\0') {
		verbose("[ERROR] UUID='%s' syntax error\n", source);
		return 0;
	}
	return uuid;
}

/** XXX AsereBLN replace by strtoul */
#if UNUSED
uint32_t ascii_hex_to_int(char *buff) 
{
	uint32_t	value = 0, i, digit;
	for(i = 0; i < (uint32_t)strlen(buff); i++)
	{
		if (buff[i] >= 48 && buff[i] <= 57)			// '0' through '9'
			digit = buff[i] - 48;	
		else if (buff[i] >= 65 && buff[i] <= 70)	// 'A' through 'F'
			digit = buff[i] - 55;
		else if (buff[i] >= 97 && buff[i] <= 102)	// 'a' through 'f'
			digit = buff[i] - 87;
		else
			return value;
		
		value = digit + 16 * value;
	}
	return	value;
}
#endif

void *convertHexStr2Binary(const char *hexStr, int *outLength)
{
    int len;
    char hexNibble;
    char hexByte[2];
    uint8_t binChar;
    uint8_t *binStr;
    int hexStrIdx, binStrIdx, hexNibbleIdx;
    
    len = strlen(hexStr);
    if (len > 1)
    {
        // the resulting binary will be the half size of the input hex string
        binStr = malloc(len / 2);
        if (!binStr) {
            *outLength = 0;
            return NULL;
        }
        binStrIdx = 0;
        hexNibbleIdx = 0;
        for (hexStrIdx = 0; hexStrIdx < len; hexStrIdx++)
        {
            hexNibble = hexStr[hexStrIdx];
            
            // ignore all chars except valid hex numbers
            if ((hexNibble >= '0' && hexNibble <= '9')
                || (hexNibble >= 'A' && hexNibble <= 'F')
                || (hexNibble >= 'a' && hexNibble <= 'f'))
            {
                hexByte[hexNibbleIdx++] = hexNibble;
                
                // found both two nibbles, convert to binary
                if (hexNibbleIdx == 2)
                {
                    binChar = 0;
                    
                    for (hexNibbleIdx = 0; (unsigned)hexNibbleIdx < sizeof(hexByte); hexNibbleIdx++)
                    {
                        if (hexNibbleIdx > 0) binChar = binChar << 4;
                        
                        if (hexByte[hexNibbleIdx] <= '9') binChar += hexByte[hexNibbleIdx] - '0';
                        else if (hexByte[hexNibbleIdx] <= 'F') binChar += hexByte[hexNibbleIdx] - ('A' - 10);
                        else if (hexByte[hexNibbleIdx] <= 'f') binChar += hexByte[hexNibbleIdx] - ('a' - 10);
                    }
                    
                    binStr[binStrIdx++] = binChar;						
                    hexNibbleIdx = 0;
                }
            }
        }
        *outLength = binStrIdx;
        return binStr;
    }
    else
    {
        *outLength = 0;
        return NULL;
    }
}