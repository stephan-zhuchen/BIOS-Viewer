//
// Created by stephan on 9/2/2023.
//

#include "OsseRegion.h"

OsseRegion::OsseRegion(UINT8 *buffer, INT64 length, INT64 offset):
    Volume(buffer, length, offset, false, nullptr) {}

bool OsseRegion::CheckValidation() {
    return Volume::CheckValidation();
}

INT64 OsseRegion::SelfDecode() {
    Type = VolumeType::OSSE;
    return Volume::SelfDecode();
}

void OsseRegion::DecodeChildVolume() {
    Volume::DecodeChildVolume();
}

void OsseRegion::setInfoStr() {
    Volume::setInfoStr();
}

OsseRegion::~OsseRegion() = default;
