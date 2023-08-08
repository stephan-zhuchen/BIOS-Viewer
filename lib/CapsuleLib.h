#pragma once

#include <string>
#include <sstream>
#include <map>
#include <QString>
#include "BaseLib.h"
#include "UEFI/BiosGuard.h"

using namespace std;
using namespace BaseLibrarySpace;

namespace CapsuleToolSpace {

    class CapsuleException : public exception {
    private:
        string message;
    public:
        CapsuleException();
        explicit CapsuleException(const string& str);
        const char* what() const noexcept override;
    };

    class CapsuleError : public exception {
    private:
        string message;
    public:
        CapsuleError();
        explicit CapsuleError(const string& str);
        const char* what() const noexcept override;
    };

    class EntryHeaderClass {
    protected:
        INT64 panelOffset{};
        INT64 panelSize{};
    public:
        virtual string getEntryName() = 0;
        virtual void   collectInfo(stringstream& Info) = 0;
        virtual INT64  panelGetOffset();
        virtual INT64  panelGetSize();
        virtual ~EntryHeaderClass();
    };

    class CapsuleOverviewClass : public EntryHeaderClass {
    public:
        string OverviewMsg;
        CapsuleOverviewClass() = default;
        void Decode(Buffer& buffer, INT64 offset, INT64 length);
        void collectInfo(stringstream& Info) override;
        ~CapsuleOverviewClass() = default;
        void setOverviewMsg(string msg);
        inline string getEntryName() override {return "Capsule Overview";};
    };

    class UefiCapsuleHeaderClass : public EntryHeaderClass {
    private:
        EFI_GUID  CapsuleGuid{};
        UINT32 HeaderSize{};
        UINT32 Flags{};
        UINT32 CapsuleImageSize{};
        UINT32 Reserved{};
        bool   PersistAcrossReset{false};
        bool   PopulateSystemTable{false};
        bool   InitiateReset{false};
    public:
        UefiCapsuleHeaderClass() = default;
        INT64   Decode(Buffer& buffer, INT64 offset);
        void    collectInfo(stringstream& Info) override;
        virtual ~UefiCapsuleHeaderClass();
        inline string getEntryName() override {return "EFI_CAPSULE_HEADER";};
    };

    class FmpCapsuleImageHeaderClass : public EntryHeaderClass {
    private:
        INT32 EFI_FIRMWARE_MANAGEMENT_CAPSULE_HEADER_INIT_VERSION = 0x01;

        UINT32 Version{};
        EFI_GUID  UpdateImageTypeId{};
        UINT8  UpdateImageIndex{};
        UINT8* reserved_bytes{};
        UINT32 UpdateImageSize{};
        UINT32 UpdateVendorCodeSize{};
        UINT64 UpdateHardwareInstance{};
        UINT64 ImageCapsuleSupport{};
    public:
        INT64  PayloadOffset{};
        string CapsuleType;

        FmpCapsuleImageHeaderClass();
        string  Decode(Buffer& buffer, INT64 offset);
        string  getCapsuleTypeFromGuid(EFI_GUID& guid) const;
        void    collectInfo(stringstream& Info) override;
        virtual ~FmpCapsuleImageHeaderClass();
        inline string getEntryName() override {return "EFI_FIRMWARE_MANAGEMENT_CAPSULE_IMAGE";};
    };

    class FmpCapsuleHeaderClass : public EntryHeaderClass {
    private:
        UINT32 Version{};
        UINT16 EmbeddedDriverCount{};
        UINT16 PayloadItemCount{};
        string CapsuleType;
        vector<INT64> ItemOffsetList;
    public:
        vector<string> CapsuleTypeList;
        vector<shared_ptr<FmpCapsuleImageHeaderClass>> FmpCapsuleImageHeaderList;

        FmpCapsuleHeaderClass() = default;
        void   Decode(Buffer& buffer, INT64 offset);
        UINT16 getPayloadItemCount() const;
        UINT16 getDriverItemCount() const;
        void   collectInfo(stringstream& Info) override;
        inline string getEntryName() override {return "EFI_FIRMWARE_MANAGEMENT_CAPSULE_HEADER";};
        virtual ~FmpCapsuleHeaderClass() = default;
    };

    class FmpAuthHeaderClass : public EntryHeaderClass {
    private:
        UINT64 MonotonicCount{};
        UINT32 dwLength{};
        UINT16 wRevision{};
        UINT16 wCertificateType{};
        EFI_GUID  CertType{};
    public:
        FmpAuthHeaderClass() = default;
        INT64 Decode(Buffer& buffer, INT64 offset);
        void  collectInfo(stringstream& Info) override;
        inline string getEntryName() override {return "EFI_FIRMWARE_IMAGE_AUTHENTICATION";};
        virtual ~FmpAuthHeaderClass();
    };

    class FmpPayloadHeaderClass : public EntryHeaderClass {
    private:
        string Signature;
        UINT32 HeaderSize;
        UINT32 FwVersion;
        UINT32 LowestSupportedVersion;
        const string FMP_PAYLOAD_HEADER_SIGNATURE = "MSS1";
    public:
        FmpPayloadHeaderClass() = default;
        INT64 Decode(Buffer& buffer, INT64 offset);
        void  collectInfo(stringstream& Info) override;
        inline string getEntryName() override {return "FMP_PAYLOAD_HEADER";};
        virtual ~FmpPayloadHeaderClass() = default;
    };

    class FirmwareVolumeHeaderClass : public EntryHeaderClass {
    private:
        const INT64 XDRSize = 4;
        const INT64 QuadSize = 8;
        const INT64 FvHeaderSize = 64;

        UINT8* ZeroVector{};
        EFI_GUID  FileSystemGuid{};
        INT64  FvLength{};
        string Signature{};
        UINT32 Attributes{};
        UINT16 HeaderLength{};
        UINT16 Checksum{};
        UINT16 ExtHeaderOffset{};
        UINT8  Reserved{};
        UINT8  Revision{};
        UINT32 NumBlocks{};
        UINT32 Length{};
    public:
        INT64 FvBeginOffset{};
        INT64 uCodeBeginOffset{};

        FirmwareVolumeHeaderClass() = default;
        INT64 Decode(Buffer& buffer, INT64 offset, const string& CapsuleType, bool FV_Exist);
        INT64 getFvLength() const;
        void  collectInfo(stringstream& Info) override;
        inline string getEntryName() override {return "EFI_FIRMWARE_VOLUME_HEADER";};
        virtual ~FirmwareVolumeHeaderClass();
    };

    class FfsFileHeaderClass : public EntryHeaderClass {
    private:
        EFI_GUID  FfsGuid{};
        UINT16 IntegrityCheck{};
        UINT8  Type{};
        UINT8  Attributes{};
        UINT8* Size{};
        UINT8  State{};
        INT64  ExtendedSize{};
        bool   isExtended{false};
        const INT64 FfsHeaderSize = 0x18;
        const INT64 ExtendedFfsHeaderSize = 0x20;
    public:
        FfsFileHeaderClass() = default;
        INT64  Decode(Buffer& buffer, INT64 offset, bool Alignment=true, INT64 RelativeAddress=0 ,INT64 AlignValue=0x8);
        INT64  getSize();
        INT64  getFfsHeaderSize();
        void   collectInfo(stringstream& Info) override;
        string getEntryName() override;
        virtual ~FfsFileHeaderClass();
        static INT64 searchFfsWithGuid(Buffer& buffer, INT64 offset, INT64 length, GUID& ffsGuid, bool Reverse);
    };

    class MicrocodeVersionClass : public EntryHeaderClass {
    private:
        UINT32 FwVersion{};
        UINT32 LowestSupportedVersion{};
        string FwVersionString{};
        const INT64 FfsHeaderSize = 10;
    public:
        MicrocodeVersionClass() = default;
        INT64  Decode(Buffer& buffer, INT64 offset);
        string getFwVersion() const;
        string getLSV() const;
        string getFwVersionString() const;
        void   collectInfo(stringstream& Info) override;
        inline string getEntryName() override {return "MicrocodeVersion";};
        virtual ~MicrocodeVersionClass() = default;
    };

    class CPUMicrocodeHeaderClass : public EntryHeaderClass {
    private:
        const INT64 XDRSize = 4;
        const INT64 CpuMicrocodeHeaderSize = 36;

        UINT32 HeaderVersion = 0;
        UINT32 UpdateRevision = 0;
        UINT32 Date = 0;
        UINT32 ProcessorSignature = 0;
        UINT32 Checksum = 0;
        UINT32 LoaderRevision = 0;
        UINT32 ProcessorFlags = 0;
        INT32  DataSize = 0;
    public:
        INT32  TotalSize = 0;
        bool   isEmpty = false;

        CPUMicrocodeHeaderClass() = default;
        INT64   Decode(Buffer& buffer, INT64 offset);
        string  getUcodeDate() const;
        string  getUcodeRevision() const;
        string  getUcodeSignature() const;
        void    collectInfo(stringstream& Info) override;
        virtual ~CPUMicrocodeHeaderClass() = default;
        inline string getEntryName() override {return "Microcode";};
        static vector<INT64> SearchMicrocodeEntryNum(Buffer& buffer, INT64 MicrocodeOffset, INT64 MicrocodeDataSize);
    };

    struct BgupConfig {
        string BgupContent;
        UINT32 BgupOffset;
        UINT32 BgupSize;
    };

    class ConfigIniClass : public EntryHeaderClass {
    private:
        string ConfigFileName{};
        std::map<std::string, std::map<std::string, std::string>> iniData;
    public:
        string iniContext{};
        INT32  NumOfUpdate;
        vector<BgupConfig> BgupList;

        ConfigIniClass() = default;
        INT64   Decode(Buffer& buffer, INT64 offset, int contextLength, string&& ConfigName);
        void    collectInfo(stringstream& Info) override;
        virtual ~ConfigIniClass() = default;
        inline string getEntryName() override {return ConfigFileName;};
        string TrimString(const string& inputString);
        string GetIniValue(const string& section, const string& key);
    };

    class BgupHeaderClass : public EntryHeaderClass {
    private:
        const INT64   XDRSize = 4;
        BGUP_HEADER   BgupHeader;
        BGUPC_HEADER  BgupCHeader;
        string        Algorithm;
        string        Content;
        INT32         ModulusSize;
        UINT8         *ModulusData{nullptr};
        INT32         RSAKeySize;
        UINT8         *UpdatePackageDigest{nullptr};
    public:
        BgupHeaderClass() = default;
        bool    SearchBgup(Buffer& buffer, INT64 BgupOffset, UINT32 &BgupSize);
        void    Decode(Buffer& buffer, INT64 offset, INT64 length, string content);
        void    collectInfo(stringstream& Info) override;
        string  getPlatId() const;
        virtual ~BgupHeaderClass();
        string getEntryName() override;
    };

    class BiosIdClass {
    private:
        string Signature{};
        string BIOS_ID_STRING{};
    public:
        BiosIdClass() = default;
        void    Decode(Buffer& buffer, INT64 offset, INT64 length);
        string  getBiosIDString() const;
        void    collectInfo(stringstream& Info);
        virtual ~BiosIdClass() = default;
    };

    class FitTableClass {
    private:
        INT64  Address{};
        INT8*  Size{};
        UINT8  Rsvd{};
        UINT16 Version{};
        UINT8  Type{};
        UINT8  Checksum{};
    public:
        FitTableClass() = default;
        void    Decode(Buffer& buffer, INT64 offset);
        UINT8   getType() const;
        INT64   getAddress() const;
        virtual ~FitTableClass();
    };

    class AcmClass : public EntryHeaderClass {
    private:
        string InfoStr;
        string AcmVersion;
    public:
        AcmClass() = default;
        void    Decode(Buffer& buffer, INT64 offset, INT64 DataLength);
        void    collectInfo(stringstream& Info) override;
        string  getAcmVersion();
        virtual ~AcmClass() = default;
        inline string getEntryName() override {return "ACM";};
    };

    class BiosClass : public EntryHeaderClass {
    private:
        bool        DebugFlag{false};
        bool        ResiliencyFlag{false};
        BiosIdClass BiosId{};
        AcmClass    Acm{};
        vector<CPUMicrocodeHeaderClass> uCodeEntryList;
    public:
        BiosClass() = default;
        void    Decode(Buffer& buffer, INT64 offset, INT64 length);
        string  getBiosIDString() const;
        void    collectInfo(stringstream& Info) override;
        virtual ~BiosClass() = default;
        inline string getEntryName() override { return "BIOS"; };
        inline bool   isDebug() { return DebugFlag; };
        inline bool   isResiliency() { return ResiliencyFlag; };
    };

    class EcClass : public EntryHeaderClass {
    private:
        string  Signature{};
        UINT8   PlatId{};
        UINT8   MajorVer{};
        UINT8   MinorVer{};
        UINT8   BuildVer{};
        string  EcImgSignature = "TKSC";
    public:
        EcClass() = default;
        void    Decode(Buffer& buffer, INT64 offset, INT64 DataLength);
        string  getEcVersion() const;
        void    collectInfo(stringstream& Info) override;
        inline string getEntryName() override {return "EC";};
        virtual ~EcClass() = default;
    };

    class MeClass : public EntryHeaderClass {
    public:
        MeClass() = default;
        void    Decode(Buffer& buffer, INT64 offset, INT64 DataLength);
        void    collectInfo(stringstream& Info) override;
        inline string getEntryName() override {return "ME";};
        virtual ~MeClass() = default;
    };

} // CapsuleTool
