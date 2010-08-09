/*
 * Module Loading functionality
 * Copyright 2009 Evan Lojewski. All rights reserved.
 *
 */

#include <mach-o/loader.h>
#include <mach-o/nlist.h>


#ifndef __BOOT_MODULES_H
#define __BOOT_MODULES_H

#define SYMBOLS_MODULE "Symbols"
#define SYMBOL_DYLD_STUB_BINDER	"dyld_stub_binder"
#define STUB_ENTRY_SIZE	6

#define SUCCESS	1
#define	ERROR	0



int jump_pointer(int (*pointer)());

int load_module(const char* module);
void module_loaded(const char* name, UInt32 version, UInt32 compat);
void add_symbol(const char* symbol, void*  addr);
void* parse_mach(void* binary);

#endif /* __BOOT_MODULES_H */