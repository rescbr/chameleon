/*
 * fputc.c
 *
 * gcc "printf decompilation" expects this to exist...
 */

#include <stdio.h>
extern size_t _fwrite(const void *buf, size_t count, FILE *f);
int fputc(int c, FILE *f)
{
	unsigned char ch = c;

	return _fwrite(&ch, 1, f) == 1 ? ch : EOF;
}
