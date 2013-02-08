/*
 * libeg/image_view.c
 * Image View handling functions (extension for libeg)
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

//
// Basic image view handling
//

EG_IMAGE_VIEW * egCreateImageView(IN UINTN Width, IN UINTN Height, IN BOOLEAN HasAlpha, IN UINTN PosX, IN UINTN PosY, IN CHAR16 *Name)
{
	EG_IMAGE_VIEW * NewImageView;
	NewImageView = (EG_IMAGE_VIEW *) AllocateZeroPool(sizeof(EG_IMAGE_VIEW));
    if (NewImageView == NULL)
        return NULL;
  
    NewImageView->Image = egCreateImage(Width, Height, HasAlpha);
    if (NewImageView->Image == NULL) {
		FreePool(NewImageView);
        return NULL;
	}
	
    NewImageView->Name = PoolPrint(Name);
    if (NewImageView->Name == NULL) {
        egFreeImage(NewImageView->Image);
		FreePool(NewImageView);
        return NULL;
    }
    
    NewImageView->PosX = PosX;
    NewImageView->PosY = PosY;
    return NewImageView;
}

EG_IMAGE_VIEW * egCreateImageViewFromData(IN EG_PIXEL *PixelData, IN UINTN Width, IN UINTN Height, IN BOOLEAN HasAlpha, IN UINTN PosX, IN UINTN PosY, IN CHAR16 *Name, BOOLEAN NeedFlip)
{
    EG_IMAGE_VIEW * NewImageView;
    
    NewImageView = egCreateImageView(Width, Height, HasAlpha, PosX, PosY, Name);
    if (NewImageView == NULL)
        return NULL;
    
    NewImageView->Image->NeedFlip = NeedFlip;
    
    memcpy(NewImageView->Image->PixelData, PixelData, Width * Height * sizeof(EG_PIXEL));
    return NewImageView;
}

VOID egFreeImageView(IN EG_IMAGE_VIEW *ImageView)
{
    if (ImageView != NULL) {
        if (ImageView->Image != NULL)
            egFreeImage(ImageView->Image);
		if (ImageView->Name != NULL)
            FreePool(ImageView->Name);
        FreePool(ImageView);
    }
}
