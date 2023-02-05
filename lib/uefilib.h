#pragma once

#include <QString>
#include <stack>
#include "BaseLib.h"
#include "../include/SymbolDefinition.h"
#include "../include/PiFirmwareFile.h"
#include "../include/PiFirmwareVolume.h"
#include "../include/PeImage.h"
#include "../include/Microcode.h"
#include "../include/VariableFormat.h"

namespace UefiSpace {
    using namespace BaseLibrarySpace;

    class FitTableClass;
    class MicrocodeHeaderClass;
    class FfsFile;
    class NvStorageVariable;

    enum class VolumeType {
        FirmwareVolume,
        FfsFile,
        CommonSection
    };

    class Volume {
    public:
        UINT8* data;   // initialized from heap
        INT64  size;
        INT64  offsetFromBegin;
        bool   isCompressed{false};
        bool   isCorrupted{false};
        const char* ErrorMsg = "offset larger than size!";
        QString InfoStr;
    public:
        VolumeType Type;
        Volume() = default;
        Volume(UINT8* fv, INT64 length, INT64 offset=0);
        virtual ~Volume();

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

    class PeCoff : public Volume
    {
    public:
        EFI_IMAGE_DOS_HEADER      dosHeader;
        EFI_TE_IMAGE_HEADER       teHeader;
        EFI_IMAGE_NT_HEADERS32    pe32Header;
        EFI_IMAGE_NT_HEADERS64    pe32plusHeader;
        bool                      isTE{false};
        bool                      isPe32Plus{false};
    public:
        PeCoff()=delete;
        PeCoff(UINT8* file, INT64 length, INT64 offset);

        string getMachineType() const;
        static string getSubsystemName(UINT16 subsystem);
    };

    class Depex : public Volume
    {
    public:
        vector<string>            OrganizedDepexList;
        Depex()=delete;
        Depex(UINT8* file, INT64 length);
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
        bool                      isAprioriRaw{false};
        vector<EFI_GUID>          AprioriList;
    public:
        CommonSection()=delete;
        CommonSection(UINT8* file, INT64 offset, FfsFile *Ffs);
        CommonSection(UINT8* file, INT64 length, INT64 offset, FfsFile *Ffs);
        ~CommonSection();
        void SelfDecode();
        void DecodeDecompressedBuffer(UINT8* DecompressedBuffer, INT64 bufferSize);
        void DecodeChildFile();
        void extracted(std::stringstream &ss);
        void setInfoStr() override;
        INT64 getHeaderSize() const override;
        static INT64 getSectionSize(UINT8* file);
    };

    class FfsFile: public Volume {
    public:
        EFI_FFS_FILE_HEADER    FfsHeader;
        EFI_FFS_FILE_HEADER2   FfsExtHeader;
        INT64                  FfsSize;
        bool                   isExtended{false};
        bool                   headerChecksumValid{false};
        bool                   dataChecksumValid{false};
        bool                   isApriori{false};
        vector<CommonSection*> Sections;
    public:
        FfsFile() = delete;
        FfsFile(UINT8* file, INT64 offset);
        ~FfsFile();

        UINT8 getType() const;
        INT64 getHeaderSize() const override;
        void decodeSections();
        void setInfoStr() override;
    };

    class FirmwareVolume: public Volume {
    public:
        EFI_FIRMWARE_VOLUME_HEADER     FirmwareVolumeHeader;
        EFI_FIRMWARE_VOLUME_EXT_HEADER FirmwareVolumeExtHeader;
        INT64                          FirmwareVolumeSize;
        vector<FfsFile*>               FfsFiles;
        Volume                         *freeSpace{nullptr};
        NvStorageVariable              *NvStorage{nullptr};
        bool                           isExt{false};
        bool                           isEmpty{false};
        bool                           isNv{false};
        bool                           checksumValid{false};
    public:
        FirmwareVolume() = delete;
        FirmwareVolume(UINT8* fv, INT64 length, INT64 offset);
        ~FirmwareVolume();

        GUID getFvGuid(bool returnExt=true) const;
        void decodeFfs();
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
        VARIABLE_STORE_HEADER    NvStoreHeader;
        vector<NvVariableEntry*> VariableList;
        bool                     AuthFlag;

        NvStorageVariable() = delete;
        NvStorageVariable(UINT8* fv, INT64 offset);
        ~NvStorageVariable();
        void setInfoStr() override;
    };

    class BiosImageVolume: public Volume {
    public:
        string BiosID;
        FitTableClass *FitTable{nullptr};
    public:
        BiosImageVolume()=delete;
        BiosImageVolume(UINT8* fv, INT64 length);
        ~BiosImageVolume();

        void setInfoStr() override;
    };

    class FitTableClass {
    public:
        FIRMWARE_INTERFACE_TABLE_ENTRY         FitHeader;
        vector<FIRMWARE_INTERFACE_TABLE_ENTRY> FitEntries;
        vector<MicrocodeHeaderClass*>          MicrocodeEntries;
        INT64 FitEntryNum{0};
        bool  isValid{false};
    public:
        FitTableClass(UINT8* fv, INT64 length);
        ~FitTableClass();
        static string getTypeName(UINT8 type);
    };

    class MicrocodeHeaderClass {
    private:
        UINT8* data;
        INT64  size;
        INT64  offset;
    public:
        bool    isEmpty{false};
        QString InfoStr;
        CPU_MICROCODE_HEADER                 microcodeHeader;
        CPU_MICROCODE_EXTENDED_TABLE_HEADER  *ExtendedTableHeader;
        vector<CPU_MICROCODE_EXTENDED_TABLE> ExtendedMicrocodeList;

        MicrocodeHeaderClass()=delete;
        MicrocodeHeaderClass(UINT8* fv, INT64 address);
        ~MicrocodeHeaderClass();
        void setInfoStr();
    };

}
