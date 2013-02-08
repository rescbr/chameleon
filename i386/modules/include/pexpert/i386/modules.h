/*
 * Module Loading functionality
 * Copyright 2009 Evan Lojewski. All rights reserved.
 *
 */

/*
 * Copyright 2012 Cadet-petit Armel <armelcadetpetit@gmail.com>. All rights reserved.
 *
 * Cleaned, Added bundles support.
 *
 */

// There is a bug with the module system / rebasing / binding
// that causes static variables to be incorrectly rebased or bound
// Disable static variables for the moment
// #define static

#ifndef _PEXPERT_MODULES_H
#define _PEXPERT_MODULES_H

#include <efi.h>

#define DEFAULT_BUNDLE_SPEC_SIZE 4096
#define macho_64 0

extern unsigned long long textAddress;
extern unsigned long long textSection;

typedef struct symbolList_t
{
    char* module;
	char* symbol;
	unsigned int addr;
	struct symbolList_t* next;
} symbolList_t;

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

#define SYMBOLS_BUNDLE "Symbols"

#define SYMBOL_DYLD_STUB_BINDER	"dyld_stub_binder"
#define SYMBOL_LOOKUP_SYMBOL	"_lookup_symbol"
#define STUB_ENTRY_SIZE	6

#define SECT_NON_LAZY_SYMBOL_PTR	"__nl_symbol_ptr"
#define SECT_SYMBOL_STUBS			"__symbol_stub"

#define SYS_CLASS  "SYMS"
#define SYSLIB_CLASS "SYS_LIB"

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
VOID register_one_callback(const char* name, void(*callback)(void*, void*, void*, void*, void*, void*));

void rebase_location(unsigned long* location, char* base, int type);
void bind_location(unsigned long* location, char* value, unsigned long addend, int type);
void rebase_macho(void* base, char* rebase_stream, unsigned long size);
EFI_STATUS bind_macho(char* module, void* base, char* bind_stream, unsigned long size);

#if macho_64
long long add_symbol(char* module,char* symbol, long long addr, char is64);
#else
long long add_symbol(char* module,char* symbol, long long addr);
#endif

unsigned int lookup_all_symbols(const char* module, const char* name);

EFI_STATUS replace_function(const char* module, const char* symbol, void* newAddress);
EFI_STATUS replace_system_function(const char* symbol, void* newAddress);
EFI_STATUS replace_function_any(const char* symbol, void* newAddress);

extern unsigned int (*lookup_symbol)(const char*, int(*strcmp_callback)(const char*, const char*));


long InitBundleSupport(void);
long FileLoadBundles(char *dirSpec, long plugin);
long LoadBundlePList(char *dirSpec, char *name, long bundleType);
long LoadMatchedBundles(void);
long MatchBundlesLibraries(void);
long LoadBundles( char * dirSpec );
void * GetBundleDict( char * bundle_id );
void * GetBundlePersonality( char * bundle_id );
char * GetBundlePath( char * bundle_id );

#endif /* _PEXPERT_MODULES_H */