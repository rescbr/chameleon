/*
 * Module Loading functionality
 * Copyright 2009 Evan Lojewski. All rights reserved.
 *
 */

// There is a bug with the module system / rebasing / binding
// that causes static variables to be incorrectly rebased or bound
// Disable static variables for the moment
// #define static

#ifndef __BOOT_MODULES_H
#define __BOOT_MODULES_H

#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include "efi.h"

extern unsigned long long textAddress;
extern unsigned long long textSection;

typedef struct symbolList_t
{
	char* symbol;
	unsigned int addr;
	struct symbolList_t* next;
} symbolList_t;

typedef struct moduleList_t
{
	char* module;
	//struct moduleHook_t* hook_list;
	struct moduleList_t* next;
} moduleList_t;

typedef struct callbackList_t
{
	void(*callback)(void*, void*, void*, void*, void*, void*);
	struct callbackList_t* next;
} callbackList_t;

typedef struct moduleHook_t
{
	const char* name;
	callbackList_t* callbacks;
	struct moduleHook_t* next;
} moduleHook_t;

#define SYMBOLS_MODULE "Symbols.dylib"

#define SYMBOL_DYLD_STUB_BINDER	"dyld_stub_binder"
#define SYMBOL_LOOKUP_SYMBOL	"_lookup_symbol"
#define STUB_ENTRY_SIZE	6

#define SECT_NON_LAZY_SYMBOL_PTR	"__nl_symbol_ptr"
#define SECT_SYMBOL_STUBS			"__symbol_stub"

EFI_STATUS init_module_system();
VOID load_all_modules();

/*
 * Modules Interface
 * execute_hook
 *		Exexutes a registered hook. All callbaks are
 *		called in the same order that they were added
 *
 * register_hook_callback
 *		registers a void function to be executed when a
 *		hook is executed.
 */
EFI_STATUS execute_hook(const char* name, void*, void*, void*, void*, void*, void*);
VOID register_hook_callback(const char* name, void(*callback)(void*, void*, void*, void*, void*, void*));

inline void rebase_location(UInt32* location, char* base, int type);
inline void bind_location(UInt32* location, char* value, UInt32 addend, int type);
void rebase_macho(void* base, char* rebase_stream, UInt32 size);
void bind_macho(void* base, char* bind_stream, UInt32 size);

EFI_STATUS load_module(char* module);
EFI_STATUS is_module_loaded(const char* name);
VOID module_loaded(const char* name/*, UInt32 version, UInt32 compat*/);

long long add_symbol(char* symbol, long long addr, char is64);

VOID* parse_mach(void* binary, 
				 EFI_STATUS(*dylib_loader)(char*),
				 long long(*symbol_handler)(char*, long long, char)
				 );

unsigned int handle_symtable(UInt32 base,
							 struct symtab_command* symtabCommand,
							 long long(*symbol_handler)(char*, long long, char),
							 char is64);
							 
unsigned int lookup_all_symbols(const char* name);

EFI_STATUS replace_function(const char* symbol, void* newAddress);

extern unsigned int (*lookup_symbol)(const char*);

#endif /* __BOOT_MODULES_H */