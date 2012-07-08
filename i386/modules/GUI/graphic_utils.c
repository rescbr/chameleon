
/* Graphic utility functions and data types
 * Prashant Vaibhav (C) 12/12/2008
 * Chameleon
 */
#include "libsaio.h"
#include "bootstruct.h"
#include "graphic_utils.h"
#include "appleClut8.h"
#include "vbe.h"
#include "gui.h"
#include "platform.h"

#define VIDEO(x) (((boot_args*)getBootArgs())->Video.v_ ## x)

#define MIN(x, y) ((x) < (y) ? (x) : (y))
static unsigned long lookUpCLUTIndex( unsigned char index,
                                     unsigned char depth );

static int
setVESATextMode( unsigned short cols,
				unsigned short rows,
				unsigned char  bitsPerPixel );

static void setupPalette( VBEPalette * p, const unsigned char * g );
static unsigned long lookUpCLUTIndex( unsigned char index,
                                     unsigned char depth );
static unsigned short
getVESAModeWithProperties( unsigned short     width,
                          unsigned short     height,
                          unsigned char      bitsPerPixel,
                          unsigned short     attributesSet,
                          unsigned short     attributesClear,
                          VBEModeInfoBlock * outModeInfo,
                          unsigned short *   vesaVersion );
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

int
convertImage( unsigned short width,
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
			img16 = malloc(width * height * 2);
			if ( !img16 ) return 1;
			for (cnt = 0; cnt < (width * height); cnt++)
				img16[cnt] = lookUpCLUTIndex(imageData[cnt], 16);
			img = (unsigned char *)img16;
            *newImageData = img;
			break;
        }
		case 32 :
        {
			img32 = malloc(width * height * 4);
			if ( !img32 ) return 1;
			for (cnt = 0; cnt < (width * height); cnt++)
				img32[cnt] = lookUpCLUTIndex(imageData[cnt], 32);
			img = (unsigned char *)img32;
            *newImageData = img;
			break;
        }
		default :
        {
			img = malloc(width * height);
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

void drawDataRectangle( unsigned short  x,
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
//
void 
printVBEModeInfo(void)
{
    VBEInfoBlock     vbeInfo;
    unsigned short * modePtr;
    VBEModeInfoBlock modeInfo;
    int              err;
    int              line;
	
  	//bzero( &vbeInfo, sizeof(vbeInfo) );
    bzero( &vbeInfo, sizeof(VBEInfoBlock) );

    strcpy( (char*)&vbeInfo, "VBE2" );
    err = getVBEInfo( &vbeInfo );
    if ( err != errSuccess )
        return;
	
    line = 0;
	
    // Activate and clear page 1
    setActiveDisplayPage(1);
    clearScreenRows(0, 24);
    setCursorPosition( 0, 0, 1 );
	
	printf("Video modes supported:\n", VBEDecodeFP(const char *, vbeInfo.OEMStringPtr));
	
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
		
        printf("Mode %x: %dx%dx%d mm:%d attr:%x\n",
               *modePtr, modeInfo.XResolution, modeInfo.YResolution,
               modeInfo.BitsPerPixel, modeInfo.MemoryModel,
               modeInfo.ModeAttributes);
		
        if (line++ >= 20) {
            pause();
            line = 0;
            clearScreenRows(0, 24);
            setCursorPosition( 0, 0, 1 );
        }
    }    
    if (line != 0) {
        pause();
    }
    setActiveDisplayPage(0);
}



char *getVBEModeInfoString(void)
{
	VBEInfoBlock     vbeInfo;
    unsigned short * modePtr;
    VBEModeInfoBlock modeInfo;
    int              err;
	
  	//bzero( &vbeInfo, sizeof(vbeInfo) );
    bzero( &vbeInfo, sizeof(VBEInfoBlock) );

    strcpy( (char*)&vbeInfo, "VBE2" );
    err = getVBEInfo( &vbeInfo );
    if ( err != errSuccess )
        return 0;
	
	char *buff=malloc(sizeof(char)*3072);
	if(!buff) return 0;
	
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
		
        sprintf(buff+strlen(buff), "Mode %x: %dx%dx%d mm:%d attr:%x\n",
				*modePtr, modeInfo.XResolution, modeInfo.YResolution,
				modeInfo.BitsPerPixel, modeInfo.MemoryModel,
				modeInfo.ModeAttributes);
		
    }   
	return buff;
}


void blendImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
				uint8_t *data)
{
    uint16_t drawWidth;
    uint8_t *vram = (uint8_t *) VIDEO(baseAddr) + VIDEO(rowBytes) * y + 4 * x;
	
    drawWidth = MIN(width, VIDEO(width) - x);
    height = MIN(height, VIDEO(height) - y);
    while (height--) {
		switch (VIDEO (depth))
		{
			case 32: /* Optimized version*/
			{
				uint32_t s; uint32_t* d; // Source (img) and destination (bkgd) pixels
				uint32_t a; // Alpha
				uint32_t dstrb, dstg, srcrb, srcg, drb, dg, rb, g, tempB; // Intermediate variables
				uint16_t pos;
				
				for (pos = 0; pos < drawWidth * 4; pos += 4) {
					// Fast pseudo-vector alpha blending, adapted from: http://www.stereopsis.com/doubleblend.html
					s = *((uint32_t*) (data + pos));
					d =   (uint32_t*) (vram + pos);
					
					// Flip B and R in source
					// TODO: use XCHG and inline assembly to do this in a faster, saner way
					tempB = (s & 0xFF0000);                     // save B
					s = (s & 0xFF00FFFF) | ((s & 0xFF) << 16);  // put R in B
					s = (s & 0xFFFFFF00) | (tempB >> 16);       // put B in R
					
					a = (s >> 24) + 1;
					
					dstrb = *d & 0xFF00FF; dstg  = *d & 0xFF00;
					srcrb =  s & 0xFF00FF; srcg  =  s & 0xFF00;
					
					drb = srcrb - dstrb;
					dg  =  srcg - dstg;
					drb *= a; dg *= a;
					drb >>= 8; dg  >>= 8;
					
					rb = (drb + dstrb) & 0xFF00FF;
					g  = (dg  + dstg) & 0xFF00;
					
					*d = rb | g;
				}
			}
				break;
				
			default: /*Universal version*/
			{
				uint32_t s;  
				uint32_t a; // Alpha
				uint32_t dr, dg, db, sr, sg, sb; // Intermediate variables
				uint16_t pos;
				int bpp = (VIDEO (depth) + 7)/8;
				
				for (pos = 0; pos < drawWidth; pos ++) {
					// Fast pseudo-vector alpha blending, adapted from: http://www.stereopsis.com/doubleblend.html
					s = *((uint32_t*) (data + 4*pos));
					
					sb = (s & 0xFF0000) >> 16;
					sg = (s & 0xFF00) >> 8;
					sr = (s & 0xFF);
					
					a = (s >> 24) + 1;
					
					switch (VIDEO (depth))
					{
						case 24:
							db = ((*(uint32_t *)(vram + bpp*pos))&0xff);
							dg = ((*(uint32_t *)(vram + bpp*pos))&0xff00)>>8;
							dr = ((*(uint32_t *)(vram + bpp*pos))&0xff0000)>>16;
							break;
						case 16://16-bit seems to be 15-bit
							/*							db = ((*(uint16_t *)(vram + bpp*pos))&0x1f)<<3;
							 dg = ((*(uint16_t *)(vram + bpp*pos))&0x07e0)>>3;
							 dr = ((*(uint16_t *)(vram + bpp*pos))&0xf800)>>8;
							 break;							*/
						case 15:
							db = ((*(uint16_t *)(vram + bpp*pos))&0x1f)<<3;
							dg = ((*(uint16_t *)(vram + bpp*pos))&0x03e0)>>2;
							dr = ((*(uint16_t *)(vram + bpp*pos))&0x7c00)>>7;
							break;		
						default:
							return;
					}
					
					dr = (((sr - dr) * a) >> 8) + dr;
					dg = (((sg - dg) * a) >> 8) + dg;
					db = (((sb - db) * a) >> 8) + db;
					switch (VIDEO (depth))
					{
						case 24:
							*(uint32_t *)(vram + bpp*pos) = (*(uint32_t *)(vram + bpp*pos) &0xff000000)
							| (db&0xff) | ((dg&0xff)<<8) | ((dr&0xff)<<16);
							break;
						case 16:
							//							*(uint16_t *)(vram + bpp*pos) = ((db&0xf8)>>3) | ((dg&0xfc)<<3) | ((dr&0xf8)<<8);
							//							break;							
						case 15:
							*(uint16_t *)(vram + bpp*pos) = ((db&0xf8)>>3) | ((dg&0xf8)<<2) | ((dr&0xf8)<<7);
							break;	
						default:
							break;
					}
					
				}				
			}
				break;
        }
        vram += VIDEO(rowBytes);
        data += width * 4;
    }
}

void drawCheckerBoard(void)
{
    uint32_t *vram = (uint32_t *) VIDEO(baseAddr);
    uint16_t x, y;
    uint8_t color;
	
    for (y = 0; y < VIDEO(height); y++, vram += VIDEO(width)) {
        for (x = 0; x < VIDEO(width); x++) {
            color = 204 + 51 * (((x / 8) % 2) == ((y / 8) % 2));
            vram[x] = (color << 16) | (color << 8) | color;
        }
    }
}

//==========================================================================
// getNumberArrayFromProperty

int
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

//==========================================================================
// Simple decompressor for boot images encoded in RLE format.

char * decodeRLE( const void * rleData, int rleBlocks, int outBytes )
{
    char *out, *cp;
	
    struct RLEBlock {
        unsigned char count;
        unsigned char value;
    } * bp = (struct RLEBlock *) rleData;
	
    out = cp = malloc( outBytes );
    if ( out == NULL ) return NULL;
	
    while ( rleBlocks-- )
    {
        memset( cp, bp->value, bp->count );
        cp += bp->count;
        bp++;
    }
	
    return out;
}

static void * stosl(void * dst, long val, long len)
{
    asm volatile ( "rep; stosl"
				  : "=c" (len), "=D" (dst)
				  : "0" (len), "1" (dst), "a" (val)
				  : "memory" );
	
    return dst;
}

void drawColorRectangle( unsigned short x,
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
	
    minfo.XResolution = 0;
    minfo.YResolution = 0;
    
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

void
setVideoMode( int mode)
{
    unsigned long params[4];
    int           count;
    int           err = errSuccess;
	
    if ( mode == GRAPHICS_MODE )
    {
  		if ( (err=GUI_initGraphicsMode ()) == errSuccess ) {
			if (get_env(envgVerboseMode)) {
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

int setVESAGraphicsMode( unsigned short width, unsigned short height, unsigned char  bitsPerPixel)
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

void getGraphicModeParams(unsigned long params[]) {
	
	params[3] = 0;
	
	VBEModeInfoBlock  minfo;
	
    unsigned short    vesaVersion;
    //unsigned short    mode = modeEndOfList;
	
	getNumberArrayFromProperty( kGraphicsModeKey, params, 4);
	
	/* mode = */getVESAModeWithProperties( params[0], params[1], params[2],
									 maColorModeBit             |
									 maModeIsSupportedBit       |
									 maGraphicsModeBit          |
									 maLinearFrameBufferAvailBit,
									 0,
									 &minfo, &vesaVersion );
	
	params[0] = minfo.XResolution;
	params[1] = minfo.YResolution;
	params[2] = 32;
}


//==========================================================================
// getVBEInfoString

char *getVBEInfoString(void)
{
	VBEInfoBlock vbeInfo;
	int err, small;
	char *buff = malloc(sizeof(char)*256);
	if(!buff) return 0;
	
	//bzero( &vbeInfo, sizeof(vbeInfo) );
    bzero( &vbeInfo, sizeof(VBEInfoBlock) );

	strcpy( (char*)&vbeInfo, "VBE2" );
	err = getVBEInfo( &vbeInfo );
	if (err != errSuccess)
		goto error;
	
	if ( strncmp( (char *)vbeInfo.VESASignature, "VESA", 4 ) )
		goto error;
	
	small = (vbeInfo.TotalMemory < 16);
	
	sprintf(buff, "VESA v%d.%d %d%s (%s)\n",
			vbeInfo.VESAVersion >> 8,
			vbeInfo.VESAVersion & 0xf,
			small ? (vbeInfo.TotalMemory * 64) : (vbeInfo.TotalMemory / 16),
			small ? "KB" : "MB",
			VBEDecodeFP(const char *, vbeInfo.OEMStringPtr) );
	
	return buff;
error:
    free(buff);
    return 0;
}

void blend( const pixmap_t *blendThis,            // Source image
		   pixmap_t *blendInto,                  // Dest image
		   const position_t position)          // Where to place the source image
{
    uint16_t sx, sy, dx, dy;
    uint32_t dstrb, dstag, srcrb, srcag, drb, dag, rb, ag, alpha;
	
	uint16_t width = (blendThis->width + position.x < blendInto->width) ? blendThis->width: blendInto->width-position.x;
	uint16_t height = (blendThis->height + position.y < blendInto->height) ? blendThis->height: blendInto->height-position.y;
	
	for (dy = position.y, sy = 0; sy < height; dy++, sy++) 
	{
        for (dx = position.x, sx = 0; sx < width; dx++, sx++) 
		{
            alpha = (pixel(blendThis, sx, sy).ch.a);
			
			/* Skip blending for fully transparent pixel */
			if (alpha == 0) continue;
			
			/* For fully opaque pixel, there is no need to interpolate */
			if (alpha == 255)
			{
				pixel(blendInto, dx, dy).value = pixel(blendThis, sx, sy).value;
				continue;
			}
			
			/* For semi-transparent pixels, do a full blend */
			//alpha++
			/* This is needed to spread the alpha over [0..256] instead of [0..255]
			 Boundary conditions were handled above */
            dstrb =  pixel(blendInto, dx, dy).value       & 0xFF00FF;
            dstag = (pixel(blendInto, dx, dy).value >> 8) & 0xFF00FF;
            srcrb =  pixel(blendThis, sx, sy).value       & 0xFF00FF;
            srcag = (pixel(blendThis, sx, sy).value >> 8) & 0xFF00FF;
            drb   = srcrb - dstrb;
            dag   = srcag - dstag;
            drb *= alpha; dag *= alpha;
            drb >>= 8; dag >>= 8;
            rb =  (drb + dstrb)       & 0x00FF00FF;
            ag = ((dag + dstag) << 8) & 0xFF00FF00;
            pixel(blendInto, dx, dy).value = (rb | ag);
        }
    }
}

position_t centeredIn( const pixmap_t *background, const pixmap_t *toCenter )
{
    position_t centered;
    centered.x = ( background->width  - toCenter->width  ) / 2;
    centered.y = ( background->height - toCenter->height ) / 2;
    return centered;
}

position_t centeredAt( const pixmap_t *pixmap, const position_t center )
{
    position_t topleft;
    topleft.x = center.x - (pixmap->width  / 2);
    topleft.y = center.y - (pixmap->height / 2);
    return topleft;
}

position_t pos(const uint16_t x, const uint16_t y) { position_t p; p.x = x; p.y = y; return p; }

void flipRB(pixmap_t *p)
{
	//if(testForQemu()) return;
	
	uint32_t x;
    register uint8_t tempB;
	for (x = 0; x < (unsigned long)(p->height) * (p->width) ; x++) {
		tempB = (p->pixels[x]).ch.b;
        (p->pixels[x]).ch.b = (p->pixels[x]).ch.r;
        (p->pixels[x]).ch.r = tempB;
	}
}
