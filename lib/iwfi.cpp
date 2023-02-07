#include "iwfi.h"
#include <iomanip>

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

}

GbE_RegionClass::~GbE_RegionClass() {
    if (data != nullptr)
        delete[] data;
}

void GbE_RegionClass::setInfoStr() {
    using namespace std;
    INT64 width = 20;
    stringstream ss;
    ss.setf(ios::left);

    ss << setw(width) << "MAC:"  << "\n"
       << setw(width) << "Version:" << endl;

    InfoStr = QString::fromStdString(ss.str());
}

ME_RegionClass::ME_RegionClass(UINT8* file, INT64 RegionLength, INT64 offset):Volume(file, RegionLength, offset) {

}

ME_RegionClass::~ME_RegionClass() {
    if (data != nullptr)
        delete[] data;
}

void ME_RegionClass::setInfoStr() {

}

EC_RegionClass::EC_RegionClass(UINT8* file, INT64 RegionLength, INT64 offset):Volume(file, RegionLength, offset) {
    UINT32 searchValue;

    for (INT64 searchOffset = 0; searchOffset < RegionLength; searchOffset += 2)
    {
        searchValue = this->getUINT32(searchOffset);
        if (searchValue == 0x43534b54) // TKSC
        {
            std::cout << "EC signature found!" << std::endl;
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
