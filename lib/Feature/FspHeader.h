//
// Created by stephan on 9/5/2023.
//
#pragma once
#include <QString>
#include "UEFI/FspHeader.h"

class FspHeader {
private:
    UINT8*   data;
    INT64    size{};
    INT64    offset{};
    TABLES   mTable{};
    bool     validFlag{true};
    QString  InfoStr;
public:
    FspHeader(UINT8* buffer, INT64 length, INT64 offset);
    ~FspHeader();
    [[nodiscard]] bool isValid() const;
    void setInfoStr();
    static bool isFspHeader(const UINT8  *ImageBase);
    [[nodiscard]] QString getInfoText() const { return InfoStr; }
};