//
// Created by stephan on 9/5/2023.
//

#include "PdtRegion.h"

PdtRegion::PdtRegion(UINT8 *buffer, INT64 length, INT64 offset):
    Volume(buffer, length, offset, false, nullptr) {}

bool PdtRegion::CheckValidation() {
    return Volume::CheckValidation();
}

INT64 PdtRegion::SelfDecode() {
    Type = VolumeType::IshPdt;
    return Volume::SelfDecode();
}

void PdtRegion::DecodeChildVolume() {
    Volume::DecodeChildVolume();
}

void PdtRegion::setInfoStr() {
    Volume::setInfoStr();
}

PdtRegion::~PdtRegion() = default;

