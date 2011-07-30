/*
 * fwrite.c
 */

#include <unistd.h>
#include <stdio.h>

size_t _fwrite(const void *buf, size_t count, FILE *f)
{
	size_t bytes = 0;
	ssize_t rv;
	const char *p = buf;

	while (count) {
		rv = 1;
        putchar(*p);

		p += rv;
		bytes += rv;
		count -= rv;
	}

	return bytes;
}
