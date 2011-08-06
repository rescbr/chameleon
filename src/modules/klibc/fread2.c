/*
 * fread2.c
 *
 * The actual fread() function as a non-inline
 */

#define __NO_FREAD_FWRITE_INLINES
#include <stdio.h>
extern size_t _fread(void *buf, size_t count, FILE *f);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE * f)
{
	return _fread(ptr, size * nmemb, f) / size;
}
