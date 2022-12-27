#pragma once

#include "BaseLib.h"
#include "../include/SymbolDefinition.h"
#include "../include/PiFirmwareFile.h"
#include "../include/PiFirmwareVolume.h"

using namespace BaseLibrarySpace;

namespace UefiSpace {

    class FirmwareVolume
    {
    public:
        FirmwareVolume();
    };

    class FirmwareVolumeHeaderClass
    {
    private:
        EFI_FIRMWARE_VOLUME_HEADER FirmwareVolumeHeader;
//        UInt8* ZeroVector{};
//        GUID*  FileSystemGuid{};
//        Int64  FvLength{};
//        string Signature{};
//        UInt32 Attributes{};
//        UInt16 HeaderLength{};
//        UInt16 Checksum{};
//        UInt16 ExtHeaderOffset{};
//        UInt8  Reserved{};
//        UInt8  Revision{};
//        UInt32 NumBlocks{};
//        UInt32 Length{};
    public:
        FirmwareVolumeHeaderClass() = delete;
        FirmwareVolumeHeaderClass(EFI_FIRMWARE_VOLUME_HEADER fv);
        GUID getFvGuid() const;
    };

//    class EFI_FFS_FILE_HEADER
//    {
//    private:
//        GUID*  FfsGuid{};
//        UInt16 IntegrityCheck{};
//        UInt8  Type{};
//        UInt8  Attributes{};
//        UInt8* Size{};
//        UInt8  State{};
//        Int64  ExtendedSize{};
//        bool   isExtended{false};
//    public:
//        EFI_FFS_FILE_HEADER();
//    };

//    class EFI_COMMON_SECTION_HEADER
//    {
//        UInt8* Size{};
//        UInt8  Type{};
//        UInt32 ExtendedSize;
//        bool   isExtended{false};
//    public:
//        EFI_COMMON_SECTION_HEADER();
//    };

//    class EFI_COMPRESSION_SECTION
//    {
//    private:
//        EFI_COMMON_SECTION_HEADER CommonHeader{};
//        UInt32                    UncompressedLength{};
//        UInt8                     CompressionType{};
//    public:
//        EFI_COMPRESSION_SECTION();
//    };

//    class EFI_FREEFORM_SUBTYPE_GUID_SECTION
//    {
//    private:
//        EFI_COMMON_SECTION_HEADER CommonHeader{};
//        GUID*                     SubTypeGuid{};
//    public:
//        EFI_FREEFORM_SUBTYPE_GUID_SECTION();
//    };

//    class EFI_GUID_DEFINED_SECTION
//    {
//    private:
//        EFI_COMMON_SECTION_HEADER CommonHeader{};
//        GUID*                     SectionDefinitionGuid{};
//        UInt16                    DataOffset{};
//        UInt16                    Attributes{};
//    public:
//        EFI_GUID_DEFINED_SECTION();
//    };

//    class EFI_USER_INTERFACE_SECTION
//    {
//        EFI_COMMON_SECTION_HEADER CommonHeader{};
//        string                    FileNameString{};
//    };

//    class EFI_VERSION_SECTION
//    {
//    private:
//        EFI_COMMON_SECTION_HEADER CommonHeader{};
//        UInt16                    BuildNumber{};
//        string                    VersionString{};
//    public:
//        EFI_VERSION_SECTION();
//    };

}
