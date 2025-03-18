//
// Created by stephan on 9/5/2023.
//
#pragma once
#include "Volume.h"
#include "UEFI/FIT.h"

class MicrocodeHeaderClass;
class AcmHeaderClass;
class FspBootManifestClass;

class FitTableClass: public Volume {
public:
    FIRMWARE_INTERFACE_TABLE_ENTRY          FitHeader{};
    vector<FIRMWARE_INTERFACE_TABLE_ENTRY>  FitEntries;
    vector<MicrocodeHeaderClass*>           MicrocodeEntries;
    vector<AcmHeaderClass*>                 AcmEntries;
    FspBootManifestClass                    *FbmEntry{nullptr};
    INT64                                   FitEntryNum{0};
    bool                                    isValid{false};
    bool                                    isChecksumValid{false};
public:
    FitTableClass(UINT8* buffer, INT64 length, INT64 offset);
    ~FitTableClass();

    INT64 SelfDecode() override;
    static string getTypeName(UINT8 type);
};
