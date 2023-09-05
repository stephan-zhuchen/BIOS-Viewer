//
// Created by stephan on 9/2/2023.
//
#pragma once
#include "Volume.h"
#include "UEFI/GbE.h"

class GbeRegion: public Volume {
private:
    GBE_MAC_ADDRESS MacAddress{};
    GBE_VERSION     GbeVersion{};
public:
    GbeRegion() = delete;
    GbeRegion(UINT8* buffer, INT64 length, INT64 offset=0);
    ~GbeRegion() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void  setInfoStr() override;
};

