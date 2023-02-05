//
// Created by Stephan on 2022/6/14.
//
#include "BaseLib.h"
#include <string>
#include <algorithm>
#include <iomanip>

using namespace std;

namespace BaseLibrarySpace {
    INT64 Buffer::getBufferSize() const {
        buffer->seekg(0, ios::end);
        INT64 size = buffer->tellg();
        buffer->seekg(0, ios::beg);
        return size;
    }

    vector<pair<array<INT64, 2>, string> > Buffer::saveItem;

    UINT8 Buffer::getUINT8() {
        buffer->seekg(offset, ios::beg);
        offset += 1;
        char value{};
        buffer->read(&value, 1);
        return (UINT16)value;
    }

    UINT16 Buffer::getUINT16() {
        buffer->seekg(offset, ios::beg);
        offset += 2;
        char value[2]{};
        buffer->read(value, 2);
        return *(UINT16*)value;
    }

    UINT32 Buffer::getUINT32() {
        buffer->seekg(offset, ios::beg);
        offset += 4;
        char value[4]{};
        buffer->read(value, 4);
        return *(UINT32*)value;
    }

    UINT64 Buffer::getUINT64() {
        buffer->seekg(offset, ios::beg);
        offset += 8;
        char value[8]{};
        buffer->read(value, 8);
        return *(UINT64*)value;
    }

    INT8 Buffer::getINT8() {
        buffer->seekg(offset, ios::beg);
        offset += 1;
        char value{};
        buffer->read(&value, 1);
        return (INT16)value;
    }

    INT16 Buffer::getINT16() {
        buffer->seekg(offset, ios::beg);
        offset += 2;
        char value[2]{};
        buffer->read(value, 2);
        return *(INT16*)value;
    }

    INT32 Buffer::getINT24() {
        buffer->seekg(offset, ios::beg);
        offset += 3;
        char value[4]{};
        buffer->read(value, 4);
        value[3] = 0;
        return *(INT32*)value;
    }

    INT32 Buffer::getINT32() {
        buffer->seekg(offset, ios::beg);
        offset += 4;
        char value[4]{};
        buffer->read(value, 4);
        return *(INT32*)value;
    }

    INT64 Buffer::getINT64() {
        buffer->seekg(offset, ios::beg);
        offset += 8;
        char value[8]{};
        buffer->read(value, 8);
        return *(INT64*)value;
    }

    EFI_GUID Buffer::getGUID() {
        buffer->seekg(offset, ios::beg);
        offset += 16;
        char value[16]{};
        buffer->read(value, 16);
        EFI_GUID guid = *(EFI_GUID*)value;
        return guid;
    }

    UINT8* Buffer::getBytes(int n) {
        buffer->seekg(offset, ios::beg);
        offset += n;
        char* value = new char[n];
        buffer->read(value, n);
        return (UINT8*)value;
    }

    INT8* Buffer::getString(int n)
    {
        buffer->seekg(offset, ios::beg);
        offset += n;
        char* value = new char[n + 1];
        buffer->read(value, n);
        value[n] = 0;
        return value;
    }

    void Buffer::setOffset(INT64 off) {
        offset = off;
    }

    INT64 Buffer::getOffset() const {
        return offset;
    }

    INT64 Buffer::getRemainingSize() const {
        return getBufferSize() - offset;
    }

    Buffer::Buffer():buffer(nullptr),offset(0) {}

    Buffer::Buffer(ifstream* inFile) {
        buffer = inFile;
        offset = 0;
    }

    Buffer::~Buffer() {
        buffer->close();
        delete buffer;
    }

    void Buffer::Align(INT64& address, INT64 RelativeAddress, INT64 alignment) {
        address -= RelativeAddress;
        address += alignment - 1;
        address = (INT64)(address / alignment);
        address = (INT64)(address * alignment);
        address += RelativeAddress;
    }

    void Buffer::UAlign(UINT64& address, UINT64 RelativeAddress, UINT64 alignment) {
        address -= RelativeAddress;
        address += alignment - 1;
        address = (UINT64)(address / alignment);
        address = (UINT64)(address * alignment);
        address += RelativeAddress;
    }

    void Buffer::prepareBufferToSave(INT64 offset, INT64 size, const string& name) {
        array<INT64, 2> temp {offset, size};
        auto tempPair = make_pair(temp, name);
        saveItem.push_back(tempPair);
    }

    INT64 Buffer::adjustBufferAddress(INT64 FullLength, INT64 offset, INT64 length) {
        return length - (FullLength - offset);
    }

    string Buffer::charToString(INT8* address, INT64 length, bool hasZeroEnding) {
        INT8 *cStr;
        if (hasZeroEnding)
            cStr = new INT8[length];
        else {
            cStr = new INT8[length + 1];
            cStr[length] = 0x0;
        }
        for (int var = 0; var < length; ++var) {
            cStr[var] = *(INT8*)(address + var);
        }
        string result = cStr;
        delete[] cStr;
        return result;
    }

    string Buffer::wcharToString(CHAR16* wcharAddress, INT64 length, bool hasZeroEnding) {
        UINT8* charAddress;
        INT64 charSize = length / 2;
        if (hasZeroEnding)
            charAddress = new UINT8[charSize];
        else {
            charAddress = new UINT8[charSize + 1];
            charAddress[charSize] = 0x0;
        }
        for (int var = 0; var < charSize; ++var) {
            charAddress[var] = (UINT8)*(wcharAddress + var);
        }
        string str = (INT8*)charAddress;
        delete[] charAddress;
        return str;
    }

    string Buffer::wstringToString(CHAR16* wcharAddress) {
        UINT8* charAddress;
        CHAR16* temp = wcharAddress;
        INT64 charSize = 1;
        while (*temp != 0) {
            charSize += 1;
            temp += 1;
        }
        charAddress = new UINT8[charSize];
        for (int var = 0; var < charSize; ++var) {
            charAddress[var] = (UINT8)*(wcharAddress + var);
        }
        string str = (INT8*)charAddress;
        delete[] charAddress;
        return str;
    }

    UINT8 Buffer::CaculateSum8(UINT8 *Buffer, INT64 Size) {
        UINT8 Sum = 0;
        //
        // Perform the byte sum for buffer
        //
        for (INT64 Index = 0; Index < Size; Index++) {
          Sum = (UINT8) (Sum + Buffer[Index]);
        }

        return (UINT8) (0x100 - Sum);
    }

    UINT16 Buffer::CaculateSum16(UINT16 *Buffer, INT64 Size) {
        UINT16  Sum = 0;
        //
        // Perform the word sum for buffer
        //
        for (INT64 Index = 0; Index < Size; Index++) {
          Sum = (UINT16) (Sum + Buffer[Index]);
        }

        return (UINT16) (0x10000 - Sum);
    }

    UINT32 Buffer::CaculateSum32(UINT16 *Buffer, INT64 Size) {
        UINT32  Sum;
        UINTN   Count;
        UINTN   Total;

//        ASSERT (Buffer != nullptr);
//        ASSERT (((UINTN)Buffer & 0x3) == 0);
//        ASSERT ((Length & 0x3) == 0);
//        ASSERT (Length <= (MAX_ADDRESS - ((UINTN)Buffer) + 1));

        Total = Size / sizeof (*Buffer);
        for (Sum = 0, Count = 0; Count < Total; Count++) {
            Sum = Sum + *(Buffer + Count);
        }

        return Sum;
    }

    INT32 Buffer::getSizeFromUINT24(UINT8* address) {
        return *(UINT32*)address & 0xFFFFFF;
    }

    void Buffer::saveBinary(const string& filename, UINT8* address, INT64 offset, INT64 size) {
        ofstream outFile(filename, ios::out | ios::binary);
        outFile.write((INT8*)(address + offset), size);
        outFile.close();
    }

    void Buffer::saveBufferToFile(string& filename, INT64 beginOffset, INT64 bufferSize) const {
        buffer->seekg(beginOffset, ios::beg);
        char* value = new char[bufferSize];
        buffer->read(value, bufferSize);

        ofstream outFile(filename, ios::out | ios::binary);
        outFile.write(value, bufferSize);
        outFile.close();
        delete[] value;
        buffer->seekg(offset, ios::beg);
    }

    GUID::GUID(const GUID& guid) {
        GuidData = guid.GuidData;
    }

    GUID::GUID(const EFI_GUID& cGuid) {
        GuidData = cGuid;
    }

    string GUID::str(bool upper) const {
        stringstream ss;
        if (upper) {
            ss << uppercase;
        }
        ss << hex
           << setfill('0')
           << setw(8)
           << this->GuidData.Data1 << "-"
           << setw(4)
           << this->GuidData.Data2 << "-"
           << setw(4)
           << this->GuidData.Data3 << "-";
        for (int i = 0; i < 8; ++i) {
            if (i == 2) {
                ss << "-";
            }
            ss << setw(2)
                << (UINT16)this->GuidData.Data4[i];
        }
        return ss.str();

    }

    ostream& operator<<(ostream& out, const GUID* guid) {
        out << hex
            << setfill('0')
            << setw(8)
            << guid->GuidData.Data1 << "-"
            << setw(4)
            << guid->GuidData.Data2 << "-"
            << setw(4)
            << guid->GuidData.Data3 << "-";
        for (int i = 0; i < 8; ++i) {
            if (i == 2) {
                out << "-";
            }
            out << setw(2)
                << (UINT16)guid->GuidData.Data4[i];
        }
        return out;
    }

    bool GUID::operator==(const GUID& guid) {
        if (this->GuidData.Data1 != guid.GuidData.Data1) {
            return false;
        }
        if (this->GuidData.Data2 != guid.GuidData.Data2) {
            return false;
        }
        if (this->GuidData.Data3 != guid.GuidData.Data3) {
            return false;
        }
        for (int i = 0; i < 8; ++i) {
            if (this->GuidData.Data4[i] != guid.GuidData.Data4[i]) {
                return false;
            }
        }
        return true;
    }

    bool GUID::operator!=(const GUID& guid) {
        if (*this == guid) {
            return false;
        }
        return true;
    }

    GUID::GUID(const char* buffer) {
        string str = buffer;
        char* endptr;
        string tempStr[5];
        const char* split = "-";
        string strs = str + split;
        size_t pos = strs.find(split);

        for (auto& i : tempStr)
        {
            string temp = strs.substr(0, pos);
            i = temp;
            strs = strs.substr(pos + 1, strs.size());
            pos = strs.find(split);
        }

        GuidData.Data1 = strtoul(tempStr[0].data(), &endptr, 16);
        GuidData.Data2 = strtoul(tempStr[1].data(), &endptr, 16);
        GuidData.Data3 = strtoul(tempStr[2].data(), &endptr, 16);
        GuidData.Data4[0] = strtoul(tempStr[3].substr(0, 2).data(), &endptr, 16);
        GuidData.Data4[1] = strtoul(tempStr[3].substr(2, 2).data(), &endptr, 16);
        for (int i = 0; i < 6; ++i) {
            GuidData.Data4[i + 2] = strtoul(tempStr[4].substr(i * 2, 2).data(), &endptr, 16);
        }
    }

    GUID& GUID::operator=(const GUID& guid) {
        if (this != &guid) {
            this->GuidData = guid.GuidData;
        }
        return *this;
    }

    GUID& GUID::operator=(const EFI_GUID& guid) {
        this->GuidData = guid;
        return *this;
    }

    CapsuleException::CapsuleException() : message("Error.") {}

    CapsuleException::CapsuleException(const string& str) : message("Error : " + str) {}

    const char* CapsuleException::what() const noexcept {
        return message.c_str();
    }

    CapsuleError::CapsuleError() : message("Error.") {}

    CapsuleError::CapsuleError(const string& str) : message(str) {}

    const char* CapsuleError::what() const noexcept {
        return message.c_str();
    }
}
