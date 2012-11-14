/*
 *  CoreHash usage example
 *
 */

#include "libsaio.h"
#include "bootstruct.h"
#include "pci.h"
#include "platform.h"
#include "cpu.h"
#include "CoreHash.h"


#ifndef DEBUG_PLATFORM
#define DEBUG_PLATFORM 0
#endif

#if DEBUG_PLATFORM
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

char *gboardproduct = NULL;
char *gPlatformName = NULL;
char *gRootDevice = NULL;

typedef enum envtype {
    kEnvPtr = 0,
    kEnvValue = 1
} envtype;

struct env_struct {
    CoreHashHeader 
	unsigned long value;
	void * ptr;
    enum envtype Type;
};

// CoreHash Declarations
CHInitStr(env_struct)
CHUnInit(env_struct)

static void CopyVarPtr (struct env_struct *var, void* ptr, size_t size);
static struct env_struct *find_env(const char *name);
static void _re_set_env_copy(struct env_struct *var , void* ptr,size_t size);
struct env_struct *platform_env = NULL;

static void CopyVarPtr (struct env_struct *var, void* ptr, size_t size)
{
    var->ptr = malloc(size);
	if (var->ptr)
	{
		memcpy(var->ptr, ptr, size);

	}
}

static struct env_struct *find_env(const char *name) {
    
    return env_struct_FindStrVar(name,platform_env);
}

static void _re_set_env_copy(struct env_struct *var , void* ptr,size_t size) {
    
	if (var->Type == kEnvPtr) {
		return ;
	} 
    
    if (var->ptr) {
        free(var->ptr);
        var->ptr = NULL;
    }
    
    CopyVarPtr(var, ptr,  size);
	
	return;    
}

void re_set_env_copy(const char *name , void* ptr,size_t size) {
	struct env_struct *var;
    
	var = find_env(name);
	if (!var|| (var->Type == kEnvPtr)) {
		printf("Unable to find environement variable %s\n",name);
		return ;
	} 
    
    _re_set_env_copy(var , ptr, size);
	
	return;    
}

static void _set_env(const char *name, unsigned long value,  enum envtype Type, void* ptr, size_t size ) {
    struct env_struct *var;
    
    var = env_struct_NewStrVar(name, &platform_env);
	if (!var) {
		return;
	}
    if (Type == kEnvPtr) {
        CopyVarPtr( var,  ptr, size);
    } 
    else if (Type == kEnvValue) 
        var->value = value;
    else
        return;
    
    var->Type = Type;   
    
}

/* Warning: set_env will create a new variable each time it will be called, 
 * if you want to set again an existing variable, please use safe_set_env or re_set_env .
 * NOTE: If you set several times the "same variable" by using this function,
 * the HASH_COUNT will grow up, 
 * but hopefully find_env will return the last variable that you have set with the same name
 * ex: set_env("test",10);
 *     set_env("test",20);
 *
 *    HASH_COUNT will be equal to 2, get_env("test") will return 20
 *
 *    safe_set_env("test",10);
 *    safe_set_env("test",20);
 * 
 *    HASH_COUNT will be equal to 1, get_env("test") will return 20
 *
 *    set_env("test",10);
 *    re_set_env("test",20);
 *
 *    HASH_COUNT will be equal to 1, get_env("test") will return 20
 *
 */
void set_env(const char *name, unsigned long value ) {
    _set_env(name, value, kEnvValue,0,0);
}

void set_env_copy(const char *name, void * ptr, size_t size ) {
    _set_env(name, 0, kEnvPtr,ptr,size);
}

unsigned long get_env_var(const char *name) {
	struct env_struct *var;
    
	var = find_env(name);
	if (!var) {
		printf("Unable to find environement variable %s\n",name);
		return 0;
	}
    
    if (var->Type != kEnvValue) {
        printf("Variable %s is not a value\n",name);
        return 0;
    }
	
	return var->value;
    
}

unsigned long get_env(const char *name) {	
	
	return get_env_var(name);
    
}

void * get_env_ptr(const char *name) {
	struct env_struct *var;
    
	var = find_env(name);
	if (!var) {
		printf("Unable to get environement ptr variable %s\n",name);
		return 0;
	}
    
    if (var->Type != kEnvPtr) {
        printf("Variable %s is not a ptr\n",name);
        return 0;
    }
	
	return var->ptr;
    
}

/* If no specified variable exist, safe_set_env will create one, else it only modify the value */
static void _safe_set_env(const char *name, unsigned long long value,  enum envtype Type, void* ptr, size_t size )
{
	struct env_struct *var;
    
	var = find_env(name);
    
    if (!var) {
        if (Type == kEnvPtr) {
            _set_env(name, 0, kEnvPtr,ptr,size);
        } 
        else if (Type == kEnvValue) {
            _set_env(name, value, kEnvValue,0,0);
        }
    } 
    else if (var->Type != Type) {
        return;        
	}     
    else {
        if (Type == kEnvValue) 
            var->value = value;
        else if (Type == kEnvPtr)
            _re_set_env_copy(var,ptr,size);            
    }
	
	return;    
}

void safe_set_env_copy(const char *name , void * ptr, size_t size ) {
    
    _safe_set_env(name, 0, kEnvPtr,ptr,size);
	
	return;    
}

void safe_set_env(const char *name , unsigned long value) {
    
    _safe_set_env(name, value, kEnvValue,0,0);
	
	return;    
}

void re_set_env(const char *name , unsigned long value) {
	struct env_struct *var;
    
	var = find_env(name);
	if (!var || (var->Type != kEnvValue)/* kEnvPtr currently unsupported */) {
		printf("Unable to reset environement value variable %s\n",name);
		return ;
	} 
    
    var->value = value;
	
	return;    
}

void unset_env(const char *name) {
	env_struct_DeleteStrVar(name, platform_env);     
}

void free_platform_env(void) {
    env_struct_DeleteAll(platform_env );     
}