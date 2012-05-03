/*
 * Copyright 2010,2011,2012 valv, cparm <armelcadetpetit@gmail.com>. All rights reserved.
 */
#include "libsaio.h"
#include "bootstruct.h"
#include "modules.h"
#include "Platform.h"
#include "cpu.h"

#define kFixFSB		        "FixFSB"
#define MSR_FSB_FREQ		0x000000cd
#define AMD_10H_11H_CONFIG  0xc0010064
#define kEnableCPUfreq	"EnableCPUfreqModule"
void CPUfreq_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6);

void CPUfreq_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	int bus_ratio;				  
	uint64_t	msr , fsbFrequency , cpuFrequency , minfsb , maxfsb ;
    
    uint8_t         Model = (uint8_t)get_env(envModel);			
    uint32_t		Vendor = (uint32_t)get_env(envVendor);			
    uint8_t         Family = (uint8_t)get_env(envFamily);
    
    uint8_t MaxCoef = (uint8_t)get_env(envMaxCoef);
    uint8_t MaxDiv = (uint8_t)get_env(envMaxDiv);
    uint8_t CurrDiv = (uint8_t)get_env(envCurrDiv);
    uint64_t FSBFrequency = get_env(envFSBFreq);
    uint8_t CurrCoef = (uint8_t)get_env(envCurrCoef);
    uint64_t CPUFrequency = get_env(envCPUFreq);
    
      
	if ((Vendor == CPUID_VENDOR_INTEL ) && ((Family == 0x06) || (Family == 0x0f)))
	{
		if ((Family == 0x06 && Model >= 0x0c) || (Family == 0x0f && Model >= 0x03))
		{
			
			if (Family == 0x06)
			{
				
				bus_ratio = 0; 								  
				msr = rdmsr64(MSR_FSB_FREQ); 
				minfsb = 183000000; 
				maxfsb = 185000000;
				
				bool		fix_fsb = false;				
				uint16_t idlo;
				uint8_t crlo, crhi = 0;				
				
				switch (Model) {
					case CPUID_MODEL_YONAH:		// Core Duo/Solo, Pentium M DC
					case CPUID_MODEL_MEROM:		// Core Xeon, Core 2 DC, 65nm
					case 0x16:					// Celeron, Core 2 SC, 65nm
					case CPUID_MODEL_PENRYN:		// Core 2 Duo/Extreme, Xeon, 45nm
					case CPUID_MODEL_ATOM:		// Atom :)
					case 0x27:					// Atom Lincroft, 45nm                                           
					{
						getBoolForKey(kFixFSB, &fix_fsb, DEFAULT_BOOT_CONFIG);                                                              
						
						if (fix_fsb)
						{
							
							int bus = (msr >> 0) & 0x7;                                                                       
							switch (bus) {
								case 0:
									fsbFrequency = 266666667;
									break;
								case 1:
									fsbFrequency = 133333333;
									break;
								case 2:
									fsbFrequency = 200000000;
									break;
								case 3:
									fsbFrequency = 166666667;
									break;
								case 4:
									fsbFrequency = 333333333;
									break;
								case 5:
									fsbFrequency = 100000000;
									break;
								case 6:
									fsbFrequency = 400000000;
									break;
								default:
									fsbFrequency = 200000000;									
									break;
							}
							
							
							if (((fsbFrequency) > (minfsb) && (fsbFrequency) < (maxfsb)) || (!fsbFrequency)) 
							{
								fsbFrequency = 200000000;
							}
                            safe_set_env(envFSBFreq, fsbFrequency);
						}
						
						if (getIntForKey("MaxBusRatio", &bus_ratio, DEFAULT_BOOT_CONFIG)) 
						{                                                                             
							msr = rdmsr64(MSR_IA32_PERF_STATUS);
							idlo = (msr >> 48) & 0xffff;								
							crlo = (idlo  >> 8) & 0xff;
							
							//printf("CPUfreq: MinCoef:      0x%x\n",crlo);
                            
                            

							if (MaxCoef) 
							{
								if (MaxDiv) 
								{
									crhi = (MaxCoef * 10) + 5;
								}
								else 
								{
									crhi = MaxCoef * 10;
								}
							}
							if (crlo == 0 || crhi == crlo) goto out;								
							
							if ((bus_ratio >= (crlo *10)) && (crhi >= bus_ratio) ) 
								//if (bus_ratio >= 60) 
							{								
								uint8_t currdiv = 0, currcoef = 0;
								
								currcoef = (int)(bus_ratio / 10);
								
								uint8_t fdiv = bus_ratio - (currcoef * 10);
								if (fdiv > 0)
									currdiv = 1;
								
                                safe_set_env(envCurrCoef,currcoef);
                                safe_set_env(envCurrDiv,currdiv);
									
							}
							
							
						}								
						
					out:
						if (CurrDiv) 
						{
							cpuFrequency = (FSBFrequency * ((CurrCoef * 2) + 1) / 2);
						}
						else 
						{
							cpuFrequency = (FSBFrequency * CurrCoef);
						}							
						
                        safe_set_env(envCPUFreq,cpuFrequency);

						verbose("CPU: FSBFreq changed to:               %dMHz\n", (uint32_t)(get_env(envFSBFreq) / 1000000));
						verbose("CPU: CPUFreq changed to:               %dMHz\n", (uint32_t)(get_env(envCPUFreq) / 1000000));
						
						break;
					}						
					case 0x1d:		// Xeon MP MP 7400
					default:						
						break;
				}
			}
		}
	}
	else if(Vendor == CPUID_VENDOR_AMD && Family == 0x0f) // valv: work in progress
	{
		
        uint8_t         ExtFamily = get_env(envExtFamily);
        uint64_t		TSCFrequency = get_env(envTSCFreq);
        
		uint8_t bus_ratio_current = 0;
		
		if(ExtFamily == 0x00 /* K8 */)
		{
			
			msr = rdmsr64(K8_FIDVID_STATUS);
			bus_ratio_current = (msr & 0x3f) / 2 + 4; 
			CurrDiv = (msr & 0x01) * 2;					           
			if (bus_ratio_current)
			{
				if (CurrDiv)
				{
                    safe_set_env(envFSBFreq,((TSCFrequency * CurrDiv)/bus_ratio_current)); // ?
				}
				else
				{
                    safe_set_env(envFSBFreq, (TSCFrequency/bus_ratio_current) );					
				}
				//fsbFrequency = (tscFrequency / bus_ratio_max); // ?
			}
		}
		else if(ExtFamily >= 0x01 /* K10+ */)
		{
			
			msr = rdmsr64(AMD_10H_11H_CONFIG);
			bus_ratio_current = ((msr) & 0x3F);
			
            safe_set_env(envCurrDiv,((2 << ((msr >> 6) & 0x07)) / 2));

            safe_set_env(envFSBFreq,(CPUFrequency/bus_ratio_current));
			
			
		}	
		
		if (!get_env(envFSBFreq)) 
		{
            safe_set_env(envFSBFreq,(DEFAULT_FSB * 1000));
			verbose("0 ! using the default value for FSB !\n");
		}
        
        safe_set_env(envCurrCoef, (bus_ratio_current / 10));

		safe_set_env(envCPUFreq,TSCFrequency);
		
		verbose("CPU (AMD): FSBFreq:               %dMHz\n", (uint32_t)(get_env(envFSBFreq) / 1000000));
		verbose("CPU (AMD): CPUFreq:               %dMHz\n", (uint32_t)(get_env(envCPUFreq) / 1000000));
		verbose("CPU (AMD): CurrCoef:			   0x%x\n",  (uint32_t)(get_env(envCurrCoef)));
		verbose("CPU (AMD): CurrDiv:               0x%x\n",  (uint32_t)(get_env(envCurrDiv)));	
	}	    

}

void CPUfreq_start(void);
void CPUfreq_start(void)
{	
	if (get_env(envFeatures) & CPUID_FEATURE_MSR)
    {
        register_hook_callback("PreBoot", &CPUfreq_hook);		
    } 
    else
    {
        verbose ("Unsupported CPU: CPUfreq disabled !!!\n");		
    }	
}
