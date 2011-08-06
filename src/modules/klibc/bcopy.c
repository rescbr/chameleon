/*
 * memcpy.c
 */

#include <string.h>
#include <stdint.h>

void bcopy(const void *src, void *dst, size_t len)
{
	memcpy(dst, src, len);
}