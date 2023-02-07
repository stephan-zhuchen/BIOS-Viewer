#pragma once
#include <vector>
#include "UefiLib.h"
#include "../include/SymbolDefinition.h"

using UefiSpace::Volume;

/**
  Flash Region Type
**/
enum FLASH_REGION_TYPE{
  FlashRegionDescriptor,
  FlashRegionBios,
  FlashRegionMe,
  FlashRegionGbE,
  FlashRegionPlatformData,
  FlashRegionDer,
  FlashRegionSecondaryBios,
  FlashRegionuCodePatch,
  FlashRegionEC,
  FlashRegionDeviceExpansion2,
  FlashRegionIE,
  FlashRegion10Gbe_A,
  FlashRegion10Gbe_B,
  FlashRegion13,
  FlashRegion14,
  FlashRegion15,
  FlashRegionAll,
  FlashRegionMax
};

struct FlashRegionBaseArea {
    UINT16 base;
    UINT16 limit;
    UINT32 getBase();
    UINT32 getLimit();
    UINT32 getSize();
};

class FlashDescriptorClass : public Volume {
public:
    std::vector<FlashRegionBaseArea> RegionList;
    INT64 FlashTotalSize;
public:
    FlashDescriptorClass() = delete;
    FlashDescriptorClass(UINT8* file, INT64 RegionLength, INT64 FlashLength);
    ~FlashDescriptorClass();
    void setInfoStr() override;
};

class GbE_RegionClass : public Volume {
public:
    GbE_RegionClass(UINT8* file, INT64 RegionLength, INT64 offset);
    ~GbE_RegionClass();
    void setInfoStr() override;
};

class ME_RegionClass : public Volume {
public:
    ME_RegionClass(UINT8* file, INT64 RegionLength, INT64 offset);
    ~ME_RegionClass();
    void setInfoStr() override;
};

class EC_RegionClass : public Volume {
private:
    std::string Signature;
    UINT8   PlatId{};
    UINT8   MajorVer{};
    UINT8   MinorVer{};
    UINT8   BuildVer{};
public:
    EC_RegionClass(UINT8* file, INT64 RegionLength, INT64 offset);
    ~EC_RegionClass();
    void setInfoStr() override;
};
