//
// Created by stephan on 9/2/2023.
//

#pragma once
#include "Volume.h"

class EcRegion: public Volume {
private:
    QString Signature;
    UINT8   PlatId{};
    UINT8   MajorVer{};
    UINT8   MinorVer{};
    UINT8   BuildVer{};
public:
    EcRegion() = delete;
    EcRegion(UINT8* buffer, INT64 length, INT64 offset=0);
    ~EcRegion() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void  setInfoStr() override;
};

