/*
 * libeg/libegint.h
 * EFI graphics library internal header
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

#ifndef __LIBEG_LIBEGINT_H__
#define __LIBEG_LIBEGINT_H__

#include <utarray.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <efi.h>
#include "libeg.h"
#include <IOGraphics.h>

/**
 Return the maximum of two operands.
 
 This macro returns the maximum of two operand specified by a and b.
 Both a and b must be the same numerical types, signed or unsigned.
 
 @param   a        The first operand with any numerical type.
 @param   b        The second operand. Can be any numerical type as long as is
 the same type as a.
 
 @return  Maximum of two operands.
 
 **/
#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

/**
 Return the minimum of two operands.
 
 This macro returns the minimal of two operand specified by a and b.
 Both a and b must be the same numerical types, signed or unsigned.
 
 @param   a        The first operand with any numerical type.
 @param   b        The second operand. It should be the same any numerical type with a.
 
 @return  Minimum of two operands.
 
 **/
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

/**
 Return the absolute value of a signed operand.
 
 This macro returns the absolute value of the signed operand specified by a.
 
 @param   a        The signed operand.
 
 @return  The absolute value of the signed operand.
 
 **/
#ifndef ABS
#define ABS(a)                          \
(((a) < 0) ? (-(a)) : (a))
#endif

//
// Generate an ASSERT if Status is an error code
//
#ifndef ASSERT_EFI_ERROR
#define ASSERT_EFI_ERROR(status)  ASSERT(!EFI_ERROR(status))
#endif

/* types */

typedef EG_IMAGE * (*EG_DECODE_FUNC)(IN UINT8 *FileData, IN UINTN FileDataLength, IN UINTN IconSize, IN BOOLEAN WantAlpha);

/* functions */

VOID egRestrictImageArea(IN EG_IMAGE *Image,
                         IN UINTN AreaPosX, IN UINTN AreaPosY,
                         IN OUT UINTN *AreaWidth, IN OUT UINTN *AreaHeight);
VOID egRawCopy(IN OUT EG_PIXEL *CompBasePtr, IN EG_PIXEL *TopBasePtr,
               IN UINTN Width, IN UINTN Height,
               IN UINTN CompLineOffset, IN UINTN TopLineOffset);
VOID egRawCompose(IN OUT EG_PIXEL *CompBasePtr, IN EG_PIXEL *TopBasePtr,
                  IN UINTN Width, IN UINTN Height,
                  IN UINTN CompLineOffset, IN UINTN TopLineOffset);

#define PLPTR(imagevar, colorname) ((UINT8 *) &((imagevar)->PixelData->colorname))

VOID egDecompressIcnsRLE(IN OUT UINT8 **CompData, IN OUT UINTN *CompLen, IN UINT8 *DestPlanePtr, IN UINTN PixelCount);
EG_IMAGE * egDecodeICNS(IN UINT8 *FileData, IN UINTN FileDataLength, IN UINTN IconSize, IN BOOLEAN WantAlpha);
VOID egInsertPlane(IN UINT8 *SrcDataPtr, IN UINT8 *DestPlanePtr, IN UINTN PixelCount);
VOID egSetPlane(IN UINT8 *DestPlanePtr, IN UINT8 Value, IN UINTN PixelCount);
VOID egCopyPlane(IN UINT8 *SrcPlanePtr, IN UINT8 *DestPlanePtr, IN UINTN PixelCount);
EG_IMAGE * egDecodePNG(IN UINT8 *FileData, IN UINTN FileDataLength, IN UINTN IconSize, IN BOOLEAN WantAlpha);

VOID egEncodeBMP(IN EG_IMAGE *Image, OUT UINT8 **FileData, OUT UINTN *FileDataLength);
EG_IMAGE * egDecodeBMP(IN UINT8 *FileData, IN UINTN FileDataLength, IN UINTN IconSize, IN BOOLEAN WantAlpha);

VOID egFlipRB(IN EG_IMAGE *Image);
BOOLEAN egComposeImageStretched(IN OUT EG_IMAGE *CompImage, IN EG_IMAGE *TopImage);


static void image_view_t_copy(void *_dst, const void *_src) {
    EG_IMAGE_VIEW *__dst = (EG_IMAGE_VIEW*)_dst, *__src = (EG_IMAGE_VIEW*)_src;
    __dst->Name = __src->Name ? PoolPrint(__src->Name) : NULL;
    __dst->PosX = __src->PosX;
    __dst->PosY = __src->PosY;
    
    __dst->Image = (EG_IMAGE*)malloc(sizeof(EG_IMAGE));
    if (__dst->Image)
    {
        EG_IMAGE *dst = (EG_IMAGE*)__dst->Image, *src = (EG_IMAGE*)__src->Image;
        dst->Width = src->Width;
        dst->Height = src->Height;
        dst->HasAlpha = src->HasAlpha;
        dst->NeedFlip = src->NeedFlip;
        
        dst->PixelData = (EG_PIXEL *)AllocateCopyPool(src->Width*sizeof(EG_PIXEL)*src->Height, src->PixelData);
    }
    
    
}

static void image_view_t_dtor(void *_elt) {
    EG_IMAGE_VIEW *__elt = (EG_IMAGE_VIEW*)_elt;
    EG_IMAGE *elt = __elt->Image;
    if (elt->PixelData)  free(elt->PixelData);
    if (__elt->Name)  free(__elt->Name);
    if (__elt->Image) free(__elt->Image);
}
static const UT_icd image_view_t_icd _UNUSED_ = {sizeof(EG_IMAGE_VIEW), NULL, image_view_t_copy, image_view_t_dtor};

#endif /* __LIBEG_LIBEGINT_H__ */

/* EOF */
