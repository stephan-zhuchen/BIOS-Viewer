//
// Created by stephan on 9/5/2023.
//
#pragma once
#include <QString>
#include "UEFI/BootGuard.h"

class AcmHeaderClass {
private:
    UINT8*           data;
    INT64            size{};
    INT64            offset{};
    bool             ProdFlag{true};
    bool             ValidFlag{true};
    QString          InfoStr;
    ACM_HEADER       acmHeader{};
    Ext_ACM_Header   ExtAcmHeader{};
    Ext_ACM_Header3  ExtAcmHeader3{};
    ACM_INFO_TABLE   *AcmInfoTable;
    ACM_VERSION      AcmVersion{};
    bool             isAcm3{false};
public:
    AcmHeaderClass() = delete;
    AcmHeaderClass(UINT8* buffer, INT64 address);
    ~AcmHeaderClass();
    [[nodiscard]] inline bool isValid() const { return ValidFlag; };
    [[nodiscard]] inline bool isProd() const { return ProdFlag; };
    void setInfoStr();
    [[nodiscard]] QString getInfoText() const { return InfoStr; }
};