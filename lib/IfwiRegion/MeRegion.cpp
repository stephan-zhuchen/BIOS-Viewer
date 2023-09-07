//
// Created by stephan on 9/2/2023.
//
#include "BaseLib.h"
#include "MeRegion.h"
#include <QDebug>
#include <utility>

using namespace BaseLibrarySpace;

MeRegion::MeRegion(UINT8 *buffer, INT64 length, INT64 offset):
    Volume(buffer, length, offset, false, nullptr) {}

bool MeRegion::CheckValidation() {
    return Volume::CheckValidation();
}

INT64 MeRegion::SelfDecode() {
    Type = VolumeType::ME;
    INT64 SearchOffset = 0;
    while (SearchOffset < size - (INT64)sizeof(INT64)) {
        if (*(UINT32*)(data + SearchOffset) == ME_VERSION_SIGNATURE || *(UINT32*)(data + SearchOffset) == ME_VERSION_SIGNATURE2) {
            VersionFound = true;
            break;
        }
        SearchOffset += 4;
    }
    if (VersionFound) {
        MeVersion = *(ME_VERSION*)(data + SearchOffset);
    }
    return size;
}

void MeRegion::DecodeChildVolume() {
    if (VersionFound) {
        auto *CSE_Layout = new CSE_LayoutClass(data, size, offsetFromBegin, this);
        CSE_Layout->SelfDecode();
        CSE_Layout->DecodeChildVolume();
        ChildVolume.push_back(CSE_Layout);

        for (CSE_PartitionClass* Partition : CSE_Layout->CSE_Partitions) {
            Partition->SelfDecode();
            Partition->DecodeChildVolume();
            Partition->ParentVolume = this;
            ChildVolume.push_back(Partition);
        }
    }
}

void MeRegion::setInfoStr() {
    using namespace std;
    INT32 width = 20;
    stringstream ss;
    ss.setf(ios::left);
    ss << setw(width) << "Version Signature:" << hex << BaseLibrarySpace::charToString((CHAR8*)&MeVersion.Signature, sizeof(UINT32), false) << "\n"
       << setw(width) << "ME Version:" << dec << MeVersion.Major << "." << MeVersion.Minor << "." << MeVersion.Bugfix << "." << MeVersion.Build << "\n";
    InfoStr = QString::fromStdString(ss.str());
}

MeRegion::~MeRegion() = default;

CSE_LayoutClass::CSE_LayoutClass(UINT8 *file, INT64 RegionLength, INT64 offset, Volume *parent):
    Volume(file, RegionLength, offset, false, parent) {}

bool CSE_LayoutClass::CheckValidation() {
    return Volume::CheckValidation();
}

INT64 CSE_LayoutClass::SelfDecode() {
    Type = VolumeType::UserDefined;
    // Data partition always points to FPT header
    ifwiHeader.ifwi16Header = *(IFWI_16_LAYOUT_HEADER*)data;
    if ((ifwiHeader.ifwi16Header.DataPartition.Offset + sizeof(UINT32) < (UINT64)size) &&
        *(UINT32*)(data + ifwiHeader.ifwi16Header.DataPartition.Offset) == FPT_HEADER_SIGNATURE)
    {
        Ver = IFWI_Ver::IFWI_16;
        CSE_Layout_Valid = true;
    }

    ifwiHeader.ifwi17Header = *(IFWI_17_LAYOUT_HEADER*)data;
    if ((ifwiHeader.ifwi17Header.DataPartition.Offset + sizeof(UINT32) < (UINT64)size) &&
        *(UINT32*)(data + ifwiHeader.ifwi17Header.DataPartition.Offset) == FPT_HEADER_SIGNATURE)
    {
        Ver = IFWI_Ver::IFWI_17;
        CSE_Layout_Valid = true;
    }
    return size;
}

void CSE_LayoutClass::DecodeChildVolume() {
    if (Ver == IFWI_Ver::IFWI_17) {
        auto *dataPartition = new CSE_PartitionClass(data + ifwiHeader.ifwi17Header.DataPartition.Offset,
                                                     ifwiHeader.ifwi17Header.DataPartition.Size,
                                                     offsetFromBegin + ifwiHeader.ifwi17Header.DataPartition.Offset,
                                                     nullptr,
                                                     "ME Data Partition",
                                                     PartitionLevel::Level1);
        dataPartition->decodeDataPartition();
        CSE_Partitions.push_back(dataPartition);

        for (int i = 0; i < BOOT_PARTITION_NUM; ++i) {
            if (ifwiHeader.ifwi17Header.BootPartition[i].Size == 0) {
                continue;
            }
            QString PartitionName = "Boot Partition " + QString::number(i + 1);
            auto *BootPartition = new CSE_PartitionClass(data + ifwiHeader.ifwi17Header.BootPartition[i].Offset,
                                                         ifwiHeader.ifwi17Header.BootPartition[i].Size,
                                                         offsetFromBegin + ifwiHeader.ifwi17Header.BootPartition[i].Offset,
                                                         nullptr,
                                                         PartitionName,
                                                         PartitionLevel::Level1);
            BootPartition->decodeBootPartition();
            CSE_Partitions.push_back(BootPartition);
        }
        std::sort(CSE_Partitions.begin(), CSE_Partitions.end(), [](CSE_PartitionClass *p1, CSE_PartitionClass *p2) { return p1->getOffset() < p2->getOffset(); });
    }
}

void CSE_LayoutClass::setInfoStr() {
    if (!CSE_Layout_Valid)
        return;
    using namespace std;
    INT32 width = 15;
    stringstream ss;
    ss.setf(ios::left);

    switch (Ver) {
        case IFWI_Ver::IFWI_16:
            ss << setw(width) << "Checksum:"    << hex << uppercase << (UINT16)ifwiHeader.ifwi16Header.Checksum << "h\n";
            width = 25;
            ss << setw(width) << "Data partition offset:" << hex << uppercase << ifwiHeader.ifwi16Header.DataPartition.Offset << "h\n"
               << setw(width) << "Data partition size:"   << hex << uppercase << ifwiHeader.ifwi16Header.DataPartition.Size << "h\n";
            for (int i = 0; i < BOOT_PARTITION_NUM; ++i) {
                ss << setw(width) << "Data partition offset:" << hex << uppercase << ifwiHeader.ifwi16Header.BootPartition[i].Offset << "h\n"
                   << setw(width) << "Data partition size:"   << hex << uppercase << ifwiHeader.ifwi16Header.BootPartition[i].Size << "h\n";
            }
            break;
        case IFWI_Ver::IFWI_17:
            ss << setw(width) << "Header size:" << hex << uppercase << ifwiHeader.ifwi17Header.HeaderSize << "h\n"
               << setw(width) << "Flags:"       << hex << uppercase << (UINT16)ifwiHeader.ifwi17Header.Flags << "h\n"
               << setw(width) << "Checksum:"    << hex << uppercase << (UINT16)ifwiHeader.ifwi17Header.Checksum << "h\n";
            width = 25;
            ss << setw(width) << "Data partition offset:" << hex << uppercase << ifwiHeader.ifwi17Header.DataPartition.Offset << "h\n"
               << setw(width) << "Data partition size:"   << hex << uppercase << ifwiHeader.ifwi17Header.DataPartition.Size << "h\n";
            for (int i = 0; i < BOOT_PARTITION_NUM; ++i) {
                ss << setw(width) << "Data partition offset:" << hex << uppercase << ifwiHeader.ifwi17Header.BootPartition[i].Offset << "h\n"
                   << setw(width) << "Data partition size:"   << hex << uppercase << ifwiHeader.ifwi17Header.BootPartition[i].Size << "h\n";
            }
            break;
        default:
            break;
    }
    InfoStr = QString::fromStdString(ss.str());
}

QStringList CSE_LayoutClass::getUserDefinedName() const {
    QStringList UserDefinedName;
    UserDefinedName << "CSE Layout Table" << "Layout";
    return UserDefinedName;
}

CSE_LayoutClass::~CSE_LayoutClass() = default;

CSE_PartitionClass::CSE_PartitionClass(UINT8 *file, INT64 RegionLength, INT64 offset, Volume *parent, QString name, PartitionLevel lv):
    Volume(file, RegionLength, offset, false, parent), PartitionName(std::move(name)), level(lv) {}

bool CSE_PartitionClass::CheckValidation() {
    return Volume::CheckValidation();
}

INT64 CSE_PartitionClass::SelfDecode() {
    Type = VolumeType::UserDefined;
    return Volume::SelfDecode();
}

void CSE_PartitionClass::DecodeChildVolume() {
    Volume::DecodeChildVolume();
}

void CSE_PartitionClass::setInfoStr() {
    Volume::setInfoStr();
}

void CSE_PartitionClass::decodeBootPartition() {
    bpdt_Header = *(BPDT_HEADER*)data;
    UINT16 numEntries = bpdt_Header.NumEntries;
    auto* firstPtEntry = (BPDT_ENTRY*)(data + sizeof(BPDT_HEADER));
    for (UINT16 i = 0; i < numEntries; i++) {
        BPDT_ENTRY* ptEntry = firstPtEntry + i;
        QString name = bpdtEntryTypeToString(ptEntry->Type);
        if (ptEntry->Size == 0)
            continue;
        auto ChildPartition = new CSE_PartitionClass(data + ptEntry->Offset, ptEntry->Size, offsetFromBegin + ptEntry->Offset, this, name, PartitionLevel::Level2);
        ChildPartition->SelfDecode();
        ChildVolume.push_back(ChildPartition);
    }
    std::sort(ChildVolume.begin(), ChildVolume.end(), [](Volume *p1, Volume *p2) { return p1->getOffset() < p2->getOffset(); });
}

void CSE_PartitionClass::decodeDataPartition() {
    fpt_Header = *(FPT_HEADER*)data;
    UINT32 numEntries = fpt_Header.NumEntries;
    auto* firstPtEntry = (FPT_HEADER_ENTRY*)(data + sizeof(FPT_HEADER));
    for (UINT16 i = 0; i < numEntries; i++) {
        FPT_HEADER_ENTRY* fptEntry = firstPtEntry + i;
        QString name = QString::fromStdString(charToString(fptEntry->Name, 4, false));
        if (fptEntry->Size == 0)
            continue;
        auto ChildPartition = new CSE_PartitionClass(data + fptEntry->Offset, fptEntry->Size, offsetFromBegin + fptEntry->Offset, this, name, PartitionLevel::Level2);
        ChildPartition->SelfDecode();
        ChildVolume.push_back(ChildPartition);
    }
    std::sort(ChildVolume.begin(), ChildVolume.end(), [](Volume *p1, Volume *p2) { return p1->getOffset() < p2->getOffset(); });
}

QString CSE_PartitionClass::bpdtEntryTypeToString(UINT16 type) {
    switch (type) {
        case BPDT_ENTRY_TYPE_SMIP:        return "OEM SMIP";
        case BPDT_ENTRY_TYPE_RBEP:        return "CSE RBE Partition";
        case BPDT_ENTRY_TYPE_FTPR:        return "Bring Up";
        case BPDT_ENTRY_TYPE_UCOD:        return "Microcode";
        case BPDT_ENTRY_TYPE_IBBP:        return "IBB";
        case BPDT_ENTRY_TYPE_S_BPDT:      return "Secondary BPDT";
        case BPDT_ENTRY_TYPE_OBBP:        return "OBB";
        case BPDT_ENTRY_TYPE_NFTP:        return "CSE Main Partition";
        case BPDT_ENTRY_TYPE_ISHC:        return "ISH Partition";
        case BPDT_ENTRY_TYPE_DLMP:        return "Debug Launch Module";
        case BPDT_ENTRY_TYPE_UEBP:        return "IFP Bypass";
        case BPDT_ENTRY_TYPE_UTOK:        return "Debug Tokens";
        case BPDT_ENTRY_TYPE_UFS_PHY:     return "UFS PHY Config";
        case BPDT_ENTRY_TYPE_UFS_GPP_LUN: return "UFS GPP LUN";
        case BPDT_ENTRY_TYPE_PMCP:        return "PMC Partition";
        case BPDT_ENTRY_TYPE_IUNP:        return "IUnit Partition";
        case BPDT_ENTRY_TYPE_NVMC:        return "NVM Config";
        case BPDT_ENTRY_TYPE_UEP:         return "Unified Emulation";
        case BPDT_ENTRY_TYPE_WCOD:        return "CSE WCOD Partition";
        case BPDT_ENTRY_TYPE_LOCL:        return "CSE LOCL Partition";
        case BPDT_ENTRY_TYPE_OEMP:        return "OEM KM Partition";
        case BPDT_ENTRY_TYPE_FITC:        return "fitc.cfg";
        case BPDT_ENTRY_TYPE_PAVP:        return "PAVP";
        case BPDT_ENTRY_TYPE_IOMP:        return "IOM Partition";
        case BPDT_ENTRY_TYPE_XPHY:        return "NPHY Partition";
        case BPDT_ENTRY_TYPE_TBTP:        return "TBT Partition";
        case BPDT_ENTRY_TYPE_PLTS:        return "Platform Settings";
        case BPDT_ENTRY_TYPE_RES27:       return "Reserved 27";
        case BPDT_ENTRY_TYPE_RES28:       return "Reserved 28";
        case BPDT_ENTRY_TYPE_RES29:       return "Reserved 29";
        case BPDT_ENTRY_TYPE_RES30:       return "Reserved 30";
        case BPDT_ENTRY_TYPE_DPHY:        return "SPHY Partition";
        case BPDT_ENTRY_TYPE_PCHC:        return "SOCC Partition";
        case BPDT_ENTRY_TYPE_ISIF:        return "ISI FW";
        case BPDT_ENTRY_TYPE_ISIC:        return "ISI Config";
        case BPDT_ENTRY_TYPE_HBMI:        return "HBM IO";
        case BPDT_ENTRY_TYPE_OMSM:        return "OOB MSM";
        case BPDT_ENTRY_TYPE_GTGP:        return "GT-GPU";
        case BPDT_ENTRY_TYPE_MDFI:        return "MDF IO";
        case BPDT_ENTRY_TYPE_PUNP:        return "PUnit";
        case BPDT_ENTRY_TYPE_PHYP:        return "GSC PHY";
        case BPDT_ENTRY_TYPE_SAMF:        return "SAM FW";
        case BPDT_ENTRY_TYPE_PPHY:        return "PPHY";
        case BPDT_ENTRY_TYPE_GBST:        return "GBST";
        case BPDT_ENTRY_TYPE_TCCP:        return "TCC";
        case BPDT_ENTRY_TYPE_PSEP:        return "PSE";
        case BPDT_ENTRY_TYPE_ESE:         return "ESE Package";
        case BPDT_ENTRY_TYPE_ACE:         return "ACE Package";
        case BPDT_ENTRY_TYPE_SPHY:        return "SOC SPHY Partition";
    }
    return "Unknown";
}

QStringList CSE_PartitionClass::getUserDefinedName() const {
    QStringList UserDefinedName;
    UserDefinedName << PartitionName << "Partition";
    return UserDefinedName;
}

CSE_PartitionClass::~CSE_PartitionClass() = default;
