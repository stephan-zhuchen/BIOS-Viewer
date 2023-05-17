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

void FlashRegionBaseArea::setBase(UINT32 address) {
    base = (UINT16)(address >> 12);
}

void FlashRegionBaseArea::setLimit(UINT32 address) {
    limit = (UINT16)((address >> 12) - 1);
}

IfwiVolume::IfwiVolume(UINT8* file, INT64 RegionLength, INT64 FlashLength, FLASH_REGION_TYPE Type):Volume(file, RegionLength, FlashLength), RegionType(Type) {}

IfwiVolume::~IfwiVolume() {}

bool IfwiVolume::isValid() const {
    return validFlag;
}

std::string IfwiVolume::getFlashmap() { return "";}

FlashDescriptorClass::FlashDescriptorClass(UINT8* file, INT64 RegionLength, INT64 FlashLength):IfwiVolume(file, RegionLength, 0, FLASH_REGION_TYPE::FlashRegionDescriptor), FlashTotalSize(FlashLength) {
    descriptorHeader = *(FLASH_DESCRIPTOR_HEADER*)data;
    if (descriptorHeader.Signature != FLASH_DESCRIPTOR_SIGNATURE) {
        validFlag = false;
        return;
    }
    descriptorMap = *(FLASH_DESCRIPTOR_MAP*)(data + sizeof(FLASH_DESCRIPTOR_HEADER));
    UINT32 FCBA_address = descriptorMap.ComponentBase * 0x10;
    UINT32 FRBA_address = descriptorMap.RegionBase * 0x10;
    FlashComponentSection = *(FLASH_DESCRIPTOR_COMPONENT_SECTION*)(data + FCBA_address);
    FlashRegionSection    = *(FLASH_DESCRIPTOR_REGION_SECTION*)(data + FRBA_address);

    if (FlashComponentSection.FlashParameters.ReadClockFrequency == FLASH_FREQUENCY_20MHZ) {
        descriptorVersion = 1;
    }

    UINT32 temp;
    for (int index = 0; index < FLASH_REGION_TYPE::FlashRegionAll; ++index) {
        temp = this->getUINT32(FRBA_address);
        FRBA_address += 4;
        FlashRegionBaseArea FlashRegion = *(FlashRegionBaseArea*)(&temp);
        RegionList.push_back(FlashRegion);
    }
    UINT8 TW = *(data + 0x23C) >> 4;
    switch (TW) {
    case _128KB:
        topswap_size = "128KB";
        break;
    case _256KB:
        topswap_size = "256KB";
        break;
    case _512KB:
        topswap_size = "512KB";
        break;
    case _1MB:
        topswap_size = "1MB";
        break;
    case _2MB:
        topswap_size = "2MB";
        break;
    case _4MB:
        topswap_size = "4MB";
        break;
    case _8MB:
        topswap_size = "8MB";
        break;
    default:
        topswap_size = "None";
        break;
    }
}

std::string FlashDescriptorClass::getFlashmap() {
    using namespace std;
    INT64 width = 8;
    stringstream ss;
    ss.setf(ios::right);
    ss << setw(width) << setfill('0') << hex << uppercase << offsetFromBegin << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << offsetFromBegin + size - 1 << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << size << "      ";
    ss << "Descriptor Region\n";
    ss << "00000014      00000017      00000004            FLMAP0 - Flash Map 0 Register\n"
          "00000018      0000001B      00000004            FLMAP1 - Flash Map 1 Register\n"
          "0000001C      0000001F      00000004            FLMAP2 - Flash Map 2 Register\n";
    ss << setw(width) << setfill('0') << hex << uppercase << descriptorMap.ComponentBase * 0x10 << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << descriptorMap.ComponentBase * 0x10 + 0x10 - 1 << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << 0x10 << "         ";
    ss << "FCBA - Flash Component Registers\n";
    ss << setw(width) << setfill('0') << hex << uppercase << descriptorMap.RegionBase * 0x10 << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << descriptorMap.RegionBase * 0x10 + sizeof(FLASH_DESCRIPTOR_REGION_SECTION) - 1 << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << sizeof(FLASH_DESCRIPTOR_REGION_SECTION) << "         ";
    ss << "Flash regions registers\n";
    ss << setw(width) << setfill('0') << hex << uppercase << descriptorMap.MasterBase * 0x10 << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << descriptorMap.MasterBase * 0x10 + 0x80 - 1 << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << 0x80 << "         ";
    ss << "Flash Master registers\n";
    ss << "00000320      0000033F      00000020         Flash Descriptor Hash\n"
          "00000340      00000343      00000004         Flash Descriptor Recovery Policy\n"
          "00000F00      00000FFF      00000100         OEM Data\n";
    ss << "00001000      00001FFF      00001000      Descriptor Region Backup 1\n"
          "00002000      00002FFF      00001000      Descriptor Region Backup 2\n"
          "00003000      00003FFF      00001000      Descriptor Region Spare\n";
    return ss.str();
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

    if (topswap_size != "None") {
        ss << setw(width) << "\nTop Swap Block Size: "  << topswap_size << "\n";
    }

    InfoStr = QString::fromStdString(ss.str());
}

GbE_RegionClass::GbE_RegionClass(UINT8* file, INT64 RegionLength, INT64 offset):IfwiVolume(file, RegionLength, offset, FLASH_REGION_TYPE::FlashRegionGbE) {
    MacAddress = *(GBE_MAC_ADDRESS*)data;
    GbeVersion = *(GBE_VERSION*)(data + GBE_VERSION_OFFSET);
}

GbE_RegionClass::~GbE_RegionClass() {
    if (data != nullptr)
        delete[] data;
}

std::string GbE_RegionClass::getFlashmap() {
    using namespace std;
    INT64 width = 8;
    stringstream ss;
    ss.setf(ios::right);
    ss << setw(width) << setfill('0') << hex << uppercase << offsetFromBegin << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << offsetFromBegin + size - 1 << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << size << "      ";
    ss << "GBE Region\n";
    return ss.str();
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

ME_RegionClass::ME_RegionClass(UINT8* file, INT64 RegionLength, INT64 offset):IfwiVolume(file, RegionLength, offset, FLASH_REGION_TYPE::FlashRegionMe) {
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

std::string ME_RegionClass::getFlashmap() {
    using namespace std;
    INT64 width = 8;
    stringstream ss;
    ss.setf(ios::right);
    ss << setw(width) << setfill('0') << hex << uppercase << offsetFromBegin << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << offsetFromBegin + size - 1 << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << size << "      ";
    ss << "CSE Region\n";
    return ss.str();
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

CSE_LayoutClass::CSE_LayoutClass(UINT8* file, INT64 RegionLength, INT64 offset):IfwiVolume(file, RegionLength, offset, FLASH_REGION_TYPE::FlashRegionMe) {
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

        CSE_PartitionClass *dataPartition = new CSE_PartitionClass(data + ifwiHeader.ifwi17Header.DataPartition.Offset,
                                                                   ifwiHeader.ifwi17Header.DataPartition.Size,
                                                                   offset + ifwiHeader.ifwi17Header.DataPartition.Offset,
                                                                   "ME Data Partition",
                                                                   CSE_PartitionClass::Level1);
        dataPartition->decodeDataPartition();
        CSE_Partitions.push_back(dataPartition);
        for (int i = 0; i < BOOT_PARTITION_NUM; ++i) {
            if (ifwiHeader.ifwi17Header.BootPartition[i].Size == 0) {
                continue;
            }
            std::string PartitionName = "Boot Partition " + std::to_string(i + 1);
            CSE_PartitionClass *BootPartition = new CSE_PartitionClass(data + ifwiHeader.ifwi17Header.BootPartition[i].Offset,
                                                                       ifwiHeader.ifwi17Header.BootPartition[i].Size,
                                                                       offset + ifwiHeader.ifwi17Header.BootPartition[i].Offset,
                                                                       PartitionName,
                                                                       CSE_PartitionClass::Level1);
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

std::string CSE_LayoutClass::getFlashmap() {
    using namespace std;
    INT64 width = 8;
    stringstream ss;
    ss.setf(ios::right);
    ss << setw(width) << setfill('0') << hex << uppercase << offsetFromBegin << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << offsetFromBegin + size - 1 << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << size << "         ";
    ss << "CSE Layout Table\n";
    return ss.str();
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

CSE_PartitionClass::CSE_PartitionClass(UINT8* file, INT64 RegionLength, INT64 offset, std::string name, PartitionLevel lv):IfwiVolume(file, RegionLength, offset, FLASH_REGION_TYPE::FlashRegionMe), PartitionName(name) {
    level = lv;
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
        ChildPartitions.push_back(new CSE_PartitionClass(data + ptEntry->Offset, ptEntry->Size, offsetFromBegin + ptEntry->Offset, name, CSE_PartitionClass::Level2));
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
        ChildPartitions.push_back(new CSE_PartitionClass(data + fptEntry->Offset, fptEntry->Size, offsetFromBegin + fptEntry->Offset, name, CSE_PartitionClass::Level2));
    }
    std::sort(ChildPartitions.begin(), ChildPartitions.end(), [](CSE_PartitionClass *p1, CSE_PartitionClass *p2) { return p1->offsetFromBegin < p2->offsetFromBegin; });
}

CSE_PartitionClass::~CSE_PartitionClass() {
    for (CSE_PartitionClass* ChildPartition:ChildPartitions) {
        delete ChildPartition;
    }
}

std::string CSE_PartitionClass::getFlashmap() {
    using namespace std;
    INT64 width = 8;
    stringstream ss;
    ss.setf(ios::right);
    ss << setw(width) << setfill('0') << hex << uppercase << offsetFromBegin << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << offsetFromBegin + size - 1 << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << size << "         ";
    for (int i = 0; i < level; ++i) {
        ss << "   ";
    }
    ss << PartitionName << "\n";
    return ss.str();
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
        case BPDT_ENTRY_TYPE_SPHY:        return "SOC SPHY Partition";
    }
    return "Unknown";
}

EC_RegionClass::EC_RegionClass(UINT8* file, INT64 RegionLength, INT64 offset):IfwiVolume(file, RegionLength, offset, FLASH_REGION_TYPE::FlashRegionEC) {
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

std::string EC_RegionClass::getFlashmap() {
    using namespace std;
    INT64 width = 8;
    stringstream ss;
    ss.setf(ios::right);
    ss << setw(width) << setfill('0') << hex << uppercase << offsetFromBegin << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << offsetFromBegin + size - 1 << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << size << "      ";
    ss << "EC Region\n";
    return ss.str();
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

OSSE_RegionClass::OSSE_RegionClass(UINT8* file, INT64 RegionLength, INT64 offset):IfwiVolume(file, RegionLength, offset, FLASH_REGION_TYPE::FlashRegionIE) {
}

OSSE_RegionClass::~OSSE_RegionClass() {
    if (data != nullptr)
        delete[] data;
}

std::string OSSE_RegionClass::getFlashmap() {
    using namespace std;
    INT64 width = 8;
    stringstream ss;
    ss.setf(ios::right);
    ss << setw(width) << setfill('0') << hex << uppercase << offsetFromBegin << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << offsetFromBegin + size - 1 << "      ";
    ss << setw(width) << setfill('0') << hex << uppercase << size << "      ";
    ss << "OSSE Region\n";
    return ss.str();
}

void OSSE_RegionClass::setInfoStr() {
    using namespace std;
//    INT64 width = 20;
    stringstream ss;
    ss.setf(ios::left);

//    ss << setw(width) << "EC signature:"  << "TKSC" << "\n"
//       << setw(width) << "Plat ID:"       << hex << (INT32)PlatId << "h\n"
//       << setw(width) << "Build Version:" << hex << (INT32)BuildVer << "h\n"
//       << setw(width) << "EC Version:"    << hex << (INT32)MajorVer << "." << hex << (INT32)MinorVer << "\n";

    InfoStr = QString::fromStdString(ss.str());
}
