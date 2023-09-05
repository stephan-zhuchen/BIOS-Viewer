//
// Created by stephan on 9/2/2023.
//

#include "BaseLib.h"
#include "NvVariable.h"
#include "UEFI/GuidDatabase.h"

using namespace BaseLibrarySpace;

NvVariableEntry::NvVariableEntry(UINT8* buffer, INT64 offset, bool isAuth, Volume* parent):
    Volume(buffer, 0, offset, false, parent), AuthFlag(isAuth) { }

bool NvVariableEntry::CheckValidation() {
    return Volume::CheckValidation();
}

INT64 NvVariableEntry::SelfDecode() {
    Type = VolumeType::UserDefined;
    if (AuthFlag) {
        AuthVariableHeader = (AUTHENTICATED_VARIABLE_HEADER*)data;
        size = sizeof(AUTHENTICATED_VARIABLE_HEADER) + AuthVariableHeader->NameSize + AuthVariableHeader->DataSize;
        VariableName = wcharToString((CHAR16*)(AuthVariableHeader + 1), AuthVariableHeader->NameSize, true);
        DataPtr = data + sizeof(AUTHENTICATED_VARIABLE_HEADER) + AuthVariableHeader->NameSize;
        DataSize = AuthVariableHeader->DataSize;
    }
    else {
        VariableHeader = (VARIABLE_HEADER*)data;
        size = sizeof(VARIABLE_HEADER) + VariableHeader->NameSize + VariableHeader->DataSize;
        VariableName = wcharToString((CHAR16*)(AuthVariableHeader + 1), VariableHeader->NameSize, true);
        DataPtr = data + sizeof(VARIABLE_HEADER) + VariableHeader->NameSize;
        DataSize = VariableHeader->DataSize;
    }
    return size;
}

void NvVariableEntry::DecodeChildVolume() {
    Volume::DecodeChildVolume();
}

void NvVariableEntry::setInfoStr() {
    INT32 width = 15;
    stringstream ss;
    ss.setf(ios::left);

    if (AuthFlag) {
        ss << "Variable GUID:\n" << AuthVariableHeader->VendorGuid.str(true) << "\n"
           << setw(width) << "Variable Name:"   << VariableName << "\n"
           << setw(width) << "Variable Size:"   << hex << DataSize << "h\n";
    } else {
        ss << "Variable GUID:\n" << VariableHeader->VendorGuid.str(true) << "\n"
           << setw(width) << "Variable Name:"   << VariableName << "\n"
           << setw(width) << "Variable Size:"   << hex << DataSize << "h\n";
    }
    InfoStr = QString::fromStdString(ss.str());
}

INT64 NvVariableEntry::getHeaderSize() const {
    if (AuthFlag)
        return sizeof(AUTHENTICATED_VARIABLE_HEADER);
    else
        return sizeof(VARIABLE_HEADER);
}

QString NvVariableEntry::getUserDefinedName() const {
    return QString::fromStdString(VariableName);
}

NvVariableEntry::~NvVariableEntry() = default;

NvStorageVariable::NvStorageVariable(UINT8 *buffer, INT64 length, INT64 offset, bool Compressed, Volume *parent):
    Volume(buffer, length, offset, Compressed, parent), AuthFlag(false) { }

bool NvStorageVariable::CheckValidation() {
    return Volume::CheckValidation();
}

INT64 NvStorageVariable::SelfDecode() {
    Type = VolumeType::NvStorage;
    NvStoreHeader = *(VARIABLE_STORE_HEADER*)data;
    size = NvStoreHeader.Size;
    if (NvStoreHeader.Signature == GuidDatabase::gEfiAuthenticatedVariableGuid) {
        AuthFlag = true;
    }
    return size;
}

void NvStorageVariable::DecodeChildVolume() {
    INT64 VariableOffset = sizeof(VARIABLE_STORE_HEADER);
    while (VariableOffset < size) {
        if (*(UINT16*)(data + VariableOffset) != 0x55AA)
            break;
        auto *VarEntry = new NvVariableEntry(data + VariableOffset, offsetFromBegin + VariableOffset, AuthFlag, this);
        INT64 VarSize = VarEntry->SelfDecode();
        VarEntry->DecodeChildVolume();
        ChildVolume.push_back(VarEntry);
        VariableOffset += VarSize;
        Align(VariableOffset, 0, 4);
    }
}

void NvStorageVariable::setInfoStr() {
    if (InfoStr != "") {
        return;
    }
    INT32 width = 15;
    stringstream ss;
    ss.setf(ios::left);

    ss << "Signature:\n" << NvStoreHeader.Signature.str(true) << "\n"
       << setw(width) << "Full size:"   << hex << uppercase << NvStoreHeader.Size << "h\n"
       << setw(width) << "Header size:" << hex << uppercase << sizeof(VARIABLE_STORE_HEADER) << "h\n"
       << setw(width) << "Format:"      << hex << uppercase << NvStoreHeader.Format << "h\n"
       << setw(width) << "State:"       << hex << uppercase << (UINT32)NvStoreHeader.State << "h\n";
    InfoStr = QString::fromStdString(ss.str());
}

NvStorageVariable::~NvStorageVariable() = default;

FaultTolerantBlock::FaultTolerantBlock(UINT8 *buffer, INT64 length, INT64 offset, bool Compressed, Volume *parent)
        : Volume(buffer, length, offset, Compressed, parent) {}

bool FaultTolerantBlock::CheckValidation() {
    return Volume::CheckValidation();
}

INT64 FaultTolerantBlock::SelfDecode() {
    Type = VolumeType::FaultTolerantBlock;
    TolerantHeader = *(EFI_FAULT_TOLERANT_WORKING_BLOCK_HEADER*)data;
    size = sizeof(EFI_FAULT_TOLERANT_WORKING_BLOCK_HEADER);
    return size;
}

void FaultTolerantBlock::DecodeChildVolume() {
    Volume::DecodeChildVolume();
}

void FaultTolerantBlock::setInfoStr() {
    if (InfoStr != "") {
        return;
    }
    INT32 width = 20;
    stringstream ss;
    ss.setf(ios::left);

    ss << "Signature:\n" << TolerantHeader.Signature.str(true) << "\n"
       << setw(width) << "Crc:"             << hex << uppercase << TolerantHeader.Crc << "h\n"
       << setw(width) << "WriteQueue Size:" << hex << uppercase << TolerantHeader.WriteQueueSize << "h\n";
    InfoStr = QString::fromStdString(ss.str());
}

FaultTolerantBlock::~FaultTolerantBlock() = default;
