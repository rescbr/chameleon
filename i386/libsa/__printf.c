#include "libsa.h"

static void (*print)(char *);

void __printf_init(void (*print_fn)(char*))
{
	print = print_fn ? print_fn : NULL;
}

void __printf(const char * format, ...)
{
	char buf[4096];
	va_list ap;
	
	if (print != NULL)
	{
		va_start(ap, format);
		vsnprintf(buf, sizeof(buf), format, ap);
		va_end(ap);
		print(buf);
	}
}