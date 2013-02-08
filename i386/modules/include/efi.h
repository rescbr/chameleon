
#ifndef __LIBEFI_H__
#define __LIBEFI_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>
#include <pexpert/i386/efi.h>

#define ASSERT(x) assert(x)

typedef uint8_t BOOLEAN;
typedef int32_t INTN;
typedef uint32_t UINTN;
typedef int8_t INT8;
typedef uint8_t UINT8;
typedef int16_t INT16;
typedef uint16_t UINT16;
typedef int32_t INT32;
typedef uint32_t UINT32;
typedef int64_t INT64;
typedef uint64_t UINT64;
typedef char CHAR8;
typedef int CHAR16;
typedef UINT64 EFI_LBA;

//
// Modifiers to absract standard types to aid in debug of problems
//
#ifndef CONST
#define CONST     const
#endif

#ifndef STATIC
#define STATIC    static
#endif

#ifndef VOID
#define VOID      void
#endif

#ifndef VOLATILE
#define VOLATILE  volatile
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/**
 Return the maximum of two operands.
 
 This macro returns the maximum of two operand specified by a and b.
 Both a and b must be the same numerical types, signed or unsigned.
 
 @param   a        The first operand with any numerical type.
 @param   b        The second operand. Can be any numerical type as long as is
 the same type as a.
 
 @return  Maximum of two operands.
 
 **/
#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

/**
 Return the minimum of two operands.
 
 This macro returns the minimal of two operand specified by a and b.
 Both a and b must be the same numerical types, signed or unsigned.
 
 @param   a        The first operand with any numerical type.
 @param   b        The second operand. It should be the same any numerical type with a.
 
 @return  Minimum of two operands.
 
 **/
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

/**
 Return the absolute value of a signed operand.
 
 This macro returns the absolute value of the signed operand specified by a.
 
 @param   a        The signed operand.
 
 @return  The absolute value of the signed operand.
 
 **/
#ifndef ABS
#define ABS(a)                          \
(((a) < 0) ? (-(a)) : (a))
#endif

//
// Generate an ASSERT if Status is an error code
//
#define ASSERT_EFI_ERROR(status)  ASSERT(!EFI_ERROR(status))

typedef enum {
	EfiPciWidthUint8,
	EfiPciWidthUint16,
	EfiPciWidthUint32,
	EfiPciWidthUint64,
	EfiPciWidthFifoUint8,
	EfiPciWidthFifoUint16,
	EfiPciWidthFifoUint32,
	EfiPciWidthFifoUint64,
	EfiPciWidthFillUint8,
	EfiPciWidthFillUint16,
	EfiPciWidthFillUint32,
	EfiPciWidthFillUint64,
	EfiPciWidthMaximum
} EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH;

typedef enum {
	EfiBltVideoFill,
	EfiBltVideoToBltBuffer,
	EfiBltBufferToVideo,
	EfiBltVideoToVideo,
	EfiGraphicsOutputBltOperationMax
} EFI_GRAPHICS_OUTPUT_BLT_OPERATION;

typedef struct {
	UINT8 Blue;
	UINT8 Green;
	UINT8 Red;
	UINT8 Reserved;
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL;

typedef struct {
	UINT8 Blue;
	UINT8 Green;
	UINT8 Red;
	UINT8 Reserved;
} EFI_UGA_PIXEL;

VOID
CopyVideoBuffer (
				 IN  UINT8                 *VbeBuffer,
				 IN  VOID                  *MemAddress,
				 IN  UINTN                 DestinationX,
				 IN  UINTN                 DestinationY,
				 IN  UINTN                 TotalBytes,
				 IN  UINT32                VbePixelWidth,
				 IN  UINTN                 BytesPerScanLine
				 );

EFI_STATUS
BiosVideoGraphicsOutputVbeBlt (
							   IN OUT  EFI_GRAPHICS_OUTPUT_BLT_PIXEL  *VbeFrameBuffer,
							   IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL      *BltBuffer, OPTIONAL
							   IN  EFI_GRAPHICS_OUTPUT_BLT_OPERATION  BltOperation,
							   IN  UINTN                              SourceX,
							   IN  UINTN                              SourceY,
							   IN  UINTN                              DestinationX,
							   IN  UINTN                              DestinationY,
							   IN  UINTN                              Width,
							   IN  UINTN                              Height,
							   IN  UINTN                              Delta
							   );

EFI_STATUS
RootBridgeIoMemRW (
				   IN     BOOLEAN                                Write,
				   IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
				   IN     UINT64                                 Address,
				   IN     UINTN                                  Count,
				   IN OUT VOID                                   *Buffer
				   );

UINT64
MmioWrite64 (
			 IN      UINTN                     Address,
			 IN      UINT64                    Value
			 );
UINT64
MmioRead64 (
			IN      UINTN                     Address
			);

UINT32
MmioWrite32 (
			 IN      UINTN                     Address,
			 IN      UINT32                    Value
			 );

UINT32
MmioRead32 (
			IN      UINTN                     Address
			);

UINT16
MmioWrite16 (
			 IN      UINTN                     Address,
			 IN      UINT16                    Value
			 );

UINT16
MmioRead16 (
			IN      UINTN                     Address
			);

UINT8
MmioWrite8 (
			IN      UINTN                     Address,
			IN      UINT8                     Value
			);

UINT8
MmioRead8 (
		   IN      UINTN                     Address
		   );

UINT64
IoWrite64 (
		   IN      UINTN                     Port,
		   IN      UINT64                    Value
		   );

UINT64
IoRead64 (
		  IN      UINTN                     Port
		  );

//
// String Services
//

/**
 Copies one Null-terminated Unicode string to another Null-terminated Unicode
 string and returns the new Unicode string.
 
 This function copies the contents of the Unicode string Source to the Unicode
 string Destination, and returns Destination. If Source and Destination
 overlap, then the results are undefined.
 
 If Destination is NULL, then ASSERT().
 If Destination is not aligned on a 16-bit boundary, then ASSERT().
 If Source is NULL, then ASSERT().
 If Source is not aligned on a 16-bit boundary, then ASSERT().
 If Source and Destination overlap, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Source contains more than
 PcdMaximumUnicodeStringLength Unicode characters not including the
 Null-terminator, then ASSERT().
 
 @param  Destination The pointer to a Null-terminated Unicode string.
 @param  Source      The pointer to a Null-terminated Unicode string.
 
 @return Destination.
 
 **/
CHAR16 *
StrCpy (
        OUT     CHAR16                    *Destination,
        IN      CONST CHAR16              *Source
        );


/**
 Copies up to a specified length from one Null-terminated Unicode string to
 another Null-terminated Unicode string and returns the new Unicode string.
 
 This function copies the contents of the Unicode string Source to the Unicode
 string Destination, and returns Destination. At most, Length Unicode
 characters are copied from Source to Destination. If Length is 0, then
 Destination is returned unmodified. If Length is greater that the number of
 Unicode characters in Source, then Destination is padded with Null Unicode
 characters. If Source and Destination overlap, then the results are
 undefined.
 
 If Length > 0 and Destination is NULL, then ASSERT().
 If Length > 0 and Destination is not aligned on a 16-bit boundary, then ASSERT().
 If Length > 0 and Source is NULL, then ASSERT().
 If Length > 0 and Source is not aligned on a 16-bit boundary, then ASSERT().
 If Source and Destination overlap, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Length is greater than
 PcdMaximumUnicodeStringLength, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Source contains more than
 PcdMaximumUnicodeStringLength Unicode characters, not including the Null-terminator,
 then ASSERT().
 
 @param  Destination The pointer to a Null-terminated Unicode string.
 @param  Source      The pointer to a Null-terminated Unicode string.
 @param  Length      The maximum number of Unicode characters to copy.
 
 @return Destination.
 
 **/
CHAR16 *
StrnCpy (
         OUT     CHAR16                    *Destination,
         IN      CONST CHAR16              *Source,
         IN      UINTN                     Length
         );


/**
 Returns the length of a Null-terminated Unicode string.
 
 This function returns the number of Unicode characters in the Null-terminated
 Unicode string specified by String.
 
 If String is NULL, then ASSERT().
 If String is not aligned on a 16-bit boundary, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and String contains more than
 PcdMaximumUnicodeStringLength Unicode characters not including the
 Null-terminator, then ASSERT().
 
 @param  String  Pointer to a Null-terminated Unicode string.
 
 @return The length of String.
 
 **/
UINTN
StrLen (
        IN      CONST CHAR16              *String
        );


/**
 Returns the size of a Null-terminated Unicode string in bytes, including the
 Null terminator.
 
 This function returns the size, in bytes, of the Null-terminated Unicode string
 specified by String.
 
 If String is NULL, then ASSERT().
 If String is not aligned on a 16-bit boundary, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and String contains more than
 PcdMaximumUnicodeStringLength Unicode characters not including the
 Null-terminator, then ASSERT().
 
 @param  String  The pointer to a Null-terminated Unicode string.
 
 @return The size of String.
 
 **/
UINTN
StrSize (
         IN      CONST CHAR16              *String
         );


/**
 Compares two Null-terminated Unicode strings, and returns the difference
 between the first mismatched Unicode characters.
 
 This function compares the Null-terminated Unicode string FirstString to the
 Null-terminated Unicode string SecondString. If FirstString is identical to
 SecondString, then 0 is returned. Otherwise, the value returned is the first
 mismatched Unicode character in SecondString subtracted from the first
 mismatched Unicode character in FirstString.
 
 If FirstString is NULL, then ASSERT().
 If FirstString is not aligned on a 16-bit boundary, then ASSERT().
 If SecondString is NULL, then ASSERT().
 If SecondString is not aligned on a 16-bit boundary, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and FirstString contains more
 than PcdMaximumUnicodeStringLength Unicode characters not including the
 Null-terminator, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and SecondString contains more
 than PcdMaximumUnicodeStringLength Unicode characters, not including the
 Null-terminator, then ASSERT().
 
 @param  FirstString   The pointer to a Null-terminated Unicode string.
 @param  SecondString  The pointer to a Null-terminated Unicode string.
 
 @retval 0      FirstString is identical to SecondString.
 @return others FirstString is not identical to SecondString.
 
 **/
INTN
StrCmp (
        IN      CONST CHAR16              *FirstString,
        IN      CONST CHAR16              *SecondString
        );


/**
 Compares up to a specified length the contents of two Null-terminated Unicode strings,
 and returns the difference between the first mismatched Unicode characters.
 
 This function compares the Null-terminated Unicode string FirstString to the
 Null-terminated Unicode string SecondString. At most, Length Unicode
 characters will be compared. If Length is 0, then 0 is returned. If
 FirstString is identical to SecondString, then 0 is returned. Otherwise, the
 value returned is the first mismatched Unicode character in SecondString
 subtracted from the first mismatched Unicode character in FirstString.
 
 If Length > 0 and FirstString is NULL, then ASSERT().
 If Length > 0 and FirstString is not aligned on a 16-bit boundary, then ASSERT().
 If Length > 0 and SecondString is NULL, then ASSERT().
 If Length > 0 and SecondString is not aligned on a 16-bit boundary, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Length is greater than
 PcdMaximumUnicodeStringLength, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and FirstString contains more than
 PcdMaximumUnicodeStringLength Unicode characters, not including the Null-terminator,
 then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and SecondString contains more than
 PcdMaximumUnicodeStringLength Unicode characters, not including the Null-terminator,
 then ASSERT().
 
 @param  FirstString   The pointer to a Null-terminated Unicode string.
 @param  SecondString  The pointer to a Null-terminated Unicode string.
 @param  Length        The maximum number of Unicode characters to compare.
 
 @retval 0      FirstString is identical to SecondString.
 @return others FirstString is not identical to SecondString.
 
 **/
INTN
StrnCmp (
         IN      CONST CHAR16              *FirstString,
         IN      CONST CHAR16              *SecondString,
         IN      UINTN                     Length
         );


/**
 Concatenates one Null-terminated Unicode string to another Null-terminated
 Unicode string, and returns the concatenated Unicode string.
 
 This function concatenates two Null-terminated Unicode strings. The contents
 of Null-terminated Unicode string Source are concatenated to the end of
 Null-terminated Unicode string Destination. The Null-terminated concatenated
 Unicode String is returned. If Source and Destination overlap, then the
 results are undefined.
 
 If Destination is NULL, then ASSERT().
 If Destination is not aligned on a 16-bit boundary, then ASSERT().
 If Source is NULL, then ASSERT().
 If Source is not aligned on a 16-bit boundary, then ASSERT().
 If Source and Destination overlap, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Destination contains more
 than PcdMaximumUnicodeStringLength Unicode characters, not including the
 Null-terminator, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Source contains more than
 PcdMaximumUnicodeStringLength Unicode characters, not including the
 Null-terminator, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and concatenating Destination
 and Source results in a Unicode string with more than
 PcdMaximumUnicodeStringLength Unicode characters, not including the
 Null-terminator, then ASSERT().
 
 @param  Destination The pointer to a Null-terminated Unicode string.
 @param  Source      The pointer to a Null-terminated Unicode string.
 
 @return Destination.
 
 **/
CHAR16 *
StrCat (
        IN OUT  CHAR16                    *Destination,
        IN      CONST CHAR16              *Source
        );


/**
 Concatenates up to a specified length one Null-terminated Unicode to the end
 of another Null-terminated Unicode string, and returns the concatenated
 Unicode string.
 
 This function concatenates two Null-terminated Unicode strings. The contents
 of Null-terminated Unicode string Source are concatenated to the end of
 Null-terminated Unicode string Destination, and Destination is returned. At
 most, Length Unicode characters are concatenated from Source to the end of
 Destination, and Destination is always Null-terminated. If Length is 0, then
 Destination is returned unmodified. If Source and Destination overlap, then
 the results are undefined.
 
 If Destination is NULL, then ASSERT().
 If Length > 0 and Destination is not aligned on a 16-bit boundary, then ASSERT().
 If Length > 0 and Source is NULL, then ASSERT().
 If Length > 0 and Source is not aligned on a 16-bit boundary, then ASSERT().
 If Source and Destination overlap, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Length is greater than
 PcdMaximumUnicodeStringLength, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Destination contains more
 than PcdMaximumUnicodeStringLength Unicode characters, not including the
 Null-terminator, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Source contains more than
 PcdMaximumUnicodeStringLength Unicode characters, not including the
 Null-terminator, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and concatenating Destination
 and Source results in a Unicode string with more than PcdMaximumUnicodeStringLength
 Unicode characters, not including the Null-terminator, then ASSERT().
 
 @param  Destination The pointer to a Null-terminated Unicode string.
 @param  Source      The pointer to a Null-terminated Unicode string.
 @param  Length      The maximum number of Unicode characters to concatenate from
 Source.
 
 @return Destination.
 
 **/
CHAR16 *
StrnCat (
         IN OUT  CHAR16                    *Destination,
         IN      CONST CHAR16              *Source,
         IN      UINTN                     Length
         );

/**
 Returns the first occurrence of a Null-terminated Unicode sub-string
 in a Null-terminated Unicode string.
 
 This function scans the contents of the Null-terminated Unicode string
 specified by String and returns the first occurrence of SearchString.
 If SearchString is not found in String, then NULL is returned.  If
 the length of SearchString is zero, then String is returned.
 
 If String is NULL, then ASSERT().
 If String is not aligned on a 16-bit boundary, then ASSERT().
 If SearchString is NULL, then ASSERT().
 If SearchString is not aligned on a 16-bit boundary, then ASSERT().
 
 If PcdMaximumUnicodeStringLength is not zero, and SearchString
 or String contains more than PcdMaximumUnicodeStringLength Unicode
 characters, not including the Null-terminator, then ASSERT().
 
 @param  String          The pointer to a Null-terminated Unicode string.
 @param  SearchString    The pointer to a Null-terminated Unicode string to search for.
 
 @retval NULL            If the SearchString does not appear in String.
 @return others          If there is a match.
 
 **/
CHAR16 *
StrStr (
        IN      CONST CHAR16              *String,
        IN      CONST CHAR16              *SearchString
        );

/**
 Convert a Null-terminated Unicode decimal string to a value of
 type UINTN.
 
 This function returns a value of type UINTN by interpreting the contents
 of the Unicode string specified by String as a decimal number. The format
 of the input Unicode string String is:
 
 [spaces] [decimal digits].
 
 The valid decimal digit character is in the range [0-9]. The
 function will ignore the pad space, which includes spaces or
 tab characters, before [decimal digits]. The running zero in the
 beginning of [decimal digits] will be ignored. Then, the function
 stops at the first character that is a not a valid decimal character
 or a Null-terminator, whichever one comes first.
 
 If String is NULL, then ASSERT().
 If String is not aligned in a 16-bit boundary, then ASSERT().
 If String has only pad spaces, then 0 is returned.
 If String has no pad spaces or valid decimal digits,
 then 0 is returned.
 If the number represented by String overflows according
 to the range defined by UINTN, then ASSERT().
 
 If PcdMaximumUnicodeStringLength is not zero, and String contains
 more than PcdMaximumUnicodeStringLength Unicode characters not including
 the Null-terminator, then ASSERT().
 
 @param  String      The pointer to a Null-terminated Unicode string.
 
 @retval Value translated from String.
 
 **/
UINTN
StrDecimalToUintn (
                   IN      CONST CHAR16              *String
                   );

/**
 Convert a Null-terminated Unicode decimal string to a value of
 type UINT64.
 
 This function returns a value of type UINT64 by interpreting the contents
 of the Unicode string specified by String as a decimal number. The format
 of the input Unicode string String is:
 
 [spaces] [decimal digits].
 
 The valid decimal digit character is in the range [0-9]. The
 function will ignore the pad space, which includes spaces or
 tab characters, before [decimal digits]. The running zero in the
 beginning of [decimal digits] will be ignored. Then, the function
 stops at the first character that is a not a valid decimal character
 or a Null-terminator, whichever one comes first.
 
 If String is NULL, then ASSERT().
 If String is not aligned in a 16-bit boundary, then ASSERT().
 If String has only pad spaces, then 0 is returned.
 If String has no pad spaces or valid decimal digits,
 then 0 is returned.
 If the number represented by String overflows according
 to the range defined by UINT64, then ASSERT().
 
 If PcdMaximumUnicodeStringLength is not zero, and String contains
 more than PcdMaximumUnicodeStringLength Unicode characters not including
 the Null-terminator, then ASSERT().
 
 @param  String          The pointer to a Null-terminated Unicode string.
 
 @retval Value translated from String.
 
 **/
UINT64
StrDecimalToUint64 (
                    IN      CONST CHAR16              *String
                    );


/**
 Convert a Null-terminated Unicode hexadecimal string to a value of type UINTN.
 
 This function returns a value of type UINTN by interpreting the contents
 of the Unicode string specified by String as a hexadecimal number.
 The format of the input Unicode string String is:
 
 [spaces][zeros][x][hexadecimal digits].
 
 The valid hexadecimal digit character is in the range [0-9], [a-f] and [A-F].
 The prefix "0x" is optional. Both "x" and "X" is allowed in "0x" prefix.
 If "x" appears in the input string, it must be prefixed with at least one 0.
 The function will ignore the pad space, which includes spaces or tab characters,
 before [zeros], [x] or [hexadecimal digit]. The running zero before [x] or
 [hexadecimal digit] will be ignored. Then, the decoding starts after [x] or the
 first valid hexadecimal digit. Then, the function stops at the first character
 that is a not a valid hexadecimal character or NULL, whichever one comes first.
 
 If String is NULL, then ASSERT().
 If String is not aligned in a 16-bit boundary, then ASSERT().
 If String has only pad spaces, then zero is returned.
 If String has no leading pad spaces, leading zeros or valid hexadecimal digits,
 then zero is returned.
 If the number represented by String overflows according to the range defined by
 UINTN, then ASSERT().
 
 If PcdMaximumUnicodeStringLength is not zero, and String contains more than
 PcdMaximumUnicodeStringLength Unicode characters not including the Null-terminator,
 then ASSERT().
 
 @param  String          The pointer to a Null-terminated Unicode string.
 
 @retval Value translated from String.
 
 **/
UINTN
StrHexToUintn (
               IN      CONST CHAR16              *String
               );


/**
 Convert a Null-terminated Unicode hexadecimal string to a value of type UINT64.
 
 This function returns a value of type UINT64 by interpreting the contents
 of the Unicode string specified by String as a hexadecimal number.
 The format of the input Unicode string String is
 
 [spaces][zeros][x][hexadecimal digits].
 
 The valid hexadecimal digit character is in the range [0-9], [a-f] and [A-F].
 The prefix "0x" is optional. Both "x" and "X" is allowed in "0x" prefix.
 If "x" appears in the input string, it must be prefixed with at least one 0.
 The function will ignore the pad space, which includes spaces or tab characters,
 before [zeros], [x] or [hexadecimal digit]. The running zero before [x] or
 [hexadecimal digit] will be ignored. Then, the decoding starts after [x] or the
 first valid hexadecimal digit. Then, the function stops at the first character that is
 a not a valid hexadecimal character or NULL, whichever one comes first.
 
 If String is NULL, then ASSERT().
 If String is not aligned in a 16-bit boundary, then ASSERT().
 If String has only pad spaces, then zero is returned.
 If String has no leading pad spaces, leading zeros or valid hexadecimal digits,
 then zero is returned.
 If the number represented by String overflows according to the range defined by
 UINT64, then ASSERT().
 
 If PcdMaximumUnicodeStringLength is not zero, and String contains more than
 PcdMaximumUnicodeStringLength Unicode characters not including the Null-terminator,
 then ASSERT().
 
 @param  String          The pointer to a Null-terminated Unicode string.
 
 @retval Value translated from String.
 
 **/
UINT64
StrHexToUint64 (
                IN      CONST CHAR16             *String
                );

/**
 Convert a Null-terminated Unicode string to a Null-terminated
 ASCII string and returns the ASCII string.
 
 This function converts the content of the Unicode string Source
 to the ASCII string Destination by copying the lower 8 bits of
 each Unicode character. It returns Destination.
 
 The caller is responsible to make sure Destination points to a buffer with size
 equal or greater than ((StrLen (Source) + 1) * sizeof (CHAR8)) in bytes.
 
 If any Unicode characters in Source contain non-zero value in
 the upper 8 bits, then ASSERT().
 
 If Destination is NULL, then ASSERT().
 If Source is NULL, then ASSERT().
 If Source is not aligned on a 16-bit boundary, then ASSERT().
 If Source and Destination overlap, then ASSERT().
 
 If PcdMaximumUnicodeStringLength is not zero, and Source contains
 more than PcdMaximumUnicodeStringLength Unicode characters not including
 the Null-terminator, then ASSERT().
 
 If PcdMaximumAsciiStringLength is not zero, and Source contains more
 than PcdMaximumAsciiStringLength Unicode characters not including the
 Null-terminator, then ASSERT().
 
 @param  Source        The pointer to a Null-terminated Unicode string.
 @param  Destination   The pointer to a Null-terminated ASCII string.
 
 @return Destination.
 
 **/
CHAR8 *
UnicodeStrToAsciiStr (
                      IN      CONST CHAR16              *Source,
                      OUT     CHAR8                     *Destination
                      );


/**
 Copies one Null-terminated ASCII string to another Null-terminated ASCII
 string and returns the new ASCII string.
 
 This function copies the contents of the ASCII string Source to the ASCII
 string Destination, and returns Destination. If Source and Destination
 overlap, then the results are undefined.
 
 If Destination is NULL, then ASSERT().
 If Source is NULL, then ASSERT().
 If Source and Destination overlap, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and Source contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 
 @param  Destination The pointer to a Null-terminated ASCII string.
 @param  Source      The pointer to a Null-terminated ASCII string.
 
 @return Destination
 
 **/
CHAR8 *
AsciiStrCpy (
             OUT     CHAR8                     *Destination,
             IN      CONST CHAR8               *Source
             );


/**
 Copies up to a specified length one Null-terminated ASCII string to another
 Null-terminated ASCII string and returns the new ASCII string.
 
 This function copies the contents of the ASCII string Source to the ASCII
 string Destination, and returns Destination. At most, Length ASCII characters
 are copied from Source to Destination. If Length is 0, then Destination is
 returned unmodified. If Length is greater that the number of ASCII characters
 in Source, then Destination is padded with Null ASCII characters. If Source
 and Destination overlap, then the results are undefined.
 
 If Destination is NULL, then ASSERT().
 If Source is NULL, then ASSERT().
 If Source and Destination overlap, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and Length is greater than
 PcdMaximumAsciiStringLength, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and Source contains more than
 PcdMaximumAsciiStringLength ASCII characters, not including the Null-terminator,
 then ASSERT().
 
 @param  Destination The pointer to a Null-terminated ASCII string.
 @param  Source      The pointer to a Null-terminated ASCII string.
 @param  Length      The maximum number of ASCII characters to copy.
 
 @return Destination
 
 **/
CHAR8 *
AsciiStrnCpy (
              OUT     CHAR8                     *Destination,
              IN      CONST CHAR8               *Source,
              IN      UINTN                     Length
              );


/**
 Returns the length of a Null-terminated ASCII string.
 
 This function returns the number of ASCII characters in the Null-terminated
 ASCII string specified by String.
 
 If Length > 0 and Destination is NULL, then ASSERT().
 If Length > 0 and Source is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and String contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 
 @param  String  The pointer to a Null-terminated ASCII string.
 
 @return The length of String.
 
 **/
UINTN
AsciiStrLen (
             IN      CONST CHAR8               *String
             );


/**
 Returns the size of a Null-terminated ASCII string in bytes, including the
 Null terminator.
 
 This function returns the size, in bytes, of the Null-terminated ASCII string
 specified by String.
 
 If String is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and String contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 
 @param  String  The pointer to a Null-terminated ASCII string.
 
 @return The size of String.
 
 **/
UINTN
AsciiStrSize (
              IN      CONST CHAR8               *String
              );


/**
 Compares two Null-terminated ASCII strings, and returns the difference
 between the first mismatched ASCII characters.
 
 This function compares the Null-terminated ASCII string FirstString to the
 Null-terminated ASCII string SecondString. If FirstString is identical to
 SecondString, then 0 is returned. Otherwise, the value returned is the first
 mismatched ASCII character in SecondString subtracted from the first
 mismatched ASCII character in FirstString.
 
 If FirstString is NULL, then ASSERT().
 If SecondString is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and FirstString contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and SecondString contains more
 than PcdMaximumAsciiStringLength ASCII characters not including the
 Null-terminator, then ASSERT().
 
 @param  FirstString   The pointer to a Null-terminated ASCII string.
 @param  SecondString  The pointer to a Null-terminated ASCII string.
 
 @retval ==0      FirstString is identical to SecondString.
 @retval !=0      FirstString is not identical to SecondString.
 
 **/
INTN
AsciiStrCmp (
             IN      CONST CHAR8               *FirstString,
             IN      CONST CHAR8               *SecondString
             );


/**
 Performs a case insensitive comparison of two Null-terminated ASCII strings,
 and returns the difference between the first mismatched ASCII characters.
 
 This function performs a case insensitive comparison of the Null-terminated
 ASCII string FirstString to the Null-terminated ASCII string SecondString. If
 FirstString is identical to SecondString, then 0 is returned. Otherwise, the
 value returned is the first mismatched lower case ASCII character in
 SecondString subtracted from the first mismatched lower case ASCII character
 in FirstString.
 
 If FirstString is NULL, then ASSERT().
 If SecondString is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and FirstString contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and SecondString contains more
 than PcdMaximumAsciiStringLength ASCII characters not including the
 Null-terminator, then ASSERT().
 
 @param  FirstString   The pointer to a Null-terminated ASCII string.
 @param  SecondString  The pointer to a Null-terminated ASCII string.
 
 @retval ==0    FirstString is identical to SecondString using case insensitive
 comparisons.
 @retval !=0    FirstString is not identical to SecondString using case
 insensitive comparisons.
 
 **/
INTN
AsciiStriCmp (
              IN      CONST CHAR8               *FirstString,
              IN      CONST CHAR8               *SecondString
              );


/**
 Compares two Null-terminated ASCII strings with maximum lengths, and returns
 the difference between the first mismatched ASCII characters.
 
 This function compares the Null-terminated ASCII string FirstString to the
 Null-terminated ASCII  string SecondString. At most, Length ASCII characters
 will be compared. If Length is 0, then 0 is returned. If FirstString is
 identical to SecondString, then 0 is returned. Otherwise, the value returned
 is the first mismatched ASCII character in SecondString subtracted from the
 first mismatched ASCII character in FirstString.
 
 If Length > 0 and FirstString is NULL, then ASSERT().
 If Length > 0 and SecondString is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and Length is greater than
 PcdMaximumAsciiStringLength, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and FirstString contains more than
 PcdMaximumAsciiStringLength ASCII characters, not including the Null-terminator,
 then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and SecondString contains more than
 PcdMaximumAsciiStringLength ASCII characters, not including the Null-terminator,
 then ASSERT().
 
 @param  FirstString   The pointer to a Null-terminated ASCII string.
 @param  SecondString  The pointer to a Null-terminated ASCII string.
 @param  Length        The maximum number of ASCII characters for compare.
 
 @retval ==0       FirstString is identical to SecondString.
 @retval !=0       FirstString is not identical to SecondString.
 
 **/
INTN
AsciiStrnCmp (
              IN      CONST CHAR8               *FirstString,
              IN      CONST CHAR8               *SecondString,
              IN      UINTN                     Length
              );


/**
 Concatenates one Null-terminated ASCII string to another Null-terminated
 ASCII string, and returns the concatenated ASCII string.
 
 This function concatenates two Null-terminated ASCII strings. The contents of
 Null-terminated ASCII string Source are concatenated to the end of Null-
 terminated ASCII string Destination. The Null-terminated concatenated ASCII
 String is returned.
 
 If Destination is NULL, then ASSERT().
 If Source is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and Destination contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and Source contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and concatenating Destination and
 Source results in a ASCII string with more than PcdMaximumAsciiStringLength
 ASCII characters, then ASSERT().
 
 @param  Destination The pointer to a Null-terminated ASCII string.
 @param  Source      The pointer to a Null-terminated ASCII string.
 
 @return Destination
 
 **/
CHAR8 *
AsciiStrCat (
             IN OUT CHAR8    *Destination,
             IN CONST CHAR8  *Source
             );


/**
 Concatenates up to a specified length one Null-terminated ASCII string to
 the end of another Null-terminated ASCII string, and returns the
 concatenated ASCII string.
 
 This function concatenates two Null-terminated ASCII strings. The contents
 of Null-terminated ASCII string Source are concatenated to the end of Null-
 terminated ASCII string Destination, and Destination is returned. At most,
 Length ASCII characters are concatenated from Source to the end of
 Destination, and Destination is always Null-terminated. If Length is 0, then
 Destination is returned unmodified. If Source and Destination overlap, then
 the results are undefined.
 
 If Length > 0 and Destination is NULL, then ASSERT().
 If Length > 0 and Source is NULL, then ASSERT().
 If Source and Destination overlap, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and Length is greater than
 PcdMaximumAsciiStringLength, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and Destination contains more than
 PcdMaximumAsciiStringLength ASCII characters, not including the Null-terminator,
 then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and Source contains more than
 PcdMaximumAsciiStringLength ASCII characters, not including the Null-terminator,
 then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and concatenating Destination and
 Source results in a ASCII string with more than PcdMaximumAsciiStringLength
 ASCII characters, not including the Null-terminator, then ASSERT().
 
 @param  Destination The pointer to a Null-terminated ASCII string.
 @param  Source      The pointer to a Null-terminated ASCII string.
 @param  Length      The maximum number of ASCII characters to concatenate from
 Source.
 
 @return Destination
 
 **/
CHAR8 *
AsciiStrnCat (
              IN OUT  CHAR8                     *Destination,
              IN      CONST CHAR8               *Source,
              IN      UINTN                     Length
              );


/**
 Returns the first occurrence of a Null-terminated ASCII sub-string
 in a Null-terminated ASCII string.
 
 This function scans the contents of the ASCII string specified by String
 and returns the first occurrence of SearchString. If SearchString is not
 found in String, then NULL is returned. If the length of SearchString is zero,
 then String is returned.
 
 If String is NULL, then ASSERT().
 If SearchString is NULL, then ASSERT().
 
 If PcdMaximumAsciiStringLength is not zero, and SearchString or
 String contains more than PcdMaximumAsciiStringLength Unicode characters
 not including the Null-terminator, then ASSERT().
 
 @param  String          The pointer to a Null-terminated ASCII string.
 @param  SearchString    The pointer to a Null-terminated ASCII string to search for.
 
 @retval NULL            If the SearchString does not appear in String.
 @retval others          If there is a match return the first occurrence of SearchingString.
 If the length of SearchString is zero,return String.
 
 **/
CHAR8 *
AsciiStrStr (
             IN      CONST CHAR8               *String,
             IN      CONST CHAR8               *SearchString
             );


/**
 Convert a Null-terminated ASCII decimal string to a value of type
 UINTN.
 
 This function returns a value of type UINTN by interpreting the contents
 of the ASCII string String as a decimal number. The format of the input
 ASCII string String is:
 
 [spaces] [decimal digits].
 
 The valid decimal digit character is in the range [0-9]. The function will
 ignore the pad space, which includes spaces or tab characters, before the digits.
 The running zero in the beginning of [decimal digits] will be ignored. Then, the
 function stops at the first character that is a not a valid decimal character or
 Null-terminator, whichever on comes first.
 
 If String has only pad spaces, then 0 is returned.
 If String has no pad spaces or valid decimal digits, then 0 is returned.
 If the number represented by String overflows according to the range defined by
 UINTN, then ASSERT().
 If String is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and String contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 
 @param  String          The pointer to a Null-terminated ASCII string.
 
 @retval The value translated from String.
 
 **/
UINTN
AsciiStrDecimalToUintn (
                        IN      CONST CHAR8               *String
                        );


/**
 Convert a Null-terminated ASCII decimal string to a value of type
 UINT64.
 
 This function returns a value of type UINT64 by interpreting the contents
 of the ASCII string String as a decimal number. The format of the input
 ASCII string String is:
 
 [spaces] [decimal digits].
 
 The valid decimal digit character is in the range [0-9]. The function will
 ignore the pad space, which includes spaces or tab characters, before the digits.
 The running zero in the beginning of [decimal digits] will be ignored. Then, the
 function stops at the first character that is a not a valid decimal character or
 Null-terminator, whichever on comes first.
 
 If String has only pad spaces, then 0 is returned.
 If String has no pad spaces or valid decimal digits, then 0 is returned.
 If the number represented by String overflows according to the range defined by
 UINT64, then ASSERT().
 If String is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and String contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 
 @param  String          The pointer to a Null-terminated ASCII string.
 
 @retval Value translated from String.
 
 **/
UINT64
AsciiStrDecimalToUint64 (
                         IN      CONST CHAR8               *String
                         );


/**
 Convert a Null-terminated ASCII hexadecimal string to a value of type UINTN.
 
 This function returns a value of type UINTN by interpreting the contents of
 the ASCII string String as a hexadecimal number. The format of the input ASCII
 string String is:
 
 [spaces][zeros][x][hexadecimal digits].
 
 The valid hexadecimal digit character is in the range [0-9], [a-f] and [A-F].
 The prefix "0x" is optional. Both "x" and "X" is allowed in "0x" prefix. If "x"
 appears in the input string, it must be prefixed with at least one 0. The function
 will ignore the pad space, which includes spaces or tab characters, before [zeros],
 [x] or [hexadecimal digits]. The running zero before [x] or [hexadecimal digits]
 will be ignored. Then, the decoding starts after [x] or the first valid hexadecimal
 digit. Then, the function stops at the first character that is a not a valid
 hexadecimal character or Null-terminator, whichever on comes first.
 
 If String has only pad spaces, then 0 is returned.
 If String has no leading pad spaces, leading zeros or valid hexadecimal digits, then
 0 is returned.
 
 If the number represented by String overflows according to the range defined by UINTN,
 then ASSERT().
 If String is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero,
 and String contains more than PcdMaximumAsciiStringLength ASCII characters not including
 the Null-terminator, then ASSERT().
 
 @param  String          The pointer to a Null-terminated ASCII string.
 
 @retval Value translated from String.
 
 **/
UINTN
AsciiStrHexToUintn (
                    IN      CONST CHAR8               *String
                    );


/**
 Convert a Null-terminated ASCII hexadecimal string to a value of type UINT64.
 
 This function returns a value of type UINT64 by interpreting the contents of
 the ASCII string String as a hexadecimal number. The format of the input ASCII
 string String is:
 
 [spaces][zeros][x][hexadecimal digits].
 
 The valid hexadecimal digit character is in the range [0-9], [a-f] and [A-F].
 The prefix "0x" is optional. Both "x" and "X" is allowed in "0x" prefix. If "x"
 appears in the input string, it must be prefixed with at least one 0. The function
 will ignore the pad space, which includes spaces or tab characters, before [zeros],
 [x] or [hexadecimal digits]. The running zero before [x] or [hexadecimal digits]
 will be ignored. Then, the decoding starts after [x] or the first valid hexadecimal
 digit. Then, the function stops at the first character that is a not a valid
 hexadecimal character or Null-terminator, whichever on comes first.
 
 If String has only pad spaces, then 0 is returned.
 If String has no leading pad spaces, leading zeros or valid hexadecimal digits, then
 0 is returned.
 
 If the number represented by String overflows according to the range defined by UINT64,
 then ASSERT().
 If String is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero,
 and String contains more than PcdMaximumAsciiStringLength ASCII characters not including
 the Null-terminator, then ASSERT().
 
 @param  String          The pointer to a Null-terminated ASCII string.
 
 @retval Value translated from String.
 
 **/
UINT64
AsciiStrHexToUint64 (
                     IN      CONST CHAR8                *String
                     );


/**
 Convert one Null-terminated ASCII string to a Null-terminated
 Unicode string and returns the Unicode string.
 
 This function converts the contents of the ASCII string Source to the Unicode
 string Destination, and returns Destination.  The function terminates the
 Unicode string Destination by appending a Null-terminator character at the end.
 The caller is responsible to make sure Destination points to a buffer with size
 equal or greater than ((AsciiStrLen (Source) + 1) * sizeof (CHAR16)) in bytes.
 
 If Destination is NULL, then ASSERT().
 If Destination is not aligned on a 16-bit boundary, then ASSERT().
 If Source is NULL, then ASSERT().
 If Source and Destination overlap, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and Source contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Source contains more than
 PcdMaximumUnicodeStringLength ASCII characters not including the
 Null-terminator, then ASSERT().
 
 @param  Source        The pointer to a Null-terminated ASCII string.
 @param  Destination   The pointer to a Null-terminated Unicode string.
 
 @return Destination.
 
 **/
CHAR16 *
AsciiStrToUnicodeStr (
                      IN      CONST CHAR8               *Source,
                      OUT     CHAR16                    *Destination
                      );


/**
 Converts an 8-bit value to an 8-bit BCD value.
 
 Converts the 8-bit value specified by Value to BCD. The BCD value is
 returned.
 
 If Value >= 100, then ASSERT().
 
 @param  Value The 8-bit value to convert to BCD. Range 0..99.
 
 @return The BCD value.
 
 **/
UINT8
DecimalToBcd8 (
               IN      UINT8                     Value
               );


/**
 Converts an 8-bit BCD value to an 8-bit value.
 
 Converts the 8-bit BCD value specified by Value to an 8-bit value. The 8-bit
 value is returned.
 
 If Value >= 0xA0, then ASSERT().
 If (Value & 0x0F) >= 0x0A, then ASSERT().
 
 @param  Value The 8-bit BCD value to convert to an 8-bit value.
 
 @return The 8-bit value is returned.
 
 **/
UINT8
BcdToDecimal8 (
               IN      UINT8                     Value
               );


/**
 Copies a source buffer to a destination buffer, and returns the destination buffer.
 
 This function copies Length bytes from SourceBuffer to DestinationBuffer, and returns
 DestinationBuffer.  The implementation must be reentrant, and it must handle the case
 where SourceBuffer overlaps DestinationBuffer.
 
 If Length is greater than (MAX_ADDRESS - DestinationBuffer + 1), then ASSERT().
 If Length is greater than (MAX_ADDRESS - SourceBuffer + 1), then ASSERT().
 
 @param  DestinationBuffer   The pointer to the destination buffer of the memory copy.
 @param  SourceBuffer        The pointer to the source buffer of the memory copy.
 @param  Length              The number of bytes to copy from SourceBuffer to DestinationBuffer.
 
 @return DestinationBuffer.
 
 **/
VOID *
CopyMem (
         OUT VOID       *DestinationBuffer,
         IN CONST VOID  *SourceBuffer,
         IN UINTN       Length
         );

/**
 Fills a target buffer with a byte value, and returns the target buffer.
 
 This function fills Length bytes of Buffer with Value, and returns Buffer.
 
 If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
 
 @param  Buffer    The memory to set.
 @param  Length    The number of bytes to set.
 @param  Value     The value with which to fill Length bytes of Buffer.
 
 @return Buffer.
 
 **/
VOID *
SetMem (
        OUT VOID  *Buffer,
        IN UINTN  Length,
        IN UINT8  Value
        );

/**
 Fills a target buffer with a 16-bit value, and returns the target buffer.
 
 This function fills Length bytes of Buffer with the 16-bit value specified by
 Value, and returns Buffer. Value is repeated every 16-bits in for Length
 bytes of Buffer.
 
 If Length > 0 and Buffer is NULL, then ASSERT().
 If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
 If Buffer is not aligned on a 16-bit boundary, then ASSERT().
 If Length is not aligned on a 16-bit boundary, then ASSERT().
 
 @param  Buffer  The pointer to the target buffer to fill.
 @param  Length  The number of bytes in Buffer to fill.
 @param  Value   The value with which to fill Length bytes of Buffer.
 
 @return Buffer.
 
 **/
VOID *
SetMem16 (
          OUT VOID   *Buffer,
          IN UINTN   Length,
          IN UINT16  Value
          );

/**
 Fills a target buffer with a 32-bit value, and returns the target buffer.
 
 This function fills Length bytes of Buffer with the 32-bit value specified by
 Value, and returns Buffer. Value is repeated every 32-bits in for Length
 bytes of Buffer.
 
 If Length > 0 and Buffer is NULL, then ASSERT().
 If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
 If Buffer is not aligned on a 32-bit boundary, then ASSERT().
 If Length is not aligned on a 32-bit boundary, then ASSERT().
 
 @param  Buffer  The pointer to the target buffer to fill.
 @param  Length  The number of bytes in Buffer to fill.
 @param  Value   The value with which to fill Length bytes of Buffer.
 
 @return Buffer.
 
 **/
VOID *
SetMem32 (
          OUT VOID   *Buffer,
          IN UINTN   Length,
          IN UINT32  Value
          );

/**
 Fills a target buffer with a 64-bit value, and returns the target buffer.
 
 This function fills Length bytes of Buffer with the 64-bit value specified by
 Value, and returns Buffer. Value is repeated every 64-bits in for Length
 bytes of Buffer.
 
 If Length > 0 and Buffer is NULL, then ASSERT().
 If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
 If Buffer is not aligned on a 64-bit boundary, then ASSERT().
 If Length is not aligned on a 64-bit boundary, then ASSERT().
 
 @param  Buffer  The pointer to the target buffer to fill.
 @param  Length  The number of bytes in Buffer to fill.
 @param  Value   The value with which to fill Length bytes of Buffer.
 
 @return Buffer.
 
 **/
VOID *
SetMem64 (
          OUT VOID   *Buffer,
          IN UINTN   Length,
          IN UINT64  Value
          );

/**
 Fills a target buffer with a value that is size UINTN, and returns the target buffer.
 
 This function fills Length bytes of Buffer with the UINTN sized value specified by
 Value, and returns Buffer. Value is repeated every sizeof(UINTN) bytes for Length
 bytes of Buffer.
 
 If Length > 0 and Buffer is NULL, then ASSERT().
 If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
 If Buffer is not aligned on a UINTN boundary, then ASSERT().
 If Length is not aligned on a UINTN boundary, then ASSERT().
 
 @param  Buffer  The pointer to the target buffer to fill.
 @param  Length  The number of bytes in Buffer to fill.
 @param  Value   The value with which to fill Length bytes of Buffer.
 
 @return Buffer.
 
 **/
VOID *
SetMemN (
         OUT VOID  *Buffer,
         IN UINTN  Length,
         IN UINTN  Value
         );

/**
 Fills a target buffer with zeros, and returns the target buffer.
 
 This function fills Length bytes of Buffer with zeros, and returns Buffer.
 
 If Length > 0 and Buffer is NULL, then ASSERT().
 If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
 
 @param  Buffer      The pointer to the target buffer to fill with zeros.
 @param  Length      The number of bytes in Buffer to fill with zeros.
 
 @return Buffer.
 
 **/
VOID *
ZeroMem (
         OUT VOID  *Buffer,
         IN UINTN  Length
         );

/**
 Compares the contents of two buffers.
 
 This function compares Length bytes of SourceBuffer to Length bytes of DestinationBuffer.
 If all Length bytes of the two buffers are identical, then 0 is returned.  Otherwise, the
 value returned is the first mismatched byte in SourceBuffer subtracted from the first
 mismatched byte in DestinationBuffer.
 
 If Length > 0 and DestinationBuffer is NULL, then ASSERT().
 If Length > 0 and SourceBuffer is NULL, then ASSERT().
 If Length is greater than (MAX_ADDRESS - DestinationBuffer + 1), then ASSERT().
 If Length is greater than (MAX_ADDRESS - SourceBuffer + 1), then ASSERT().
 
 @param  DestinationBuffer The pointer to the destination buffer to compare.
 @param  SourceBuffer      The pointer to the source buffer to compare.
 @param  Length            The number of bytes to compare.
 
 @return 0                 All Length bytes of the two buffers are identical.
 @retval Non-zero          The first mismatched byte in SourceBuffer subtracted from the first
 mismatched byte in DestinationBuffer.
 
 **/
INTN
CompareMem (
            IN CONST VOID  *DestinationBuffer,
            IN CONST VOID  *SourceBuffer,
            IN UINTN       Length
            );

/**
 Scans a target buffer for an 8-bit value, and returns a pointer to the matching 8-bit value
 in the target buffer.
 
 This function searches target the buffer specified by Buffer and Length from the lowest
 address to the highest address for an 8-bit value that matches Value.  If a match is found,
 then a pointer to the matching byte in the target buffer is returned.  If no match is found,
 then NULL is returned.  If Length is 0, then NULL is returned.
 
 If Length > 0 and Buffer is NULL, then ASSERT().
 If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
 
 @param  Buffer      The pointer to the target buffer to scan.
 @param  Length      The number of bytes in Buffer to scan.
 @param  Value       The value to search for in the target buffer.
 
 @return A pointer to the matching byte in the target buffer, otherwise NULL.
 
 **/
VOID *
ScanMem8 (
          IN CONST VOID  *Buffer,
          IN UINTN       Length,
          IN UINT8       Value
          );

/**
 Scans a target buffer for a 16-bit value, and returns a pointer to the matching 16-bit value
 in the target buffer.
 
 This function searches target the buffer specified by Buffer and Length from the lowest
 address to the highest address for a 16-bit value that matches Value.  If a match is found,
 then a pointer to the matching byte in the target buffer is returned.  If no match is found,
 then NULL is returned.  If Length is 0, then NULL is returned.
 
 If Length > 0 and Buffer is NULL, then ASSERT().
 If Buffer is not aligned on a 16-bit boundary, then ASSERT().
 If Length is not aligned on a 16-bit boundary, then ASSERT().
 If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
 
 @param  Buffer      The pointer to the target buffer to scan.
 @param  Length      The number of bytes in Buffer to scan.
 @param  Value       The value to search for in the target buffer.
 
 @return A pointer to the matching byte in the target buffer, otherwise NULL.
 
 **/
VOID *
ScanMem16 (
           IN CONST VOID  *Buffer,
           IN UINTN       Length,
           IN UINT16      Value
           );

/**
 Scans a target buffer for a 32-bit value, and returns a pointer to the matching 32-bit value
 in the target buffer.
 
 This function searches target the buffer specified by Buffer and Length from the lowest
 address to the highest address for a 32-bit value that matches Value.  If a match is found,
 then a pointer to the matching byte in the target buffer is returned.  If no match is found,
 then NULL is returned.  If Length is 0, then NULL is returned.
 
 If Length > 0 and Buffer is NULL, then ASSERT().
 If Buffer is not aligned on a 32-bit boundary, then ASSERT().
 If Length is not aligned on a 32-bit boundary, then ASSERT().
 If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
 
 @param  Buffer      The pointer to the target buffer to scan.
 @param  Length      The number of bytes in Buffer to scan.
 @param  Value       The value to search for in the target buffer.
 
 @return A pointer to the matching byte in the target buffer, otherwise NULL.
 
 **/
VOID *
ScanMem32 (
           IN CONST VOID  *Buffer,
           IN UINTN       Length,
           IN UINT32      Value
           );

/**
 Scans a target buffer for a 64-bit value, and returns a pointer to the matching 64-bit value
 in the target buffer.
 
 This function searches target the buffer specified by Buffer and Length from the lowest
 address to the highest address for a 64-bit value that matches Value.  If a match is found,
 then a pointer to the matching byte in the target buffer is returned.  If no match is found,
 then NULL is returned.  If Length is 0, then NULL is returned.
 
 If Length > 0 and Buffer is NULL, then ASSERT().
 If Buffer is not aligned on a 64-bit boundary, then ASSERT().
 If Length is not aligned on a 64-bit boundary, then ASSERT().
 If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
 
 @param  Buffer      The pointer to the target buffer to scan.
 @param  Length      The number of bytes in Buffer to scan.
 @param  Value       The value to search for in the target buffer.
 
 @return A pointer to the matching byte in the target buffer, otherwise NULL.
 
 **/
VOID *
ScanMem64 (
           IN CONST VOID  *Buffer,
           IN UINTN       Length,
           IN UINT64      Value
           );

/**
 Scans a target buffer for a UINTN sized value, and returns a pointer to the matching
 UINTN sized value in the target buffer.
 
 This function searches target the buffer specified by Buffer and Length from the lowest
 address to the highest address for a UINTN sized value that matches Value.  If a match is found,
 then a pointer to the matching byte in the target buffer is returned.  If no match is found,
 then NULL is returned.  If Length is 0, then NULL is returned.
 
 If Length > 0 and Buffer is NULL, then ASSERT().
 If Buffer is not aligned on a UINTN boundary, then ASSERT().
 If Length is not aligned on a UINTN boundary, then ASSERT().
 If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
 
 @param  Buffer      The pointer to the target buffer to scan.
 @param  Length      The number of bytes in Buffer to scan.
 @param  Value       The value to search for in the target buffer.
 
 @return A pointer to the matching byte in the target buffer, otherwise NULL.
 
 **/
VOID *
ScanMemN (
          IN CONST VOID  *Buffer,
          IN UINTN       Length,
          IN UINTN       Value
          );



//
// String Services
//

/**
 Copies one Null-terminated Unicode string to another Null-terminated Unicode
 string and returns the new Unicode string.
 
 This function copies the contents of the Unicode string Source to the Unicode
 string Destination, and returns Destination. If Source and Destination
 overlap, then the results are undefined.
 
 If Destination is NULL, then ASSERT().
 If Destination is not aligned on a 16-bit boundary, then ASSERT().
 If Source is NULL, then ASSERT().
 If Source is not aligned on a 16-bit boundary, then ASSERT().
 If Source and Destination overlap, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Source contains more than
 PcdMaximumUnicodeStringLength Unicode characters not including the
 Null-terminator, then ASSERT().
 
 @param  Destination The pointer to a Null-terminated Unicode string.
 @param  Source      The pointer to a Null-terminated Unicode string.
 
 @return Destination.
 
 **/
CHAR16 *
StrCpy (
        OUT     CHAR16                    *Destination,
        IN      CONST CHAR16              *Source
        );


/**
 Copies up to a specified length from one Null-terminated Unicode string to
 another Null-terminated Unicode string and returns the new Unicode string.
 
 This function copies the contents of the Unicode string Source to the Unicode
 string Destination, and returns Destination. At most, Length Unicode
 characters are copied from Source to Destination. If Length is 0, then
 Destination is returned unmodified. If Length is greater that the number of
 Unicode characters in Source, then Destination is padded with Null Unicode
 characters. If Source and Destination overlap, then the results are
 undefined.
 
 If Length > 0 and Destination is NULL, then ASSERT().
 If Length > 0 and Destination is not aligned on a 16-bit boundary, then ASSERT().
 If Length > 0 and Source is NULL, then ASSERT().
 If Length > 0 and Source is not aligned on a 16-bit boundary, then ASSERT().
 If Source and Destination overlap, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Length is greater than
 PcdMaximumUnicodeStringLength, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Source contains more than
 PcdMaximumUnicodeStringLength Unicode characters, not including the Null-terminator,
 then ASSERT().
 
 @param  Destination The pointer to a Null-terminated Unicode string.
 @param  Source      The pointer to a Null-terminated Unicode string.
 @param  Length      The maximum number of Unicode characters to copy.
 
 @return Destination.
 
 **/
CHAR16 *
StrnCpy (
         OUT     CHAR16                    *Destination,
         IN      CONST CHAR16              *Source,
         IN      UINTN                     Length
         );


/**
 Returns the length of a Null-terminated Unicode string.
 
 This function returns the number of Unicode characters in the Null-terminated
 Unicode string specified by String.
 
 If String is NULL, then ASSERT().
 If String is not aligned on a 16-bit boundary, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and String contains more than
 PcdMaximumUnicodeStringLength Unicode characters not including the
 Null-terminator, then ASSERT().
 
 @param  String  Pointer to a Null-terminated Unicode string.
 
 @return The length of String.
 
 **/
UINTN
StrLen (
        IN      CONST CHAR16              *String
        );


/**
 Returns the size of a Null-terminated Unicode string in bytes, including the
 Null terminator.
 
 This function returns the size, in bytes, of the Null-terminated Unicode string
 specified by String.
 
 If String is NULL, then ASSERT().
 If String is not aligned on a 16-bit boundary, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and String contains more than
 PcdMaximumUnicodeStringLength Unicode characters not including the
 Null-terminator, then ASSERT().
 
 @param  String  The pointer to a Null-terminated Unicode string.
 
 @return The size of String.
 
 **/
UINTN
StrSize (
         IN      CONST CHAR16              *String
         );


/**
 Compares two Null-terminated Unicode strings, and returns the difference
 between the first mismatched Unicode characters.
 
 This function compares the Null-terminated Unicode string FirstString to the
 Null-terminated Unicode string SecondString. If FirstString is identical to
 SecondString, then 0 is returned. Otherwise, the value returned is the first
 mismatched Unicode character in SecondString subtracted from the first
 mismatched Unicode character in FirstString.
 
 If FirstString is NULL, then ASSERT().
 If FirstString is not aligned on a 16-bit boundary, then ASSERT().
 If SecondString is NULL, then ASSERT().
 If SecondString is not aligned on a 16-bit boundary, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and FirstString contains more
 than PcdMaximumUnicodeStringLength Unicode characters not including the
 Null-terminator, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and SecondString contains more
 than PcdMaximumUnicodeStringLength Unicode characters, not including the
 Null-terminator, then ASSERT().
 
 @param  FirstString   The pointer to a Null-terminated Unicode string.
 @param  SecondString  The pointer to a Null-terminated Unicode string.
 
 @retval 0      FirstString is identical to SecondString.
 @return others FirstString is not identical to SecondString.
 
 **/
INTN
StrCmp (
        IN      CONST CHAR16              *FirstString,
        IN      CONST CHAR16              *SecondString
        );


/**
 Compares up to a specified length the contents of two Null-terminated Unicode strings,
 and returns the difference between the first mismatched Unicode characters.
 
 This function compares the Null-terminated Unicode string FirstString to the
 Null-terminated Unicode string SecondString. At most, Length Unicode
 characters will be compared. If Length is 0, then 0 is returned. If
 FirstString is identical to SecondString, then 0 is returned. Otherwise, the
 value returned is the first mismatched Unicode character in SecondString
 subtracted from the first mismatched Unicode character in FirstString.
 
 If Length > 0 and FirstString is NULL, then ASSERT().
 If Length > 0 and FirstString is not aligned on a 16-bit boundary, then ASSERT().
 If Length > 0 and SecondString is NULL, then ASSERT().
 If Length > 0 and SecondString is not aligned on a 16-bit boundary, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Length is greater than
 PcdMaximumUnicodeStringLength, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and FirstString contains more than
 PcdMaximumUnicodeStringLength Unicode characters, not including the Null-terminator,
 then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and SecondString contains more than
 PcdMaximumUnicodeStringLength Unicode characters, not including the Null-terminator,
 then ASSERT().
 
 @param  FirstString   The pointer to a Null-terminated Unicode string.
 @param  SecondString  The pointer to a Null-terminated Unicode string.
 @param  Length        The maximum number of Unicode characters to compare.
 
 @retval 0      FirstString is identical to SecondString.
 @return others FirstString is not identical to SecondString.
 
 **/
INTN
StrnCmp (
         IN      CONST CHAR16              *FirstString,
         IN      CONST CHAR16              *SecondString,
         IN      UINTN                     Length
         );


/**
 Concatenates one Null-terminated Unicode string to another Null-terminated
 Unicode string, and returns the concatenated Unicode string.
 
 This function concatenates two Null-terminated Unicode strings. The contents
 of Null-terminated Unicode string Source are concatenated to the end of
 Null-terminated Unicode string Destination. The Null-terminated concatenated
 Unicode String is returned. If Source and Destination overlap, then the
 results are undefined.
 
 If Destination is NULL, then ASSERT().
 If Destination is not aligned on a 16-bit boundary, then ASSERT().
 If Source is NULL, then ASSERT().
 If Source is not aligned on a 16-bit boundary, then ASSERT().
 If Source and Destination overlap, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Destination contains more
 than PcdMaximumUnicodeStringLength Unicode characters, not including the
 Null-terminator, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Source contains more than
 PcdMaximumUnicodeStringLength Unicode characters, not including the
 Null-terminator, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and concatenating Destination
 and Source results in a Unicode string with more than
 PcdMaximumUnicodeStringLength Unicode characters, not including the
 Null-terminator, then ASSERT().
 
 @param  Destination The pointer to a Null-terminated Unicode string.
 @param  Source      The pointer to a Null-terminated Unicode string.
 
 @return Destination.
 
 **/
CHAR16 *
StrCat (
        IN OUT  CHAR16                    *Destination,
        IN      CONST CHAR16              *Source
        );


/**
 Concatenates up to a specified length one Null-terminated Unicode to the end
 of another Null-terminated Unicode string, and returns the concatenated
 Unicode string.
 
 This function concatenates two Null-terminated Unicode strings. The contents
 of Null-terminated Unicode string Source are concatenated to the end of
 Null-terminated Unicode string Destination, and Destination is returned. At
 most, Length Unicode characters are concatenated from Source to the end of
 Destination, and Destination is always Null-terminated. If Length is 0, then
 Destination is returned unmodified. If Source and Destination overlap, then
 the results are undefined.
 
 If Destination is NULL, then ASSERT().
 If Length > 0 and Destination is not aligned on a 16-bit boundary, then ASSERT().
 If Length > 0 and Source is NULL, then ASSERT().
 If Length > 0 and Source is not aligned on a 16-bit boundary, then ASSERT().
 If Source and Destination overlap, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Length is greater than
 PcdMaximumUnicodeStringLength, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Destination contains more
 than PcdMaximumUnicodeStringLength Unicode characters, not including the
 Null-terminator, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Source contains more than
 PcdMaximumUnicodeStringLength Unicode characters, not including the
 Null-terminator, then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and concatenating Destination
 and Source results in a Unicode string with more than PcdMaximumUnicodeStringLength
 Unicode characters, not including the Null-terminator, then ASSERT().
 
 @param  Destination The pointer to a Null-terminated Unicode string.
 @param  Source      The pointer to a Null-terminated Unicode string.
 @param  Length      The maximum number of Unicode characters to concatenate from
 Source.
 
 @return Destination.
 
 **/
CHAR16 *
StrnCat (
         IN OUT  CHAR16                    *Destination,
         IN      CONST CHAR16              *Source,
         IN      UINTN                     Length
         );

/**
 Returns the first occurrence of a Null-terminated Unicode sub-string
 in a Null-terminated Unicode string.
 
 This function scans the contents of the Null-terminated Unicode string
 specified by String and returns the first occurrence of SearchString.
 If SearchString is not found in String, then NULL is returned.  If
 the length of SearchString is zero, then String is returned.
 
 If String is NULL, then ASSERT().
 If String is not aligned on a 16-bit boundary, then ASSERT().
 If SearchString is NULL, then ASSERT().
 If SearchString is not aligned on a 16-bit boundary, then ASSERT().
 
 If PcdMaximumUnicodeStringLength is not zero, and SearchString
 or String contains more than PcdMaximumUnicodeStringLength Unicode
 characters, not including the Null-terminator, then ASSERT().
 
 @param  String          The pointer to a Null-terminated Unicode string.
 @param  SearchString    The pointer to a Null-terminated Unicode string to search for.
 
 @retval NULL            If the SearchString does not appear in String.
 @return others          If there is a match.
 
 **/
CHAR16 *
StrStr (
        IN      CONST CHAR16              *String,
        IN      CONST CHAR16              *SearchString
        );

/**
 Convert a Null-terminated Unicode decimal string to a value of
 type UINTN.
 
 This function returns a value of type UINTN by interpreting the contents
 of the Unicode string specified by String as a decimal number. The format
 of the input Unicode string String is:
 
 [spaces] [decimal digits].
 
 The valid decimal digit character is in the range [0-9]. The
 function will ignore the pad space, which includes spaces or
 tab characters, before [decimal digits]. The running zero in the
 beginning of [decimal digits] will be ignored. Then, the function
 stops at the first character that is a not a valid decimal character
 or a Null-terminator, whichever one comes first.
 
 If String is NULL, then ASSERT().
 If String is not aligned in a 16-bit boundary, then ASSERT().
 If String has only pad spaces, then 0 is returned.
 If String has no pad spaces or valid decimal digits,
 then 0 is returned.
 If the number represented by String overflows according
 to the range defined by UINTN, then ASSERT().
 
 If PcdMaximumUnicodeStringLength is not zero, and String contains
 more than PcdMaximumUnicodeStringLength Unicode characters not including
 the Null-terminator, then ASSERT().
 
 @param  String      The pointer to a Null-terminated Unicode string.
 
 @retval Value translated from String.
 
 **/
UINTN
StrDecimalToUintn (
                   IN      CONST CHAR16              *String
                   );

/**
 Convert a Null-terminated Unicode decimal string to a value of
 type UINT64.
 
 This function returns a value of type UINT64 by interpreting the contents
 of the Unicode string specified by String as a decimal number. The format
 of the input Unicode string String is:
 
 [spaces] [decimal digits].
 
 The valid decimal digit character is in the range [0-9]. The
 function will ignore the pad space, which includes spaces or
 tab characters, before [decimal digits]. The running zero in the
 beginning of [decimal digits] will be ignored. Then, the function
 stops at the first character that is a not a valid decimal character
 or a Null-terminator, whichever one comes first.
 
 If String is NULL, then ASSERT().
 If String is not aligned in a 16-bit boundary, then ASSERT().
 If String has only pad spaces, then 0 is returned.
 If String has no pad spaces or valid decimal digits,
 then 0 is returned.
 If the number represented by String overflows according
 to the range defined by UINT64, then ASSERT().
 
 If PcdMaximumUnicodeStringLength is not zero, and String contains
 more than PcdMaximumUnicodeStringLength Unicode characters not including
 the Null-terminator, then ASSERT().
 
 @param  String          The pointer to a Null-terminated Unicode string.
 
 @retval Value translated from String.
 
 **/
UINT64
StrDecimalToUint64 (
                    IN      CONST CHAR16              *String
                    );


/**
 Convert a Null-terminated Unicode hexadecimal string to a value of type UINTN.
 
 This function returns a value of type UINTN by interpreting the contents
 of the Unicode string specified by String as a hexadecimal number.
 The format of the input Unicode string String is:
 
 [spaces][zeros][x][hexadecimal digits].
 
 The valid hexadecimal digit character is in the range [0-9], [a-f] and [A-F].
 The prefix "0x" is optional. Both "x" and "X" is allowed in "0x" prefix.
 If "x" appears in the input string, it must be prefixed with at least one 0.
 The function will ignore the pad space, which includes spaces or tab characters,
 before [zeros], [x] or [hexadecimal digit]. The running zero before [x] or
 [hexadecimal digit] will be ignored. Then, the decoding starts after [x] or the
 first valid hexadecimal digit. Then, the function stops at the first character
 that is a not a valid hexadecimal character or NULL, whichever one comes first.
 
 If String is NULL, then ASSERT().
 If String is not aligned in a 16-bit boundary, then ASSERT().
 If String has only pad spaces, then zero is returned.
 If String has no leading pad spaces, leading zeros or valid hexadecimal digits,
 then zero is returned.
 If the number represented by String overflows according to the range defined by
 UINTN, then ASSERT().
 
 If PcdMaximumUnicodeStringLength is not zero, and String contains more than
 PcdMaximumUnicodeStringLength Unicode characters not including the Null-terminator,
 then ASSERT().
 
 @param  String          The pointer to a Null-terminated Unicode string.
 
 @retval Value translated from String.
 
 **/
UINTN
StrHexToUintn (
               IN      CONST CHAR16              *String
               );


/**
 Convert a Null-terminated Unicode hexadecimal string to a value of type UINT64.
 
 This function returns a value of type UINT64 by interpreting the contents
 of the Unicode string specified by String as a hexadecimal number.
 The format of the input Unicode string String is
 
 [spaces][zeros][x][hexadecimal digits].
 
 The valid hexadecimal digit character is in the range [0-9], [a-f] and [A-F].
 The prefix "0x" is optional. Both "x" and "X" is allowed in "0x" prefix.
 If "x" appears in the input string, it must be prefixed with at least one 0.
 The function will ignore the pad space, which includes spaces or tab characters,
 before [zeros], [x] or [hexadecimal digit]. The running zero before [x] or
 [hexadecimal digit] will be ignored. Then, the decoding starts after [x] or the
 first valid hexadecimal digit. Then, the function stops at the first character that is
 a not a valid hexadecimal character or NULL, whichever one comes first.
 
 If String is NULL, then ASSERT().
 If String is not aligned in a 16-bit boundary, then ASSERT().
 If String has only pad spaces, then zero is returned.
 If String has no leading pad spaces, leading zeros or valid hexadecimal digits,
 then zero is returned.
 If the number represented by String overflows according to the range defined by
 UINT64, then ASSERT().
 
 If PcdMaximumUnicodeStringLength is not zero, and String contains more than
 PcdMaximumUnicodeStringLength Unicode characters not including the Null-terminator,
 then ASSERT().
 
 @param  String          The pointer to a Null-terminated Unicode string.
 
 @retval Value translated from String.
 
 **/
UINT64
StrHexToUint64 (
                IN      CONST CHAR16             *String
                );

/**
 Convert a Null-terminated Unicode string to a Null-terminated
 ASCII string and returns the ASCII string.
 
 This function converts the content of the Unicode string Source
 to the ASCII string Destination by copying the lower 8 bits of
 each Unicode character. It returns Destination.
 
 The caller is responsible to make sure Destination points to a buffer with size
 equal or greater than ((StrLen (Source) + 1) * sizeof (CHAR8)) in bytes.
 
 If any Unicode characters in Source contain non-zero value in
 the upper 8 bits, then ASSERT().
 
 If Destination is NULL, then ASSERT().
 If Source is NULL, then ASSERT().
 If Source is not aligned on a 16-bit boundary, then ASSERT().
 If Source and Destination overlap, then ASSERT().
 
 If PcdMaximumUnicodeStringLength is not zero, and Source contains
 more than PcdMaximumUnicodeStringLength Unicode characters not including
 the Null-terminator, then ASSERT().
 
 If PcdMaximumAsciiStringLength is not zero, and Source contains more
 than PcdMaximumAsciiStringLength Unicode characters not including the
 Null-terminator, then ASSERT().
 
 @param  Source        The pointer to a Null-terminated Unicode string.
 @param  Destination   The pointer to a Null-terminated ASCII string.
 
 @return Destination.
 
 **/
CHAR8 *
UnicodeStrToAsciiStr (
                      IN      CONST CHAR16              *Source,
                      OUT     CHAR8                     *Destination
                      );


/**
 Copies one Null-terminated ASCII string to another Null-terminated ASCII
 string and returns the new ASCII string.
 
 This function copies the contents of the ASCII string Source to the ASCII
 string Destination, and returns Destination. If Source and Destination
 overlap, then the results are undefined.
 
 If Destination is NULL, then ASSERT().
 If Source is NULL, then ASSERT().
 If Source and Destination overlap, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and Source contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 
 @param  Destination The pointer to a Null-terminated ASCII string.
 @param  Source      The pointer to a Null-terminated ASCII string.
 
 @return Destination
 
 **/
CHAR8 *
AsciiStrCpy (
             OUT     CHAR8                     *Destination,
             IN      CONST CHAR8               *Source
             );


/**
 Copies up to a specified length one Null-terminated ASCII string to another
 Null-terminated ASCII string and returns the new ASCII string.
 
 This function copies the contents of the ASCII string Source to the ASCII
 string Destination, and returns Destination. At most, Length ASCII characters
 are copied from Source to Destination. If Length is 0, then Destination is
 returned unmodified. If Length is greater that the number of ASCII characters
 in Source, then Destination is padded with Null ASCII characters. If Source
 and Destination overlap, then the results are undefined.
 
 If Destination is NULL, then ASSERT().
 If Source is NULL, then ASSERT().
 If Source and Destination overlap, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and Length is greater than
 PcdMaximumAsciiStringLength, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and Source contains more than
 PcdMaximumAsciiStringLength ASCII characters, not including the Null-terminator,
 then ASSERT().
 
 @param  Destination The pointer to a Null-terminated ASCII string.
 @param  Source      The pointer to a Null-terminated ASCII string.
 @param  Length      The maximum number of ASCII characters to copy.
 
 @return Destination
 
 **/
CHAR8 *
AsciiStrnCpy (
              OUT     CHAR8                     *Destination,
              IN      CONST CHAR8               *Source,
              IN      UINTN                     Length
              );


/**
 Returns the length of a Null-terminated ASCII string.
 
 This function returns the number of ASCII characters in the Null-terminated
 ASCII string specified by String.
 
 If Length > 0 and Destination is NULL, then ASSERT().
 If Length > 0 and Source is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and String contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 
 @param  String  The pointer to a Null-terminated ASCII string.
 
 @return The length of String.
 
 **/
UINTN
AsciiStrLen (
             IN      CONST CHAR8               *String
             );


/**
 Returns the size of a Null-terminated ASCII string in bytes, including the
 Null terminator.
 
 This function returns the size, in bytes, of the Null-terminated ASCII string
 specified by String.
 
 If String is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and String contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 
 @param  String  The pointer to a Null-terminated ASCII string.
 
 @return The size of String.
 
 **/
UINTN
AsciiStrSize (
              IN      CONST CHAR8               *String
              );


/**
 Compares two Null-terminated ASCII strings, and returns the difference
 between the first mismatched ASCII characters.
 
 This function compares the Null-terminated ASCII string FirstString to the
 Null-terminated ASCII string SecondString. If FirstString is identical to
 SecondString, then 0 is returned. Otherwise, the value returned is the first
 mismatched ASCII character in SecondString subtracted from the first
 mismatched ASCII character in FirstString.
 
 If FirstString is NULL, then ASSERT().
 If SecondString is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and FirstString contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and SecondString contains more
 than PcdMaximumAsciiStringLength ASCII characters not including the
 Null-terminator, then ASSERT().
 
 @param  FirstString   The pointer to a Null-terminated ASCII string.
 @param  SecondString  The pointer to a Null-terminated ASCII string.
 
 @retval ==0      FirstString is identical to SecondString.
 @retval !=0      FirstString is not identical to SecondString.
 
 **/
INTN
AsciiStrCmp (
             IN      CONST CHAR8               *FirstString,
             IN      CONST CHAR8               *SecondString
             );


/**
 Performs a case insensitive comparison of two Null-terminated ASCII strings,
 and returns the difference between the first mismatched ASCII characters.
 
 This function performs a case insensitive comparison of the Null-terminated
 ASCII string FirstString to the Null-terminated ASCII string SecondString. If
 FirstString is identical to SecondString, then 0 is returned. Otherwise, the
 value returned is the first mismatched lower case ASCII character in
 SecondString subtracted from the first mismatched lower case ASCII character
 in FirstString.
 
 If FirstString is NULL, then ASSERT().
 If SecondString is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and FirstString contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and SecondString contains more
 than PcdMaximumAsciiStringLength ASCII characters not including the
 Null-terminator, then ASSERT().
 
 @param  FirstString   The pointer to a Null-terminated ASCII string.
 @param  SecondString  The pointer to a Null-terminated ASCII string.
 
 @retval ==0    FirstString is identical to SecondString using case insensitive
 comparisons.
 @retval !=0    FirstString is not identical to SecondString using case
 insensitive comparisons.
 
 **/
INTN
AsciiStriCmp (
              IN      CONST CHAR8               *FirstString,
              IN      CONST CHAR8               *SecondString
              );


/**
 Compares two Null-terminated ASCII strings with maximum lengths, and returns
 the difference between the first mismatched ASCII characters.
 
 This function compares the Null-terminated ASCII string FirstString to the
 Null-terminated ASCII  string SecondString. At most, Length ASCII characters
 will be compared. If Length is 0, then 0 is returned. If FirstString is
 identical to SecondString, then 0 is returned. Otherwise, the value returned
 is the first mismatched ASCII character in SecondString subtracted from the
 first mismatched ASCII character in FirstString.
 
 If Length > 0 and FirstString is NULL, then ASSERT().
 If Length > 0 and SecondString is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and Length is greater than
 PcdMaximumAsciiStringLength, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and FirstString contains more than
 PcdMaximumAsciiStringLength ASCII characters, not including the Null-terminator,
 then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and SecondString contains more than
 PcdMaximumAsciiStringLength ASCII characters, not including the Null-terminator,
 then ASSERT().
 
 @param  FirstString   The pointer to a Null-terminated ASCII string.
 @param  SecondString  The pointer to a Null-terminated ASCII string.
 @param  Length        The maximum number of ASCII characters for compare.
 
 @retval ==0       FirstString is identical to SecondString.
 @retval !=0       FirstString is not identical to SecondString.
 
 **/
INTN
AsciiStrnCmp (
              IN      CONST CHAR8               *FirstString,
              IN      CONST CHAR8               *SecondString,
              IN      UINTN                     Length
              );


/**
 Concatenates one Null-terminated ASCII string to another Null-terminated
 ASCII string, and returns the concatenated ASCII string.
 
 This function concatenates two Null-terminated ASCII strings. The contents of
 Null-terminated ASCII string Source are concatenated to the end of Null-
 terminated ASCII string Destination. The Null-terminated concatenated ASCII
 String is returned.
 
 If Destination is NULL, then ASSERT().
 If Source is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and Destination contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and Source contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 If PcdMaximumAsciiStringLength is not zero and concatenating Destination and
 Source results in a ASCII string with more than PcdMaximumAsciiStringLength
 ASCII characters, then ASSERT().
 
 @param  Destination The pointer to a Null-terminated ASCII string.
 @param  Source      The pointer to a Null-terminated ASCII string.
 
 @return Destination
 
 **/
CHAR8 *
AsciiStrCat (
             IN OUT CHAR8    *Destination,
             IN CONST CHAR8  *Source
             );


/**
 Concatenates up to a specified length one Null-terminated ASCII string to
 the end of another Null-terminated ASCII string, and returns the
 concatenated ASCII string.
 
 This function concatenates two Null-terminated ASCII strings. The contents
 of Null-terminated ASCII string Source are concatenated to the end of Null-
 terminated ASCII string Destination, and Destination is returned. At most,
 Length ASCII characters are concatenated from Source to the end of
 Destination, and Destination is always Null-terminated. If Length is 0, then
 Destination is returned unmodified. If Source and Destination overlap, then
 the results are undefined.
 
 If Length > 0 and Destination is NULL, then ASSERT().
 If Length > 0 and Source is NULL, then ASSERT().
 If Source and Destination overlap, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and Length is greater than
 PcdMaximumAsciiStringLength, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and Destination contains more than
 PcdMaximumAsciiStringLength ASCII characters, not including the Null-terminator,
 then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and Source contains more than
 PcdMaximumAsciiStringLength ASCII characters, not including the Null-terminator,
 then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and concatenating Destination and
 Source results in a ASCII string with more than PcdMaximumAsciiStringLength
 ASCII characters, not including the Null-terminator, then ASSERT().
 
 @param  Destination The pointer to a Null-terminated ASCII string.
 @param  Source      The pointer to a Null-terminated ASCII string.
 @param  Length      The maximum number of ASCII characters to concatenate from
 Source.
 
 @return Destination
 
 **/
CHAR8 *
AsciiStrnCat (
              IN OUT  CHAR8                     *Destination,
              IN      CONST CHAR8               *Source,
              IN      UINTN                     Length
              );


/**
 Returns the first occurrence of a Null-terminated ASCII sub-string
 in a Null-terminated ASCII string.
 
 This function scans the contents of the ASCII string specified by String
 and returns the first occurrence of SearchString. If SearchString is not
 found in String, then NULL is returned. If the length of SearchString is zero,
 then String is returned.
 
 If String is NULL, then ASSERT().
 If SearchString is NULL, then ASSERT().
 
 If PcdMaximumAsciiStringLength is not zero, and SearchString or
 String contains more than PcdMaximumAsciiStringLength Unicode characters
 not including the Null-terminator, then ASSERT().
 
 @param  String          The pointer to a Null-terminated ASCII string.
 @param  SearchString    The pointer to a Null-terminated ASCII string to search for.
 
 @retval NULL            If the SearchString does not appear in String.
 @retval others          If there is a match return the first occurrence of SearchingString.
 If the length of SearchString is zero,return String.
 
 **/
CHAR8 *
AsciiStrStr (
             IN      CONST CHAR8               *String,
             IN      CONST CHAR8               *SearchString
             );


/**
 Convert a Null-terminated ASCII decimal string to a value of type
 UINTN.
 
 This function returns a value of type UINTN by interpreting the contents
 of the ASCII string String as a decimal number. The format of the input
 ASCII string String is:
 
 [spaces] [decimal digits].
 
 The valid decimal digit character is in the range [0-9]. The function will
 ignore the pad space, which includes spaces or tab characters, before the digits.
 The running zero in the beginning of [decimal digits] will be ignored. Then, the
 function stops at the first character that is a not a valid decimal character or
 Null-terminator, whichever on comes first.
 
 If String has only pad spaces, then 0 is returned.
 If String has no pad spaces or valid decimal digits, then 0 is returned.
 If the number represented by String overflows according to the range defined by
 UINTN, then ASSERT().
 If String is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and String contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 
 @param  String          The pointer to a Null-terminated ASCII string.
 
 @retval The value translated from String.
 
 **/
UINTN
AsciiStrDecimalToUintn (
                        IN      CONST CHAR8               *String
                        );


/**
 Convert a Null-terminated ASCII decimal string to a value of type
 UINT64.
 
 This function returns a value of type UINT64 by interpreting the contents
 of the ASCII string String as a decimal number. The format of the input
 ASCII string String is:
 
 [spaces] [decimal digits].
 
 The valid decimal digit character is in the range [0-9]. The function will
 ignore the pad space, which includes spaces or tab characters, before the digits.
 The running zero in the beginning of [decimal digits] will be ignored. Then, the
 function stops at the first character that is a not a valid decimal character or
 Null-terminator, whichever on comes first.
 
 If String has only pad spaces, then 0 is returned.
 If String has no pad spaces or valid decimal digits, then 0 is returned.
 If the number represented by String overflows according to the range defined by
 UINT64, then ASSERT().
 If String is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and String contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 
 @param  String          The pointer to a Null-terminated ASCII string.
 
 @retval Value translated from String.
 
 **/
UINT64
AsciiStrDecimalToUint64 (
                         IN      CONST CHAR8               *String
                         );


/**
 Convert a Null-terminated ASCII hexadecimal string to a value of type UINTN.
 
 This function returns a value of type UINTN by interpreting the contents of
 the ASCII string String as a hexadecimal number. The format of the input ASCII
 string String is:
 
 [spaces][zeros][x][hexadecimal digits].
 
 The valid hexadecimal digit character is in the range [0-9], [a-f] and [A-F].
 The prefix "0x" is optional. Both "x" and "X" is allowed in "0x" prefix. If "x"
 appears in the input string, it must be prefixed with at least one 0. The function
 will ignore the pad space, which includes spaces or tab characters, before [zeros],
 [x] or [hexadecimal digits]. The running zero before [x] or [hexadecimal digits]
 will be ignored. Then, the decoding starts after [x] or the first valid hexadecimal
 digit. Then, the function stops at the first character that is a not a valid
 hexadecimal character or Null-terminator, whichever on comes first.
 
 If String has only pad spaces, then 0 is returned.
 If String has no leading pad spaces, leading zeros or valid hexadecimal digits, then
 0 is returned.
 
 If the number represented by String overflows according to the range defined by UINTN,
 then ASSERT().
 If String is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero,
 and String contains more than PcdMaximumAsciiStringLength ASCII characters not including
 the Null-terminator, then ASSERT().
 
 @param  String          The pointer to a Null-terminated ASCII string.
 
 @retval Value translated from String.
 
 **/
UINTN
AsciiStrHexToUintn (
                    IN      CONST CHAR8               *String
                    );


/**
 Convert a Null-terminated ASCII hexadecimal string to a value of type UINT64.
 
 This function returns a value of type UINT64 by interpreting the contents of
 the ASCII string String as a hexadecimal number. The format of the input ASCII
 string String is:
 
 [spaces][zeros][x][hexadecimal digits].
 
 The valid hexadecimal digit character is in the range [0-9], [a-f] and [A-F].
 The prefix "0x" is optional. Both "x" and "X" is allowed in "0x" prefix. If "x"
 appears in the input string, it must be prefixed with at least one 0. The function
 will ignore the pad space, which includes spaces or tab characters, before [zeros],
 [x] or [hexadecimal digits]. The running zero before [x] or [hexadecimal digits]
 will be ignored. Then, the decoding starts after [x] or the first valid hexadecimal
 digit. Then, the function stops at the first character that is a not a valid
 hexadecimal character or Null-terminator, whichever on comes first.
 
 If String has only pad spaces, then 0 is returned.
 If String has no leading pad spaces, leading zeros or valid hexadecimal digits, then
 0 is returned.
 
 If the number represented by String overflows according to the range defined by UINT64,
 then ASSERT().
 If String is NULL, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero,
 and String contains more than PcdMaximumAsciiStringLength ASCII characters not including
 the Null-terminator, then ASSERT().
 
 @param  String          The pointer to a Null-terminated ASCII string.
 
 @retval Value translated from String.
 
 **/
UINT64
AsciiStrHexToUint64 (
                     IN      CONST CHAR8                *String
                     );


/**
 Convert one Null-terminated ASCII string to a Null-terminated
 Unicode string and returns the Unicode string.
 
 This function converts the contents of the ASCII string Source to the Unicode
 string Destination, and returns Destination.  The function terminates the
 Unicode string Destination by appending a Null-terminator character at the end.
 The caller is responsible to make sure Destination points to a buffer with size
 equal or greater than ((AsciiStrLen (Source) + 1) * sizeof (CHAR16)) in bytes.
 
 If Destination is NULL, then ASSERT().
 If Destination is not aligned on a 16-bit boundary, then ASSERT().
 If Source is NULL, then ASSERT().
 If Source and Destination overlap, then ASSERT().
 If PcdMaximumAsciiStringLength is not zero, and Source contains more than
 PcdMaximumAsciiStringLength ASCII characters not including the Null-terminator,
 then ASSERT().
 If PcdMaximumUnicodeStringLength is not zero, and Source contains more than
 PcdMaximumUnicodeStringLength ASCII characters not including the
 Null-terminator, then ASSERT().
 
 @param  Source        The pointer to a Null-terminated ASCII string.
 @param  Destination   The pointer to a Null-terminated Unicode string.
 
 @return Destination.
 
 **/
CHAR16 *
AsciiStrToUnicodeStr (
                      IN      CONST CHAR8               *Source,
                      OUT     CHAR16                    *Destination
                      );


/**
 Converts an 8-bit value to an 8-bit BCD value.
 
 Converts the 8-bit value specified by Value to BCD. The BCD value is
 returned.
 
 If Value >= 100, then ASSERT().
 
 @param  Value The 8-bit value to convert to BCD. Range 0..99.
 
 @return The BCD value.
 
 **/
UINT8
DecimalToBcd8 (
               IN      UINT8                     Value
               );


/**
 Converts an 8-bit BCD value to an 8-bit value.
 
 Converts the 8-bit BCD value specified by Value to an 8-bit value. The 8-bit
 value is returned.
 
 If Value >= 0xA0, then ASSERT().
 If (Value & 0x0F) >= 0x0A, then ASSERT().
 
 @param  Value The 8-bit BCD value to convert to an 8-bit value.
 
 @return The 8-bit value is returned.
 
 **/
UINT8
BcdToDecimal8 (
               IN      UINT8                     Value
               );


/**
Allocates one or more 4KB pages of type EfiBootServicesData.

Allocates the number of 4KB pages of type EfiBootServicesData and returns a pointer to the
allocated buffer.  The buffer returned is aligned on a 4KB boundary.  If Pages is 0, then NULL
is returned.  If there is not enough memory remaining to satisfy the request, then NULL is
returned.

@param  Pages                 The number of 4 KB pages to allocate.

@return A pointer to the allocated buffer or NULL if allocation fails.

**/
VOID *
AllocatePages (
               IN UINTN  Pages
               );

/**
 Allocates one or more 4KB pages of type EfiRuntimeServicesData.
 
 Allocates the number of 4KB pages of type EfiRuntimeServicesData and returns a pointer to the
 allocated buffer.  The buffer returned is aligned on a 4KB boundary.  If Pages is 0, then NULL
 is returned.  If there is not enough memory remaining to satisfy the request, then NULL is
 returned.
 
 @param  Pages                 The number of 4 KB pages to allocate.
 
 @return A pointer to the allocated buffer or NULL if allocation fails.
 
 **/
VOID *
AllocateRuntimePages (
                      IN UINTN  Pages
                      );

/**
 Allocates one or more 4KB pages of type EfiReservedMemoryType.
 
 Allocates the number of 4KB pages of type EfiReservedMemoryType and returns a pointer to the
 allocated buffer.  The buffer returned is aligned on a 4KB boundary.  If Pages is 0, then NULL
 is returned.  If there is not enough memory remaining to satisfy the request, then NULL is
 returned.
 
 @param  Pages                 The number of 4 KB pages to allocate.
 
 @return A pointer to the allocated buffer or NULL if allocation fails.
 
 **/
VOID *
AllocateReservedPages (
                       IN UINTN  Pages
                       );

/**
 Frees one or more 4KB pages that were previously allocated with one of the page allocation
 functions in the Memory Allocation Library.
 
 Frees the number of 4KB pages specified by Pages from the buffer specified by Buffer.  Buffer
 must have been allocated on a previous call to the page allocation services of the Memory
 Allocation Library.  If it is not possible to free allocated pages, then this function will
 perform no actions.
 
 If Buffer was not allocated with a page allocation function in the Memory Allocation Library,
 then ASSERT().
 If Pages is zero, then ASSERT().
 
 @param  Buffer                Pointer to the buffer of pages to free.
 @param  Pages                 The number of 4 KB pages to free.
 
 **/
VOID
FreePages (
           IN VOID   *Buffer,
           IN UINTN  Pages
           );

/**
 Allocates one or more 4KB pages of type EfiBootServicesData at a specified alignment.
 
 Allocates the number of 4KB pages specified by Pages of type EfiBootServicesData with an
 alignment specified by Alignment.  The allocated buffer is returned.  If Pages is 0, then NULL is
 returned.  If there is not enough memory at the specified alignment remaining to satisfy the
 request, then NULL is returned.
 
 If Alignment is not a power of two and Alignment is not zero, then ASSERT().
 
 @param  Pages                 The number of 4 KB pages to allocate.
 @param  Alignment             The requested alignment of the allocation.  Must be a power of two.
 If Alignment is zero, then byte alignment is used.
 
 @return A pointer to the allocated buffer or NULL if allocation fails.
 
 **/
VOID *
AllocateAlignedPages (
                      IN UINTN  Pages,
                      IN UINTN  Alignment
                      );

/**
 Allocates one or more 4KB pages of type EfiRuntimeServicesData at a specified alignment.
 
 Allocates the number of 4KB pages specified by Pages of type EfiRuntimeServicesData with an
 alignment specified by Alignment.  The allocated buffer is returned.  If Pages is 0, then NULL is
 returned.  If there is not enough memory at the specified alignment remaining to satisfy the
 request, then NULL is returned.
 
 If Alignment is not a power of two and Alignment is not zero, then ASSERT().
 
 @param  Pages                 The number of 4 KB pages to allocate.
 @param  Alignment             The requested alignment of the allocation.  Must be a power of two.
 If Alignment is zero, then byte alignment is used.
 
 @return A pointer to the allocated buffer or NULL if allocation fails.
 
 **/
VOID *
AllocateAlignedRuntimePages (
                             IN UINTN  Pages,
                             IN UINTN  Alignment
                             );

/**
 Allocates one or more 4KB pages of type EfiReservedMemoryType at a specified alignment.
 
 Allocates the number of 4KB pages specified by Pages of type EfiReservedMemoryType with an
 alignment specified by Alignment.  The allocated buffer is returned.  If Pages is 0, then NULL is
 returned.  If there is not enough memory at the specified alignment remaining to satisfy the
 request, then NULL is returned.
 
 If Alignment is not a power of two and Alignment is not zero, then ASSERT().
 
 @param  Pages                 The number of 4 KB pages to allocate.
 @param  Alignment             The requested alignment of the allocation.  Must be a power of two.
 If Alignment is zero, then byte alignment is used.
 
 @return A pointer to the allocated buffer or NULL if allocation fails.
 
 **/
VOID *
AllocateAlignedReservedPages (
                              IN UINTN  Pages,
                              IN UINTN  Alignment
                              );

/**
 Frees one or more 4KB pages that were previously allocated with one of the aligned page
 allocation functions in the Memory Allocation Library.
 
 Frees the number of 4KB pages specified by Pages from the buffer specified by Buffer.  Buffer
 must have been allocated on a previous call to the aligned page allocation services of the Memory
 Allocation Library.  If it is not possible to free allocated pages, then this function will
 perform no actions.
 
 If Buffer was not allocated with an aligned page allocation function in the Memory Allocation
 Library, then ASSERT().
 If Pages is zero, then ASSERT().
 
 @param  Buffer                Pointer to the buffer of pages to free.
 @param  Pages                 The number of 4 KB pages to free.
 
 **/
VOID
FreeAlignedPages (
                  IN VOID   *Buffer,
                  IN UINTN  Pages
                  );

/**
 Allocates a buffer of type EfiBootServicesData.
 
 Allocates the number bytes specified by AllocationSize of type EfiBootServicesData and returns a
 pointer to the allocated buffer.  If AllocationSize is 0, then a valid buffer of 0 size is
 returned.  If there is not enough memory remaining to satisfy the request, then NULL is returned.
 
 @param  AllocationSize        The number of bytes to allocate.
 
 @return A pointer to the allocated buffer or NULL if allocation fails.
 
 **/
VOID *
AllocatePool (
              IN UINTN  AllocationSize
              );

/**
 Allocates a buffer of type EfiRuntimeServicesData.
 
 Allocates the number bytes specified by AllocationSize of type EfiRuntimeServicesData and returns
 a pointer to the allocated buffer.  If AllocationSize is 0, then a valid buffer of 0 size is
 returned.  If there is not enough memory remaining to satisfy the request, then NULL is returned.
 
 @param  AllocationSize        The number of bytes to allocate.
 
 @return A pointer to the allocated buffer or NULL if allocation fails.
 
 **/
VOID *
AllocateRuntimePool (
                     IN UINTN  AllocationSize
                     );

/**
 Allocates a buffer of type EfiReservedMemoryType.
 
 Allocates the number bytes specified by AllocationSize of type EfiReservedMemoryType and returns
 a pointer to the allocated buffer.  If AllocationSize is 0, then a valid buffer of 0 size is
 returned.  If there is not enough memory remaining to satisfy the request, then NULL is returned.
 
 @param  AllocationSize        The number of bytes to allocate.
 
 @return A pointer to the allocated buffer or NULL if allocation fails.
 
 **/
VOID *
AllocateReservedPool (
                      IN UINTN  AllocationSize
                      );

/**
 Allocates and zeros a buffer of type EfiBootServicesData.
 
 Allocates the number bytes specified by AllocationSize of type EfiBootServicesData, clears the
 buffer with zeros, and returns a pointer to the allocated buffer.  If AllocationSize is 0, then a
 valid buffer of 0 size is returned.  If there is not enough memory remaining to satisfy the
 request, then NULL is returned.
 
 @param  AllocationSize        The number of bytes to allocate and zero.
 
 @return A pointer to the allocated buffer or NULL if allocation fails.
 
 **/
VOID *
AllocateZeroPool (
                  IN UINTN  AllocationSize
                  );

/**
 Allocates and zeros a buffer of type EfiRuntimeServicesData.
 
 Allocates the number bytes specified by AllocationSize of type EfiRuntimeServicesData, clears the
 buffer with zeros, and returns a pointer to the allocated buffer.  If AllocationSize is 0, then a
 valid buffer of 0 size is returned.  If there is not enough memory remaining to satisfy the
 request, then NULL is returned.
 
 @param  AllocationSize        The number of bytes to allocate and zero.
 
 @return A pointer to the allocated buffer or NULL if allocation fails.
 
 **/
VOID *
AllocateRuntimeZeroPool (
                         IN UINTN  AllocationSize
                         );

/**
 Allocates and zeros a buffer of type EfiReservedMemoryType.
 
 Allocates the number bytes specified by AllocationSize of type EfiReservedMemoryType, clears the
 buffer with zeros, and returns a pointer to the allocated buffer.  If AllocationSize is 0, then a
 valid buffer of 0 size is returned.  If there is not enough memory remaining to satisfy the
 request, then NULL is returned.
 
 @param  AllocationSize        The number of bytes to allocate and zero.
 
 @return A pointer to the allocated buffer or NULL if allocation fails.
 
 **/
VOID *
AllocateReservedZeroPool (
                          IN UINTN  AllocationSize
                          );

/**
 Copies a buffer to an allocated buffer of type EfiBootServicesData.
 
 Allocates the number bytes specified by AllocationSize of type EfiBootServicesData, copies
 AllocationSize bytes from Buffer to the newly allocated buffer, and returns a pointer to the
 allocated buffer.  If AllocationSize is 0, then a valid buffer of 0 size is returned.  If there
 is not enough memory remaining to satisfy the request, then NULL is returned.
 
 If Buffer is NULL, then ASSERT().
 If AllocationSize is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
 
 @param  AllocationSize        The number of bytes to allocate and zero.
 @param  Buffer                The buffer to copy to the allocated buffer.
 
 @return A pointer to the allocated buffer or NULL if allocation fails.
 
 **/
VOID *
AllocateCopyPool (
                  IN UINTN       AllocationSize,
                  IN CONST VOID  *Buffer
                  );

/**
 Copies a buffer to an allocated buffer of type EfiRuntimeServicesData.
 
 Allocates the number bytes specified by AllocationSize of type EfiRuntimeServicesData, copies
 AllocationSize bytes from Buffer to the newly allocated buffer, and returns a pointer to the
 allocated buffer.  If AllocationSize is 0, then a valid buffer of 0 size is returned.  If there
 is not enough memory remaining to satisfy the request, then NULL is returned.
 
 If Buffer is NULL, then ASSERT().
 If AllocationSize is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
 
 @param  AllocationSize        The number of bytes to allocate and zero.
 @param  Buffer                The buffer to copy to the allocated buffer.
 
 @return A pointer to the allocated buffer or NULL if allocation fails.
 
 **/
VOID *
AllocateRuntimeCopyPool (
                         IN UINTN       AllocationSize,
                         IN CONST VOID  *Buffer
                         );

/**
 Copies a buffer to an allocated buffer of type EfiReservedMemoryType.
 
 Allocates the number bytes specified by AllocationSize of type EfiReservedMemoryType, copies
 AllocationSize bytes from Buffer to the newly allocated buffer, and returns a pointer to the
 allocated buffer.  If AllocationSize is 0, then a valid buffer of 0 size is returned.  If there
 is not enough memory remaining to satisfy the request, then NULL is returned.
 
 If Buffer is NULL, then ASSERT().
 If AllocationSize is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
 
 @param  AllocationSize        The number of bytes to allocate and zero.
 @param  Buffer                The buffer to copy to the allocated buffer.
 
 @return A pointer to the allocated buffer or NULL if allocation fails.
 
 **/
VOID *
AllocateReservedCopyPool (
                          IN UINTN       AllocationSize,
                          IN CONST VOID  *Buffer
                          );

/**
 Reallocates a buffer of type EfiBootServicesData.
 
 Allocates and zeros the number bytes specified by NewSize from memory of type
 EfiBootServicesData.  If OldBuffer is not NULL, then the smaller of OldSize and
 NewSize bytes are copied from OldBuffer to the newly allocated buffer, and
 OldBuffer is freed.  A pointer to the newly allocated buffer is returned.
 If NewSize is 0, then a valid buffer of 0 size is  returned.  If there is not
 enough memory remaining to satisfy the request, then NULL is returned.
 
 If the allocation of the new buffer is successful and the smaller of NewSize and OldSize
 is greater than (MAX_ADDRESS - OldBuffer + 1), then ASSERT().
 
 @param  OldSize        The size, in bytes, of OldBuffer.
 @param  NewSize        The size, in bytes, of the buffer to reallocate.
 @param  OldBuffer      The buffer to copy to the allocated buffer.  This is an optional
 parameter that may be NULL.
 
 @return A pointer to the allocated buffer or NULL if allocation fails.
 
 **/
VOID *
ReallocatePool (
                IN UINTN  OldSize,
                IN UINTN  NewSize,
                IN VOID   *OldBuffer  OPTIONAL
                );

/**
 Reallocates a buffer of type EfiRuntimeServicesData.
 
 Allocates and zeros the number bytes specified by NewSize from memory of type
 EfiRuntimeServicesData.  If OldBuffer is not NULL, then the smaller of OldSize and
 NewSize bytes are copied from OldBuffer to the newly allocated buffer, and
 OldBuffer is freed.  A pointer to the newly allocated buffer is returned.
 If NewSize is 0, then a valid buffer of 0 size is  returned.  If there is not
 enough memory remaining to satisfy the request, then NULL is returned.
 
 If the allocation of the new buffer is successful and the smaller of NewSize and OldSize
 is greater than (MAX_ADDRESS - OldBuffer + 1), then ASSERT().
 
 @param  OldSize        The size, in bytes, of OldBuffer.
 @param  NewSize        The size, in bytes, of the buffer to reallocate.
 @param  OldBuffer      The buffer to copy to the allocated buffer.  This is an optional
 parameter that may be NULL.
 
 @return A pointer to the allocated buffer or NULL if allocation fails.
 
 **/
VOID *
ReallocateRuntimePool (
                       IN UINTN  OldSize,
                       IN UINTN  NewSize,
                       IN VOID   *OldBuffer  OPTIONAL
                       );

/**
 Reallocates a buffer of type EfiReservedMemoryType.
 
 Allocates and zeros the number bytes specified by NewSize from memory of type
 EfiReservedMemoryType.  If OldBuffer is not NULL, then the smaller of OldSize and
 NewSize bytes are copied from OldBuffer to the newly allocated buffer, and
 OldBuffer is freed.  A pointer to the newly allocated buffer is returned.
 If NewSize is 0, then a valid buffer of 0 size is  returned.  If there is not
 enough memory remaining to satisfy the request, then NULL is returned.
 
 If the allocation of the new buffer is successful and the smaller of NewSize and OldSize
 is greater than (MAX_ADDRESS - OldBuffer + 1), then ASSERT().
 
 @param  OldSize        The size, in bytes, of OldBuffer.
 @param  NewSize        The size, in bytes, of the buffer to reallocate.
 @param  OldBuffer      The buffer to copy to the allocated buffer.  This is an optional
 parameter that may be NULL.
 
 @return A pointer to the allocated buffer or NULL if allocation fails.
 
 **/
VOID *
ReallocateReservedPool (
                        IN UINTN  OldSize,
                        IN UINTN  NewSize,
                        IN VOID   *OldBuffer  OPTIONAL
                        );

/**
 Frees a buffer that was previously allocated with one of the pool allocation functions in the
 Memory Allocation Library.
 
 Frees the buffer specified by Buffer.  Buffer must have been allocated on a previous call to the
 pool allocation services of the Memory Allocation Library.  If it is not possible to free pool
 resources, then this function will perform no actions.
 
 If Buffer was not allocated with a pool allocation function in the Memory Allocation Library,
 then ASSERT().
 
 @param  Buffer                Pointer to the buffer to free.
 
 **/
VOID
FreePool (
          IN VOID   *Buffer
          );


UINTN
SPrint (
        OUT CHAR16  *Str,
        IN UINTN    StrSize,
        IN CHAR16   *fmt,
        ...
        );

CHAR16 *
PoolPrint (
           IN CHAR16           *fmt,
           ...
           );

typedef struct {
    CHAR16      *str;
    UINTN       len;
    UINTN       maxlen;
} POOL_PRINT;

CHAR16 *
CatPrint (
          IN OUT POOL_PRINT   *Str,
          IN CHAR16           *fmt,
          ...
          );

VOID
ValueToHex (
            IN CHAR16   *Buffer,
            IN UINT64   v
            );

VOID
ValueToString (
               IN CHAR16   *Buffer,
               IN BOOLEAN  Comma,
               IN INT64    v
               );

VOID
TimeToString (
              OUT CHAR16      *Buffer,
              IN EFI_TIME     *Time
              );

VOID
GuidToString (
              OUT CHAR16      *Buffer,
              IN EFI_GUID     *Guid
              );

VOID
StatusToString (
                OUT CHAR16      *Buffer,
                EFI_STATUS      Status
                );

UINT64
GetPowerOfTwo (
               IN UINT64 Operand
               );

UINT8
Log2 (
      IN UINT64   Operand
      );

UINT64
DivU64x32 (
           IN UINT64   Dividend,
           IN UINTN    Divisor,
           OUT UINTN   *Remainder OPTIONAL
           );

UINT64
RShiftU64 (
           IN UINT64   Operand,
           IN UINTN    Count
           );

UINT64
MultU64x32 (
            IN UINT64   Multiplicand,
            IN UINTN    Multiplier
            );

UINT64
LShiftU64 (
           IN UINT64   Operand,
           IN UINTN    Count
           );

UINT64
Power10U64 (
            IN UINT64   Operand,
            IN UINTN    Power
            );

/**
 Retrieves a 32-bit PCD token value based on a token name.
 
 Returns the 32-bit value for the token specified by TokenName.
 If TokenName is not a valid token in the token space, then the module will not build.
 
 @param   TokenName  The name of the PCD token to retrieve a current value for.
 
 @return  32-bit value for the token specified by TokenName.
 
 **/
#define _PCD_TOKEN_PcdMaximumAsciiStringLength  6U
#define _PCD_VALUE_PcdMaximumAsciiStringLength  1000000U
#define _PCD_GET_MODE_32_PcdMaximumAsciiStringLength  _PCD_VALUE_PcdMaximumAsciiStringLength

#define _PCD_TOKEN_PcdMaximumUnicodeStringLength  7U
#define _PCD_VALUE_PcdMaximumUnicodeStringLength  1000000U
#define _PCD_GET_MODE_32_PcdMaximumUnicodeStringLength  _PCD_VALUE_PcdMaximumUnicodeStringLength

#define PcdGet32(TokenName)                 _PCD_GET_MODE_32_##TokenName

#define  BIT0     0x00000001
#define  BIT1     0x00000002
#define  BIT2     0x00000004
#define  BIT3     0x00000008
#define  BIT4     0x00000010
#define  BIT5     0x00000020
#define  BIT6     0x00000040
#define  BIT7     0x00000080
#define  BIT8     0x00000100
#define  BIT9     0x00000200
#define  BIT10    0x00000400
#define  BIT11    0x00000800
#define  BIT12    0x00001000
#define  BIT13    0x00002000
#define  BIT14    0x00004000
#define  BIT15    0x00008000
#define  BIT16    0x00010000
#define  BIT17    0x00020000
#define  BIT18    0x00040000
#define  BIT19    0x00080000
#define  BIT20    0x00100000
#define  BIT21    0x00200000
#define  BIT22    0x00400000
#define  BIT23    0x00800000
#define  BIT24    0x01000000
#define  BIT25    0x02000000
#define  BIT26    0x04000000
#define  BIT27    0x08000000
#define  BIT28    0x10000000
#define  BIT29    0x20000000
#define  BIT30    0x40000000
#define  BIT31    0x80000000
#define  BIT32    0x0000000100000000ULL
#define  BIT33    0x0000000200000000ULL
#define  BIT34    0x0000000400000000ULL
#define  BIT35    0x0000000800000000ULL
#define  BIT36    0x0000001000000000ULL
#define  BIT37    0x0000002000000000ULL
#define  BIT38    0x0000004000000000ULL
#define  BIT39    0x0000008000000000ULL
#define  BIT40    0x0000010000000000ULL
#define  BIT41    0x0000020000000000ULL
#define  BIT42    0x0000040000000000ULL
#define  BIT43    0x0000080000000000ULL
#define  BIT44    0x0000100000000000ULL
#define  BIT45    0x0000200000000000ULL
#define  BIT46    0x0000400000000000ULL
#define  BIT47    0x0000800000000000ULL
#define  BIT48    0x0001000000000000ULL
#define  BIT49    0x0002000000000000ULL
#define  BIT50    0x0004000000000000ULL
#define  BIT51    0x0008000000000000ULL
#define  BIT52    0x0010000000000000ULL
#define  BIT53    0x0020000000000000ULL
#define  BIT54    0x0040000000000000ULL
#define  BIT55    0x0080000000000000ULL
#define  BIT56    0x0100000000000000ULL
#define  BIT57    0x0200000000000000ULL
#define  BIT58    0x0400000000000000ULL
#define  BIT59    0x0800000000000000ULL
#define  BIT60    0x1000000000000000ULL
#define  BIT61    0x2000000000000000ULL
#define  BIT62    0x4000000000000000ULL
#define  BIT63    0x8000000000000000ULL


///
/// Maximum legal IA-32 address.
///
#define MAX_ADDRESS   0xFFFFFFFF

typedef CHAR16  CHAR_W;
#define STRING_W(_s)                                  L##_s

#endif /* __LIBEFI_H__ */

/* EOF */
