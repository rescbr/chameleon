/*
 * assert.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void __assert_fail(const char *expr, const char *file, unsigned int line)
{
	printf("Assertion %s failed, file %s, line %u\n", expr, file, line);
	while(1);
}

void __assert_rtn(const char *func, const char *file, int line, const char* expr)
{
	printf("Assertion %s failed, file %s, line %u, function %s\n", expr, file, line, func);
	while(1);
}
