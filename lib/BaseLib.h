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
#include "../include/SymbolDefinition.h"

namespace BaseLibrarySpace {

    using namespace std;

    class GUID {
    public:
        EFI_GUID GuidData{};
        GUID() = delete;
        GUID(const GUID& guid);
        GUID(const char* buffer);
        GUID(const EFI_GUID& cGuid);
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
        static vector<pair<array<INT64, 2>, string> >saveItem;

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
        Buffer(ifstream* inFile);
        ~Buffer();

        void setOffset(INT64 off);
        INT64 getOffset() const;
        INT64 getBufferSize() const;
        INT64 getRemainingSize() const;
        static void Align(INT64& address, INT64 RelativeAddress, INT64 alignment);
        static void prepareBufferToSave(INT64 offset, INT64 size, const string& name);
        static INT64 adjustBufferAddress(INT64 FullLength, INT64 offset, INT64 length);
        static string charToString(INT8* address, INT64 length, bool hasZeroEnding=false);
        static string wstringToString(CHAR16* wcharAddress);
        static UINT8 CaculateSum8(UINT8 *Buffer, INT64 Size);
        static UINT16 CaculateSum16(UINT16 *Buffer, INT64 Size);
        void saveBufferToFile(string& filename, INT64 beginOffset, INT64 bufferSize) const;
    };

    class CapsuleException : public exception {
    private:
        string message;
    public:
        CapsuleException();
        explicit CapsuleException(const string& str);
        const char* what() const noexcept override;
    };

    class CapsuleError : public exception {
    private:
        string message;
    public:
        CapsuleError();
        explicit CapsuleError(const string& str);
        const char* what() const noexcept override;
    };
}
