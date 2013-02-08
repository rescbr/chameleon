/*++
 
 Copyright (c) 1998  Intel Corporation
 
 Module Name:
 
 print.c
 
 Abstract:
 
 
 
 
 Revision History
 
 --*/
#include <efi.h>



//
//
//


#define PRINT_STRING_LEN            200
#define PRINT_ITEM_BUFFER_LEN       100

typedef struct {
    BOOLEAN             Ascii;
    UINTN               Index;
    union {
        CHAR16          *pw;
        CHAR8           *pc;
    } un;
} POINTER;

#define pw	un.pw
#define pc	un.pc

typedef struct _pitem {
    
    POINTER     Item;
    CHAR16      Scratch[PRINT_ITEM_BUFFER_LEN];
    UINTN       Width;
    UINTN       FieldWidth;
    UINTN       *WidthParse;
    CHAR16      Pad;
    BOOLEAN     PadBefore;
    BOOLEAN     Comma;
    BOOLEAN     Long;
} PRINT_ITEM;


typedef struct _pstate {
    // Input
    POINTER     fmt;
    va_list     args;
    
    // Output
    CHAR16      *Buffer;
    CHAR16      *End;
    CHAR16      *Pos;
    UINTN       Len;
    
    UINTN       Attr;
    UINTN       RestoreAttr;
    
    UINTN       AttrNorm;
    UINTN       AttrHighlight;
    UINTN       AttrError;
    
    INTN        (*Output)(VOID *context, CHAR16 *str);
    INTN        (*SetAttr)(VOID *context, UINTN attr);
    VOID        *Context;
    
    // Current item being formatted
    struct _pitem  *Item;
} PRINT_STATE;


//
//
//

STATIC
UINTN
_Print (
        IN PRINT_STATE     *ps
        );


INTN
_SPrint (
         IN VOID     *Context,
         IN CHAR16   *Buffer
         );

INTN
_PoolPrint (
            IN VOID     *Context,
            IN CHAR16   *Buffer
            );

VOID
_PoolCatPrint (
               IN CHAR16           *fmt,
               IN va_list          args,
               IN OUT POOL_PRINT   *spc,
               IN INTN             (*Output)(VOID *context, CHAR16 *str)
               );

VOID
_PoolCatPrint (
               IN CHAR16           *fmt,
               IN va_list          args,
               IN OUT POOL_PRINT   *spc,
               IN INTN             (*Output)(VOID *context, CHAR16 *str)
               )
// Dispath function for SPrint, PoolPrint, and CatPrint
{
    PRINT_STATE         ps;
    
    ZeroMem (&ps, sizeof(ps));
    ps.Output  = Output;
    ps.Context = spc;
    ps.fmt.pw = fmt;
    va_copy(ps.args, args);
    _Print (&ps);
    va_end(ps.args);
}


INTN
_SPrint (
         IN VOID     *Context,
         IN CHAR16   *Buffer
         )
// Append string worker for SPrint, PoolPrint and CatPrint
{
    UINTN           len;
    POOL_PRINT      *spc;
    
    spc = Context;
    len = StrLen(Buffer);
    
    //
    // Is the string is over the max truncate it
    //
    
    if (spc->len + len > spc->maxlen) {
        len = spc->maxlen - spc->len;
    }
    
    //
    // Append the new text
    //
    
    CopyMem (spc->str + spc->len, Buffer, len * sizeof(CHAR16));
    spc->len += len;
    
    //
    // Null terminate it
    //
    
    if (spc->len < spc->maxlen) {
        spc->str[spc->len] = 0;
    } else if (spc->maxlen) {
        spc->str[spc->maxlen-1] = 0;
    }
    
    return 0;
}


INTN
_PoolPrint (
            IN VOID     *Context,
            IN CHAR16   *Buffer
            )
// Append string worker for PoolPrint and CatPrint
{
    UINTN           newlen;
    POOL_PRINT      *spc;
    
    spc = Context;
    newlen = spc->len + StrLen(Buffer) + 1;
    
    //
    // Is the string is over the max, grow the buffer
    //
    
    if (newlen > spc->maxlen) {
        
        //
        // Grow the pool buffer
        //
        
        newlen += PRINT_STRING_LEN;
        spc->maxlen = newlen;
        spc->str = ReallocatePool (                                 
                                   spc->len * sizeof(CHAR16),
                                   spc->maxlen * sizeof(CHAR16),
                                   spc->str
                                   );
        
        if (!spc->str) {
            spc->len = 0;
            spc->maxlen = 0;
        }
    }
    
    //
    // Append the new text
    //
    
    return _SPrint (Context, Buffer);
}




UINTN
SPrint (
        OUT CHAR16  *Str,
        IN UINTN    StrSize,
        IN CHAR16   *fmt,
        ...
        )
/*++
 
 Routine Description:
 
 Prints a formatted unicode string to a buffer
 
 Arguments:
 
 Str         - Output buffer to print the formatted string into
 
 StrSize     - Size of Str.  String is truncated to this size.
 A size of 0 means there is no limit
 
 fmt         - The format string
 
 Returns:
 
 String length returned in buffer
 
 --*/
{
    POOL_PRINT          spc;
    va_list             args;
    
    
    va_start (args, fmt);
    spc.str    = Str;
    spc.maxlen = StrSize / sizeof(CHAR16) - 1;
    spc.len    = 0;
    
    _PoolCatPrint (fmt, args, &spc, _SPrint);
    va_end (args);
    return spc.len;
}


CHAR16 *
PoolPrint (
           IN CHAR16           *fmt,
           ...
           )
/*++
 
 Routine Description:
 
 Prints a formatted unicode string to allocated pool.  The caller
 must free the resulting buffer.
 
 Arguments:
 
 fmt         - The format string
 
 Returns:
 
 Allocated buffer with the formatted string printed in it.
 The caller must free the allocated buffer.   The buffer
 allocation is not packed.
 
 --*/
{
    POOL_PRINT          spc;
    va_list             args;
    
    ZeroMem (&spc, sizeof(spc));
    va_start (args, fmt);
    _PoolCatPrint (fmt, args, &spc, _PoolPrint);
    va_end (args);
    return spc.str;
}



CHAR16 *
CatPrint (
          IN OUT POOL_PRINT   *Str,
          IN CHAR16           *fmt,
          ...
          )
/*++
 
 Routine Description:
 
 Concatenates a formatted unicode string to allocated pool.
 The caller must free the resulting buffer.
 
 Arguments:
 
 Str         - Tracks the allocated pool, size in use, and
 amount of pool allocated.
 
 fmt         - The format string
 
 Returns:
 
 Allocated buffer with the formatted string printed in it.
 The caller must free the allocated buffer.   The buffer
 allocation is not packed.
 
 --*/
{
    va_list             args;
    
    va_start (args, fmt);
    _PoolCatPrint (fmt, args, Str, _PoolPrint);
    va_end (args);
    return Str->str;
}


VOID
TimeToString (
              OUT CHAR16      *Buffer,
              IN EFI_TIME     *Time
              )
{
    UINTN       Hour, Year;
    CHAR16      AmPm;
    
    AmPm = 'a';
    Hour = Time->Hour;
    if (Time->Hour == 0) {
        Hour = 12;
    } else if (Time->Hour >= 12) {
        AmPm = 'p';
        if (Time->Hour >= 13) {
            Hour -= 12;
        }
    }
    
    Year = Time->Year % 100;
    
    // bugbug: for now just print it any old way
    SPrint (Buffer, 0, L"%02d/%02d/%02d  %02d:%02d%c",
            Time->Month,
            Time->Day,
            Year,
            Hour,
            Time->Minute,
            AmPm
            );
}

STATIC
VOID
PFLUSH (
        IN OUT PRINT_STATE     *ps
        )
{
    *ps->Pos = 0;
    //if (IsLocalPrint(ps->Output))
        ps->Output(ps->Context, ps->Buffer);
    //else
    //	uefi_call_wrapper(ps->Output, 2, ps->Context, ps->Buffer);
    ps->Pos = ps->Buffer;
}

STATIC
VOID
PSETATTR (
          IN OUT PRINT_STATE  *ps,
          IN UINTN             Attr
          )
{
    PFLUSH (ps);
    
    ps->RestoreAttr = ps->Attr;
    //if (ps->SetAttr) {
    //    uefi_call_wrapper(ps->SetAttr, 2, ps->Context, Attr);
    //}
    
    ps->Attr = Attr;
}

STATIC
VOID
PPUTC (
       IN OUT PRINT_STATE     *ps,
       IN CHAR16              c
       )
{
    // if this is a newline, add a carraige return
    if (c == '\n') {
        PPUTC (ps, '\r');
    }
    
    *ps->Pos = c;
    ps->Pos += 1;
    ps->Len += 1;
    
    // if at the end of the buffer, flush it
    if (ps->Pos >= ps->End) {
        PFLUSH(ps);
    }
}


STATIC
CHAR16
PGETC (
       IN POINTER      *p
       )
{
    CHAR16      c;
    
    c = p->Ascii ? p->pc[p->Index] : p->pw[p->Index];
    p->Index += 1;
    
    return  c;
}


STATIC
VOID
PITEM (
       IN OUT PRINT_STATE  *ps
       )
{
    UINTN               Len, i;
    PRINT_ITEM          *Item;
    CHAR16              c;
    
    // Get the length of the item
    Item = ps->Item;
    Item->Item.Index = 0;
    while (Item->Item.Index < Item->FieldWidth) {
        c = PGETC(&Item->Item);
        if (!c) {
            Item->Item.Index -= 1;
            break;
        }
    }
    Len = Item->Item.Index;
    
    // if there is no item field width, use the items width
    if (Item->FieldWidth == (UINTN) -1) {
        Item->FieldWidth = Len;
    }
    
    // if item is larger then width, update width
    if (Len > Item->Width) {
        Item->Width = Len;
    }
    
    
    // if pad field before, add pad char
    if (Item->PadBefore) {
        for (i=Item->Width; i < Item->FieldWidth; i+=1) {
            PPUTC (ps, ' ');
        }
    }
    
    // pad item
    for (i=Len; i < Item->Width; i++) {
        PPUTC (ps, Item->Pad);
    }
    
    // add the item
    Item->Item.Index=0;
    while (Item->Item.Index < Len) {
        PPUTC (ps, PGETC(&Item->Item));
    }
    
    // If pad at the end, add pad char
    if (!Item->PadBefore) {
        for (i=Item->Width; i < Item->FieldWidth; i+=1) {
            PPUTC (ps, ' ');
        }
    }
}

STATIC
UINTN
_Print (
        IN PRINT_STATE     *ps
        )
/*++
 
 Routine Description:
 
 %w.lF   -   w = width
 l = field width
 F = format of arg
 
 Args F:
 0       -   pad with zeros
 -       -   justify on left (default is on right)
 ,       -   add comma's to field
 *       -   width provided on stack
 n       -   Set output attribute to normal (for this field only)
 h       -   Set output attribute to highlight (for this field only)
 e       -   Set output attribute to error (for this field only)
 l       -   Value is 64 bits
 
 a       -   ascii string
 s       -   unicode string
 X       -   fixed 8 byte value in hex
 x       -   hex value
 d       -   value as decimal
 c       -   Unicode char
 t       -   EFI time structure
 g       -   Pointer to GUID
 r       -   EFI status code (result code)
 
 N       -   Set output attribute to normal
 H       -   Set output attribute to highlight
 E       -   Set output attribute to error
 %       -   Print a %
 
 Arguments:
 
 SystemTable     - The system table
 
 Returns:
 
 Number of charactors written
 
 --*/
{
    CHAR16          c;
    UINTN           Attr;
    PRINT_ITEM      Item;
    CHAR16          Buffer[PRINT_STRING_LEN];
    
    ps->Len = 0;
    ps->Buffer = Buffer;
    ps->Pos = Buffer;
    ps->End = Buffer + PRINT_STRING_LEN - 1;
    ps->Item = &Item;
    
    ps->fmt.Index = 0;
    while ((c = PGETC(&ps->fmt))) {
        
        if (c != '%') {
            PPUTC ( ps, c );
            continue;
        }
        
        // setup for new item
        Item.FieldWidth = (UINTN) -1;
        Item.Width = 0;
        Item.WidthParse = &Item.Width;
        Item.Pad = ' ';
        Item.PadBefore = TRUE;
        Item.Comma = FALSE;
        Item.Long = FALSE;
        Item.Item.Ascii = FALSE;
        Item.Item.pw = NULL;
        ps->RestoreAttr = 0;
        Attr = 0;
        
        while ((c = PGETC(&ps->fmt))) {
            
            switch (c) {
                    
                case '%':
                    //
                    // %% -> %
                    //
                    Item.Item.pw = Item.Scratch;
                    Item.Item.pw[0] = '%';
                    Item.Item.pw[1] = 0;
                    break;
                    
                case '0':
                    Item.Pad = '0';
                    break;
                    
                case '-':
                    Item.PadBefore = FALSE;
                    break;
                    
                case ',':
                    Item.Comma = TRUE;
                    break;
                    
                case '.':
                    Item.WidthParse = &Item.FieldWidth;
                    break;
                    
                case '*':
                    *Item.WidthParse = va_arg(ps->args, UINTN);
                    break;
                    
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    *Item.WidthParse = 0;
                    do {
                        *Item.WidthParse = *Item.WidthParse * 10 + c - '0';
                        c = PGETC(&ps->fmt);
                    } while (c >= '0'  &&  c <= '9') ;
                    ps->fmt.Index -= 1;
                    break;
                    
                case 'a':
                    Item.Item.pc = va_arg(ps->args, CHAR8 *);
                    Item.Item.Ascii = TRUE;
                    if (!Item.Item.pc) {
                        Item.Item.pc = (CHAR8 *)"(null)";
                    }
                    break;
                    
                case 's':
                    Item.Item.pw = va_arg(ps->args, CHAR16 *);
                    if (!Item.Item.pw) {
                        Item.Item.pw = L"(null)";
                    }
                    break;
                    
                case 'c':
                    Item.Item.pw = Item.Scratch;
                    Item.Item.pw[0] = (CHAR16) va_arg(ps->args, UINTN);
                    Item.Item.pw[1] = 0;
                    break;
                    
                case 'l':
                    Item.Long = TRUE;
                    break;
                    
                case 'X':
                    Item.Width = Item.Long ? 16 : 8;
                    Item.Pad = '0';
                case 'x':
                    Item.Item.pw = Item.Scratch;
                    ValueToHex (
                                Item.Item.pw,
                                Item.Long ? va_arg(ps->args, UINT64) : va_arg(ps->args, UINT32)
                                );
                    
                    break;
                    
                    
                case 'g':
                    //Item.Item.pw = Item.Scratch;
                    //GuidToString (Item.Item.pw, va_arg(ps->args, EFI_GUID *));
                    break;
                    
                case 'd':
                    Item.Item.pw = Item.Scratch;
                    ValueToString (
                                   Item.Item.pw,
                                   Item.Comma,
                                   Item.Long ? va_arg(ps->args, UINT64) : va_arg(ps->args, UINT32)
                                   );
                    break
                    ;
                case 't':
                    Item.Item.pw = Item.Scratch;
                    TimeToString (Item.Item.pw, va_arg(ps->args, EFI_TIME *));
                    break;
                    
                case 'r':
                    Item.Item.pw = Item.Scratch;
                    StatusToString (Item.Item.pw, va_arg(ps->args, EFI_STATUS));
                    break;
                    
                case 'n':
                    PSETATTR(ps, ps->AttrNorm);
                    break;
                    
                case 'h':
                    PSETATTR(ps, ps->AttrHighlight);
                    break;
                    
                case 'e':
                    PSETATTR(ps, ps->AttrError);
                    break;
                    
                case 'N':
                    Attr = ps->AttrNorm;
                    break;
                    
                case 'H':
                    Attr = ps->AttrHighlight;
                    break;
                    
                case 'E':
                    Attr = ps->AttrError;
                    break;
                    
                default:
                    Item.Item.pw = Item.Scratch;
                    Item.Item.pw[0] = '?';
                    Item.Item.pw[1] = 0;
                    break;
            }
            
            // if we have an Item
            if (Item.Item.pw) {
                PITEM (ps);
                break;
            }
            
            // if we have an Attr set
            if (Attr) {
                PSETATTR(ps, Attr);
                ps->RestoreAttr = 0;
                break;
            }
        }
        
        if (ps->RestoreAttr) {
            PSETATTR(ps, ps->RestoreAttr);
        }
    }
    
    // Flush buffer
    PFLUSH (ps);
    return ps->Len;
}

STATIC CHAR8 Hex[] = {'0','1','2','3','4','5','6','7',
    '8','9','A','B','C','D','E','F'};

VOID
ValueToHex (
            IN CHAR16   *Buffer,
            IN UINT64   v
            )
{
    CHAR8           str[30], *p1;
    CHAR16          *p2;
    
    if (!v) {
        Buffer[0] = '0';
        Buffer[1] = 0;
        return ;
    }
    
    p1 = str;
    p2 = Buffer;
    
    while (v) {
        *(p1++) = Hex[v & 0xf];
        v = RShiftU64 (v, 4);
    }
    
    while (p1 != str) {
        *(p2++) = *(--p1);
    }
    *p2 = 0;
}


VOID
ValueToString (
               IN CHAR16   *Buffer,
               IN BOOLEAN  Comma,
               IN INT64    v
               )
{
    STATIC CHAR8 ca[] = {  3, 1, 2 };
    CHAR8        str[40], *p1;
    CHAR16       *p2;
    UINTN        c, r;
    
    if (!v) {
        Buffer[0] = '0';
        Buffer[1] = 0;
        return ;
    }
    
    p1 = str;
    p2 = Buffer;
    
    if (v < 0) {
        *(p2++) = '-';
        v = -v;
    }
    
    while (v) {
        v = (INT64)DivU64x32 ((UINT64)v, 10, &r);
        *(p1++) = (CHAR8)r + '0';
    }
    
    c = (Comma ? ca[(p1 - str) % 3] : 999) + 1;
    while (p1 != str) {
        
        c -= 1;
        if (!c) {
            *(p2++) = ',';
            c = 3;
        }
        
        *(p2++) = *(--p1);
    }
    *p2 = 0;
}

struct {
    EFI_STATUS      Code;
    CHAR_W	    *Desc;
} ErrorCodeTable[] = {
	{  EFI_SUCCESS,                L"Success"},
	{  EFI_LOAD_ERROR,             L"Load Error"},
	{  EFI_INVALID_PARAMETER,      L"Invalid Parameter"},
	{  EFI_UNSUPPORTED,            L"Unsupported"},
	{  EFI_BAD_BUFFER_SIZE,        L"Bad Buffer Size"},
	{  EFI_BUFFER_TOO_SMALL,       L"Buffer Too Small"},
	{  EFI_NOT_READY,              L"Not Ready"},
	{  EFI_DEVICE_ERROR,           L"Device Error"},
	{  EFI_WRITE_PROTECTED,        L"Write Protected"},
	{  EFI_OUT_OF_RESOURCES,       L"Out of Resources"},
	{  EFI_VOLUME_CORRUPTED,       L"Volume Corrupt"},
	{  EFI_VOLUME_FULL,            L"Volume Full"},
	{  EFI_NO_MEDIA,               L"No Media"},
	{  EFI_MEDIA_CHANGED,          L"Media changed"},
	{  EFI_NOT_FOUND,              L"Not Found"},
	{  EFI_ACCESS_DENIED,          L"Access Denied"},
	{  EFI_NO_RESPONSE,            L"No Response"},
	{  EFI_NO_MAPPING,             L"No mapping"},
	{  EFI_TIMEOUT,                L"Time out"},
	{  EFI_NOT_STARTED,            L"Not started"},
	{  EFI_ALREADY_STARTED,        L"Already started"},
	{  EFI_ABORTED,                L"Aborted"},
	{  EFI_ICMP_ERROR,             L"ICMP Error"},
	{  EFI_TFTP_ERROR,             L"TFTP Error"},
	{  EFI_PROTOCOL_ERROR,         L"Protocol Error"},
    
	// warnings
	{  EFI_WARN_DELETE_FAILURE,    L"Warning Delete Failure"},
	{  EFI_WARN_WRITE_FAILURE,     L"Warning Write Failure"},
	{  EFI_WARN_BUFFER_TOO_SMALL,  L"Warning Buffer Too Small"},
	{  0, NULL}
} ;


VOID
StatusToString (
                OUT CHAR16          *Buffer,
                IN EFI_STATUS       Status
                )
{
    UINTN           Index;
    
    for (Index = 0; ErrorCodeTable[Index].Desc; Index +=1) {
        if (ErrorCodeTable[Index].Code == Status) {
            StrCpy (Buffer, ErrorCodeTable[Index].Desc);
            return;
        }
    }
    
    SPrint (Buffer, 0, L"%X", Status);
}
