//
// Created by stephan on 9/4/2023.
//
#include "BaseLib.h"
#include "CapsuleHeader.h"
#include "UEFI/GuidDatabase.h"

using namespace BaseLibrarySpace;

CapsuleCommonHeader::CapsuleCommonHeader(UINT8* buffer, INT64 length, INT64 offset):
    Volume(buffer, length, offset, false, nullptr) {}

CapsuleCommonHeader::~CapsuleCommonHeader() = default;

bool CapsuleCommonHeader::CheckValidation() {
    CapsuleHeader = *(EFI_CAPSULE_HEADER*)data;
    if (CapsuleHeader.CapsuleGuid != GuidDatabase::gEfiFmpCapsuleGuid) {
        return false;
    }
    return true;
}

INT64 CapsuleCommonHeader::SelfDecode() {
    if (!CheckValidation()) {
        return 0;
    }
    Type = VolumeType::CapsuleCommonHeader;
    PersistAcrossReset  = (CapsuleHeader.Flags & CAPSULE_FLAGS_PERSIST_ACROSS_RESET) != 0;
    PopulateSystemTable = (CapsuleHeader.Flags & CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE) != 0;
    InitiateReset       = (CapsuleHeader.Flags & CAPSULE_FLAGS_INITIATE_RESET) != 0;

    INT64 offset = sizeof(EFI_CAPSULE_HEADER);
    Align(offset, 0, 0x8);

    FmpHeader = *(EFI_FIRMWARE_MANAGEMENT_CAPSULE_HEADER*)(data + offset);
    ItemOffsetVector.resize(FmpHeader.EmbeddedDriverCount + FmpHeader.PayloadItemCount);
    offset += sizeof(EFI_FIRMWARE_MANAGEMENT_CAPSULE_HEADER);
    for (int index = 0; index < FmpHeader.EmbeddedDriverCount + FmpHeader.PayloadItemCount; index++) {
        INT64 ItemOffset = this->getINT64(offset);
        offset += sizeof(INT64);
        if (ItemOffset > size) {
            return 0;
        }
        ItemOffsetVector[index] = ItemOffset + 0x20;
    }
    size = offset;

    return size;
}

void CapsuleCommonHeader::setInfoStr() {
    using namespace std;
    INT32 width = 25;
    stringstream ss;
    ss.setf(ios::left);

    ss << setw(width) << "CapsuleGuid:"        << CapsuleHeader.CapsuleGuid.str(true) << "\n"
       << setw(width) << "HeaderSize:"         << hex << uppercase << CapsuleHeader.HeaderSize << "h\n"
       << setw(width) << "Flags:"              << hex << uppercase << CapsuleHeader.Flags << "h\n"
       << setw(width) << "CapsuleImageSize:"   << hex << uppercase << CapsuleHeader.CapsuleImageSize << "h\n";

    ss << "\nFlag:\n";
    if (PersistAcrossReset) {ss << "PersistAcrossReset\n";}
    if (PopulateSystemTable) {ss << "PopulateSystemTable\n";}
    if (InitiateReset) {ss << "InitiateReset\n\n";}

    ss << setw(width) << "Version:"              << hex << uppercase << FmpHeader.Version << "h\n"
       << setw(width) << "EmbeddedDriverCount:"  << hex << uppercase << FmpHeader.EmbeddedDriverCount << "h\n"
       << setw(width) << "PayloadItemCount:"     << hex << uppercase << FmpHeader.PayloadItemCount << "h\n";

    InfoStr = QString::fromStdString(ss.str());
}

FirmwareManagementHeader::FirmwareManagementHeader(UINT8* buffer, INT64 length, INT64 offset):
    Volume(buffer, length, offset, false, nullptr) {}

FirmwareManagementHeader::~FirmwareManagementHeader() = default;

bool FirmwareManagementHeader::CheckValidation() {
    return true;
}

INT64 FirmwareManagementHeader::SelfDecode() {
    Type = VolumeType::FirmwareManagementHeader;
    FmpCapsuleImageHeader = *(EFI_FIRMWARE_MANAGEMENT_CAPSULE_IMAGE_HEADER*)data;
    INT64 offset = 0;
    if (FmpCapsuleImageHeader.Version <= 2) {
        offset = sizeof(EFI_FIRMWARE_MANAGEMENT_CAPSULE_IMAGE_HEADER) - sizeof(UINT64);
    } else {
        offset = sizeof(EFI_FIRMWARE_MANAGEMENT_CAPSULE_IMAGE_HEADER);
    }

    CapsuleType = getCapsuleTypeFromGuid(FmpCapsuleImageHeader.UpdateImageTypeId);

    FmpAuthHeader = *(EFI_FIRMWARE_IMAGE_AUTHENTICATION*)(data + offset);

    INT64 MonotonicCountSize = 8;
    offset += FmpAuthHeader.AuthInfo.Hdr.dwLength + MonotonicCountSize;
    FmpPayloadHeader = *(FMP_PAYLOAD_HEADER*)(data + offset);
    QString Signature = QString::fromStdString(charToString((CHAR8*)&FmpPayloadHeader.Signature, sizeof(FmpPayloadHeader.Signature), false));
    if (Signature != "MSS1")
        return 0;

    size = offset + sizeof(FMP_PAYLOAD_HEADER);
    return size;
}

void FirmwareManagementHeader::setInfoStr() {
    using namespace std;
    INT32 width = 25;
    stringstream ss;
    ss.setf(ios::left);

    ss << setw(width) << "Version:"              << hex << uppercase << FmpCapsuleImageHeader.Version << "h\n"
       << setw(width) << "UpdateImageTypeId:"    << FmpCapsuleImageHeader.UpdateImageTypeId.str(true) << "\n"
       << setw(width) << "UpdateImageIndex:"     << hex << uppercase << (UINT32)FmpCapsuleImageHeader.UpdateImageIndex << "h\n"
       << setw(width) << "UpdateImageSize:"      << hex << uppercase << FmpCapsuleImageHeader.UpdateImageSize << "h\n"
       << setw(width) << "UpdateVendorCodeSize:" << hex << uppercase << FmpCapsuleImageHeader.UpdateVendorCodeSize << "h\n";

    if (FmpCapsuleImageHeader.Version >= 2) {
        ss << setw(width) << "UpdateHardwareInstance:" << hex << uppercase << FmpCapsuleImageHeader.UpdateHardwareInstance << "h\n";
    }
    if (FmpCapsuleImageHeader.Version >= 3) {
        ss << setw(width) << "ImageCapsuleSupport:" << hex << uppercase << FmpCapsuleImageHeader.ImageCapsuleSupport << "h\n";
    }

    ss << setw(width) << "MonotonicCount:"     << hex << uppercase << FmpAuthHeader.MonotonicCount << "h\n"
       << setw(width) << "dwLength:"           << hex << uppercase << FmpAuthHeader.AuthInfo.Hdr.dwLength << "h\n"
       << setw(width) << "wRevision:"          << hex << uppercase << FmpAuthHeader.AuthInfo.Hdr.wRevision << "h\n"
       << setw(width) << "wCertificateType:"   << hex << uppercase << FmpAuthHeader.AuthInfo.Hdr.wCertificateType << "h\n"
       << setw(width) << "CertType:"           << hex << uppercase << FmpAuthHeader.AuthInfo.CertType.str(true) << "h\n\n";

    ss << setw(width) << "Signature:"              << charToString((CHAR8*)&FmpPayloadHeader.Signature, sizeof(FmpPayloadHeader.Signature), false) << "\n"
       << setw(width) << "HeaderSize:"             << hex << uppercase << FmpPayloadHeader.HeaderSize << "h\n"
       << setw(width) << "FwVersion:"              << hex << uppercase << FmpPayloadHeader.FwVersion << "h\n"
       << setw(width) << "LowestSupportedVersion:" << hex << uppercase << FmpPayloadHeader.LowestSupportedVersion << "h\n";

    InfoStr = QString::fromStdString(ss.str());
}

QString FirmwareManagementHeader::getCapsuleTypeFromGuid(EFI_GUID& guid) {
    if (guid == GuidDatabase::gFmpDeviceMonolithicDefaultGuid) {
        return "Monolithic";
    }
    else if (guid == GuidDatabase::gFmpDeviceBiosDefaultGuid) {
        return "BIOS";
    }
    else if (guid == GuidDatabase::gFmpDeviceExtendedBiosDefaultGuid) {
        return "Extended BIOS";
    }
    else if (guid == GuidDatabase::gFmpDeviceIfwiDefaultGuid) {
        return "IFWI";
    }
    else if (guid == GuidDatabase::gFmpDeviceBtGAcmDefaultGuid) {
        return "BtgAcm";
    }
    else if (guid == GuidDatabase::gFmpDeviceMicrocodeDefaultGuid) {
        return "uCode";
    }
    else if (guid == GuidDatabase::gFmpDeviceMeDefaultGuid) {
        return "ME";
    }
    else if (guid == GuidDatabase::gFmpDeviceEcDefaultGuid) {
        return "EC";
    }
    else if (guid == GuidDatabase::gFmpDeviceFspDefaultGuid) {
        return "FSP";
    }
    else if (guid == GuidDatabase::gFmpDevicePlatformRetimerGuid) {
        return "Retimer";
    }
    else if (guid == GuidDatabase::gFmpDeviceMeFwAdlHConsGuid) {
        return "ME H_Cons";
    }
    else if (guid == GuidDatabase::gFmpDeviceMeFwAdlLpCorpGuid) {
        return "ME Lp_Corp";
    }
    else if (guid == GuidDatabase::gFmpDeviceMeFwAdlHCorpGuid) {
        return "ME H_Corp";
    }
    else {
        return "Unknown";
    }
}

IniConfigFile::IniConfigFile(UINT8 *buffer, INT64 length, INT64 offset):
    Volume(buffer, length, offset, false, nullptr) {}

bool IniConfigFile::CheckValidation() {
    return Volume::CheckValidation();
}

INT64 IniConfigFile::SelfDecode() {
    Type = VolumeType::IniConfig;
    iniContext = charToString((CHAR8*)data, size, false);

    std::stringstream ss(iniContext);
    std::string currentSection;

    std::string line;
    while (std::getline(ss, line))
    {
        if (!line.empty() && line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1);
        }

        if (line[0] == '[' && line[line.size() - 1] == ']') {
            currentSection = line.substr(1, line.size() - 2);
            continue;
        }

        if (currentSection.empty())
            continue;

        std::size_t sepPos = line.find('=');
        if (sepPos != std::string::npos) {
            std::string key = TrimString(line.substr(0, sepPos));
            std::string value = TrimString(line.substr(sepPos + 1));
            iniData[currentSection][key] = value;
        }
    }

    string NumOfUpdateStr = GetIniValue("Head", "NumOfUpdate");
    NumOfUpdate = std::stoi(NumOfUpdateStr);

    for (int idx = 0; idx < NumOfUpdate; ++idx) {
        string UpdateIdx = "Update" + std::to_string(idx);
        string SecionName = GetIniValue("Head", UpdateIdx);
        // todo: assert
        UINT32 BgupOffset = std::stoul(GetIniValue(SecionName, "HelperOffset"), nullptr, 16);
        UINT32 BgupSize = std::stoul(GetIniValue(SecionName, "HelperLength"), nullptr, 16);
        BgupList.push_back({SecionName, BgupOffset, BgupSize});
    }

    std::sort(BgupList.begin(), BgupList.end(), [](BgupConfig &config1, BgupConfig &config2) { return config1.BgupOffset < config2.BgupOffset; });

    return size;
}

void IniConfigFile::setInfoStr() {
    InfoStr = QString::fromStdString(iniContext);
}

std::string IniConfigFile::TrimString(const string &inputString) {
    std::size_t start = inputString.find_first_not_of(" \t\r\n");
    std::size_t end = inputString.find_last_not_of(" \t\r\n");

    if (start == std::string::npos || end == std::string::npos)
        return "";

    return inputString.substr(start, end - start + 1);
}

std::string IniConfigFile::GetIniValue(const string &section, const string &key) {
    if (iniData.count(section) && iniData[section].count(key))
        return iniData[section][key];

    return "";
}

IniConfigFile::~IniConfigFile() = default;
