/*
 * egpng.c
 * Loading function for PNG images (extension for libeg)
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
#include <pexpert/i386/modules.h>
#include "PngFile.h"

EG_IMAGE * egDecodePNG(IN UINT8 *FileData, IN UINTN FileDataLength, IN UINTN IconSize, IN BOOLEAN WantAlpha)
{
    EG_IMAGE            *NewImage;
	UINT8				*imagedata;
	int					width,height;	
	int					piChannels;
	VOID				*BkgColor = NULL;
	BOOLEAN				ret ;

	if (FileData == NULL)
		return NULL;
	
	width = 0;
	height = 0;
	piChannels = 0;
	imagedata = NULL;
	BkgColor = NULL;
	    
    ret = PngLoadData(FileData,  FileDataLength,  &width,
					  &height, &imagedata, &piChannels, BkgColor);
    
	if (ret == FALSE) 
	{
		return NULL;
	}
	
    if (width && height && imagedata && piChannels)
	{
		NewImage = egCreateImageFromData((EG_PIXEL*)imagedata, width, height, (piChannels == 4) ? TRUE : FALSE, FALSE);
		
		if (NewImage) {
			egFlipRB(NewImage);					
			return NewImage;
		}

	}
	
	return NULL;  
}