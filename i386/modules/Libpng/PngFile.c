/*-------------------------------------
 *  PNGFILE.C -- Image File Functions
 *-------------------------------------
 *
 * Copyright 2000, Willem van Schaik.
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

#include "libsaio.h"
#include "stdio.h"

#include "png.h"
#include "pngfile.h"
#include "modules.h"

png_const_charp msg;

static png_structp png_ptr = NULL;
static png_infop info_ptr = NULL;

void png_file_init(void);
void png_file_init(void){}

static bool PngHandleFile (FILE *pfFile , unsigned char **ppbImageData,
                    int *piWidth, int *piHeight, int *piChannels, void *BkgColor);
/* cexcept interface */

static void
png_cexcept_error(png_structp png_ptr, png_const_charp msg)
{
   if(png_ptr)
     ;
   printf("libpng error: %s\n", msg);
   {
      Throw (-1);
   }
}

#ifndef PNG_STDIO_SUPPORTED
typedef FILE                * png_FILE_p;
static void PNGCBAPI
png_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_size_t check = 0;
	png_voidp io_ptr;
	
	/* fread() returns 0 on error, so it is OK to store this in a png_size_t
	 * instead of an int, which is what fread() actually returns.
	 */
	io_ptr = png_get_io_ptr(png_ptr);
	if (io_ptr != NULL)
	{
		check = fread(data, 1, length, (png_FILE_p)io_ptr);
	}
	
	if (check != length)
	{
		png_error(png_ptr, "Read Error");
	}

}

#endif

/* PNG image handler functions */

static bool PngHandleFile (FILE *pfFile , unsigned char **ppbImageData,
                   int *piWidth, int *piHeight, int *piChannels, void *BkgColor)
{
    png_byte            pbSig[8];
    int                 iBitDepth;
    int                 iColorType;
#ifdef PNG_FLOATING_ARITHMETIC_SUPPORTED
#ifdef PNG_FLOATING_POINT_SUPPORTED    
    double              dGamma;
    png_color_16       *pBackground;
    png_color *pBkgColor = (png_color *)BkgColor;
#endif
#endif
    png_uint_32         ulChannels;
    png_uint_32         ulRowBytes;
    png_byte           *pbImageData = *ppbImageData;
    static png_byte   **ppbRowPointers = NULL;
    int                 i;
    
    /* first check the eight byte PNG signature */
    
    fread(pbSig, 1, 8, pfFile);
    
    if (png_sig_cmp(pbSig, 0, 8))
    {
        *ppbImageData = pbImageData = NULL;
        return false;
    }
    
    /* create the two png(-info) structures */
    
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL,
                                     (png_error_ptr)png_cexcept_error, (png_error_ptr)NULL);
    if (!png_ptr)
    {
        *ppbImageData = pbImageData = NULL;
        return false;
    }
    
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        *ppbImageData = pbImageData = NULL;
        return false;
    }
    
    CEXCEPTION_T e = CEXCEPTION_NONE;
    
    Try
    {
        
        /* initialize the png structure */
        
#ifdef PNG_STDIO_SUPPORTED
        png_init_io(png_ptr, pfFile);
#else
        png_set_read_fn(png_ptr, (png_voidp)pfFile, png_read_data);
#endif
        
        png_set_sig_bytes(png_ptr, 8);
        
        /* read all PNG info up to image data */
        
        png_read_info(png_ptr, info_ptr);
        
        /* get width, height, bit-depth and color-type */
        
        png_get_IHDR(png_ptr, info_ptr, (png_uint_32*)piWidth, (png_uint_32*)piHeight, &iBitDepth,
                     &iColorType, NULL, NULL, NULL);
        
        /* expand images of all color-type and bit-depth to 3x8-bit RGB */
        /* let the library process alpha, transparency, background, etc. */
        
#ifdef PNG_READ_16_TO_8_SUPPORTED
        if (iBitDepth == 16)
#  ifdef PNG_READ_SCALE_16_TO_8_SUPPORTED
            png_set_scale_16(png_ptr);
#  else
        png_set_strip_16(png_ptr);
#  endif
#endif
        if (iColorType == PNG_COLOR_TYPE_PALETTE)
            png_set_expand(png_ptr);
        if (iBitDepth < 8)
            png_set_expand(png_ptr);
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
            png_set_expand(png_ptr);
        if (iColorType == PNG_COLOR_TYPE_GRAY ||
            iColorType == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png_ptr);
#ifdef PNG_FLOATING_ARITHMETIC_SUPPORTED
#ifdef PNG_FLOATING_POINT_SUPPORTED
        /* set the background color to draw transparent and alpha images over */
        if (png_get_bKGD(png_ptr, info_ptr, &pBackground))
        {
            png_set_background(png_ptr, pBackground, PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
            pBkgColor->red   = (png_byte) pBackground->red;
            pBkgColor->green = (png_byte) pBackground->green;
            pBkgColor->blue  = (png_byte) pBackground->blue;
        }
        else
        {
            pBkgColor = NULL;
        }
        
        /* if required set gamma conversion */
        if (png_get_gAMA(png_ptr, info_ptr, &dGamma))
            png_set_gamma(png_ptr, (double) 2.2, dGamma);
#endif
#endif

        /* after the transformations are registered, update info_ptr data */
        
        png_read_update_info(png_ptr, info_ptr);
        
        /* get again width, height and the new bit-depth and color-type */
        
        png_get_IHDR(png_ptr, info_ptr, (png_uint_32*)piWidth, (png_uint_32*)piHeight, &iBitDepth,
                     &iColorType, NULL, NULL, NULL);
        
        
        /* row_bytes is the width x number of channels */
        
        ulRowBytes = png_get_rowbytes(png_ptr, info_ptr);
        ulChannels = png_get_channels(png_ptr, info_ptr);
        
        *piChannels = ulChannels;
        
        /* now we can allocate memory to store the image */
        
        if (pbImageData)
        {
            free (pbImageData);
            pbImageData = NULL;
        }
        if ((pbImageData = (png_byte *) malloc(ulRowBytes * (*piHeight)
                                               * sizeof(png_byte))) == NULL)
        {
            png_error(png_ptr, "Visual PNG: out of memory");
        }
        *ppbImageData = pbImageData;
        
        /* and allocate memory for an array of row-pointers */
        
        if ((ppbRowPointers = (png_bytepp) malloc((*piHeight)
                                                  * sizeof(png_bytep))) == NULL)
        {
            png_error(png_ptr, "Visual PNG: out of memory");
        }
        
        /* set the individual row-pointers to point at the correct offsets */
        
        for (i = 0; i < (*piHeight); i++)
            ppbRowPointers[i] = pbImageData + i * ulRowBytes;
        
        /* now we can go ahead and just read the whole image */
        
        png_read_image(png_ptr, ppbRowPointers);
        
        /* read the additional chunks in the PNG file (not really needed) */
        
        png_read_end(png_ptr, NULL);
        
        /* and we're done */
        
        free (ppbRowPointers);
        ppbRowPointers = NULL;
        
        /* yepp, done */
    }
    
    Catch (e)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        
        *ppbImageData = pbImageData = NULL;
        
        if(ppbRowPointers)
            free (ppbRowPointers);
        
        
        return false;
    }
    
    
    return true;
}

bool PngLoadImage (const char* pstrFileName, unsigned char **ppbImageData,
                   int *piWidth, int *piHeight, int *piChannels, void *BkgColor)
{
    static FILE        *pfFile;    
    png_byte           *pbImageData = *ppbImageData;
    bool ret;
    /* open the PNG input file */

    if (!pstrFileName)
    {
        *ppbImageData = pbImageData = NULL;
        return false;
    }

    if (!(pfFile = fopen(pstrFileName, "rb")))
    {
        *ppbImageData = pbImageData = NULL;
        return false;
    }
    ret = PngHandleFile(pfFile, ppbImageData,
                piWidth, piHeight, piChannels, BkgColor);

    fclose (pfFile);

    return ret;
}

bool PngLoadData(unsigned char *pngData, int pngSize, int *piWidth,
                 int *piHeight, unsigned char **ppbImageData, int *piChannels, void *BkgColor)
{
    
    static FILE        *pfFile;
    png_byte           *pbImageData = *ppbImageData;
    bool ret;
    int fd ;
    /* open the PNG input file */
    
    if (!pngData)
    {
        *ppbImageData = pbImageData = NULL;
        return false;
    }
    
    if ((fd = openmem((char*)pngData, pngSize)) < 0)
    {
        *ppbImageData = pbImageData = NULL;
        return false;
    }
    
    if (!(pfFile = fdopen(fd, "rb")))
    {
        *ppbImageData = pbImageData = NULL;
        return false;
    }
    ret = PngHandleFile(pfFile, ppbImageData,
                        piWidth, piHeight, piChannels, BkgColor);
    
    fclose (pfFile);
	
    return ret;
}

void PngLoadImage_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
    void *BkgColor = NULL;
    const char*pstrFileName = (const char*)arg1;
    int *piWidth = (int *)arg3;
    int *piHeight = (int *)arg4;
    unsigned char **ppbImageData = (unsigned char **)arg5;
    int *piChannels = (int *)arg6;
    bool ret ;
    ret = PngLoadImage (pstrFileName, ppbImageData,
                   piWidth, piHeight, piChannels, BkgColor);
    
    if (ret == false) {
        *piHeight =0;
        *piWidth = 0;
        *ppbImageData = NULL;
        *piChannels = 0;
    }
}

void PngLoadData_hook(void* arg1, void* arg2, void* arg3, void* arg4, void* arg5, void* arg6)
{
    void *BkgColor = NULL;
    unsigned char *pngData = (unsigned char *)arg1;
    int pngSize = *(int*)arg2;
    int *piWidth = (int *)arg3;
    int *piHeight = (int *)arg4;
    unsigned char **ppbImageData = (unsigned char **)arg5;
    int *piChannels = (int *)arg6;
    bool ret ;

    
    ret = PngLoadData(pngData,  pngSize, piWidth,
                piHeight, ppbImageData, piChannels, BkgColor);
    
    if (ret == false) {
        *piHeight =0;
        *piWidth = 0;
        *ppbImageData = NULL;
        *piChannels = 0;
    }
}

void Libpng_start(void);
void Libpng_start(void)
{
	register_hook_callback("PngLoadData", &PngLoadData_hook);
    register_hook_callback("PngLoadImage", &PngLoadImage_hook);

	
}

/*-----------------
 *  end of source
 *-----------------
 */
