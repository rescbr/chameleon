
/* Graphic utility functions and data types
 * Prashant Vaibhav (C) 12/12/2008
 * Chameleon
 */
#include "boot.h"
#include "graphic_utils.h"
#include "graphics.h"
#include "vbe.h"
#include "gui.h"

#define VIDEO(x) (bootArgs->Video.v_ ## x)

#define MIN(x, y) ((x) < (y) ? (x) : (y))



int
convertImage( unsigned short width,
			 unsigned short height,
			 const unsigned char *imageData,
			 unsigned char **newImageData )
{
    int cnt;
    unsigned char *img = 0;
    unsigned short *img16;
    unsigned long *img32;
	
    switch ( VIDEO(depth) ) {
		case 16 :
			img16 = malloc(width * height * 2);
			if ( !img16 ) break;
			for (cnt = 0; cnt < (width * height); cnt++)
				img16[cnt] = lookUpCLUTIndex(imageData[cnt], 16);
			img = (unsigned char *)img16;
			break;
			
		case 32 :
			img32 = malloc(width * height * 4);
			if ( !img32 ) break;
			for (cnt = 0; cnt < (width * height); cnt++)
				img32[cnt] = lookUpCLUTIndex(imageData[cnt], 32);
			img = (unsigned char *)img32;
			break;
			
		default :
			img = malloc(width * height);
			bcopy(imageData, img, width * height);
			break;
    }
    *newImageData = img;
    return 0;
}


//==========================================================================
//
void 
printVBEModeInfo()
{
    VBEInfoBlock     vbeInfo;
    unsigned short * modePtr;
    VBEModeInfoBlock modeInfo;
    int              err;
    int              line;
	
  	bzero( &vbeInfo, sizeof(vbeInfo) );
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
		
        bzero( &modeInfo, sizeof(modeInfo) );
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



char *getVBEModeInfoString()
{
	VBEInfoBlock     vbeInfo;
    unsigned short * modePtr;
    VBEModeInfoBlock modeInfo;
    int              err;
	
  	bzero( &vbeInfo, sizeof(vbeInfo) );
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
		
        bzero( &modeInfo, sizeof(modeInfo) );
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
					}
					
				}				
			}
				break;
        }
        vram += VIDEO(rowBytes);
        data += width * 4;
    }
}

void drawCheckerBoard()
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



void getGraphicModeParams(unsigned long params[]) {
	
	params[3] = 0;
	
	VBEModeInfoBlock  minfo;
	
    unsigned short    vesaVersion;
    unsigned short    mode = modeEndOfList;

	getNumberArrayFromProperty( kGraphicsModeKey, params, 4);
	
	mode = getVESAModeWithProperties( params[0], params[1], params[2],
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

char *getVBEInfoString()
{
	VBEInfoBlock vbeInfo;
	int err, small;
	char *buff = malloc(sizeof(char)*256);
	if(!buff) return 0;
	
	bzero( &vbeInfo, sizeof(vbeInfo) );
	strcpy( (char*)&vbeInfo, "VBE2" );
	err = getVBEInfo( &vbeInfo );
	if (err != errSuccess)
		return 0;
	
	if ( strncmp( (char *)vbeInfo.VESASignature, "VESA", 4 ) )
		return 0;
	
	small = (vbeInfo.TotalMemory < 16);
	
	sprintf(buff, "VESA v%d.%d %d%s (%s)\n",
			vbeInfo.VESAVersion >> 8,
			vbeInfo.VESAVersion & 0xf,
			small ? (vbeInfo.TotalMemory * 64) : (vbeInfo.TotalMemory / 16),
			small ? "KB" : "MB",
			VBEDecodeFP(const char *, vbeInfo.OEMStringPtr) );
	
	return buff;
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
	for (x = 0; x < (p->height) * (p->width) ; x++) {
		tempB = (p->pixels[x]).ch.b;
        (p->pixels[x]).ch.b = (p->pixels[x]).ch.r;
        (p->pixels[x]).ch.r = tempB;
	}
}
