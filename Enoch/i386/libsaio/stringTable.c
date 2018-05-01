/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Copyright 1993 NeXT, Inc.
 * All rights reserved.
 */

#include "bootstruct.h"
#include "libsaio.h"
#include "boot.h"
#include "xml.h"

extern char *Language;
extern char *LoadableFamilies;

bool sysConfigValid;

#if UNUSED

/*
 * Compare a string to a key with quoted characters
 */
static inline int keyncmp(const char *str, const char *key, int n)
{
    int c;
    while (n--) {
	c = *key++;
	if (c == '\\') {
	    switch(c = *key++) {
	    case 'n':
		c = '\n';
		break;
	    case 'r':
		c = '\r';
		break;
	    case 't':
		c = '\t';
		break;
	    default:
		break;
	    }
	} else if (c == '\"') {
	    // Premature end of key
	    return 1;
	}
	if (c != *str++) {
	    return 1;
	}
    }
    return 0;
}

static void eatThru(char val, const char **table_p)
{
	register const char *table = *table_p;
	register bool found = false;

	while (*table && !found)
	{
		if (*table == '\\') table += 2;
		else
		{
			if (*table == val) found = true;
			table++;
		}
	}
	*table_p = table;
}

/* Remove key and its associated value from the table. */
bool removeKeyFromTable(const char *key, char *table)
{
    register int len;
    register char *tab;
    char *buf;

    len = strlen(key);
    tab = (char *)table;
    buf = (char *)malloc(len + 3);

    snprintf(buf, len + 3,"\"%s\"", key);
    len = strlen(buf);

    while(*tab) {
        if(strncmp(buf, tab, len) == 0) {
            char c;

            while((c = *(tab + len)) != ';') {
                if(c == 0) {
                    len = -1;
                    goto out;
                }
                len++;
            }
            len++;
            if(*(tab + len) == '\n') len++;
            goto out;
        }
        tab++;
    }
    len = -1;
out:
    free(buf);

    if(len == -1) return false;

    while((*tab = *(tab + len))) {
        tab++;
    }

    return true;
}

char *newStringFromList(char **list, int *size)
{
	char *begin = *list, *end;
	char *newstr;
	int newsize = *size;
	int bufsize;
    
	while (*begin && newsize && isspace(*begin)) {
		begin++;
		newsize--;
	}
	end = begin;
	while (*end && newsize && !isspace(*end)) {
		end++;
		newsize--;
	}
	if (begin == end) {
		return 0;
	}
	bufsize = end - begin + 1;
	newstr = malloc(bufsize);
	if (!newstr)
	{
		return 0;
	}
	strlcpy(newstr, begin, bufsize);
	*list = end;
	*size = newsize;
	return newstr;
}

#endif

/* 
 * compress == compress escaped characters to one character
 */
int stringLength(const char *table, int compress)
{
	int ret = 0;

	while (*table) {
		if (*table == '\\') {
			table += 2;
			ret += 1 + (compress ? 0 : 1);
		}
		else
		{
			if (*table == '\"')
			{
				return ret;
			}
			ret++;
			table++;
		}
	}
	return ret;
}


bool getValueForConfigTableKey(config_file_t *config, const char *key, const char **val, int *size)
{
	if (config->dictionary != 0 ) {
		// Look up key in XML dictionary
		TagPtr value;
		value = XMLGetProperty(config->dictionary, key);
		if (value != 0) {
			if (value->type != kTagTypeString) {
				error("Non-string tag '%s' found in config file\n", key);
				return false;
			}
			*val = value->string;
			*size = strlen(value->string);
			return true;
		}
	} else {

		// Legacy plist-style table

	}

	return false;
}

#if UNUSED

/*
 * Returns a new malloc'ed string if one is found
 * in the string table matching 'key'.  Also translates
 * \n escapes in the string.
 */
char *newStringForStringTableKey(char *table, char *key, config_file_t *config)
{
	const char *val;
	char *newstr, *p;
	int size;
    
	if (getValueForConfigTableKey(config, key, &val, &size))
	{
		newstr = (char *)malloc(size+1);
		if (!newstr)
		{
			return 0;
		}
		for (p = newstr; size; size--, p++, val++) {
			if ((*p = *val) == '\\') {
				switch (*++val) {
					case 'r':
						*p = '\r';
					break;
					case 'n':
						*p = '\n';
					break;
					case 't':
						*p = '\t';
					break;
					default:
						*p = *val;
					break;
				}
				size--;
			}
		}
		*p = '\0';
		return newstr;
	} else {
		return 0;
	}
}

#endif

char *newStringForKey(char *key, config_file_t *config)
{
	const char *val;
	char *newstr;
	int size;
    
	if (getValueForKey(key, &val, &size, config) && size) {
		newstr = (char *)malloc(size + 1);
		if (!newstr)
		{
			return 0;
		}
		strlcpy(newstr, val, size + 1);
		return newstr;
	} else {
		return 0;
	}
}

/* parse a command line
 * in the form: [<argument> ...]  [<option>=<value> ...]
 * both <option> and <value> must be either composed of
 * non-whitespace characters, or enclosed in quotes.
 */
static const char *getToken(const char *line, const char **begin, int *len)
{
	if (*line == '\"') {
		*begin = ++line;
		while (*line && *line != '\"') {
			line++;
		}
		*len = line++ - *begin;
	} else {
		*begin = line;
		while (*line && !isspace(*line) && *line != '=') {
			line++;
		}
		*len = line - *begin;
	}
	return line;
}

bool getValueForBootKey(const char *line, const char *match, const char **matchval, int *len)
{
	const char *key, *value;
	int key_len, value_len;
	bool retval = false;
    
	while (*line) {
		/* look for keyword or argument */
		while (isspace(*line))
		{
			line++;
		}

		/* now look for '=' or whitespace */
		line = getToken(line, &key, &key_len);
		/* line now points to '=' or space */
		if (*line && !isspace(*line)) {
			line = getToken(++line, &value, &value_len);
		} else {
			value = line;
			value_len = 0;
		}
		if ((strlen(match) == key_len) && strncmp(match, key, key_len) == 0)
		{
			// create a new string
			char* newstr = malloc(value_len + 1);
			strncpy(newstr, value, value_len);
			newstr[value_len] = 0;

			*matchval = newstr;
			*len = value_len;
			retval = true;
			/* Continue to look for this key; last one wins. */
		}
	}

	return retval;
}

/* Return NULL if no option has been successfully retrieved, or the string otherwise */
const char *getStringForKey(const char *key,  config_file_t *config)
{
	static const char *value = 0;
	int len = 0;
	if (!getValueForKey(key, &value, &len, config))
	{
		value = 0;
	}
	return value;
}

/* Returns TRUE if a value was found, FALSE otherwise.
 * The boolean value of the key is stored in 'val'.
 */
bool getBoolForKey(const char *key, bool *result_val, config_file_t *config)
{
	const char *key_val;
	int size;
    
	// looking for real boolean (<true/> or <false/>)
	// if is a boolean tag, return immediately
	TagPtr entry = XMLGetProperty(config->dictionary,key);

	if(XMLIsBoolean(entry))
	{
		int value = XMLCastBoolean(entry);
		if (value) {
			*result_val = true;
			return true;
		}
		else
		{
			*result_val = false;
			return false;
		}
	}

	// check if is a boolean as "string" (Yes/No)
	// (IMHO this should be deprecated soon)
	if (getValueForKey(key, &key_val, &size, config))
	{
		if ((size >= 1) && (key_val[0] == 'Y' || key_val[0] == 'y'))
		{
			*result_val = true;
		}
		else
		{
			*result_val = false;
		}
		return true;
	}
	return false;
}

bool getIntForKey(const char *key, int *value, config_file_t *config)
{
	const char *string = NULL;
	int         size   = 0;
    
	if (getValueForKey(key, &string, &size, config))
	{
		if (size)
		{
			char               *end  = NULL;
			unsigned long long  nres = 0;
			long long           sres = 0;

			/*
			 * strtouq is used here to properly detect any overflow and still
			 * accepting extreme values like INT_MIN and UINT_MAX.
			 */
			nres = strtouq(string, &end, 0);

			/*
			 * basic error and over/overflow detection.
			 * needed since strtouq() basically always return something.
			 */
			if (end == string) {
				// no digits found
				return false;
			} else if (nres == UQUAD_MAX) {
				// overflow occured: value is too large.
				return false;
			}

			/*
			 * Now we make sure that the signed value or unsigned positive
			 * value can be stored in a 32-bit integer.
			 */
			sres = (long long)nres;

			if (sres < INT_MIN) {
				// signed int underflow:
				// if the value is negative, it must be signed, and therefore
				// cannot be less than INT_MAX.
				return false;
			} else if (sres > UINT_MAX) {
				// unsigned int overflow:
				// when reading 32 hex values, the output value is way above
				// INT_MAX and should be treated as an unsigned int value.
				return false;
			}

			if (value != NULL) {
				*value = (int)sres;
			}

			return true;
		}
	}

	return false;
}
/*
 *
 */
bool getDimensionForKey( const char *key, unsigned int *value, config_file_t *config, unsigned int dimension_max, unsigned int object_size )
{
	const char *val;

	int size = 0;
	int sum = 0;

	bool negative = false;
	bool percentage = false;

	if (getValueForKey(key, &val, &size, config))
	{
		if ( size )
		{
			if (*val == '-')
			{
				negative = true;
				val++;
				size--;
			}

			if (val[size-1] == '%')
			{
				percentage = true;
				size--;
			}

			// convert string to integer
			for (sum = 0; size > 0; size--)
			{
				if (*val < '0' || *val > '9')
				{
					return false;
				}

				sum = (sum * 10) + (*val++ - '0');
			}

			if (percentage)
			{
				sum = ( dimension_max * sum ) / 100;
			}

			// calculate offset from opposite origin
			if (negative)
			{
				sum =  ( ( dimension_max - object_size ) - sum );
			}

		}
		else
		{

			// null value calculate center
			sum = ( dimension_max - object_size ) / 2;

		}

		*value = (uint16_t) sum;
		return true;
	}

	// key not found
	return false;
}

/*
 *	get color value from plist format #RRGGBB
 */
bool getColorForKey( const char *key, unsigned int *value, config_file_t *config )
{
	const char *val;
	int size;

	if (getValueForKey(key, &val, &size, config))
	{
		if (*val == '#') {
			val++;
			*value = strtol(val, NULL, 16);
			return true;
		}
	}
	return false;
}

bool getValueForKey( const char *key, const char **val, int *size, config_file_t *config )
{
	const char *overrideVal;
	int overrideSize;
	bool override, ret;

	if (getValueForBootKey(bootArgs->CommandLine, key, val, size))
	{
		return true;
	}

	ret = getValueForConfigTableKey(config, key, val, size);

	// Try to find alternate keys in bootInfo->chameleonConfig (if config can be overriden)
	// and prefer its values with the exceptions for
	// "Kernel"="mach_kernel" and "Kernel Flags"="".

	if (config->canOverride)
	{
		if (getValueForConfigTableKey(&bootInfo->chameleonConfig, key, &overrideVal, &overrideSize))
		{
			override = true;

			// NOTE: Values are defined by apple as being in com.apple.Boot.plist
			//        kHelperRootUUIDKey, kKernelArchKey, kMKextCacheKey, kKernelCacheKey, kKernelNameKey, kKernelFlagsKey
			if (ret && (strcmp(key, kKernelNameKey) == 0) && (overrideSize == 0))
			{
				override = false;
			}

			if (ret && (strcmp(key, kKernelFlagsKey) == 0) && (overrideSize == 0))
			{
				override = false;
			}

			if (override)
			{
				*val = overrideVal;
				*size = overrideSize;
				ret = true;
			}
		}
	}
	return ret;
}

#if UNUSED
void printSystemConfig(char *p1)
{
	char *p2 = p1, tmp;

	while (*p1 != '\0')
	{
		while (*p2 != '\0' && *p2 != '\n')
		{
			p2++;
		}
	tmp = *p2;
	*p2 = '\0';
	printf("%s\n", p1);
	*p2 = tmp;
	if (tmp == '\0')
	{
		break;
	}
	p1 = ++p2;
    }
}
#endif

//==========================================================================
// ParseXMLFile
// Modifies the input buffer.
// Expects to see one dictionary in the XML file.
// Puts the first dictionary it finds in the
// tag pointer and returns 0, or returns -1 if not found
// (and does not modify dict pointer).
// Prints an error message if there is a parsing error.
//
int ParseXMLFile( char *buffer, TagPtr *dict )
{
	long       length, pos;
	TagPtr     tag = 0;
	pos = 0;
	char       *configBuffer;

	length = strlen(buffer) + 1;
  
	configBuffer = malloc(strlen(buffer)+1);
	if (!configBuffer)
	{
		return -1;
	}
	strlcpy(configBuffer, buffer, length );

	while (1)
	{
		length = XMLParseNextTag(configBuffer + pos, &tag);
			if (length == -1)
			{
				break;
			}

		pos += length;

		if (tag == 0)
		{
			continue;
		}
		if (tag->type == kTagTypeDict)
		{
			break;
		}

		XMLFreeTag(tag);
	}
	free(configBuffer);
	if (length < 0) {
		error ("Error parsing plist file\n");
		return -1;
	}
	*dict = tag;
	return 0;
}

/* loadConfigFile
 *
 * Returns 0 - successful.
 *		  -1 - unsuccesful.
 */
int loadConfigFile (const char *configFile, config_file_t *config)
{
	int fd, count;

	if ((fd = open_bvdev("bt(0,0)", configFile, 0)) < 0)
	{
		return -1;
	}
	// read file
	count = read(fd, config->plist, IO_CONFIG_DATA_SIZE);
	close(fd);

	// build xml dictionary
	ParseXMLFile(config->plist, &config->dictionary);
	return 0;
}

/* loadSystemConfig
 *
 * Returns 0 - successful.
 *		  -1 - unsuccesful.
 */
int loadSystemConfig(config_file_t *config)
{
	// Micky1979, the order is important
	char *dirspec[] = {
		"/com.apple.recovery.boot/com.apple.Boot.plist",			// OS X Recovery
		"/macOS Install Data/Locked Files/Boot Files/com.apple.Boot.plist",	// macOS Upgrade (10.12)+
		"/macOS Install Data/com.apple.Boot.plist",				// macOS Upgrade (10.12)
		"/OS X Install Data/com.apple.Boot.plist",				// OS X Upgrade (10.8+)
		"/Mac OS X Install Data/com.apple.Boot.plist",				// OS X Upgrade (Lion 10.7)
		"/.IABootFiles/com.apple.Boot.plist",					// OS X Installer
		"/Library/Preferences/SystemConfiguration/com.apple.Boot.plist"		// (Installed System or Installer)
	};

	int i, fd, count, ret=-1;

	for(i = 0; i< sizeof(dirspec)/sizeof(dirspec[0]); i++)
	{
		if ((fd = open(dirspec[i], 0)) >= 0)
		{
			// read file
			count = read(fd, config->plist, IO_CONFIG_DATA_SIZE);
			close(fd);

			// build xml dictionary
			ParseXMLFile(config->plist, &config->dictionary);
			sysConfigValid = true;	
			ret=0;

			break;
		}
	}

	if(ret == -1) ret = loadHelperConfig(config);

	// Always enable canOverride flag (for SystemConfig)
	config->canOverride = true;

	return ret;
}

/* loadChameleonConfig
 *
 * Returns 0 - successful.
 *		  -1 - unsuccesful.
 */
int loadChameleonConfig(config_file_t *config, BVRef chain)
{
	char *dirspec[] = {
		"/Extra/org.chameleon.Boot.plist",
		"/Extra/com.apple.Boot.plist",           /* DEPRECATED */
	};

	int i;

	for(i = 0; i< sizeof(dirspec)/sizeof(dirspec[0]); i++)
	{
		if ( loadChameleonConfigForDevice(config, "rd(0,0)", dirspec[i]) == 0 )
		{
			return 0;
		}

		if ( loadChameleonConfigForDevice(config, "", dirspec[i]) == 0 )
		{
			return 0;
		}

		if ( loadChameleonConfigForDevice(config, "bt(0,0)", dirspec[i]) == 0 )
		{
			return 0;
		}
		BVRef bvr;
		for ( bvr = chain; bvr; bvr = bvr->next ) /* C99 Error */
		{
			char device[256];
			getDeviceDescription(bvr, device);

			if ( loadChameleonConfigForDevice(config, device, dirspec[i]) == 0 )
			{
				return 0;
			}
		}
	}
	return -1;
}

/* loadChameleonConfigForDevice
 *
 * Returns 0 - successful.
 *		  -1 - unsuccesful.
 */
int loadChameleonConfigForDevice(config_file_t *config, const char *device, const char *path)
{
	char full_path[1024];
	int fd;

	snprintf(full_path, sizeof(full_path), "%s%s", device, path);

	if ((fd = open(full_path, 0)) >= 0)
	{
		// Check for depreciated file names and annoy the user about it.
		if(strstr(full_path, "com.apple.Boot.plist")) {
			printf("%s is deprecated.\n", full_path);
			full_path[strlen(full_path) - strlen("com.apple.Boot.plist")] = 0;
			printf("Please use the file %sorg.chameleon.Boot.plist instead.\n", full_path);
			pause();
		}
		// read file
		read(fd, config->plist, IO_CONFIG_DATA_SIZE);
		close(fd);

		// build xml dictionary
		ParseXMLFile(config->plist, &config->dictionary);
		sysConfigValid = true;
		return 0;
	}
	return -1;
}

/* loadHelperConfig
 *
 * Returns 0 - successful.
 *		  -1 - unsuccesful.
 */
int loadHelperConfig(config_file_t *config)
{
	int rfd, pfd, sfd, count, ret=-1, fixedsize;

	char *dirspec[] = {
		"/com.apple.boot.P/Library/Preferences/SystemConfiguration/com.apple.Boot.plist",
		"/com.apple.boot.R/Library/Preferences/SystemConfiguration/com.apple.Boot.plist",
		"/com.apple.boot.S/Library/Preferences/SystemConfiguration/com.apple.Boot.plist"
	};

	// This is a simple rock - paper scissors algo. R beats S, P beats R, S beats P
	// If all three, S is used for now. This should be changed to something else (say, timestamp?)

	pfd = open(dirspec[0], 0);
	if(pfd >= 0)	// com.apple.boot.P exists
	{

		sfd = open(dirspec[2], 0); // com.apple.boot.S takes precidence if it also exists
		if(sfd >= 0)
		{
			// Use sfd
			fixedsize = MIN(file_size(sfd), IO_CONFIG_DATA_SIZE);
			count = read(sfd, config->plist, fixedsize);
			close(sfd);
			close(pfd);
			if (count != fixedsize) return -1;

			// build xml dictionary
			ParseXMLFile(config->plist, &config->dictionary);
			sysConfigValid = true;
			ret=0;

		}
		else
		{
			// used pfd
			fixedsize = MIN(file_size(pfd), IO_CONFIG_DATA_SIZE);
			count = read(pfd, config->plist, fixedsize);
			close(pfd);
			if (count != fixedsize) return -1;

			// build xml dictionary
			ParseXMLFile(config->plist, &config->dictionary);
			sysConfigValid = true;
			ret = 0;
		}

	}
	else
	{
		rfd = open(dirspec[1], 0); // com.apple.boot.R exists
		if(rfd >= 0)
		{
			pfd = open(dirspec[2], 0); // com.apple.boot.P takes recidence if it exists
			if(pfd >= 0)
			{
				// use sfd
				fixedsize = MIN(file_size(pfd), IO_CONFIG_DATA_SIZE);
				count = read(pfd, config->plist, fixedsize);
				close(pfd);
				close(rfd);
				if (count != fixedsize) return -1;

				// build xml dictionary
				ParseXMLFile(config->plist, &config->dictionary);
				sysConfigValid = true;
				ret = 0;

			}
			else
			{
				// use rfd
				fixedsize = MIN(file_size(rfd), IO_CONFIG_DATA_SIZE);
				count = read(rfd, config->plist, fixedsize);
				close(rfd);
				if (count != fixedsize) return -1;
				
				// build xml dictionary
				ParseXMLFile(config->plist, &config->dictionary);
				sysConfigValid = true;
				ret = 0;

			}

		}
		else
		{
			sfd = open(dirspec[2], 0); // com.apple.boot.S exists, but nothing else does
			if(sfd >= 0)
			{
				// use sfd
				fixedsize = MIN(file_size(sfd), IO_CONFIG_DATA_SIZE);
				count = read(sfd, config->plist, fixedsize);
				close(sfd);
				if (count != fixedsize) return -1;

				// build xml dictionary
				ParseXMLFile(config->plist, &config->dictionary);
				sysConfigValid = true;
				ret = 0;

			}
		}

	}

	return ret;
}

char *newString(const char *oldString)
{
	if ( oldString )
	{
		return strcpy(malloc(strlen(oldString)+1), oldString);
	}
	else
	{
		return NULL;
	}
}

/*
 * Extracts the next argument from the command line, double quotes are allowed here.
 */
char *getNextArg(char **argPtr, char *val)
{
	char * ptr = *argPtr;
	const char * strStart;
	int len = 0;
	bool isQuoted = false;

	*val = '\0';

	// Scan for the next non-whitespace character.
	while ( *ptr && (*ptr == ' ' || *ptr == '=') )
	{
		ptr++;
	}
  
	strStart = ptr;

	// Skip the leading double quote character.
	if (*ptr == '\"')
	{
		isQuoted = true;
		ptr++;
		strStart++;
	}

	// Scan for the argument terminator character.
	// This can be either a NULL character - in case we reach the end of the string,
	// a double quote in case of quoted argument,
	// or a whitespace character (' ' or '=') for non-quoted argument.
	while (*ptr && !( (isQuoted && (*ptr == '\"')) ||
                    (!isQuoted && (*ptr == ' ' || *ptr == '=')) )
        ) {
		ptr++;
	}

	len = ptr - strStart;

	// Skip the closing double quote character and adjust
	// the starting pointer for the next getNextArg call.
	if (*ptr && isQuoted && *ptr == '\"') {
		ptr++;
	}

	// Copy the extracted argument to val.
	strncat(val, strStart, len);

	// Set command line pointer.
	*argPtr = ptr;

	return ptr;
}
