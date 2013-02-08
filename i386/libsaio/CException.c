/*
 * contains some ideas from the Mark VanderVoord's CException project
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
#include "CException.h"
#include "libsaio.h"
#include "modules.h"
#include "assert.h"

static CEXCEPTION_FRAME_T *CExceptionFrames = NULL;
static CEXCEPTION_FRAME_T *CExceptionMainFrames = NULL;

#ifdef DEBUG_EXCEPTION
static unsigned long frameCount = 0;
#endif	

//------------------------------------------------------------------------------------------
//  Install or Restore Default handler
//------------------------------------------------------------------------------------------
int Install_Default_Handler(void)
{
#define XTRY \
{																\
pushFrame();                                                \
if ((e = setjmp(CExceptionFrames->cx)) == 0) {                                \
e = CEXCEPTION_NONE;                                       \
if (1)
	
#define XCATCH(e)                                               \
else { }                                            \
}                                                           \
}																\
if (e != CEXCEPTION_NONE)
    
    CEXCEPTION_T e = CEXCEPTION_NONE;
	
	XTRY
	{
		CExceptionMainFrames = CExceptionFrames;
	}
	XCATCH(e)
	{
        if (e != EXIT_SUCCESS) {
            popFrame();
            halt();
        }
    }
    return 1;
}

//------------------------------------------------------------------------------------------
//  Throw
//------------------------------------------------------------------------------------------
void Throw(CEXCEPTION_T ExceptionID)
{    
	longjmp(CExceptionFrames->cx, ExceptionID);
    
}

void pushFrame(void)
{
	CEXCEPTION_FRAME_T* frame = (CEXCEPTION_FRAME_T*)malloc(sizeof(CEXCEPTION_FRAME_T));    
	assert( frame != NULL );
	bzero(frame,sizeof(CEXCEPTION_FRAME_T));	
	frame->pFrame = CExceptionFrames;
	CExceptionFrames = frame;
#ifdef DEBUG_EXCEPTION
	frameCount++;
#endif	
}

void popFrame(void)
{
    
	CEXCEPTION_FRAME_T* frame = NULL;
	assert( CExceptionFrames != NULL );
	frame = CExceptionFrames->pFrame;
	if (CExceptionFrames->zone) execute_hook("destroy_zone", CExceptionFrames->zone, NULL, NULL, NULL, NULL, NULL);
	free(CExceptionFrames);
	CExceptionFrames = frame;
#ifdef DEBUG_EXCEPTION
    if (frameCount) frameCount--;
#endif    
}

#ifdef DEBUG_EXCEPTION
unsigned long getFramesCount(void)
{    
	return frameCount;
}
#endif

CEXCEPTION_FRAME_T * getcurrentFrame(void)
{    
	return CExceptionFrames;
}

CEXCEPTION_FRAME_T * getMainFrames(void)
{    
	return CExceptionMainFrames;
}