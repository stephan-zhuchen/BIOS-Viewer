//
// Created by stephan on 9/2/2023.
//

#include "FlashDescriptorRegion.h"

UINT32 FlashRegionBaseArea::getBase() const {
    return (UINT32)base * 0x1000;
}

UINT32 FlashRegionBaseArea::getLimit() const {
    return (UINT32)(limit + 1) * 0x1000;
}

UINT32 FlashRegionBaseArea::getSize() const {
    return getLimit() - getBase();
}

void FlashRegionBaseArea::setBase(UINT32 address) {
    base = (UINT16)(address >> 12);
}

void FlashRegionBaseArea::setLimit(UINT32 address) {
    limit = (UINT16)((address >> 12) - 1);
}

FlashDescriptorRegion::FlashDescriptorRegion(UINT8 *buffer, INT64 length, INT64 offset):
    Volume(buffer, length, offset, false, nullptr) {}

bool FlashDescriptorRegion::CheckValidation() {
    descriptorHeader = *(FLASH_DESCRIPTOR_HEADER*)data;
    if (descriptorHeader.Signature != FLASH_DESCRIPTOR_SIGNATURE) {
        return false;
    }
    return true;
}

INT64 FlashDescriptorRegion::SelfDecode() {
    Type = VolumeType::FlashDescriptor;
    RegionType = FLASH_REGION_TYPE::FlashRegionDescriptor;
    if (!CheckValidation()) {
        return 0;
    }
    size = 0x1000;
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
            topSwapSize = "128KB";
            break;
        case _256KB:
            topSwapSize = "256KB";
            break;
        case _512KB:
            topSwapSize = "512KB";
            break;
        case _1MB:
            topSwapSize = "1MB";
            break;
        case _2MB:
            topSwapSize = "2MB";
            break;
        case _4MB:
            topSwapSize = "4MB";
            break;
        case _8MB:
            topSwapSize = "8MB";
            break;
        default:
            topSwapSize = "None";
            break;
    }
    return size;
}

void FlashDescriptorRegion::DecodeChildVolume() {
    Volume::DecodeChildVolume();
}

void FlashDescriptorRegion::setInfoStr() {
    using namespace std;
    INT32 width = 20;
    stringstream ss;
    ss.setf(ios::left);

    ss << setw(width) << "EC   region offset:"  << hex << RegionList.at(FLASH_REGION_TYPE::FlashRegionEC).getBase() << "h\n"
       << setw(width) << "GbE  region offset:"  << hex << RegionList.at(FLASH_REGION_TYPE::FlashRegionGbE).getBase() << "h\n"
       << setw(width) << "ME   region offset:"  << hex << RegionList.at(FLASH_REGION_TYPE::FlashRegionMe).getBase() << "h\n"
       << setw(width) << "BIOS region offset:"  << hex << RegionList.at(FLASH_REGION_TYPE::FlashRegionBios).getBase() << "h\n";

    if (topSwapSize != "None") {
        ss << setw(width) << "\nTop Swap Block Size: " << topSwapSize << "\n";
    }

    InfoStr = QString::fromStdString(ss.str());
}

FlashDescriptorRegion::~FlashDescriptorRegion() = default;
