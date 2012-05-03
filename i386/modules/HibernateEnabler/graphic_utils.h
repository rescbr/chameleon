 /* Graphic utility functions and data types
  * Prashant Vaibhav (C) 12/12/2008
  * Chameleon
  */

// Everything here is 32 bits per pixel non-premultiplied ARGB format
//

#ifndef H_GRAPHIC_UTILS_H
#define H_GRAPHIC_UTILS_H

#include "libsa.h"


void drawPreview(void *src, uint8_t * saveunder);
void
spinActivityIndicator_hook(void *arg1, void *arg2, void *arg3, void *arg4, void* arg5, void* arg6);

void updateProgressBar(uint8_t * saveunder, int32_t firstBlob, int32_t select);

void
loadImageScale (void *input, int iw, int ih, int ip, void *output, int ow, int oh, int op, int or);

void
setVideoMode( int mode);

#endif//H_GRAPHIC_UTILS_H
