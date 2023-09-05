//
// Created by stephan on 9/2/2023.
//

#include "GbeRegion.h"

GbeRegion::GbeRegion(UINT8 *buffer, INT64 length, INT64 offset):
    Volume(buffer, length, offset, false, nullptr) {}

bool GbeRegion::CheckValidation() {
    return Volume::CheckValidation();
}

INT64 GbeRegion::SelfDecode() {
    Type = VolumeType::GbE;
    MacAddress = *(GBE_MAC_ADDRESS*)data;
    GbeVersion = *(GBE_VERSION*)(data + GBE_VERSION_OFFSET);
    return size;
}

void GbeRegion::DecodeChildVolume() {
    Volume::DecodeChildVolume();
}

void GbeRegion::setInfoStr() {
    using namespace std;
    INT32 width = 15;
    stringstream ss;
    ss.setf(ios::left);

    ss << setw(width) << "MAC:" << hex << (UINT16)MacAddress.vendor[0] << ":"
       << (UINT16)MacAddress.vendor[1] << ":"
       << (UINT16)MacAddress.vendor[2] << ":"
       << (UINT16)MacAddress.device[0] << ":"
       << (UINT16)MacAddress.device[1] << ":"
       << (UINT16)MacAddress.device[2] << "\n"
       << setw(width) << "Image ID:"  << (UINT16)GbeVersion.id << "\n"
       << setw(width) << "Version:" << (UINT16)GbeVersion.major << "." << (UINT16)GbeVersion.minor << endl;

    InfoStr = QString::fromStdString(ss.str());
}

GbeRegion::~GbeRegion() = default;
