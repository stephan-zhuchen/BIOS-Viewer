//
// Created by stephan on 8/29/2023.
//

#pragma once
#include "Volume.h"
#include "string"
#include "UEFI/PiFirmwareFile.h"
#include "UEFI/PeImage.h"

using std::string;
class FspHeader;
class AcpiClass;
class ELF;

class PeCoff {
private:
    UINT8* data{};
    INT64  size{};
public:
    EFI_IMAGE_DOS_HEADER      dosHeader{};
    EFI_TE_IMAGE_HEADER       teHeader{};
    EFI_IMAGE_NT_HEADERS32    pe32Header{};
    EFI_IMAGE_NT_HEADERS64    pe32plusHeader{};
    bool                      isTE{false};
    bool                      isPe32Plus{false};

    PeCoff()=delete;
    PeCoff(UINT8* file, INT64 length);

    [[nodiscard]] string getMachineType() const;
    static string getSubsystemName(UINT16 subsystem);
};

class Depex {
private:
    UINT8* data{};
    INT64  size{};
public:
    QVector<string>            OrganizedDepexList;
    Depex()=delete;
    Depex(UINT8* file, INT64 length);
    static string getOpcodeString(UINT8 op);
};

class CommonSection: public Volume {
private:
    EFI_COMMON_SECTION_HEADER CommonHeader;
    //EFI_COMPRESSION_SECTION
    UINT32                    UncompressedLength;
    UINT8                     CompressionType;
    //EFI_FREEFORM_SUBTYPE_GUID_SECTION
    EFI_GUID                  SubTypeGuid;
    //EFI_GUID_DEFINED_SECTION
    EFI_GUID                  SectionDefinitionGuid;
    UINT16                    DataOffset;
    UINT16                    Attributes;
    //EFI_USER_INTERFACE_SECTION
    string                    FileNameString;
    //EFI_VERSION_SECTION
    UINT16                    BuildNumber;
    string                    VersionString;

    UINT32                    HeaderSize;
    bool                      isExtend{false};
    bool                      isValid;
    QVector<EFI_GUID>         AprioriList;
public:
    PeCoff                    *peCoffHeader{nullptr};
    Depex                     *dependency{nullptr};

    CommonSection()=delete;
    CommonSection(UINT8* file, INT64 length, INT64 offset, bool Compressed=false, Volume* parent= nullptr);
    ~CommonSection() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void  setInfoStr() override;
    Volume *Reorganize() override;
    [[nodiscard]] INT64 getHeaderSize() const override;
    [[nodiscard]] EFI_GUID getVolumeGuid() const override;

    [[nodiscard]] UINT8 getSectionType() const;
    [[nodiscard]] inline QString getUiName() const { return QString::fromStdString(FileNameString); }
    [[nodiscard]] inline EFI_GUID getSubTypeGuid() const { return SubTypeGuid; };
    [[nodiscard]] inline EFI_GUID getSectionDefinitionGuid() const { return SectionDefinitionGuid; };

    void DecodeDecompressedBuffer(UINT8* DecompressedBuffer, INT64 bufferSize);
};

