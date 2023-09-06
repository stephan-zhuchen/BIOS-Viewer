//
// Created by stephan on 8/28/2023.
//

#pragma once
#include "C/Base.h"
#include <QString>
#include <QVector>
#include <QMetaType>
#include <utility>

struct Decompressed {
    std::vector<UINT8> decompressedBuffer;
    UINT32 decompressedOffset;
    UINT32 CompressedSize;
};

enum class VolumeType {
    Overview,
    FirmwareVolume,
    FfsFile,
    CommonSection,
    ELF,
    UplInfo,
    Apriori,
    FspHeader,
    AcpiTable,
    FlashDescriptor,
    EC,
    GbE,
    ME,
    OSSE,
    BIOS,
    BtgAcm,
    IshPdt,
    Microcode,
    NvStorage,
    FaultTolerantBlock,
    CapsuleCommonHeader,
    FirmwareManagementHeader,
    IniConfig,
    BiosGuardPackage,
    Microcodeversion,
    UserDefined,
    Empty,
    Other
};

class Volume {
protected:
    UINT8* data{};
    INT64  size{};
    INT64  offsetFromBegin{0};

    bool   Compressed{false};
    bool   Corrupted{false};

    UINT32              decompressedSize{0};
    UINT8               *DecompressedBufferOnHeap{nullptr};
    VolumeType          Type{VolumeType::Empty};
    VolumeType          SubType{VolumeType::Empty};
    QString             InfoStr;
    QString             UniqueVolumeName;
public:
    Volume              *ParentVolume{nullptr};
    QList<Volume*>      ChildVolume;

    Volume() = default;
    Volume(UINT8* buffer, INT64 length, INT64 offset=0, bool Compressed=false, Volume* parent= nullptr);
    virtual ~Volume();

    // Data Getter
//    [[nodiscard]] EFI_GUID getVolumeGuid() const;
    [[nodiscard]] EFI_GUID getGUID(INT64 offset);
    [[nodiscard]] UINT8  getUINT8(INT64 offset);
    [[nodiscard]] UINT16 getUINT16(INT64 offset);
    [[nodiscard]] UINT32 getUINT32(INT64 offset);
    [[nodiscard]] UINT64 getUINT64(INT64 offset);
    [[nodiscard]] CHAR8  getINT8(INT64 offset);
    [[nodiscard]] INT16  getINT16(INT64 offset);
    [[nodiscard]] INT32  getINT24(INT64 offset);
    [[nodiscard]] INT32  getINT32(INT64 offset);
    [[nodiscard]] INT64  getINT64(INT64 offset);
    [[nodiscard]] UINT8* getBytes(INT64 offset, INT64 length);

    // Attribute Getter
    [[nodiscard]] inline UINT8* getData() const { return data; }
    [[nodiscard]] inline INT64 getSize() const { return size; }
    [[nodiscard]] inline INT64 getOffset() const { return offsetFromBegin; }
    [[nodiscard]] inline bool isCompressed() const { return Compressed; }
    [[nodiscard]] inline bool isCorrupted() const { return Corrupted; }
    [[nodiscard]] inline VolumeType getVolumeType() const { return Type; }
    [[nodiscard]] inline VolumeType getVolumeSubType() const { return SubType; }
    [[nodiscard]] inline QString    getInfoText() const { return InfoStr; }
    [[nodiscard]] inline QString    getUniqueVolumeName() const { return UniqueVolumeName; }
    inline void setVolumeType(VolumeType tp) { Type = tp; }
    inline void setVolumeSubType(VolumeType tp) { SubType = tp; }
    inline void setUniqueVolumeName(QString name) { UniqueVolumeName = std::move(name); }

    // Virtual function
    virtual bool  CheckValidation();
    virtual INT64 SelfDecode();
    virtual void  DecodeChildVolume();
    virtual void  setInfoStr();
    [[nodiscard]] virtual INT64    getHeaderSize() const;
    [[nodiscard]] virtual EFI_GUID getVolumeGuid() const;
    [[nodiscard]] virtual QStringList getUserDefinedName() const;

    void setInfoText(const QString &text);
    bool  GetDecompressedVolume(std::vector<UINT8>& DecompressedVolume);
    void  SearchDecompressedVolume(Volume *volume, std::vector<Decompressed*>& DecompressedVolumeList);
};
Q_DECLARE_METATYPE(Volume);