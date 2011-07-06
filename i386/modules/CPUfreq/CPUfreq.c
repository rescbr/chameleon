/*
 * Copyright 2010,2011 valv, cparm <armelcadetpetit@gmail.com>. All rights reserved.
 */
#include "boot.h"
#include "bootstruct.h"
#include "libsaio.h"
#include "modules.h"
#include "Platform.h"
#include "cpu.h"

#define kFixFSB		        "FixFSB"
#define MSR_FSB_FREQ		0x000000cd
#define AMD_10H_11H_CONFIG  0xc0010064
#define kEnableCPUfreq	"EnableCPUfreqModule"

void CPUfreq_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{	
	int bus_ratio;				  
	uint64_t	msr , fsbFrequency , cpuFrequency , minfsb , maxfsb ;
	
	if ((Platform->CPU.Vendor == 0x756E6547 /* Intel */) && ((Platform->CPU.Family == 0x06) || (Platform->CPU.Family == 0x0f))) {
		if ((Platform->CPU.Family == 0x06 && Platform->CPU.Model >= 0x0c) || (Platform->CPU.Family == 0x0f && Platform->CPU.Model >= 0x03)) {
			
			if (Platform->CPU.Family == 0x06) {
				
				bus_ratio = 0; 								  
				msr = rdmsr64(MSR_FSB_FREQ); 
				fsbFrequency = 0;
				cpuFrequency = 0; 
				minfsb = 183000000; 
				maxfsb = 185000000;
				
				bool		fix_fsb = false;				
				uint16_t idlo;
				uint8_t crlo, crhi = 0;				
				
				switch (Platform->CPU.Model) {
					case CPUID_MODEL_YONAH:		// Core Duo/Solo, Pentium M DC
					case CPUID_MODEL_MEROM:		// Core Xeon, Core 2 DC, 65nm
					case 0x16:					// Celeron, Core 2 SC, 65nm
					case CPUID_MODEL_PENRYN:		// Core 2 Duo/Extreme, Xeon, 45nm
					case CPUID_MODEL_ATOM:		// Atom :)
					case 0x27:					// Atom Lincroft, 45nm                                           
						
						getBoolForKey(kFixFSB, &fix_fsb, &bootInfo->bootConfig);                                                              
						
						if (fix_fsb) {
							
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
							
							
							if (((fsbFrequency) > (minfsb) && (fsbFrequency) < (maxfsb)) || (!fsbFrequency)) {
								fsbFrequency = 200000000;
							}
							Platform->CPU.FSBFrequency = fsbFrequency;
						}
							
						if (getIntForKey("MaxBusRatio", &bus_ratio, &bootInfo->bootConfig)) 
						{                                                                             
								msr = rdmsr64(MSR_IA32_PERF_STATUS);
								idlo = (msr >> 48) & 0xffff;								
								crlo = (idlo  >> 8) & 0xff;
								
								//printf("CPUfreq: MinCoef:      0x%x\n",crlo);
								
								if (Platform->CPU.MaxCoef) 
								{
									if (Platform->CPU.MaxDiv) 
									{
										crhi = (Platform->CPU.MaxCoef * 10) + 5;
									}
									else 
									{
										crhi = Platform->CPU.MaxCoef * 10;
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
									
									Platform->CPU.CurrCoef = currcoef;
									Platform->CPU.CurrDiv = currdiv;									
								}
								
								
						}								
							
out:
							if (Platform->CPU.CurrDiv) 
							{
								cpuFrequency = (Platform->CPU.FSBFrequency * ((Platform->CPU.CurrCoef * 2) + 1) / 2);
							}
							else 
							{
								cpuFrequency = (Platform->CPU.FSBFrequency * Platform->CPU.CurrCoef);
							}							
							
							Platform->CPU.CPUFrequency = cpuFrequency;
							verbose("CPU: FSBFreq changed to:               %dMHz\n", Platform->CPU.FSBFrequency / 1000000);
							verbose("CPU: CPUFreq changed to:               %dMHz\n", Platform->CPU.CPUFrequency / 1000000);
							//verbose("CPUfreq: FSB Fix applied !\n");							
						 						
						break;
					case 0x1d:		// Xeon MP MP 7400
					default:						
						break;
				}
			}
		}
	}
	else if(Platform->CPU.Vendor == 0x68747541 /* AMD */ && Platform->CPU.Family == 0x0f) // valv: work in progress
		    {
			        verbose("CPU: ");
			        // valv: mobility check			       
			        if ((strstr(Platform->CPU.BrandString, "obile") == 0) || (strstr(Platform->CPU.BrandString, "Atom") == 0))
					{
						          Platform->CPU.isMobile = true;
					}
					
			        verbose("%s\n", Platform->CPU.BrandString);
					uint8_t bus_ratio_current = 0;
				
			        if(Platform->CPU.ExtFamily == 0x00 /* K8 */)
				        {
							
					            msr = rdmsr64(K8_FIDVID_STATUS);
								bus_ratio_current = (msr & 0x3f) / 2 + 4; 
								Platform->CPU.CurrDiv = (msr & 0x01) * 2;					           
					            if (bus_ratio_current)
						            {
							                if (Platform->CPU.CurrDiv)
								                {
									                    Platform->CPU.FSBFrequency = ((Platform->CPU.TSCFrequency * Platform->CPU.CurrDiv) / bus_ratio_current); // ?
									           }
							                else
								                {
									                    Platform->CPU.FSBFrequency = (Platform->CPU.TSCFrequency / bus_ratio_current);
									            }
							                //fsbFrequency = (tscFrequency / bus_ratio_max); // ?
							            }
					        }
			        else if(Platform->CPU.ExtFamily >= 0x01 /* K10+ */)
				        {
								
					            msr = rdmsr64(AMD_10H_11H_CONFIG);
							    bus_ratio_current = ((msr) & 0x3F);
								Platform->CPU.CurrDiv = (2 << ((msr >> 6) & 0x07)) / 2;
					            Platform->CPU.FSBFrequency = (Platform->CPU.CPUFrequency / bus_ratio_current);
								

					    }	
				
					if (!Platform->CPU.FSBFrequency) 
					{
						Platform->CPU.FSBFrequency = (DEFAULT_FSB * 1000);
						verbose("0 ! using the default value for FSB !\n");
					}
					Platform->CPU.CurrCoef = bus_ratio_current / 10;
					Platform->CPU.CPUFrequency = Platform->CPU.TSCFrequency;
					
					verbose("CPU (AMD): FSBFreq:               %dMHz\n", Platform->CPU.FSBFrequency / 1000000);
					verbose("CPU (AMD): CPUFreq:               %dMHz\n", Platform->CPU.CPUFrequency / 1000000);
					verbose("CPU (AMD): CurrCoef:			   0x%x\n",  Platform->CPU.CurrCoef);
					verbose("CPU (AMD): CurrDiv:               0x%x\n",  Platform->CPU.CurrDiv);	
			   }

}

void CPUfreq_start()
{	
	bool enable = true;
	getBoolForKey(kEnableCPUfreq, &enable, &bootInfo->bootConfig) ;
	
	if (enable) {
		if (Platform->CPU.Features & CPUID_FEATURE_MSR) {
			register_hook_callback("PreBoot", &CPUfreq_hook);		
		} else {
			verbose ("Unsupported CPU: CPUfreq disabled !!!\n");		
		}	
	}
}
