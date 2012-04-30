
/* Graphic utility functions and data types
 * Prashant Vaibhav (C) 12/12/2008
 * Chameleon
 */
#include "libsaio.h"
#include "bootstruct.h"
#include "graphic_utils.h"
#include "IOHibernatePrivate.h"
#include "bmdecompress.h"
#include "vbe.h"
#include "appleClut8.h"


#ifndef offsetof
#define offsetof(st, m) \
((size_t) ( (char *)&((st *)(0))->m - (char *)0 ))
#endif

#define VIDEO(x) (((boot_args_common*)getBootArgs())->Video.v_ ## x)


#define MIN(x, y) ((x) < (y) ? (x) : (y))

int previewTotalSectors = 0;
uint8_t *previewSaveunder = 0;
int previewLoadedSectors = 0;
#if UNUSED
static int setVESAGraphicsMode( unsigned short width, unsigned short height, unsigned char  bitsPerPixel, unsigned short refreshRate );
#else
static int setVESAGraphicsMode( unsigned short width, unsigned short height, unsigned char  bitsPerPixel);
#endif

void
loadImageScale (void *input, int iw, int ih, int ip, void *output, int ow, int oh, int op, int or)
{
	int x,y, off;
	int red=0x7f, green=0x7f, blue=0x7f;
	for (x=0;x<ow;x++)
		for (y=0;y<oh;y++)
		{
			off=(x*iw)/ow+((y*ih)/oh)*iw;
			switch (ip)
			{
				case 16:
				{
					uint16_t val;
					val=((uint16_t *)input)[off];
					red=(val>>7)&0xf8;
					green=(val>>2)&0xf8;
					blue=(val<<3)&0xf8;
					break;		
				}
				case 32:
				{
					uint32_t val;
					val=((uint32_t *)input)[off];
					red=(val>>16)&0xff;
					green=(val>>8)&0xff;
					blue=(val)&0xff;
					break;
				}
				default:
					break;
			}
			char *ptr=(char *)output+x*(op/8)+y*or;
			switch (op)
			{
				case 16:
					*((uint16_t *)ptr) = ((red   & 0xF8) << 7) | 
					((green & 0xF8) << 2) |
					((blue  & 0xF8) >> 3);
					break;
				case 32 :
					*((uint32_t *)ptr) = (red << 16) | (green << 8) | blue;
					break;
				default:
					break;
			}
		}
}

//==========================================================================
// getNumberArrayFromProperty

static int
getNumberArrayFromProperty( const char *  propKey,
                           unsigned long numbers[],
                           unsigned long maxArrayCount )
{
    char * propStr;
    unsigned long    count = 0;
	
    propStr = newStringForKey( (char *) propKey , DEFAULT_BOOT_CONFIG );
    if ( propStr )
    {
        char * delimiter = propStr;
        char * p = propStr;
		
        while ( count < maxArrayCount && *p != '\0' )
        {
            unsigned long val = strtoul( p, &delimiter, 10 );
            if ( p != delimiter )
            {
                numbers[count++] = val;
                p = delimiter;
            }
            while ( ( *p != '\0' ) && !isdigit(*p) )
                p++;
        }
		
        free( propStr );
    }
	
    return count;
}

static int initGraphicsMode ()
{
	unsigned long params[4];
	int           count;
	
	params[3] = 0;
	count = getNumberArrayFromProperty( kGraphicsModeKey, params, 4 );
	
	// Try to find a resolution if "Graphics Mode" setting is not available.
	if ( count < 3 )
	{
		params[0] = DEFAULT_SCREEN_WIDTH;
		params[1] = DEFAULT_SCREEN_HEIGHT;
		params[2] = 32;
	}
	
	// Map from pixel format to bits per pixel.
	
	if ( params[2] == 256 ) params[2] = 8;
	if ( params[2] == 555 ) params[2] = 16;
	if ( params[2] == 888 ) params[2] = 32;
#if UNUSED
	return setVESAGraphicsMode( params[0], params[1], params[2], params[3] );	
#else
	return setVESAGraphicsMode( params[0], params[1], params[2] );
#endif
}

//==========================================================================
// setupPalette

static void setupPalette( VBEPalette * p, const unsigned char * g )
{
    int             i;
    unsigned char * source = (unsigned char *) g;
	
    for (i = 0; i < 256; i++)
    {
        (*p)[i] = 0;
        (*p)[i] |= ((unsigned long)((*source++) >> 2)) << 16;   // Red
        (*p)[i] |= ((unsigned long)((*source++) >> 2)) << 8;    // Green
        (*p)[i] |= ((unsigned long)((*source++) >> 2));         // Blue
		
    }
}

//==========================================================================
// getVESAModeWithProperties
//
// Return the VESA mode that matches the properties specified.
// If a mode is not found, then return the "best" available mode.

static unsigned short
getVESAModeWithProperties( unsigned short     width,
                          unsigned short     height,
                          unsigned char      bitsPerPixel,
                          unsigned short     attributesSet,
                          unsigned short     attributesClear,
                          VBEModeInfoBlock * outModeInfo,
                          unsigned short *   vesaVersion )
{
    VBEInfoBlock     vbeInfo;
    unsigned short * modePtr;
    VBEModeInfoBlock modeInfo;
    unsigned char    modeBitsPerPixel;
    unsigned short   matchedMode = modeEndOfList;
    int              err;
	
    // Clear output mode info.
	
    //bzero( outModeInfo, sizeof(*outModeInfo) );
	bzero( outModeInfo, sizeof(VBEModeInfoBlock) );
	
    // Get VBE controller info containing the list of supported modes.
	
    //bzero( &vbeInfo, sizeof(vbeInfo) );
    bzero( &vbeInfo, sizeof(VBEInfoBlock) );
    
    strcpy( (char*)&vbeInfo, "VBE2" );
    err = getVBEInfo( &vbeInfo );
    if ( err != errSuccess )
    {
        return modeEndOfList;
    }
	
    // Report the VESA major/minor version number.
	
    if (vesaVersion) *vesaVersion = vbeInfo.VESAVersion;
	
    // Loop through the mode list, and find the matching mode.
	
    for ( modePtr = VBEDecodeFP( unsigned short *, vbeInfo.VideoModePtr );
		 *modePtr != modeEndOfList; modePtr++ )
    {
        // Get mode information.
		
        //bzero( &modeInfo, sizeof(modeInfo) );
        bzero( &modeInfo, sizeof(VBEModeInfoBlock) );
		
        err = getVBEModeInfo( *modePtr, &modeInfo );
        if ( err != errSuccess )
        {
            continue;
        }
		
#if DEBUG
        printf("Mode %x: %dx%dx%d mm:%d attr:%x\n",
               *modePtr, modeInfo.XResolution, modeInfo.YResolution,
               modeInfo.BitsPerPixel, modeInfo.MemoryModel,
               modeInfo.ModeAttributes);
#endif
		
        // Filter out unwanted modes based on mode attributes.
		
        if ( ( ( modeInfo.ModeAttributes & attributesSet ) != attributesSet )
			||   ( ( modeInfo.ModeAttributes & attributesClear ) != 0 ) )
        {
            continue;
        }
		
        // Pixel depth in bits.
		
        modeBitsPerPixel = modeInfo.BitsPerPixel;
		
        if ( ( modeBitsPerPixel == 4 ) && ( modeInfo.MemoryModel == 0 ) )
        {
            // Text mode, 16 colors.
        }
        else if ( ( modeBitsPerPixel == 8 ) && ( modeInfo.MemoryModel == 4 ) )
        {
            // Packed pixel, 256 colors.
        }
        else if ( ( ( modeBitsPerPixel == 16 ) || ( modeBitsPerPixel == 15 ) )
				 &&   ( modeInfo.MemoryModel   == 6 )
				 &&   ( modeInfo.RedMaskSize   == 5 )
				 &&   ( modeInfo.GreenMaskSize == 5 )
				 &&   ( modeInfo.BlueMaskSize  == 5 ) )
        {
            // Direct color, 16 bpp (1:5:5:5).
            modeInfo.BitsPerPixel = modeBitsPerPixel = 16;
        }
        else if ( ( modeBitsPerPixel == 32 )
				 &&   ( modeInfo.MemoryModel   == 6 )
				 &&   ( modeInfo.RedMaskSize   == 8 )
				 &&   ( modeInfo.GreenMaskSize == 8 )
				 &&   ( modeInfo.BlueMaskSize  == 8 ) )
        {
            // Direct color, 32 bpp (8:8:8:8).
        }
        else
        {
            continue; // Not a supported mode.
        }
		
        // Modes larger than the specified dimensions are skipped.
		
        if ( ( modeInfo.XResolution > width  ) ||
			( modeInfo.YResolution > height ) )
        {
            continue;
        }
		
        // Perfect match, we're done looking.
		
        if ( ( modeInfo.XResolution == width  ) &&
			( modeInfo.YResolution == height ) &&
			( modeBitsPerPixel     == bitsPerPixel ) )
        {
            matchedMode = *modePtr;
            //bcopy( &modeInfo, outModeInfo, sizeof(modeInfo) );            
            bcopy( &modeInfo, outModeInfo, sizeof(VBEModeInfoBlock) );
			
            break;
        }
		
        // Save the next "best" mode in case a perfect match is not found.
		
        if ( modeInfo.XResolution == outModeInfo->XResolution &&
			modeInfo.YResolution == outModeInfo->YResolution &&
			modeBitsPerPixel     <= outModeInfo->BitsPerPixel )
        {
            continue;  // Saved mode has more depth.
        }
        if ( modeInfo.XResolution < outModeInfo->XResolution ||
			modeInfo.YResolution < outModeInfo->YResolution ||
			modeBitsPerPixel     < outModeInfo->BitsPerPixel )
        {
            continue;  // Saved mode has more resolution.
        }
		
        matchedMode = *modePtr;
        bcopy( &modeInfo, outModeInfo, sizeof(VBEModeInfoBlock) );
		
    }
	
    return matchedMode;
}

//==========================================================================
// setVESAGraphicsMode
#if UNUSED
static int setVESAGraphicsMode( unsigned short width, unsigned short height, unsigned char  bitsPerPixel, unsigned short refreshRate )
#else
static int setVESAGraphicsMode( unsigned short width, unsigned short height, unsigned char  bitsPerPixel)
#endif
{
    VBEModeInfoBlock  minfo;
    unsigned short    mode;
    unsigned short    vesaVersion;
    int               err = errFuncNotSupported;
	
    do {
        mode = getVESAModeWithProperties( width, height, bitsPerPixel,
										 maColorModeBit             |
										 maModeIsSupportedBit       |
										 maGraphicsModeBit          |
										 maLinearFrameBufferAvailBit,
										 0,
										 &minfo, &vesaVersion );
        if ( mode == modeEndOfList )
        {
            break;
        }
#if UNUSED
		//
		// FIXME : generateCRTCTiming() causes crash.
		//
		
		//         if ( (vesaVersion >> 8) >= 3 && refreshRate >= 60 &&
		//              (gBootMode & kBootModeSafe) == 0 )
		//         {
		//             VBECRTCInfoBlock timing;
		//     
		//             // Generate CRTC timing for given refresh rate.
		// 
		//             generateCRTCTiming( minfo.XResolution, minfo.YResolution,
		//                                 refreshRate, kCRTCParamRefreshRate,
		//                                 &timing );
		// 
		//             // Find the actual pixel clock supported by the hardware.
		// 
		//             getVBEPixelClock( mode, &timing.PixelClock );
		// 
		//             // Re-compute CRTC timing based on actual pixel clock.
		// 
		//             generateCRTCTiming( minfo.XResolution, minfo.YResolution,
		//                                 timing.PixelClock, kCRTCParamPixelClock,
		//                                 &timing );
		// 
		//             // Set the video mode and use specified CRTC timing.
		// 
		//             err = setVBEMode( mode | kLinearFrameBufferBit |
		//                               kCustomRefreshRateBit, &timing );
		//         }
		//         else
		//         {
		//             // Set the mode with default refresh rate.
		// 
		//             err = setVBEMode( mode | kLinearFrameBufferBit, NULL );
		//         }
#endif
        // Set the mode with default refresh rate.
		
        err = setVBEMode( mode | kLinearFrameBufferBit, NULL );
		
        if ( err != errSuccess )
        {
            break;
        }
		
        // Set 8-bit color palette.
		
        if ( minfo.BitsPerPixel == 8 )
        {
            VBEPalette palette;
            setupPalette( &palette, appleClut8 );
            if ((err = setVBEPalette(palette)) != errSuccess)
            {
                break;
            }
        }
		
        // Is this required for buggy Video BIOS implementations?
        // On which adapter?
		
        if ( minfo.BytesPerScanline == 0 )
			minfo.BytesPerScanline = ( minfo.XResolution *
									  minfo.BitsPerPixel ) >> 3;
		
        // Update KernBootStruct using info provided by the selected
        // VESA mode.
        Boot_Video	Video;		/* Video Information */

        
        Video.v_display  = GRAPHICS_MODE;
        Video.v_width    = minfo.XResolution;
        Video.v_height   = minfo.YResolution;
        Video.v_depth    = minfo.BitsPerPixel;
        Video.v_rowBytes = minfo.BytesPerScanline;
        Video.v_baseAddr = VBEMakeUInt32(minfo.PhysBasePtr);
        
        
		setBootArgsVideoStruct(&Video);

    }
    while ( 0 );
	
    return err;
}

//==========================================================================
// setVESATextMode

static int
setVESATextMode( unsigned short cols,
				unsigned short rows,
				unsigned char  bitsPerPixel )
{
    VBEModeInfoBlock  minfo;
	minfo.XResolution = 0;
    minfo.YResolution = 0;
    unsigned short    mode = modeEndOfList;
	
    if ( (cols != 80) || (rows != 25) )  // not 80x25 mode
    {
        mode = getVESAModeWithProperties( cols, rows, bitsPerPixel,
                                         maColorModeBit |
                                         maModeIsSupportedBit,
                                         maGraphicsModeBit,
                                         &minfo, NULL );
    }
	
    if ( ( mode == modeEndOfList ) || ( setVBEMode(mode, NULL) != errSuccess ) )
    {
        video_mode( 2 );  // VGA BIOS, 80x25 text mode.
        minfo.XResolution = 80;
        minfo.YResolution = 25;
    }
	
    // Update KernBootStruct using info provided by the selected
    // VESA mode.
	Boot_Video	Video;		/* Video Information */
    
    
    Video.v_display  = VGA_TEXT_MODE;
    Video.v_width    = 0xb8000;
    Video.v_height   = minfo.XResolution;
    Video.v_depth    = minfo.YResolution;
    Video.v_rowBytes = 8;
    Video.v_baseAddr = 0x8000;    
    
    setBootArgsVideoStruct(&Video); 
   
	
    return errSuccess;  // always return success
}

//==========================================================================
// setVideoMode
//
// Set the video mode to VGA_TEXT_MODE or GRAPHICS_MODE.
#if UNUSED
void
setVideoMode( int mode, int drawgraphics)
#else
void
setVideoMode( int mode)
#endif
{
    unsigned long params[4];
    int           count;
    int           err = errSuccess;
	
    if ( mode == GRAPHICS_MODE )
    {
  		if ( (err=initGraphicsMode ()) == errSuccess ) {
			if (gVerboseMode) {
				// Tell the kernel to use text mode on a linear frame buffer display
                setBootArgsVideoMode(FB_TEXT_MODE);
			} else {
                setBootArgsVideoMode(GRAPHICS_MODE);
			}
		}
    }
	
    if ( (mode == VGA_TEXT_MODE) || (err != errSuccess) )
    {
        count = getNumberArrayFromProperty( kTextModeKey, params, 2 );
        if ( count < 2 )
        {
            params[0] = 80;  // Default text mode is 80x25.
            params[1] = 25;
        }
		
		setVESATextMode( params[0], params[1], 4 );
        setBootArgsVideoMode(VGA_TEXT_MODE);

    }	

}

//==========================================================================
// LookUpCLUTIndex

static unsigned long lookUpCLUTIndex( unsigned char index,
                                     unsigned char depth )
{
    long result, red, green, blue;
	
    red   = appleClut8[index * 3 + 0];
    green = appleClut8[index * 3 + 1];
    blue  = appleClut8[index * 3 + 2];
	
    switch (depth) {
        case 16 :
            result = ((red   & 0xF8) << 7) | 
			((green & 0xF8) << 2) |
			((blue  & 0xF8) >> 3);
            result |= (result << 16);
            break;
			
        case 32 :
            result = (red << 16) | (green << 8) | blue;
            break;
			
        default :
            result = index | (index << 8);
            result |= (result << 16);
            break;
    }
	
    return result;
}

static void * stosl(void * dst, long val, long len)
{
    asm volatile ( "rep; stosl"
				  : "=c" (len), "=D" (dst)
				  : "0" (len), "1" (dst), "a" (val)
				  : "memory" );
	
    return dst;
}

static void drawColorRectangle( unsigned short x,
                          unsigned short y,
                          unsigned short width,
                          unsigned short height,
                          unsigned char  colorIndex )
{
    long   pixelBytes;
    long   color = lookUpCLUTIndex( colorIndex, VIDEO(depth) );
    char * vram;
	
    pixelBytes = VIDEO(depth) / 8;
    vram       = (char *) VIDEO(baseAddr) +
	VIDEO(rowBytes) * y + pixelBytes * x;
	
    width = MIN(width, VIDEO(width) - x);
    height = MIN(height, VIDEO(height) - y);
	
    while ( height-- )
    {
        int rem = ( pixelBytes * width ) % 4;
        if ( rem ) bcopy( &color, vram, rem );
        stosl( vram + rem, color, pixelBytes * width / 4 );
        vram += VIDEO(rowBytes);
    }
}

DECLARE_IOHIBERNATEPROGRESSALPHA

void drawPreview(void *src, uint8_t * saveunder)
{
	uint8_t *  screen;
	uint32_t   rowBytes, pixelShift;
	uint32_t   x, y;
	int32_t    blob;
	uint32_t   alpha, in, color, result;
	uint8_t *  out;
	void *uncomp;
	int origwidth, origheight, origbpx;
	uint32_t   saveindex[kIOHibernateProgressCount] = { 0 };
	
	if (src && (uncomp=DecompressData(src, &origwidth, &origheight, &origbpx)))
	{
#if UNUSED
		if (!setVESAGraphicsMode(origwidth, origheight, origbpx, 0))
#else
		if (!setVESAGraphicsMode(origwidth, origheight, origbpx))
#endif
			if (initGraphicsMode () != errSuccess)
				return;
		screen = (uint8_t *) VIDEO (baseAddr);
		rowBytes = VIDEO (rowBytes);
		loadImageScale (uncomp, origwidth, origheight, origbpx, screen, VIDEO(width), VIDEO(height), VIDEO(depth), VIDEO (rowBytes));
	}
	else
	{
		if (initGraphicsMode () != errSuccess)
			return;
		screen = (uint8_t *) VIDEO (baseAddr);
		rowBytes = VIDEO (rowBytes);
		// Set the screen to 75% grey.
        drawColorRectangle(0, 0, VIDEO(width), VIDEO(height), 0x01 /* color index */);
	}
	
	
	pixelShift = VIDEO (depth) >> 4;
	if (pixelShift < 1) return;
	
	screen += ((VIDEO (width) 
				- kIOHibernateProgressCount * (kIOHibernateProgressWidth + kIOHibernateProgressSpacing)) << (pixelShift - 1))
	+ (VIDEO (height) - kIOHibernateProgressOriginY - kIOHibernateProgressHeight) * rowBytes;
	
	for (y = 0; y < kIOHibernateProgressHeight; y++)
	{
		out = screen + y * rowBytes;
		for (blob = 0; blob < kIOHibernateProgressCount; blob++)
		{
			color = blob ? kIOHibernateProgressDarkGray : kIOHibernateProgressMidGray;
			for (x = 0; x < kIOHibernateProgressWidth; x++)
			{
				alpha  = gIOHibernateProgressAlpha[y][x];
				result = color;
				if (alpha)
				{
					if (0xff != alpha)
					{
						if (1 == pixelShift)
						{
							in = *((uint16_t *)out) & 0x1f;	// 16
							in = (in << 3) | (in >> 2);
						}
						else
							in = *((uint32_t *)out) & 0xff;	// 32
						saveunder[blob * kIOHibernateProgressSaveUnderSize + saveindex[blob]++] = in;
						result = ((255 - alpha) * in + alpha * result + 0xff) >> 8;
					}
					if (1 == pixelShift)
					{
						result >>= 3;
						*((uint16_t *)out) = (result << 10) | (result << 5) | result;	// 16
					}
					else
						*((uint32_t *)out) = (result << 16) | (result << 8) | result;	// 32
				}
				out += (1 << pixelShift);
			}
			out += (kIOHibernateProgressSpacing << pixelShift);
		}
	}
}

void updateProgressBar(uint8_t * saveunder, int32_t firstBlob, int32_t select)
{
	uint8_t * screen;
	uint32_t  rowBytes, pixelShift;
	uint32_t  x, y;
	int32_t   blob, lastBlob;
	uint32_t  alpha, in, color, result;
	uint8_t * out;
	uint32_t  saveindex[kIOHibernateProgressCount] = { 0 };
	
	pixelShift = VIDEO(depth) >> 4;
	if (pixelShift < 1) return;
	screen = (uint8_t *) VIDEO (baseAddr);
	rowBytes = VIDEO (rowBytes);
	
	screen += ((VIDEO (width) 
				- kIOHibernateProgressCount * (kIOHibernateProgressWidth + kIOHibernateProgressSpacing)) << (pixelShift - 1))
	+ (VIDEO (height) - kIOHibernateProgressOriginY - kIOHibernateProgressHeight) * rowBytes;
	
	lastBlob  = (select < kIOHibernateProgressCount) ? select : (kIOHibernateProgressCount - 1);
	
	screen += (firstBlob * (kIOHibernateProgressWidth + kIOHibernateProgressSpacing)) << pixelShift;
	
	for (y = 0; y < kIOHibernateProgressHeight; y++)
	{
		out = screen + y * rowBytes;
		for (blob = firstBlob; blob <= lastBlob; blob++)
		{
			color = (blob < select) ? kIOHibernateProgressLightGray : kIOHibernateProgressMidGray;
			for (x = 0; x < kIOHibernateProgressWidth; x++)
			{
				alpha  = gIOHibernateProgressAlpha[y][x];
				result = color;
				if (alpha)
				{
					if (0xff != alpha)
					{
						in = saveunder[blob * kIOHibernateProgressSaveUnderSize + saveindex[blob]++];
						result = ((255 - alpha) * in + alpha * result + 0xff) / 255;
					}
					if (1 == pixelShift)
					{
						result >>= 3;
						*((uint16_t *)out) = (result << 10) | (result << 5) | result;	// 16
					}
					else
						*((uint32_t *)out) = (result << 16) | (result << 8) | result;	// 32
				}
				out += (1 << pixelShift);
			}
			out += (kIOHibernateProgressSpacing << pixelShift);
		}
	}
}

void
spinActivityIndicator_hook(void *arg1, void *arg2, void *arg3, void *arg4, void* arg5, void* arg6)
{
	int sectors = *(int*)arg1;
	bool *doreturn = (bool*)arg2;
	
	if (previewTotalSectors && previewSaveunder)
	{
		int blob, lastBlob;
		
		lastBlob = (previewLoadedSectors * kIOHibernateProgressCount) / previewTotalSectors;
		previewLoadedSectors+=sectors;
		blob = (previewLoadedSectors * kIOHibernateProgressCount) / previewTotalSectors;
		
		if (blob!=lastBlob)
			updateProgressBar (previewSaveunder, lastBlob, blob);
		*doreturn = true;
	}
}