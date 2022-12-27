#pragma once

using UINT8  = unsigned char;
using UINT16 = unsigned short;
using CHAR16 = UINT16;
using UINT32 = unsigned int;
using UINT64 = unsigned long long;
using INT8   = char;
using INT16  = short;
using INT32  = int;
using INT64  = long long;

using EFI_GUID = struct {
    UINT32    Data1;
    UINT16    Data2;
    UINT16    Data3;
    UINT8     Data4[8];
};
