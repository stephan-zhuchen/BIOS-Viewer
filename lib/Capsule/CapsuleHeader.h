//
// Created by stephan on 9/4/2023.
//
#pragma once
#include <string>
#include "Volume.h"
#include "UEFI/CapsuleSpec.h"

class CapsuleCommonHeader: public Volume {
private:
    EFI_CAPSULE_HEADER                      CapsuleHeader;
    EFI_FIRMWARE_MANAGEMENT_CAPSULE_HEADER  FmpHeader;
    bool   PersistAcrossReset{false};
    bool   PopulateSystemTable{false};
    bool   InitiateReset{false};
public:
    QVector<INT64>                          ItemOffsetVector;

    CapsuleCommonHeader() = delete;
    CapsuleCommonHeader(UINT8* buffer, INT64 length, INT64 offset);
    ~CapsuleCommonHeader() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  setInfoStr() override;
};

class FirmwareManagementHeader: public Volume {
private:
    EFI_FIRMWARE_MANAGEMENT_CAPSULE_IMAGE_HEADER  FmpCapsuleImageHeader;
    EFI_FIRMWARE_IMAGE_AUTHENTICATION             FmpAuthHeader;
    FMP_PAYLOAD_HEADER                            FmpPayloadHeader;
    QString                                       CapsuleType;
public:
    FirmwareManagementHeader() = delete;
    FirmwareManagementHeader(UINT8* buffer, INT64 length, INT64 offset);
    ~FirmwareManagementHeader() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  setInfoStr() override;

    inline QString getCapsuleType() {return CapsuleType;}
    static QString getCapsuleTypeFromGuid(EFI_GUID& guid);
};

struct BgupConfig {
    std::string BgupContent;
    UINT32      BgupOffset;
    UINT32      BgupSize;
};

class IniConfigFile: public Volume {
private:
    std::string         iniContext;
    INT32               NumOfUpdate;
    std::map<std::string, std::map<std::string, std::string>> iniData;
public:
    QVector<BgupConfig> BgupList;

    IniConfigFile() = delete;
    IniConfigFile(UINT8* buffer, INT64 length, INT64 offset);
    ~IniConfigFile() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  setInfoStr() override;

    std::string TrimString(const std::string& inputString);
    std::string GetIniValue(const std::string& section, const std::string& key);
};