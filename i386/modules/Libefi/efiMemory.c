
#include "MemLibInternals.h"

/** @file
 Implementation of the InternalMemCopyMem routine. This function is broken
 out into its own source file so that it can be excluded from a build for a
 particular platform easily if an optimized version is desired.
 
 Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
 This program and the accompanying materials
 are licensed and made available under the terms and conditions of the BSD License
 which accompanies this distribution.  The full text of the license may be found at
 http://opensource.org/licenses/bsd-license.php.
 
 THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
 
 **/

/**
 Copy Length bytes from Source to Destination.
 
 @param  DestinationBuffer The target of the copy request.
 @param  SourceBuffer      The place to copy from.
 @param  Length            The number of bytes to copy.
 
 @return Destination
 
 **/
VOID *
InternalMemCopyMem (
                    OUT     VOID                      *DestinationBuffer,
                    IN      CONST VOID                *SourceBuffer,
                    IN      UINTN                     Length
                    )
{
    //
    // Declare the local variables that actually move the data elements as
    // volatile to prevent the optimizer from replacing this function with
    // the intrinsic memcpy()
    //
    volatile UINT8                    *Destination8;
    CONST UINT8                       *Source8;
    
    if (SourceBuffer > DestinationBuffer) {
        Destination8 = (UINT8*)DestinationBuffer;
        Source8 = (CONST UINT8*)SourceBuffer;
        while (Length-- != 0) {
            *(Destination8++) = *(Source8++);
        }
    } else if (SourceBuffer < DestinationBuffer) {
        Destination8 = (UINT8*)DestinationBuffer + Length;
        Source8 = (CONST UINT8*)SourceBuffer + Length;
        while (Length-- != 0) {
            *(--Destination8) = *(--Source8);
        }
    }
    return DestinationBuffer;
}

/**
 Copies a source buffer to a destination buffer, and returns the destination buffer.
 
 This function copies Length bytes from SourceBuffer to DestinationBuffer, and returns
 DestinationBuffer.  The implementation must be reentrant, and it must handle the case
 where SourceBuffer overlaps DestinationBuffer.
 
 If Length is greater than (MAX_ADDRESS - DestinationBuffer + 1), then ASSERT().
 If Length is greater than (MAX_ADDRESS - SourceBuffer + 1), then ASSERT().
 
 @param  DestinationBuffer   A pointer to the destination buffer of the memory copy.
 @param  SourceBuffer        A pointer to the source buffer of the memory copy.
 @param  Length              The number of bytes to copy from SourceBuffer to DestinationBuffer.
 
 @return DestinationBuffer.
 
 **/
VOID *
CopyMem (
         OUT VOID       *DestinationBuffer,
         IN CONST VOID  *SourceBuffer,
         IN UINTN       Length
         )
{
    if (Length == 0) {
        return DestinationBuffer;
    }
    ASSERT ((Length - 1) <= (MAX_ADDRESS - (UINTN)DestinationBuffer));
    ASSERT ((Length - 1) <= (MAX_ADDRESS - (UINTN)SourceBuffer));
    
    if (DestinationBuffer == SourceBuffer) {
        return DestinationBuffer;
    }
    return InternalMemCopyMem (DestinationBuffer, SourceBuffer, Length);
}


/**
 Fills a target buffer with a 16-bit value, and returns the target buffer.
 
 @param  Buffer  The pointer to the target buffer to fill.
 @param  Length  The count of 16-bit value to fill.
 @param  Value   The value with which to fill Length bytes of Buffer.
 
 @return Buffer
 
 **/
VOID *
InternalMemSetMem16 (
                     OUT     VOID                      *Buffer,
                     IN      UINTN                     Length,
                     IN      UINT16                    Value
                     )
{
	ASSERT(Buffer != NULL);
    do {
        ((UINT16*)Buffer)[--Length] = Value;
    } while (Length != 0);
    return Buffer;
}

/**
 Fills a target buffer with a 32-bit value, and returns the target buffer.
 
 @param  Buffer  The pointer to the target buffer to fill.
 @param  Length  The count of 32-bit value to fill.
 @param  Value   The value with which to fill Length bytes of Buffer.
 
 @return Buffer
 
 **/
VOID *
InternalMemSetMem32 (
                     OUT     VOID                      *Buffer,
                     IN      UINTN                     Length,
                     IN      UINT32                    Value
                     )
{
	ASSERT(Buffer != NULL);
    do {
        ((UINT32*)Buffer)[--Length] = Value;
    } while (Length != 0);
    return Buffer;
}

/**
 Fills a target buffer with a 64-bit value, and returns the target buffer.
 
 @param  Buffer  The pointer to the target buffer to fill.
 @param  Length  The count of 64-bit value to fill.
 @param  Value   The value with which to fill Length bytes of Buffer.
 
 @return Buffer
 
 **/
VOID *
InternalMemSetMem64 (
                     OUT     VOID                      *Buffer,
                     IN      UINTN                     Length,
                     IN      UINT64                    Value
                     )
{
	ASSERT(Buffer != NULL);
    do {
        ((UINT64*)Buffer)[--Length] = Value;
    } while (Length != 0);
    return Buffer;
}

/**
 Set Buffer to 0 for Size bytes.
 
 @param  Buffer Memory to set.
 @param  Length The number of bytes to set.
 
 @return Buffer
 
 **/
VOID *
InternalMemZeroMem (
                    OUT     VOID                      *Buffer,
                    IN      UINTN                     Length
                    )
{
    return InternalMemSetMem (Buffer, Length, 0);
}

/**
 Compares two memory buffers of a given length.
 
 @param  DestinationBuffer The first memory buffer.
 @param  SourceBuffer      The second memory buffer.
 @param  Length            Length of DestinationBuffer and SourceBuffer memory
 regions to compare. Must be non-zero.
 
 @return 0                 All Length bytes of the two buffers are identical.
 @retval Non-zero          The first mismatched byte in SourceBuffer subtracted from the first
 mismatched byte in DestinationBuffer.
 
 **/
INTN
InternalMemCompareMem (
                       IN      CONST VOID                *DestinationBuffer,
                       IN      CONST VOID                *SourceBuffer,
                       IN      UINTN                     Length
                       )
{
    while ((--Length != 0) &&
           (*(INT8*)DestinationBuffer == *(INT8*)SourceBuffer)) {
        DestinationBuffer = (INT8*)DestinationBuffer + 1;
        SourceBuffer = (INT8*)SourceBuffer + 1;
    }
    return (INTN)*(UINT8*)DestinationBuffer - (INTN)*(UINT8*)SourceBuffer;
}

/**
 Scans a target buffer for an 8-bit value, and returns a pointer to the
 matching 8-bit value in the target buffer.
 
 @param  Buffer  The pointer to the target buffer to scan.
 @param  Length  The count of 8-bit value to scan. Must be non-zero.
 @param  Value   The value to search for in the target buffer.
 
 @return The pointer to the first occurrence, or NULL if not found.
 
 **/
CONST VOID *
InternalMemScanMem8 (
                     IN      CONST VOID                *Buffer,
                     IN      UINTN                     Length,
                     IN      UINT8                     Value
                     )
{
    CONST UINT8                       *Pointer;
    
    Pointer = (CONST UINT8*)Buffer;
	ASSERT(Pointer != NULL);
    do {
        if (*Pointer == Value) {
            return Pointer;
        }
        ++Pointer;
    } while (--Length != 0);
    return NULL;
}

/**
 Scans a target buffer for a 16-bit value, and returns a pointer to the
 matching 16-bit value in the target buffer.
 
 @param  Buffer  The pointer to the target buffer to scan.
 @param  Length  The count of 16-bit value to scan. Must be non-zero.
 @param  Value   The value to search for in the target buffer.
 
 @return The pointer to the first occurrence, or NULL if not found.
 
 **/
CONST VOID *
InternalMemScanMem16 (
                      IN      CONST VOID                *Buffer,
                      IN      UINTN                     Length,
                      IN      UINT16                    Value
                      )
{
    CONST UINT16                      *Pointer;
    
    Pointer = (CONST UINT16*)Buffer;
	ASSERT(Pointer != NULL);
    do {
        if (*Pointer == Value) {
            return Pointer;
        }
        ++Pointer;
    } while (--Length != 0);
    return NULL;
}

/**
 Scans a target buffer for a 32-bit value, and returns a pointer to the
 matching 32-bit value in the target buffer.
 
 @param  Buffer  The pointer to the target buffer to scan.
 @param  Length  The count of 32-bit value to scan. Must be non-zero.
 @param  Value   The value to search for in the target buffer.
 
 @return The pointer to the first occurrence, or NULL if not found.
 
 **/
CONST VOID *
InternalMemScanMem32 (
                      IN      CONST VOID                *Buffer,
                      IN      UINTN                     Length,
                      IN      UINT32                    Value
                      )
{
    CONST UINT32                      *Pointer;
    
    Pointer = (CONST UINT32*)Buffer;
	ASSERT(Pointer != NULL);
    do {
        if (*Pointer == Value) {
            return Pointer;
        }
        ++Pointer;
    } while (--Length != 0);
    return NULL;
}

/**
 Scans a target buffer for a 64-bit value, and returns a pointer to the
 matching 64-bit value in the target buffer.
 
 @param  Buffer  The pointer to the target buffer to scan.
 @param  Length  The count of 64-bit value to scan. Must be non-zero.
 @param  Value   The value to search for in the target buffer.
 
 @return The pointer to the first occurrence, or NULL if not found.
 
 **/
CONST VOID *
InternalMemScanMem64 (
                      IN      CONST VOID                *Buffer,
                      IN      UINTN                     Length,
                      IN      UINT64                    Value
                      )
{
    CONST UINT64                      *Pointer;
    
    Pointer = (CONST UINT64*)Buffer;
	ASSERT(Pointer != NULL);	
    do {
        if (*Pointer == Value) {
            return Pointer;
        }
        ++Pointer;
    } while (--Length != 0);
    return NULL;
}

/**
 Scans a target buffer for an 8-bit value, and returns a pointer to the matching 8-bit value
 in the target buffer.
 
 This function searches the target buffer specified by Buffer and Length from the lowest
 address to the highest address for an 8-bit value that matches Value.  If a match is found,
 then a pointer to the matching byte in the target buffer is returned.  If no match is found,
 then NULL is returned.  If Length is 0, then NULL is returned.
 
 If Length > 0 and Buffer is NULL, then ASSERT().
 If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
 
 @param  Buffer      The pointer to the target buffer to scan.
 @param  Length      The number of bytes in Buffer to scan.
 @param  Value       The value to search for in the target buffer.
 
 @return A pointer to the matching byte in the target buffer, or NULL otherwise.
 
 **/
VOID *
ScanMem8 (
          IN CONST VOID  *Buffer,
          IN UINTN       Length,
          IN UINT8       Value
          )
{
    if (Length == 0) {
        return NULL;
    }
    ASSERT (Buffer != NULL);
    ASSERT ((Length - 1) <= (MAX_ADDRESS - (UINTN)Buffer));
    
    return (VOID*)InternalMemScanMem8 (Buffer, Length, Value);
}

/**
 Scans a target buffer for a UINTN sized value, and returns a pointer to the matching
 UINTN sized value in the target buffer.
 
 This function searches the target buffer specified by Buffer and Length from the lowest
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
 
 @return A pointer to the matching byte in the target buffer, or NULL otherwise.
 
 **/
VOID *
ScanMemN (
          IN CONST VOID  *Buffer,
          IN UINTN       Length,
          IN UINTN       Value
          )
{
    if (sizeof (UINTN) == sizeof (UINT64)) {
        return ScanMem64 (Buffer, Length, (UINT64)Value);
    } else {
        return ScanMem32 (Buffer, Length, (UINT32)Value);
    }
}

/**
 Scans a target buffer for a 16-bit value, and returns a pointer to the matching 16-bit value
 in the target buffer.
 
 This function searches the target buffer specified by Buffer and Length from the lowest
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
 
 @return A pointer to the matching byte in the target buffer or NULL otherwise.
 
 **/
VOID *
ScanMem16 (
           IN CONST VOID  *Buffer,
           IN UINTN       Length,
           IN UINT16      Value
           )
{
    if (Length == 0) {
        return NULL;
    }
    
    ASSERT (Buffer != NULL);
    ASSERT (((UINTN)Buffer & (sizeof (Value) - 1)) == 0);
    ASSERT ((Length - 1) <= (MAX_ADDRESS - (UINTN)Buffer));
    ASSERT ((Length & (sizeof (Value) - 1)) == 0);
    
    return (VOID*)InternalMemScanMem16 (Buffer, Length / sizeof (Value), Value);
}

/**
 Scans a target buffer for a 32-bit value, and returns a pointer to the matching 32-bit value
 in the target buffer.
 
 This function searches the target buffer specified by Buffer and Length from the lowest
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
 
 @return A pointer to the matching byte in the target buffer or NULL otherwise.
 
 **/
VOID *
ScanMem32 (
           IN CONST VOID  *Buffer,
           IN UINTN       Length,
           IN UINT32      Value
           )
{
    if (Length == 0) {
        return NULL;
    }
    
    ASSERT (Buffer != NULL);
    ASSERT (((UINTN)Buffer & (sizeof (Value) - 1)) == 0);
    ASSERT ((Length - 1) <= (MAX_ADDRESS - (UINTN)Buffer));
    ASSERT ((Length & (sizeof (Value) - 1)) == 0);
    
    return (VOID*)InternalMemScanMem32 (Buffer, Length / sizeof (Value), Value);
}

/**
 Scans a target buffer for a 64-bit value, and returns a pointer to the matching 64-bit value
 in the target buffer.
 
 This function searches the target buffer specified by Buffer and Length from the lowest
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
 
 @return A pointer to the matching byte in the target buffer or NULL otherwise.
 
 **/
VOID *
ScanMem64 (
           IN CONST VOID  *Buffer,
           IN UINTN       Length,
           IN UINT64      Value
           )
{
    if (Length == 0) {
        return NULL;
    }
    
    ASSERT (Buffer != NULL);
    ASSERT (((UINTN)Buffer & (sizeof (Value) - 1)) == 0);
    ASSERT ((Length - 1) <= (MAX_ADDRESS - (UINTN)Buffer));
    ASSERT ((Length & (sizeof (Value) - 1)) == 0);
    
    return (VOID*)InternalMemScanMem64 (Buffer, Length / sizeof (Value), Value);
}

/**
 Set Buffer to Value for Size bytes.
 
 @param  Buffer   The memory to set.
 @param  Length   The number of bytes to set.
 @param  Value    The value of the set operation.
 
 @return Buffer
 
 **/
VOID *
InternalMemSetMem (
                   OUT     VOID                      *Buffer,
                   IN      UINTN                     Length,
                   IN      UINT8                     Value
                   )
{
    //
    // Declare the local variables that actually move the data elements as
    // volatile to prevent the optimizer from replacing this function with
    // the intrinsic memset()
    //
	ASSERT(Buffer != NULL);
    volatile UINT8                    *Pointer;    
    Pointer = (UINT8*)Buffer;
    while (Length-- > 0) {
        *(Pointer++) = Value;
    }
    return Buffer;
}

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
          )
{
    if (Length == 0) {
        return Buffer;
    }
    
    ASSERT (Buffer != NULL);
    ASSERT ((Length - 1) <= (MAX_ADDRESS - (UINTN)Buffer));
    ASSERT ((((UINTN)Buffer) & (sizeof (Value) - 1)) == 0);
    ASSERT ((Length & (sizeof (Value) - 1)) == 0);
    
    return InternalMemSetMem16 (Buffer, Length / sizeof (Value), Value);
}

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
          )
{
    if (Length == 0) {
        return Buffer;
    }
    
    ASSERT (Buffer != NULL);
    ASSERT ((Length - 1) <= (MAX_ADDRESS - (UINTN)Buffer));
    ASSERT ((((UINTN)Buffer) & (sizeof (Value) - 1)) == 0);
    ASSERT ((Length & (sizeof (Value) - 1)) == 0);
    
    return InternalMemSetMem32 (Buffer, Length / sizeof (Value), Value);
}

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
          )
{
    if (Length == 0) {
        return Buffer;
    }
    
    ASSERT (Buffer != NULL);
    ASSERT ((Length - 1) <= (MAX_ADDRESS - (UINTN)Buffer));
    ASSERT ((((UINTN)Buffer) & (sizeof (Value) - 1)) == 0);
    ASSERT ((Length & (sizeof (Value) - 1)) == 0);
    
    return InternalMemSetMem64 (Buffer, Length / sizeof (Value), Value);
}

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
        )
{
    if (Length == 0) {
        return Buffer;
    }
    
    ASSERT ((Length - 1) <= (MAX_ADDRESS - (UINTN)Buffer));
    
    return InternalMemSetMem (Buffer, Length, Value);
}

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
         )
{
    if (sizeof (UINTN) == sizeof (UINT64)) {
        return SetMem64 (Buffer, Length, (UINT64)Value);
    } else {
        return SetMem32 (Buffer, Length, (UINT32)Value);
    }
}

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
         )
{
    ASSERT (!(Buffer == NULL && Length > 0));
    ASSERT (Length <= (MAX_ADDRESS - (UINTN)Buffer + 1));
    return InternalMemZeroMem (Buffer, Length);
}


VOID *
AllocatePool (
              IN UINTN  AllocationSize
              )
{
	ASSERT(AllocationSize != 0);
    return malloc (AllocationSize);
}


VOID *
AllocateZeroPool (
                  IN UINTN  AllocationSize
                  )
{
    VOID  *Memory;
    
    Memory = AllocatePool (AllocationSize);
    if (Memory != NULL) {
        Memory = ZeroMem (Memory, AllocationSize);
    }
    return Memory;
}

VOID *
AllocateCopyPool (
                  IN UINTN       AllocationSize,
                  IN CONST VOID  *Buffer
                  )
{
    VOID  *Memory;
    
    ASSERT (Buffer != NULL);
    ASSERT (AllocationSize <= (MAX_ADDRESS - (UINTN) Buffer + 1));
    
    Memory = AllocatePool ( AllocationSize);
    if (Memory != NULL) {
        Memory = CopyMem (Memory, Buffer, AllocationSize);
    }
    return Memory;
}

VOID *
ReallocatePool (
                IN UINTN  OldSize,
                IN UINTN  NewSize,
                IN VOID   *OldBuffer  OPTIONAL
                )
{
    VOID  *NewBuffer;
    
    NewBuffer = AllocateZeroPool (NewSize);
    if (NewBuffer != NULL && OldBuffer != NULL) {
        CopyMem (NewBuffer, OldBuffer, MIN (OldSize, NewSize));
        FreePool (OldBuffer);
    }
    return NewBuffer;
}

VOID
FreePool (
          IN VOID   *Buffer
          )
{
    ASSERT (Buffer != NULL);
    
    free (Buffer);
    
}
