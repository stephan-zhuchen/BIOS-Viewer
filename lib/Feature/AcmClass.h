//
// Created by stephan on 9/5/2023.
//
#pragma once
#include <QString>
#include "Volume.h"
#include "UEFI/BootGuard.h"

class AcmHeaderClass: public Volume {
private:
    bool             ProdFlag{true};
    bool             ValidFlag{true};
    ACM_HEADER       acmHeader{};
    Ext_ACM_Header   ExtAcmHeader{};
    Ext_ACM_Header3  ExtAcmHeader3{};
    ACM_INFO_TABLE   *AcmInfoTable;
    ACM_VERSION      AcmVersion{};
    bool             isAcm3{false};
public:
    AcmHeaderClass() = delete;
    AcmHeaderClass(UINT8* buffer, INT64 length, INT64 offset);
    ~AcmHeaderClass() override;
    void setInfoStr() override;
    INT64 SelfDecode() override;

    [[nodiscard]] inline bool isValid() const { return ValidFlag; };
    [[nodiscard]] inline bool isProd() const { return ProdFlag; };
};