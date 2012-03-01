/*
 *  HibernateEnabler.c
 *  Chameleon
 *
 *  Created by cparm.
 */

#include "bootstruct.h"
#include "modules.h"
#include "resume.h"
#include "sl.h"
#include "graphic_utils.h"

#define kWake				"Wake"				/* boot.c */
#define kForceWake			"ForceWake"			/* boot.c */
#define kWakeImage			"WakeImage"			/* boot.c */
#define kEnableHibernate	"EnableHibernateModule"
void HibernateEnabler_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);
void HibernateEnabler_start(void);

void HibernateEnabler_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
	bool tryresume,tryresumedefault, forceresume;
	
	if (!getBoolForKey (kWake, &tryresume, DEFAULT_BOOT_CONFIG))
	{
		tryresume = true;
		tryresumedefault = true;
	}
	else
	{
		tryresumedefault = false;
	}
	
	if (!getBoolForKey (kForceWake, &forceresume, DEFAULT_BOOT_CONFIG))
	{
		forceresume = false;
	}
	
	if (forceresume)
	{
		tryresume = true;
		tryresumedefault = false;
	}
	
	while (tryresume) {
		const char *tmp, *val;
		int len, ret = -1;
		long flags, sleeptime;
		BVRef bvr;
		if (!getValueForKey(kWakeImage, &val, &len, DEFAULT_BOOT_CONFIG))
			val="/private/var/vm/sleepimage";
		
		// Do this first to be sure that root volume is mounted
		ret = GetFileInfo(0, val, &flags, &sleeptime);
		
		if ((bvr = getBootVolumeRef(val, &tmp)) == NULL)
			break;
		
		// Can't check if it was hibernation Wake=y is required
		if (bvr->modTime == 0 && tryresumedefault)
			break;
		
		if ((ret != 0) || ((flags & kFileTypeMask) != kFileTypeFlat))
			break;
		
		if (!forceresume && ((sleeptime+3)<bvr->modTime))
		{
			printf ("Hibernate image is too old by %d seconds. Use ForceWake=y to override\n",bvr->modTime-sleeptime);
			break;
		}
		
		HibernateBoot((char *)val);
		break;
	}
	
}

void HibernateEnabler_start(void)
{
	bool enable = true;
	getBoolForKey(kEnableHibernate, &enable, DEFAULT_BOOT_CONFIG) ;
	
	if (enable)
	{
		register_hook_callback("PreBoot", &HibernateEnabler_hook);
		register_hook_callback("spinActivity_hook", &spinActivityIndicator_hook);
	}
}