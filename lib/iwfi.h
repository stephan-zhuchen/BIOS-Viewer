#pragma once
#include <vector>
#include "UefiLib.h"
#include "../include/GbE.h"
#include "../include/ME.h"
#include "../include/descriptor.h"
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

class CSE_LayoutClass;
class CSE_PartitionClass;

struct FlashRegionBaseArea {
    UINT16 base;
    UINT16 limit;
    UINT32 getBase();
    UINT32 getLimit();
    UINT32 getSize();
};

class IfwiVolume : public Volume {
protected:
    bool validFlag{true};
public:
    bool isValid() const;
    IfwiVolume(UINT8* file, INT64 RegionLength, INT64 FlashLength);
    ~IfwiVolume();
    virtual std::string getFlashmap();
};

class FlashDescriptorClass : public IfwiVolume {
public:
    FLASH_DESCRIPTOR_HEADER descriptorHeader;
    FLASH_DESCRIPTOR_MAP    descriptorMap;
    FLASH_DESCRIPTOR_COMPONENT_SECTION FlashComponentSection;
    FLASH_DESCRIPTOR_REGION_SECTION    FlashRegionSection;
    std::vector<FlashRegionBaseArea> RegionList;
    INT64 FlashTotalSize;
    UINT8 descriptorVersion{2};
    enum TopSwap{_64KB=0, _128KB, _256KB, _512KB, _1MB, _2MB, _4MB, _8MB};
    std::string topswap_size;
public:
    FlashDescriptorClass() = delete;
    FlashDescriptorClass(UINT8* file, INT64 RegionLength, INT64 FlashLength);
    ~FlashDescriptorClass();
    std::string getFlashmap() override;
    void setInfoStr() override;
};

class GbE_RegionClass : public IfwiVolume {
private:
    GBE_MAC_ADDRESS MacAddress;
    GBE_VERSION     GbeVersion;
public:
    GbE_RegionClass(UINT8* file, INT64 RegionLength, INT64 offset);
    ~GbE_RegionClass();
    std::string getFlashmap() override;
    void setInfoStr() override;
};

class ME_RegionClass : public IfwiVolume {
private:
    ME_VERSION        MeVersion;
public:
    CSE_LayoutClass   *CSE_Layout{nullptr};
    ME_RegionClass(UINT8* file, INT64 RegionLength, INT64 offset);
    ~ME_RegionClass();
    std::string getFlashmap() override;
    void setInfoStr() override;
};

class CSE_LayoutClass : public IfwiVolume {
private:
    bool CSE_Layout_Valid{false};
    enum class IFWI_Ver {IFWI_16=0, IFWI_17} Ver;
    union IFWI_LAYOUT_HEADER {
        IFWI_16_LAYOUT_HEADER   ifwi16Header;
        IFWI_17_LAYOUT_HEADER   ifwi17Header;
    } ifwiHeader;
public:
    std::vector<CSE_PartitionClass*>  CSE_Partitions;
    CSE_LayoutClass(UINT8* file, INT64 RegionLength, INT64 offset);
    ~CSE_LayoutClass();
    bool isValid() const;
    std::string getFlashmap() override;
    void setInfoStr() override;
};

class CSE_PartitionClass : public IfwiVolume {
private:
    BPDT_HEADER bpdt_Header;
    FPT_HEADER  fpt_Header;
public:
    std::string PartitionName;
    enum PartitionLevel {Level1=0, Level2, Level3} level;
    std::vector<CSE_PartitionClass*> ChildPartitions;
    CSE_PartitionClass(UINT8* file, INT64 RegionLength, INT64 offset, std::string name, PartitionLevel lv);
    ~CSE_PartitionClass();
    void decodeBootPartition();
    void decodeDataPartition();
    static std::string bpdtEntryTypeToString(const UINT16 type);
    std::string getFlashmap() override;
    void setInfoStr() override;
};

class EC_RegionClass : public IfwiVolume {
private:
    std::string Signature;
    UINT8   PlatId{};
    UINT8   MajorVer{};
    UINT8   MinorVer{};
    UINT8   BuildVer{};
public:
    EC_RegionClass(UINT8* file, INT64 RegionLength, INT64 offset);
    ~EC_RegionClass();
    std::string getFlashmap() override;
    void setInfoStr() override;
};

class OSSE_RegionClass : public IfwiVolume {
private:
public:
    OSSE_RegionClass(UINT8* file, INT64 RegionLength, INT64 offset);
    ~OSSE_RegionClass();
    std::string getFlashmap() override;
    void setInfoStr() override;
};
