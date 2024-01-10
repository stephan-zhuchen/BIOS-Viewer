#include "Vpd.h"
#include "BaseLib.h"
#include "UefiFileSystem/NvVariable.h"

using namespace BaseLibrarySpace;

Vpd::Vpd(UINT8* buffer, INT64 length, INT64 offset, bool Compressed, Volume* parent):
    Volume(buffer, length, offset, Compressed, parent) { }

Vpd::~Vpd() = default;

bool Vpd::CheckValidation() {
    while (VpdOffset < size) {
        UINT32 VpdSignature = *(UINT32*)(data + VpdOffset);
        if (VpdSignature == 0x4244534E) { // NSDB
            return true;
        }
        VpdOffset += 4;
    }
    return false;
}

INT64 Vpd::SelfDecode() {
    if (!CheckValidation()) {
        return 0;
    }
    Type = VolumeType::Vpd;
    VpdPcdHeader = *(PCD_NV_STORE_DEFAULT_BUFFER_HEADER*)(data + VpdOffset);
    VpdOffset += sizeof(PCD_NV_STORE_DEFAULT_BUFFER_HEADER);

    PcdDefaultData = *(PCD_DEFAULT_DATA*)(data + VpdOffset);
    VpdOffset += sizeof(PCD_DEFAULT_DATA);

    VariableStoreHeader = *(VARIABLE_STORE_HEADER*)(data + VpdOffset);
    auto NvStorage = new NvStorageVariable(data + VpdOffset, VariableStoreHeader.Size, offsetFromBegin + VpdOffset, Compressed, this);
    NvStorage->SelfDecode();
    NvStorage->DecodeChildVolume();
    this->ChildVolume.push_back(NvStorage);
    return VpdOffset;
}

void Vpd::DecodeChildVolume() {

}

void Vpd::setInfoStr() {
    INT32 width = 15;
    stringstream ss;
    ss.setf(ios::left);

    ss << setw(width) << "Signature:" << charToString((CHAR8*)&VpdPcdHeader.Signature, sizeof(UINT32)) << "\n"
       << setw(width) << "Length:"    << hex << VpdPcdHeader.Length << "h\n"
       << setw(width) << "MaxLength:" << hex << VpdPcdHeader.MaxLength << "h\n";

    string compressed = "No";
    if (isCompressed())
        compressed = "Yes";
    ss << "\nCompressed: " << compressed;
    InfoStr = QString::fromStdString(ss.str());
}
