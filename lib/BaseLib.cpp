//
// Created by Stephan on 2022/6/14.
//
#include "BaseLib.h"
#include <string>
#include <algorithm>
#include <iomanip>

using namespace std;

namespace BaseLibrarySpace {
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
    string charToString(const CHAR8* address, INT64 length, bool hasZeroEnding) {
        CHAR8 *cStr;
        if (hasZeroEnding)
            cStr = new CHAR8[length];
        else {
            cStr = new CHAR8[length + 1];
            cStr[length] = 0x0;
        }
        for (int var = 0; var < length; ++var) {
            cStr[var] = *(CHAR8*)(address + var);
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
        string str = (CHAR8*)charAddress;
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
        string str = (CHAR8*)charAddress;
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

    string DumpHex(UINT8* HexData, INT64 length, INT64 ColumeSize, bool SingleLine, INT64 indent) {
        auto InternalDumpData = [](stringstream &ss, UINT8* Data, INT64 Size) {
            for (INT64 Index = 0; Index < Size; Index++) {
                ss << setw(2) << setfill('0') << hex << (UINT16)Data[Index] << " ";
            }
        };
        stringstream ss;
        if (SingleLine) {
            for (int i = 0; i < length; ++i) {
                ss << setw(indent) << std::setfill(' ') << "" << setw(2) << setfill('0') << hex << (UINT16)HexData[i];
            }
            return ss.str();
        }

        INT64 Index;
        INT64 Count = length / ColumeSize;
        INT64 Left  = length % ColumeSize;
        for (Index = 0; Index < Count; Index++) {
            ss << setw(indent) << std::setfill(' ') << "" << setw(4) << setfill('0') << hex << Index * ColumeSize << ": ";
            InternalDumpData(ss, HexData + Index * ColumeSize, ColumeSize);
            ss << "\n";
        }
        if (Left != 0) {
            ss << setw(indent) << std::setfill(' ') << "" << setw(4) << setfill('0') << hex << Index * ColumeSize << ": ";
            InternalDumpData (ss, HexData + Index * ColumeSize, Left);
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
        outFile.write((CHAR8*)(address + offset), size);
        outFile.close();
    }
}
