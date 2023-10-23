//
// Created by stephan on 9/5/2023.
//
#pragma once
#include <QString>
#include "Volume.h"
#include "UEFI/FspHeader.h"

class FspHeader: public Volume {
private:
    TABLES   mTable{};
    bool     validFlag{true};
public:
    FspHeader(UINT8* buffer, INT64 length, INT64 offset);
    ~FspHeader();
    [[nodiscard]] bool isValid() const;

    INT64 SelfDecode() override;
    void  setInfoStr() override;

    static bool isFspHeader(const UINT8 *ImageBase);
};
