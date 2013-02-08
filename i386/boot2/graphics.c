/*
 * Copyright (c) 1999-2003 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 1999-2003 Apple Computer, Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 2.0 (the "License").  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON- INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*
 * Copyright 1993 NeXT, Inc.
 * All rights reserved.
 */

#include "boot.h"
#include "graphics.h"
#include "vbe.h"
#include "appleClut8.h"
#include "bootstruct.h"
#include "modules.h"
#include "platform.h"

#define VIDEO(x) (bootArgs->Video.v_ ## x)

#define MIN(x, y) ((x) < (y) ? (x) : (y))

static void setupPalette( VBEPalette * p, const unsigned char * g );
static int
setVESATextMode( unsigned short cols,
				unsigned short rows,
				unsigned char  bitsPerPixel );

static int initGraphicsMode (void);

static unsigned long lookUpCLUTIndex( unsigned char index,
                                     unsigned char depth );

static void * stosl(void * dst, long val, long len);

int
__convertImage( unsigned short width,
               unsigned short height,
               const unsigned char *imageData,
               unsigned char **newImageData )
{
    int cnt;
    unsigned char *img;
    unsigned short *img16;
    unsigned long *img32;
	
    switch ( VIDEO(depth) ) {
		case 16 :
        {
			img16 = calloc(2,width * height);
			if ( !img16 ) return 1;
			for (cnt = 0; cnt < (width * height); cnt++)
				img16[cnt] = lookUpCLUTIndex(imageData[cnt], 16);
			img = (unsigned char *)img16;
            *newImageData = img;
			break;
        }
		case 32 :
        {
			img32 = calloc(4,width * height);
			if ( !img32 ) return 1;
			for (cnt = 0; cnt < (width * height); cnt++)
				img32[cnt] = lookUpCLUTIndex(imageData[cnt], 32);
			img = (unsigned char *)img32;
            *newImageData = img;
			break;
        }
		default :
        {
			img = calloc(1,width * height);
            if ( !img ) return 1; 
			bcopy(imageData, img, width * height);
            *newImageData = img;
			break;
        }
    }
    
    return 0;
}

//==========================================================================
// drawDataRectangle

void __drawDataRectangle( unsigned short  x,
                         unsigned short  y,
                         unsigned short  width,
                         unsigned short  height,
                         unsigned char * data )
{
    unsigned short drawWidth;
    long   pixelBytes = VIDEO(depth) / 8;
    unsigned char * vram   = (unsigned char *) VIDEO(baseAddr) +
	VIDEO(rowBytes) * y + pixelBytes * x;
	
    drawWidth = MIN(width, VIDEO(width) - x);
    height = MIN(height, VIDEO(height) - y);
    while ( height-- ) {
        bcopy( data, vram, drawWidth * pixelBytes );
        vram += VIDEO(rowBytes);
        data += width * pixelBytes;
    }
}

//==========================================================================
// getVESAModeWithProperties
//
// Return the VESA mode that matches the properties specified.
// If a mode is not found, then return the "best" available mode.

unsigned short
__getVESAModeWithProperties( unsigned short     width,
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
	
	bzero( outModeInfo, sizeof(VBEModeInfoBlock) );
	
    // Get VBE controller info containing the list of supported modes.
	
    bzero( &vbeInfo, sizeof(VBEInfoBlock) );
    
    strlcpy( (char*)&vbeInfo, "VBE2", sizeof(VBEInfoBlock) );
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
        //bcopy( &modeInfo, outModeInfo, sizeof(modeInfo) );
        bcopy( &modeInfo, outModeInfo, sizeof(VBEModeInfoBlock) );
		
    }
	
    return matchedMode;
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
// setVESAGraphicsMode

int __setVESAGraphicsMode( unsigned short width, unsigned short height, unsigned char  bitsPerPixel)
{
    VBEModeInfoBlock  minfo;
    unsigned short    mode;
    unsigned short    vesaVersion;
    int               err = errFuncNotSupported;
	
    do {
        mode = __getVESAModeWithProperties( width, height, bitsPerPixel,
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
		//              (get_env(envgBootMode) & kBootModeSafe) == 0 )
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
// Simple decompressor for boot images encoded in RLE format.

char * __decodeRLE( const void * rleData, int rleBlocks, int outBytes )
{
    char *out, *cp;
	
    struct RLEBlock {
        unsigned char count;
        unsigned char value;
    } * bp = (struct RLEBlock *) rleData;
	
    out = cp = calloc(1, outBytes );
    if ( out == NULL ) return NULL;
	
    while ( rleBlocks-- )
    {
        memset( cp, bp->value, bp->count );
        cp += bp->count;
        bp++;
    }
	
    return out;
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

//==========================================================================
// drawColorRectangle

static void * stosl(void * dst, long val, long len)
{
    asm volatile ( "rep; stosl"
				  : "=c" (len), "=D" (dst)
				  : "0" (len), "1" (dst), "a" (val)
				  : "memory" );
	
    return dst;
}

void __drawColorRectangle( unsigned short x,
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
        mode = __getVESAModeWithProperties( cols, rows, bitsPerPixel,
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
// getNumberArrayFromProperty

int
__getNumberArrayFromProperty( const char *  propKey,
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

static int initGraphicsMode (void)
{
	unsigned long params[4];
	int           count;
	
	params[3] = 0;
	count = __getNumberArrayFromProperty( kGraphicsModeKey, params, 4 );
	
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
    
	return __setVESAGraphicsMode( params[0], params[1], params[2] );
}

//==========================================================================
// setVideoMode
//
// Set the video mode to VGA_TEXT_MODE or GRAPHICS_MODE.

void
__setVideoMode( int mode)
{
    unsigned long params[4];
    int           count;
    int           err = errSuccess;
	
    if ( mode == GRAPHICS_MODE )
    {
  		if ( (err=initGraphicsMode ()) == errSuccess ) {
			if (get_env(envgVerboseMode)) {
				// Tell the kernel to use text mode on a linear frame buffer display
				bootArgs->Video.v_display = FB_TEXT_MODE;
			} else {
				bootArgs->Video.v_display = GRAPHICS_MODE;
			}
		}
    }
	
    if ( (mode == VGA_TEXT_MODE) || (err != errSuccess) )
    {
        count = __getNumberArrayFromProperty( kTextModeKey, params, 2 );
        if ( count < 2 )
        {
            params[0] = 80;  // Default text mode is 80x25.
            params[1] = 25;
        }
		
		setVESATextMode( params[0], params[1], 4 );
        bootArgs->Video.v_display = VGA_TEXT_MODE;
    }
}