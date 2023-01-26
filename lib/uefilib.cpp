#include <iomanip>
#include "UefiLib.h"
#include "../include/Base.h"
#include "../include/Microcode.h"
#include "../include/GuidDefinition.h"
#include "LzmaDecompress/LzmaDecompressLibInternal.h"
#include "BaseUefiDecompress/UefiDecompressLib.h"

namespace UefiSpace {

    Volume::Volume(UINT8* fv, INT64 length, INT64 offset):data(fv), size(length), offsetFromBegin(offset) {}

    Volume::~Volume() {
    }

    EFI_GUID Volume::getGUID(INT64 offset) {
        if (offset > size) {
            throw exception(ErrorMsg);
        }
        EFI_GUID guid = *(EFI_GUID*)(data + offset);
        return guid;
    }

    UINT8 Volume::getUINT8(INT64 offset) {
        if (offset > size) {
            throw exception(ErrorMsg);
        }
        return data[offset];
    }

    UINT16 Volume::getUINT16(INT64 offset) {
        if (offset > size) {
            throw exception(ErrorMsg);
        }
        return *(UINT16*)(data + offset);
    }

    UINT32 Volume::getUINT32(INT64 offset) {
        if (offset > size) {
            throw exception(ErrorMsg);
        }
        return *(UINT32*)(data + offset);
    }

    UINT8* Volume::getBytes(INT64 offset, INT64 length) {
        if (offset > size) {
            throw exception(ErrorMsg);
        }
        UINT8 *value = new UINT8[length];
        for (INT64 i = 0; i < length; i++) {
            value[i] = data[offset + i];
        }
        return value;
    }

    UINT64 Volume::getUINT64(INT64 offset) {
        if (offset > size) {
            throw exception(ErrorMsg);
        }
        return *(UINT64*)(data + offset);
    }

    INT8 Volume::getINT8(INT64 offset) {
        if (offset > size) {
            throw exception(ErrorMsg);
        }
        return (INT8) data[offset];
    }

    INT16 Volume::getINT16(INT64 offset) {
        if (offset > size) {
            throw exception(ErrorMsg);
        }
        return *(INT16*)(data + offset);
    }

    INT32 Volume::getINT24(INT64 offset) {
        if (offset > size) {
            throw exception(ErrorMsg);
        }
        UINT8 value[4] {0};
        for (INT64 i = 0; i < 3; i++) {
            value[i] = data[offset + i];
        }
        return *(INT32*)value;
    }

    INT32 Volume::getINT32(INT64 offset) {
        if (offset > size) {
            throw exception(ErrorMsg);
        }
        return *(INT32*)(data + offset);
    }

    INT64 Volume::getINT64(INT64 offset) {
        if (offset > size) {
            throw exception(ErrorMsg);
        }
        return *(INT64*)(data + offset);
    }

    INT64 Volume::getSize() const {
        return size;
    }

    void Volume::setInfoStr() {

    }

    CommonSection::CommonSection(UINT8* file, INT64 offset):Volume(file, 0, offset) {
        Type = VolumeType::CommonSection;
        CommonHeader = *(EFI_COMMON_SECTION_HEADER*)data;
        size = SECTION_SIZE(&CommonHeader);
        if (IS_SECTION2(&CommonHeader)) {
            EFI_COMMON_SECTION_HEADER2 *ExtHeader = (EFI_COMMON_SECTION_HEADER2*)data;
            size = SECTION2_SIZE(ExtHeader);
        }
    }

    CommonSection::CommonSection(UINT8* file, INT64 length, INT64 offset):Volume(file, length, offset) {
        Type = VolumeType::CommonSection;
    }

    CommonSection::~CommonSection() {
        if (FileNameString != nullptr)
            delete[] FileNameString;
        if (VersionString != nullptr)
            delete[] VersionString;
        if (peCoffHeader != nullptr)
            delete peCoffHeader;
        if (dependency != nullptr)
            delete dependency;
        for(auto file:ChildFile) {
            delete file;
        }
    }

    INT64 CommonSection::getSectionSize(UINT8* file) {
        EFI_COMMON_SECTION_HEADER *Header = (EFI_COMMON_SECTION_HEADER*)file;
        INT64 size = SECTION_SIZE(Header);
        if (IS_SECTION2(Header)) {
            EFI_COMMON_SECTION_HEADER2 *ExtHeader = (EFI_COMMON_SECTION_HEADER2*)file;
            size = SECTION2_SIZE(ExtHeader);
        }
        return size;
    }

    void CommonSection::SelfDecode() {
        INT64 HeaderSize = sizeof(EFI_COMMON_SECTION_HEADER);
        CommonHeader = *(EFI_COMMON_SECTION_HEADER*)data;
        INT64 offset = sizeof(EFI_COMMON_SECTION_HEADER);
        INT32 SectionSize = SECTION_SIZE(&CommonHeader);
        if (IS_SECTION2(&CommonHeader)) {
            isExtend = true;
            ExtendedSize = this->getUINT32(offset);
            SectionSize = ExtendedSize;
            HeaderSize = sizeof(EFI_COMMON_SECTION_HEADER2);
            offset += 4;
        }

        switch (CommonHeader.Type) {
        case EFI_SECTION_COMPRESSION:
            UINT32 ScratchSize;
            VOID* destination;
            VOID* scratch;
            RETURN_STATUS status;
            UncompressedLength = this->getINT32(offset);
            CompressionType = this->getUINT8(offset + 4);
            HeaderSize += 5;
            if (CompressionType == EFI_STANDARD_COMPRESSION) {
                ScratchSize = 0;
                status = UefiDecompressGetInfo(data + HeaderSize, SectionSize - HeaderSize, &decompressedSize, &ScratchSize);
                if (status != RETURN_SUCCESS) {
                    throw exception();
                }

                destination = malloc(decompressedSize);
                scratch = malloc(ScratchSize);
                status = UefiDecompress(data + HeaderSize, destination, scratch);
                cout << "UefiDecompress Status = " << status << ", size = " << decompressedSize << endl;
                if (status != RETURN_SUCCESS) {
                    throw exception();
                }
                DecodeDecompressedBuffer((UINT8*)destination, decompressedSize);
                free(scratch);
            }
            break;
        case EFI_SECTION_FREEFORM_SUBTYPE_GUID:
            SubTypeGuid = this->getGUID(offset);
            break;
        case EFI_SECTION_GUID_DEFINED:
            INT64 ChildSectionSize;

            SectionDefinitionGuid = this->getGUID(offset);
            DataOffset = this->getUINT16(offset + 0x10);
            Attributes = this->getUINT16(offset + 0x12);
//            offset += 0x14;
            HeaderSize = sizeof(EFI_GUID_DEFINED_SECTION);
            if (isExtend) {
                HeaderSize = sizeof(EFI_GUID_DEFINED_SECTION2);
            }
            if (SectionDefinitionGuid == GuidDatabase::gEfiCertTypeRsa2048Sha256Guid) {
                cout << "Rsa2048, SectionSize = " << hex << SectionSize << endl;
                HeaderSize = 0x228;
                DataOffset = HeaderSize;
                offset = HeaderSize;
                while (offset < SectionSize) {
                    ChildFile.push_back(new CommonSection(data + offset, offsetFromBegin + offset));
                    ChildSectionSize = CommonSection::getSectionSize(data + offset);
                    offset += ChildSectionSize;
                    Buffer::Align(offset, 0, 0x4);
                }
                break;
            }

            if (SectionDefinitionGuid == GuidDatabase::gLzmaCustomDecompressGuid) {
                ScratchSize = 0;
                status = LzmaUefiDecompressGetInfo(data + HeaderSize, SectionSize - HeaderSize, &decompressedSize, &ScratchSize);
                if (status != RETURN_SUCCESS) {
                    throw exception();
                }

                destination = malloc(decompressedSize);
                scratch = malloc(ScratchSize);
                status = LzmaUefiDecompress(data + HeaderSize, SectionSize - HeaderSize, destination, scratch);
                cout << "LzmaUefiDecompress Status = " << status << ", size = " << decompressedSize << endl;
                if (status != RETURN_SUCCESS) {
                    throw exception();
                }
                DecodeDecompressedBuffer((UINT8*)destination, decompressedSize);
                free(scratch);
            }
            break;
        case EFI_SECTION_PE32:
        case EFI_SECTION_TE:
            peCoffHeader = new PeCoff(data + HeaderSize, SectionSize - HeaderSize, offsetFromBegin + HeaderSize);
            break;
        case EFI_SECTION_USER_INTERFACE:
            FileNameString = (UINT16*)this->getBytes(offset, SectionSize - HeaderSize);
            break;
        case EFI_SECTION_VERSION:
            BuildNumber = this->getUINT16(offset);
            VersionString = (UINT16*)this->getBytes(offset + 2, SectionSize - HeaderSize - 2);
            break;
        case EFI_SECTION_FIRMWARE_VOLUME_IMAGE:
            ChildFile.push_back(new FirmwareVolume(data + HeaderSize, SectionSize - HeaderSize, offsetFromBegin + HeaderSize));
            break;
        case EFI_SECTION_DXE_DEPEX:
        case EFI_SECTION_PEI_DEPEX:
        case EFI_SECTION_MM_DEPEX:
            dependency = new Depex(data + HeaderSize, SectionSize - HeaderSize);
            break;
        case EFI_SECTION_RAW:
            if (isAprioriRaw) {
                INT64 index = 0;
                INT64 RemainingSize = SectionSize - HeaderSize;
                while (RemainingSize >= sizeof(EFI_GUID)) {
                    EFI_GUID AprioriFileGuid = *(EFI_GUID*)(data + HeaderSize + index * sizeof(EFI_GUID));
                    AprioriList.push_back(AprioriFileGuid);
                    RemainingSize -= sizeof(EFI_GUID);
                    index += 1;
                }
            }
            break;
        default:
            break;
        }

        DecodeChildFile();
    }

    void CommonSection::DecodeDecompressedBuffer(UINT8* DecompressedBuffer, INT64 bufferSize) {
        INT64 offset = 0;
        while (offset < bufferSize) {
            EFI_COMMON_SECTION_HEADER *hdr = (EFI_COMMON_SECTION_HEADER*)(DecompressedBuffer + offset);
            INT32 SectionSize = SECTION_SIZE(hdr);
            if (IS_SECTION2(hdr)) {
                EFI_COMMON_SECTION_HEADER2 *hdr2 = (EFI_COMMON_SECTION_HEADER2*)(DecompressedBuffer + offset);
                SectionSize = SECTION2_SIZE(hdr2);
            }
            auto DecompressedSection = new CommonSection(DecompressedBuffer + offset, SectionSize, offset);
            DecompressedSection->isCompressed = true;
            ChildFile.push_back(DecompressedSection);
            offset += SectionSize;
            Buffer::Align(offset, 0, 0x4);
        }
    }

    void CommonSection::DecodeChildFile() {
        for(auto volume:ChildFile) {
            switch (volume->Type) {
            case VolumeType::FirmwareVolume:
                ((FirmwareVolume*)volume)->decodeFfs();
                break;
            case VolumeType::FfsFile:
                ((FfsFile*)volume)->decodeSections();
                break;
            case VolumeType::CommonSection:
                ((CommonSection*)volume)->SelfDecode();
                break;
            default:
                throw exception();
                break;
            }
        }
    }

    void CommonSection::setInfoStr() {
        INT64 width = 20;
        stringstream ss;
        stringstream guidInfo;
        ss.setf(ios::left);

        if (CommonHeader.Type == EFI_SECTION_GUID_DEFINED)
            guidInfo << "Section GUID:\n" << GUID(SectionDefinitionGuid).str(true) << "\n";

        ss << setw(width) << "Type:"        << hex << uppercase << (UINT32)CommonHeader.Type << "h\n"
           << setw(width) << "Full size:"   << hex << uppercase << getSize() << "h\n"
           << setw(width) << "Header size:" << hex << uppercase << getHeaderSize() << "h\n"
           << setw(width) << "Body size:"   << hex << uppercase << getSize() - getHeaderSize() << "h\n";

        switch (CommonHeader.Type) {
        case EFI_SECTION_COMPRESSION:
            ss << setw(width) << "Compression type:"  << hex << uppercase << (UINT32)CompressionType << "h\n"
               << setw(width) << "Decompressed size:" << hex << uppercase << decompressedSize << "h\n"
               << "Compression algorithm: EFI 1.1\n";
            break;
        case EFI_SECTION_GUID_DEFINED:
            ss << setw(width) << "Data offset:"   << hex << uppercase << DataOffset << "h\n"
               << setw(width) << "Attributes:"    << hex << uppercase << Attributes << "h\n";

            if (SectionDefinitionGuid == GuidDatabase::gLzmaCustomDecompressGuid) {
                ss << "Compression algorithm: LZMA\n"
                   << setw(width) << "Decompressed size:" << hex << uppercase << decompressedSize << "h\n";
            } else if (SectionDefinitionGuid == GuidDatabase::gEfiCertTypeRsa2048Sha256Guid) {
                ss << "Certificate type: RSA2048/SHA256";
            }
            break;
        case EFI_SECTION_PE32:
            UINT16 e_magic;
            UINT32 peSignature;
            UINT16 peOptionalSignature;
            UINT16 SubSystem;
            e_magic = peCoffHeader->dosHeader.e_magic;
            peSignature = peCoffHeader->pe32Header.Signature;
            peOptionalSignature = peCoffHeader->pe32Header.OptionalHeader.Magic;
            SubSystem = peCoffHeader->pe32Header.OptionalHeader.Subsystem;

            ss << setw(width) << "DOS signature:" << hex << uppercase << e_magic << "h (" << Buffer::charToString((INT8*)&e_magic, sizeof(UINT16), false) << ")\n"
               << setw(width) << "PE signature:" << hex << uppercase << peSignature << "h (" << Buffer::charToString((INT8*)&peSignature, sizeof(UINT32), false) << ")\n"
               << setw(width) << "Machine type:" << peCoffHeader->getMachineType() << "\n"
               << setw(width) << "Number of sections:" << hex << uppercase << peCoffHeader->pe32Header.FileHeader.NumberOfSections << "h\n"
               << setw(width) << "Characteristics:" << hex << uppercase << peCoffHeader->pe32Header.FileHeader.Characteristics << "h\n"
               << setw(width) << "Optional header signature:" << hex << uppercase << peOptionalSignature << "h\n"
               << setw(width) << "Subsystem:" << hex << uppercase << SubSystem << "h (" << PeCoff::getSubsystemName(SubSystem) << ")\n"
               << setw(width) << "EntryPoint Address:" << hex << uppercase << peCoffHeader->pe32Header.OptionalHeader.AddressOfEntryPoint << "h\n"
               << setw(width) << "Base of code:" << hex << uppercase << peCoffHeader->pe32Header.OptionalHeader.BaseOfCode << "h\n"
               << setw(width) << "Base of data:" << hex << uppercase << peCoffHeader->pe32Header.OptionalHeader.BaseOfData << "h\n";
            if (peCoffHeader->isPe32Plus)
                ss << setw(width) << "Image base:" << hex << uppercase << peCoffHeader->pe32plusHeader.OptionalHeader.ImageBase << "h\n";
            else
                ss << setw(width) << "Image base:" << hex << uppercase << peCoffHeader->pe32Header.OptionalHeader.ImageBase << "h\n";
            break;
        case EFI_SECTION_TE:
            e_magic = peCoffHeader->teHeader.Signature;
            SubSystem = peCoffHeader->teHeader.Subsystem;

            ss << setw(width) << "TE signature:" << hex << uppercase << e_magic << "h (" << Buffer::charToString((INT8*)&e_magic, sizeof(UINT16), false) << ")\n"
               << setw(width) << "Machine type:" << peCoffHeader->getMachineType() << "\n"
               << setw(width) << "Number of sections:" << hex << uppercase << (UINT32)peCoffHeader->teHeader.NumberOfSections << "h\n"
               << setw(width) << "Subsystem:" << hex << uppercase << SubSystem << "h (" << PeCoff::getSubsystemName(SubSystem) << ")\n"
               << setw(width) << "Stripped size:" << hex << uppercase << peCoffHeader->teHeader.StrippedSize << "h\n"
               << setw(width) << "Base of code:" << hex << uppercase << peCoffHeader->teHeader.BaseOfCode << "h\n"
               << setw(width) << "EntryPoint Address:" << hex << uppercase << peCoffHeader->teHeader.AddressOfEntryPoint << "h\n"
               << setw(width) << "Image base:" << hex << uppercase << peCoffHeader->teHeader.ImageBase << "h\n"
               << setw(width) << "VirtualAddress:" << hex << uppercase << peCoffHeader->teHeader.DataDirectory->VirtualAddress << "h\n";
            break;
        case EFI_SECTION_FREEFORM_SUBTYPE_GUID:
            ss << "Section GUID:\n" << GUID(SubTypeGuid).str(true) << "\n";
            break;
        case EFI_SECTION_USER_INTERFACE:
            ss << setw(width) << "FileName:"      << Buffer::wstringToString(FileNameString) << "\n";
            break;
        case EFI_SECTION_VERSION:
            ss << setw(width) << "BuildNumber:"   << hex << uppercase << BuildNumber << "h\n"
               << setw(width) << "Version:"       << Buffer::wstringToString(VersionString) << "\n";
            break;
        case EFI_SECTION_DXE_DEPEX:
        case EFI_SECTION_PEI_DEPEX:
        case EFI_SECTION_MM_DEPEX:
            ss << "\n" << "Dependency:\n";
            for (auto &depexStr : dependency->OrganizedDepexList)
                ss << depexStr << "\n";
            break;
        case EFI_SECTION_RAW:
            if (isAprioriRaw) {
                ss << "Apriori List:\n";
                for (auto ApriFile:AprioriList) {
                    ss << guidData->getNameFromGuid(ApriFile) << "\n";
                }
            }
            break;
        default:
            break;
        }

        InfoStr = QString::fromStdString(guidInfo.str() +  ss.str());
    }

    INT64 CommonSection::getHeaderSize() const {
        INT64 headerSize = 0;
        switch (CommonHeader.Type) {
        case EFI_SECTION_COMPRESSION:
            headerSize = sizeof(EFI_COMPRESSION_SECTION);
            break;
        case EFI_SECTION_GUID_DEFINED:
            headerSize = sizeof(EFI_GUID_DEFINED_SECTION);
            break;
        case EFI_SECTION_FREEFORM_SUBTYPE_GUID:
            headerSize = sizeof(EFI_FREEFORM_SUBTYPE_GUID_SECTION);
            break;
        case EFI_SECTION_USER_INTERFACE:
            headerSize = sizeof(EFI_USER_INTERFACE_SECTION);
            break;
        case EFI_SECTION_VERSION:
            headerSize = sizeof(EFI_VERSION_SECTION);
            break;
        default:
            headerSize = sizeof(EFI_COMMON_SECTION_HEADER);
            break;

        if (isExtend)
            headerSize += sizeof(UINT32);
        }
        return headerSize;
    }

    FfsFile::FfsFile(UINT8* file, INT64 length, INT64 offset, bool isExt):Volume(file, length, offset) {
        Type = VolumeType::FfsFile;
        isExtended = isExt;
        FfsHeader = *(EFI_FFS_FILE_HEADER*)data;
        if (isExtended) {
            FfsExtHeader = *(EFI_FFS_FILE_HEADER2*)data;
        }

        UINT8 headerSumValue = 0;
//        UINT8 dataSumValue = 0;
        if (isExtended) {
            headerSumValue = Buffer::CaculateSum8((UINT8*)&FfsExtHeader, sizeof(EFI_FFS_FILE_HEADER2));
            headerSumValue = headerSumValue + FfsExtHeader.State + FfsExtHeader.IntegrityCheck.Checksum.File;
        } else {
            headerSumValue = Buffer::CaculateSum8((UINT8*)&FfsHeader, sizeof(EFI_FFS_FILE_HEADER));
            headerSumValue = headerSumValue + FfsHeader.State + FfsHeader.IntegrityCheck.Checksum.File;

//            dataSumValue = Buffer::CaculateSum8((UINT8*)(data + sizeof(EFI_FFS_FILE_HEADER)), getSize() - sizeof(EFI_FFS_FILE_HEADER));
//            dataSumValue -= FfsHeader.IntegrityCheck.Checksum.File;
//            cout << "ffs dataSumValue = " << hex << (UINT32)dataSumValue << endl;
        }
        if (headerSumValue == 0)
            headerChecksumValid = true;
        if ((FfsHeader.Attributes | FFS_ATTRIB_CHECKSUM) == 0x0)
            dataChecksumValid = true;
        if (FfsHeader.Name.Data1 == GuidDatabase::gPeiAprioriFileNameGuid.Data1 || FfsHeader.Name.Data1 == GuidDatabase::gAprioriGuid.Data1) {
            isApriori = true;
        }
    }

    FfsFile::~FfsFile() {
        for(auto sec:Sections) {
            delete sec;
        }
    }
    UINT8 FfsFile::getType() const {
        if (isExtended) {
            return FfsExtHeader.Type;
        }
        return FfsHeader.Type;
    }

    INT64 FfsFile::getHeaderSize() const {
        if (isExtended)
            return sizeof(EFI_FFS_FILE_HEADER2);
        return sizeof(EFI_FFS_FILE_HEADER);
    }

    void FfsFile::decodeSections() {
        if (!headerChecksumValid)
            return;
        INT64 offset = sizeof(EFI_FFS_FILE_HEADER);
        if (isExtended) {
            offset = sizeof(EFI_FFS_FILE_HEADER2);
        }
        if (isApriori) {
            CommonSection *Sec = new CommonSection(data + offset, offsetFromBegin + offset);
            Sec->isAprioriRaw = true;
            Sec->SelfDecode();
            Sections.push_back(Sec);
            return;
        }
        while (offset < size) {
            EFI_COMMON_SECTION_HEADER *SecHeader = (EFI_COMMON_SECTION_HEADER*)(data + offset);
            INT64 SecSize = SECTION_SIZE(SecHeader);
            if (SecSize == 0xFFFFFF) {
                SecSize = this->getUINT32(offset + sizeof(EFI_COMMON_SECTION_HEADER));
            }
            CommonSection *Sec = new CommonSection(data + offset, SecSize, offsetFromBegin + offset);
            Sec->SelfDecode();
            Sections.push_back(Sec);
            offset += SecSize;
            Buffer::Align(offset, 0, 0x4);
        }
    }

    void FfsFile::setInfoStr() {
        INT64 width = 18;
        stringstream ss;
        ss.setf(ios::left);

        ss << "File GUID:\n" << GUID(FfsHeader.Name).str(true) << "\n"
           << setw(width) << "Type:"        << hex << (UINT32)FfsHeader.Type << "h\n"
           << setw(width) << "Attributes:"  << hex << uppercase << (UINT32)FfsHeader.Attributes << "h\n"
           << setw(width) << "Full size:"   << hex << uppercase << getSize() << "h\n"
           << setw(width) << "Header size:" << hex << uppercase << getHeaderSize() << "h\n"
           << setw(width) << "Body size:"   << hex << uppercase << getSize() - getHeaderSize() << "h\n"
           << setw(width) << "State:"       << hex << (UINT32)FfsHeader.State << "h\n"
           << setw(width) << "Header Checksum:" << hex << uppercase << (UINT32)FfsHeader.IntegrityCheck.Checksum.Header << "h" << (headerChecksumValid ? ", valid":", not valid") << "\n"
           << setw(width) << "Data Checksum:"   << hex << uppercase << (UINT32)FfsHeader.IntegrityCheck.Checksum.File << "h" << (headerChecksumValid ? ", valid":", not valid") << "\n";

        InfoStr = QString::fromStdString(ss.str());
    }

    FirmwareVolume::FirmwareVolume(UINT8* fv, INT64 length, INT64 offset):Volume(fv, length, offset) {
        if (!isValidFirmwareVolume((EFI_FIRMWARE_VOLUME_HEADER*)data)) {
            isEmpty = true;
        }

        Type = VolumeType::FirmwareVolume;
        FirmwareVolumeHeader = *(EFI_FIRMWARE_VOLUME_HEADER*)data;
        if (FirmwareVolumeHeader.ExtHeaderOffset == 0) {
            FirmwareVolumeSize = 0x48;
            isExt = false;
        } else {
            FirmwareVolumeSize = 0x78;
            FirmwareVolumeExtHeader = *(EFI_FIRMWARE_VOLUME_EXT_HEADER*)(data + FirmwareVolumeHeader.ExtHeaderOffset);
            isExt = true;
        }

        if (FirmwareVolumeHeader.FileSystemGuid == GuidDatabase::gEfiSystemNvDataFvGuid) {
            isNv = true;
        }

        UINT16 sumValue = Buffer::CaculateSum16((UINT16*)&FirmwareVolumeHeader, sizeof(EFI_FIRMWARE_VOLUME_HEADER) / sizeof (UINT16));
        if (sumValue == 0)
            checksumValid = true;
    }

    FirmwareVolume::~FirmwareVolume() {
        GUID guid = getFvGuid(true);
        cout << "~FirmwareVolume: " << &guid << endl;
        for (auto file:FfsFiles) {
            delete file;
        }
        if (freeSpace != nullptr) {
            delete freeSpace;
        }
    }

    GUID FirmwareVolume::getFvGuid(bool returnExt) const {
        if (returnExt) {
            if (isExt) {
                return GUID(FirmwareVolumeExtHeader.FvName);
            }
        }
        return GUID(FirmwareVolumeHeader.FileSystemGuid);
    }

    void FirmwareVolume::decodeFfs() {
        if (isEmpty || !checksumValid || isNv)
            return;
        INT64 offset = FirmwareVolumeSize;
        while (offset < size) {
            bool isExtended = false;
            EFI_FFS_FILE_HEADER  FfsHeader = *(EFI_FFS_FILE_HEADER*)(data + offset);
            INT64 FfsSize = FFS_FILE_SIZE(&FfsHeader);

            if (FfsSize == 0) {
                isExtended = true;
                EFI_FIRMWARE_VOLUME_EXT_HEADER ExtFfs = *(EFI_FIRMWARE_VOLUME_EXT_HEADER*)(data + offset);
                FfsSize = (INT64)ExtFfs.ExtHeaderSize;
            } else if (FfsSize == 0xFFFFFF) {
                if (FfsHeader.State == 0xFF) {
                    freeSpace = new Volume(data + offset, size - offset);
                }
                break;
            }
            FfsFile *Ffs = new FfsFile(data + offset, FfsSize, offsetFromBegin + offset, isExtended);
            switch (Ffs->getType()) {
            case EFI_FV_FILETYPE_FIRMWARE_VOLUME_IMAGE:
            case EFI_FV_FILETYPE_FREEFORM:
            case EFI_FV_FILETYPE_SECURITY_CORE:
            case EFI_FV_FILETYPE_PEI_CORE:
            case EFI_FV_FILETYPE_DXE_CORE:
            case EFI_FV_FILETYPE_PEIM:
            case EFI_FV_FILETYPE_DRIVER:
            case EFI_FV_FILETYPE_APPLICATION:
            case EFI_FV_FILETYPE_MM:
                Ffs->decodeSections();
                break;
            default:
                break;
            }
            FfsFiles.push_back(Ffs);
            offset += FfsSize;
            Buffer::Align(offset, 0, 0x8);
        }
    }

    void FirmwareVolume::setInfoStr() {
        INT64 width = 15;
        stringstream ss;
        ss.setf(ios::left);
        if (isEmpty)
            return;

        ss << "FileSystem GUID:\n" << GUID(FirmwareVolumeHeader.FileSystemGuid).str(true) << "\n"
           << setw(width) << "Signature:"   << Buffer::charToString((INT8*)(&FirmwareVolumeHeader.Signature), sizeof(UINT32)) << "\n"
           << setw(width) << "Full size:"   << hex << uppercase << FirmwareVolumeHeader.FvLength << "h\n"
           << setw(width) << "Header size:" << hex << uppercase << FirmwareVolumeSize << "h\n"
           << setw(width) << "Body size:"   << hex << uppercase << FirmwareVolumeHeader.FvLength - FirmwareVolumeSize << "h\n"
           << setw(width) << "Revision:"    << hex << (UINT32)FirmwareVolumeHeader.Revision << "\n"
           << setw(width) << "Attributes:"  << hex << uppercase << FirmwareVolumeHeader.Attributes << "h\n"
           << setw(width) << "Checksum:"    << hex << uppercase << FirmwareVolumeHeader.Checksum << "h\n";

        if (isExt) {
            ss << "Extended header size:" << hex << FirmwareVolumeExtHeader.ExtHeaderSize << "h\n"
               << "Volume GUID:\n" << GUID(FirmwareVolumeExtHeader.FvName).str(true) << "\n";
        }

        InfoStr = QString::fromStdString(ss.str());
    }

    bool FirmwareVolume::isValidFirmwareVolume(EFI_FIRMWARE_VOLUME_HEADER* address) {
        if (address->Signature != 0x4856465F)
            return false;
        UINT8* ZeroVector = address->ZeroVector;
        if ((*(UINT64*)ZeroVector != 0x0) || ((*((UINT64*)ZeroVector + 1) != 0x0)))
            return false;
        if (address->FileSystemGuid.Data1 == 0xFFFFFFFF || address->FileSystemGuid.Data1 == 0x0) {
            return false;
        }
        UINT16 sumValue = Buffer::CaculateSum16((UINT16*)address, sizeof(EFI_FIRMWARE_VOLUME_HEADER) / sizeof (UINT16));
        if (sumValue != 0)
            return false;
        return true;
    }

    BiosImageVolume::BiosImageVolume(UINT8* fv, INT64 length):Volume(fv, length) {
        FitTable = new FitTableClass(fv, length);
    }

    BiosImageVolume::~BiosImageVolume() {
        cout << "~BiosImageVolume" << endl;
        delete FitTable;
        delete[] data;
    }

    void BiosImageVolume::setInfoStr() {
        INT64 width = 15;
        stringstream ss;
        ss.setf(ios::left);

        ss << setw(width) << "BIOS ID:" << BiosID << "\n";

        InfoStr = QString::fromStdString(ss.str());
    }

    FitTableClass::FitTableClass(UINT8* fv, INT64 length) {
        INT64 FitTableAddress = *(INT64*)(fv + length - DEFAULT_FIT_TABLE_POINTER_OFFSET) & 0xFFFFFF;
        FitTableAddress = Buffer::adjustBufferAddress(0x1000000, FitTableAddress, length); // get the relative address of FIT table
        FitHeader = *(FIRMWARE_INTERFACE_TABLE_ENTRY*)(fv + FitTableAddress);
        UINT64 FitSignature = FitHeader.Address;
        if (FitSignature == (UINT64)FIT_SIGNATURE) {
            isValid = true;
            FitEntryNum = *(UINT32*)(FitHeader.Size) & 0xFFFFFF;

            for (INT64 index = 1; index < FitEntryNum; ++index) {
                FIRMWARE_INTERFACE_TABLE_ENTRY FitEntry = *(FIRMWARE_INTERFACE_TABLE_ENTRY*)(fv + FitTableAddress + sizeof(FIRMWARE_INTERFACE_TABLE_ENTRY) * index);
                FitEntries.push_back(FitEntry);
                if (FitEntry.Type == FIT_TABLE_TYPE_MICROCODE) {
                    UINT64 MicrocodeAddress = FitEntry.Address & 0xFFFFFF;
                    UINT64 RelativeMicrocodeAddress = Buffer::adjustBufferAddress(0x1000000, MicrocodeAddress, length);
                    MicrocodeHeaderClass *MicrocodeEntry = new MicrocodeHeaderClass(fv + RelativeMicrocodeAddress, MicrocodeAddress);
                    MicrocodeEntries.push_back(MicrocodeEntry);
                }
            }
        } else
            isValid = false;;
    }

    FitTableClass::~FitTableClass() {
        for (auto MicrocodeEntry:MicrocodeEntries)
            delete MicrocodeEntry;
    }

    string FitTableClass::getTypeName(UINT8 type) {
        string typeName;
        switch (type) {
        case FIT_TABLE_TYPE_HEADER:
            typeName = "Header";
            break;
        case FIT_TABLE_TYPE_MICROCODE:
            typeName = "Microcode";
            break;
        case FIT_TABLE_TYPE_STARTUP_ACM:
            typeName = "Startup ACM";
            break;
        case FIT_TABLE_TYPE_DIAGNST_ACM:
            typeName = "Diagnst ACM";
            break;
        case FIT_TABLE_TYPE_PROT_BOOT_POLICY:
            typeName = "Port Boot Policy";
            break;
        case FIT_TABLE_TYPE_BIOS_MODULE:
            typeName = "BIOS Module";
            break;
        case FIT_TABLE_TYPE_TPM_POLICY:
            typeName = "TPM Policy";
            break;
        case FIT_TABLE_TYPE_BIOS_POLICY:
            typeName = "BIOS Policy";
            break;
        case FIT_TABLE_TYPE_TXT_POLICY:
            typeName = "TXT Policy";
            break;
        case FIT_TABLE_TYPE_KEY_MANIFEST:
            typeName = "Key Manifest";
            break;
        case FIT_TABLE_TYPE_BOOT_POLICY_MANIFEST:
            typeName = "Boot Policy Manifest";
            break;
        case FIT_TABLE_TYPE_BIOS_DATA_AREA:
            typeName = "BIOS Data Area";
            break;
        case FIT_TABLE_TYPE_CSE_SECURE_BOOT:
            typeName = "CSE Secure Boot";
            break;
        case FIT_TABLE_TYPE_VAB_PROVISION_TABLE:
            typeName = "VAB Provision Table";
            break;
        case FIT_TABLE_TYPE_VAB_BOOT_IMAGE_MANIFEST:
            typeName = "Boot Image Manifest";
            break;
        case FIT_TABLE_TYPE_VAB_BOOT_KEY_MANIFEST:
            typeName = "Boot Key Manifest";
            break;
        default:
            typeName = "Unkown";
            break;
        }
        return typeName;
    }

    MicrocodeHeaderClass::MicrocodeHeaderClass(UINT8* fv, INT64 address):data(fv), offset(address) {
        microcodeHeader = *(CPU_MICROCODE_HEADER*)fv;
        if (microcodeHeader.HeaderVersion == 0xFFFFFFFF) {
            isEmpty = true;
            return;
        }

        UINT32 ExtendedTableLength = microcodeHeader.TotalSize - (microcodeHeader.DataSize + sizeof(CPU_MICROCODE_HEADER));
        cout << "ExtendedTableLength = " << hex << ExtendedTableLength << endl;
        if (ExtendedTableLength != 0) {
            ExtendedTableHeader = (CPU_MICROCODE_EXTENDED_TABLE_HEADER *)(fv + microcodeHeader.DataSize + sizeof(CPU_MICROCODE_HEADER));
            if ((ExtendedTableLength > sizeof(CPU_MICROCODE_EXTENDED_TABLE_HEADER)) && ((ExtendedTableLength & 0x3) == 0)) {
//                UINT32 CheckSum32 = Buffer::CaculateSum32((UINT32 *)ExtendedTableHeader, ExtendedTableLength);
                UINT32 ExtendedTableCount = ExtendedTableHeader->ExtendedSignatureCount;
                if (ExtendedTableCount <= (ExtendedTableLength - sizeof(CPU_MICROCODE_EXTENDED_TABLE_HEADER)) / sizeof(CPU_MICROCODE_EXTENDED_TABLE)) {
                    CPU_MICROCODE_EXTENDED_TABLE *ExtendedTable = (CPU_MICROCODE_EXTENDED_TABLE *)(ExtendedTableHeader + 1);
                    for (INT32 Index = 0; Index < ExtendedTableCount; Index++) {
                        ExtendedMicrocodeList.push_back(*ExtendedTable);
                        ExtendedTable += 1;
                    }
                }
            }
        }
    }

    MicrocodeHeaderClass::~MicrocodeHeaderClass() {}

    void MicrocodeHeaderClass::setInfoStr() {
        INT64 width = 20;
        stringstream ss;
        ss.setf(ios::left);

        ss << "Microcode Info:" << "\n"
           << setw(width) << "Offset:" << hex << uppercase << offset << "h\n";
        if (!isEmpty) {
            ss << setw(width) << "HeaderVersion:" << hex << uppercase << microcodeHeader.HeaderVersion << "h\n"
               << setw(width) << "UpdateRevision:" << hex << uppercase << microcodeHeader.UpdateRevision << "h\n"
               << setw(width) << "Date:" << hex << uppercase << microcodeHeader.Date.Bits.Year << "-" << microcodeHeader.Date.Bits.Month << "-" << microcodeHeader.Date.Bits.Day << "\n"
               << setw(width) << "ProcessorSignature:" << hex << uppercase << microcodeHeader.ProcessorSignature.Uint32 << "h\n"
               << setw(width) << "Checksum:" << hex << uppercase << microcodeHeader.Checksum << "h\n"
               << setw(width) << "LoaderRevision:" << hex << uppercase << microcodeHeader.LoaderRevision << "h\n"
               << setw(width) << "ProcessorFlags:" << hex << uppercase << microcodeHeader.ProcessorFlags << "h\n"
               << setw(width) << "DataSize:" << hex << uppercase << microcodeHeader.DataSize << "h\n"
               << setw(width) << "TotalSize:" << hex << uppercase << microcodeHeader.TotalSize << "h\n";
        }

        if (ExtendedMicrocodeList.size() != 0) {
            for (auto ExtendedMicrocode:ExtendedMicrocodeList) {
                ss << "\nExtended Microcode Info:" << "\n"
                   << setw(width) << "ProcessorSignature:" << hex << uppercase << ExtendedMicrocode.ProcessorSignature.Uint32 << "h\n";
            }
        }

        InfoStr = QString::fromStdString(ss.str());
    }
}
