//
// Created by stephan on 9/5/2023.
//
#pragma once
#include "Volume.h"
#include <string>
#include "UEFI/Microcode.h"

class MicrocodeHeaderClass: public Volume {
public:
    bool                                    isEmpty{false};
    bool                                    isCorrupted{false};
    CPU_MICROCODE_HEADER                    microcodeHeader{};
    CPU_MICROCODE_EXTENDED_TABLE_HEADER     *ExtendedTableHeader;
    QVector<CPU_MICROCODE_EXTENDED_TABLE>   ExtendedMicrocodeList;

    MicrocodeHeaderClass()=delete;
    MicrocodeHeaderClass(UINT8* buffer, INT64 length, INT64 offset);
    ~MicrocodeHeaderClass() override;

    bool CheckValidation() override;
    INT64 SelfDecode() override;
    void setInfoStr() override;
    [[nodiscard]] QStringList getUserDefinedName() const override;

    static QVector<INT64> SearchMicrocodeEntryNum(const UINT8* buffer, INT64 MicrocodeRegionSize);
};

class MicrocodeVersion: public Volume {
private:
    UINT32 FwVersion{};
    UINT32 LowestSupportedVersion{};
    std::string FwVersionString{};
public:
    MicrocodeVersion()=delete;
    MicrocodeVersion(UINT8* buffer, INT64 length, INT64 offset);
    ~MicrocodeVersion() override;

    INT64 SelfDecode() override;
    void setInfoStr() override;
};
