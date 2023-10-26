//
// Created by stephan on 8/29/2023.
//

#include "BaseLib.h"
#include "CommonSection.h"
#include "FirmwareVolume.h"
#include "UEFI/GuidDatabase.h"
#include "UEFI/PiDependency.h"
#include "LzmaDecompress/LzmaDecompressLib.h"
#include "BaseUefiDecompress/BaseUefiDecompressLib.h"
#include "BrotliDecompress/BrotliDecompressLib.h"
#include "Feature/AcpiClass.h"
#include "Elf.h"

using namespace BaseLibrarySpace;

PeCoff::PeCoff(UINT8 *file, INT64 length):
        data(file), size(length)
{
    UINT16 magic = *(UINT16*)file;
    if (magic == EFI_IMAGE_DOS_SIGNATURE) {
        dosHeader = *(EFI_IMAGE_DOS_HEADER*)file;
        pe32Header = *(EFI_IMAGE_NT_HEADERS32*)(file + dosHeader.e_lfanew);
        if (pe32Header.OptionalHeader.Magic == EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            isPe32Plus = true;
            pe32plusHeader = *(EFI_IMAGE_NT_HEADERS64*)(file + dosHeader.e_lfanew);
        }
    } else if (magic == EFI_TE_IMAGE_HEADER_SIGNATURE) {
        isTE = true;
        teHeader = *(EFI_TE_IMAGE_HEADER*)file;
    } else
        throw BiosException("Wrong Magic number");
}

string PeCoff::getMachineType() const {
    UINT16 machine;
    if (isTE)
        machine = teHeader.Machine;
    else
        machine = pe32Header.FileHeader.Machine;
    switch (machine) {
        case IMAGE_FILE_MACHINE_I386:
            return "x86";
        case IMAGE_FILE_MACHINE_EBC:
            return "EBC";
        case IMAGE_FILE_MACHINE_X64:
            return "x86_64";
        case IMAGE_FILE_MACHINE_ARM:
            return "ARM";
        case IMAGE_FILE_MACHINE_ARMT:
            return "ARMT";
        case IMAGE_FILE_MACHINE_ARM64:
            return "ARM64";
        case IMAGE_FILE_MACHINE_RISCV64:
            return "RISC-V";
        case IMAGE_FILE_MACHINE_LOONGARCH64:
            return "LoongArch";
        default:
            break;
    }
    return "";
}

string PeCoff::getSubsystemName(UINT16 subsystem) {
    string SubSystemName;
    switch (subsystem) {
        case EFI_IMAGE_SUBSYSTEM_UNKNOWN:
            SubSystemName = "Unknown";
            break;
        case EFI_IMAGE_SUBSYSTEM_NATIVE:
            SubSystemName = "Native";
            break;
        case EFI_IMAGE_SUBSYSTEM_WINDOWS_GUI:
            SubSystemName = "Windows GUI";
            break;
        case EFI_IMAGE_SUBSYSTEM_WINDOWS_CUI:
            SubSystemName = "Windows CUI";
            break;
        case EFI_IMAGE_SUBSYSTEM_OS2_CUI:
            SubSystemName = "OS2 CUI";
            break;
        case EFI_IMAGE_SUBSYSTEM_POSIX_CUI:
            SubSystemName = "POSIX CUI";
            break;
        case EFI_IMAGE_SUBSYSTEM_EFI_APPLICATION:
            SubSystemName = "Application";
            break;
        case EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
            SubSystemName = "Boot Service Driver";
            break;
        case EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
            SubSystemName = "Runtime Driver";
            break;
        case EFI_IMAGE_SUBSYSTEM_SAL_RUNTIME_DRIVER:
            SubSystemName = "SAL Runtime Driver";
            break;
        default:
            break;
    }
    return SubSystemName;
}

Depex::Depex(UINT8 *file, INT64 length):
        data(file), size(length)
{
    INT64 offset = 0;
    UINT8 Opcode = *(UINT8*)(data + offset);
    offset += sizeof(UINT8);
    while (Opcode != EFI_DEP_END) {
        if (Opcode == EFI_DEP_PUSH) {
            EFI_GUID depexGuid = *(EFI_GUID*)(data + offset);
            offset += sizeof(EFI_GUID);
            OrganizedDepexList.push_back(guidData->getNameFromGuid(depexGuid));
        } else if (Opcode == EFI_DEP_BEFORE || Opcode == EFI_DEP_AFTER || Opcode == EFI_DEP_TRUE || Opcode == EFI_DEP_FALSE) {
            OrganizedDepexList.push_back(getOpcodeString(Opcode));
        } else if (Opcode == EFI_DEP_NOT) {
            string temp = OrganizedDepexList.at(OrganizedDepexList.size() - 1);
            OrganizedDepexList.pop_back();
            temp.insert(0, "NOT ");
            OrganizedDepexList.push_back(temp);
        } else if (Opcode == EFI_DEP_AND) {
            string top = OrganizedDepexList.at(OrganizedDepexList.size() - 1);
            OrganizedDepexList.pop_back();
            string second = OrganizedDepexList.at(OrganizedDepexList.size() - 1);
            OrganizedDepexList.pop_back();
            std::stringstream ss;
            ss << second << "\nAND\n" << top;
            string newDepex = ss.str();
            OrganizedDepexList.push_back(newDepex);
        } else if (Opcode == EFI_DEP_OR) {
            string top = OrganizedDepexList.at(OrganizedDepexList.size() - 1);
            OrganizedDepexList.pop_back();
            string second = OrganizedDepexList.at(OrganizedDepexList.size() - 1);
            OrganizedDepexList.pop_back();
            std::stringstream ss;
            ss << second << "\nOR\n" << top;
            string newDepex = ss.str();
            OrganizedDepexList.push_back(newDepex);
        }
        Opcode = *(UINT8*)(data + offset);
        offset += sizeof(UINT8);
    }
}

string Depex::getOpcodeString(UINT8 op) {
    string opStr;
    switch (op) {
        case EFI_DEP_BEFORE:
            opStr = "Before";
            break;
        case EFI_DEP_AFTER:
            opStr = "After";
            break;
        case EFI_DEP_PUSH:
            opStr = "Push";
            break;
        case EFI_DEP_AND:
            opStr = "And";
            break;
        case EFI_DEP_OR:
            opStr = "Or";
            break;
        case EFI_DEP_NOT:
            opStr = "Not";
            break;
        case EFI_DEP_TRUE:
            opStr = "True";
            break;
        case EFI_DEP_FALSE:
            opStr = "False";
            break;
        case EFI_DEP_END:
            opStr = "End";
            break;
        default:
            throw BiosException("Invalid opcode");
    }
    return opStr;
}

CommonSection::CommonSection(UINT8 *file, INT64 length, INT64 offset, bool Compressed, Volume *parent):
        Volume(file, length, offset, Compressed, parent) {}

CommonSection::~CommonSection() {
    safeDelete(peCoffHeader);
    safeDelete(dependency);
}

bool CommonSection::CheckValidation() {
    return Volume::CheckValidation();
}

INT64 CommonSection::SelfDecode() {
    Type = VolumeType::CommonSection;
    CommonHeader = *(EFI_COMMON_SECTION_HEADER*)data;
    HeaderSize = sizeof(EFI_COMMON_SECTION_HEADER);
    size = SECTION_SIZE(&CommonHeader);
    if (IS_SECTION2(&CommonHeader)) {
        isExtSection = true;
        auto *ExtHeader = (EFI_COMMON_SECTION_HEADER2*)data;
        HeaderSize = sizeof(EFI_COMMON_SECTION_HEADER2);
        size = SECTION2_SIZE(ExtHeader);
    }

    switch (CommonHeader.Type) {
        case EFI_SECTION_COMPRESSION:
        case EFI_SECTION_FREEFORM_SUBTYPE_GUID:
        case EFI_SECTION_GUID_DEFINED:
        case EFI_SECTION_PE32:
        case EFI_SECTION_TE:
        case EFI_SECTION_USER_INTERFACE:
        case EFI_SECTION_VERSION:
        case EFI_SECTION_FIRMWARE_VOLUME_IMAGE:
        case EFI_SECTION_DXE_DEPEX:
        case EFI_SECTION_PEI_DEPEX:
        case EFI_SECTION_MM_DEPEX:
        case EFI_SECTION_RAW:
            break;
        default:
            // Invalid Section Type
            return 0;
    }

    return size;
}

void CommonSection::DecodeChildVolume() {
    INT64 offset = HeaderSize;
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
                status = UefiDecompressGetInfo(data + HeaderSize, size - HeaderSize, &decompressedSize, &ScratchSize);
                if (status != RETURN_SUCCESS) {
                    throw exception();
                }

                DecompressedBufferOnHeap = new UINT8[decompressedSize];
                scratch = malloc(ScratchSize);
                status = UefiTianoDecompress(data + HeaderSize, DecompressedBufferOnHeap, scratch, 1);
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
            GuidDefinedSection.SectionDefinitionGuid = this->getGUID(offset);
            GuidDefinedSection.DataOffset = this->getUINT16(offset + 0x10);
            GuidDefinedSection.Attributes = this->getUINT16(offset + 0x12);
            HeaderSize = sizeof(EFI_GUID_DEFINED_SECTION);
            if (isExtSection) {
                HeaderSize = sizeof(EFI_GUID_DEFINED_SECTION2);
            }

            // RSA2048/SHA256
            if (GuidDefinedSection.SectionDefinitionGuid == GuidDatabase::gEfiCertTypeRsa2048Sha256Guid) {
                RSA2048SHA256 = *(EFI_CERT_BLOCK_RSA_2048_SHA256*)(data + HeaderSize);
                HeaderSize += sizeof(EFI_CERT_BLOCK_RSA_2048_SHA256);
                GuidDefinedSection.DataOffset = HeaderSize;
                offset = HeaderSize;
                while (offset < size) {
                    auto ChildSection = new CommonSection(data + offset, HeaderSize, offsetFromBegin + offset, Compressed, this);
                    ChildSectionSize = ChildSection->SelfDecode();
                    if (ChildSectionSize <= 0) {
                        safeDelete(ChildSection);
                        break;
                    }
                    ChildSection->DecodeChildVolume();
                    ChildVolume.push_back(ChildSection);
                    offset += ChildSectionSize;
                    Align(offset, 0, 0x4);
                }
                break;
            }

            // Lzma Decompress
            else if (GuidDefinedSection.SectionDefinitionGuid == GuidDatabase::gLzmaCustomDecompressGuid) {
                ScratchSize = 0;
                status = LzmaUefiDecompressGetInfo(data + HeaderSize, size - HeaderSize, &decompressedSize, &ScratchSize);
                if (status != RETURN_SUCCESS) {
                    throw exception();
                }

                DecompressedBufferOnHeap = new UINT8[decompressedSize];
                scratch = malloc(ScratchSize);
                status = LzmaUefiDecompress(data + HeaderSize, size - HeaderSize, DecompressedBufferOnHeap, scratch);
                if (status != RETURN_SUCCESS) {
                    free(scratch);
                    throw exception();
                }
                DecodeDecompressedBuffer(DecompressedBufferOnHeap, decompressedSize);
                free(scratch);
            }

            // Brotli Decompress
            else if (GuidDefinedSection.SectionDefinitionGuid == GuidDatabase::gBrotliCustomDecompressGuid) {
                ScratchSize = 0;
                status = BrotliUefiDecompressGetInfo(data + HeaderSize, size - HeaderSize, &decompressedSize, &ScratchSize);
                if (status != RETURN_SUCCESS) {
                    throw exception();
                }

                DecompressedBufferOnHeap = new UINT8[decompressedSize];
                scratch = malloc(ScratchSize);
                status = BrotliUefiDecompress(data + HeaderSize, size - HeaderSize, DecompressedBufferOnHeap, scratch);
                if (status != RETURN_SUCCESS) {
                    free(scratch);
                    throw exception();
                }
                DecodeDecompressedBuffer(DecompressedBufferOnHeap, decompressedSize);
                free(scratch);
            }

            // Tiano Decompress
            else if (GuidDefinedSection.SectionDefinitionGuid == GuidDatabase::gTianoCustomDecompressGuid) {
                ScratchSize = 0;
                status = UefiDecompressGetInfo(data + HeaderSize, size - HeaderSize, &decompressedSize, &ScratchSize);
                if (status != RETURN_SUCCESS) {
                    throw exception();
                }

                DecompressedBufferOnHeap = new UINT8[decompressedSize];
                scratch = malloc(ScratchSize);
                status = UefiTianoDecompress(data + HeaderSize, DecompressedBufferOnHeap, scratch, 2);
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
            peCoffHeader = new PeCoff(data + HeaderSize, size - HeaderSize);
            break;
        case EFI_SECTION_USER_INTERFACE:
            UINT16* char16FileName;
            char16FileName = (UINT16*)this->getBytes(offset, size - HeaderSize);
            FileNameString = wstringToString(char16FileName);
            if (FileNameString == "FmpDxe") {
                ParentVolume->setUniqueVolumeName(QString::fromStdString(FileNameString) + " " + GuidDatabase::getFmpDeviceName(ParentVolume->getVolumeGuid()));
            } else {
                ParentVolume->setUniqueVolumeName(QString::fromStdString(FileNameString));
            }
            safeArrayDelete(char16FileName);
            break;
        case EFI_SECTION_VERSION:
            UINT16* char16VersionString;
            BuildNumber = this->getUINT16(offset);
            char16VersionString = (UINT16*)this->getBytes(offset + 2, size - HeaderSize - 2);
            VersionString = wstringToString(char16VersionString);
            safeArrayDelete(char16VersionString);
            break;
        case EFI_SECTION_FIRMWARE_VOLUME_IMAGE:
            FirmwareVolume *fv;
            fv = new FirmwareVolume(data + HeaderSize, size - HeaderSize, offsetFromBegin + HeaderSize, Compressed, this);
            fv->SelfDecode();
            fv->DecodeChildVolume();
            ChildVolume.push_back(fv);
            break;
        case EFI_SECTION_DXE_DEPEX:
        case EFI_SECTION_PEI_DEPEX:
        case EFI_SECTION_MM_DEPEX:
            dependency = new Depex(data + HeaderSize, size - HeaderSize);
            break;
        case EFI_SECTION_RAW:
            if (ParentVolume->getVolumeSubType() == VolumeType::Apriori) {
                INT64 index = 0;
                UniqueVolumeName = "Apriori List";
                INT64 RemainingSize = size - HeaderSize;
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
}

void CommonSection::setInfoStr() {
    if (InfoStr != "") {
        return;
    }
    INT32 width = 20;
    stringstream ss;
    stringstream guidInfo;
    ss.setf(ios::left);

    if (CommonHeader.Type == EFI_SECTION_GUID_DEFINED)
        guidInfo << "Section GUID:\n" << GuidDefinedSection.SectionDefinitionGuid.str(true) << "\n";

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
            ss << setw(width) << "Data offset:"   << hex << uppercase << GuidDefinedSection.DataOffset << "h\n"
               << setw(width) << "Attributes:"    << hex << uppercase << GuidDefinedSection.Attributes << "h\n";

            if (GuidDefinedSection.SectionDefinitionGuid == GuidDatabase::gLzmaCustomDecompressGuid) {
                ss << "Compression algorithm: LZMA\n"
                   << setw(width) << "Decompressed size:" << hex << uppercase << decompressedSize << "h\n";
            } else if (GuidDefinedSection.SectionDefinitionGuid == GuidDatabase::gEfiCertTypeRsa2048Sha256Guid) {
                ss << setw(width) << "Certificate type:" << "RSA2048/SHA256\n"
                   << "PubKey:\n" << DumpHex(RSA2048SHA256.PublicKey, 256, 8)
                   << "\nSignature:\n" << DumpHex(RSA2048SHA256.Signature, 256, 8);
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

            ss << setw(width) << "DOS signature:" << hex << uppercase << e_magic << "h (" << charToString((CHAR8*)&e_magic, sizeof(UINT16), false) << ")\n"
               << setw(width) << "PE signature:" << hex << uppercase << peSignature << "h (" << charToString((CHAR8*)&peSignature, sizeof(UINT32), false) << ")\n"
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

            ss << setw(width) << "TE signature:" << hex << uppercase << e_magic << "h (" << charToString((CHAR8*)&e_magic, sizeof(UINT16), false) << ")\n"
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
            ss << "Section GUID:\n" << SubTypeGuid.str(true) << "\n";
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
            if (ParentVolume->getVolumeSubType() == VolumeType::Apriori) {
                ss << "\nApriori List:\n";
                for (auto ApriFile: AprioriList) {
                    ss << guidData->getNameFromGuid(ApriFile) << "\n";
                }
            }
            break;
        default:
            break;
    }

    string compressed = "No";
    if (isCompressed())
        compressed = "Yes";
    ss << "\nCompressed: " << compressed;
    InfoStr = QString::fromStdString(guidInfo.str() +  ss.str());
}

Volume* CommonSection::Reorganize() {
    Volume *newVolume = nullptr;
    if (AcpiClass::isAcpiHeader(data + HeaderSize, size - HeaderSize)) {
        AcpiClass *acpiVolume = new AcpiClass(data + HeaderSize, size - HeaderSize, offsetFromBegin + HeaderSize, false);
        acpiVolume->SelfDecode();
        acpiVolume->setUniqueVolumeName("ACPI Table - " + acpiVolume->AcpiTableSignature);
        newVolume = acpiVolume;
    }
    else if (ELF::IsElfFormat(data + HeaderSize)) {
        ELF *elfVolume = new ELF(data + HeaderSize, size - HeaderSize, offsetFromBegin + HeaderSize, Compressed);
        if (elfVolume->SelfDecode() != 0) {
            elfVolume->DecodeChildVolume();
            newVolume = elfVolume;
        }
    }

    if (newVolume != nullptr) {
        newVolume->setCompressedFlag(this->Compressed);
        newVolume->ParentVolume = this->ParentVolume;
        for(int i = 0; i < this->ParentVolume->ChildVolume.size(); ++i) {
                if (this->ParentVolume->ChildVolume[i] == this) {
                    this->ParentVolume->ChildVolume[i] = newVolume;
                }
        }
        for (auto child:this->ChildVolume) {
                newVolume->ChildVolume.append(child);
                child->ParentVolume = newVolume;
        }
        this->ParentVolume = nullptr;
        this->ChildVolume.clear();
        return newVolume;
    }

    return nullptr;
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
    if (isExtSection)
        headerSize += sizeof(UINT32);
    return headerSize;
}

void CommonSection::DecodeDecompressedBuffer(UINT8 *DecompressedBuffer, INT64 bufferSize) {
    INT64 offset = 0;
    while (offset < bufferSize) {
        auto DecompressedSection = new CommonSection(DecompressedBuffer + offset, 0, offset, true, this);
        INT64 SectionSize = DecompressedSection->SelfDecode();
        DecompressedSection->DecodeChildVolume();
        ChildVolume.push_back(DecompressedSection);
        offset += SectionSize;
        Align(offset, 0, 0x4);
    }
}

UINT8 CommonSection::getSectionType() const {
    return CommonHeader.Type;
}

EFI_GUID CommonSection::getVolumeGuid() const {
    switch (CommonHeader.Type) {
        case EFI_SECTION_GUID_DEFINED:
            return GuidDefinedSection.SectionDefinitionGuid;
        case EFI_SECTION_FREEFORM_SUBTYPE_GUID:
            return SubTypeGuid;
        default:
            break;
    }
    return {};
}
