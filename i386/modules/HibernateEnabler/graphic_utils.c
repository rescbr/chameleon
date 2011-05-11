
/* Graphic utility functions and data types
 * Prashant Vaibhav (C) 12/12/2008
 * Chameleon
 */

#include "graphic_utils.h"
#include "graphics.h"
#include "IOHibernatePrivate.h"
#include "bmdecompress.h"

#define VIDEO(x) (bootArgs->Video.v_ ## x)

#define MIN(x, y) ((x) < (y) ? (x) : (y))

int previewTotalSectors = 0;
uint8_t *previewSaveunder = 0;
int previewLoadedSectors = 0;

void
loadImageScale (void *input, int iw, int ih, int ip, void *output, int ow, int oh, int op, int or)
{
	int x,y, off;
	int red=0x7f, green=0x7f, blue=0x7f;
	for (x=0;x<ow;x++)
		for (y=0;y<oh;y++)
		{
			off=(x*iw)/ow+((y*ih)/oh)*iw;
			switch (ip)
			{
				case 16:
				{
					uint16_t val;
					val=((uint16_t *)input)[off];
					red=(val>>7)&0xf8;
					green=(val>>2)&0xf8;
					blue=(val<<3)&0xf8;
					break;		
				}
				case 32:
				{
					uint32_t val;
					val=((uint32_t *)input)[off];
					red=(val>>16)&0xff;
					green=(val>>8)&0xff;
					blue=(val)&0xff;
					break;
				}				
			}
			char *ptr=(char *)output+x*(op/8)+y*or;
			switch (op)
			{
				case 16:
					*((uint16_t *)ptr) = ((red   & 0xF8) << 7) | 
					((green & 0xF8) << 2) |
					((blue  & 0xF8) >> 3);
					break;
				case 32 :
					*((uint32_t *)ptr) = (red << 16) | (green << 8) | blue;
					break;
			}
		}
}

DECLARE_IOHIBERNATEPROGRESSALPHA

void drawPreview(void *src, uint8_t * saveunder)
{
	uint8_t *  screen;
	uint32_t   rowBytes, pixelShift;
	uint32_t   x, y;
	int32_t    blob;
	uint32_t   alpha, in, color, result;
	uint8_t *  out;
	void *uncomp;
	int origwidth, origheight, origbpx;
	uint32_t   saveindex[kIOHibernateProgressCount] = { 0 };
	
	if (src && (uncomp=DecompressData(src, &origwidth, &origheight, &origbpx)))
	{
		if (!setVESAGraphicsMode(origwidth, origheight, origbpx, 0))
			if (initGraphicsMode () != errSuccess)
				return;
		screen = (uint8_t *) VIDEO (baseAddr);
		rowBytes = VIDEO (rowBytes);
		loadImageScale (uncomp, origwidth, origheight, origbpx, screen, VIDEO(width), VIDEO(height), VIDEO(depth), VIDEO (rowBytes));
	}
	else
	{
		if (initGraphicsMode () != errSuccess)
			return;
		screen = (uint8_t *) VIDEO (baseAddr);
		rowBytes = VIDEO (rowBytes);
		// Set the screen to 75% grey.
        drawColorRectangle(0, 0, VIDEO(width), VIDEO(height), 0x01 /* color index */);
	}
	
	
	pixelShift = VIDEO (depth) >> 4;
	if (pixelShift < 1) return;
	
	screen += ((VIDEO (width) 
				- kIOHibernateProgressCount * (kIOHibernateProgressWidth + kIOHibernateProgressSpacing)) << (pixelShift - 1))
	+ (VIDEO (height) - kIOHibernateProgressOriginY - kIOHibernateProgressHeight) * rowBytes;
	
	for (y = 0; y < kIOHibernateProgressHeight; y++)
	{
		out = screen + y * rowBytes;
		for (blob = 0; blob < kIOHibernateProgressCount; blob++)
		{
			color = blob ? kIOHibernateProgressDarkGray : kIOHibernateProgressMidGray;
			for (x = 0; x < kIOHibernateProgressWidth; x++)
			{
				alpha  = gIOHibernateProgressAlpha[y][x];
				result = color;
				if (alpha)
				{
					if (0xff != alpha)
					{
						if (1 == pixelShift)
						{
							in = *((uint16_t *)out) & 0x1f;	// 16
							in = (in << 3) | (in >> 2);
						}
						else
							in = *((uint32_t *)out) & 0xff;	// 32
						saveunder[blob * kIOHibernateProgressSaveUnderSize + saveindex[blob]++] = in;
						result = ((255 - alpha) * in + alpha * result + 0xff) >> 8;
					}
					if (1 == pixelShift)
					{
						result >>= 3;
						*((uint16_t *)out) = (result << 10) | (result << 5) | result;	// 16
					}
					else
						*((uint32_t *)out) = (result << 16) | (result << 8) | result;	// 32
				}
				out += (1 << pixelShift);
			}
			out += (kIOHibernateProgressSpacing << pixelShift);
		}
	}
}

void updateProgressBar(uint8_t * saveunder, int32_t firstBlob, int32_t select)
{
	uint8_t * screen;
	uint32_t  rowBytes, pixelShift;
	uint32_t  x, y;
	int32_t   blob, lastBlob;
	uint32_t  alpha, in, color, result;
	uint8_t * out;
	uint32_t  saveindex[kIOHibernateProgressCount] = { 0 };
	
	pixelShift = VIDEO(depth) >> 4;
	if (pixelShift < 1) return;
	screen = (uint8_t *) VIDEO (baseAddr);
	rowBytes = VIDEO (rowBytes);
	
	screen += ((VIDEO (width) 
				- kIOHibernateProgressCount * (kIOHibernateProgressWidth + kIOHibernateProgressSpacing)) << (pixelShift - 1))
	+ (VIDEO (height) - kIOHibernateProgressOriginY - kIOHibernateProgressHeight) * rowBytes;
	
	lastBlob  = (select < kIOHibernateProgressCount) ? select : (kIOHibernateProgressCount - 1);
	
	screen += (firstBlob * (kIOHibernateProgressWidth + kIOHibernateProgressSpacing)) << pixelShift;
	
	for (y = 0; y < kIOHibernateProgressHeight; y++)
	{
		out = screen + y * rowBytes;
		for (blob = firstBlob; blob <= lastBlob; blob++)
		{
			color = (blob < select) ? kIOHibernateProgressLightGray : kIOHibernateProgressMidGray;
			for (x = 0; x < kIOHibernateProgressWidth; x++)
			{
				alpha  = gIOHibernateProgressAlpha[y][x];
				result = color;
				if (alpha)
				{
					if (0xff != alpha)
					{
						in = saveunder[blob * kIOHibernateProgressSaveUnderSize + saveindex[blob]++];
						result = ((255 - alpha) * in + alpha * result + 0xff) / 255;
					}
					if (1 == pixelShift)
					{
						result >>= 3;
						*((uint16_t *)out) = (result << 10) | (result << 5) | result;	// 16
					}
					else
						*((uint32_t *)out) = (result << 16) | (result << 8) | result;	// 32
				}
				out += (1 << pixelShift);
			}
			out += (kIOHibernateProgressSpacing << pixelShift);
		}
	}
}

void
spinActivityIndicator_hook(void *arg1, void *arg2, void *arg3, void *arg4, void* arg5, void* arg6)
{
	int sectors = *(int*)arg1;
	bool *doreturn = (bool*)arg2;
	
	if (previewTotalSectors && previewSaveunder)
	{
		int blob, lastBlob;
		
		lastBlob = (previewLoadedSectors * kIOHibernateProgressCount) / previewTotalSectors;
		previewLoadedSectors+=sectors;
		blob = (previewLoadedSectors * kIOHibernateProgressCount) / previewTotalSectors;
		
		if (blob!=lastBlob)
			updateProgressBar (previewSaveunder, lastBlob, blob);
		*doreturn = true;
	}
}