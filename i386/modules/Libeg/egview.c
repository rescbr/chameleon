/*
 * egview.c
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
#include <utarray.h>
#include "libegint.h"
#include <CException.h>
#include <sys/time.h>

STATIC VOID _egViewBufferUpdate(IN OUT EG_VIEW *View)
{
    EG_IMAGE_VIEW *p = NULL;

    ASSERT(View);
    ASSERT(View->CompImage);
    ASSERT(View->CompImage->PixelData);

    memset(View->CompImage->PixelData,0,View->CompImage->Width*View->CompImage->Height*sizeof(EG_PIXEL));
    
    if (View->Background) {
        
        if (View->useBackgroundColor == TRUE)
        {
            ASSERT(View->Background->PixelData);
            egFillImage(View->CompImage,
                        
                        (EG_PIXEL*)&View->Background->PixelData[0]);
        }
        else
            egComposeImage(View->CompImage, View->Background, 0, 0);
        
    }
    
    if (View->Array) {
        while( (p=(EG_IMAGE_VIEW*)utarray_next(View->Array,p))) {
            if (p->Image)
            {
                if (p->Name)
                {
                    egFlipRB(p->Image);
                    egComposeImage(View->CompImage, p->Image, p->PosX, p->PosY);
                    
                }
                
            }
        }
    }
    
    View->isDirty = FALSE;

}

static long long
timeval_diff(struct timeval *difference,
             struct timeval *end_time,
             struct timeval *start_time
             )
{
    struct timeval temp_diff;
    
    if(difference==NULL)
    {
        difference=&temp_diff;
    }
    
    difference->tv_sec =end_time->tv_sec -start_time->tv_sec ;
    difference->tv_usec=end_time->tv_usec-start_time->tv_usec;
    
    /* Using while instead of if below makes the code slightly more robust. */
    
    while(difference->tv_usec<0)
    {
        difference->tv_usec+=1000000;
        difference->tv_sec -=1;
    }
    
    return 1000000LL*difference->tv_sec+
    difference->tv_usec;
    
} 

VOID egViewBufferUpdate(IN OUT EG_VIEW *View)
{
    CEXCEPTION_T e = CEXCEPTION_NONE;

    static struct timeval earlier;
    static struct timeval later;
    static BOOLEAN use_earlier = TRUE;
    static BOOLEAN use_later = FALSE;
    static long long diff = 0;
    int h = 16666;
    
    Try
    {
        ASSERT(View);
        ASSERT(View->CompImage);        

        if (use_earlier == TRUE && use_later == FALSE)
        {
            memset(&earlier,0,sizeof(struct timeval));

            if(gettimeofday(&earlier,NULL))
            {
                
                ASSERT(FALSE);
            }
            use_later = TRUE;
            use_earlier = FALSE;

        }
        else if(use_earlier == FALSE && use_later == TRUE)
        {
            
            memset(&later,0,sizeof(struct timeval));
            
            if(gettimeofday(&later,NULL))
            {
                
                ASSERT(FALSE);
            }
            use_earlier = TRUE;            
            use_later = FALSE;
        }
        else
        {
            memset(&later,0,sizeof(struct timeval));
            memset(&earlier,0,sizeof(struct timeval));        
        }
        
        diff = timeval_diff(NULL,&later,&earlier);
        
        if (diff != 0) {
            
            
            if (diff >= h || diff <= 0-h) {
                _egViewBufferUpdate(View);
            }
            else
            {
                if (diff > 0) {
                    diff = h - diff ;
                }
                else
                {
                    diff = h + diff ;
                }
                delay(diff);
                _egViewBufferUpdate(View);

            }            
        }
        
        
    }
    Catch(e)
    {
    }    
}

VOID egViewUpdate(IN OUT EG_VIEW *View)
{
    if (View != NULL) {
        
        if (View->isDirty == TRUE)
        {
            egViewBufferUpdate(View);
            
            //if (!egHasGraphicsMode())
            //    return;
            
            ASSERT(View->CompImage);
            ASSERT(View->CompImage->PixelData);
            
            if (View->isDirty == FALSE)
                egVramWrite ( View->CompImage->PixelData, View->CompImage->Width, View->CompImage->Height, 0 , 0, sizeof(EG_PIXEL) );
        }
        
    }
}

VOID egViewSetUpdateValue(IN OUT EG_VIEW *View, IN BOOLEAN AutoUpdate)
{
    if (View != NULL) {
        
        View->AutoUpdate = AutoUpdate;
    }
}

BOOLEAN egViewGetUpdateValue(IN OUT EG_VIEW *View)
{
    if (View != NULL) {
        
        return View->AutoUpdate;
        
    }    
    return FALSE; // we return false if the view is null

}

VOID egViewSetDirtyValue(IN OUT EG_VIEW *View, IN BOOLEAN Dirty)
{
    if (View != NULL) {
        
        View->isDirty = Dirty;
    }
}

BOOLEAN egViewGetDirtyValue(IN OUT EG_VIEW *View)
{
    if (View != NULL) {
        
        return View->isDirty;
        
    }
    return FALSE; // we return false if the view is null
    
}

EG_VIEW * egCreateView(IN UINTN Width, IN UINTN Height, IN BOOLEAN HasAlpha)
{
    EG_VIEW         *NewView = NULL;
	CEXCEPTION_T e = CEXCEPTION_NONE;
    
    NewView = (EG_VIEW *) AllocateZeroPool(sizeof(EG_VIEW));
    if (NewView == NULL)
        return NULL;
    
    NewView->CompImage = egCreateImage( Width,   Height,  HasAlpha);
    if (NewView->CompImage == NULL) {
        FreePool(NewView);
        return NULL;
    }
    
	NewView->Background = egCreateImage( Width,   Height,  HasAlpha);
    if (NewView->Background == NULL) {
		egFreeImage(NewView->CompImage);
        FreePool(NewView);
        return NULL;
    }
    NewView->useBackgroundColor = TRUE;

	
	Try
    {
        utarray_new(NewView->Array, &image_view_t_icd);
        
    }
    Catch(e)
    {
		if (NewView)
            egFreeView(NewView);
            
            return NULL;
    }
	NewView->isDirty = FALSE;
    NewView->AutoUpdate = FALSE;
    
    return NewView;
}

VOID egFreeView(IN EG_VIEW *View)
{
    CEXCEPTION_T e = CEXCEPTION_NONE;

    Try
    {
        ASSERT(View);
        
        if(View->Array)        
            utarray_free(View->Array);
        
        if (View->CompImage)
            egFreeImage(View->CompImage);
        
        if (View->Background)
            egFreeImage(View->Background);
        
        FreePool (View);
        View = NULL;
    }
    Catch(e)
    {
    }    
}

VOID egViewSetBackgroundColor(IN OUT EG_VIEW *CompView, IN EG_PIXEL *Color)
{
    if (CompView)
    {
        if (CompView->Background && Color)
        {
            if (CompView->Background->PixelData)
            {
                EG_PIXEL *BackgroundColor = (EG_PIXEL*)&CompView->Background->PixelData[0];
                ASSERT(BackgroundColor);
                BackgroundColor->r = Color->r;
                BackgroundColor->g = Color->g;
                BackgroundColor->b = Color->b;
                BackgroundColor->a = Color->a;
                CompView->useBackgroundColor = TRUE;
                CompView->isDirty = TRUE;
                if (CompView->AutoUpdate) {
                    egViewUpdate(CompView);
                }
            }
            
        }
    }
}

VOID egUseBackgroundColor(IN OUT EG_VIEW *CompView, IN BOOLEAN Use)
{
    if (CompView)
    {              
                
          CompView->useBackgroundColor = Use;
        
    }
}

EG_PIXEL * egViewGetBackgroundColor(IN OUT EG_VIEW *CompView)
{
    EG_PIXEL * Color = NULL ;
    
    
    if (CompView->useBackgroundColor == TRUE)
    {        
        if (CompView)
        {
            if (CompView->Background)
            {
                if (CompView->Background->PixelData) {
                    Color = (EG_PIXEL*)AllocateZeroPool(sizeof(EG_PIXEL)) ;
                    ASSERT(Color);

                    Color->r = CompView->Background->PixelData[0].r ;
                    Color->g = CompView->Background->PixelData[0].g ;
                    Color->b = CompView->Background->PixelData[0].b ;
                    Color->a = CompView->Background->PixelData[0].a ;
                }
            }
        }
    }
    
    
    return Color;
}

VOID egViewSetBackground(IN OUT EG_VIEW *CompView, IN EG_IMAGE *Background)
{
    if (CompView)
    {
        if (CompView->Background && Background)
        {
            egCopyImage(CompView->Background, Background);
            CompView->isDirty = TRUE;
            CompView->useBackgroundColor = FALSE;

            if (CompView->AutoUpdate) {
                egViewUpdate(CompView);
            }
        }
    }
}

EG_IMAGE_VIEW *egViewAddImageView(IN OUT EG_VIEW *CompView, IN EG_IMAGE_VIEW *TopView)
{
    CEXCEPTION_T e = CEXCEPTION_NONE;
    EG_IMAGE_VIEW *NewImageView = NULL;
    Try
    {
        ASSERT(CompView);
        ASSERT(CompView->Array);
        ASSERT(TopView);
        
        utarray_push_back(CompView->Array, TopView);
        NewImageView = (EG_IMAGE_VIEW *)utarray_back(CompView->Array);
        ASSERT(NewImageView);

        CompView->isDirty = TRUE;
        if (CompView->AutoUpdate) {
            egViewUpdate(CompView);
        }
        
    }
    Catch(e)
    {
    }
    
    return NewImageView;
    
}

VOID egViewRemoveImageView(IN OUT EG_VIEW *CompView, IN EG_IMAGE_VIEW *ImageView)
{
    CEXCEPTION_T e = CEXCEPTION_NONE;
    Try
    {
        ASSERT(CompView);
        ASSERT(CompView->Array);
        ASSERT(ImageView);
        
        UINTN Index = utarray_eltidx(CompView->Array,ImageView);
        utarray_erase(CompView->Array,Index,1);
        CompView->isDirty = TRUE;
                
        if (CompView->AutoUpdate) {
            egViewUpdate(CompView);
        }
        
    }
    Catch(e)
    {
    }
}

VOID egViewRemoveImageViewByName(IN OUT EG_VIEW *CompView, IN CHAR16 *Name, IN UINTN NameSize)
{
    CEXCEPTION_T e = CEXCEPTION_NONE;
	EG_IMAGE_VIEW *p = NULL;
    Try
    {
        ASSERT(CompView);
        ASSERT(CompView->Array);
        ASSERT(Name);
        ASSERT((NameSize > 0));
        
        while( (p=(EG_IMAGE_VIEW*)utarray_next(CompView->Array,p))) {
            if (p->Image)
            {
                if (p->Name)
                {
                    if (StrnCmp(p->Name, Name, NameSize) == 0)
                    {
                        UINTN Index = utarray_eltidx(CompView->Array,p);
                        utarray_erase(CompView->Array,Index,1);
                        CompView->isDirty = TRUE;
                        break;
                        
                    }
                }
                
            }
        }
        
        if (CompView->AutoUpdate) {
            egViewUpdate(CompView);
        }
        
    }
    Catch(e)
    {
    }    
}
