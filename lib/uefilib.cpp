#include <iomanip>
#include "UefiLib.h"
#include "../include/Base.h"
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
        if (FileNameString != nullptr) {
            delete[] FileNameString;
        }
        if (VersionString != nullptr) {
            delete[] VersionString;
        }
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
        case EFI_SECTION_RAW:
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
        INT64 width = 18;
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

//    INT64 FfsFile::getSize() const{
//        if (isExtended)
//            return (INT64)FFS_FILE2_SIZE(&FfsExtHeader);
//        return (INT64)FFS_FILE_SIZE(&FfsHeader);
//    }

    INT64 FfsFile::getHeaderSize() const {
        if (isExtended)
            return sizeof(EFI_FFS_FILE_HEADER2);
        return sizeof(EFI_FFS_FILE_HEADER);
    }

    void FfsFile::decodeSections() {
        INT64 offset = sizeof(EFI_FFS_FILE_HEADER);
        if (isExtended) {
            offset = sizeof(EFI_FFS_FILE_HEADER2);
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
           << setw(width) << "Header Checksum:" << hex << uppercase << (UINT32)FfsHeader.IntegrityCheck.Checksum.Header << "h\n"
           << setw(width) << "Data Checksum:"   << hex << uppercase << (UINT32)FfsHeader.IntegrityCheck.Checksum.File << "h\n";

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
        if (isEmpty) {
            return;
        }
        if (isNv) {
            return;
        }
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
        UINT8* ZeroVector = address->ZeroVector;
        if ((*(UINT64*)ZeroVector != 0x0) || ((*((UINT64*)ZeroVector + 1) != 0x0)))
            return false;
        if (address->FileSystemGuid.Data1 == 0xFFFFFFFF) {
            return false;
        }
        return true;
    }
}
