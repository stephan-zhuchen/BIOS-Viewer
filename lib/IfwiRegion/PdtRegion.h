//
// Created by stephan on 9/5/2023.
//
#pragma once
#include "Volume.h"

class PdtRegion: public Volume {
public:
    PdtRegion() = delete;
    PdtRegion(UINT8* buffer, INT64 length, INT64 offset=0);
    ~PdtRegion() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void  setInfoStr() override;
};
