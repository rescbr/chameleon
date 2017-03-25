//
//  binaryPatcher.h
//  
//  binary patcher of any kinds (kexts, kernel, ACPI tables etc..)
//
//

#ifndef binaryPatcher_h
#define binaryPatcher_h

#include <string.h>
#include <stdbool.h>
#include <libkern/OSTypes.h>
#include "saio_types.h"
#include "libsaio.h"
#include "xml.h"

#include "bootargs.h"
#include "saio_types.h"
#include "bios.h"
#include "device_tree.h"

// Micky1979: Next five functions (+ needed struct) are to split a string like "10.10.5,10.7,10.11.6,10.8.x"
// in their components separated by comma (in this case)
struct MatchOSes {
    int   count;
    char* array[100];
};

/** Returns a boolean and then enable disable the patch if MachOSEntry have a match for the booted OS. */
bool IsPatchEnabled(char *MatchOSEntry, char *CurrOS);

/** return true if a given os contains '.' as separator,
 and then match components of the current booted OS. Also allow 10.10.x format meaning all revisions
 of the 10.10 OS */
bool IsOSValid(char *MatchOS, char *CurrOS);

/** return MatchOSes struct (count+array) with the components of str that contains the given char sep as separator. */
struct MatchOSes *GetStrArraySeparatedByChar(char *str, char sep);

/** free MatchOSes struct and its array. */
void deallocMatchOSes(struct MatchOSes *s);

/** count occurrences of a given char in a char* string. */
int countOccurrences( char *s, char c );


unsigned int FindAndReplace(void *sourceData,
                            UInt32 SourceSize,
                            UInt32 StartLocation,
                            UInt8 *Search,
                            unsigned int SearchSize,
                            UInt8 *Replace,
                            int MaxReplaces);

unsigned int FindAndReplace(void *sourceData,
                            UInt32 SourceSize,
                            UInt32 StartLocation,
                            UInt8 *Search,
                            unsigned int SearchSize,
                            UInt8 *Replace,
                            int MaxReplaces);

/*
unsigned int FindAndCount(void *sourceData,
                          UInt32 SourceSize,
                          UInt8 *Search,
                          unsigned int SearchSize);
*/
void pach_binaryUsingDictionary(void *data,
                                UInt32 dataLen,
                                UInt32 StartLocation,
                                char *reason,
                                TagPtr config);

#endif /* binaryPatcher_h */
