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

#ifndef _CEXCEPTION_H
#define _CEXCEPTION_H

#include <setjmp.h>

#ifdef __cplusplus
extern "C"
{
#endif
	
#define CEXCEPTION_USE_CONFIG_FILE
//#define DEBUG_EXCEPTION

	//To Use CException, you have a number of options:
	//1. Just include it and run with the defaults
	//2. Define any of the following symbols at the command line to override them
	//3. Include a header file before CException.h everywhere which defines any of these
	//4. Create an Exception.h in your path, and just define EXCEPTION_USE_CONFIG_FILE first
	
#ifdef CEXCEPTION_USE_CONFIG_FILE
#include "CExceptionConfig.h"
#endif
	
	//This is the value to assign when there isn't an exception
#ifndef CEXCEPTION_NONE
#define CEXCEPTION_NONE      (0x5A5A5A5A)
#endif
    
#ifndef SIGABRT
#define SIGABRT    (-2)
#endif
    
	//This is number of exception stacks to keep track of (one per task)
#ifndef CEXCEPTION_NUM_ID
#define CEXCEPTION_NUM_ID    (1) //there is only the one stack by default
#endif
	
	//This is the method of getting the current exception stack index (0 if only one stack)
#ifndef CEXCEPTION_GET_ID
#define CEXCEPTION_GET_ID    (0) //use the first index always because there is only one anyway
#endif
	
	//The type to use to store the exception values.
#ifndef CEXCEPTION_T
#define CEXCEPTION_T         unsigned int
#endif
	
	//This is an optional special handler for when there is no global Catch
#ifndef CEXCEPTION_NO_CATCH_HANDLER
#define CEXCEPTION_NO_CATCH_HANDLER(id)
#endif
	
	//exception frame structures
	typedef struct CEXCEPTION_FRAME {
		jmp_buf cx;
		struct CEXCEPTION_FRAME *pFrame;   
		void *zone; 
		void *atexit;
		int new_registration;
		int err_no;
	} CEXCEPTION_FRAME_T;
		
    void pushFrame(void);
    void popFrame(void);
    unsigned long getFramesCount(void);
	CEXCEPTION_FRAME_T * getcurrentFrame(void);
	CEXCEPTION_FRAME_T * getMainFrames(void);

	//Try (see C file for explanation)
#define Try														\
{																\
    pushFrame();                                                \
    if ((e = setjmp(getcurrentFrame()->cx)) == 0) {                                \
        e = CEXCEPTION_NONE;                                       \
        if (1)
	
	//Catch (see C file for explanation)
#define Catch(e)													\
        else { }                                            \
    }                                                           \
    popFrame();\
}																\
if (e != CEXCEPTION_NONE)
	
    
	//Throw an Error
	void Throw(CEXCEPTION_T ExceptionID);
	int Install_Default_Handler(void);
	
#ifdef __cplusplus
}   // extern "C"
#endif


#endif // _CEXCEPTION_H
