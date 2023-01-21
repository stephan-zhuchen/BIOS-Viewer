#pragma once

#include "BaseLib.h"
#include <string>
#include <sstream>
#include <map>

using namespace std;
using namespace BaseLibrarySpace;

namespace CapsuleToolSpace {

    class EntryHeaderClass {
    protected:
        Int64 panelOffset{};
        Int64 panelSize{};
    public:
        virtual string getEntryName() = 0;
        virtual void   collectInfo(stringstream& Info) = 0;
        virtual Int64  panelGetOffset();
        virtual Int64  panelGetSize();
        virtual ~EntryHeaderClass();
    };

    class UefiCapsuleHeaderClass : public EntryHeaderClass {
    private:
        GUID*  CapsuleGuid{};
        UInt32 HeaderSize{};
        UInt32 Flags{};
        UInt32 CapsuleImageSize{};
        UInt32 Reserved{};
        bool   PersistAcrossReset{false};
        bool   PopulateSystemTable{false};
        bool   InitiateReset{false};
    public:
        UefiCapsuleHeaderClass() = default;
        Int64   Decode(Buffer& buffer, Int64 offset);
        void    collectInfo(stringstream& Info) override;
        virtual ~UefiCapsuleHeaderClass();
        inline string getEntryName() override {return "EFI_CAPSULE_HEADER";};
    };

    class FmpCapsuleImageHeaderClass : public EntryHeaderClass {
    private:
        Int32 EFI_FIRMWARE_MANAGEMENT_CAPSULE_HEADER_INIT_VERSION = 0x01;
        GUID* FMP_CLIENT_PLATFORM_SYSTEM_MONO;
        GUID* FMP_CLIENT_PLATFORM_SYSTEM_BIOS;
        GUID* FMP_CLIENT_PLATFORM_SYSTEM_EXT_BIOS;
        GUID* FMP_CLIENT_PLATFORM_SYSTEM_IFWI;
        GUID* FMP_CLIENT_PLATFORM_SYSTEM_BTGACM;
        GUID* FMP_CLIENT_PLATFORM_SYSTEM_UCODE;
        GUID* FMP_CLIENT_PLATFORM_SYSTEM_ME;
        GUID* FMP_CLIENT_PLATFORM_SYSTEM_EC;
        GUID* FMP_CLIENT_PLATFORM_TBT_RETIMER;
        GUID* FMP_CLIENT_PLATFORM_SYSTEM_ISH_PDT;
        GUID* FmpDeviceMeFwAdlLpConsGuid;
        GUID* FmpDeviceMeFwAdlHConsGuid;
        GUID* FmpDeviceMeFwAdlLpCorpGuid;
        GUID* FmpDeviceMeFwAdlHCorpGuid;

        UInt32 Version{};
        GUID*  UpdateImageTypeId{};
        UInt8  UpdateImageIndex{};
        UInt8* reserved_bytes{};
        UInt32 UpdateImageSize{};
        UInt32 UpdateVendorCodeSize{};
        UInt64 UpdateHardwareInstance{};
        UInt64 ImageCapsuleSupport{};
    public:
        Int64  PayloadOffset{};
        string CapsuleType;

        FmpCapsuleImageHeaderClass();
        string  Decode(Buffer& buffer, Int64 offset);
        string  getCapsuleTypeFromGuid(GUID& guid) const;
        void    collectInfo(stringstream& Info) override;
        virtual ~FmpCapsuleImageHeaderClass();
        inline string getEntryName() override {return "EFI_FIRMWARE_MANAGEMENT_CAPSULE_IMAGE";};
    };

    class FmpCapsuleHeaderClass : public EntryHeaderClass {
    private:
        UInt32 Version{};
        UInt16 EmbeddedDriverCount{};
        UInt16 PayloadItemCount{};
        string CapsuleType;
        vector<Int64> ItemOffsetList;
    public:
        vector<string> CapsuleTypeList;
        vector<shared_ptr<FmpCapsuleImageHeaderClass>> FmpCapsuleImageHeaderList;

        FmpCapsuleHeaderClass() = default;
        void   Decode(Buffer& buffer, Int64 offset);
        UInt16 getPayloadItemCount() const;
        void   collectInfo(stringstream& Info) override;
        inline string getEntryName() override {return "EFI_FIRMWARE_MANAGEMENT_CAPSULE_HEADER";};
        virtual ~FmpCapsuleHeaderClass() = default;
    };

    class FmpAuthHeaderClass : public EntryHeaderClass {
    private:
        UInt64 MonotonicCount{};
        UInt32 dwLength{};
        UInt16 wRevision{};
        UInt16 wCertificateType{};
        GUID*  CertType{};
    public:
        FmpAuthHeaderClass() = default;
        Int64 Decode(Buffer& buffer, Int64 offset);
        void  collectInfo(stringstream& Info) override;
        inline string getEntryName() override {return "EFI_FIRMWARE_IMAGE_AUTHENTICATION";};
        virtual ~FmpAuthHeaderClass();
    };

    class FmpPayloadHeaderClass : public EntryHeaderClass {
    private:
        string Signature;
        UInt32 HeaderSize;
        UInt32 FwVersion;
        UInt32 LowestSupportedVersion;
        const string FMP_PAYLOAD_HEADER_SIGNATURE = "MSS1";
    public:
        FmpPayloadHeaderClass() = default;
        Int64 Decode(Buffer& buffer, Int64 offset);
        void  collectInfo(stringstream& Info) override;
        inline string getEntryName() override {return "FMP_PAYLOAD_HEADER";};
        virtual ~FmpPayloadHeaderClass() = default;
    };

    class FirmwareVolumeHeaderClass : public EntryHeaderClass {
    private:
        const Int64 XDRSize = 4;
        const Int64 QuadSize = 8;
        const Int64 FvHeaderSize = 64;

        UInt8* ZeroVector{};
        GUID*  FileSystemGuid{};
        Int64  FvLength{};
        string Signature{};
        UInt32 Attributes{};
        UInt16 HeaderLength{};
        UInt16 Checksum{};
        UInt16 ExtHeaderOffset{};
        UInt8  Reserved{};
        UInt8  Revision{};
        UInt32 NumBlocks{};
        UInt32 Length{};
    public:
        Int64 FvBeginOffset{};
        Int64 uCodeBeginOffset{};

        FirmwareVolumeHeaderClass() = default;
        Int64 Decode(Buffer& buffer, Int64 offset, const string& CapsuleType, bool FV_Exist);
        Int64 getFvLength() const;
        void  collectInfo(stringstream& Info) override;
        inline string getEntryName() override {return "EFI_FIRMWARE_VOLUME_HEADER";};
        virtual ~FirmwareVolumeHeaderClass();
    };

    class FfsFileHeaderClass : public EntryHeaderClass {
    private:
        GUID*  FfsGuid{};
        UInt16 IntegrityCheck{};
        UInt8  Type{};
        UInt8  Attributes{};
        UInt8* Size{};
        UInt8  State{};
        Int64  ExtendedSize{};
        bool   isExtended{false};
        const Int64 FfsHeaderSize = 0x18;
        const Int64 ExtendedFfsHeaderSize = 0x20;
    public:
        FfsFileHeaderClass() = default;
        Int64  Decode(Buffer& buffer, Int64 offset, bool Alignment=true, Int64 RelativeAddress=0 ,Int64 AlignValue=0x8);
        Int64  getSize();
        Int64  getFfsHeaderSize();
        void   collectInfo(stringstream& Info) override;
        string getEntryName() override;
        virtual ~FfsFileHeaderClass();
        static Int64 searchFfsWithGuid(Buffer& buffer, Int64 offset, Int64 length, GUID& ffsGuid, bool Reverse);
    };

    class MicrocodeVersionClass : public EntryHeaderClass {
    private:
        UInt32 FwVersion{};
        UInt32 LowestSupportedVersion{};
        string FwVersionString{};
        const Int64 FfsHeaderSize = 10;
    public:
        MicrocodeVersionClass() = default;
        Int64  Decode(Buffer& buffer, Int64 offset);
        string getFwVersion() const;
        string getLSV() const;
        string getFwVersionString() const;
        void   collectInfo(stringstream& Info) override;
        inline string getEntryName() override {return "MicrocodeVersion";};
        virtual ~MicrocodeVersionClass() = default;
    };

    class CPUMicrocodeHeaderClass : public EntryHeaderClass {
    private:
        const Int64 XDRSize = 4;
        const Int64 CpuMicrocodeHeaderSize = 36;

        UInt32 HeaderVersion = 0;
        UInt32 UpdateRevision = 0;
        UInt32 Date = 0;
        UInt32 ProcessorSignature = 0;
        UInt32 Checksum = 0;
        UInt32 LoaderRevision = 0;
        UInt32 ProcessorFlags = 0;
        Int32  DataSize = 0;
    public:
        Int32  TotalSize = 0;
        bool   isEmpty = false;

        CPUMicrocodeHeaderClass() = default;
        Int64   Decode(Buffer& buffer, Int64 offset);
        string  getUcodeDate() const;
        string  getUcodeRevision() const;
        string  getUcodeSignature() const;
        void    collectInfo(stringstream& Info) override;
        virtual ~CPUMicrocodeHeaderClass() = default;
        inline string getEntryName() override {return "Microcode";};
        static vector<Int64> SearchMicrocodeEntryNum(Buffer& buffer, Int64 MicrocodeOffset, Int64 MicrocodeDataSize);
    };

    class ConfigIniClass : public EntryHeaderClass {
    private:
        string ConfigFileName{};
    public:
        string iniContext{};

        ConfigIniClass() = default;
        Int64   Decode(Buffer& buffer, Int64 offset, int contextLength, string&& ConfigName);
        void    collectInfo(stringstream& Info) override;
        virtual ~ConfigIniClass() = default;
        inline string getEntryName() override {return ConfigFileName;};
    };

    class BgupHeaderClass : public EntryHeaderClass {
    private:
        const Int64 XDRSize = 4;
        UInt16 Version{};
        UInt16 Reserved3{};
        string PlatId{};
        UInt16 PkgAttributes{};
        UInt16 Reserved4{};
        UInt16 PslMajorVer{};
        UInt16 PslMinorVer{};
        UInt32 ScriptSectionSize{};
        UInt32 DataSectionSize{};
        UInt32 BiosSvn{};
        UInt32 EcSvn{};
        UInt32 VendorSpecific{};
    public:
        BgupHeaderClass() = default;
        bool    SearchBgup(Buffer& buffer, Int64 BgupOffset);
        void    Decode(Buffer& buffer, Int64 offset);
        void    collectInfo(stringstream& Info) override;
        string  getPlatId() const;
        virtual ~BgupHeaderClass() = default;
        inline string getEntryName() override {return "BGUP_HEADER";};
    };

    class BiosIdClass {
    private:
        string Signature{};
        string BIOS_ID_STRING{};
    public:
        BiosIdClass() = default;
        void    Decode(Buffer& buffer, Int64 offset, Int64 length);
        string  getBiosIDString() const;
        void    collectInfo(stringstream& Info);
        virtual ~BiosIdClass() = default;
    };

    class FitTableClass {
    private:
        Int64  Address{};
        Int8*  Size{};
        UInt8  Rsvd{};
        UInt16 Version{};
        UInt8  Type{};
        UInt8  Checksum{};
    public:
        FitTableClass() = default;
        void    Decode(Buffer& buffer, Int64 offset);
        UInt8   getType() const;
        Int64   getAddress() const;
        virtual ~FitTableClass();
    };

    class AcmClass : public EntryHeaderClass {
    private:
        UInt8 VERSION_NUMBER;
        UInt8 VERSION_MAJOR;
        UInt8 VERSION_MINOR;
    public:
        AcmClass() = default;
        void    Decode(Buffer& buffer, Int64 offset, Int64 DataLength);
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
        void    Decode(Buffer& buffer, Int64 offset, Int64 length);
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
        UInt8   PlatId{};
        UInt8   MajorVer{};
        UInt8   MinorVer{};
        UInt8   BuildVer{};
        string  EcImgSignature = "TKSC";
    public:
        EcClass() = default;
        void    Decode(Buffer& buffer, Int64 offset, Int64 DataLength);
        string  getEcVersion() const;
        void    collectInfo(stringstream& Info) override;
        inline string getEntryName() override {return "EC";};
        virtual ~EcClass() = default;
    };

    class MeClass : public EntryHeaderClass {
    public:
        MeClass() = default;
        void    Decode(Buffer& buffer, Int64 offset, Int64 DataLength);
        void    collectInfo(stringstream& Info) override;
        inline string getEntryName() override {return "ME";};
        virtual ~MeClass() = default;
    };

} // CapsuleTool
