/*
 * Convert.c
 *   Implement conversion utility functions
 *   Create UUID parsing functions and gather other conversion routines
 *   --Rek
 */

#include "convert.h"

/* Return a string that is the representation of a 16 bytes UUID */
void getStringFromUUID(const uuid_t uuid, uuid_string_t out)
{
    sprintf((char*) out, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            uuid[0], uuid[1], uuid[2], uuid[3],
            uuid[4], uuid[5], uuid[6], uuid[7],
            uuid[8], uuid[9], uuid[10],uuid[11],
            uuid[12],uuid[13],uuid[14],uuid[15]);
}

/* Parse an UUID string and return a new allocated UUID */
uuid_t* newUUIDFromString(const char *source)
{
    if (! source) return NULL;

    const char* p = source;
    int   i;
    int   len;
    char  uuid_hex[UUID_LEN*2+1];

    // Check if UUID is valid
    for (i=0; *p != 0 && i < UUID_LEN*2; p++) {
        if (*p == '-')
            continue;

        if (!isxdigit(*p)) {
            return NULL;
        }
        uuid_hex[i++] = *p;
    }

    // invalid size
    if (*p != 0 || i != UUID_LEN*2) {
        return NULL;
    }

    uuid_hex[i] = 0; // null terminated string

    return convertHexStr2Binary(uuid_hex, &len);
}

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
    binStrIdx = 0;
    hexNibbleIdx = 0;
    for (hexStrIdx = 0; hexStrIdx < len; hexStrIdx++)
    {
      hexNibble = hexStr[hexStrIdx];

      // ignore all chars except valid hex numbers
      if (hexNibble >= '0' && hexNibble <= '9'
        || hexNibble >= 'A' && hexNibble <= 'F'
        || hexNibble >= 'a' && hexNibble <= 'f')
      {
        hexByte[hexNibbleIdx++] = hexNibble;

        // found both two nibbles, convert to binary
        if (hexNibbleIdx == 2)
        {
          binChar = 0;

          for (hexNibbleIdx = 0; hexNibbleIdx < sizeof(hexByte); hexNibbleIdx++)
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
