#include "BaseMemoryLib.h"

/**
  Copy Length bytes from Source to Destination.

  @param  DestinationBuffer The target of the copy request.
  @param  SourceBuffer      The place to copy from.
  @param  Length            The number of bytes to copy.

  @return Destination

**/
VOID *
EFIAPI
InternalMemCopyMem (
  OUT     VOID        *DestinationBuffer,
  IN      CONST VOID  *SourceBuffer,
  IN      UINTN       Length
  )
{
  //
  // Declare the local variables that actually move the data elements as
  // volatile to prevent the optimizer from replacing this function with
  // the intrinsic memcpy()
  //
  volatile UINT8   *Destination8;
  CONST UINT8      *Source8;
  volatile UINT32  *Destination32;
  CONST UINT32     *Source32;
  volatile UINT64  *Destination64;
  CONST UINT64     *Source64;
  UINTN            Alignment;

  if ((((UINTN)DestinationBuffer & 0x7) == 0) && (((UINTN)SourceBuffer & 0x7) == 0) && (Length >= 8)) {
    if (SourceBuffer > DestinationBuffer) {
      Destination64 = (UINT64 *)DestinationBuffer;
      Source64      = (CONST UINT64 *)SourceBuffer;
      while (Length >= 8) {
        *(Destination64++) = *(Source64++);
        Length            -= 8;
      }

      // Finish if there are still some bytes to copy
      Destination8 = (UINT8 *)Destination64;
      Source8      = (CONST UINT8 *)Source64;
      while (Length-- != 0) {
        *(Destination8++) = *(Source8++);
      }
    } else if (SourceBuffer < DestinationBuffer) {
      Destination64 = (UINT64 *)((UINTN)DestinationBuffer + Length);
      Source64      = (CONST UINT64 *)((UINTN)SourceBuffer + Length);

      // Destination64 and Source64 were aligned on a 64-bit boundary
      // but if length is not a multiple of 8 bytes then they won't be
      // anymore.

      Alignment = Length & 0x7;
      if (Alignment != 0) {
        Destination8 = (UINT8 *)Destination64;
        Source8      = (CONST UINT8 *)Source64;

        while (Alignment-- != 0) {
          *(--Destination8) = *(--Source8);
          --Length;
        }

        Destination64 = (UINT64 *)Destination8;
        Source64      = (CONST UINT64 *)Source8;
      }

      while (Length > 0) {
        *(--Destination64) = *(--Source64);
        Length            -= 8;
      }
    }
  } else if ((((UINTN)DestinationBuffer & 0x3) == 0) && (((UINTN)SourceBuffer & 0x3) == 0) && (Length >= 4)) {
    if (SourceBuffer > DestinationBuffer) {
      Destination32 = (UINT32 *)DestinationBuffer;
      Source32      = (CONST UINT32 *)SourceBuffer;
      while (Length >= 4) {
        *(Destination32++) = *(Source32++);
        Length            -= 4;
      }

      // Finish if there are still some bytes to copy
      Destination8 = (UINT8 *)Destination32;
      Source8      = (CONST UINT8 *)Source32;
      while (Length-- != 0) {
        *(Destination8++) = *(Source8++);
      }
    } else if (SourceBuffer < DestinationBuffer) {
      Destination32 = (UINT32 *)((UINTN)DestinationBuffer + Length);
      Source32      = (CONST UINT32 *)((UINTN)SourceBuffer + Length);

      // Destination32 and Source32 were aligned on a 32-bit boundary
      // but if length is not a multiple of 4 bytes then they won't be
      // anymore.

      Alignment = Length & 0x3;
      if (Alignment != 0) {
        Destination8 = (UINT8 *)Destination32;
        Source8      = (CONST UINT8 *)Source32;

        while (Alignment-- != 0) {
          *(--Destination8) = *(--Source8);
          --Length;
        }

        Destination32 = (UINT32 *)Destination8;
        Source32      = (CONST UINT32 *)Source8;
      }

      while (Length > 0) {
        *(--Destination32) = *(--Source32);
        Length            -= 4;
      }
    }
  } else {
    if (SourceBuffer > DestinationBuffer) {
      Destination8 = (UINT8 *)DestinationBuffer;
      Source8      = (CONST UINT8 *)SourceBuffer;
      while (Length-- != 0) {
        *(Destination8++) = *(Source8++);
      }
    } else if (SourceBuffer < DestinationBuffer) {
      Destination8 = (UINT8 *)DestinationBuffer + (Length - 1);
      Source8      = (CONST UINT8 *)SourceBuffer + (Length - 1);
      while (Length-- != 0) {
        *(Destination8--) = *(Source8--);
      }
    }
  }

  return DestinationBuffer;
}

/**
  Set Buffer to Value for Size bytes.

  @param  Buffer   The memory to set.
  @param  Length   The number of bytes to set.
  @param  Value    The value of the set operation.

  @return Buffer

**/
VOID *
EFIAPI
InternalMemSetMem (
  OUT     VOID   *Buffer,
  IN      UINTN  Length,
  IN      UINT8  Value
  )
{
  //
  // Declare the local variables that actually move the data elements as
  // volatile to prevent the optimizer from replacing this function with
  // the intrinsic memset()
  //
  volatile UINT8   *Pointer8;
  volatile UINT32  *Pointer32;
  volatile UINT64  *Pointer64;
  UINT32           Value32;
  UINT64           Value64;

  if ((((UINTN)Buffer & 0x7) == 0) && (Length >= 8)) {
    // Generate the 64bit value
    Value32 = (Value << 24) | (Value << 16) | (Value << 8) | Value;
    Value64 = LShiftU64 (Value32, 32) | Value32;

    Pointer64 = (UINT64 *)Buffer;
    while (Length >= 8) {
      *(Pointer64++) = Value64;
      Length        -= 8;
    }

    // Finish with bytes if needed
    Pointer8 = (UINT8 *)Pointer64;
  } else if ((((UINTN)Buffer & 0x3) == 0) && (Length >= 4)) {
    // Generate the 32bit value
    Value32 = (Value << 24) | (Value << 16) | (Value << 8) | Value;

    Pointer32 = (UINT32 *)Buffer;
    while (Length >= 4) {
      *(Pointer32++) = Value32;
      Length        -= 4;
    }

    // Finish with bytes if needed
    Pointer8 = (UINT8 *)Pointer32;
  } else {
    Pointer8 = (UINT8 *)Buffer;
  }

  while (Length-- > 0) {
    *(Pointer8++) = Value;
  }

  return Buffer;
}

/**
  Fills a target buffer with a 16-bit value, and returns the target buffer.

  @param  Buffer  The pointer to the target buffer to fill.
  @param  Length  The count of 16-bit value to fill.
  @param  Value   The value with which to fill Length bytes of Buffer.

  @return Buffer

**/
VOID *
EFIAPI
InternalMemSetMem16 (
  OUT     VOID    *Buffer,
  IN      UINTN   Length,
  IN      UINT16  Value
  )
{
  for ( ; Length != 0; Length--) {
    ((UINT16 *)Buffer)[Length - 1] = Value;
  }

  return Buffer;
}

/**
  Shifts a 64-bit integer left between 0 and 63 bits. The low bits
  are filled with zeros. The shifted value is returned.

  This function shifts the 64-bit value Operand to the left by Count bits. The
  low Count bits are set to zero. The shifted value is returned.

  @param  Operand The 64-bit operand to shift left.
  @param  Count   The number of bits to shift left.

  @return Operand << Count.

**/
UINT64
EFIAPI
InternalMathLShiftU64 (
  IN      UINT64  Operand,
  IN      UINTN   Count
  )
{
  return Operand << Count;
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
EFIAPI
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
  Fills a target buffer with a byte value, and returns the target buffer.

  This function fills Length bytes of Buffer with Value, and returns Buffer.

  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().

  @param  Buffer    The memory to set.
  @param  Length    The number of bytes to set.
  @param  Value     The value with which to fill Length bytes of Buffer.

  @return Buffer.

**/
VOID *
EFIAPI
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
EFIAPI
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
  Shifts a 64-bit integer left between 0 and 63 bits. The low bits are filled
  with zeros. The shifted value is returned.

  This function shifts the 64-bit value Operand to the left by Count bits. The
  low Count bits are set to zero. The shifted value is returned.

  If Count is greater than 63, then ASSERT().

  @param  Operand The 64-bit operand to shift left.
  @param  Count   The number of bits to shift left.

  @return Operand << Count.

**/
UINT64
EFIAPI
LShiftU64 (
  IN      UINT64  Operand,
  IN      UINTN   Count
  )
{
  ASSERT (Count < 64);
  return InternalMathLShiftU64 (Operand, Count);
}

/**
  Reads a 32-bit value from memory that may be unaligned.

  This function returns the 32-bit value pointed to by Buffer. The function
  guarantees that the read operation does not produce an alignment fault.

  If the Buffer is NULL, then ASSERT().

  @param  Buffer  A pointer to a 32-bit value that may be unaligned.

  @return The 32-bit value read from Buffer.

**/
UINT32
EFIAPI
ReadUnaligned32 (
  IN CONST UINT32  *Buffer
  )
{
  ASSERT (Buffer != NULL);

  return *Buffer;
}
