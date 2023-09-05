//
// Created by stephan on 9/2/2023.
//

#pragma once
#include "Volume.h"

class OsseRegion: public Volume  {
private:
public:
    OsseRegion() = delete;
    OsseRegion(UINT8* buffer, INT64 length, INT64 offset=0);
    ~OsseRegion() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void  setInfoStr() override;
};
