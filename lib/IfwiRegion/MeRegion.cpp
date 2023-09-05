//
// Created by stephan on 9/2/2023.
//

#include "MeRegion.h"
#include <QDebug>

MeRegion::MeRegion(UINT8 *buffer, INT64 length, INT64 offset):
    Volume(buffer, length, offset, false, nullptr) {}

bool MeRegion::CheckValidation() {
    return Volume::CheckValidation();
}

INT64 MeRegion::SelfDecode() {
    Type = VolumeType::ME;
    return Volume::SelfDecode();
}

void MeRegion::DecodeChildVolume() {
    Volume::DecodeChildVolume();
}

void MeRegion::setInfoStr() {
    Volume::setInfoStr();
}

MeRegion::~MeRegion() {
    qDebug() << "~MeRegion";
}
