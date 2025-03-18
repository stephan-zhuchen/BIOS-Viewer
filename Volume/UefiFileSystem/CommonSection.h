//
// Created by stephan on 8/29/2023.
//

#pragma once
#include "Volume.h"
#include "UEFI/PiFirmwareFile.h"
#include "UEFI/CapsuleSpec.h"
#include "Payload/PE32.h"

using std::string;
class FspHeader;
class AcpiClass;
class ELF;

class Depex {
private:
    UINT8* data{};
    INT64  size{};
public:
    vector<string>            OrganizedDepexList;
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
    EFI_GUID_DEFINED_SECTION  GuidDefinedSection;
    EFI_CERT_BLOCK_RSA_2048_SHA256 RSA2048SHA256;
    //EFI_USER_INTERFACE_SECTION
    string                    FileNameString;
    //EFI_VERSION_SECTION
    UINT16                    BuildNumber;
    string                    VersionString;

    UINT32                    HeaderSize;
    bool                      isExtSection{false};
    bool                      isValid;
    vector<EFI_GUID>          AprioriList;
public:
    PE32                      *Pe32Header{nullptr};
    Depex                     *Dependency{nullptr};

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
    [[nodiscard]] inline string getUiName() const { return FileNameString; }
    [[nodiscard]] inline EFI_GUID getSubTypeGuid() const { return SubTypeGuid; };
    [[nodiscard]] inline EFI_GUID getSectionDefinitionGuid() const { return GuidDefinedSection.SectionDefinitionGuid; };

    void DecodeDecompressedBuffer(UINT8* DecompressedBuffer, INT64 bufferSize);
};

