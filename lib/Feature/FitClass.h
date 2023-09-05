//
// Created by stephan on 9/5/2023.
//
#pragma once
#include <QVector>
#include "UEFI/FIT.h"

class MicrocodeHeaderClass;
class AcmHeaderClass;

class FitTableClass {
public:
    FIRMWARE_INTERFACE_TABLE_ENTRY          FitHeader{};
    QVector<FIRMWARE_INTERFACE_TABLE_ENTRY> FitEntries;
    QVector<MicrocodeHeaderClass*>          MicrocodeEntries;
    QVector<AcmHeaderClass*>                AcmEntries;
    INT64 FitEntryNum{0};
    bool  isValid{false};
    bool  isChecksumValid{false};
public:
    FitTableClass(UINT8* fv, INT64 length);
    ~FitTableClass();
    static QString getTypeName(UINT8 type);
};