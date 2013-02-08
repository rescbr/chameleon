/*
 *
 * Copyright (c) 2012-2013 Cadet-Petit Armel
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Cadet-Petit Armel nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "libegint.h"
#include <xml.h>
#include <IOGraphics.h>
#include <pexpert/i386/modules.h>
#include <sys/time.h>
#include <ctype.h>
#include <string.h>
#include "pngfile.h"

//#include "art.h"


#ifndef DEBUG_TRYME
#define DEBUG_TRYME 0
#endif

#if DEBUG_TRYME
#define DBG(x...)		printf(x)
#else
#define DBG(x...)
#endif

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

#define VIDEO(x) (((boot_args*)getBootArgs())->Video.v_ ## x)

#define vram VIDEO(baseAddr)

#define arc4random_unirange(lo,hi) arc4random_uniform(hi - lo + 1) + lo
#define arc4random_range(lo,hi) (arc4random() % (hi - lo + 1)) + lo

EG_PIXEL StdBackgroundPixel  = { 0xbf, 0xbf, 0xbf, 0 };
EG_PIXEL MenuBackgroundPixel = { 0xbf, 0xbf, 0xbf, 0 };
EG_PIXEL BlackColor = { 0xff, 0xff, 0xff, 0 };

extern long   GetDirEntry(const char *dirSpec, long long *dirIndex, const char **name,
                          long *flags, long *time);
extern long   GetFileInfo(const char *dirSpec, const char *name,
                          long *flags, long *time);

extern int    getc(void);

#define DEFAULT_SCREEN_WIDTH 1024
#define DEFAULT_SCREEN_HEIGHT 768

#define VGA_TEXT_MODE		  0
#define GRAPHICS_MODE         1
#define FB_TEXT_MODE          2

unsigned long screen_params[4] = {DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, 32, 0};	// here we store the used screen resolution
config_file_t    themeConfig;				           // theme.plist

bool LoadEmbedGui(void);

#define CHARACTERS_COUNT	223

/*
 * Font structure.
 */
typedef struct {
	UINTN	Height;			// Font Height
	UINTN	Width;			// Font Width for monospace font only
	EG_IMAGE	*chars[CHARACTERS_COUNT];
	UINTN    count;          // Number of chars in font
} EG_FONT;

typedef union {
    struct {
        UINT8 b;
        UINT8 g;
        UINT8 r;
        UINT8 a;
    } ch;
    UINT8     channel[4];
    UINT32    value;
} EG_LEGACY_PIXEL;

typedef struct {
    UINT16	height;
    UINT16	width;
    EG_LEGACY_PIXEL*	pixels;
} EG_LEGACY_IMAGE;

typedef struct {
    UINT32 x;
    UINT32 y;
} EG_POSITION;

#define PIXEL(p,x,y) ((p)->PixelData[(x) + (y) * (p)->Width])
#define LEGACY_PIXEL(p,x,y,w) (((EG_LEGACY_PIXEL*)(p))[(x) + (y) * (w)])

// File Permissions and Types
enum {
    kPermOtherExecute  = 1 << 0,
    kPermOtherWrite    = 1 << 1,
    kPermOtherRead     = 1 << 2,
    kPermGroupExecute  = 1 << 3,
    kPermGroupWrite    = 1 << 4,
    kPermGroupRead     = 1 << 5,
    kPermOwnerExecute  = 1 << 6,
    kPermOwnerWrite    = 1 << 7,
    kPermOwnerRead     = 1 << 8,
    kPermMask          = 0x1FF,
    kOwnerNotRoot      = 1 << 9,
    kFileTypeUnknown   = 0x0 << 16,
    kFileTypeFlat      = 0x1 << 16,
    kFileTypeDirectory = 0x2 << 16,
    kFileTypeLink      = 0x3 << 16,
    kFileTypeMask      = 0x3 << 16
};

static long long
timeval_diff(struct timeval *difference,
             struct timeval *end_time,
             struct timeval *start_time
             )
{
    struct timeval temp_diff;
    
    if(difference==NULL)
    {
        difference=&temp_diff;
    }
    
    difference->tv_sec =end_time->tv_sec -start_time->tv_sec ;
    difference->tv_usec=end_time->tv_usec-start_time->tv_usec;
    
    /* Using while instead of if below makes the code slightly more robust. */
    
    while(difference->tv_usec<0)
    {
        difference->tv_usec+=1000000;
        difference->tv_sec -=1;
    }
    
    return 1000000LL*difference->tv_sec+
    difference->tv_usec;
    
} 

extern const char *getToken(const char *line, const char **begin, int *len);

bool getValueForConfigTableKey(config_file_t *config, const char *key, const char **val, int *size)
{
    
	if (config->dictionary != 0 ) {
		// Look up key in XML dictionary
		TagPtr value;
		value = XMLGetProperty(config->dictionary, key);
		if (value != 0) {
			if (value->type != kTagTypeString) {
				printf("Non-string tag '%s' found in config file\n",
					  key);
				abort();
				return false;
			}
			*val = value->string;
			*size = strlen(value->string);
			return true;
		}
	} else {
		
		// Legacy plist-style table
		
	}
	
	return false;
}

extern bool getValueForBootKey(const char *line, const char *match, const char **matchval, int *len);
extern bool getValueForKey( const char *key, const char **val, int *size, config_file_t *config );
extern bool   getIntForKey(const char *key, int *val, config_file_t *configBuff);
extern int    loadConfigFile(const char *configFile, config_file_t *configBuff);

static bool getDimensionForKey( const char *key, unsigned int *value, config_file_t *config, unsigned int dimension_max, unsigned int object_size )
{
	const char *val;
	
    int size = 0;
	int sum = 0;
    
	bool negative = false;
	bool percentage = false;    
	
    if (getValueForKey(key, &val, &size, config))
	{
		if ( size )
		{
			if (*val == '-')
			{
				negative = true;
				val++;
				size--;
			}
			
			if (val[size-1] == '%')
			{
				percentage = true;
				size--;
			}
			
			// convert string to integer
			for (sum = 0; size > 0; size--)
			{
				if (*val < '0' || *val > '9')
					return false;
				
				sum = (sum * 10) + (*val++ - '0');
			}
			
			if (percentage)
				sum = ( dimension_max * sum ) / 100;
			
			// calculate offset from opposite origin
			if (negative)
				sum =  ( ( dimension_max - object_size ) - sum );
			
		} else {
			
			// null value calculate center
			sum = ( dimension_max - object_size ) / 2;
			
		}
		
		*value = (uint16_t) sum;
		return true;
	}
	
	// key not found
    return false;
}

static bool getColorForKey( const char *key, unsigned int *value, config_file_t *config )
{
    const char *val;
    int size;
       
    if (getValueForKey(key, &val, &size, config))
	{
		if (*val == '#')
		{
            val++;
			*value = strtol(val, NULL, 16);
			return true;
        }
    }
    return false;
}

BOOLEAN LoadGui(VOID)
{
	
	int					val;
	long long	 index = 0;	
	uint32_t color = 0 ;			// color value formatted RRGGBB
	unsigned int pixel;
    EG_IMAGE_VIEW * deviceView = NULL;
    //EG_IMAGE_VIEW *PoofImageView[5] ;
    EG_IMAGE_VIEW * iconeRef = NULL;
	EG_VIEW * screen = NULL;

	UINT8	ImagesType = 0;
	
	long   ret, length, flags, time;
	CONST	CHAR8 * name;

	CHAR8 dirspec[512];
	BOOLEAN THEMEDIR_FOUND = FALSE;
	
	ret = GetFileInfo("rd(0,0)/Extra/Themes/", "Default", &flags, &time);
	if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeDirectory)) 
	{
		snprintf(dirspec, sizeof(dirspec),"rd(0,0)/Extra/Themes/Default/");
		THEMEDIR_FOUND = TRUE;
		
	}
	else
	{
		
		ret = GetFileInfo("/Extra/Themes/", "Default", &flags, &time);
		if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeDirectory))
		{
			snprintf(dirspec, sizeof(dirspec), "/Extra/Themes/Default/");
			THEMEDIR_FOUND = TRUE;
			
		}
		else
		{
			ret = GetFileInfo("bt(0,0)/Extra/Themes/", "Default", &flags, &time);
			if ((ret == 0) && ((flags & kFileTypeMask) == kFileTypeDirectory))
			{
				snprintf(dirspec, sizeof(dirspec),"bt(0,0)/Extra/Themes/Default/");
				THEMEDIR_FOUND = TRUE;
				
			} 
		}
	}
	
	if (THEMEDIR_FOUND == FALSE)
	{
		return FALSE;
	}
	
	{
		// parse  themeconfig file
		CHAR8 configdirspec[1024];
		snprintf(configdirspec, sizeof(configdirspec), "%stheme.plist",dirspec);
		
		if (loadConfigFile (configdirspec, &themeConfig) != 0)
		{
			DBG("Unable to load embed theme plist datas.\n");
			
			return FALSE;
		}				
		
		DBG("themeconfig file parsed\n");
		
		// parse display size parameters (if any)
		if (getIntForKey("screen_width", &val, &themeConfig) && val > 0) {
			screen_params[0] = val;
		}
		if (getIntForKey("screen_height", &val, &themeConfig) && val > 0) {
			screen_params[1] = val;
		}	
		
		screen = egCreateView(screen_params[0], screen_params[1], TRUE);
		if (!screen) {		
			
			return FALSE;
		}    
		DBG("screen buffer created\n");
		
		if(getColorForKey("screen_bgcolor", &color, &themeConfig) && color)
		{
			color = (color & 0x00FFFFFF);		
			EG_PIXEL Color;
			
			Color.r = ((color >> 16) & 0xFF) ;  
			Color.g = ((color >> 8) & 0xFF) ;   
			Color.b = ((color) & 0xFF) ;
			Color.a = 255;
			egViewSetBackgroundColor(screen,&Color);        
			
		}
		else
		{
			// flush CompImage
			EG_PIXEL Color;
			Color.r = 0 ;
			Color.g = 0 ;
			Color.b = 0 ;
			Color.a = 0;
			egViewSetBackgroundColor(screen,&Color);        
			
		}
	}
	
	while (1) {
		ret = GetDirEntry(dirspec, &index, &name, &flags, &time);
		if (ret == -1) break;

		if (!name) continue;
		
		DBG("testing %s\n", name);
		
		// Make sure this is a not directory.
		if ((flags & kFileTypeMask) != kFileTypeFlat) continue;		
		
		length = AsciiStrLen(name);
		
		if (!length) continue;

		// Make sure the image file is supported.
		{
			if (AsciiStriCmp(name + length - 4, ".png") == 0)
			{
				ImagesType = 1 ;//"PNG"
			}
			else if (AsciiStriCmp(name + length - 4, ".bmp") == 0) 
			{
				ImagesType = 2 ;//"BMP"
				
			}
			else if (AsciiStriCmp(name + length - 4, ".icns") == 0)
			{
				ImagesType = 3 ;//"ICNS"
			}			
			else
			{
				DBG("Image %s format is not supported and will be ignored\n", name);
				continue;
			}
		}
				
		if (!(ImagesType > 0)) {
			continue;
		}
		
		CHAR8 FilePath8[1024];		
		snprintf(FilePath8, sizeof(FilePath8), "%s%s",dirspec,name);
		
		//CHAR16 FilePath[1024];
		//AsciiStrToUnicodeStr(FilePath8, FilePath);		
		
		DBG("loading %s\n", name);
		EG_IMAGE * image = NULL/*egLoadImage(FilePath, TRUE)*/;
		
		//if (!image) continue;
		
		{
			void *BkgColor = NULL;
			const char*pstrFileName = FilePath8;
			int piWidth  = 0;
			int piHeight = 0;
			unsigned char *ppbImageData = NULL ;
			int piChannels = 0;
			bool ret ;
			ret = PngLoadImage (pstrFileName, &ppbImageData,
								&piWidth, &piHeight, &piChannels, BkgColor);
			
			if (ret != FALSE) {
				
				image = egCreateImageFromData((EG_PIXEL *)ppbImageData, piWidth, piHeight, (piChannels == 4) ? TRUE : FALSE, TRUE);
				
				if (!image) continue;
			}
		}
		
		DBG("Creating view for %s\n", name);
		
		CHAR16 name16[128];
		AsciiStrToUnicodeStr(name, name16);

		if (ImagesType == 1 ) //"PNG"
		{
			DBG("png file detected\n");

			if (strcmp(name,"logo.png") == 0)
			{
				DBG("computing logo.png\n");

				EG_POSITION pos ;
				/*
				pos.x = (screen_params[0] - MIN(image->Width, screen_params[0])) /2;
				pos.y = (screen_params[1] - MIN(image->Height, screen_params[1])) /2;
				
				if(getDimensionForKey("logo_pos_x", &pixel, &themeConfig, screen_params[0] , image->Width ) )
					pos.x = pixel;
				
				if(getDimensionForKey("logo_pos_y", &pixel, &themeConfig, screen_params[1] , image->Height ) )
					pos.y = pixel;				
				*/
				pos.y = 20;				
				pos.x = 20;

				
				EG_IMAGE_VIEW * TopImageView =  egCreateImageViewFromData((EG_PIXEL *)image->PixelData, image->Width, image->Height, TRUE , pos.x, pos.y, name16, TRUE);
				
				if (TopImageView) {				
					egViewAddImageView(screen, TopImageView);
					egFreeImageView(TopImageView);
				}			
				
				egFreeImage(image);
				
				
			} 
			else if (strcmp(name,"device_hfsplus.png")==0)
			{
				DBG("computing device_hfsplus.png\n");

				EG_POSITION pos ;
				pos.x = 0;
				pos.y = 0;
				
				if(getDimensionForKey("devices_pos_x", &pixel, &themeConfig, screen_params[0] , image->Width ) )
					pos.x  = pixel;
				else
					pos.x  = (screen_params[0] - MIN(image->Width, screen_params[0])) /2;
				
				if(getDimensionForKey("devices_pos_y", &pixel, &themeConfig, screen_params[1] , image->Height ) )
					pos.y = pixel;
				else
					pos.y = (screen_params[1] - MIN(image->Height, screen_params[1])) /2;			
												
				EG_IMAGE_VIEW * TopImageView =  egCreateImageViewFromData((EG_PIXEL *)image->PixelData, image->Width, image->Height, TRUE , pos.x, pos.y, name16, TRUE);			
				
				if (TopImageView) {				
					deviceView =  egViewAddImageView(screen, TopImageView);				
					egFreeImageView(TopImageView);
				}	
				
				egFreeImage(image);
				
				
			} 
			else 
				DBG("file not handled\n");
			
		} 
		else if (ImagesType == 3 ) //"ICNS"
		{
			DBG("icns file detected\n");

			EG_IMAGE * icnsImage = NULL;
			if ((icnsImage = egDecodeICNS((UINT8 *)image->PixelData, image->Width * image->Height * 4, 128, TRUE)))
			{				
				
				EG_IMAGE_VIEW * iconeView =  egCreateImageViewFromData((EG_PIXEL *)image->PixelData, image->Width, image->Height, TRUE , 20, 20, name16, TRUE);	
				
				if (iconeView) {
					iconeRef = egViewAddImageView(screen, iconeView);
					egFreeImageView(iconeView);
				}				
				
				egFreeImage(icnsImage);
				egFreeImage(image);


			}            
		}			
		
	}	
	
       
	if (!screen) {
		return FALSE;
	}
	
    __setVideoMode( GRAPHICS_MODE );
    egViewUpdate(screen);

    getc();
	DBG("LETS HAVE SOME FUN\n"); // you should never see this message

	
	/********************************
	 *								*								
	 *		LETS HAVE SOME FUN	    *			
	 *								*	
	 *								*	
	 ********************************/
    
#define TEXT_YMARGIN (2)
#define LAYOUT_TEXT_WIDTH (512)
#define LAYOUT_TOTAL_HEIGHT (368)
#define FONT_CELL_HEIGHT (12)
#define TEXT_LINE_HEIGHT (FONT_CELL_HEIGHT + TEXT_YMARGIN * 2)
    
    
    UINTN px = (screen_params[0] - MIN(((LAYOUT_TEXT_WIDTH *2 )/ 10) , screen_params[0])) /2, i;
    UINTN py = (screen_params[1] - MIN(((TEXT_LINE_HEIGHT * 8) /  10) , screen_params[1])) /2;
    EG_IMAGE_VIEW * ConsoleImageView = egCreateImageView(LAYOUT_TEXT_WIDTH / 10, (TEXT_LINE_HEIGHT * 8) /  10, FALSE, px , py, L"SCREEN_Console");
    assert(ConsoleImageView);
    assert(ConsoleImageView->Image);
    
    egFillImage(ConsoleImageView->Image, &MenuBackgroundPixel);
    
    EG_IMAGE_VIEW * ConsoleImageViewRef = egViewAddImageView(screen, ConsoleImageView);
    egFreeImageView(ConsoleImageView);
    egViewUpdate(screen);
    
    // ios/osx style window bounce effect
    for (i = 1; ConsoleImageViewRef->Image->Width <= LAYOUT_TEXT_WIDTH *2*2; i++) {
        
        if (ConsoleImageViewRef->Image->Height > TEXT_LINE_HEIGHT * 8 *2)
            break;
#define soustraction(x,y)   ((x) - (y))
        ConsoleImageViewRef->Image->Width += soustraction(i, (i==1)?0:1);
        ConsoleImageViewRef->Image->Height += soustraction(i, (i==1)?0:1);
        px = (screen_params[0] - MIN(ConsoleImageViewRef->Image->Width , screen_params[0])) /2;
        py = (screen_params[1] - MIN(ConsoleImageViewRef->Image->Height , screen_params[1])) /2;
        
        ConsoleImageViewRef->PosY = py ;
        ConsoleImageViewRef->PosX = px ;
        
        free(ConsoleImageViewRef->Image->PixelData);
        
        ConsoleImageViewRef->Image->PixelData = (EG_PIXEL*)malloc(ConsoleImageViewRef->Image->Width * ConsoleImageViewRef->Image->Height * sizeof(EG_PIXEL));
        
        egFillImage(ConsoleImageViewRef->Image, &MenuBackgroundPixel);
        screen->isDirty = TRUE;
        egViewUpdate(screen);
        
        
    }
    
    for (i = 0; i < 10; i++) {
        
        
#define soustraction(x,y)   ((x) - (y))
        ConsoleImageViewRef->Image->Width -= i;
        ConsoleImageViewRef->Image->Height -= i;
        px = (screen_params[0] - MIN(ConsoleImageViewRef->Image->Width , screen_params[0])) /2;
        py = (screen_params[1] - MIN(ConsoleImageViewRef->Image->Height , screen_params[1])) /2;
        
        ConsoleImageViewRef->PosY = py ;
        ConsoleImageViewRef->PosX = px ;
        
        free(ConsoleImageViewRef->Image->PixelData);
        
        ConsoleImageViewRef->Image->PixelData = (EG_PIXEL*)malloc(ConsoleImageViewRef->Image->Width * ConsoleImageViewRef->Image->Height * sizeof(EG_PIXEL));
        
        egFillImage(ConsoleImageViewRef->Image, &MenuBackgroundPixel);

        screen->isDirty = TRUE;
        egViewUpdate(screen);
        
        
    }
	egViewRemoveImageView(screen, ConsoleImageViewRef);

    /*
    UINTN TextWidth;

    EG_POSITION pos ;
    pos.x = (screen_params[0] - MIN(LAYOUT_TEXT_WIDTH, screen_params[0])) /2;
    pos.y = (screen_params[1] - MIN(TEXT_LINE_HEIGHT, screen_params[1])) /2;
    if (deviceView) {
        pos.y = deviceView->PosY - 10;
        
    }
      
    EG_IMAGE_VIEW * TextImageView = egCreateImageView(LAYOUT_TEXT_WIDTH, TEXT_LINE_HEIGHT, FALSE, pos.x, pos.y , L"SCREEN_HEADER");

    assert(TextImageView);
    assert(TextImageView->Image);
    
    egFillImage(TextImageView->Image, &MenuBackgroundPixel);
    
    // render the text
    egMeasureText(L"WELCOME TO THE EG GRAPHIC TEST... PRESS ANY KEY TO CONTINUE.", &TextWidth, NULL);
    egRenderText(L"WELCOME TO THE EG GRAPHIC TEST... PRESS ANY KEY TO CONTINUE.", TextImageView->Image, (TextImageView->Image->Width - TextWidth) >> 1, 2);
    
    egViewAddImageView(screen, TextImageView);
    egFreeImageView(TextImageView);
    egViewUpdate(screen);
    getc();
    egViewRemoveImageViewByName(screen, L"SCREEN_HEADER", sizeof(L"SCREEN_HEADER"));
    */
    egViewUpdate(screen);
    getc();
    deviceView = deviceView ? deviceView : iconeRef;
    if (deviceView) {
        struct timeval earlier;
        struct timeval later;
        
        uint32_t c;
        uint32_t i;
        uint32_t x,y;
        uint32_t x2,y2;

        for (c = 0; c < 5; c++) {
            if(gettimeofday(&earlier,NULL))
            {                
                assert(1);
            }
            int inc = 1,count = 0;
            x = deviceView->PosX;
            y = deviceView->PosY;
            
            int inc2 = 1,count2 = 0;
            x2 = iconeRef->PosX;
            y2 = iconeRef->PosY;
            
            // bounce left to right test (aka say NO)
            for (i = 0; i<100; i++) {
                
                if (count >= 5 && count2 >= 5) break;
                
                if (count < 5) {
                    if (deviceView->PosX >= x )
                    {
                        if (deviceView->PosX == x )
                        {
                            count++;
                            if (count == 5) {
                                screen->isDirty = TRUE;
                                egViewUpdate(screen);
                                inc = -1;
                            } else inc = 1;
                        }
                        
                        if (inc >= 0)
                        {
                            if (deviceView->PosX >= x+10 ) {
                                deviceView->PosX-=2;
                                inc = 0;
                                screen->isDirty = TRUE;
                                
                            }
                            else
                            {
                                if (inc == 1) {
                                    deviceView->PosX+=2;
                                    
                                } else deviceView->PosX-=2;
                                screen->isDirty = TRUE;
                                
                                
                            }
                        }
                        
                        
                        
                        
                    }
                }
                                
                egViewUpdate(screen);
                
                if (count2 < 5 && deviceView != iconeRef) {
                    if (iconeRef->PosX >= x2 )
                    {
                        if (iconeRef->PosX == x2 )
                        {
                            count2++;
                            if (count2 == 5) {
                                screen->isDirty = TRUE;
                                egViewUpdate(screen);
                                inc2 = -1;
                            } else inc2 = 1;
                        }
                        
                        if (inc2 >= 0)
                        {
                            if (iconeRef->PosX >= x2+10 ) {
                                iconeRef->PosX-=2;
                                inc2 = 0;
                                screen->isDirty = TRUE;
                                
                            }
                            else
                            {
                                if (inc2 == 1) {
                                    iconeRef->PosX+=2;
                                    
                                } else iconeRef->PosX-=2;
                                screen->isDirty = TRUE;
                                
                            }
                        }                       
                        
                        
                    }
                }
                
            }
            if(gettimeofday(&later,NULL))
            {
                
                exit(1);
            }
            
            CHAR16 Destination[512];                    
            
            SPrint (
                    Destination,
                    sizeof(Destination),
                    L"animation %d took %lld microseconds to finish", c,
                    timeval_diff(NULL,&later,&earlier));
            
            UINTN TextWidth;
            
            EG_POSITION pos ;
            pos.x = (screen_params[0] - MIN(LAYOUT_TEXT_WIDTH, screen_params[0])) /2;
            pos.y = (screen_params[1] - MIN(TEXT_LINE_HEIGHT, screen_params[1])) /2;
            if (deviceView) {
                pos.y = deviceView->PosY + 10;
                
            }
            
            EG_IMAGE_VIEW * TextImageView = egCreateImageView(300, TEXT_LINE_HEIGHT, FALSE, pos.x, pos.y , L"SCREEN_NOTIFICATION");
            
            assert(TextImageView);
            assert(TextImageView->Image);
            MenuBackgroundPixel.a -=50;
            egFillImage(TextImageView->Image, &MenuBackgroundPixel);
            
            // render the text
            egMeasureText(Destination, &TextWidth, NULL);
            egRenderText(Destination, TextImageView->Image, (TextImageView->Image->Width - TextWidth) >> 1, 2);
            
            egViewAddImageView(screen, TextImageView);
            egFreeImageView(TextImageView);
            egViewUpdate(screen);
            
            getc();
            egViewRemoveImageViewByName(screen, L"SCREEN_NOTIFICATION", sizeof(L"SCREEN_NOTIFICATION"));

        }
        
        
        // change background test
        EG_PIXEL *bColor = egViewGetBackgroundColor(screen); // save original color
        for (i = 0; i<15; i++) {
            EG_PIXEL Color;
            Color.r = arc4random_unirange(0,255) ;
            Color.g = arc4random_unirange(0,255) ;
            Color.b = arc4random_unirange(0,255) ;
            Color.a = arc4random_unirange(0,255);
            
            egViewSetBackgroundColor(screen, &Color); 
            delay(100000);
            egViewUpdate(screen);
        }
        sleep(1);        
        egViewSetBackgroundColor(screen,bColor); // restore color
        free(bColor);
        
        // move image test
        for (i = 0; i<100; i++) {
            
            
            
           deviceView->PosX +=2;
           deviceView->PosY +=2;
            
           screen->isDirty = TRUE;
            
            egViewUpdate(screen);
        }
        
        int incx , incy ;
        int bxpl , bypl ;

        
        incx = arc4random_unirange(0,1);
        incy = arc4random_unirange(0,1);
            
        bxpl = arc4random_unirange(3,6);
        bypl = arc4random_unirange(3,6);

        
        for (i = 0; i<5000; i++) {
            
            

            if (deviceView->PosX >= bxpl  && incx <= 0) {
                deviceView->PosX -=bxpl;
                incx = 0;

            }
            else if (deviceView->PosX + deviceView->Image->Width + bxpl < screen->CompImage->Width )
            {
                deviceView->PosX +=bxpl;
                if (incx != 1) {
                    // will exit rect
                    bxpl = arc4random_unirange(3,6);

                }
                incx = 1;
            }
            else if (deviceView->PosX + deviceView->Image->Width + bxpl >= screen->CompImage->Width  && incx != 0)
            {
                incx = -1;
                bxpl = arc4random_unirange(3,6);

                //continue;
            }
            
            
            if (deviceView->PosY >= bypl  && incy <= 0) {
                deviceView->PosY -=bypl;
                incy = 0;
                
            }
            else if (deviceView->PosY + deviceView->Image->Height + bypl < screen->CompImage->Height )
            {
                deviceView->PosY += bypl;

                if (incy != 1) {
                    // will exit rect (top)
                    bypl = arc4random_unirange(3,6);
                    
                }

                incy = 1;
            }
            else if (deviceView->PosY + deviceView->Image->Height + bypl >= screen->CompImage->Height  && incy != 0)
            {
                // will exit rect ( bottom)

                incy = -1;
                bypl = arc4random_unirange(3,6);

                //continue;
            }            
                        
            if (incy>= 0 && incx>=0) {
                screen->isDirty = TRUE;
                
                egViewUpdate(screen);
            }
            
            
            
        }
        
        getc();
        
        egViewRemoveImageViewByName(screen, L"plug_in_Icon", sizeof(L"plug_in_Icon"));
        egViewUpdate(screen);
        getc();        
       
    }    
    
    __setVideoMode(VGA_TEXT_MODE);

    egFreeView(screen);

	return TRUE;
	
}
