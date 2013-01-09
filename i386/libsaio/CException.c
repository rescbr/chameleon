#include "CException.h"
#include "libsaio.h"

volatile CEXCEPTION_FRAME_T CExceptionFrames[CEXCEPTION_NUM_ID] = { {0} };

//------------------------------------------------------------------------------------------
//  Install or Restore Default handler
//------------------------------------------------------------------------------------------
int Install_Default_Handler(void)
{
#define XTRY Try
#define XCATCH(e)                                               \
else { }                                                    \
CExceptionFrames[MY_ID].Exception = CEXCEPTION_NONE;        \
}                                                           \
else                                                        \
{ e = CExceptionFrames[MY_ID].Exception; e=e; }             \
CExceptionFrames[MY_ID].pFrame = PrevFrame;                 \
}                                                           \
if (CExceptionFrames[CEXCEPTION_GET_ID].Exception != CEXCEPTION_NONE)
    
    CEXCEPTION_T e = CEXCEPTION_NONE;
	
	XTRY
	{
	}
	XCATCH(e)
	{
        halt();
    }
    return 1;
}

//------------------------------------------------------------------------------------------
//  Throw
//------------------------------------------------------------------------------------------
void Throw(CEXCEPTION_T ExceptionID)
{
    unsigned int MY_ID = CEXCEPTION_GET_ID;
    CExceptionFrames[MY_ID].Exception = ExceptionID;
    if (CExceptionFrames[MY_ID].pFrame)
    {
        longjmp(*CExceptionFrames[MY_ID].pFrame, 1);
    }
    CEXCEPTION_NO_CATCH_HANDLER(MY_ID);
}

//------------------------------------------------------------------------------------------
//  Explanation of what it's all for:
//------------------------------------------------------------------------------------------
/*
 #define Try
 {                                                                   <- give us some local scope.  most compilers are happy with this
 jmp_buf *PrevFrame, NewFrame;                                   <- prev frame points to the last try block's frame.  new frame gets created on stack for this Try block
 unsigned int MY_ID = CEXCEPTION_GET_ID;                         <- look up this task's id for use in frame array.  always 0 if single-tasking
 PrevFrame = CExceptionFrames[CEXCEPTION_GET_ID].pFrame;         <- set pointer to point at old frame (which array is currently pointing at)
 CExceptionFrames[MY_ID].pFrame = &NewFrame;                     <- set array to point at my new frame instead, now
 CExceptionFrames[MY_ID].Exception = CEXCEPTION_NONE;            <- initialize my exception id to be NONE
 if (setjmp(NewFrame) == 0) {                                    <- do setjmp.  it returns 1 if longjump called, otherwise 0
 if (1)                                                          <- this is here to force proper scoping.  it requires braces or a single line to be but after Try, otherwise won't compile.  This is always true at this point.
 if (1) is also always true and the compiler will not warn
 #define Catch(e)
 else { }                                                    <- this also forces proper scoping.  Without this they could stick their own 'else' in and it would get ugly
 CExceptionFrames[MY_ID].Exception = CEXCEPTION_NONE;        <- no errors happened, so just set the exception id to NONE (in case it was corrupted)
 }
 else                                                            <- an exception occurred
 { e = CExceptionFrames[MY_ID].Exception; e=e;}                  <- assign the caught exception id to the variable passed in.
 CExceptionFrames[MY_ID].pFrame = PrevFrame;                     <- make the pointer in the array point at the previous frame again, as if NewFrame never existed.
 }                                                                   <- finish off that local scope we created to have our own variables
 if (CExceptionFrames[CEXCEPTION_GET_ID].Exception != CEXCEPTION_NONE)  <- start the actual 'catch' processing if we have an exception id saved away
 if (Install_or_Restore_Default_Handler)                                <- and finally we re-install the default exceptions handler to be sure that throw, exit, assert, etc... point to the right place
 */

