//
// Created by Stephan on 2022/6/14.
//

#include <iomanip>
#include <vector>
#include "CapsuleClassLib.h"

using namespace std;
using namespace BaseLibrarySpace;

namespace CapsuleToolSpace {

    Int64 EntryHeaderClass::panelGetOffset() {
        return panelOffset;
    }

    Int64 EntryHeaderClass::panelGetSize() {
        return panelSize;
    }

    EntryHeaderClass::~EntryHeaderClass() {
    }

    UefiCapsuleHeaderClass::~UefiCapsuleHeaderClass() {
        delete CapsuleGuid;
    }

    Int64 UefiCapsuleHeaderClass::Decode(Buffer& buffer, Int64 offset) {
        panelOffset = offset;
        panelSize = 0x20;

        buffer.setOffset(offset);
        CapsuleGuid = buffer.parseGUID();
        HeaderSize = buffer.parseUINT32();
        Flags = buffer.parseUINT32();
        CapsuleImageSize = buffer.parseUINT32();
        Reserved = buffer.parseUINT32();

        const UInt32 CAPSULE_FLAGS_PERSIST_ACROSS_RESET   = 0x00010000;
        const UInt32 CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE  = 0x00020000;
        const UInt32 CAPSULE_FLAGS_INITIATE_RESET         = 0x00040000;

        PersistAcrossReset  = (Flags & CAPSULE_FLAGS_PERSIST_ACROSS_RESET) != 0;
        PopulateSystemTable = (Flags & CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE) != 0;
        InitiateReset       = (Flags & CAPSULE_FLAGS_INITIATE_RESET) != 0;

        GUID EFI_FIRMWARE_MANAGEMENT_CAPSULE_ID_GUID{ "6DCBD5ED-E82D-4C44-BDA1-7194199AD92A" };
        if (*CapsuleGuid != EFI_FIRMWARE_MANAGEMENT_CAPSULE_ID_GUID) {
            cout << CapsuleGuid << endl;
            throw CapsuleError("No Capsule!");
        }
        return buffer.getOffset();
    }

    void UefiCapsuleHeaderClass::collectInfo(stringstream& Info) {
        Info << hex << setfill('0')
            << "CapsuleGuid =" << CapsuleGuid
            << "\nHeaderSize =" << setw(8) << HeaderSize
            << "\nFlags = " << setw(8) << Flags
            << "\nCapsuleImageSize = " << setw(8) << CapsuleImageSize
            << "\n\n"
            << "Flag:\n";
        if (PersistAcrossReset) {Info << "PersistAcrossReset\n";}
        if (PopulateSystemTable) {Info << "PopulateSystemTable\n";}
        if (InitiateReset) {Info << "InitiateReset\n";}
    }

    string FmpCapsuleImageHeaderClass::getCapsuleTypeFromGuid(GUID& guid) const {
        if (guid == *FMP_CLIENT_PLATFORM_SYSTEM_MONO) {
            return "Monolithic";
        }
        else if (guid == *FMP_CLIENT_PLATFORM_SYSTEM_BIOS) {
            return "BIOS";
        }
        else if (guid == *FMP_CLIENT_PLATFORM_SYSTEM_EXT_BIOS) {
            return "Extended BIOS";
        }
        else if (guid == *FMP_CLIENT_PLATFORM_SYSTEM_IFWI) {
            return "IFWI";
        }
        else if (guid == *FMP_CLIENT_PLATFORM_SYSTEM_BTGACM) {
            return "BtgAcm";
        }
        else if (guid == *FMP_CLIENT_PLATFORM_SYSTEM_UCODE) {
            return "uCode";
        }
        else if (guid == *FMP_CLIENT_PLATFORM_SYSTEM_ME) {
            return "ME";
        }
        else if (guid == *FMP_CLIENT_PLATFORM_SYSTEM_EC) {
            return "EC";
        }
        else if (guid == *FMP_CLIENT_PLATFORM_TBT_RETIMER) {
            return "Retimer";
        }
        else if (guid == *FMP_CLIENT_PLATFORM_SYSTEM_ISH_PDT) {
            return "IshPdt";
        }
        else if (guid == *FmpDeviceMeFwAdlLpConsGuid) {
            return "ME Lp_Cons";
        }
        else if (guid == *FmpDeviceMeFwAdlHConsGuid) {
            return "ME H_Cons";
        }
        else if (guid == *FmpDeviceMeFwAdlLpCorpGuid) {
            return "ME Lp_Corp";
        }
        else if (guid == *FmpDeviceMeFwAdlHCorpGuid) {
            return "ME H_Corp";
        }
        throw CapsuleError();
    }

    FmpCapsuleImageHeaderClass::FmpCapsuleImageHeaderClass() {
        FMP_CLIENT_PLATFORM_SYSTEM_MONO    = new GUID("7FEB1D5D-33F4-48D3-BD11-C4B36B6D0E57");
        FMP_CLIENT_PLATFORM_SYSTEM_BIOS    = new GUID("6C8E136F-D3E6-4131-AC32-4687CB4ABD27");
        FMP_CLIENT_PLATFORM_SYSTEM_EXT_BIOS = new GUID("fd0aadc7-9696-4795-8f34-53069f759fb0");
        FMP_CLIENT_PLATFORM_SYSTEM_IFWI    = new GUID("F9FA5980-95F5-491A-9583-136D7F26D63E");
        FMP_CLIENT_PLATFORM_SYSTEM_BTGACM  = new GUID("4E88068B-41B2-4E05-893C-DB0B43F7D348");
        FMP_CLIENT_PLATFORM_SYSTEM_UCODE   = new GUID("69585D92-B50A-4AD7-B265-2EB1AE066574");
        FMP_CLIENT_PLATFORM_SYSTEM_ME      = new GUID("0EAB05C1-766A-4805-A039-3081DE0210C7");
        FMP_CLIENT_PLATFORM_SYSTEM_EC      = new GUID("3DD84775-EC79-4ECB-8404-74DE030C3F77");
        FMP_CLIENT_PLATFORM_TBT_RETIMER    = new GUID("2FE2CBFC-B9AA-4A93-AB5B-40173B581C42");
        FMP_CLIENT_PLATFORM_SYSTEM_ISH_PDT = new GUID("76CA0AD8-4A14-4389-B7E5-FD88791762AD");
        FmpDeviceMeFwAdlLpConsGuid         = new GUID("23192307-d667-4bdf-af1a-6059db171246");
        FmpDeviceMeFwAdlHConsGuid          = new GUID("7aa69739-8f78-41cb-bf44-854e2cb516bd");
        FmpDeviceMeFwAdlLpCorpGuid         = new GUID("4e78ce68-5389-4a95-bf10-e3568c30caf8");
        FmpDeviceMeFwAdlHCorpGuid          = new GUID("347efe23-9f9a-4b26-b4db-e2414872dd14");
    }

    FmpCapsuleImageHeaderClass::~FmpCapsuleImageHeaderClass() {
        delete FMP_CLIENT_PLATFORM_SYSTEM_MONO;
        delete FMP_CLIENT_PLATFORM_SYSTEM_BIOS;
        delete FMP_CLIENT_PLATFORM_SYSTEM_EXT_BIOS;
        delete FMP_CLIENT_PLATFORM_SYSTEM_IFWI;
        delete FMP_CLIENT_PLATFORM_SYSTEM_BTGACM;
        delete FMP_CLIENT_PLATFORM_SYSTEM_UCODE;
        delete FMP_CLIENT_PLATFORM_SYSTEM_ME;
        delete FMP_CLIENT_PLATFORM_SYSTEM_EC;
        delete FMP_CLIENT_PLATFORM_TBT_RETIMER;
        delete FMP_CLIENT_PLATFORM_SYSTEM_ISH_PDT;
        delete FmpDeviceMeFwAdlLpConsGuid;
        delete FmpDeviceMeFwAdlHConsGuid;
        delete FmpDeviceMeFwAdlLpCorpGuid;
        delete FmpDeviceMeFwAdlHCorpGuid;
        delete UpdateImageTypeId;
        delete reserved_bytes;
    }

    string FmpCapsuleImageHeaderClass::Decode(Buffer& buffer, Int64 offset) {
        panelOffset = offset;
        panelSize = 0x20;

        buffer.setOffset(offset);
        Version = buffer.parseUINT32();
        UpdateImageTypeId = buffer.parseGUID();
        UpdateImageIndex = buffer.parseUINT8();
        reserved_bytes = buffer.parseBytes(3);
        UpdateImageSize = buffer.parseUINT32();
        UpdateVendorCodeSize = buffer.parseUINT32();
        if (Version >= 2) {
            panelSize += 0x8;
            UpdateHardwareInstance = buffer.parseUINT64();
        }
        if (Version >= 3) {
            panelSize += 0x8;
            ImageCapsuleSupport = buffer.parseUINT64();
        }
        PayloadOffset = buffer.getOffset();
        CapsuleType = getCapsuleTypeFromGuid(*UpdateImageTypeId);
        return CapsuleType;
    }

    void FmpCapsuleImageHeaderClass::collectInfo(stringstream& Info)
    {
        Info << hex << setfill('0')
            << "Version =" << setw(8) << Version << endl
            << "UpdateImageTypeId =" << UpdateImageTypeId << endl
            << "UpdateImageIndex =" << setw(2) << (UInt16)UpdateImageIndex << endl
            << "UpdateImageSize =" << setw(8) << UpdateImageSize << endl
            << "UpdateVendorCodeSize =" << setw(8) << UpdateVendorCodeSize << endl;
        if (Version >= 2) {
            Info << "UpdateHardwareInstance =" << setw(16) << UpdateHardwareInstance << endl;
        }
        if (Version >= 3) {
            Info << "ImageCapsuleSupport =" << setw(16) << ImageCapsuleSupport << endl;
        }
    }

    void FmpCapsuleHeaderClass::Decode(Buffer& buffer, Int64 offset) {
        panelOffset = offset;
        panelSize = 8;

        buffer.setOffset(offset);
        Int64 ItemOffset;
        Version = buffer.parseUINT32();
        EmbeddedDriverCount = buffer.parseUINT16();
        PayloadItemCount = buffer.parseUINT16();
        //assert: Version >= 0x01
        ItemOffsetList = vector<Int64>(EmbeddedDriverCount + PayloadItemCount, 0);
        CapsuleTypeList = vector<string>(PayloadItemCount, "");

        for (int index = 0; index < PayloadItemCount; index++) {
            FmpCapsuleImageHeaderList.push_back(nullptr);
        }

        for (int index = 0; index < EmbeddedDriverCount + PayloadItemCount; index++) {
            ItemOffset = buffer.parseINT64();
            //assert: ItemOffset <= (UInt64)Buffer.buffer.Length
            ItemOffsetList[index] = ItemOffset + 0x20;
        }

        for (int index = EmbeddedDriverCount; index < EmbeddedDriverCount + PayloadItemCount; index++) {
            ItemOffset = ItemOffsetList[index];
            auto FmpCapsuleImageHeader = make_shared<FmpCapsuleImageHeaderClass>();
            CapsuleType = FmpCapsuleImageHeader->Decode(buffer, ItemOffset);
            CapsuleTypeList[index] = CapsuleType;
            FmpCapsuleImageHeaderList[index] = FmpCapsuleImageHeader;
        }
    }

    void FmpCapsuleHeaderClass::collectInfo(stringstream& Info)
    {
        Info << hex << setfill('0')
            << "Version =" << setw(8) << Version << endl
            << "EmbeddedDriverCount =" << setw(4) << EmbeddedDriverCount << endl
            << "PayloadItemCount =" << setw(4) << PayloadItemCount << endl;
    }

    UInt16 FmpCapsuleHeaderClass::getPayloadItemCount() const {
        return PayloadItemCount;
    }

    Int64 FmpAuthHeaderClass::Decode(Buffer& buffer, Int64 offset) {
        panelOffset = offset;
        panelSize = 0x20;

        buffer.setOffset(offset);
        Int64 MonotonicCountSize = 8;
        MonotonicCount = buffer.parseUINT64();
        dwLength = buffer.parseUINT32();
        wRevision = buffer.parseUINT16();
        wCertificateType = buffer.parseUINT16();
        CertType = buffer.parseGUID();
        if (dwLength < 24){
            throw CapsuleError("dwLength too small");
        }
        if (wRevision != 0x0200){
            throw CapsuleError("Wrong wRevision");
        }
        if (wCertificateType != 0x0EF1){
            throw CapsuleError("Wrong wCertificateType");
        }
        GUID EFI_CERT_TYPE_PKCS7_GUID{ "4aafd29d-68df-49ee-8aa9-347d375665a7" };
        if (*CertType != EFI_CERT_TYPE_PKCS7_GUID) {
            throw CapsuleError("Wrong FmpAuthHeader");
        }
        return offset + MonotonicCountSize + dwLength;
    }

    void FmpAuthHeaderClass::collectInfo(stringstream& Info)
    {
        Info << hex << setfill('0')
            << "MonotonicCount =" << setw(16) << MonotonicCount << endl
            << "dwLength =" << setw(8) << dwLength << endl
            << "wRevision =" << setw(4) << wRevision << endl
            << "wCertificateType =" << setw(4) << wCertificateType << endl
            << "CertType =" << CertType << endl;
    }

    FmpAuthHeaderClass::~FmpAuthHeaderClass() {
        delete CertType;
    }

    Int64 FmpPayloadHeaderClass::Decode(Buffer& buffer, Int64 offset) {
        panelOffset = offset;
        panelSize = 0x10;

        buffer.setOffset(offset);
        char* bufferSignature;
        char signatureArray[5]{ 0 };

        bufferSignature = (char*)buffer.parseBytes(4);
        HeaderSize = buffer.parseUINT32();
        FwVersion = buffer.parseUINT32();
        LowestSupportedVersion = buffer.parseUINT32();

        for (int i = 0; i < 4; ++i) {
            signatureArray[i] = bufferSignature[i];
        }
        Signature = (char*)signatureArray;
        if (Signature != FMP_PAYLOAD_HEADER_SIGNATURE) {
            throw CapsuleException("No FmpPayload Header!");
        }
        if (HeaderSize < 16){
            throw CapsuleError("HeaderSize too small");
        }
        return buffer.getOffset();
    }

    void FmpPayloadHeaderClass::collectInfo(stringstream& Info)
    {
        Info << hex << setfill('0')
            << "Signature =" << Signature << endl
            << "HeaderSize =" << setw(8) << HeaderSize << endl
            << "FwVersion =" << setw(8) << FwVersion << endl
            << "LowestSupportedVersion =" << setw(8) << LowestSupportedVersion << endl;
    }

    Int64 FirmwareVolumeHeaderClass::Decode(Buffer& buffer, Int64 offset, const string& CapsuleType, bool FV_Exist) {
        buffer.setOffset(offset);
        char* bufferSignature;
        char signatureArray[5]{};
        string fvSignature = "_FVH";
        if (!FV_Exist) {
            return offset + XDRSize + QuadSize;
        }
        if (buffer.getRemainingSize() < XDRSize + FvHeaderSize) {
            throw CapsuleError("Buffer size smaller than header!");
        }
        if (CapsuleType == "uCode") {
            buffer.setOffset(offset + XDRSize);
        }
        panelOffset = offset;
        panelSize = 0x40;

        ZeroVector = buffer.parseBytes(16);
        FileSystemGuid = buffer.parseGUID();
        FvLength = buffer.parseINT64();
        bufferSignature = (char*)buffer.parseBytes(4);
        Attributes = buffer.parseUINT32();
        HeaderLength = buffer.parseUINT16();
        Checksum = buffer.parseUINT16();
        ExtHeaderOffset = buffer.parseUINT16();
        Reserved = buffer.parseUINT8();
        Revision = buffer.parseUINT8();
        NumBlocks = buffer.parseUINT32();
        Length = buffer.parseUINT32();

        for (int i = 0; i < 4; ++i) {
            signatureArray[i] = bufferSignature[i];
        }
        Signature = (char*)signatureArray;
        if (fvSignature != Signature) {
            throw CapsuleException("No FV header!");
        }

        delete[] bufferSignature;
        FvBeginOffset = offset + XDRSize;
        uCodeBeginOffset = offset + XDRSize + 0x1000;   //4K alignment
        buffer.offset += QuadSize;
        return buffer.offset;
    }

    void FirmwareVolumeHeaderClass::collectInfo(stringstream& Info)
    {
        Info << hex << setfill('0')
            << "FileSystemGuid =" << FileSystemGuid << endl
            << "Checksum =" << setw(4) << Checksum << endl
            << "NumBlocks =" << setw(8) << NumBlocks << endl
            << "Length =" << setw(8) << Length << endl;
    }

    Int64 FirmwareVolumeHeaderClass::getFvLength() const
    {
        return FvLength;
    }

    FirmwareVolumeHeaderClass::~FirmwareVolumeHeaderClass() {
        delete[] ZeroVector;
        delete[] FileSystemGuid;
    }

//    Int64 FirmwareVolumeHeaderClass::searchFvWithGuid(Buffer& buffer, Int64 offset, GUID& guid) {
//        Int64 searchOffset = 0;
//        buffer.setOffset(offset);
//        Int64 remainingSize = buffer.getRemainingSize();
//        Int64 _8byte = 0x8;
//        GUID* bufferGuid = buffer.parseGUID();

//        while ((*bufferGuid).GuidData.Data1 != guid.GuidData.Data1) {
//            searchOffset += _8byte;
//            if (searchOffset > remainingSize) {
//                return 0; //todo: can't find FV
//            }
//            buffer.setOffset(offset + searchOffset);
//            bufferGuid = buffer.parseGUID();
//        }
//        return buffer.getOffset() - 16;
//    }

    Int64 FfsFileHeaderClass::Decode(Buffer& buffer, Int64 offset, bool Alignment, Int64 RelativeAddress, Int64 AlignValue) {
        if (Alignment){
            Buffer::Align(offset, RelativeAddress, AlignValue);
        }
        buffer.setOffset(offset);
        if (buffer.getRemainingSize() < FfsHeaderSize) {
            throw CapsuleError("Buffer size smaller than header!");
        }
        panelOffset = offset;
        panelSize = getFfsHeaderSize();

        FfsGuid = buffer.parseGUID();
        IntegrityCheck = buffer.parseUINT16();
        Type = buffer.parseUINT8();
        Attributes = buffer.parseUINT8();
        Size = buffer.parseBytes(3);
        State = buffer.parseUINT8();

        if (((int)Attributes == 0x01) && ((*(int*)Size) & 0x00FFFFFF) == 0) {
            ExtendedSize = buffer.parseINT64(); // EFI_FFS_FILE_HEADER2
            isExtended = true;
        }

        return buffer.offset;
    }

    void FfsFileHeaderClass::collectInfo(stringstream& Info)
    {
        Info << hex << setfill('0')
            << "FfsGuid =" << FfsGuid << endl
            << "IntegrityCheck =" << setw(4) << IntegrityCheck << endl
            << "Type =" << setw(2) << (UInt16)Type << endl
            << "Attributes =" << setw(2) << (UInt16)Attributes << endl;
        if (isExtended)
        {
            Info << hex << setfill('0')
                 << "Size =" << setw(6) << 0x0 << endl
                 << "State =" << setw(2) << (UInt16)State << endl
                 << "ExtendedSize =" << setw(16) << getSize() << endl;
        }
        else
        {
            Info << hex << setfill('0')
                << "Size =" << setw(6) << getSize() << endl
                << "State =" << setw(2) << (UInt16)State << endl;
        }
    }

    string FfsFileHeaderClass::getEntryName()
    {
        string EntryName = "EFI_FFS_FILE_HEADER";
        if (isExtended)
        {
            EntryName = "EFI_FFS_FILE_HEADER2";
        }
        return EntryName;
    }

    FfsFileHeaderClass::~FfsFileHeaderClass() {
        delete FfsGuid;
        delete[] Size;
    }

    Int64 FfsFileHeaderClass::getSize() {
        Int64 ffsSize = 0;
        if (isExtended)
        {
            ffsSize = ExtendedSize;
        }
        else
        {
            ffsSize = *(Int32*)Size;
            ffsSize &= 0xFFFFFF;
        }
        return ffsSize;
    }

    Int64 FfsFileHeaderClass::getFfsHeaderSize()
    {
        if (isExtended)
        {
            return ExtendedFfsHeaderSize;
        }
        return FfsHeaderSize;
    }

    Int64 FfsFileHeaderClass::searchFfsWithGuid(Buffer& buffer, Int64 offset, Int64 length, GUID& ffsGuid, bool Reverse) {
        //only support reverse search!!!
        Int64 _8Byte = 0x08; // Ffs Header is aligned with 8 bytes
        if (Reverse) {
            _8Byte = -0x08;
        }
        Int64 searchOffset = 0;
        buffer.setOffset(offset);
        //Int64 remainingSize = buffer.getRemainingSize();
        Int64 remainingSize = length;
        buffer.setOffset(offset + remainingSize - 0x10);

        UInt32 bufferGuidData1 = buffer.parseUINT32();
        UInt32 ffsGuidData1 = ffsGuid.GuidData.Data1;

        while (bufferGuidData1 != ffsGuidData1) {
            searchOffset += _8Byte;
            if (abs(searchOffset) > remainingSize) {
                throw CapsuleError("Can't find Ffs!");
            }
            buffer.setOffset(offset + remainingSize + searchOffset);
            bufferGuidData1 = buffer.parseUINT32();
        }

        buffer.setOffset(offset + remainingSize + searchOffset);
        GUID* bufferGuid = buffer.parseGUID();
        if (*bufferGuid != ffsGuid) {
            delete bufferGuid;
            throw CapsuleError("Can't find Ffs!");
        }

        delete bufferGuid;
        return buffer.getOffset() + 12; //ffs remaining header and XDR buffer (4 byte)
    }

    Int64 MicrocodeVersionClass::Decode(Buffer& buffer, Int64 offset) {
        panelOffset = offset;
        panelSize = 0x8;

        buffer.setOffset(offset);
        UInt16 Character;
        if (buffer.getRemainingSize() < FfsHeaderSize) {
            throw CapsuleError("Buffer size smaller than header!");
        }
        FwVersion = buffer.parseUINT32();
        LowestSupportedVersion = buffer.parseUINT32();

        Character = buffer.parseUINT16();
        while (Character != 0x0000) {
            panelSize += 2;
            FwVersionString += (char)Character;
            Character = buffer.parseUINT16();
        }
        return buffer.offset;
    }

    void MicrocodeVersionClass::collectInfo(stringstream& Info)
    {
        Info << hex << setfill('0')
            << "FwVersion =" << setw(8) << FwVersion << endl
            << "LowestSupportedVersion =" << setw(8) << LowestSupportedVersion << endl
            << "FwVersionString =" << FwVersionString << endl;
    }

    string MicrocodeVersionClass::getFwVersion() const {
        stringstream fmt;
        fmt << hex << setfill('0') << setw(8) << FwVersion;
        return fmt.str();
    }

    string MicrocodeVersionClass::getLSV() const {
        stringstream fmt;
        fmt << hex << setfill('0') << setw(8) << LowestSupportedVersion;
        return fmt.str();
    }

    string MicrocodeVersionClass::getFwVersionString() const {
        return FwVersionString;
    }

    Int64 CPUMicrocodeHeaderClass::Decode(Buffer& buffer, Int64 offset) {
        buffer.setOffset(offset);
        if (buffer.getRemainingSize() < CpuMicrocodeHeaderSize) {
            throw CapsuleError("Buffer size smaller than header!");
        }

        HeaderVersion = buffer.parseUINT32();
        UpdateRevision = buffer.parseUINT32();
        Date = buffer.parseUINT32();
        ProcessorSignature = buffer.parseUINT32();
        Checksum = buffer.parseUINT32();
        LoaderRevision = buffer.parseUINT32();
        ProcessorFlags = buffer.parseUINT32();
        DataSize = buffer.parseINT32();
        TotalSize = buffer.parseINT32();

        panelOffset = offset;
        panelSize = TotalSize;

        if (HeaderVersion != 0x1)
        {
            isEmpty = true;
        }

        return buffer.offset;
    }

    void CPUMicrocodeHeaderClass::collectInfo(stringstream& Info)
    {
        Info << hex << setfill('0')
            << "CPU ID = " << setw(8) << ProcessorSignature << endl
            << "HeaderVersion = " << setw(8) << HeaderVersion << endl
            << "UpdateRevision = " << setw(8) << UpdateRevision << endl
            << "Date = " << setw(8) << Date << endl
            << "ProcessorSignature = " << setw(8) << ProcessorSignature << endl
            << "Checksum = " << setw(8) << Checksum << endl
            << "LoaderRevision = " << setw(8) << LoaderRevision << endl
            << "ProcessorFlags = " << setw(8) << ProcessorFlags << endl
            << "DataSize = " << setw(8) << DataSize << endl
            << "TotalSize = " << setw(8) << TotalSize << endl;
    }

    string CPUMicrocodeHeaderClass::getUcodeDate() const
    {
        stringstream Info;
        Info << hex << "Date = " << (Date & 0xFFFF) << "." << ((Date & 0xFF000000) >> 24) << "." << ((Date & 0xFF0000) >> 16) << endl;
        return Info.str();
    }

    string CPUMicrocodeHeaderClass::getUcodeRevision() const {
        stringstream fmt;
        fmt << hex << UpdateRevision;
        return fmt.str();
    }

    string CPUMicrocodeHeaderClass::getUcodeSignature() const {
        stringstream fmt;
        fmt << hex << ProcessorSignature;
        return fmt.str();
    }

    vector<Int64> CPUMicrocodeHeaderClass::SearchMicrocodeEntryNum(Buffer& buffer, Int64 MicrocodeOffset, Int64 MicrocodeDataSize)
    {
        Int64 searchOffset = 0;
        UInt32 HeaderVersion;
        vector<Int64> MicrocodeEntryList;

        while (searchOffset < MicrocodeDataSize)
        {
            buffer.setOffset(MicrocodeOffset + searchOffset);
            HeaderVersion = buffer.parseUINT32();
            if (HeaderVersion == 0x1)
            {
                MicrocodeEntryList.push_back(MicrocodeOffset + searchOffset);
            }
            searchOffset += 0x1000;
        }
        return MicrocodeEntryList;
    }

    Int64 ConfigIniClass::Decode(Buffer& buffer, Int64 offset, int contextLength, string&& ConfigName)
    {
        panelOffset = offset;
        panelSize = contextLength;

        buffer.setOffset(offset);
        char* Context = (char*)buffer.parseString(contextLength);
        iniContext = Context;
        ConfigFileName = ConfigName;

        delete[] Context;
        return buffer.offset;
    }

    void ConfigIniClass::collectInfo(stringstream& Info)
    {
        Info << iniContext;
    }

    bool BgupHeaderClass::SearchBgup(Buffer& buffer, Int64 BgupOffset) {
        Int64 BufferSize = buffer.getBufferSize();
        if (BgupOffset > BufferSize)
        {
            throw CapsuleError("Buffer size smaller than FV header! Binary corrupted!");
        }
        else if (BgupOffset == BufferSize)
        {
            return false;
        }
        buffer.setOffset(BgupOffset + XDRSize);
        Version = buffer.parseUINT16();
        if (Version == 0x0002) {
            return true;
        }
        return false;
    }

    void BgupHeaderClass::Decode(Buffer& buffer, Int64 offset)
    {
        panelOffset = offset;
        panelSize = 0x22;

        buffer.setOffset(offset);
        Version = buffer.parseUINT16();
        Reserved3 = buffer.parseUINT16();
        char* cPlatId = (char*)buffer.parseBytes(16);
        PkgAttributes = buffer.parseUINT16();
        Reserved4 = buffer.parseUINT16();
        PslMajorVer = buffer.parseUINT16();
        PslMinorVer = buffer.parseUINT16();
        ScriptSectionSize = buffer.parseUINT32();
        DataSectionSize = buffer.parseUINT32();
        BiosSvn = buffer.parseUINT32();
        EcSvn = buffer.parseUINT32();
        VendorSpecific = buffer.parseUINT32();

        char arrayPlatId[17]{ 0 };
        for (int i = 0; i < 16; i++) {
            arrayPlatId[i] = cPlatId[i];
        }
        PlatId = (char*)arrayPlatId;
        delete[] cPlatId;
    }

    string BgupHeaderClass::getPlatId() const
    {
        return PlatId;
    }

    void BgupHeaderClass::collectInfo(stringstream& Info)
    {
        Info << hex << setfill('0')
            << "Version = " << setw(4) << Version << endl
            << "PlatId = " << PlatId << endl
            << "PkgAttributes = " << setw(4) << PkgAttributes << endl
            << "PslMajorVer = " << setw(4) << PslMajorVer << endl
            << "PslMinorVer = " << setw(4) << PslMajorVer << endl
            << "ScriptSectionSize = " << setw(8) << PslMajorVer << endl
            << "DataSectionSize = " << setw(8) << DataSectionSize << endl
            << "BiosSvn = " << setw(8) << BiosSvn << endl
            << "EcSvn = " << setw(8) << EcSvn << endl
            << "VendorSpecific = " << setw(8) << VendorSpecific << endl;
    }

    void BiosIdClass::Decode(Buffer& buffer, Int64 offset, Int64 length) {
        Int64 PayloadOffset;
        char signatureArray[9]{ 0 };
        GUID gBiosIdGuid{ "C3E36D09-8294-4b97-A857-D5288FE33E28" };
        PayloadOffset = FfsFileHeaderClass::searchFfsWithGuid(buffer, offset, length, gBiosIdGuid, true);
        buffer.setOffset(PayloadOffset);

        char* bufferSignature = (char*)buffer.parseBytes(8);
        for (int i = 0; i < 8; ++i) {
            signatureArray[i] = bufferSignature[i];
        }

        Signature = (char*)signatureArray;
        if (Signature != "$IBIOSI$") {
            throw CapsuleError("Bios ID not found!");
        }

        BIOS_ID_STRING = "";
        auto* BIOS_ID = (UInt16*)buffer.parseBytes(66);
        for (int i = 0; i < 32; ++i) {
            BIOS_ID_STRING += (char)BIOS_ID[i];
        }
        delete[] bufferSignature;
        delete[] BIOS_ID;
    }

    string BiosIdClass::getBiosIDString() const {
        return BIOS_ID_STRING;
    }

    void BiosIdClass::collectInfo(stringstream& Info)
    {
        Info << "BIOS ID : " << BIOS_ID_STRING << endl;
    }

    void FitTableClass::Decode(Buffer& buffer, Int64 offset)
    {
        buffer.setOffset(offset);
        Address  = buffer.parseINT64();
        Size     = (char*)buffer.parseBytes(3);
        Rsvd     = buffer.parseUINT8();
        Version  = buffer.parseUINT16();
        Type     = buffer.parseUINT8() & 0x7F;
        Checksum = buffer.parseUINT8();
    }

    UInt8 FitTableClass::getType() const
    {
        return Type;
    }

    Int64 FitTableClass::getAddress() const
    {
        return Address & 0xFFFFFF;
    }

    FitTableClass::~FitTableClass()
    {
        delete [] Size;
    }

    void BiosClass::Decode(Buffer& buffer, Int64 offset, Int64 length)
    {
        const Int64 BiosFullSize = 0x1000000;
        const Int64 IbbSize      = 0x400000;
        Int64  FitPointerAddress = offset + length - 0x40;
        panelOffset = offset;
        panelSize = length;

        // Detect Resiliency BIOS
        buffer.setOffset(offset + length - 0x10);
        GUID *ResetVecorIBB = buffer.parseGUID();
        buffer.setOffset(offset + length - IbbSize - 0x10);
        GUID *ResetVecorIBBR = buffer.parseGUID();
        if (*ResetVecorIBB == *ResetVecorIBBR) {
            ResiliencyFlag = true;
        }
        delete ResetVecorIBB;
        delete ResetVecorIBBR;

        buffer.setOffset(FitPointerAddress);
        Int64 FitTableAddress = buffer.parseINT64() & 0xFFFFFF;
        FitTableAddress = Buffer::adjustBufferAddress(BiosFullSize, FitTableAddress, length); // get the relative address of FIT table
        buffer.setOffset(offset + FitTableAddress);
        string FitSignature = buffer.parseString(5);
        uCodeEntryList.clear();
        cout << "FitTableAddress = 0x" << hex << offset + FitTableAddress << endl;
        if (FitSignature == "_FIT_")
        {
            buffer.setOffset(offset + FitTableAddress + 8);
            Int32 FitEntryNum = (*(Int32*)buffer.parseBytes(3)) & 0xFFFFFF;
            for (int i = 0; i < FitEntryNum - 1; i++)
            {
                Int64 EntryOffset = offset + FitTableAddress + 0x10 * (i + 1);
                FitTableClass FitTable {};
                FitTable.Decode(buffer, EntryOffset);
                bool isUcode = (FitTable.getType() == 0x1);
                bool isAcm = (FitTable.getType() == 0x2);
                if (isUcode)
                {
                    CPUMicrocodeHeaderClass uCodeEntry{};
                    Int64 uCodeAddress = Buffer::adjustBufferAddress(BiosFullSize, FitTable.getAddress(), length) + offset;
                    uCodeEntry.Decode(buffer, uCodeAddress);
                    uCodeEntryList.push_back(uCodeEntry);
                }
                if (isAcm)
                {
                    Int64 AcmAddress = Buffer::adjustBufferAddress(BiosFullSize, FitTable.getAddress(), length) + offset;
                    Acm.Decode(buffer, AcmAddress, 0x3E800);
                }
            }
        }

        BiosId.Decode(buffer, offset, length);
    }

    string BiosClass::getBiosIDString() const
    {
        string BiosIdString{ BiosId.getBiosIDString() };
        return BiosIdString;
    }

    void BiosClass::collectInfo(stringstream& Info)
    {
        BiosId.collectInfo(Info);
        Info << "\n";

        Int64  uCodeEntryNum = uCodeEntryList.size();
        for (int i = 0; i < uCodeEntryNum; i++)
        {
            if (uCodeEntryList[i].isEmpty)
            {
                continue;
            }
            Info << "uCode Version = " << uCodeEntryList[i].getUcodeRevision()
                 << " CPU ID = " << uCodeEntryList[i].getUcodeSignature()
                 << " uCode Size = 0x" << hex << uCodeEntryList[i].TotalSize << "\n";
        }

        Info << "\n";
        Acm.collectInfo(Info);
    }

    void AcmClass::Decode(Buffer& buffer, Int64 offset, Int64 DataLength)
    {
        Int64 searchOffset;
        bool BtGAcmHeaderFound = false;
        GUID BtGAcmHeader { "7fc03aaa-46a7-18db-2eac-698f8d417f5a" }; //This is not GUID, only for search convenience

        buffer.setOffset(offset);
        if (buffer.getRemainingSize() < DataLength) {
            throw CapsuleError("Buffer size smaller than Ffs data!");
        }

        for (searchOffset = 0; searchOffset < DataLength; searchOffset += 0x10)
        {
            buffer.setOffset(offset + searchOffset);
            GUID* temp = buffer.parseGUID();
            if (*temp == BtGAcmHeader)
            {
                delete temp;
                BtGAcmHeaderFound = true;
                break;
            }
            delete temp;
        }
        if (!BtGAcmHeaderFound)
        {
            throw CapsuleError("BtGAcmHeader not Found!");
        }

        panelOffset = offset;
        panelSize = DataLength;

        buffer.setOffset(offset + searchOffset + 0x25);     // 0x25 is from line 60 in Intel\AlderLakePlatSamplePkg\Features\CapsuleUpdate\Tools\GetFwVersionFromBin\GetBtGAcmIdFromBin.py
        VERSION_NUMBER = buffer.parseUINT8();
        VERSION_MAJOR = buffer.parseUINT8();
        VERSION_MINOR = buffer.parseUINT8();
    }

    void AcmClass::collectInfo(stringstream& Info)
    {
        Info << "ACM version: " << hex << setfill('0') << setw(2)
             << (UInt16)VERSION_NUMBER << "."
             << setfill('0') << setw(2)
             << (UInt16)VERSION_MAJOR << "."
             << setfill('0') << setw(2)
             << (UInt16)VERSION_MINOR
             << endl;
    }

    string AcmClass::getAcmVersion()
    {
        stringstream Info;
        collectInfo(Info);
        return Info.str();
    }

    void EcClass::Decode(Buffer& buffer, Int64 offset, Int64 DataLength)
    {
        string searchValue;
        Int64 searchOffset = 0;
        panelOffset = offset;
        panelSize = DataLength;

        for (searchOffset = 0; searchOffset < DataLength; searchOffset += 2)
        {
            buffer.setOffset(offset + searchOffset);
            searchValue = buffer.parseString(4);
            if (searchValue == EcImgSignature)
            {
                Signature = searchValue;
                PlatId = buffer.parseUINT8();
                MajorVer = buffer.parseUINT8();
                MinorVer = buffer.parseUINT8();
                BuildVer = buffer.parseUINT8();
                return;
            }
        }
        throw CapsuleError("EC header not Found!");
    }

    string EcClass::getEcVersion() const
    {
        string EcVersion = to_string(MajorVer) + "." + to_string(MinorVer);
        return EcVersion;
    }

    void EcClass::collectInfo(stringstream& Info)
    {
        Info << "Signature = " << Signature << endl
             << "MajorVer = " << (Int16)MajorVer << endl
             << "MinorVer = " << (Int16)MinorVer << endl
             << "BuildVer = " << (Int16)BuildVer << endl;
    }

    void MeClass::Decode(Buffer& buffer, Int64 offset, Int64 DataLength)
    {
        panelOffset = offset;
        panelSize = DataLength;
    }

    void MeClass::collectInfo(stringstream& Info)
    {
        Info << "Unsupported" << endl;
    }

} // CapsuleTool
