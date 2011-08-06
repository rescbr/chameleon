/*
 * strstr.c
 */

#include <string.h>
void *memmem(const void *haystack, size_t n, const void *needle, size_t m);
char *strstr(const char *haystack, const char *needle)
{
	return (char *)memmem(haystack, strlen(haystack), needle,
			      strlen(needle));
}
