/*
 * strndup.c
 */

#include <stdlib.h>
#include <string.h>

char *strndup(const char *s, size_t n)
{
	int l = n > strlen(s) ? strlen(s) + 1 : n + 1;
	char *d = malloc(l);

	if (d)
		memcpy(d, s, l);
	d[n] = '\0';
	return d;
}
