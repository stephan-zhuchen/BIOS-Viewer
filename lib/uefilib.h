#pragma once

#include <QString>
#include <QDebug>
#include <stack>
#include "BaseLib.h"
#include "SymbolDefinition.h"
#include "UEFI/PiFirmwareFile.h"
#include "UEFI/PiFirmwareVolume.h"
#include "UEFI/PeImage.h"
#include "UEFI/Microcode.h"
#include "UEFI/VariableFormat.h"
#include "UEFI/BootGuard.h"
#include "UEFI/FspHeader.h"
#include "UEFI/Acpi.h"
#include "openssl/sha.h"

class Elf;

namespace UefiSpace {
    using namespace BaseLibrarySpace;

    class FitTableClass;
    class MicrocodeHeaderClass;
    class AcmHeaderClass;
    class FfsFile;
    class NvStorageVariable;
//    class KeyManifestClass;
//    class BootPolicyManifestClass;
//    class BpmElement;
//    class IBBS_Class;
    class ACPI_Class;

    enum class VolumeType {
        FirmwareVolume,
        FfsFile,
        CommonSection,
        ELF,
        Other
    };

    class Volume {
    public:
        UINT8* data{};   // initialized from heap
        INT64  size{};
        INT64  offsetFromBegin{};
        bool   isCompressed{false};
        bool   isCorrupted{false};
        const char* ErrorMsg = "offset larger than size!";
        QString AdditionalMsg;
        QString InfoStr;
    public:
        VolumeType Type;
        Volume() = default;
        Volume(UINT8* fv, INT64 length, INT64 offset=0, bool Compressed=false);
        virtual ~Volume();

        EFI_GUID getVolumeGuid() const;
        EFI_GUID getGUID(INT64 offset);
        UINT8  getUINT8(INT64 offset);
        UINT16 getUINT16(INT64 offset);
        UINT32 getUINT32(INT64 offset);
        UINT64 getUINT64(INT64 offset);
        INT8   getINT8(INT64 offset);
        INT16  getINT16(INT64 offset);
        INT32  getINT24(INT64 offset);
        INT32  getINT32(INT64 offset);
        INT64  getINT64(INT64 offset);
        UINT8* getBytes(INT64 offset, INT64 length);
        INT64  getSize() const;
        virtual INT64 getHeaderSize() const;
        virtual void setInfoStr();
    };

    class EmptyVolume : public Volume {
    public:
        EmptyVolume()=delete;
        EmptyVolume(UINT8* file, INT64 length, INT64 offset);
    };

    class PeCoff : public Volume
    {
    public:
        EFI_IMAGE_DOS_HEADER      dosHeader{};
        EFI_TE_IMAGE_HEADER       teHeader{};
        EFI_IMAGE_NT_HEADERS32    pe32Header{};
        EFI_IMAGE_NT_HEADERS64    pe32plusHeader{};
        bool                      isTE{false};
        bool                      isPe32Plus{false};
    public:
        PeCoff()=delete;
        PeCoff(UINT8* file, INT64 length, INT64 offset, bool Compressed=false);

        string getMachineType() const;
        static string getSubsystemName(UINT16 subsystem);
    };

    class Depex : public Volume
    {
    public:
        vector<string>            OrganizedDepexList;
        Depex()=delete;
        Depex(UINT8* file, INT64 length, bool Compressed=false);
        static string getOpcodeString(UINT8 op);
    };

    class CommonSection: public Volume {
    public:
        EFI_COMMON_SECTION_HEADER CommonHeader;
        UINT32                    ExtendedSize;
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

        bool                      isExtend{false};
        UINT32                    decompressedSize{0};
        FfsFile                   *ParentFFS{nullptr};
        vector<Volume*>           ChildFile;
        PeCoff                    *peCoffHeader{nullptr};
        Depex                     *dependency{nullptr};
        bool                      isValid;
        bool                      isAprioriRaw{false};
        bool                      isElfFormat{false};
        bool                      isFspHeader{false};
        bool                      isAcpiHeader{false};
        vector<EFI_GUID>          AprioriList;
        ACPI_Class                *AcpiTable{nullptr};
        UINT8                     *DecompressedBufferOnHeap{nullptr};
    public:
        CommonSection()=delete;
        CommonSection(UINT8* file, INT64 offset, FfsFile *Ffs, bool Compressed=false);
        CommonSection(UINT8* file, INT64 length, INT64 offset, FfsFile *Ffs, bool Compressed=false);
        ~CommonSection() override;
        void SelfDecode();
        void DecodeDecompressedBuffer(UINT8* DecompressedBuffer, INT64 bufferSize);
        void DecodeChildFile();
        bool CheckValidation();
        void setInfoStr() override;
        INT64 getHeaderSize() const override;
        static INT64 getSectionSize(UINT8* file);
    };

    class FfsFile: public Volume {
    public:
        EFI_FFS_FILE_HEADER    FfsHeader{};
        EFI_FFS_FILE_HEADER2   FfsExtHeader{};
        INT64                  FfsSize;
        bool                   isExtended{false};
        bool                   headerChecksumValid{false};
        bool                   dataChecksumValid{false};
        bool                   isApriori{false};
        vector<CommonSection*> Sections;
    public:
        FfsFile() = delete;
        FfsFile(UINT8* file, INT64 offset, bool Compressed=false);
        ~FfsFile() override;

        UINT8 getType() const;
        INT64 getHeaderSize() const override;
        void decodeSections();
        void setInfoStr() override;
    };

    class FirmwareVolume: public Volume {
    public:
        EFI_FIRMWARE_VOLUME_HEADER     FirmwareVolumeHeader{};
        EFI_FIRMWARE_VOLUME_EXT_HEADER FirmwareVolumeExtHeader{};
        INT64                          FirmwareVolumeSize;
        vector<FfsFile*>               FfsFiles;
        Volume                         *freeSpace{nullptr};
        NvStorageVariable              *NvStorage{nullptr};
        bool                           isExt{false};
        bool                           isEmpty;
        bool                           isNv{false};
        bool                           checksumValid{false};
    public:
        FirmwareVolume() = delete;
        FirmwareVolume(UINT8* fv, INT64 length, INT64 offset, bool empty=false, bool Compressed=false);
        ~FirmwareVolume() override;

        GUID getFvGuid(bool returnExt=true) const;
        void decodeFfs(bool multiThread=false);
        INT64 getHeaderSize() const override;
        void setInfoStr() override;

        static bool isValidFirmwareVolume(EFI_FIRMWARE_VOLUME_HEADER* address);
    };

    class NvVariableEntry : public Volume {
    public:
        VARIABLE_HEADER               *VariableHeader;
        AUTHENTICATED_VARIABLE_HEADER *AuthVariableHeader;
        bool                          AuthFlag;
        string                        VariableName;
        UINT8                         *DataPtr;
        INT64                         DataSize;

        NvVariableEntry() = delete;
        NvVariableEntry(UINT8* fv, INT64 offset, bool isAuth);
        INT64 getHeaderSize() const override;
        void setInfoStr() override;
    };

    class NvStorageVariable : public Volume {
    public:
        VARIABLE_STORE_HEADER    NvStoreHeader{};
        vector<NvVariableEntry*> VariableList;
        bool                     AuthFlag;

        NvStorageVariable() = delete;
        NvStorageVariable(UINT8* fv, INT64 offset);
        ~NvStorageVariable() override;
        void setInfoStr() override;
    };

    class FspHeader : public Volume {
    private:
        TABLES mTable{};
        bool   validFlag{true};
    public:
        FspHeader(UINT8* fv, INT64 length, INT64 offset);
        ~FspHeader() override;
        bool isValid() const;
        void setInfoStr() override;
        static bool isFspHeader(const UINT8  *ImageBase);
    };

    class BiosImageVolume: public Volume {
    public:
        string BiosID;
        FitTableClass           *FitTable{nullptr};
        vector<FirmwareVolume*> *FvData;
        pair<INT64, INT64>      NV_Region;  // pair<offset, size>
        pair<INT64, INT64>      OBB_Region;
        pair<INT64, INT64>      IBB_Region;
        pair<INT64, INT64>      IBBR_Region;
        vector<ACPI_Class*>     AcpiTables;
        UINT8                   ObbDigest[SHA256_DIGEST_LENGTH];
        bool   ObbDigestValid{false};
        bool   foundBiosID{false};
        bool   isResiliency{false};
        bool   DebugFlag{false};
    public:
        BiosImageVolume()=delete;
        BiosImageVolume(UINT8* fv, INT64 length, INT64 offset=0);
        ~BiosImageVolume();

        void setBiosID();
        void getObbDigest();
        void setDebugFlag();
        void decodeBiosRegion();
        void decodeAcpiTable(Volume* Vol);
        string getFlashmap();
        void setInfoStr() override;
    };

    class FitTableClass {
    public:
        FIRMWARE_INTERFACE_TABLE_ENTRY         FitHeader{};
        vector<FIRMWARE_INTERFACE_TABLE_ENTRY> FitEntries;
        vector<MicrocodeHeaderClass*>          MicrocodeEntries;
        vector<AcmHeaderClass*>                AcmEntries;
        INT64 FitEntryNum{0};
        bool  isValid{false};
        bool  isChecksumValid{false};
    public:
        FitTableClass(UINT8* fv, INT64 length);
        ~FitTableClass();
        static string getTypeName(UINT8 type);
    };

    class MicrocodeHeaderClass {
    private:
        UINT8* data;
        INT64  size{};
        INT64  offset;
    public:
        bool    isEmpty{false};
        QString InfoStr;
        CPU_MICROCODE_HEADER                 microcodeHeader{};
        CPU_MICROCODE_EXTENDED_TABLE_HEADER  *ExtendedTableHeader;
        vector<CPU_MICROCODE_EXTENDED_TABLE> ExtendedMicrocodeList;

        MicrocodeHeaderClass()=delete;
        MicrocodeHeaderClass(UINT8* fv, INT64 address);
        ~MicrocodeHeaderClass();
        void setInfoStr();
    };

    class BootGuardClass {
    protected:
        UINT8* data;
        INT64  size;
        INT64  offset;
        static void InternalDumpData(stringstream &ss, UINT8* Data, INT64 Size);
    public:
        BootGuardClass()=delete;
        BootGuardClass(UINT8* fv, INT64 length, INT64 address);
        virtual void setInfoStr();
        static string DumpHex(UINT8* HexData, INT64 length, bool SingleLine = false);
        static string getAlgName(UINT16 Alg);
        virtual ~BootGuardClass();
    };

    class AcmHeaderClass : public BootGuardClass {
    private:
        bool   ProdFlag{true};
        bool   ValidFlag{true};
    public:
        QString          InfoStr;
        ACM_HEADER       acmHeader{};
        Ext_ACM_Header   ExtAcmHeader{};
        Ext_ACM_Header3  ExtAcmHeader3{};
        ACM_INFO_TABLE   *AcmInfoTable;
        ACM_VERSION      AcmVersion{};
        bool             isAcm3{false};
        AcmHeaderClass() = delete;
        AcmHeaderClass(UINT8* fv, INT64 address);
        ~AcmHeaderClass() override;
        bool isValid() const;
        bool isProd() const;
        void setInfoStr() override;
    };

    class ACPI_Class : public Volume {
    public:
        EFI_ACPI_DESCRIPTION_HEADER   AcpiHeader{};
        string                        AcpiTableSignature;
        string                        AcpiTableOemID;
        string                        AcpiTableOemTableID;
        bool                          ValidFlag{false};
    public:
        ACPI_Class()=delete;
        ACPI_Class(UINT8* fv, INT64 length, INT64 offset, bool needValidation=true);
        ~ACPI_Class() override;
        bool isValid() const;
        static bool isAcpiHeader(const UINT8  *ImageBase, INT64 length);
        void setInfoStr() override;
    };

}
