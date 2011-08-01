MODULE_NAME = klibc
MODULE_DESCRIPTION = This module provides a standard c library for modules to link to if the library provided by chameleon is insufficient. This is currently only used by the uClibc++ library.
MODULE_AUTHOR = 
MODULE_VERSION = "1.5.20"
MODULE_COMPAT_VERSION = "1.5.20"
MODULE_START = $(MODULE_NAME)_start
MODULE_DEPENDENCIES = 

DIR = klibc

MODULE_OBJS = 	klibc \
		__ashldi3 __ashrdi3 __clzsi2 __divdi3 __divsi3 \
		__lshrdi3 __moddi3 __modsi3 __udivdi3 \
		__udivmoddi4 __udivmodsi4 __udivsi3 \
		__umoddi3 __umodsi3 \
		strntoumax strntoimax atol atoll \
		strcasecmp strncasecmp strlcat strndup strnlen  \
		strsep strtoimax strtok_r strtok strtol strtoll \
		strtotimespec strtotimeval \
		strtoul strtoull strtoumax strxspn strpbrk \
		bsearch calloc \
		jrand48 lrand48 mrand48 srand48 nrand48 seed48 \
		memccpy memchr memmem memmove memrchr memswap  \
		qsort sha1hash onexit atexit exit \
		snprintf vsnprintf sscanf vsscanf\
		fwrite fprintf vfprintf printf

include ../MakeInc.dir
