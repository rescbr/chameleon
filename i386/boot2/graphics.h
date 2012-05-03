/*
 *  graphics.h
 *  
 *
 *  Created by fassl on 22.12.08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */


#ifndef __BOOT_GRAPHICS_H
#define __BOOT_GRAPHICS_H

#include "bootstruct.h"
#include "vbe.h"

char * __decodeRLE( const void * rleData, int rleBlocks, int outBytes );

void __drawColorRectangle( unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned char  colorIndex );
void __drawDataRectangle( unsigned short  x,
                         unsigned short  y,
                         unsigned short  width,
                         unsigned short  height,
                         unsigned char * data );
int
__convertImage( unsigned short width,
               unsigned short height,
               const unsigned char *imageData,
               unsigned char **newImageData );


int __setVESAGraphicsMode( unsigned short width, unsigned short height, unsigned char  bitsPerPixel);

int __getNumberArrayFromProperty( const char *  propKey,
                                 unsigned long numbers[],
                                 unsigned long maxArrayCount );


unsigned short
__getVESAModeWithProperties( unsigned short     width,
                            unsigned short     height,
                            unsigned char      bitsPerPixel,
                            unsigned short     attributesSet,
                            unsigned short     attributesClear,
                            VBEModeInfoBlock * outModeInfo,
                            unsigned short *   vesaVersion );

void
__setVideoMode( int mode);

#endif /* !__BOOT_GRAPHICS_H */
