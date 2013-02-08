/*
 * libeg/libeg.h
 * EFI graphics library header for users
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

#ifndef __LIBEG_LIBEG_H__
#define __LIBEG_LIBEG_H__


/* types */

/* This should be compatible with EFI_UGA_PIXEL */
typedef struct {
    UINT8 b, g, r, a;
} EG_PIXEL;

typedef struct {
    UINTN       Width;
    UINTN       Height;
    BOOLEAN     HasAlpha;
    EG_PIXEL    *PixelData;
    BOOLEAN     NeedFlip;
    UINTN		piChannels;
} EG_IMAGE;

#define EG_EIPIXELMODE_GRAY         (0)
#define EG_EIPIXELMODE_GRAY_ALPHA   (1)
#define EG_EIPIXELMODE_COLOR        (2)
#define EG_EIPIXELMODE_COLOR_ALPHA  (3)
#define EG_EIPIXELMODE_ALPHA        (4)
#define EG_MAX_EIPIXELMODE          EG_EIPIXELMODE_ALPHA

#define EG_EICOMPMODE_NONE          (0)
#define EG_EICOMPMODE_RLE           (1)
#define EG_EICOMPMODE_EFICOMPRESS   (2)

typedef struct {
    UINTN       Width;
    UINTN       Height;
    UINTN       PixelMode;
    UINTN       CompressMode;
    const UINT8 *Data;
    UINTN       DataLength;
} EG_EMBEDDED_IMAGE;

/* functions */

VOID egInitScreen(VOID);
VOID egGetScreenSize(OUT UINTN *ScreenWidth, OUT UINTN *ScreenHeight);
CHAR16 * egScreenDescription(VOID);
BOOLEAN egHasGraphicsMode(VOID);
BOOLEAN egIsGraphicsModeEnabled(VOID);
VOID egSetGraphicsModeEnabled(IN BOOLEAN Enable);

// NOTE: Even when egHasGraphicsMode() returns FALSE, you should
//  call egSetGraphicsModeEnabled(FALSE) to ensure the system
//  is running in text mode. egHasGraphicsMode() only determines
//  if libeg can draw to the screen in graphics mode.
// ADDITIONAL NOTE: As for UEFI, you should also do this in chameleon, egHasGraphicsMode() 
// only return a boolean set in egInitScreen(), and do not check if graphic mode is really enabled

EG_IMAGE * egCreateImage(IN UINTN Width, IN UINTN Height, IN BOOLEAN HasAlpha);
EG_IMAGE * egCreateFilledImage(IN UINTN Width, IN UINTN Height, IN BOOLEAN HasAlpha, IN EG_PIXEL *Color);
EG_IMAGE * egNewImageFromImage(IN EG_IMAGE *Image);
VOID egCopyImage(IN OUT EG_IMAGE *Image1, IN EG_IMAGE *Image2);
EG_IMAGE * egCreateImageFromData(IN EG_PIXEL *PixelData, IN UINTN Width, IN UINTN Height, IN BOOLEAN HasAlpha, BOOLEAN FlipDone);
EG_IMAGE * egNewScaledImage(IN EG_IMAGE *Image, IN UINTN NewW, IN UINTN NewH);
VOID egCopyScaledImage(IN OUT EG_IMAGE *CompImage, IN EG_IMAGE *Image);
VOID egFreeImage(IN EG_IMAGE *Image);
EG_IMAGE * egLoadImage(IN CHAR16 *FilePath, IN BOOLEAN WantAlpha);
EG_IMAGE * egLoadIcon(IN CHAR16 *FilePath, IN UINTN IconSize);
EG_IMAGE * egDecodeImage(IN UINT8 *FileData, IN UINTN FileDataLength, IN CHAR16 *Format, IN BOOLEAN WantAlpha);

EG_IMAGE * egPrepareEmbeddedImage(IN EG_EMBEDDED_IMAGE *EmbeddedImage, IN BOOLEAN WantAlpha);
EG_IMAGE * egEnsureImageSize(IN EG_IMAGE *Image, IN UINTN Width, IN UINTN Height, IN EG_PIXEL *Color);

EFI_STATUS egLoadFile(IN CHAR16 *FileName,
                      OUT UINT8 **FileData, OUT UINTN *FileDataLength);
/*
EFI_STATUS egSaveFile(IN EFI_FILE_HANDLE BaseDir OPTIONAL, IN CHAR16 *FileName,
                      IN UINT8 *FileData, IN UINTN FileDataLength);
*/
VOID egFillImage(IN OUT EG_IMAGE *CompImage, IN EG_PIXEL *Color);
VOID egFillImageArea(IN OUT EG_IMAGE *CompImage,
                     IN UINTN AreaPosX, IN UINTN AreaPosY,
                     IN UINTN AreaWidth, IN UINTN AreaHeight,
                     IN EG_PIXEL *Color);
VOID egComposeImage(IN OUT EG_IMAGE *CompImage, IN EG_IMAGE *TopImage, IN UINTN PosX, IN UINTN PosY);

VOID egMeasureText(IN CHAR16 *Text, OUT UINTN *Width, OUT UINTN *Height);
VOID egRenderText(IN CHAR16 *Text, IN OUT EG_IMAGE *CompImage, IN UINTN PosX, IN UINTN PosY);

VOID egClearScreen(IN EG_PIXEL *Color);
VOID egDrawImage(IN EG_IMAGE *Image, IN UINTN ScreenPosX, IN UINTN ScreenPosY);

VOID egDrawImageArea(IN EG_IMAGE *Image,
                     IN UINTN AreaPosX, IN UINTN AreaPosY,
                     IN UINTN AreaWidth, IN UINTN AreaHeight,
                     IN UINTN ScreenPosX, IN UINTN ScreenPosY);

EG_IMAGE * egScreenShot(VOID);

VOID egVramWrite (IN VOID *data, IN UINTN width, IN UINTN Height, IN UINTN PosX, IN UINTN PosY, UINTN BytesPerScanLine);


/*
 * Image View structure.
 */
typedef struct {
	EG_IMAGE	*Image;
	CHAR16		*Name;
    UINTN       PosX;
    UINTN       PosY;
} EG_IMAGE_VIEW;

EG_IMAGE_VIEW * egCreateImageView(IN UINTN Width, IN UINTN Height, IN BOOLEAN HasAlpha, IN UINTN PosX, IN UINTN PosY, IN CHAR16 *Name);

EG_IMAGE_VIEW * egCreateImageViewFromData(IN EG_PIXEL *PixelData, IN UINTN Width, IN UINTN Height, IN BOOLEAN HasAlpha, IN UINTN PosX, IN UINTN PosY, IN CHAR16 *Name, BOOLEAN NeedFlip);

VOID egFreeImageView(IN EG_IMAGE_VIEW *ImageView);

/*
 * View structure.
 */
typedef struct {
    EG_IMAGE    *CompImage;
	EG_IMAGE    *Background;
    UT_array    *Array; // Arrey of image View
	BOOLEAN		isDirty;
    BOOLEAN     AutoUpdate;
    BOOLEAN     useBackgroundColor;
} EG_VIEW;




VOID egViewBufferUpdate(IN OUT EG_VIEW *View);
VOID egViewUpdate(IN OUT EG_VIEW *View);
EG_VIEW * egCreateView(IN UINTN Width, IN UINTN Height, IN BOOLEAN HasAlpha);
VOID egFreeView(IN EG_VIEW *View);
VOID egViewSetBackgroundColor(IN OUT EG_VIEW *CompView, IN EG_PIXEL *Color);
VOID egViewSetBackground(IN OUT EG_VIEW *CompView, IN EG_IMAGE *Background);
EG_IMAGE_VIEW * egViewAddImageView(IN OUT EG_VIEW *CompView, IN EG_IMAGE_VIEW *TopView);
VOID egViewRemoveImageViewByName(IN OUT EG_VIEW *CompView, IN CHAR16 *Name, IN UINTN NameSize);
VOID egViewRemoveImageView(IN OUT EG_VIEW *CompView, IN EG_IMAGE_VIEW *ImageView);
EG_PIXEL * egViewGetBackgroundColor(IN OUT EG_VIEW *CompView);
VOID egViewSetUpdateValue(IN OUT EG_VIEW *View, IN BOOLEAN AutoUpdate);
BOOLEAN egViewGetUpdateValue(IN OUT EG_VIEW *View);
VOID egViewSetDirtyValue(IN OUT EG_VIEW *View, IN BOOLEAN Dirty);
BOOLEAN egViewGetDirtyValue(IN OUT EG_VIEW *View);
VOID egUseBackgroundColor(IN OUT EG_VIEW *CompView, IN BOOLEAN Use);

#endif /* __LIBEG_LIBEG_H__ */

/* EOF */
