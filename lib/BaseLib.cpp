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
        return value;
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
        if (n <= 0)
            return nullptr;
        buffer->seekg(offset, ios::beg);
        offset += n;
        char* value = new char[n];
        buffer->read(value, n);
        return (UINT8*)value;
    }

    string Buffer::getString(int n)
    {
        buffer->seekg(offset, ios::beg);
        offset += n;
        char* value = new char[n + 1];
        buffer->read(value, n);
        value[n] = 0;
        string str(value);
        delete[] value;
        return str;
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
        GuidData.Data2 = (UINT16)strtoul(tempStr[1].data(), &endptr, 16);
        GuidData.Data3 = (UINT16)strtoul(tempStr[2].data(), &endptr, 16);
        GuidData.Data4[0] = (UINT8)strtoul(tempStr[3].substr(0, 2).data(), &endptr, 16);
        GuidData.Data4[1] = (UINT8)strtoul(tempStr[3].substr(2, 2).data(), &endptr, 16);
        for (int i = 0; i < 6; ++i) {
            GuidData.Data4[i + 2] = (UINT8)strtoul(tempStr[4].substr(i * 2, 2).data(), &endptr, 16);
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

    BiosException::BiosException() : message("Error.") {}

    BiosException::BiosException(const string& str) : message("Error : " + str) {}

    const char* BiosException::what() const noexcept {
        return message.c_str();
    }

    /**
    * @brief Aligns a given address to a specified alignment.
    *
    * This function aligns the given address to the specified alignment by adjusting the address value.
    *
    * @param address The memory address to be aligned. It is modified by the function.
    * @param RelativeAddress The relative address to be aligned, given with respect to the original address.
    * @param alignment The alignment value. The address will be aligned to a multiple of this value.
    */
    void Align(INT64& address, INT64 RelativeAddress, INT64 alignment) {
        address -= RelativeAddress;
        address += alignment - 1;
        address = (INT64)(address / alignment);
        address = (INT64)(address * alignment);
        address += RelativeAddress;
    }

    /**
    * @brief Adjusts the buffer address based on the offset and length values.
    *
    * This function is used to calculate the adjusted buffer address based on the
    * provided FullLength, offset, and length values. The adjusted address is used
    * to efficiently access a specific portion of the buffer.
    *
    * @param FullLength The length of the full buffer.
    * @param offset The offset value indicating the starting position of the desired portion.
    * @param length The length of the desired portion.
    *
    * @return The adjusted buffer address.
    */
    INT64 adjustBufferAddress(INT64 FullLength, INT64 offset, INT64 length) {
        return length - (FullLength - offset);
    }

    /**
    * @brief Converts a character array to a string.
    *
    * This function takes a character array and converts it into a string representation.
    * The resulting string will be terminated by a null character if `hasZeroEnding` is true.
    *
    * @param address A pointer to the character array.
    * @param length The length of the character array.
    * @param hasZeroEnding Indicates whether the resulting string should be terminated by a null character.
    *
    * @return The converted string.
    *
    * @note The `address` parameter must point to a valid character array.
    * @note The `length` parameter must be equal to or greater than zero.
    * @note If `hasZeroEnding` is true, the length of the resulting string will be `length + 1`.
    * @note It is the caller's responsibility to ensure that the resulting string does not exceed the maximum size limit of a string.
    */
    string charToString(const INT8* address, INT64 length, bool hasZeroEnding) {
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

    /**
    * @brief Convert WCHAR string to std::string.
    *
    * This function converts a WCHAR string to a std::string. It allows for specifying the length
    * of the WCHAR string, as well as whether or not the WCHAR string has a zero-ending character.
    *
    * @param wcharAddress Pointer to the start of the WCHAR string.
    * @param length Length of the WCHAR string.
    * @param hasZeroEnding Specifies whether or not the WCHAR string has a zero-ending character.
    *
    * @return Converted std::string object.
    */
    string wcharToString(const CHAR16* wcharAddress, INT64 length, bool hasZeroEnding) {
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

    /**
    * @brief Converts a wide string to a standard string.
    *
    * This function takes a null-terminated wide character string (wcharAddress) and
    * converts it to a null-terminated standard character string. The standard string
    * representation is returned.
    *
    * @param wcharAddress The address of the wide character string to convert.
    * @return The standard string representation of the wide character string.
    */
    string wstringToString(CHAR16* wcharAddress) {
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

    /**
    * @brief Calculates the sum of the values in an array of UINT8 integers.
    *
    * This function takes a buffer of UINT8 integers and calculates the sum of these values. The size of the buffer
    * is provided as a parameter.
    *
    * @param Buffer Pointer to an array of UINT8 integers.
    * @param Size The size of the buffer.
    *
    * @return The sum of the values in the buffer.
    */
    UINT8 CalculateSum8(const UINT8 *Buffer, INT64 Size) {
        UINT8 Sum = 0;
        //
        // Perform the byte sum for buffer
        //
        for (INT64 Index = 0; Index < Size; Index++) {
            Sum = (UINT8) (Sum + Buffer[Index]);
        }

        return (UINT8) (0x100 - Sum);
    }

    /**
    * @brief Calculates the sum of unsigned 16-bit integers in a given buffer.
    *
    * This function takes an array of 16-bit unsigned integers (Buffer) and the size of the buffer (Size),
    * and calculates the sum of all the elements in the buffer. The buffer is expected to be a valid memory
    * region with at least 'Size' elements.
    *
    * @param Buffer    Pointer to the array of 16-bit unsigned integers.
    * @param Size      The number of elements in the array.
    *
    * @return The sum of all the 16-bit unsigned integers in the buffer.
    */
    UINT16 CalculateSum16(const UINT16 *Buffer, INT64 Size) {
        UINT16  Sum = 0;
        //
        // Perform the word sum for buffer
        //
        for (INT64 Index = 0; Index < Size; Index++) {
            Sum = (UINT16) (Sum + Buffer[Index]);
        }

        return (UINT16) (0x10000 - Sum);
    }

    /**
    * @brief Calculates the sum of a 32-bit unsigned integer array.
    *
    * This function takes a buffer of unsigned 32-bit integers and calculates the sum of all the integers in the buffer.
    *
    * @param Buffer Pointer to the buffer containing the unsigned 32-bit integers.
    * @param Size The size of the buffer.
    *
    * @return The sum of all the unsigned 32-bit integers in the buffer.
    */
    UINT32 CalculateSum32(const UINT32 *Buffer, INT64 Size) {
        UINT32  Sum;
        UINTN   Count;
        UINTN   Total;

        Total = Size / sizeof (*Buffer);
        for (Sum = 0, Count = 0; Count < Total; Count++) {
            Sum = Sum + *(Buffer + Count);
        }
        return Sum;
    }

    /**
    * @brief Retrieves the size from a UINT24 address.
    *
    * This function takes a UINT24 address and retrieves the size
    * stored within the address.
    *
    * @param address Pointer to the UINT24 address.
    * @return The size retrieved from the UINT24 address.
    */
    INT32 getSizeFromUINT24(const UINT8* address) {
        return (INT32)*(UINT32*)address & 0xFFFFFF;
    }

    string DumpHex(UINT8* HexData, INT64 length, bool SingleLine) {
        const INT64 COLUME_SIZE = 16;
        auto InternalDumpData = [](stringstream &ss, UINT8* Data, INT64 Size) {
            for (INT64 Index = 0; Index < Size; Index++) {
                ss << setw(2) << setfill('0') << hex << (UINT16)Data[Index] << " ";
            }
        };
        stringstream ss;
        if (SingleLine) {
            for (int i = 0; i < length; ++i) {
                ss << setw(2) << setfill('0') << hex << (UINT16)HexData[i];
            }
            return ss.str();
        }

        INT64 Index;
        INT64 Count = length / COLUME_SIZE;
        INT64 Left  = length % COLUME_SIZE;
        for (Index = 0; Index < Count; Index++) {
            ss << setw(3) << setfill('0') << hex << Index * COLUME_SIZE << ": ";
            InternalDumpData(ss, HexData + Index * COLUME_SIZE, COLUME_SIZE);
            ss << "\n";
        }
        if (Left != 0) {
            ss << setw(3) << setfill('0') << hex << Index * COLUME_SIZE << ": ";
            InternalDumpData (ss, HexData + Index * COLUME_SIZE, Left);
            ss << "\n";
        }
        return ss.str();
    }

    /**
    * @brief Save binary data to a file.
    *
    * This function saves binary data to a file given by the filename parameter.
    *
    * @param filename The name of the file to save the binary data to.
    * @param address Pointer to the memory address containing the binary data.
    * @param offset The offset from the beginning of the data to start saving.
    * @param size The size, in bytes, of the binary data to save.
    *
    * @return void
    *
    */
    void saveBinary(const string& filename, UINT8* address, INT64 offset, INT64 size) {
        ofstream outFile(filename, ios::out | ios::binary);
        outFile.write((INT8*)(address + offset), size);
        outFile.close();
    }
}
