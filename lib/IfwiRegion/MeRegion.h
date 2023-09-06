//
// Created by stephan on 9/2/2023.
//

#pragma once
#include "Volume.h"
#include "UEFI/ME.h"

enum class IFWI_Ver {IFWI_16=0, IFWI_17};
enum PartitionLevel {Level1=0, Level2, Level3};
class CSE_PartitionClass;

class MeRegion: public Volume {
private:
    ME_VERSION     MeVersion{};
    bool           VersionFound{false};
//    CSE_LayoutClass   *CSE_Layout{nullptr};
public:
    MeRegion() = delete;
    MeRegion(UINT8* buffer, INT64 length, INT64 offset=0);
    ~MeRegion() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void  setInfoStr() override;
};

class CSE_LayoutClass : public Volume {
private:
    bool CSE_Layout_Valid{false};
    IFWI_Ver Ver;
    union IFWI_LAYOUT_HEADER {
        IFWI_16_LAYOUT_HEADER   ifwi16Header;
        IFWI_17_LAYOUT_HEADER   ifwi17Header;
    } ifwiHeader{};
public:
    QVector<CSE_PartitionClass*>  CSE_Partitions;
    CSE_LayoutClass(UINT8* file, INT64 RegionLength, INT64 offset, Volume *parent);
    ~CSE_LayoutClass() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void setInfoStr() override;
    [[nodiscard]] QStringList getUserDefinedName() const override;
};

class CSE_PartitionClass : public Volume {
private:
    BPDT_HEADER bpdt_Header{};
    FPT_HEADER  fpt_Header{};
    QString     PartitionName;
    PartitionLevel level;
public:
    CSE_PartitionClass(UINT8* file, INT64 RegionLength, INT64 offset, Volume *parent, QString name, PartitionLevel lv);
    ~CSE_PartitionClass() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void setInfoStr() override;
    [[nodiscard]] QStringList getUserDefinedName() const override;

    void decodeBootPartition();
    void decodeDataPartition();
    static QString bpdtEntryTypeToString(UINT16 type);
};