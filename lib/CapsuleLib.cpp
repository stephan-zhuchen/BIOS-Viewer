//
// Created by Stephan on 2022/6/14.
//

#include <iomanip>
#include <vector>
#include "CapsuleLib.h"
#include "uefilib.h"
#include "UEFI/GuidDefinition.h"

using namespace std;

namespace CapsuleToolSpace {

    CapsuleException::CapsuleException() : message("Error.") {}

    CapsuleException::CapsuleException(const string& str) : message("Error : " + str) {}

    const char* CapsuleException::what() const noexcept {
        return message.c_str();
    }

    CapsuleError::CapsuleError() : message("Error.") {}

    CapsuleError::CapsuleError(const string& str) : message(str) {}

    const char* CapsuleError::what() const noexcept {
        return message.c_str();
    }

    INT64 EntryHeaderClass::panelGetOffset() {
        return panelOffset;
    }

    INT64 EntryHeaderClass::panelGetSize() {
        return panelSize;
    }

    EntryHeaderClass::~EntryHeaderClass() {
    }

    void CapsuleOverviewClass::Decode(Buffer& buffer, INT64 offset, INT64 length) {
        panelOffset = offset;
        panelSize = length;
    }

    void CapsuleOverviewClass::collectInfo(stringstream& Info) {
        Info << OverviewMsg;
    }

    void CapsuleOverviewClass::setOverviewMsg(string msg) {
        OverviewMsg = msg;
    }

    UefiCapsuleHeaderClass::~UefiCapsuleHeaderClass() {
    }

    INT64 UefiCapsuleHeaderClass::Decode(Buffer& buffer, INT64 offset) {
        panelOffset = offset;
        panelSize = 0x20;

        buffer.setOffset(offset);
        CapsuleGuid = buffer.getGUID();
        HeaderSize = buffer.getUINT32();
        Flags = buffer.getUINT32();
        CapsuleImageSize = buffer.getUINT32();
        Reserved = buffer.getUINT32();

        const UINT32 CAPSULE_FLAGS_PERSIST_ACROSS_RESET   = 0x00010000;
        const UINT32 CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE  = 0x00020000;
        const UINT32 CAPSULE_FLAGS_INITIATE_RESET         = 0x00040000;

        PersistAcrossReset  = (Flags & CAPSULE_FLAGS_PERSIST_ACROSS_RESET) != 0;
        PopulateSystemTable = (Flags & CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE) != 0;
        InitiateReset       = (Flags & CAPSULE_FLAGS_INITIATE_RESET) != 0;

        GUID EFI_FIRMWARE_MANAGEMENT_CAPSULE_ID_GUID{ "6DCBD5ED-E82D-4C44-BDA1-7194199AD92A" };
        if (CapsuleGuid != GuidDatabase::gEfiFmpCapsuleGuid) {
            cout << CapsuleGuid << endl;
            throw CapsuleError("No Capsule!");
        }
        return buffer.getOffset();
    }

    void UefiCapsuleHeaderClass::collectInfo(stringstream& Info) {
        INT32 width = 20;
        Info.setf(ios::left);

        Info << setw(width) << "CapsuleGuid:"        << GUID(CapsuleGuid).str(true) << "\n"
             << setw(width) << "HeaderSize:"         << hex << uppercase << HeaderSize << "h\n"
             << setw(width) << "Flags:"              << hex << uppercase << Flags << "h\n"
             << setw(width) << "CapsuleImageSize:"   << hex << uppercase << CapsuleImageSize << "h\n";

        Info << "\nFlag:\n";
        if (PersistAcrossReset) {Info << "PersistAcrossReset\n";}
        if (PopulateSystemTable) {Info << "PopulateSystemTable\n";}
        if (InitiateReset) {Info << "InitiateReset\n";}
    }

    string FmpCapsuleImageHeaderClass::getCapsuleTypeFromGuid(EFI_GUID& guid) const {
        if (guid == GuidDatabase::gFmpDeviceMonolithicDefaultGuid) {
            return "Monolithic";
        }
        else if (guid == GuidDatabase::gFmpDeviceBiosDefaultGuid) {
            return "BIOS";
        }
        else if (guid == GuidDatabase::gFmpDeviceExtendedBiosDefaultGuid) {
            return "Extended BIOS";
        }
        else if (guid == GuidDatabase::gFmpDeviceIfwiDefaultGuid) {
            return "IFWI";
        }
        else if (guid == GuidDatabase::gFmpDeviceBtGAcmDefaultGuid) {
            return "BtgAcm";
        }
        else if (guid == GuidDatabase::gFmpDeviceMicrocodeDefaultGuid) {
            return "uCode";
        }
        else if (guid == GuidDatabase::gFmpDeviceMeDefaultGuid) {
            return "ME";
        }
        else if (guid == GuidDatabase::gFmpDeviceEcDefaultGuid) {
            return "EC";
        }
        else if (guid == GuidDatabase::gFmpDevicePlatformRetimerGuid) {
            return "Retimer";
        }
        else if (guid == GuidDatabase::gFmpDeviceIshPdtDefaultGuid) {
            return "IshPdt";
        }
        else if (guid == GuidDatabase::gFmpDeviceMeFwAdlLpConsGuid) {
            return "ME Lp_Cons";
        }
        else if (guid == GuidDatabase::gFmpDeviceMeFwAdlHConsGuid) {
            return "ME H_Cons";
        }
        else if (guid == GuidDatabase::gFmpDeviceMeFwAdlLpCorpGuid) {
            return "ME Lp_Corp";
        }
        else if (guid == GuidDatabase::gFmpDeviceMeFwAdlHCorpGuid) {
            return "ME H_Corp";
        }
        else {
            return "Unknown";
        }
    }

    FmpCapsuleImageHeaderClass::FmpCapsuleImageHeaderClass() {
    }

    FmpCapsuleImageHeaderClass::~FmpCapsuleImageHeaderClass() {
        delete reserved_bytes;
    }

    string FmpCapsuleImageHeaderClass::Decode(Buffer& buffer, INT64 offset) {
        panelOffset = offset;
        panelSize = 0x20;

        buffer.setOffset(offset);
        Version = buffer.getUINT32();
        UpdateImageTypeId = buffer.getGUID();
        UpdateImageIndex = buffer.getUINT8();
        reserved_bytes = buffer.getBytes(3);
        UpdateImageSize = buffer.getUINT32();
        UpdateVendorCodeSize = buffer.getUINT32();
        if (Version >= 2) {
            panelSize += 0x8;
            UpdateHardwareInstance = buffer.getUINT64();
        }
        if (Version >= 3) {
            panelSize += 0x8;
            ImageCapsuleSupport = buffer.getUINT64();
        }
        PayloadOffset = buffer.getOffset();
        CapsuleType = getCapsuleTypeFromGuid(UpdateImageTypeId);
        return CapsuleType;
    }

    void FmpCapsuleImageHeaderClass::collectInfo(stringstream& Info)
    {
        INT32 width = 24;
        Info.setf(ios::left);

        Info << setw(width) << "Version:"              << hex << uppercase << Version << "h\n"
             << setw(width) << "UpdateImageTypeId:"    << GUID(UpdateImageTypeId).str(true) << "\n"
             << setw(width) << "UpdateImageIndex:"     << hex << uppercase << (UINT32)UpdateImageIndex << "h\n"
             << setw(width) << "UpdateImageSize:"      << hex << uppercase << UpdateImageSize << "h\n"
             << setw(width) << "UpdateVendorCodeSize:" << hex << uppercase << UpdateVendorCodeSize << "h\n";

        if (Version >= 2) {
            Info << setw(width) << "UpdateHardwareInstance:" << hex << uppercase << UpdateHardwareInstance << "h\n";
        }
        if (Version >= 3) {
            Info << setw(width) << "ImageCapsuleSupport:" << hex << uppercase << ImageCapsuleSupport << "h\n";
        }
    }

    void FmpCapsuleHeaderClass::Decode(Buffer& buffer, INT64 offset) {
        panelOffset = offset;
        panelSize = 8;

        buffer.setOffset(offset);
        INT64 ItemOffset;
        Version = buffer.getUINT32();
        EmbeddedDriverCount = buffer.getUINT16();
        PayloadItemCount = buffer.getUINT16();
        //assert: Version >= 0x01
        ItemOffsetList = vector<INT64>(EmbeddedDriverCount + PayloadItemCount, 0);
        CapsuleTypeList = vector<string>(PayloadItemCount, "");

        for (int index = 0; index < PayloadItemCount; index++) {
            FmpCapsuleImageHeaderList.push_back(nullptr);
        }

        for (int index = 0; index < EmbeddedDriverCount + PayloadItemCount; index++) {
            ItemOffset = buffer.getINT64();
            //assert: ItemOffset <= (UINT64)Buffer.buffer.Length
            ItemOffsetList[index] = ItemOffset + 0x20;
        }

        for (int index = EmbeddedDriverCount; index < EmbeddedDriverCount + PayloadItemCount; index++) {
            ItemOffset = ItemOffsetList[index];
            auto FmpCapsuleImageHeader = make_shared<FmpCapsuleImageHeaderClass>();
            CapsuleType = FmpCapsuleImageHeader->Decode(buffer, ItemOffset);
            CapsuleTypeList[index - EmbeddedDriverCount] = CapsuleType;
            FmpCapsuleImageHeaderList[index - EmbeddedDriverCount] = FmpCapsuleImageHeader;
        }
    }

    void FmpCapsuleHeaderClass::collectInfo(stringstream& Info)
    {
        INT32 width = 25;
        Info.setf(ios::left);

        Info << setw(width) << "Version:"              << hex << uppercase << Version << "h\n"
             << setw(width) << "EmbeddedDriverCount:"  << hex << uppercase << EmbeddedDriverCount << "h\n"
             << setw(width) << "PayloadItemCount:"     << hex << uppercase << PayloadItemCount << "h\n";
    }

    UINT16 FmpCapsuleHeaderClass::getPayloadItemCount() const {
        return PayloadItemCount;
    }

    UINT16 FmpCapsuleHeaderClass::getDriverItemCount() const {
        return EmbeddedDriverCount;
    }

    INT64 FmpAuthHeaderClass::Decode(Buffer& buffer, INT64 offset) {
        panelOffset = offset;
        panelSize = 0x20;

        buffer.setOffset(offset);
        INT64 MonotonicCountSize = 8;
        MonotonicCount = buffer.getUINT64();
        dwLength = buffer.getUINT32();
        wRevision = buffer.getUINT16();
        wCertificateType = buffer.getUINT16();
        CertType = buffer.getGUID();
        if (dwLength < 24){
            throw CapsuleError("dwLength too small");
        }
        if (wRevision != 0x0200){
            throw CapsuleError("Wrong wRevision");
        }
        if (wCertificateType != 0x0EF1){
            throw CapsuleError("Wrong wCertificateType");
        }
        if (CertType != GuidDatabase::gEfiCertPkcs7Guid) {
            throw CapsuleError("Wrong FmpAuthHeader");
        }
        return offset + MonotonicCountSize + dwLength;
    }

    void FmpAuthHeaderClass::collectInfo(stringstream& Info)
    {
        INT32 width = 20;
        Info.setf(ios::left);

        Info << setw(width) << "MonotonicCount:"     << hex << uppercase << MonotonicCount << "h\n"
             << setw(width) << "dwLength:"           << hex << uppercase << dwLength << "h\n"
             << setw(width) << "wRevision:"          << hex << uppercase << wRevision << "h\n"
             << setw(width) << "wCertificateType:"   << hex << uppercase << wCertificateType << "h\n"
             << setw(width) << "CertType:"           << hex << uppercase << CertType << "h\n";
    }

    FmpAuthHeaderClass::~FmpAuthHeaderClass() {
    }

    INT64 FmpPayloadHeaderClass::Decode(Buffer& buffer, INT64 offset) {
        panelOffset = offset;
        panelSize = 0x10;

        buffer.setOffset(offset);
        char* bufferSignature;
        char signatureArray[5]{ 0 };

        bufferSignature = (char*)buffer.getBytes(4);
        HeaderSize = buffer.getUINT32();
        FwVersion = buffer.getUINT32();
        LowestSupportedVersion = buffer.getUINT32();

        for (int i = 0; i < 4; ++i) {
            signatureArray[i] = bufferSignature[i];
        }
        Signature = (char*)signatureArray;
        delete[] bufferSignature;
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
        INT32 width = 25;
        Info.setf(ios::left);

        Info << setw(width) << "Signature:"              << Signature << "\n"
             << setw(width) << "HeaderSize:"             << hex << uppercase << HeaderSize << "h\n"
             << setw(width) << "FwVersion:"              << hex << uppercase << FwVersion << "h\n"
             << setw(width) << "LowestSupportedVersion:" << hex << uppercase << LowestSupportedVersion << "h\n";
    }

    INT64 FirmwareVolumeHeaderClass::Decode(Buffer& buffer, INT64 offset, const string& CapsuleType, bool FV_Exist) {
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

        ZeroVector = buffer.getBytes(16);
        FileSystemGuid = buffer.getGUID();
        FvLength = buffer.getINT64();
        bufferSignature = (char*)buffer.getBytes(4);
        Attributes = buffer.getUINT32();
        HeaderLength = buffer.getUINT16();
        Checksum = buffer.getUINT16();
        ExtHeaderOffset = buffer.getUINT16();
        Reserved = buffer.getUINT8();
        Revision = buffer.getUINT8();
        NumBlocks = buffer.getUINT32();
        Length = buffer.getUINT32();

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
        INT32 width = 25;
        Info.setf(ios::left);

        Info << setw(width) << "FileSystemGuid:" << GUID(FileSystemGuid).str(true) << "\n"
             << setw(width) << "Checksum:"       << hex << uppercase << Checksum << "h\n"
             << setw(width) << "NumBlocks:"      << hex << uppercase << NumBlocks << "h\n"
             << setw(width) << "Length:"         << hex << uppercase << Length << "h\n";
    }

    INT64 FirmwareVolumeHeaderClass::getFvLength() const
    {
        return FvLength;
    }

    FirmwareVolumeHeaderClass::~FirmwareVolumeHeaderClass() {
        delete[] ZeroVector;
    }

//    INT64 FirmwareVolumeHeaderClass::searchFvWithGuid(Buffer& buffer, INT64 offset, GUID& guid) {
//        INT64 searchOffset = 0;
//        buffer.setOffset(offset);
//        INT64 remainingSize = buffer.getRemainingSize();
//        INT64 _8byte = 0x8;
//        GUID* bufferGuid = buffer.getGUID();

//        while ((*bufferGuid).GuidData.Data1 != guid.GuidData.Data1) {
//            searchOffset += _8byte;
//            if (searchOffset > remainingSize) {
//                return 0; //todo: can't find FV
//            }
//            buffer.setOffset(offset + searchOffset);
//            bufferGuid = buffer.getGUID();
//        }
//        return buffer.getOffset() - 16;
//    }

    INT64 FfsFileHeaderClass::Decode(Buffer& buffer, INT64 offset, bool Alignment, INT64 RelativeAddress, INT64 AlignValue) {
        if (Alignment){
            Align(offset, RelativeAddress, AlignValue);
        }
        buffer.setOffset(offset);
        if (buffer.getRemainingSize() < FfsHeaderSize) {
            throw CapsuleError("Buffer size smaller than header!");
        }
        panelOffset = offset;
        panelSize = getFfsHeaderSize();

        FfsGuid = buffer.getGUID();
        IntegrityCheck = buffer.getUINT16();
        Type = buffer.getUINT8();
        Attributes = buffer.getUINT8();
        Size = buffer.getBytes(3);
        State = buffer.getUINT8();

        if (((int)Attributes == 0x01) && ((*(int*)Size) & 0x00FFFFFF) == 0) {
            ExtendedSize = buffer.getINT64(); // EFI_FFS_FILE_HEADER2
            isExtended = true;
        }

        return buffer.offset;
    }

    void FfsFileHeaderClass::collectInfo(stringstream& Info)
    {
        Info << hex << setfill('0')
            << "FfsGuid =" << FfsGuid << endl
            << "IntegrityCheck =" << setw(4) << IntegrityCheck << endl
            << "Type =" << setw(2) << (UINT16)Type << endl
            << "Attributes =" << setw(2) << (UINT16)Attributes << endl;
        if (isExtended)
        {
            Info << hex << setfill('0')
                 << "Size =" << setw(6) << 0x0 << endl
                 << "State =" << setw(2) << (UINT16)State << endl
                 << "ExtendedSize =" << setw(16) << getSize() << endl;
        }
        else
        {
            Info << hex << setfill('0')
                << "Size =" << setw(6) << getSize() << endl
                << "State =" << setw(2) << (UINT16)State << endl;
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
        delete[] Size;
    }

    INT64 FfsFileHeaderClass::getSize() {
        INT64 ffsSize = 0;
        if (isExtended)
        {
            ffsSize = ExtendedSize;
        }
        else
        {
            ffsSize = *(INT32*)Size;
            ffsSize &= 0xFFFFFF;
        }
        return ffsSize;
    }

    INT64 FfsFileHeaderClass::getFfsHeaderSize()
    {
        if (isExtended)
        {
            return ExtendedFfsHeaderSize;
        }
        return FfsHeaderSize;
    }

    INT64 FfsFileHeaderClass::searchFfsWithGuid(Buffer& buffer, INT64 offset, INT64 length, GUID& ffsGuid, bool Reverse) {
        //only support reverse search!!!
        INT64 _8Byte = 0x08; // Ffs Header is aligned with 8 bytes
        if (Reverse) {
            _8Byte = -0x08;
        }
        INT64 searchOffset = 0;
        buffer.setOffset(offset);
        //INT64 remainingSize = buffer.getRemainingSize();
        INT64 remainingSize = length;
        buffer.setOffset(offset + remainingSize - 0x10);

        UINT32 bufferGuidData1 = buffer.getUINT32();
        UINT32 ffsGuidData1 = ffsGuid.GuidData.Data1;

        while (bufferGuidData1 != ffsGuidData1) {
            searchOffset += _8Byte;
            if (abs(searchOffset) > remainingSize) {
                throw CapsuleError("Can't find Ffs!");
            }
            buffer.setOffset(offset + remainingSize + searchOffset);
            bufferGuidData1 = buffer.getUINT32();
        }

        buffer.setOffset(offset + remainingSize + searchOffset);
        GUID bufferGuid = GUID(buffer.getGUID());
        if (bufferGuid != ffsGuid) {
            throw CapsuleError("Can't find Ffs!");
        }

        return buffer.getOffset() + 12; //ffs remaining header and XDR buffer (4 byte)
    }

    INT64 MicrocodeVersionClass::Decode(Buffer& buffer, INT64 offset) {
        panelOffset = offset;
        panelSize = 0x8;

        buffer.setOffset(offset);
        UINT16 Character;
        if (buffer.getRemainingSize() < FfsHeaderSize) {
            throw CapsuleError("Buffer size smaller than header!");
        }
        FwVersion = buffer.getUINT32();
        LowestSupportedVersion = buffer.getUINT32();

        Character = buffer.getUINT16();
        while (Character != 0x0000) {
            panelSize += 2;
            FwVersionString += (char)Character;
            Character = buffer.getUINT16();
        }
        return buffer.offset;
    }

    void MicrocodeVersionClass::collectInfo(stringstream& Info)
    {
        INT32 width = 25;
        Info.setf(ios::left);

        Info << setw(width) << "FwVersion:"              << hex << uppercase << FwVersion << "h\n"
             << setw(width) << "LowestSupportedVersion:" << hex << uppercase << LowestSupportedVersion << "h\n"
             << setw(width) << "FwVersionString:"        << FwVersionString << "\n";
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

    INT64 CPUMicrocodeHeaderClass::Decode(Buffer& buffer, INT64 offset) {
        buffer.setOffset(offset);
        if (buffer.getRemainingSize() < CpuMicrocodeHeaderSize) {
            throw CapsuleError("Buffer size smaller than header!");
        }

        HeaderVersion = buffer.getUINT32();
        UpdateRevision = buffer.getUINT32();
        Date = buffer.getUINT32();
        ProcessorSignature = buffer.getUINT32();
        Checksum = buffer.getUINT32();
        LoaderRevision = buffer.getUINT32();
        ProcessorFlags = buffer.getUINT32();
        DataSize = buffer.getINT32();
        TotalSize = buffer.getINT32();

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
        INT32 width = 20;
        Info.setf(ios::left);

        Info << setw(width) << "CPU ID:"             << hex << uppercase << ProcessorSignature << "\n"
             << setw(width) << "HeaderVersion:"      << hex << uppercase << HeaderVersion << "h\n"
             << setw(width) << "UpdateRevision:"     << hex << uppercase << UpdateRevision << "h\n"
             << setw(width) << "Date:"               << hex << uppercase << Date << "\n"
             << setw(width) << "ProcessorSignature:" << hex << uppercase << ProcessorSignature << "h\n"
             << setw(width) << "Checksum:"           << hex << uppercase << Checksum << "h\n"
             << setw(width) << "LoaderRevision:"     << hex << uppercase << LoaderRevision << "h\n"
             << setw(width) << "ProcessorFlags:"     << hex << uppercase << ProcessorFlags << "h\n"
             << setw(width) << "DataSize:"           << hex << uppercase << DataSize << "h\n"
             << setw(width) << "TotalSize:"          << hex << uppercase << TotalSize << "h\n";
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

    vector<INT64> CPUMicrocodeHeaderClass::SearchMicrocodeEntryNum(Buffer& buffer, INT64 MicrocodeOffset, INT64 MicrocodeDataSize)
    {
        INT64 searchOffset = 0;
        UINT32 HeaderVersion;
        vector<INT64> MicrocodeEntryList;

        while (searchOffset < MicrocodeDataSize)
        {
            buffer.setOffset(MicrocodeOffset + searchOffset);
            HeaderVersion = buffer.getUINT32();
            if (HeaderVersion == 0x1)
            {
                MicrocodeEntryList.push_back(MicrocodeOffset + searchOffset);
            }
            searchOffset += 0x1000;
        }
        return MicrocodeEntryList;
    }

    INT64 ConfigIniClass::Decode(Buffer& buffer, INT64 offset, int contextLength, string&& ConfigName)
    {
        panelOffset = offset;
        panelSize = contextLength;

        buffer.setOffset(offset);
        iniContext = buffer.getString(contextLength);
        ConfigFileName = ConfigName;

        std::stringstream ss(iniContext);
        std::string currentSection;

        std::string line;
        while (std::getline(ss, line))
        {
            if (!line.empty() && line[line.length() - 1] == '\r') {
                line = line.substr(0, line.length() - 1);
            }

            if (line[0] == '[' && line[line.size() - 1] == ']') {
                currentSection = line.substr(1, line.size() - 2);
                continue;
            }

            if (currentSection.empty())
                continue;

            std::size_t sepPos = line.find('=');
            if (sepPos != std::string::npos) {
                std::string key = TrimString(line.substr(0, sepPos));
                std::string value = TrimString(line.substr(sepPos + 1));
                iniData[currentSection][key] = value;
            }
        }

        string NumOfUpdateStr = GetIniValue("Head", "NumOfUpdate");
        NumOfUpdate = std::stoi(NumOfUpdateStr);

        for (int idx = 0; idx < NumOfUpdate; ++idx) {
            string UpdateIdx = "Update" + std::to_string(idx);
            string SecionName = GetIniValue("Head", UpdateIdx);
            // todo: assert
            UINT32 BgupOffset = std::stoul(GetIniValue(SecionName, "HelperOffset"), nullptr, 16);
            UINT32 BgupSize = std::stoul(GetIniValue(SecionName, "HelperLength"), nullptr, 16);
            BgupList.push_back({SecionName, BgupOffset, BgupSize});
        }

        std::sort(BgupList.begin(), BgupList.end(), [](BgupConfig &config1, BgupConfig &config2) { return config1.BgupOffset < config2.BgupOffset; });

        return buffer.offset;
    }

    void ConfigIniClass::collectInfo(stringstream& Info)
    {
        Info << iniContext;
    }

    string ConfigIniClass::TrimString(const string& inputString) {
        std::size_t start = inputString.find_first_not_of(" \t\r\n");
        std::size_t end = inputString.find_last_not_of(" \t\r\n");

        if (start == std::string::npos || end == std::string::npos)
            return "";

        return inputString.substr(start, end - start + 1);
    }

    string ConfigIniClass::GetIniValue(const string& section, const string& key) {
        if (iniData.count(section) && iniData[section].count(key))
            return iniData[section][key];

        return "";
    }

    bool BgupHeaderClass::SearchBgup(Buffer& buffer, INT64 BgupOffset, UINT32 &BgupSize) {
        INT64 BufferSize = buffer.getBufferSize();
        if (BgupOffset > BufferSize)
        {
            throw CapsuleError("Buffer size smaller than FV header! Binary corrupted!");
        }
        else if (BgupOffset == BufferSize)
        {
            return false;
        }
        buffer.setOffset(BgupOffset);
        UINT32 XDRValue = buffer.getUINT32();
        BgupSize = swapEndian<UINT32>(XDRValue);

        UINT16 Version = buffer.getUINT16();
        if (Version == 0x0002) {
            return true;
        }
        return false;
    }

    void BgupHeaderClass::Decode(Buffer& buffer, INT64 offset, INT64 length, string content)
    {
        using UefiSpace::BiosGuardClass;

        buffer.setOffset(offset);
        UINT8 *BgupBuffer = buffer.getBytes(length);
        bgup = new BiosGuardClass(BgupBuffer, 0, length);
        bgup->setInfoStr();
        safeArrayDelete(BgupBuffer);

        Content = content;

        buffer.setOffset(offset);
        UINT8* BgupHeaderData = buffer.getBytes(sizeof(BGUP_HEADER));
        BgupHeader = *(BGUP_HEADER*)BgupHeaderData;
        delete[] BgupHeaderData;

        panelOffset = offset;
        panelSize = length;
    }

    string BgupHeaderClass::getPlatId() const
    {
        return charToString((CHAR8*)BgupHeader.PlatId, 16);
    }

    BgupHeaderClass::~BgupHeaderClass() {
        safeDelete(bgup);
    }

    string BgupHeaderClass::getEntryName() {
        return "BGUP - " + Content;
    }

    void BgupHeaderClass::collectInfo(stringstream& Info)
    {
        Info << bgup->InfoStr.toStdString();
    }

    void BiosIdClass::Decode(Buffer& buffer, INT64 offset, INT64 length) {
        INT64 PayloadOffset;
        char signatureArray[9]{ 0 };
        GUID gBiosIdGuid{ "C3E36D09-8294-4b97-A857-D5288FE33E28" };
        PayloadOffset = FfsFileHeaderClass::searchFfsWithGuid(buffer, offset, length, gBiosIdGuid, true);
        buffer.setOffset(PayloadOffset);

        char* bufferSignature = (char*)buffer.getBytes(8);
        for (int i = 0; i < 8; ++i) {
            signatureArray[i] = bufferSignature[i];
        }

        Signature = (char*)signatureArray;
        if (Signature != "$IBIOSI$") {
            throw CapsuleError("Bios ID not found!");
        }

        BIOS_ID_STRING = "";
        auto* BIOS_ID = (UINT16*)buffer.getBytes(66);
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

    void FitTableClass::Decode(Buffer& buffer, INT64 offset)
    {
        buffer.setOffset(offset);
        Address  = buffer.getINT64();
        Size     = (char*)buffer.getBytes(3);
        Rsvd     = buffer.getUINT8();
        Version  = buffer.getUINT16();
        Type     = buffer.getUINT8() & 0x7F;
        Checksum = buffer.getUINT8();
    }

    UINT8 FitTableClass::getType() const
    {
        return Type;
    }

    INT64 FitTableClass::getAddress() const
    {
        return Address & 0xFFFFFF;
    }

    FitTableClass::~FitTableClass()
    {
        delete [] Size;
    }

    void BiosClass::Decode(Buffer& buffer, INT64 offset, INT64 length)
    {
        const INT64 BiosFullSize = 0x1000000;
        const INT64 IbbSize      = 0x400000;
        INT64  FitPointerAddress = offset + length - 0x40;
        panelOffset = offset;
        panelSize = length;

        // Detect Resiliency BIOS
        buffer.setOffset(offset + length - 0x10);
        EFI_GUID ResetVecorIBB = buffer.getGUID();
        buffer.setOffset(offset + length - IbbSize - 0x10);
        EFI_GUID ResetVecorIBBR = buffer.getGUID();
        if (ResetVecorIBB == ResetVecorIBBR) {
            ResiliencyFlag = true;
        }

        buffer.setOffset(FitPointerAddress);
        INT64 FitTableAddress = buffer.getINT64() & 0xFFFFFF;
        FitTableAddress = adjustBufferAddress(BiosFullSize, FitTableAddress, length); // get the relative address of FIT table
        buffer.setOffset(offset + FitTableAddress);
        string FitSignature = buffer.getString(5);
        uCodeEntryList.clear();
        if (FitSignature == "_FIT_")
        {
            buffer.setOffset(offset + FitTableAddress + 8);
            INT32 Num = buffer.getINT32();
            INT32 FitEntryNum = (Num & 0xFFFFFF);
            for (int i = 0; i < FitEntryNum - 1; i++)
            {
                INT64 EntryOffset = offset + FitTableAddress + 0x10 * (i + 1);
                FitTableClass FitTable {};
                FitTable.Decode(buffer, EntryOffset);
                bool isUcode = (FitTable.getType() == 0x1);
                bool isAcm = (FitTable.getType() == 0x2);
                if (isUcode)
                {
                    CPUMicrocodeHeaderClass uCodeEntry{};
                    INT64 uCodeAddress = adjustBufferAddress(BiosFullSize, FitTable.getAddress(), length) + offset;
                    uCodeEntry.Decode(buffer, uCodeAddress);
                    uCodeEntryList.push_back(uCodeEntry);
                }
                if (isAcm)
                {
                    INT64 AcmAddress = adjustBufferAddress(BiosFullSize, FitTable.getAddress(), length) + offset;
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

        INT64  uCodeEntryNum = uCodeEntryList.size();
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

        Info << "\n" << Acm.getAcmVersion();
    }

    void AcmClass::Decode(Buffer& buffer, INT64 offset, INT64 DataLength)
    {
        buffer.setOffset(offset);
        if (buffer.getRemainingSize() < DataLength) {
            throw CapsuleError("Buffer size smaller than Ffs data!");
        }

        buffer.setOffset(offset);
        UINT8* AcmBuffer = buffer.getBytes(DataLength);
        auto *AcmEntry = new UefiSpace::AcmHeaderClass(AcmBuffer, 0);
        AcmEntry->setInfoStr();
        InfoStr = AcmEntry->InfoStr.toStdString();

        stringstream Info;
        Info << "Acm Version:"   << (UINT32)AcmEntry->AcmVersion.AcmMajorVersion << "." << (UINT32)AcmEntry->AcmVersion.AcmMinorVersion << "." << (UINT32)AcmEntry->AcmVersion.AcmRevision << "\n";
        AcmVersion = Info.str();
        safeDelete(AcmEntry);
        safeArrayDelete(AcmBuffer);

        panelOffset = offset;
        panelSize = DataLength;
    }

    void AcmClass::collectInfo(stringstream& Info)
    {
        Info << InfoStr;
    }

    string AcmClass::getAcmVersion()
    {
        return AcmVersion;
    }

    void EcClass::Decode(Buffer& buffer, INT64 offset, INT64 DataLength)
    {
        string searchValue;
        INT64 searchOffset = 0;
        panelOffset = offset;
        panelSize = DataLength;

        for (searchOffset = 0; searchOffset < DataLength; searchOffset += 2)
        {
            buffer.setOffset(offset + searchOffset);
            searchValue = buffer.getString(4);
            if (searchValue == EcImgSignature)
            {
                Signature = searchValue;
                PlatId = buffer.getUINT8();
                MajorVer = buffer.getUINT8();
                MinorVer = buffer.getUINT8();
                BuildVer = buffer.getUINT8();
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
             << "MajorVer = " << (INT16)MajorVer << endl
             << "MinorVer = " << (INT16)MinorVer << endl
             << "BuildVer = " << (INT16)BuildVer << endl;
    }

    void MeClass::Decode(Buffer& buffer, INT64 offset, INT64 DataLength)
    {
        panelOffset = offset;
        panelSize = DataLength;
    }

    void MeClass::collectInfo(stringstream& Info)
    {
        Info << "Unsupported" << endl;
    }

} // CapsuleTool
