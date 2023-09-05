//
// Created by stephan on 9/2/2023.
//
#pragma once
#include "Volume.h"
#include "UEFI/descriptor.h"

enum TopSwap {
    _64KB=0,
    _128KB,
    _256KB,
    _512KB,
    _1MB,
    _2MB,
    _4MB,
    _8MB
};

struct FlashRegionBaseArea {
    UINT16 base;
    UINT16 limit;
    [[nodiscard]] UINT32 getBase() const;
    [[nodiscard]] UINT32 getLimit() const;
    [[nodiscard]] UINT32 getSize() const;
    void setBase(UINT32 address);
    void setLimit(UINT32 address);
};

class FlashDescriptorRegion: public Volume {
private:
    FLASH_REGION_TYPE                   RegionType;
    FLASH_DESCRIPTOR_HEADER             descriptorHeader{};
    FLASH_DESCRIPTOR_MAP                descriptorMap{};
    FLASH_DESCRIPTOR_COMPONENT_SECTION  FlashComponentSection{};
    FLASH_DESCRIPTOR_REGION_SECTION     FlashRegionSection{};
    INT64                               FlashTotalSize;
    UINT8                               descriptorVersion{2};
    std::string                         topSwapSize;
public:
    QVector<FlashRegionBaseArea>        RegionList;

    FlashDescriptorRegion() = delete;
    FlashDescriptorRegion(UINT8* buffer, INT64 length, INT64 offset=0);
    ~FlashDescriptorRegion() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void  setInfoStr() override;
};

