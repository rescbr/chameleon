/*
 *  platform_env.c
 * 
 * Copyright 2012 Cadet-petit Armel <armelcadetpetit@gmail.com>. All rights reserved.
 */

#include "libsaio.h"
#include "bootstruct.h"
#include "pci.h"
#include "platform.h"
#include "cpu.h"

#ifndef DEBUG_PLATFORM
#define DEBUG_PLATFORM 0
#endif

#if DEBUG_PLATFORM
#define DBG(x...)	printf(x)
#else
#define DBG(x...)
#endif

static char *gboardproduct = NULL;
static char *gPlatformName = NULL;
static char *gRootDevice = NULL;

void SetgRootDevice(const char * str)
{
    gRootDevice = (char*)str;
}
void Setgboardproduct(const char * str)
{
    gboardproduct = (char*)str;
}
void SetgPlatformName(const char * str)
{
    gPlatformName = (char*)str;
}

char * GetgPlatformName(void)
{
    return gPlatformName ;
}
char * Getgboardproduct(void)
{
    return gboardproduct;
}
char * GetgRootDevice(void)
{
    return gRootDevice;
}

#ifdef rootpath
static char gRootPath[ROOT_PATH_LEN];
void SetgRootPath(const char * str)
{
    bzero(gRootPath,sizeof(gRootPath));
    memcpy(gRootPath,str, sizeof(gRootPath));
}
char * GetgRootPath(void)
{
    return gRootPath ;
}
#endif

typedef enum envtype {
    kEnvPtr = 0,
    kEnvValue = 1
} envtype;

struct env_struct {
    unsigned long long value;                    
    char name[10]; 
    void * ptr;
    //int lock;
    enum envtype Type;
    UT_hash_handle hh;         /* makes this structure hashable */
};

static void CopyVarPtr (struct env_struct *var, void* ptr, size_t size);
static struct env_struct *find_env(const char *name);
static void _re_set_env_copy(struct env_struct *var , void* ptr,size_t size);
struct env_struct *platform_env = NULL;


static void CopyVarPtr (struct env_struct *var, void* ptr, size_t size)
{
    var->ptr = malloc(size);
    memcpy(var->ptr, ptr, size);
}

static struct env_struct *find_env(const char *name) {
    struct env_struct *var;
    
	if (setjmp(h_buf_error) == -1) {
#if DEBUG_PLATFORM
		printf("find_env: Unable to find environement variable\n"); 
        getc();
#endif
		return NULL;
	} else {
		HASH_FIND_STR( platform_env, name, var );  
	}
    return var;
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
		printf("re_set_env_copy: Unable to find environement variable %s\n",name);
#if DEBUG_PLATFORM
        getc();
#endif
		return ;
	} 
    
    _re_set_env_copy(var , ptr, size);
	
	return;    
}

static void _set_env(const char *name, unsigned long long value,  enum envtype Type, void* ptr, size_t size ) {
    struct env_struct *var;
    
    var = (struct env_struct*)malloc(sizeof(struct env_struct));
    if (Type == kEnvPtr) {
        CopyVarPtr( var,  ptr, size);
    } 
    else if (Type == kEnvValue) 
        var->value = value;
    else
        return;
    
    var->Type = Type;
    
    strlcpy(var->name, name, sizeof(var->name));
	
	if (setjmp(h_buf_error) == -1) {
		printf("_set_env: Unable to set environement variable"); // don't try to acces to the string 'name', 
		//'cause we just returned from the longjump, stack as already changed state.
#if DEBUG_PLATFORM
		getc();
#endif
		return;
	} else {
		HASH_ADD_STR( platform_env, name, var );
	}
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
void set_env(const char *name, unsigned long long value ) {
    _set_env(name, value, kEnvValue,0,0);
}

void set_env_copy(const char *name, void * ptr, size_t size ) {
    _set_env(name, 0, kEnvPtr,ptr,size);
}

unsigned long long get_env_var(const char *name) {
	struct env_struct *var;
    
	var = find_env(name);
	if (!var) {
#if DEBUG_PLATFORM
		printf("get_env_var: Unable to find environement variable %s\n",name);
        getc();
#endif
		return 0;
	}
    
    if (var->Type != kEnvValue) {
        printf("get_env_var: Variable %s is not a value\n",name);
#if DEBUG_PLATFORM
        getc();
#endif
        return 0;
    }
	
	return var->value;
    
}

unsigned long long get_env(const char *name) {	
	
	return get_env_var(name);;
    
}

void * get_env_ptr(const char *name) {
	struct env_struct *var;
    
	var = find_env(name);
	if (!var) {
#if DEBUG_PLATFORM
		printf("get_env_ptr: Unable to get environement ptr variable %s\n",name);
        getc();
#endif
		return 0;
	}
    
    if (var->Type != kEnvPtr) {
        printf("get_env_ptr: Variable %s is not a ptr\n",name);
#if DEBUG_PLATFORM
        getc();
#endif
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

void safe_set_env(const char *name , unsigned long long value) {
    
    _safe_set_env(name, value, kEnvValue,0,0);
	
	return;    
}

void re_set_env(const char *name , unsigned long long value) {
	struct env_struct *var;
    
	var = find_env(name);
	if (!var || (var->Type == kEnvValue)) {
		printf("re_set_env: Unable to reset environement value variable %s\n",name);
#if DEBUG_PLATFORM
        getc();
#endif
		return ;
	} 
    
    var->value = value;
	
	return;    
}

static void delete_env(struct env_struct *var) {
	
	if (setjmp(h_buf_error) == -1) {
		printf("delete_env: Unable to delete environement variable\n");
#if DEBUG_PLATFORM
        getc();	
#endif
		return;
	} else {
		HASH_DEL( platform_env, var);  
	}
    free(var);
}

void unset_env(const char *name) {
    struct env_struct *var;
    
    if ((var = find_env(name))) {
        delete_env(var);
    }      
}

void free_platform_env(void) {
    struct env_struct *current_var, *tmp; 
    
	if (setjmp(h_buf_error) == -1) {
		printf("free_platform_env: Unable to delete all environement variables\n"); 
#if DEBUG_PLATFORM
		getc();
#endif
		return;
	} else {
		HASH_ITER(hh, platform_env, current_var, tmp) {    
			HASH_DEL(platform_env,current_var);
			free(current_var);           
		}
	}     
}

#if DEBUG_PLATFORM
void debug_platform_env(void)
{
    struct env_struct *current_var = platform_env;
    for(current_var=platform_env;current_var;current_var=(struct env_struct*)(current_var->hh.next)) 
    {
        if (current_var->Type == kEnvValue)
            printf(" Name = %s | Type = VALUE | Value = 0x%04x\n",current_var->name,(uint32_t)current_var->value);
        else if (current_var->Type == kEnvPtr )
            printf(" Name = %s | Type = PTR(Copy) | Value = 0x%x\n",current_var->name,(uint32_t)current_var->ptr);

    }
    getc();
}
#endif

/** 
    Scan platform hardware information, called by the main entry point (common_boot() ) 
    _before_ bootConfig xml parsing settings are loaded
*/
void scan_platform(void)
{	
	//memset(&Platform, 0, sizeof(PlatformInfo_t));
	build_pci_dt();
	scan_cpu();
    
#if DEBUG_PLATFORM
    DBG("CPU: %s\n", (char*)get_env_ptr(envBrandString));
    if (get_env(envVendor) == CPUID_VENDOR_AMD)
	DBG("CPU: Vendor/Model/ExtModel: 0x%x/0x%x/0x%x\n", (uint32_t)get_env(envVendor), (uint32_t)get_env(envModel), (uint32_t)get_env(envExtModel));
	DBG("CPU: Family/ExtFamily:      0x%x/0x%x\n", (uint32_t)get_env(envFamily), (uint32_t)get_env(envExtFamily));
    if (get_env(envVendor) == CPUID_VENDOR_AMD) {
        DBG("CPU (AMD): TSCFreq:               %dMHz\n", (uint32_t)(get_env(envTSCFreq) / 1000000));
        DBG("CPU (AMD): FSBFreq:               %dMHz\n", (uint32_t)(get_env(envFSBFreq) / 1000000));
        DBG("CPU (AMD): CPUFreq:               %dMHz\n", (uint32_t)(get_env(envCPUFreq) / 1000000));
        DBG("CPU (AMD): MaxCoef/CurrCoef:      0x%x/0x%x\n", (uint32_t)get_env(envMaxCoef), (uint32_t)get_env(envCurrCoef));
        DBG("CPU (AMD): MaxDiv/CurrDiv:        0x%x/0x%x\n", (uint32_t)get_env(envMaxDiv), (uint32_t)get_env(envCurrDiv));
    } else if (get_env(envVendor) == CPUID_VENDOR_INTEL){
        DBG("CPU: TSCFreq:               %dMHz\n", (uint32_t)(get_env(envTSCFreq) / 1000000));
        DBG("CPU: FSBFreq:               %dMHz\n", (uint32_t)(get_env(envFSBFreq) / 1000000));
        DBG("CPU: CPUFreq:               %dMHz\n", (uint32_t)(get_env(envCPUFreq) / 1000000));
        DBG("CPU: MaxCoef/CurrCoef:      0x%x/0x%x\n", (uint32_t)(get_env(envMaxCoef)), (uint32_t)(get_env(envCurrCoef)));
        DBG("CPU: MaxDiv/CurrDiv:        0x%x/0x%x\n", (uint32_t)(get_env(envMaxDiv)), (uint32_t)(get_env(envCurrDiv)));
    }			
	
	DBG("CPU: NoCores/NoThreads:     %d/%d\n", (uint32_t)(get_env(envNoCores)), (uint32_t)(get_env(envNoThreads)));
	DBG("CPU: Features:              0x%08x\n", (uint32_t)(get_env(envFeatures)));
    DBG("CPU: ExtFeatures:           0x%08x\n", (uint32_t)(get_env(envExtFeatures)));
#ifndef AMD_SUPPORT
    DBG("CPU: MicrocodeVersion:      %d\n", (uint32_t)(get_env(envMicrocodeVersion)));
#endif
    pause();
#endif
}

#ifdef ShowCurrentDate

// shamefully ripped to http://wiki.osdev.org/CMOS

/*
 * http://wiki.osdev.org/OSDev_Wiki:General_disclaimer
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#define CURRENT_YEAR        2012                            // Change this each year!

static int century_register = 0x00;                                // Set by ACPI table parsing code if possible(... in FADT table)

enum {
    cmos_address = 0x70,
    cmos_data    = 0x71
};


static int get_update_in_progress_flag() {
    outb(cmos_address, 0x0A);
    return (inb(cmos_data) & 0x80);
}


static unsigned char get_RTC_register(int reg) {
    outb(cmos_address, reg);
    return inb(cmos_data);
}

static void read_rtc(EFI_TIME *time) {
    unsigned char century;
    unsigned char last_second;
    unsigned char last_minute;
    unsigned char last_hour;
    unsigned char last_day;
    unsigned char last_month;
    unsigned char last_year;
    unsigned char last_century;
    unsigned char registerB;
    
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned int year;
    // Note: This uses the "read registers until you get the same values twice in a row" technique
    //       to avoid getting dodgy/inconsistent values due to RTC updates
    
    while (get_update_in_progress_flag());                // Make sure an update isn't in progress
    second = get_RTC_register(0x00);
    minute = get_RTC_register(0x02);
    hour = get_RTC_register(0x04);
    day = get_RTC_register(0x07);
    month = get_RTC_register(0x08);
    year = get_RTC_register(0x09);
    if(century_register != 0) {
        century = get_RTC_register(century_register);
    }
    
    do {
        last_second = second;
        last_minute = minute;
        last_hour = hour;
        last_day = day;
        last_month = month;
        last_year = year;
        last_century = century;
        
        while (get_update_in_progress_flag());           // Make sure an update isn't in progress
        second = get_RTC_register(0x00);
        minute = get_RTC_register(0x02);
        hour = get_RTC_register(0x04);
        day = get_RTC_register(0x07);
        month = get_RTC_register(0x08);
        year = get_RTC_register(0x09);
        if(century_register != 0) {
            century = get_RTC_register(century_register);
        }
    } while( (last_second == second) && (last_minute == minute) && (last_hour == hour) &&
            (last_day == day) && (last_month == month) && (last_year == year) &&
            (last_century == century) );
    
    registerB = get_RTC_register(0x0B);
    
    // Convert BCD to binary values if necessary
    
    if (!(registerB & 0x04)) {
        second = (second & 0x0F) + ((second / 16) * 10);
        minute = (minute & 0x0F) + ((minute / 16) * 10);
        hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
        day = (day & 0x0F) + ((day / 16) * 10);
        month = (month & 0x0F) + ((month / 16) * 10);
        year = (year & 0x0F) + ((year / 16) * 10);
        if(century_register != 0) {
            century = (century & 0x0F) + ((century / 16) * 10);
        }
    }    
        
    // Calculate the full (4-digit) year
    
    if(century_register != 0) {
        year += century * 100;
    } else {
        //year += (CURRENT_YEAR / 100) * 100;
        //if(year < CURRENT_YEAR) year += 100;        

        if ((year += 1900) < 1970)
            year += 100;
    }
    
    time->Second = second;
    time->Minute = minute;
    time->Hour = hour;
    time->Day = day;
    time->Month = month;
    time->Year = year;
}

void rtc_time(EFI_TIME *time) {
    
    read_rtc(time);    
    
    return ;
}

char * Date(void)
{
    EFI_TIME rtctime;            
    rtc_time(&rtctime);
    
    return newStringWithFormat("%02d/%02d/%04d %02d:%02d:%02d",rtctime.Month,rtctime.Day,rtctime.Year,
                               rtctime.Hour,rtctime.Minute,rtctime.Second));
}
#endif