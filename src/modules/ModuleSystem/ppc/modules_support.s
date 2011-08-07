#ifdef CONFIG_MODULESYSTEM_MODULE
#include <architecture/ppc/asm_help.h>

;LABEL(dyld_stub_binder)
;	jmp		_dyld_stub_binder

LABEL(dyld_void_start)
LABEL(_ModuleSystem_start)
	blr

#endif