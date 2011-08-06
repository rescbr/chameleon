#include <stdio.h>

void uClibcxx_start()
{
}

void abort()
{
	printf("uClibc+: abort()\n");
	while(1);
}


