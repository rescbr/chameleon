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

#endif//H_GRAPHIC_UTILS_H
