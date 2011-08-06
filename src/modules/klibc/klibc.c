/*
 * klibc.c
 *
 * glue + initialization
 */

#include <stdio.h>

int _DefaultRuneLocale;	// todo: fixme

void klibc_start()
{
}

void _exit(int status)
{
    printf("exit() called\n");
    while(1);
}

char __toupper(char c)
{
	return ((c) & ~32);
}

void __divide_error()
{
	printf("Divide by 0\n");
    while(1);
}

// hack
int
__maskrune(int _c, unsigned long _f)
{
	return 0;
	//return ((_c < 0 || _c >= _CACHED_RUNES) ? ___runetype(_c) :
	//		_CurrentRuneLocale->__runetype[_c]) & _f;
}
