/*
 * Module Loading functionality
 * Copyright 2009 Evan Lojewski. All rights reserved.
 *
 */

#include <mach-o/loader.h>
#include <mach-o/nlist.h>

#ifndef __BOOT_MODULES_H
#define __BOOT_MODULES_H


typedef struct symbolList_t{
	char* symbol;
	unsigned int addr;
	struct symbolList_t* next;
} symbolList_t;

typedef struct moduleList_t{
	char* module;
	unsigned int version;
	unsigned int compat;
	struct moduleList_t* next;
} moduleList_t;


#define SYMBOLS_MODULE "Symbols"

#define SYMBOL_DYLD_STUB_BINDER	"dyld_stub_binder"
#define STUB_ENTRY_SIZE	6

#define SECT_NON_LAZY_SYMBOL_PTR	"__nl_symbol_ptr"
#define SECT_SYMBOL_STUBS			"__symbol_stub"

#define SUCCESS	1
#define	ERROR	0

int load_module(const char* module);
int is_module_laoded(const char* name);
void module_loaded(char* name, UInt32 version, UInt32 compat);

void add_symbol(char* symbol, void*  addr);
void* parse_mach(void* binary);

unsigned int handle_symtable(UInt32 base, struct symtab_command* symtabCommand, char* symbolStub, char* nonlazy);

unsigned int lookup_all_symbols(const char* name);

extern unsigned int (*lookup_symbol)(const char*);

#endif /* __BOOT_MODULES_H */