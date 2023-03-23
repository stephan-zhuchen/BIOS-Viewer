#include "iwfi.h"
#include "BaseLib.h"
#include <iomanip>
#include <algorithm>

UINT32 FlashRegionBaseArea::getBase() {
    return (UINT32)base * 0x1000;
}

UINT32 FlashRegionBaseArea::getLimit() {
    return (UINT32)(limit + 1) * 0x1000;
}

UINT32 FlashRegionBaseArea::getSize() {
    return getLimit() - getBase();
}

FlashDescriptorClass::FlashDescriptorClass(UINT8* file, INT64 RegionLength, INT64 FlashLength):Volume(file, RegionLength, 0), FlashTotalSize(FlashLength) {
    UINT32 FRBA_address = this->getUINT8(0x16) * 0x10;
    UINT32 temp;
    for (int index = 0; index < FLASH_REGION_TYPE::FlashRegionAll; ++index) {
        temp = this->getUINT32(FRBA_address);
        FRBA_address += 4;
        FlashRegionBaseArea FlashRegion = *(FlashRegionBaseArea*)(&temp);
        RegionList.push_back(FlashRegion);
    }
}

FlashDescriptorClass::~FlashDescriptorClass() {
    if (data != nullptr)
        delete[] data;
}

void FlashDescriptorClass::setInfoStr() {
    using namespace std;
    INT64 width = 20;
    stringstream ss;
    ss.setf(ios::left);

    ss << setw(width) << "EC   region offset:"  << hex << RegionList.at(FLASH_REGION_TYPE::FlashRegionEC).getBase() << "h\n"
       << setw(width) << "GbE  region offset:"  << hex << RegionList.at(FLASH_REGION_TYPE::FlashRegionGbE).getBase() << "h\n"
       << setw(width) << "ME   region offset:"  << hex << RegionList.at(FLASH_REGION_TYPE::FlashRegionMe).getBase() << "h\n"
       << setw(width) << "BIOS region offset:"  << hex << RegionList.at(FLASH_REGION_TYPE::FlashRegionBios).getBase() << "h\n";

    InfoStr = QString::fromStdString(ss.str());
}

GbE_RegionClass::GbE_RegionClass(UINT8* file, INT64 RegionLength, INT64 offset):Volume(file, RegionLength, offset) {
    MacAddress = *(GBE_MAC_ADDRESS*)data;
    GbeVersion = *(GBE_VERSION*)(data + GBE_VERSION_OFFSET);
}

GbE_RegionClass::~GbE_RegionClass() {
    if (data != nullptr)
        delete[] data;
}

void GbE_RegionClass::setInfoStr() {
    using namespace std;
    INT64 width = 15;
    stringstream ss;
    ss.setf(ios::left);

    ss << setw(width) << "MAC:" << hex << (UINT16)MacAddress.vendor[0] << ":"
                                << (UINT16)MacAddress.vendor[1] << ":"
                                << (UINT16)MacAddress.vendor[2] << ":"
                                << (UINT16)MacAddress.device[0] << ":"
                                << (UINT16)MacAddress.device[1] << ":"
                                << (UINT16)MacAddress.device[2] << "\n"
       << setw(width) << "Image ID:"  << (UINT16)GbeVersion.id << "\n"
       << setw(width) << "Version:" << (UINT16)GbeVersion.major << "." << (UINT16)GbeVersion.minor << endl;

    InfoStr = QString::fromStdString(ss.str());
}

ME_RegionClass::ME_RegionClass(UINT8* file, INT64 RegionLength, INT64 offset):Volume(file, RegionLength, offset) {
    INT64 SearchOffset = 0;
    bool versionFound = false;
    while (SearchOffset < RegionLength - sizeof(INT64)) {
        if (*(UINT32*)(data + SearchOffset) == ME_VERSION_SIGNATURE || *(UINT32*)(data + SearchOffset) == ME_VERSION_SIGNATURE2) {
            versionFound = true;
            break;
        }
        SearchOffset += 4;
    }
    if (versionFound) {
        MeVersion = *(ME_VERSION*)(data + SearchOffset);
        CSE_Layout = new CSE_LayoutClass(file, RegionLength, offset);
    }
}

ME_RegionClass::~ME_RegionClass() {
    if (data != nullptr)
        delete[] data;
    if (CSE_Layout != nullptr)
        delete CSE_Layout;
}

void ME_RegionClass::setInfoStr() {
    using namespace std;
    INT64 width = 15;
    stringstream ss;
    ss.setf(ios::left);
    ss << setw(width) << "Version Signature:" << hex << BaseLibrarySpace::Buffer::charToString((INT8*)&MeVersion.Signature, sizeof(UINT32), false) << "\n"
       << setw(width) << "ME Version:" << dec << MeVersion.Major << "." << MeVersion.Minor << "." << MeVersion.Bugfix << "." << MeVersion.Build << "\n";
    InfoStr = QString::fromStdString(ss.str());
}

CSE_LayoutClass::CSE_LayoutClass(UINT8* file, INT64 RegionLength, INT64 offset):Volume(file, RegionLength, offset) {
    // Data partition always points to FPT header
    ifwiHeader.ifwi16Header = *(IFWI_16_LAYOUT_HEADER*)data;
    if ((ifwiHeader.ifwi16Header.DataPartition.Offset + sizeof(UINT32) < (UINT64)RegionLength) &&
        *(UINT32*)(data + ifwiHeader.ifwi16Header.DataPartition.Offset) == FPT_HEADER_SIGNATURE)
    {
        Ver = IFWI_Ver::IFWI_16;
        CSE_Layout_Valid = true;
    }

    ifwiHeader.ifwi17Header = *(IFWI_17_LAYOUT_HEADER*)data;
    if ((ifwiHeader.ifwi17Header.DataPartition.Offset + sizeof(UINT32) < (UINT64)RegionLength) &&
        *(UINT32*)(data + ifwiHeader.ifwi17Header.DataPartition.Offset) == FPT_HEADER_SIGNATURE)
    {
        Ver = IFWI_Ver::IFWI_17;
        CSE_Layout_Valid = true;

        CSE_PartitionClass *dataPartition = new CSE_PartitionClass(data + ifwiHeader.ifwi17Header.DataPartition.Offset, ifwiHeader.ifwi17Header.DataPartition.Size, offset + ifwiHeader.ifwi17Header.DataPartition.Offset, "ME Data Partition");
        dataPartition->decodeDataPartition();
        CSE_Partitions.push_back(dataPartition);
        for (int i = 0; i < BOOT_PARTITION_NUM; ++i) {
            if (ifwiHeader.ifwi17Header.BootPartition[i].Size == 0) {
                continue;
            }
            std::string PartitionName = "Boot Partition " + std::to_string(i + 1);
            CSE_PartitionClass *BootPartition = new CSE_PartitionClass(data + ifwiHeader.ifwi17Header.BootPartition[i].Offset, ifwiHeader.ifwi17Header.BootPartition[i].Size, offset + ifwiHeader.ifwi17Header.BootPartition[i].Offset, PartitionName);
            BootPartition->decodeBootPartition();
            CSE_Partitions.push_back(BootPartition);
        }
        std::sort(CSE_Partitions.begin(), CSE_Partitions.end(), [](CSE_PartitionClass *p1, CSE_PartitionClass *p2) { return p1->offsetFromBegin < p2->offsetFromBegin; });
    }
}

CSE_LayoutClass::~CSE_LayoutClass() {
    for (CSE_PartitionClass* partition:CSE_Partitions)
        delete partition;
}

bool CSE_LayoutClass::isValid() const {
    return CSE_Layout_Valid;
}

void CSE_LayoutClass::setInfoStr() {
    if (!CSE_Layout_Valid)
        return;
    using namespace std;
    INT64 width = 15;
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

CSE_PartitionClass::CSE_PartitionClass(UINT8* file, INT64 RegionLength, INT64 offset, std::string name):Volume(file, RegionLength, offset), PartitionName(name) {

}

void CSE_PartitionClass::decodeBootPartition() {
    bpdt_Header = *(BPDT_HEADER*)data;
    UINT16 numEntries = bpdt_Header.NumEntries;
    BPDT_ENTRY* firstPtEntry = (BPDT_ENTRY*)(data + sizeof(BPDT_HEADER));
    for (UINT16 i = 0; i < numEntries; i++) {
        BPDT_ENTRY* ptEntry = firstPtEntry + i;
        std::string name = bpdtEntryTypeToString(ptEntry->Type);
        if (ptEntry->Size == 0)
            continue;
        ChildPartitions.push_back(new CSE_PartitionClass(data + ptEntry->Offset, ptEntry->Size, offsetFromBegin + ptEntry->Offset, name));
    }
    std::sort(ChildPartitions.begin(), ChildPartitions.end(), [](CSE_PartitionClass *p1, CSE_PartitionClass *p2) { return p1->offsetFromBegin < p2->offsetFromBegin; });
}

void CSE_PartitionClass::decodeDataPartition() {
    fpt_Header = *(FPT_HEADER*)data;
    UINT32 numEntries = fpt_Header.NumEntries;
    FPT_HEADER_ENTRY* firstPtEntry = (FPT_HEADER_ENTRY*)(data + sizeof(FPT_HEADER));
    for (UINT16 i = 0; i < numEntries; i++) {
        FPT_HEADER_ENTRY* fptEntry = firstPtEntry + i;
        std::string name = UefiSpace::Buffer::charToString(fptEntry->Name, 4, false);
        if (fptEntry->Size == 0)
            continue;
        ChildPartitions.push_back(new CSE_PartitionClass(data + fptEntry->Offset, fptEntry->Size, offsetFromBegin + fptEntry->Offset, name));
    }
    std::sort(ChildPartitions.begin(), ChildPartitions.end(), [](CSE_PartitionClass *p1, CSE_PartitionClass *p2) { return p1->offsetFromBegin < p2->offsetFromBegin; });
}

CSE_PartitionClass::~CSE_PartitionClass() {
    for (CSE_PartitionClass* ChildPartition:ChildPartitions) {
        delete ChildPartition;
    }
}

void CSE_PartitionClass::setInfoStr() {}

std::string CSE_PartitionClass::bpdtEntryTypeToString(const UINT16 type) {
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
    }
    return "Unknown";
}

EC_RegionClass::EC_RegionClass(UINT8* file, INT64 RegionLength, INT64 offset):Volume(file, RegionLength, offset) {
    UINT32 searchValue;

    for (INT64 searchOffset = 0; searchOffset < RegionLength; searchOffset += 2)
    {
        searchValue = this->getUINT32(searchOffset);
        if (searchValue == 0x43534b54) // TKSC
        {
            Signature = searchValue;
            PlatId = this->getUINT8(searchOffset + 4);
            MajorVer = this->getUINT8(searchOffset + 5);
            MinorVer = this->getUINT8(searchOffset + 6);
            BuildVer = this->getUINT8(searchOffset + 7);
            return;
        }
    }
}

EC_RegionClass::~EC_RegionClass() {
    if (data != nullptr)
        delete[] data;
}

void EC_RegionClass::setInfoStr() {
    using namespace std;
    INT64 width = 20;
    stringstream ss;
    ss.setf(ios::left);

    ss << setw(width) << "EC signature:"  << "TKSC" << "\n"
       << setw(width) << "Plat ID:"       << hex << (INT32)PlatId << "h\n"
       << setw(width) << "Build Version:" << hex << (INT32)BuildVer << "h\n"
       << setw(width) << "EC Version:"    << hex << (INT32)MajorVer << "." << hex << (INT32)MinorVer << "\n";

    InfoStr = QString::fromStdString(ss.str());
}
