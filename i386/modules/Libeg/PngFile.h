/*------------------------------------------*/
/*  PNGFILE.H -- Header File for pngfile.c*/
/*------------------------------------------*/

/* Copyright 2000, Willem van Schaik.*/

/* This code is released under the libpng license.*/
/* For conditions of distribution and use, see the disclaimer*/
/* and license in png.h*/


bool PngLoadImage (const char* pstrFileName, unsigned char **ppbImageData,
                   int *piWidth, int *piHeight, int *piChannels, void *BkgColor);



bool PngLoadData(unsigned char *pngData, int pngSize, int *piWidth,
                 int *piHeight, unsigned char **ppbImageData, int *piChannels, void *BkgColor);

