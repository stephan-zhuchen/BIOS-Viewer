//
// Created by stephan on 9/2/2023.
//

#pragma once
#include "Volume.h"
#include "UEFI/ME.h"

class MeRegion: public Volume {
private:
    ME_VERSION        MeVersion{};
//    CSE_LayoutClass   *CSE_Layout{nullptr};
public:
    MeRegion() = delete;
    MeRegion(UINT8* buffer, INT64 length, INT64 offset=0);
    ~MeRegion() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void  setInfoStr() override;
};
