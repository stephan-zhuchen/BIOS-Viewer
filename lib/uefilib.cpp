#include <iomanip>
#include <thread>
#include <atomic>
#include <algorithm>
#include "UefiLib.h"
#include "Base.h"
#include "Microcode.h"
#include "ElfLib.h"
#include "GuidDefinition.h"
#include "LzmaDecompress/LzmaDecompressLibInternal.h"
#include "BaseUefiDecompress/UefiDecompressLib.h"

namespace UefiSpace {

    Volume::Volume(UINT8* fv, INT64 length, INT64 offset, bool Compressed):
        data(fv),
        size(length),
        offsetFromBegin(offset),
        isCompressed(Compressed) { }

    Volume::~Volume() {
    }

    EFI_GUID Volume::getVolumeGuid() const {
        EFI_GUID VolumeGuid {0};
        if (Type == VolumeType::FirmwareVolume) {
            VolumeGuid = ((FirmwareVolume*)this)->getFvGuid().GuidData;
        } else if (Type == VolumeType::FfsFile) {
            VolumeGuid = ((FfsFile*)this)->FfsHeader.Name;
        }
        return VolumeGuid;
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

    INT64 Volume::getHeaderSize() const {
        return 0;
    }

    void Volume::setInfoStr() {
    }

    EmptyVolume::EmptyVolume(UINT8* file, INT64 length, INT64 offset):Volume(file, length, offset) {

    }

    CommonSection::CommonSection(UINT8* file, INT64 offset, FfsFile *Ffs, bool Compressed):Volume(file, 0, offset, Compressed) {
        Type = VolumeType::CommonSection;
        ParentFFS = Ffs;
        CommonHeader = *(EFI_COMMON_SECTION_HEADER*)data;
        size = SECTION_SIZE(&CommonHeader);
        if (IS_SECTION2(&CommonHeader)) {
            EFI_COMMON_SECTION_HEADER2 *ExtHeader = (EFI_COMMON_SECTION_HEADER2*)data;
            size = SECTION2_SIZE(ExtHeader);
        }
    }

    CommonSection::CommonSection(UINT8* file, INT64 length, INT64 offset, FfsFile *Ffs, bool Compressed):Volume(file, length, offset, Compressed) {
        Type = VolumeType::CommonSection;
        ParentFFS = Ffs;
        CommonHeader = *(EFI_COMMON_SECTION_HEADER*)data;
    }

    CommonSection::~CommonSection() {
        if (peCoffHeader != nullptr)
            delete peCoffHeader;
        if (dependency != nullptr)
            delete dependency;
        for (auto file : ChildFile) {
            delete file;
        }
        if (AcpiTable != nullptr)
            delete AcpiTable;
        if (DecompressedBufferOnHeap != nullptr)
            delete[] DecompressedBufferOnHeap;
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

                DecompressedBufferOnHeap = new UINT8[decompressedSize];
                scratch = malloc(ScratchSize);
                status = UefiDecompress(data + HeaderSize, DecompressedBufferOnHeap, scratch);
                if (status != RETURN_SUCCESS) {
                    free(scratch);
                    throw exception();
                }
                DecodeDecompressedBuffer(DecompressedBufferOnHeap, decompressedSize);
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
            HeaderSize = sizeof(EFI_GUID_DEFINED_SECTION);
            if (isExtend) {
                HeaderSize = sizeof(EFI_GUID_DEFINED_SECTION2);
            }
            if (SectionDefinitionGuid == GuidDatabase::gEfiCertTypeRsa2048Sha256Guid) {
                HeaderSize = 0x228;
                DataOffset = HeaderSize;
                offset = HeaderSize;
                while (offset < SectionSize) {
                    ChildFile.push_back(new CommonSection(data + offset, offsetFromBegin + offset, this->ParentFFS, this->isCompressed));
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

                DecompressedBufferOnHeap = new UINT8[decompressedSize];
                scratch = malloc(ScratchSize);
                status = LzmaUefiDecompress(data + HeaderSize, SectionSize - HeaderSize, DecompressedBufferOnHeap, scratch);
                if (status != RETURN_SUCCESS) {
                    free(scratch);
                    throw exception();
                }
                DecodeDecompressedBuffer(DecompressedBufferOnHeap, decompressedSize);
                free(scratch);
            }
            break;
        case EFI_SECTION_PE32:
        case EFI_SECTION_TE:
            peCoffHeader = new PeCoff(data + HeaderSize, SectionSize - HeaderSize, offsetFromBegin + HeaderSize, this->isCompressed);
            break;
        case EFI_SECTION_USER_INTERFACE:
            UINT16* char16FileName;
            char16FileName = (UINT16*)this->getBytes(offset, SectionSize - HeaderSize);
            FileNameString = Buffer::wstringToString(char16FileName);
            delete[] char16FileName;
            break;
        case EFI_SECTION_VERSION:
            UINT16* char16VersionString;
            BuildNumber = this->getUINT16(offset);
            char16VersionString = (UINT16*)this->getBytes(offset + 2, SectionSize - HeaderSize - 2);
            VersionString = Buffer::wstringToString(char16VersionString);
            delete[] char16VersionString;
            break;
        case EFI_SECTION_FIRMWARE_VOLUME_IMAGE:
            ChildFile.push_back(new FirmwareVolume(data + HeaderSize, SectionSize - HeaderSize, offsetFromBegin + HeaderSize, false, this->isCompressed));
            break;
        case EFI_SECTION_DXE_DEPEX:
        case EFI_SECTION_PEI_DEPEX:
        case EFI_SECTION_MM_DEPEX:
            dependency = new Depex(data + HeaderSize, SectionSize - HeaderSize, this->isCompressed);
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
            else if (Elf::IsElfFormat(data + HeaderSize)) {
                isElfFormat = true;
                Elf elf = Elf(data + HeaderSize, SectionSize - HeaderSize, offsetFromBegin + HeaderSize, this->isCompressed);
                if (elf.isValid()) {
                    elf.decodeSections();
                    elf.setInfoStr();
                    AdditionalMsg = elf.InfoStr;
                    for (Volume* fv:elf.UpldFiles) {
                        ChildFile.push_back(fv);
                    }
                }
            } else if (FspHeader::isFspHeader(data + HeaderSize)) {
                isFspHeader = true;
                FspHeader fspH = FspHeader(data + HeaderSize, SectionSize - HeaderSize, offsetFromBegin + HeaderSize);
                if (fspH.isValid()) {
                    fspH.setInfoStr();
                    AdditionalMsg = fspH.InfoStr;
                }
            } else if (ACPI_Class::isAcpiHeader(data + HeaderSize, SectionSize - HeaderSize)) {
                isAcpiHeader = true;
                AcpiTable = new ACPI_Class(data + HeaderSize, SectionSize - HeaderSize, offsetFromBegin + HeaderSize, false);
                if (AcpiTable->isValid()) {
                    AcpiTable->setInfoStr();
                    AdditionalMsg = AcpiTable->InfoStr;
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
            auto DecompressedSection = new CommonSection(DecompressedBuffer + offset, SectionSize, offset, this->ParentFFS);
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
            case VolumeType::ELF:
                break;
            default:
                break;
            }
        }
    }

    bool CommonSection::CheckValidation() {
        UINT8 SecTpye = CommonHeader.Type;
        if ((SecTpye >= EFI_SECTION_ALL && SecTpye <= EFI_SECTION_DISPOSABLE) ||
            (SecTpye >= EFI_SECTION_PE32 && SecTpye <= EFI_SECTION_SMM_DEPEX) ||
            SecTpye != 0x1A) {
            if (!IS_SECTION2(&CommonHeader)) {
                if (SECTION_SIZE(&CommonHeader) < getHeaderSize()) {
                    return false;
                }
            } else {
                if (SECTION2_SIZE(&CommonHeader) < getHeaderSize()) {
                    return false;
                }
            }
            return true;
        }
        return false;
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
            ss << setw(width) << "FileName:"      << FileNameString << "\n";
            break;
        case EFI_SECTION_VERSION:
            ss << setw(width) << "BuildNumber:"   << hex << uppercase << BuildNumber << "h\n"
               << setw(width) << "Version:"       << VersionString << "\n";
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
            } else if (isElfFormat || isFspHeader || isAcpiHeader) {
                ss.clear();
                ss.str("");
                ss << AdditionalMsg.toStdString();
            }
            break;
        default:
            break;
        }

        string compressed = "No";
        if (this->isCompressed)
            compressed = "Yes";
        ss << setw(width) << "Compressed:" << compressed;
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
        }
        if (isExtend)
            headerSize += sizeof(UINT32);
        return headerSize;
    }

    FfsFile::FfsFile(UINT8* file, INT64 offset, bool Compressed):Volume(file, 0, offset, Compressed) {
        Type = VolumeType::FfsFile;

        // The FFS file header is the first part of the file
        FfsHeader = *(EFI_FFS_FILE_HEADER*)data;
        FfsSize = FFS_FILE_SIZE(&FfsHeader);

        // If the FFS file header's size is zero, then it's an extended header
        if (FfsSize == 0) {
            isExtended = true;
            FfsExtHeader = *(EFI_FFS_FILE_HEADER2*)data;
            FfsSize = (INT64)FfsExtHeader.ExtendedSize;
        }

        size = FfsSize;

        // Calculate the header checksum
        UINT8 headerSumValue = 0;
        if (isExtended) {
            headerSumValue = Buffer::CaculateSum8((UINT8*)&FfsExtHeader, sizeof(EFI_FFS_FILE_HEADER2));
            headerSumValue = headerSumValue + FfsExtHeader.State + FfsExtHeader.IntegrityCheck.Checksum.File;
        } else {
            headerSumValue = Buffer::CaculateSum8((UINT8*)&FfsHeader, sizeof(EFI_FFS_FILE_HEADER));
            headerSumValue = headerSumValue + FfsHeader.State + FfsHeader.IntegrityCheck.Checksum.File;
        }
        if (headerSumValue == 0)
            headerChecksumValid = true;

        // Calculate the data checksum
        if ((FfsHeader.Attributes | FFS_ATTRIB_CHECKSUM) == 0x0)
            dataChecksumValid = true;

        // Check if the file is an Apriori file
        if (FfsHeader.Name.Data1 == GuidDatabase::gPeiAprioriFileNameGuid.Data1 || FfsHeader.Name.Data1 == GuidDatabase::gAprioriGuid.Data1) {
            isApriori = true;
        }
    }

    FfsFile::~FfsFile() {
        for(auto section:Sections) {
            delete section;
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
            // Apriori files are encoded as raw CommonSections
            CommonSection *Sec = new CommonSection(data + offset, offsetFromBegin + offset, this, this->isCompressed);
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
            CommonSection *Sec = new CommonSection(data + offset, SecSize, offsetFromBegin + offset, this, this->isCompressed);
            if (!Sec->CheckValidation()) {
                delete Sec;
                break;
            }
            Sec->SelfDecode();
            Sections.push_back(Sec);
            if (offset + SecSize > offset) {
                // Align to 4 bytes
                offset += SecSize;
                Buffer::Align(offset, 0, 0x4);
            } else {
                break;
            }
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

        string compressed = "No";
        if (this->isCompressed)
            compressed = "Yes";
        ss << setw(width) << "Compressed:" << compressed;

        InfoStr = QString::fromStdString(ss.str());
    }

    FirmwareVolume::FirmwareVolume(UINT8* fv, INT64 length, INT64 offset, bool empty, bool Compressed):Volume(fv, length, offset, Compressed), isEmpty(empty) {
        if (length < 0x40) { // Minimum size of a firmware volume
            isCorrupted = true;
            isEmpty = true;
            return;
        }

        // Check if this is a valid firmware volume, and if not, mark it as empty
        if (!isValidFirmwareVolume((EFI_FIRMWARE_VOLUME_HEADER*)data)) {
            isEmpty = true;
        }

        // Initialize the base class
        Type = VolumeType::FirmwareVolume;
        FirmwareVolumeHeader = *(EFI_FIRMWARE_VOLUME_HEADER*)data;

        // Check that the firmware volume is not longer than the volume
        if (FirmwareVolumeHeader.FvLength > length) {
            isCorrupted = true;
        }

        // Check if the firmware volume has an extended header
        if (FirmwareVolumeHeader.ExtHeaderOffset == 0) {
            FirmwareVolumeSize = 0x48;
            isExt = false;
        } else {
            EFI_FFS_FILE_HEADER *ExtFvFfs = (EFI_FFS_FILE_HEADER*)(data + 0x48);
            FirmwareVolumeSize = 0x48 + FFS_FILE_SIZE(ExtFvFfs);
            if (length < FirmwareVolumeHeader.ExtHeaderOffset) {
                isCorrupted = true;
                return;
            }
            FirmwareVolumeExtHeader = *(EFI_FIRMWARE_VOLUME_EXT_HEADER*)(data + FirmwareVolumeHeader.ExtHeaderOffset);
            isExt = true;
        }

        // Check if the firmware volume is an NV volume
        if (FirmwareVolumeHeader.FileSystemGuid == GuidDatabase::gEfiSystemNvDataFvGuid) {
            isNv = true;
        }

        // Check if the firmware volume has a valid checksum
        UINT16 sumValue = Buffer::CaculateSum16((UINT16*)&FirmwareVolumeHeader, sizeof(EFI_FIRMWARE_VOLUME_HEADER) / sizeof (UINT16));
        if (sumValue == 0)
            checksumValid = true;
    }

    FirmwareVolume::~FirmwareVolume() {
        for (auto file:FfsFiles) {
            delete file;
        }
        if (freeSpace != nullptr) {
            delete freeSpace;
        }
        if (NvStorage != nullptr) {
            delete NvStorage;
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

    void FirmwareVolume::decodeFfs(bool multithread) {
        INT64 offset = FirmwareVolumeSize;
        Buffer::Align(offset, 0, 0x8);
        if (isEmpty || isCorrupted || !checksumValid)
            return;
        if (isNv) {
            NvStorage = new NvStorageVariable(data + offset, offsetFromBegin + offset);
            return;
        }
        vector<thread> threadPool;
        atomic_flag spinlock = ATOMIC_FLAG_INIT;
        while (offset < size) {
            EFI_FFS_FILE_HEADER  FfsHeader = *(EFI_FFS_FILE_HEADER*)(data + offset);
            INT64 FfsSize = FFS_FILE_SIZE(&FfsHeader);

            // If the size of the current FFS file is 0xFFFFFF and the file is marked as deleted (0xFF),
            // then the current FFS file is not used and can be overwritten.
            if (FfsSize == 0xFFFFFF && FfsHeader.State == 0xFF) {
                freeSpace = new Volume(data + offset, size - offset, offsetFromBegin + offset, this->isCompressed);
                break;
            }

            auto FvDecoder = [this, &spinlock](INT64 off) {
                FfsFile *Ffs = new FfsFile(data + off, offsetFromBegin + off, this->isCompressed);

                // The following file types have sections that can be decoded:
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
                case EFI_FV_FILETYPE_RAW:
                    Ffs->decodeSections();
                    break;
                default:
                    break;
                }
                while (spinlock.test_and_set()) {}
                FfsFiles.push_back(Ffs);
                spinlock.clear();
            };

            // If multithreading is enabled, then create a thread that will decode the sections of the current FFS file.
            if (multithread) {
                threadPool.emplace_back(FvDecoder, offset);
            } else {
                FvDecoder(offset);
            }

            // If the current FFS file size is valid, then update the offset to point to the next FFS file.
            if (offset + FfsSize > offset){
                offset += FfsSize;
                Buffer::Align(offset, 0, 0x8);
            } else {
                // Otherwise, the current FFS file is not valid, so create a volume that represents the free space
                // between the current FFS file and the end of the FV.
                freeSpace = new Volume(data + offset, size - offset, offsetFromBegin + offset, this->isCompressed);
                break;
            }
        }
        if (multithread) {
            for (thread &t:threadPool) {
                t.join();
            }
            // Bug here, occasionally hang
            std::sort(FfsFiles.begin(), FfsFiles.end(), [](FfsFile *f1, FfsFile *f2) { return f1->offsetFromBegin < f2->offsetFromBegin; });
        }
    }

    INT64 FirmwareVolume::getHeaderSize() const {
        return FirmwareVolumeSize;
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
           << setw(width) << "Checksum:"    << hex << uppercase << FirmwareVolumeHeader.Checksum << "h";

        if (checksumValid) {
            ss << " (Valid)\n";
        } else {
            ss << " (Invalid)\n";
        }

        if (isExt) {
            ss << "Extended header size:" << hex << FirmwareVolumeExtHeader.ExtHeaderSize << "h\n"
               << "Volume GUID:\n" << GUID(FirmwareVolumeExtHeader.FvName).str(true) << "\n";
        }

        string compressed = "No";
        if (this->isCompressed)
            compressed = "Yes";
        ss << setw(width) << "Compressed:" << compressed;

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

    NvVariableEntry::NvVariableEntry(UINT8* fv, INT64 offset, bool isAuth):Volume(fv, 0, offset), AuthFlag(isAuth) {
        if (AuthFlag) {
            AuthVariableHeader = (AUTHENTICATED_VARIABLE_HEADER*)fv;
            size = sizeof(AUTHENTICATED_VARIABLE_HEADER) + AuthVariableHeader->NameSize + AuthVariableHeader->DataSize;
            VariableName = Buffer::wcharToString((CHAR16*)(AuthVariableHeader + 1), AuthVariableHeader->NameSize, true);
            DataPtr = fv + sizeof(AUTHENTICATED_VARIABLE_HEADER) + AuthVariableHeader->NameSize;
            DataSize = AuthVariableHeader->DataSize;
        }
        else {
            VariableHeader = (VARIABLE_HEADER*)fv;
            size = sizeof(VARIABLE_HEADER) + VariableHeader->NameSize + VariableHeader->DataSize;
            Buffer::wcharToString((CHAR16*)(AuthVariableHeader + 1), VariableHeader->NameSize, true);
            DataPtr = fv + sizeof(VARIABLE_HEADER) + VariableHeader->NameSize;
            DataSize = VariableHeader->DataSize;
        }
    }

    INT64 NvVariableEntry::getHeaderSize() const {
        if (AuthFlag)
            return sizeof(AUTHENTICATED_VARIABLE_HEADER);
        else
            return sizeof(VARIABLE_HEADER);
    }

    void NvVariableEntry::setInfoStr() {
        INT64 width = 15;
        stringstream ss;
        ss.setf(ios::left);

        if (AuthFlag) {
            ss << "Variable GUID:\n" << GUID(AuthVariableHeader->VendorGuid).str(true) << "\n"
               << setw(width) << "Variable Name:"   << VariableName << "\n"
               << setw(width) << "Variable Size:"   << hex << DataSize << "h\n";
        } else {
            ss << "Variable GUID:\n" << GUID(VariableHeader->VendorGuid).str(true) << "\n"
               << setw(width) << "Variable Name:"   << VariableName << "\n"
               << setw(width) << "Variable Size:"   << hex << DataSize << "h\n";
        }
        InfoStr = QString::fromStdString(ss.str());
    }

    NvStorageVariable::NvStorageVariable(UINT8* fv, INT64 offset):Volume(fv, 0, offset), AuthFlag(false) {
        NvStoreHeader = *(VARIABLE_STORE_HEADER*)fv;
        size = NvStoreHeader.Size;
        if (NvStoreHeader.Signature == GuidDatabase::gEfiAuthenticatedVariableGuid) {
            AuthFlag = true;
        }
        INT64 VariableOffset = sizeof(VARIABLE_STORE_HEADER);
        while (VariableOffset < size) {
            if (*(UINT16*)(fv + VariableOffset) != 0x55AA)
                break;
            NvVariableEntry *VarEntry = new NvVariableEntry(fv + VariableOffset, offsetFromBegin + VariableOffset, AuthFlag);
            VariableList.push_back(VarEntry);
            VariableOffset += VarEntry->size;
            Buffer::Align(VariableOffset, 0, 4);
        }
    }

    NvStorageVariable::~NvStorageVariable() {
        for(NvVariableEntry* VarEntry:VariableList)
            delete VarEntry;
    }

    void NvStorageVariable::setInfoStr() {
        INT64 width = 15;
        stringstream ss;
        ss.setf(ios::left);

        ss << "Signature:\n" << GUID(NvStoreHeader.Signature).str(true) << "\n"
           << setw(width) << "Full size:"   << hex << uppercase << NvStoreHeader.Size << "h\n"
           << setw(width) << "Header size:" << hex << uppercase << sizeof(VARIABLE_STORE_HEADER) << "h\n"
           << setw(width) << "Format:"      << hex << uppercase << NvStoreHeader.Format << "h\n"
           << setw(width) << "State:"       << hex << uppercase << (UINT32)NvStoreHeader.State << "h\n";
        InfoStr = QString::fromStdString(ss.str());
    }

    FspHeader::FspHeader(UINT8* fv, INT64 length, INT64 offset):Volume(fv, length, offset) {
        UINT32 s = sizeof(TABLES);
        if (s != length) {
            validFlag = false;
        }
        mTable = *(TABLES*)fv;
        if ((mTable.FspInfoHeader.Signature == FSP_INFO_HEADER_SIGNATURE) &&
            (mTable.FspInfoExtendedHeader.Signature == FSP_INFO_EXTENDED_HEADER_SIGNATURE) &&
            (mTable.FspPatchTable.Signature == FSP_PATCH_TABLE_SIGNATURE)) {
            validFlag = true;
        } else {
            validFlag = false;
        }
    }

    FspHeader::~FspHeader() {}

    bool FspHeader::isValid() const {
        return validFlag;
    }

    void FspHeader::setInfoStr() {
        INT64 width = 25;
        stringstream ss;
        ss.setf(ios::left);
        ss << setw(width) << "SpecVersion:"      << hex << uppercase << (UINT16)mTable.FspInfoHeader.SpecVersion << "h\n"
           << setw(width) << "HeaderRevision:"   << hex << uppercase << (UINT16)mTable.FspInfoHeader.HeaderRevision << "h\n"
           << setw(width) << "ImageRevision:"    << hex << uppercase << (UINT16)mTable.FspInfoHeader.ImageRevision << "h\n"
           << setw(width) << "ImageId:"          << hex << uppercase << Buffer::charToString(mTable.FspInfoHeader.ImageId, 8)  << "\n"
           << setw(width) << "ImageSize:"        << hex << uppercase << mTable.FspInfoHeader.ImageSize << "h\n"
           << setw(width) << "ImageBase:"        << hex << uppercase << mTable.FspInfoHeader.ImageBase << "h\n"
           << setw(width) << "ImageAttribute:"            << hex << uppercase << mTable.FspInfoHeader.ImageAttribute << "h\n"
           << setw(width) << "ComponentAttribute:"        << hex << uppercase << mTable.FspInfoHeader.ComponentAttribute << "h\n"
           << setw(width) << "CfgRegionOffset:"           << hex << uppercase << mTable.FspInfoHeader.CfgRegionOffset << "h\n"
           << setw(width) << "CfgRegionSize:"             << hex << uppercase << mTable.FspInfoHeader.CfgRegionSize << "h\n"
           << setw(width) << "TempRamInitEntryOffset:"    << hex << uppercase << mTable.FspInfoHeader.TempRamInitEntryOffset << "h\n"
           << setw(width) << "NotifyPhaseEntryOffset:"    << hex << uppercase << mTable.FspInfoHeader.NotifyPhaseEntryOffset << "h\n"
           << setw(width) << "FspMemoryInitEntryOffset:"  << hex << uppercase << mTable.FspInfoHeader.FspMemoryInitEntryOffset << "h\n"
           << setw(width) << "TempRamExitEntryOffset:"    << hex << uppercase << mTable.FspInfoHeader.TempRamExitEntryOffset << "h\n"
           << setw(width) << "FspSiliconInitEntryOffset:" << hex << uppercase << mTable.FspInfoHeader.FspSiliconInitEntryOffset << "h\n"
           << setw(width) << "FspMultiPhaseSiInitEntryOffset:"   << hex << uppercase << mTable.FspInfoHeader.FspMultiPhaseSiInitEntryOffset << "h\n"
           << setw(width) << "ExtendedImageRevision:"     << hex << uppercase << mTable.FspInfoHeader.ExtendedImageRevision << "h\n"
           << setw(width) << "FspMultiPhaseMemInitEntryOffset:"  << hex << uppercase << mTable.FspInfoHeader.FspMultiPhaseMemInitEntryOffset << "h\n"
           << setw(width) << "FspSmmInitEntryOffset:"     << hex << uppercase << mTable.FspInfoHeader.FspSmmInitEntryOffset << "h\n"
           << setw(width) << "FspProducerId:"             << hex << uppercase << Buffer::charToString(mTable.FspInfoExtendedHeader.FspProducerId, 6) << "\n"
           << setw(width) << "FspProducerRevision:"       << hex << uppercase << mTable.FspInfoExtendedHeader.FspProducerRevision << "h\n"
           << setw(width) << "FspProducerDataSize:"       << hex << uppercase << mTable.FspInfoExtendedHeader.FspProducerDataSize << "h\n"
           << setw(width) << "BuildTimeStamp:"            << hex << uppercase << mTable.FspProduceDataType1.BuildTimeStamp << "\n"
           << setw(width) << "PatchEntryNum:"             << hex << uppercase << mTable.FspPatchTable.PatchEntryNum << "h\n";
        InfoStr = QString::fromStdString(ss.str());
    }

    bool FspHeader::isFspHeader(const UINT8 *ImageBase) {
        UINT32 *signature = (UINT32*)ImageBase;
        if (*signature == FSP_INFO_HEADER_SIGNATURE) {
            return true;
        }
        return false;
    }

    BiosImageVolume::BiosImageVolume(UINT8* fv, INT64 length, INT64 offset):Volume(fv, length, offset) {
        // Initialize the FIT table.
        try {
            FitTable = new FitTableClass(fv, length);
            const INT64 IBB_length = 0x400000;
            if (length >= IBB_length * 2) {
                // Check for resiliency.
                INT64 IBB_begin_address = length - IBB_length;
                INT64 IBBR_begin_address = length - IBB_length * 2;
                isResiliency = true;
                for (int index = 0; index < IBB_length; ++index) {
                    if (fv[IBB_begin_address + index] != fv[IBBR_begin_address + index]) {
                        isResiliency = false;
                        break;
                    }
                }
            }
        } catch (...) {
            FitTable = nullptr;
        }
    }

    BiosImageVolume::~BiosImageVolume() {
        if (FitTable != nullptr)
            delete FitTable;
        delete[] data;
    }

    void BiosImageVolume::setDebugFlag() {
        INT64 pos = BiosID.find(".");
        if (pos != string::npos && pos + 1 < BiosID.size()) {
            if (BiosID[pos + 1] == 'R') {
                DebugFlag = false;
            }
            else if (BiosID[pos + 1] == 'D') {
                DebugFlag = true;
            }
        }
    }

    void BiosImageVolume::decodeBiosRegion() {
        INT32 NV_index;
        for (NV_index = 0; NV_index < FvData->size(); ++NV_index) {
            FirmwareVolume *fv = FvData->at(NV_index);
            if (fv->getFvGuid().GuidData == guidData->gEfiSystemNvDataFvGuid) {
                NV_Region.first = fv->offsetFromBegin;
                NV_Region.second = fv->size;
                break;
            }
        }
        OBB_Region.first = NV_Region.first + NV_Region.second;

        INT32 OBB_index;
        INT64 OBB_size = 0;
        for (OBB_index = NV_index + 1; OBB_index < FvData->size(); ++OBB_index) {
            FirmwareVolume *fv = FvData->at(OBB_index);
            OBB_size += fv->getSize();
            UINT8 digest[SHA256_DIGEST_LENGTH];
            SHA256(this->data + OBB_Region.first - this->offsetFromBegin, OBB_size, digest);
            if (memcmp(ObbDigest, digest, SHA256_DIGEST_LENGTH) == 0) {
                OBB_Region.second = OBB_size;
                break;
            }
        }

        if (isResiliency) {
            IBBR_Region.first = OBB_Region.first + OBB_Region.second;
            IBBR_Region.second = 0x400000;
            IBB_Region.first = IBBR_Region.first + IBBR_Region.second;
            IBB_Region.second = 0x400000;
        } else {
            IBB_Region.first = OBB_Region.first + OBB_Region.second;
            IBB_Region.second = this->size - NV_Region.second - OBB_Region.second;
        }

        for (FirmwareVolume *fv:*FvData) {
            decodeAcpiTable(fv);
        }
    }

    void BiosImageVolume::decodeAcpiTable(Volume* Vol) {
        switch (Vol->Type) {
        case VolumeType::FirmwareVolume:
            for (FfsFile *ffs:((FirmwareVolume*)Vol)->FfsFiles) {
                decodeAcpiTable(ffs);
            }
            break;
        case VolumeType::FfsFile:
            for (CommonSection *sec:((FfsFile*)Vol)->Sections) {
                decodeAcpiTable(sec);
            }
            break;
        case VolumeType::CommonSection:
            if (((CommonSection*)Vol)->isAcpiHeader) {
                AcpiTables.push_back(((CommonSection*)Vol)->AcpiTable);
            }
            for (Volume *child:((CommonSection*)Vol)->ChildFile) {
                decodeAcpiTable(child);
            }
            break;
        default:
            break;
        }
    }

    string BiosImageVolume::getFlashmap() {
        decodeBiosRegion();
        INT64 width = 8;
        stringstream ss;
        ss.setf(ios::right);
        ss << setw(width) << setfill('0') << hex << uppercase << offsetFromBegin << "      ";
        ss << setw(width) << setfill('0') << hex << uppercase << offsetFromBegin + size - 1 << "      ";
        ss << setw(width) << setfill('0') << hex << uppercase << size << "      BIOS Region\n";

        if (ObbDigestValid) {
            ss << setw(width) << setfill('0') << hex << uppercase << NV_Region.first << "      ";
            ss << setw(width) << setfill('0') << hex << uppercase << NV_Region.first + NV_Region.second - 1 << "      ";
            ss << setw(width) << setfill('0') << hex << uppercase << NV_Region.second << "         NV Region\n";

            ss << setw(width) << setfill('0') << hex << uppercase << OBB_Region.first << "      ";
            ss << setw(width) << setfill('0') << hex << uppercase << OBB_Region.first + OBB_Region.second - 1 << "      ";
            ss << setw(width) << setfill('0') << hex << uppercase << OBB_Region.second << "         OBB Region\n";

            if (isResiliency) {
                ss << setw(width) << setfill('0') << hex << uppercase << IBBR_Region.first << "      ";
                ss << setw(width) << setfill('0') << hex << uppercase << IBBR_Region.first + IBBR_Region.second - 1 << "      ";
                ss << setw(width) << setfill('0') << hex << uppercase << IBBR_Region.second << "         IBBR Region\n";
            }

            ss << setw(width) << setfill('0') << hex << uppercase << IBB_Region.first << "      ";
            ss << setw(width) << setfill('0') << hex << uppercase << IBB_Region.first + IBB_Region.second - 1 << "      ";
            ss << setw(width) << setfill('0') << hex << uppercase << IBB_Region.second << "         IBB Region\n";
        }
        return ss.str();
    }

    void BiosImageVolume::setBiosID() {
        for (size_t idx = FvData->size(); idx > 0; --idx) {
            FirmwareVolume *volume = FvData->at(idx - 1);
            for(auto file:volume->FfsFiles) {
                if (file->FfsHeader.Name == GuidDatabase::gBiosIdGuid) {
                    if (file->Sections.size() == 0)
                        return;
                    CommonSection *sec = file->Sections.at(0);
                    CHAR16 *biosIdStr = (CHAR16*)(sec->data + sizeof(EFI_COMMON_SECTION_HEADER) + 8);
                    BiosID = Buffer::wstringToString(biosIdStr);
                    foundBiosID = true;
                    setDebugFlag();
                    return;
                }
            }
        }
    }

    void BiosImageVolume::getObbDigest() {
        for (size_t idx = FvData->size(); idx > 0; --idx) {
            FirmwareVolume *volume = FvData->at(idx - 1);
            for(auto file:volume->FfsFiles) {
                if (file->FfsHeader.Name == GuidDatabase::gObbSha256HashBinGuid) {
                    if (file->Sections.size() == 0)
                        return;
                    CommonSection *sec = file->Sections.at(0);
                    if (sec->getSize() != sizeof(EFI_COMMON_SECTION_HEADER) + SHA256_DIGEST_LENGTH)
                        return;
                    UINT8 *digest = (UINT8*)(sec->data + sizeof(EFI_COMMON_SECTION_HEADER));
                    memcpy(ObbDigest, digest, SHA256_DIGEST_LENGTH);
                    ObbDigestValid = true;
                    return;
                }
            }
        }
    }

    void BiosImageVolume::setInfoStr() {
        INT64 width = 15;
        stringstream ss;
        ss.setf(ios::left);

        if (foundBiosID) {
            ss << setw(width) << "BIOS ID:" << BiosID << "\n\n";
            if (isResiliency)
                ss << "Resiliency ";
            if (DebugFlag)
                ss << "Debug BIOS" << "\n";
            else
                ss << "Release BIOS" << "\n";
        }

        InfoStr = QString::fromStdString(ss.str());
    }

    FitTableClass::FitTableClass(UINT8* fv, INT64 length) {
        INT64 FitTableAddress = *(INT64*)(fv + length - DEFAULT_FIT_TABLE_POINTER_OFFSET) & 0xFFFFFF;
        FitTableAddress = Buffer::adjustBufferAddress(0x1000000, FitTableAddress, length); // get the relative address of FIT table
        if (FitTableAddress > length || FitTableAddress < 0) {
            throw exception("NO FIT table!");
        }
        FitHeader = *(FIRMWARE_INTERFACE_TABLE_ENTRY*)(fv + FitTableAddress);
        UINT64 FitSignature = FitHeader.Address;
        if (FitSignature == (UINT64)FIT_SIGNATURE) {
            isValid = true;
            FitEntryNum = *(UINT32*)(FitHeader.Size) & 0xFFFFFF;

            UINT8 Checksum = Buffer::CaculateSum8((UINT8 *)(fv + FitTableAddress), sizeof(FIRMWARE_INTERFACE_TABLE_ENTRY) * FitEntryNum);
            if (Checksum == 0) {
                isChecksumValid = true;
            }

            for (INT64 index = 1; index < FitEntryNum; ++index) {
                FIRMWARE_INTERFACE_TABLE_ENTRY FitEntry = *(FIRMWARE_INTERFACE_TABLE_ENTRY*)(fv + FitTableAddress + sizeof(FIRMWARE_INTERFACE_TABLE_ENTRY) * index);
                FitEntries.push_back(FitEntry);
                if (FitEntry.Type == FIT_TABLE_TYPE_MICROCODE) {
                    UINT64 MicrocodeAddress = FitEntry.Address & 0xFFFFFF;
                    UINT64 RelativeMicrocodeAddress = Buffer::adjustBufferAddress(0x1000000, MicrocodeAddress, length);
                    if (RelativeMicrocodeAddress > (UINT64)length)
                        continue;
                    MicrocodeHeaderClass *MicrocodeEntry = new MicrocodeHeaderClass(fv + RelativeMicrocodeAddress, MicrocodeAddress);
                    MicrocodeEntries.push_back(MicrocodeEntry);
                } else if (FitEntry.Type == FIT_TABLE_TYPE_STARTUP_ACM) {
                    UINT64 AcmAddress = FitEntry.Address & 0xFFFFFF;
                    UINT64 RelativeAcmAddress = Buffer::adjustBufferAddress(0x1000000, AcmAddress, length);
                    if (RelativeAcmAddress > (UINT64)length)
                        continue;
                    AcmHeaderClass *AcmEntry = new AcmHeaderClass(fv + RelativeAcmAddress, AcmAddress);
                    AcmEntries.push_back(AcmEntry);
                } else if (FitEntry.Type == FIT_TABLE_TYPE_KEY_MANIFEST) {
                    // Use external BpmGen2 tool to parse KM and BPM info
                    continue;
//                    UINT64 KmAddress = FitEntry.Address & 0xFFFFFF;
//                    UINT64 RelativeKmAddress = Buffer::adjustBufferAddress(0x1000000, KmAddress, length);
//                    if (RelativeKmAddress > (UINT64)length && KmEntry != nullptr)
//                        continue;
//                    KmEntry = new KeyManifestClass(fv + RelativeKmAddress, length - RelativeKmAddress);
                } else if (FitEntry.Type == FIT_TABLE_TYPE_BOOT_POLICY_MANIFEST) {
                    continue;
//                    UINT64 BpmAddress = FitEntry.Address & 0xFFFFFF;
//                    UINT64 RelativeBpmAddress = Buffer::adjustBufferAddress(0x1000000, BpmAddress, length);
//                    INT64 FitEntrySize = (FitEntry.Size[2] << 16) + (FitEntry.Size[1] << 8) + FitEntry.Size[0];
//                    if (RelativeBpmAddress > (UINT64)length && BpmEntry != nullptr && RelativeBpmAddress + (UINT64)FitEntrySize > (UINT64)length)
//                        continue;
//                    BpmEntry = new BootPolicyManifestClass(fv + RelativeBpmAddress, FitEntrySize);
                }
            }
        } else {
            isValid = false;
            throw exception("NO FIT table!");
        }
    }

    FitTableClass::~FitTableClass() {
        for (auto MicrocodeEntry:MicrocodeEntries)
            delete MicrocodeEntry;
        for (auto AcmEntry:AcmEntries)
            delete AcmEntry;
        if (KmEntry != nullptr)
            delete KmEntry;
        if (BpmEntry != nullptr)
            delete BpmEntry;
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
        if (ExtendedTableLength != 0) {
            ExtendedTableHeader = (CPU_MICROCODE_EXTENDED_TABLE_HEADER *)(fv + microcodeHeader.DataSize + sizeof(CPU_MICROCODE_HEADER));
            if ((ExtendedTableLength > sizeof(CPU_MICROCODE_EXTENDED_TABLE_HEADER)) && ((ExtendedTableLength & 0x3) == 0)) {
                UINT32 CheckSum32 = Buffer::CaculateSum32((UINT32 *)ExtendedTableHeader, ExtendedTableLength);
                if (CheckSum32 != 0)
                    return;
                UINT32 ExtendedTableCount = ExtendedTableHeader->ExtendedSignatureCount;
                if (ExtendedTableCount <= (ExtendedTableLength - sizeof(CPU_MICROCODE_EXTENDED_TABLE_HEADER)) / sizeof(CPU_MICROCODE_EXTENDED_TABLE)) {
                    CPU_MICROCODE_EXTENDED_TABLE *ExtendedTable = (CPU_MICROCODE_EXTENDED_TABLE *)(ExtendedTableHeader + 1);
                    for (UINT32 Index = 0; Index < ExtendedTableCount; Index++) {
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
