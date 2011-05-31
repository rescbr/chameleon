/*
 *  platform.c
 *
 * AsereBLN: cleanup
 */

#include "libsaio.h"
#include "boot.h"
#include "bootstruct.h"
#include "pci.h"
#include "platform.h"
#include "cpu.h"
#include "spd.h"
#include "dram_controllers.h"

#ifndef DEBUG_PLATFORM
#define DEBUG_PLATFORM 0
#endif

#if DEBUG_PLATFORM
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

PlatformInfo_t    Platform;
pci_dt_t * dram_controller_dev = NULL;

//Azi: temporarily placing this here; from removed mem.c, needed by DEBUG_PLATFORM
// check if replaceable by other or completely remove?? whatever...
#define DC(c) (c >= 0x20 && c < 0x7f ? (char) c : '.')
#define STEP 16
void dumpPhysAddr(const char * title, void * a, int len)
{
    int i,j;
    u_int8_t* ad = (u_int8_t*) a;
    char buffer[80];
    char str[16];

    if(ad==NULL) return;

    printf("%s addr=0x%08x len=%04d\n",title ? title : "Dump of ", a, len);
    printf("Ofs-00-01-02-03-04-05-06-07-08-09-0A-0B-0C-0D-0E-0F       ASCII\n");
    i = (len/STEP)*STEP;
    for (j=0; j < i; j+=STEP)
    {
        printf("%02x: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x  %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
               j, 
               ad[j], ad[j+1], ad[j+2], ad[j+3] , ad[j+4], ad[j+5], ad[j+6], ad[j+7],
               ad[j+8], ad[j+9], ad[j+10], ad[j+11] , ad[j+12], ad[j+13], ad[j+14], ad[j+15],
               DC(ad[j]), DC(ad[j+1]), DC(ad[j+2]), DC(ad[j+3]) , DC(ad[j+4]), DC(ad[j+5]), DC(ad[j+6]), DC(ad[j+7]),
               DC(ad[j+8]), DC(ad[j+9]), DC(ad[j+10]), DC(ad[j+11]) , DC(ad[j+12]), DC(ad[j+13]), DC(ad[j+14]), DC(ad[j+15])
               ); 
    }

    if (len%STEP==0) return;
    sprintf(buffer,"%02x:", i);
    for (j=0; j < STEP; j++)  {
        if (j<(len%STEP))
            sprintf(str, " %02x", ad[i+j]);
        else
            strcpy(str, "   " );  
        strncat(buffer, str, sizeof(buffer));
    }
    strncat(buffer,"  ", sizeof(buffer));
    for (j=0; j < (len%STEP); j++)  {
        sprintf(str, "%c", DC(ad[i+j]));  
        strncat(buffer, str, sizeof(buffer));
    }
    printf("%s\n",buffer);
}

/** Return if a CPU feature specified by feature is activated (true) or not (false)  */
bool platformCPUFeature(uint32_t feature)
{
	if (Platform.CPU.Features & feature) {
		return true;
	} else {
		return false;
	}
}

/** scan mem for memory autodection purpose */
void scan_mem() {
    static bool done = false;
    if (done) return;

    bool useAutodetection = true;
    getBoolForKey(kUseMemDetect, &useAutodetection, &bootInfo->bootConfig);

    if (useAutodetection) {
		if (dram_controller_dev!=NULL) {
			scan_dram_controller(dram_controller_dev); // Rek: pci dev ram controller direct and fully informative scan ...
		}
        scan_spd(&Platform);
    }
    done = true;
}

/** 
    Scan platform hardware information, called by the main entry point (common_boot() ) 
    _before_ bootConfig xml parsing settings are loaded
*/
void scan_platform(void)
{
	memset(&Platform, 0, sizeof(Platform));
	build_pci_dt();
	scan_cpu(&Platform);
	//scan_mem(); Rek: called after pci devs init in fake_efi now ...
}
