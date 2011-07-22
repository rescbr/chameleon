/*
 * printf.c
 */

#include <stdio.h>
#include <stdarg.h>

#define BUFFER_SIZE	16384

int printf(const char *format, ...)
{
	va_list ap;
	int rv;

	va_start(ap, format);
	rv = vfprintf(NULL, format, ap);
	va_end(ap);
	return rv;
}
