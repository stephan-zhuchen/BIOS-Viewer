//
// Created by stephan on 9/4/2023.
//
#pragma once
#include "Volume.h"
#include "UEFI/CapsuleSpec.h"
#include <map>

class CapsuleCommonHeader: public Volume {
private:
    EFI_CAPSULE_HEADER                      CapsuleHeader;
    EFI_FIRMWARE_MANAGEMENT_CAPSULE_HEADER  FmpHeader;
    bool   PersistAcrossReset{false};
    bool   PopulateSystemTable{false};
    bool   InitiateReset{false};
public:
    vector<INT64>                           ItemOffsetVector;

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
    string                                       CapsuleType;
public:
    FirmwareManagementHeader() = delete;
    FirmwareManagementHeader(UINT8* buffer, INT64 length, INT64 offset);
    ~FirmwareManagementHeader() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  setInfoStr() override;

    inline string getCapsuleType() {return CapsuleType;}
    static string getCapsuleTypeFromGuid(EFI_GUID& guid);
};

struct BgupConfig {
    string      BgupContent;
    UINT32      BgupOffset;
    UINT32      BgupSize;
};

class IniConfigFile: public Volume {
private:
    string      iniContext;
    INT32       NumOfUpdate;
    std::map<string, std::map<string, string>> iniData;
public:
    vector<BgupConfig> BgupList;

    IniConfigFile() = delete;
    IniConfigFile(UINT8* buffer, INT64 length, INT64 offset);
    ~IniConfigFile() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  setInfoStr() override;

    string TrimString(const string& inputString);
    string GetIniValue(const string& section, const string& key);
};
