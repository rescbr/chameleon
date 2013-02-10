/*
 * libeg/image.c
 * Image handling functions
 *
 * Copyright (c) 2006 Christoph Pfisterer
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
 *  * Neither the name of Christoph Pfisterer nor the names of the
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
#include <sys/stat.h>
#include <pexpert/i386/boot.h>
#include "memcpy.h" // a fast memcpy
#include <string.h>

#define MAX_FILE_SIZE (1024*1024*1024)


//
// Basic image handling
//

EG_IMAGE * egCreateImage(IN UINTN Width, IN UINTN Height, IN BOOLEAN HasAlpha)
{
    EG_IMAGE        *NewImage;
    
    NewImage = (EG_IMAGE *) AllocateZeroPool(sizeof(EG_IMAGE));
    if (NewImage == NULL)
        return NULL;
    NewImage->PixelData = (EG_PIXEL *) AllocateZeroPool(Width * Height * sizeof(EG_PIXEL));
    if (NewImage->PixelData == NULL) {
        FreePool(NewImage);
        return NULL;
    }
    
    NewImage->Width = Width;
    NewImage->Height = Height;
    NewImage->HasAlpha = HasAlpha;
    NewImage->NeedFlip = TRUE;
    return NewImage;
}

EG_IMAGE * egCreateFilledImage(IN UINTN Width, IN UINTN Height, IN BOOLEAN HasAlpha, IN EG_PIXEL *Color)
{
    EG_IMAGE        *NewImage;
    
    NewImage = egCreateImage(Width, Height, HasAlpha);
    if (NewImage == NULL)
        return NULL;
    
    egFillImage(NewImage, Color);
    return NewImage;
}

VOID egCopyImage(IN OUT EG_IMAGE *Image1, IN EG_IMAGE *Image2)
{    
    if (Image1 == NULL || Image2 == NULL)
        return ;
    
    CopyMem(Image1->PixelData, Image2->PixelData, Image2->Width * Image2->Height * sizeof(EG_PIXEL));
    
    Image1->Width = Image2->Width;
    Image1->Height = Image2->Height;
    Image1->HasAlpha = Image2->HasAlpha;
    Image1->NeedFlip = Image2->NeedFlip;
    
    return ;
}

EG_IMAGE * egNewImageFromImage(IN EG_IMAGE *Image)
{
    EG_IMAGE        *NewImage;
    
    NewImage = egCreateImage(Image->Width, Image->Height, Image->HasAlpha);
    if (NewImage == NULL)
        return NULL;
    
    egCopyImage(NewImage, Image);
    
    return NewImage;
}

EG_IMAGE * egCreateImageFromData(IN EG_PIXEL *PixelData, IN UINTN Width, IN UINTN Height, IN BOOLEAN HasAlpha, BOOLEAN NeedFlip)
{
    EG_IMAGE        *NewImage;
    
    NewImage = egCreateImage(Width, Height, HasAlpha);
    if (NewImage == NULL)
        return NULL;
    
    CopyMem(NewImage->PixelData, PixelData, Width * Height * sizeof(EG_PIXEL));
    
    NewImage->NeedFlip = NeedFlip;
    
    return NewImage;
}

VOID egCopyScaledImage(IN OUT EG_IMAGE *CompImage, IN EG_IMAGE *Image)
{
#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif
    INTN       x,  x1, y, y1;
#if UNUSED
	INTN       x0, x2;
	INTN       y0, y2;
#endif
    UINTN     RatioH, RatioW, Ratio;
    EG_PIXEL  *Dest;
        
    RatioW = (CompImage->Width << 4) / Image->Width;
    RatioH = (CompImage->Height << 4) / Image->Height;
    
    Ratio = MAX(RatioH,RatioW);
        
    Dest = CompImage->PixelData;
    for (y = 0; y < CompImage->Height; y++) {
        y1 = (y<<4) / Ratio;
#if UNUSED
        y0 = ((y1 > 0)?(y1-1):y1)*Image->Width;
        y2 = ((y1 < Image->Height)?(y1+1):y1)*Image->Width;
#endif
        y1 *= Image->Width;
        for (x = 0; x < CompImage->Width; x++) {
            x1 = (x<<4) / Ratio;
#if UNUSED
            x0 = (x1 > 0)?(x1-1):x1;
            x2 = (x1 < Image->Width)?(x1+1):x1;
#endif
            //TODO - make sum of 5 points
            *Dest++ = Image->PixelData[x1+y1];
        }
    }   
}


EG_IMAGE * egNewScaledImage(IN EG_IMAGE *Image, IN UINTN NewW, IN UINTN NewH)
{

    EG_IMAGE        *NewImage;

    NewImage = egCreateImage(NewW, NewH, Image->HasAlpha);
    if (NewImage == NULL)
        return NULL;
    
    egCopyScaledImage(NewImage,Image);    
    
    return NewImage;
}

VOID egFreeImage(IN EG_IMAGE *Image)
{
    if (Image != NULL) {
        if (Image->PixelData != NULL)
            FreePool(Image->PixelData);
        FreePool(Image);
    }
}

//
// Basic file operations
//

EFI_STATUS egLoadFile(IN CHAR16 *FileName,
                      OUT UINT8 **FileData, OUT UINTN *FileDataLength)
{
    FILE				*FileHandle;
    UINT64              ReadSize;
    UINTN               BufferSize;
    UINT8               *Buffer;
	CHAR8				FileName8[1024];
	struct				stat buf;

	UnicodeStrToAsciiStr(FileName, FileName8);

    FileHandle = fopen(FileName8, "rb");
    if (!FileHandle)
        return EFI_NOT_FOUND;    
	printf("file opened\n");

    fstat(fileno(FileHandle), &buf);	
    ReadSize = buf.st_size;	
	if (!ReadSize)
	{
		fclose(FileHandle);
		return EFI_LOAD_ERROR;
	} 
	else if (ReadSize > MAX_FILE_SIZE)
        ReadSize = MAX_FILE_SIZE;
	printf("file size = %d\n", (INTN)ReadSize);

    BufferSize = (UINTN)ReadSize;   // was limited to 1 GB above, so this is safe
    Buffer = (UINT8 *) AllocatePool(BufferSize);
    if (Buffer == NULL) {
        fclose(FileHandle);
        return EFI_OUT_OF_RESOURCES;
    }
	printf("AllocatePool = %p\n", (VOID*)Buffer);

	ReadSize = fread(Buffer, 1, BufferSize, FileHandle);
    fclose(FileHandle);
    if (BufferSize != ReadSize) {
        FreePool(Buffer);
        return EFI_BAD_BUFFER_SIZE;
    }
	printf("Buffer read successfuly\n");

    *FileData = Buffer;
    *FileDataLength = BufferSize;
    return EFI_SUCCESS;
}

//
// Loading images from files and embedded data
//

static CHAR16 * egFindExtension(IN CHAR16 *FileName)
{
    INTN i;
    
    for (i = StrLen(FileName); i >= 0; i--) {
        if (FileName[i] == '.')
            return FileName + i + 1;
        if (FileName[i] == '/' || FileName[i] == '\\')
            break;
    }
    return FileName + StrLen(FileName);
}

static EG_IMAGE * egDecodeAny(IN UINT8 *FileData, IN UINTN FileDataLength,
                              IN CHAR16 *Format, IN UINTN IconSize, IN BOOLEAN WantAlpha)
{
    EG_DECODE_FUNC  DecodeFunc;
    EG_IMAGE        *NewImage;
    
    // dispatch by extension
    DecodeFunc = NULL;
    if (StrCmp(Format, L"BMP") == 0)
        DecodeFunc = egDecodeBMP;
    else if (StrCmp(Format, L"ICNS") == 0)
        DecodeFunc = egDecodeICNS;
    else if (StrCmp(Format, L"PNG" ) == 0)
        DecodeFunc = egDecodePNG;
    
    if (DecodeFunc == NULL)
        return NULL;
    
    // decode it
    NewImage = DecodeFunc(FileData, FileDataLength, IconSize, WantAlpha);
    
    return NewImage;
}

EG_IMAGE * egLoadImage(IN CHAR16 *FilePath, IN BOOLEAN WantAlpha)
{
    EFI_STATUS      Status;
    UINT8           *FileData;
    UINTN           FileDataLength;
    EG_IMAGE        *NewImage;
    
    if (FilePath == NULL)
        return NULL;
    
    // load file
	printf("load file\n");
    Status = egLoadFile(FilePath, &FileData, &FileDataLength);
    if (EFI_ERROR(Status))
        return NULL;
    
    // decode it
	printf("decode it\n");
    NewImage = egDecodeAny(FileData, FileDataLength, egFindExtension(FilePath), 128, WantAlpha);
    FreePool(FileData);
    
    return NewImage;
}

EG_IMAGE * egLoadIcon(IN CHAR16 *FilePath, IN UINTN IconSize)
{
    EFI_STATUS      Status;
    UINT8           *FileData;
    UINTN           FileDataLength;
    EG_IMAGE        *NewImage;
    
    if (FilePath == NULL)
        return NULL;
    
    // load file
    Status = egLoadFile(FilePath, &FileData, &FileDataLength);
    if (EFI_ERROR(Status))
        return NULL;
    
    // decode it
    NewImage = egDecodeAny(FileData, FileDataLength, egFindExtension(FilePath), IconSize, TRUE);
    FreePool(FileData);
    
    return NewImage;
}

EG_IMAGE * egDecodeImage(IN UINT8 *FileData, IN UINTN FileDataLength, IN CHAR16 *Format, IN BOOLEAN WantAlpha)
{
    return egDecodeAny(FileData, FileDataLength, Format, 128, WantAlpha);
}

EG_IMAGE * egPrepareEmbeddedImage(IN EG_EMBEDDED_IMAGE *EmbeddedImage, IN BOOLEAN WantAlpha)
{
    EG_IMAGE            *NewImage;
    UINT8               *CompData;
    UINTN               CompLen;
    UINTN               PixelCount;
    
    // sanity check
    if (EmbeddedImage->PixelMode > EG_MAX_EIPIXELMODE ||
        (EmbeddedImage->CompressMode != EG_EICOMPMODE_NONE && EmbeddedImage->CompressMode != EG_EICOMPMODE_RLE))
        return NULL;
    
    // allocate image structure and pixel buffer
    NewImage = egCreateImage(EmbeddedImage->Width, EmbeddedImage->Height, WantAlpha);
    if (NewImage == NULL)
        return NULL;
    
    CompData = (UINT8 *)EmbeddedImage->Data;   // drop const
    CompLen  = EmbeddedImage->DataLength;
    PixelCount = EmbeddedImage->Width * EmbeddedImage->Height;
    
    // FUTURE: for EG_EICOMPMODE_EFICOMPRESS, decompress whole data block here
    
    if (EmbeddedImage->PixelMode == EG_EIPIXELMODE_GRAY ||
        EmbeddedImage->PixelMode == EG_EIPIXELMODE_GRAY_ALPHA) {
        
        // copy grayscale plane and expand
        if (EmbeddedImage->CompressMode == EG_EICOMPMODE_RLE) {
            egDecompressIcnsRLE(&CompData, &CompLen, PLPTR(NewImage, r), PixelCount);
        } else {
            egInsertPlane(CompData, PLPTR(NewImage, r), PixelCount);
            CompData += PixelCount;
        }
        egCopyPlane(PLPTR(NewImage, r), PLPTR(NewImage, g), PixelCount);
        egCopyPlane(PLPTR(NewImage, r), PLPTR(NewImage, b), PixelCount);
        
    } else if (EmbeddedImage->PixelMode == EG_EIPIXELMODE_COLOR ||
               EmbeddedImage->PixelMode == EG_EIPIXELMODE_COLOR_ALPHA) {
        
        // copy color planes
        if (EmbeddedImage->CompressMode == EG_EICOMPMODE_RLE) {
            egDecompressIcnsRLE(&CompData, &CompLen, PLPTR(NewImage, r), PixelCount);
            egDecompressIcnsRLE(&CompData, &CompLen, PLPTR(NewImage, g), PixelCount);
            egDecompressIcnsRLE(&CompData, &CompLen, PLPTR(NewImage, b), PixelCount);
        } else {
            egInsertPlane(CompData, PLPTR(NewImage, r), PixelCount);
            CompData += PixelCount;
            egInsertPlane(CompData, PLPTR(NewImage, g), PixelCount);
            CompData += PixelCount;
            egInsertPlane(CompData, PLPTR(NewImage, b), PixelCount);
            CompData += PixelCount;
        }
        
    } else {
        
        // set color planes to black
        egSetPlane(PLPTR(NewImage, r), 0, PixelCount);
        egSetPlane(PLPTR(NewImage, g), 0, PixelCount);
        egSetPlane(PLPTR(NewImage, b), 0, PixelCount);
        
    }
    
    if (WantAlpha && (EmbeddedImage->PixelMode == EG_EIPIXELMODE_GRAY_ALPHA ||
                      EmbeddedImage->PixelMode == EG_EIPIXELMODE_COLOR_ALPHA ||
                      EmbeddedImage->PixelMode == EG_EIPIXELMODE_ALPHA)) {
        
        // copy alpha plane
        if (EmbeddedImage->CompressMode == EG_EICOMPMODE_RLE) {
            egDecompressIcnsRLE(&CompData, &CompLen, PLPTR(NewImage, a), PixelCount);
        } else {
            egInsertPlane(CompData, PLPTR(NewImage, a), PixelCount);
            CompData += PixelCount;
        }
        
    } else {
        egSetPlane(PLPTR(NewImage, a), WantAlpha ? 255 : 0, PixelCount);
    }
    
    return NewImage;
}

//
// Compositing
//

VOID egRestrictImageArea(IN EG_IMAGE *Image,
															 IN UINTN AreaPosX, IN UINTN AreaPosY,
															 IN OUT UINTN *AreaWidth, IN OUT UINTN *AreaHeight)
{
    if (AreaPosX >= Image->Width || AreaPosY >= Image->Height) {
        // out of bounds, operation has no effect
        *AreaWidth  = 0;
        *AreaHeight = 0;
    } else {
        // calculate affected area
        if (*AreaWidth > Image->Width - AreaPosX)
            *AreaWidth = Image->Width - AreaPosX;
        if (*AreaHeight > Image->Height - AreaPosY)
            *AreaHeight = Image->Height - AreaPosY;
    }
}

VOID egFillImage(IN OUT EG_IMAGE *CompImage, IN EG_PIXEL *Color)
{
    UINTN       i;
    EG_PIXEL    FillColor;
    EG_PIXEL    *PixelPtr;
    
    FillColor = *Color;
    if (!CompImage->HasAlpha)
        FillColor.a = 0;
    
    PixelPtr = CompImage->PixelData;
    for (i = 0; i < CompImage->Width * CompImage->Height; i++, PixelPtr++)
        *PixelPtr = FillColor;
}

VOID egFillImageArea(IN OUT EG_IMAGE *CompImage,
														 IN UINTN AreaPosX, IN UINTN AreaPosY,
														 IN UINTN AreaWidth, IN UINTN AreaHeight,
														 IN EG_PIXEL *Color)
{
    UINTN       x, y;
    EG_PIXEL    FillColor;
    EG_PIXEL    *PixelPtr;
    EG_PIXEL    *PixelBasePtr;
    
    egRestrictImageArea(CompImage, AreaPosX, AreaPosY, &AreaWidth, &AreaHeight);
    
    if (AreaWidth > 0) {
        FillColor = *Color;
        if (!CompImage->HasAlpha)
            FillColor.a = 0;
        
        PixelBasePtr = CompImage->PixelData + AreaPosY * CompImage->Width + AreaPosX;
        for (y = 0; y < AreaHeight; y++) {
            PixelPtr = PixelBasePtr;
            for (x = 0; x < AreaWidth; x++, PixelPtr++)
                *PixelPtr = FillColor;
            PixelBasePtr += CompImage->Width;
        }
    }
}

VOID egRawCopy(IN OUT EG_PIXEL *CompBasePtr, IN EG_PIXEL *TopBasePtr,
												   IN UINTN Width, IN UINTN Height,
												   IN UINTN CompLineOffset, IN UINTN TopLineOffset)
{
    UINTN       x, y;
    EG_PIXEL    *TopPtr, *CompPtr;
    
    for (y = 0; y < Height; y++) {
        TopPtr = TopBasePtr;
        CompPtr = CompBasePtr;
        for (x = 0; x < Width; x++) {
            *CompPtr = *TopPtr;
            TopPtr++, CompPtr++;
        }
        TopBasePtr += TopLineOffset;
        CompBasePtr += CompLineOffset;
    }
}

VOID egRawCompose(IN OUT EG_PIXEL *CompBasePtr, IN EG_PIXEL *TopBasePtr,
													 IN UINTN Width, IN UINTN Height,
													 IN UINTN CompLineOffset, IN UINTN TopLineOffset)
{
    UINTN       x, y;
    EG_PIXEL    *TopPtr, *CompPtr;
    UINTN       Alpha;
    UINTN       RevAlpha;
    UINTN       Temp;
    
    for (y = 0; y < Height; y++) {
        TopPtr = TopBasePtr;
        CompPtr = CompBasePtr;
        for (x = 0; x < Width; x++) {
            Alpha = TopPtr->a;
            RevAlpha = 255 - Alpha;
            Temp = (UINTN)CompPtr->b * RevAlpha + (UINTN)TopPtr->b * Alpha + 0x80;
            CompPtr->b = (Temp + (Temp >> 8)) >> 8;
            Temp = (UINTN)CompPtr->g * RevAlpha + (UINTN)TopPtr->g * Alpha + 0x80;
            CompPtr->g = (Temp + (Temp >> 8)) >> 8;
            Temp = (UINTN)CompPtr->r * RevAlpha + (UINTN)TopPtr->r * Alpha + 0x80;
            CompPtr->r = (Temp + (Temp >> 8)) >> 8;
            /*
			 CompPtr->b = ((UINTN)CompPtr->b * RevAlpha + (UINTN)TopPtr->b * Alpha) / 255;
			 CompPtr->g = ((UINTN)CompPtr->g * RevAlpha + (UINTN)TopPtr->g * Alpha) / 255;
			 CompPtr->r = ((UINTN)CompPtr->r * RevAlpha + (UINTN)TopPtr->r * Alpha) / 255;
			 */
            TopPtr++, CompPtr++;
        }
        TopBasePtr += TopLineOffset;
        CompBasePtr += CompLineOffset;
    }
}

VOID egComposeImage(IN OUT EG_IMAGE *CompImage, IN EG_IMAGE *TopImage, IN UINTN PosX, IN UINTN PosY)
{
    UINTN       CompWidth, CompHeight;
    
    CompWidth  = TopImage->Width;
    CompHeight = TopImage->Height;
    egRestrictImageArea(CompImage, PosX, PosY, &CompWidth, &CompHeight);
    
    // compose
    if (CompWidth > 0) {
        if (CompImage->HasAlpha) {
            CompImage->HasAlpha = FALSE;
            egSetPlane(PLPTR(CompImage, a), 0, CompImage->Width * CompImage->Height);
        }
        
        if (TopImage->HasAlpha)
            egRawCompose(CompImage->PixelData + PosY * CompImage->Width + PosX, TopImage->PixelData,
                         CompWidth, CompHeight, CompImage->Width, TopImage->Width);
        else
            egRawCopy(CompImage->PixelData + PosY * CompImage->Width + PosX, TopImage->PixelData,
                      CompWidth, CompHeight, CompImage->Width, TopImage->Width);
    }
}

BOOLEAN egComposeImageStretched(IN OUT EG_IMAGE *CompImage, IN EG_IMAGE *TopImage)
{
#define MARGIN 0
    UINT8		*pStretchedImage;
    UINT8		*pImg;
	UINT8		*pbImage , *pDiData;
    UINTN       cImgChannels;    
	UINTN		deltaNewSize, deltaWinSize;
	UINTN		cxImgPos, cyImgPos;
	UINTN		xOld, yOld;
    UINTN		xNew, yNew;
	UINTN		xImg, yImg;
    UINTN		xWin, yWin;
	UINT8		r, g, b, a;
	UINTN		cxImgSize, cyImgSize;
	UINT8		*src, *dst;
	const INT32 cDIChannels = 3;
    UINT16 wImgRowBytes;
    UINT16 wDIRowBytes;
	
	
	deltaWinSize = MAX(CompImage->Width,CompImage->Height); 
	cxImgSize = TopImage->Width;
	cyImgSize = TopImage->Height;
	pbImage	  = (UINT8*)TopImage->PixelData;
	pDiData   = (UINT8*)CompImage->PixelData;	
	
	{
        deltaNewSize = deltaWinSize - 2 * MARGIN;
        cImgChannels = (TopImage->HasAlpha) ? 4 : 3;		
        
        
        // stretch the image to it's window determined size
        
        // the following two are the same, but the first has side-effects
        // because of rounding
        //      if ((deltaNewSize / deltaNewSize) > (cyImgSize / cxImgSize))
        if ((deltaNewSize * cxImgSize) > (cyImgSize * deltaNewSize))
        {
            deltaNewSize = deltaNewSize * cyImgSize / cxImgSize;
            cxImgPos = MARGIN;
            cyImgPos = (deltaWinSize - deltaNewSize) / 2;
        }
        else
        {
            deltaNewSize = deltaNewSize * cxImgSize / cyImgSize;
            cyImgPos = MARGIN;
            cxImgPos = (deltaWinSize - deltaNewSize) / 2;
        }
        
        pStretchedImage = AllocatePool (cImgChannels * deltaNewSize * deltaNewSize);
        if (!pStretchedImage)
        {
            return FALSE;
        }
		pImg = pStretchedImage;
        
        for (yNew = 0; yNew < deltaNewSize; yNew++)
        {
            yOld = yNew * cyImgSize / deltaNewSize;
            for (xNew = 0; xNew < deltaNewSize; xNew++)
            {
                xOld = xNew * cxImgSize / deltaNewSize;
                
                r = *(pbImage + cImgChannels * ((yOld * cxImgSize) + xOld) + 0);
                g = *(pbImage + cImgChannels * ((yOld * cxImgSize) + xOld) + 1);
                b = *(pbImage + cImgChannels * ((yOld * cxImgSize) + xOld) + 2);
                *pImg++ = r;
                *pImg++ = g;
                *pImg++ = b;
                if (cImgChannels == 4)
                {
                    a = *(pbImage + cImgChannels * ((yOld * cxImgSize) + xOld)
                          + 3);
                    *pImg++ = a;
                }
            }
        }
		
		// calculate row-bytes
        
        wImgRowBytes = cImgChannels * deltaNewSize;
        wDIRowBytes = (unsigned short) ((cDIChannels * deltaWinSize + 3L) >> 2) << 2;
        
        // copy image to screen
		
        for (yImg = 0, yWin = cyImgPos; yImg < deltaNewSize; yImg++, yWin++)
        {
            if (yWin >= CompImage->Width - cyImgPos)
                break;
            src = pStretchedImage + yImg * wImgRowBytes;
            dst = pDiData + yWin * wDIRowBytes + cxImgPos * cDIChannels;
            
            for (xImg = 0, xWin = cxImgPos; xImg < deltaNewSize; xImg++, xWin++)
            {
                if (xWin >= deltaWinSize - cxImgPos)
                    break;
                r = *src++;
                g = *src++;
                b = *src++;
                *dst++ = b; /* note the reverse order */
                *dst++ = g;
                *dst++ = r;
                if (cImgChannels == 4)
                {
                    a = *src++;
                }
            }
        }		
        
        // free memory
        
        if (pStretchedImage != NULL)
        {
            FreePool (pStretchedImage);
            pStretchedImage = NULL;
        }		
        
    }
	return TRUE;

}

VOID egFlipRB(IN EG_IMAGE *Image)
{	
	UINT32 x;
    register UINT8 tempB;
    if (Image->NeedFlip == TRUE)
    {
        for (x = 0; x < (unsigned long)(Image->Height) * (Image->Width) ; x++) {
            tempB = Image->PixelData[x].b;
            Image->PixelData[x].b = Image->PixelData[x].r;
            Image->PixelData[x].r = tempB;
        }
        Image->NeedFlip = FALSE;
    }
}

EG_IMAGE * egEnsureImageSize(IN EG_IMAGE *Image, IN UINTN Width, IN UINTN Height, IN EG_PIXEL *Color)
{
    EG_IMAGE *NewImage;
	
    if (Image == NULL)
        return NULL;
    if (Image->Width == Width && Image->Height == Height)
        return Image;
    
    NewImage = egCreateFilledImage(Width, Height, Image->HasAlpha, Color);
    if (NewImage == NULL) {
        egFreeImage(Image);
        return NULL;
    }
    egComposeImage(NewImage, Image, 0, 0);
    egFreeImage(Image);
    
    return NewImage;
}

//
// misc internal functions
//

VOID egInsertPlane(IN UINT8 *SrcDataPtr, IN UINT8 *DestPlanePtr, IN UINTN PixelCount)
{
    UINTN i;
    
    for (i = 0; i < PixelCount; i++) {
        *DestPlanePtr = *SrcDataPtr++;
        DestPlanePtr += 4;
    }
}

VOID egSetPlane(IN UINT8 *DestPlanePtr, IN UINT8 Value, IN UINTN PixelCount)
{
    UINTN i;
    
    for (i = 0; i < PixelCount; i++) {
        *DestPlanePtr = Value;
        DestPlanePtr += 4;
    }
}

VOID egCopyPlane(IN UINT8 *SrcPlanePtr, IN UINT8 *DestPlanePtr, IN UINTN PixelCount)
{
    UINTN i;
    
    for (i = 0; i < PixelCount; i++) {
        *DestPlanePtr = *SrcPlanePtr;
        DestPlanePtr += 4, SrcPlanePtr += 4;
    }
}




#define VIDEO(x) (((boot_args*)getBootArgs())->Video.v_ ## x)
//#define vram VIDEO(baseAddr)

VOID egVramWrite (IN VOID *data, IN UINTN width, IN UINTN Height, IN UINTN PosX, IN UINTN PosY, UINTN BytesPerScanLine)
{
    
    UINT8 *vram = (UINT8 *)VIDEO(baseAddr);
    UINTN depth = (UINTN)VIDEO (depth);
    UINTN rowBytes = (UINTN)VIDEO (rowBytes);
    UINTN vHeight = (UINTN)VIDEO (height);
    UINTN vWidth ;

	if (depth == 0x20  && rowBytes == (UINT32)width * BytesPerScanLine)
		memcpy((CHAR8 *)vram, data, rowBytes * vHeight);
	else
	{
		UINT32 r, g, b;
		UINT32 i, j;
        
        UINT32 x, y;
        vWidth = (UINTN)VIDEO (width);

        x = MIN(vWidth, width);
        y = MIN(vHeight, Height);

		for (i = PosY; i < y; i++)
			for (j = PosX; j < x; j++)
			{
				b = ((UINT8 *) data)[4*i*width + 4*j];
				g = ((UINT8 *) data)[4*i*width + 4*j + 1];
				r = ((UINT8 *) data)[4*i*width + 4*j + 2];
				switch (depth)
				{
					case 32:
                       
						*(UINT32 *)(((UINT8 *)vram)+i* x*BytesPerScanLine + j*4) = (b&0xff) | ((g&0xff)<<8) | ((r&0xff)<<16);
						break;
					case 24:
                        *(UINT32 *)(((UINT8 *)vram)+i*x*BytesPerScanLine + j*3) = ((*(UINT32 *)(((UINT8 *)vram)+i*x*BytesPerScanLine + j*3))&0xff000000)
						| (b&0xff) | ((g&0xff)<<8) | ((r&0xff)<<16);
						break;
					case 16:
						// Somehow 16-bit is always 15-bits really
						//						*(uint16_t *)(((uint8_t *)vram)+i*VIDEO (rowBytes) + j*2) = ((b&0xf8)>>3) | ((g&0xfc)<<3) | ((r&0xf8)<<8);
						//						break;
					case 15:
                        
						*(UINT16 *)(((UINT8 *)vram)+i*x*BytesPerScanLine + j*2) = ((b&0xf8)>>3) | ((g&0xf8)<<2) | ((r&0xf8)<<7);
						break;
					default:
						break;
				}
			}
	}
}