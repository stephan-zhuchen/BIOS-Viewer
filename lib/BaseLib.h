//
// Created by Stephan on 2022/6/14.
//
#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <array>
#include <utility>
#include <exception>
#include <fstream>
#include "SymbolDefinition.h"

namespace BaseLibrarySpace {

    using namespace std;

    class GUID {
    public:
        EFI_GUID GuidData{};
        GUID() = delete;
        GUID(const GUID& guid);
        explicit GUID(const char* buffer);
        explicit GUID(const EFI_GUID& cGuid);
        string str(bool upper=false) const;
        friend ostream& operator<<(ostream& out, const GUID* guid);
        bool operator==(const GUID& guid);
        bool operator!=(const GUID& guid);
        GUID& operator=(const GUID& guid);
        GUID& operator=(const EFI_GUID& guid);
    };

    class Buffer {
    public:
        ifstream* buffer;
        INT64 offset;

        EFI_GUID getGUID();
        UINT8  getUINT8();
        UINT16 getUINT16();
        UINT32 getUINT32();
        UINT64 getUINT64();
        INT8   getINT8();
        INT16  getINT16();
        INT32  getINT24();
        INT32  getINT32();
        INT64  getINT64();
        UINT8* getBytes(int n);
        INT8*  getString(int n);

        Buffer();
        explicit Buffer(ifstream* inFile);
        ~Buffer();

        void setOffset(INT64 off);
        INT64 getOffset() const;
        INT64 getBufferSize() const;
        INT64 getRemainingSize() const;
        static void   Align(INT64& address, INT64 RelativeAddress, INT64 alignment);
        static INT64  adjustBufferAddress(INT64 FullLength, INT64 offset, INT64 length);
        static string charToString(const INT8* address, INT64 length, bool hasZeroEnding=false);
        static string wcharToString(const CHAR16* address, INT64 length, bool hasZeroEnding=false);
        static string wstringToString(CHAR16* wcharAddress);
        static UINT8  CalculateSum8(const UINT8 *Buffer, INT64 Size);
        static UINT16 CalculateSum16(const UINT16 *Buffer, INT64 Size);
        static UINT32 CalculateSum32(const UINT32 *Buffer, INT64 Size);
        static INT32  getSizeFromUINT24(const UINT8* address);
        static void   saveBinary(const string& filename, UINT8* address, INT64 offset, INT64 size);
    };

    class BiosException : public exception {
    private:
        string message;
    public:
        BiosException();
        explicit BiosException(const string& str);
        const char* what() const noexcept override;
    };

    template <typename T>
    void safeDelete(T*& ptr)
    {
        delete ptr;
        ptr = nullptr; // To avoid dangling pointer issues
    }

    template <typename T>
    void safeArrayDelete(T *&ptr)
    {
        delete[] ptr;
        ptr = nullptr;
    }
}
