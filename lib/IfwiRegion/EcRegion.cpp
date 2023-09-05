//
// Created by stephan on 9/2/2023.
//

#include "EcRegion.h"

EcRegion::EcRegion(UINT8 *buffer, INT64 length, INT64 offset):
    Volume(buffer, length, offset, false, nullptr) {}

bool EcRegion::CheckValidation() {
    return Volume::CheckValidation();
}

INT64 EcRegion::SelfDecode() {
    Type = VolumeType::EC;
    UINT32 searchValue;

    for (INT64 searchOffset = 0; searchOffset < size; searchOffset += 2) {
        searchValue = this->getUINT32(searchOffset);
        if (searchValue == 0x43534b54) {
            Signature = "TKSC";
            PlatId = this->getUINT8(searchOffset + 4);
            MajorVer = this->getUINT8(searchOffset + 5);
            MinorVer = this->getUINT8(searchOffset + 6);
            BuildVer = this->getUINT8(searchOffset + 7);
            return size;
        }
    }
    return 0;
}

void EcRegion::DecodeChildVolume() {
    Volume::DecodeChildVolume();
}

void EcRegion::setInfoStr() {
    using namespace std;
    INT32 width = 20;
    stringstream ss;
    ss.setf(ios::left);

    ss << setw(width) << "EC signature:"  << Signature.toStdString() << "\n"
       << setw(width) << "Plat ID:"       << hex << (INT32)PlatId << "h\n"
       << setw(width) << "Build Version:" << hex << (INT32)BuildVer << "h\n"
       << setw(width) << "EC Version:"    << hex << (INT32)MajorVer << "." << hex << (INT32)MinorVer << "\n";

    InfoStr = QString::fromStdString(ss.str());
}

EcRegion::~EcRegion() = default;
