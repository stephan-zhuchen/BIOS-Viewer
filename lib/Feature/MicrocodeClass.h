//
// Created by stephan on 9/5/2023.
//
#pragma once
#include "UEFI/Microcode.h"

class MicrocodeHeaderClass {
private:
    UINT8* data;
    INT64  size{};
    INT64  offset;
public:
    bool    isEmpty{false};
    QString InfoStr;
    CPU_MICROCODE_HEADER                    microcodeHeader{};
    CPU_MICROCODE_EXTENDED_TABLE_HEADER     *ExtendedTableHeader;
    QVector<CPU_MICROCODE_EXTENDED_TABLE>   ExtendedMicrocodeList;

    MicrocodeHeaderClass()=delete;
    MicrocodeHeaderClass(UINT8* buffer, INT64 address);
    ~MicrocodeHeaderClass();
    void setInfoStr();
};