/*
 * fputs.c
 *
 * This isn't quite fputs() in the stdio sense, since we don't
 * have stdio, but it takes a file descriptor argument instead
 * of the FILE *.
 */

#include <stdio.h>
#include <string.h>

extern size_t _fwrite(const void *buf, size_t count, FILE *f);
int fputs(const char *s, FILE *file)
{
	return _fwrite(s, strlen(s), file);
}
