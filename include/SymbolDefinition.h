#pragma once

#define CONST   const
#define STATIC  static
#define VOID    void
#define UINTN   UINT64
#define INTN    INT64
#define IN
#define OUT
#define OPTIONAL
#define EFIAPI
#define SCRATCH_BUFFER_REQUEST_SIZE  SIZE_64KB

using UINT8  = unsigned char;
using BOOLEAN = UINT8;
using CHAR8  = char;
using UINT16 = unsigned short;
using CHAR16 = UINT16;
using UINT32 = unsigned int;
using UINT64 = unsigned long long;
using INT16  = short;
using INT32  = int;
using INT64  = long long;
using EFI_HANDLE = VOID*;
