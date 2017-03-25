//
//  binaryPatcher.c
//  
//  binary patcher of any kinds (kexts, kernel, ACPI tables etc..)
//
//

#include "binaryPatcher.h"
#include "boot.h" /* to expose gDarwinBuildVerStr */

// Clover
// Searches Source for Search pattern of size SearchSize and return number of occurrences
//
unsigned int FindAndCount(void *sourceData,
                          UInt32 SourceSize,
                          UInt32 StartLocation,
                          UInt8 *Search,
                          unsigned int SearchSize)
{
    UInt8 *Source = ( UInt8 *)sourceData;
    SourceSize += StartLocation;
    unsigned int     NumFounds = 0;
    UInt8     *End = Source + SourceSize;

    while (Source < End) {
        if (memcmp(Source, Search, SearchSize) == 0) {
            NumFounds++;
            Source += SearchSize;
        } else {
            Source++;
        }
    }
    return NumFounds;
}

// Clover
//
// Searches Source for Search pattern of size SearchSize
// and replaces it with Replace up to MaxReplaces times.
// If MaxReplaces <= 0, then there is no restriction on number of replaces.
// Replace should have the same size as Search.
// Returns number of replaces done.
//

unsigned int FindAndReplace(void *sourceData,
                            UInt32 SourceSize,
                            UInt32 StartLocation,
                            UInt8 *Search,
                            unsigned int SearchSize,
                            UInt8 *Replace,
                            int MaxReplaces)
{
    UInt8 *Source = ( UInt8 *)sourceData;
    Source += StartLocation;
    unsigned int     NumReplaces = 0;
    bool   NoReplacesRestriction = MaxReplaces <= 0;
    UInt8     *End = Source + SourceSize;
    if (!Source || !Search || !Replace || !SearchSize) {
        return 0;
    }
    
    while ((Source < End) && (NoReplacesRestriction || (MaxReplaces > 0))) {
        if (memcmp(Source, Search, SearchSize) == 0) {
            memcpy(Source, Replace, SearchSize);
            NumReplaces++;
            MaxReplaces--;
            Source += SearchSize;
        } else {
            Source++;
        }
    }
    return NumReplaces;
}

bool IsPatchEnabled (char *MatchOSEntry, char *CurrOS)
{
    int i;
    bool ret = false;
    struct MatchOSes *mos; // = malloc(sizeof(struct MatchOSes));
    
    if (!MatchOSEntry || !CurrOS) {
        return true; //undefined matched corresponds to old behavior
    }
    
    mos = GetStrArraySeparatedByChar(MatchOSEntry, ',');
    if (!mos) {
        return true; //memory fails -> anyway the patch enabled
    }
    
    for (i = 0; i < mos->count; ++i) {
        // dot represent MatchOS
        if (
            ((strstr(mos->array[i], ".") != NULL) && IsOSValid(mos->array[i], CurrOS)) || // MatchOS
            (strstr(mos->array[i], CurrOS) != NULL) // MatchBuild
            ) {
            ret =  true;
            break;
        }
    }
    deallocMatchOSes(mos);
    return ret;
}
// FIXME
// the following is an improved versione of what Clover have
// but malloc in Enoch fails to allocate memory (also using the same code in Clover ).
// For now we use a buffer and for this reason deallocMatchOSes() does nothing..
struct MatchOSes *GetStrArraySeparatedByChar(char *str, char sep)
{
    char buffer[strlen(str) +1];
    struct MatchOSes *mo;
    int len = 0, i = 0, inc = 1;
    
    char doubleSep[2];
    
    mo = malloc(sizeof(struct MatchOSes));
    if (!mo) {
        return NULL;
    }
    mo->count = countOccurrences( str, sep ) + 1;
    len = (int)strlen(str);
    doubleSep[0] = sep; doubleSep[1] = sep;
    
    if(strstr(str, doubleSep) || !len || str[0] == sep || str[len -1] == sep) {
        mo->count = 0;
        mo->array[0] = NULL;
        return mo;
    }
    
    if (mo->count > 1) {
        int *indexes = (int *) malloc(mo->count + 1);
        
        for (i = 0; i < len; ++i) {
            char c = str[i];
            if (c == sep) {
                indexes[inc]=i;
                inc++;
            }
        }
        
        indexes[0] = 0; // manually add first index
        indexes[mo->count] = len; // manually add last index
        
        int startLocation = 0, endLocation = 0;
        
        for (i = 0; i < mo->count; ++i) {
            unsigned int newLen = 0;
            startLocation = i ? indexes[i] + 1 : indexes[0];
            endLocation = (i == mo->count - 1) ? len : indexes[i + 1];
            
            newLen = (endLocation - startLocation);
            
            //char *lastStr = (char *) malloc(newLen+1);
            //strncpy(lastStr, str + startLocation, newLen);
            strncpy(buffer, str + startLocation, newLen);
            buffer[newLen /*strlen(lastStr)*/] = '\0';
            mo->array[i] = buffer;
            
            //printf("%s [len = %lu]\n", mo->array[i], strlen(mo->array[i]));
            if (endLocation == len) break;
        }
        
        free(indexes);
    }
    else {
        //char *lastStr = (char *) malloc(strlen(str)+1);
        //strncpy(lastStr, str, strlen(str));
        strncpy(buffer, str, strlen(str));
        buffer[strlen(str)] = '\0';
        mo->array[0] = buffer;
        //printf("%s [len = %lu]\n", mo->array[0], strlen(mo->array[0]));
    }
    //printf("------------\n");
    
    return mo;
}

void deallocMatchOSes(struct MatchOSes *s)
{
    /*
    int i;
    
    if (!s) {
        return;
    }
    
    for (i = 0; i < s->count; i++) {
        if (s->array[i]) {
            free(s->array[i]);
        }
    }
    free(s);
    */
}

bool IsOSValid(char *MatchOS, char *CurrOS)
{
    /* example for valid matches are:
     10.7, only 10.7 (10.7.1 will be skipped)
     10.10.2 only 10.10.2 (10.10.1 or 10.10.5 will be skipped)
     10.10.x (or 10.10.X), in this case is valid for all minor version of 10.10 (10.10.(0-9))
     */
    
    bool ret = false;
    struct MatchOSes *osToc;
    struct MatchOSes *currOStoc;
    
    if (!MatchOS || !CurrOS) {
        return true; //undefined matched corresponds to old behavior
    }
    
    osToc = GetStrArraySeparatedByChar(MatchOS, '.');
    currOStoc = GetStrArraySeparatedByChar(CurrOS,  '.');
    
    if (osToc->count == 2) {
        if (strcmp(osToc->array[0], currOStoc->array[0]) == 0
            && strcmp(osToc->array[1], currOStoc->array[1]) == 0) {
            ret = true;
        }
    } else if (osToc->count == 3) {
        if (currOStoc->count == 3) {
            if (strcmp(osToc->array[0], currOStoc->array[0]) == 0
                && strcmp(osToc->array[1], currOStoc->array[1]) == 0
                && strcmp(osToc->array[2], currOStoc->array[2]) == 0) {
                ret = true;
            } else if (strcmp(osToc->array[0], currOStoc->array[0]) == 0
                       && strcmp(osToc->array[1], currOStoc->array[1]) == 0
                       && (strcmp(osToc->array[2], "x") == 0 || strcmp(osToc->array[2], "X") == 0)) {
                ret = true;
            }
        } else if (currOStoc->count == 2) {
            if (strcmp(osToc->array[0], currOStoc->array[0]) == 0
                && strcmp(osToc->array[1], currOStoc->array[1]) == 0) {
                ret = true;
            } else if (strcmp(osToc->array[0], currOStoc->array[0]) == 0
                       && strcmp(osToc->array[1], currOStoc->array[1]) == 0
                       && (strcmp(osToc->array[2], "x") == 0 || strcmp(osToc->array[2], "X") == 0)) {
                ret = true;
            }
        }
        
    }
    
    deallocMatchOSes(osToc);
    deallocMatchOSes(currOStoc);
    return ret;
}

int countOccurrences( char *s, char c )
{
    return *s == '\0'
    ? 0
    : countOccurrences( s + 1, c ) + (*s == c);
}
// End of MatchOS


//
// Micky1979
//
// Function to iterate the kernel.plist/kexts.plist (Acpi.plist?) and apply patches
//
void pach_binaryUsingDictionary(void *data,
                                UInt32 dataLen,
                                UInt32 StartLocation,
                                char *reason,
                                TagPtr config)
{
    bool canPatch = false;
    UInt8 *bytes = (UInt8 *)data;
    int	numPatch, count;
    numPatch = -1;
    count = 0;
    char * realReason;
    TagPtr		PatchTag = NULL;
    
    if (!config) return;
    
    /* coming soon..
    if (!strcmp(reason, "AcpiPatches")) {
        // on all the acpi tables
        realReason = "Acpi";
    }
    else
        if (!strcmp(reason, "DsdtPatches")) {
            // only on dsdt
            realReason = "Dsdt";
        }
        else
            */
            if (!strcmp(reason, "KernelPatches")) {
                realReason = "kernel";
            }
            else
            {
                realReason = reason;
            }

    if ((PatchTag = XMLCastArray(XMLGetProperty(config, (const char *)reason))))
    {
        count = XMLTagCount(PatchTag);
        
        /* for (i=0; i<count; i++) normal */
        for (unsigned i = count ; i-- > 0 ;) /* reversed iteration since xml.c add it reversed */
        {
            numPatch ++;
            if (!bytes) {
                verbose("\t  Skipping patch for %s because no bytes to patch..\n", realReason);
                return;
            }
            
            TagPtr index = XMLGetElement( PatchTag, i );
            if (index)
            {
                char *Comment    = NULL;
                char *MatchOS    = NULL;
                char *MatchBuild = NULL;
                char *MatchUUID  = NULL;
                
                int numPatches = 0;
                
                TagPtr FindPtr    = XMLGetProperty(index, (const char*)"Find");
                TagPtr ReplacePtr = XMLGetProperty(index, (const char*)"Replace");

                Comment     = XMLCastString(XMLGetProperty(index, (const char*)"Comment"));
                MatchOS     = XMLCastString(XMLGetProperty(index, (const char*)"MatchOS"));
                MatchBuild  = XMLCastString(XMLGetProperty(index, (const char*)"MatchBuild"));
                MatchUUID   = XMLCastString(XMLGetProperty(index, (const char*)"MatchUUID"));
                
                Comment     = (Comment    != NULL && strlen(Comment)    >0) ? Comment    : "untitled";
                MatchOS     = (MatchOS    != NULL && strlen(MatchOS)    >0) ? MatchOS    : ""; // if lenght is 0 IsPatchEnabled() will not be called!
                MatchBuild  = (MatchBuild != NULL && strlen(MatchBuild) >0) ? MatchBuild : "";
                MatchUUID   = (MatchUUID  != NULL && strlen(MatchUUID)  >0) ? MatchUUID  : "";
                
                canPatch = true;
                // the order is important to skip patches
                if (strlen(MatchUUID))
                {
                    // to be implemented
                    // check MatchUUID and disable this patch if does not match. Usefull to load or skip patches for certain volumes
                    // canPatch = IsPatchEnabled(MatchUUID, (char *)uuid_to_be_taken_somewhere);
                }
                else
                {
                    MatchUUID = "not implemented";
                }
                
                if (strlen(MatchOS))
                {
                    // check MatchOS and disable this patch if does not match
                    canPatch = IsPatchEnabled(MatchOS, (char *)gMacOSVersion);
                }
                else
                {
                    MatchOS = "not set";
                }
                
                if (strlen(MatchBuild))
                {
                    // check MatchBuild and disable this patch if does not match
                    canPatch = IsPatchEnabled(MatchBuild, (char *)gMacOSVersion);
                }
                else
                {
                    MatchBuild = "not set";
                }

                char  buf[1024];
                sprintf(buf, "\tPatching %s [Item %d] (%s) MatchOS[ %s ] MatchBuild[ %s ]: ",
                        realReason, numPatch, Comment, MatchOS, MatchBuild);
                verbose("%s", buf);
                
                if (canPatch)
                {
                    if (FindPtr && ReplacePtr)
                    {
                        if (!XMLIsData(FindPtr))
                        {
                            verbose("\n\t\t\tUser Error, Find not in data tag, patch skipped\n");
                            return;
                        }
                        
                        if (!XMLIsData(ReplacePtr))
                        {
                            verbose("\n\t\t\tUser Error, Replace not in data tag, patch skipped\n");
                            return;
                        }
                        
                        // don't allow patches with less than 2 bytes (it's ridiculous)
                        if (sizeof(FindPtr->data) <= 2)
                        {
                            verbose("\n\t\t\tUser Error, Find is less than 2 bytes (or less), patch skipped\n");
                            return;
                        }
                        
                        if (sizeof(FindPtr->data) != sizeof(ReplacePtr->data))
                        {
                            verbose("\n\t\t\tUser Error, Find and Replace does not have the same length, patch skipped\n");
                            return;
                        }
                        
                        if (sizeof(FindPtr->data) > (dataLen - StartLocation))
                        {
                            verbose("\n\t\t\tUser Error, Find is bigger than the hole data, patch skipped\n");
                            return;
                        }
                    }
                    else
                    {
                        verbose("\n\t\t\tUser Error, Find or Replace (or both) are missing, patch skipped\n");
                        return;
                    }
                }
                
                
                // patch it
                if (canPatch) {
                    numPatches = FindAndReplace(data,
                                                (UInt32)dataLen,
                                                StartLocation,
                                                FindPtr->data,
                                                sizeof(FindPtr->data),
                                                ReplacePtr->data,
                                                0);
                    verbose("%d substitutions made!\n", numPatches);
                }
                else
                {
                    verbose("disabled!\n");
                }
            }
        }
    }
}
