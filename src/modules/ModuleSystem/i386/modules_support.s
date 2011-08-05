#ifdef CONFIG_MODULESYSTEM_MODULE
#include <architecture/i386/asm_help.h>
	
LABEL(dyld_void_start)
LABEL(_ModuleSystem_start)
    ret

#endif