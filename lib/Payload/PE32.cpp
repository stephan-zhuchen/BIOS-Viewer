#include "PE32.h"
#include "BaseLib.h"

using namespace BaseLibrarySpace;

PE32::PE32(UINT8* file, INT64 length, INT64 offset, bool Compressed, Volume* parent):
        Volume(file, length, offset, Compressed, parent) {}

PE32::~PE32() {
    if (convertedPe32Data != nullptr) {
        delete[] convertedPe32Data;
        convertedPe32Data = nullptr;
    }
}

INT64 PE32::SelfDecode() {
    Type = VolumeType::PE32;
    UINT16 magic = *(UINT16*)data;
    if (magic == EFI_IMAGE_DOS_SIGNATURE) {
        dosHeader = *(EFI_IMAGE_DOS_HEADER*)data;
        pe32Header = *(EFI_IMAGE_NT_HEADERS32*)(data + dosHeader.e_lfanew);
        if (pe32Header.OptionalHeader.Magic == EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            isPe32Plus = true;
            pe32plusHeader = *(EFI_IMAGE_NT_HEADERS64*)(data + dosHeader.e_lfanew);
        }
    } else if (magic == EFI_TE_IMAGE_HEADER_SIGNATURE) {
        isTE = true;
        teHeader = *(EFI_TE_IMAGE_HEADER*)data;
    } else {
        isValid = false;
        return 0;
    }
    return size;
}

void PE32::setInfoStr() {
    if (InfoStr != "") {
        return;
    }
    INT32 width = 20;
    stringstream ss;
    ss.setf(ios::left);
    if (!isTE) {
        UINT16 e_magic;
        UINT32 peSignature;
        UINT16 SubSystem;
        e_magic = dosHeader.e_magic;
        peSignature = pe32Header.Signature;
        SubSystem = pe32Header.OptionalHeader.Subsystem;

        ss << setw(width) << "DOS signature:" << hex << uppercase << e_magic << "h (" << charToString((CHAR8*)&e_magic, sizeof(UINT16), false) << ")\n"
           << setw(width) << "PE signature:" << hex << uppercase << peSignature << "h (" << charToString((CHAR8*)&peSignature, sizeof(UINT32), false) << ")\n"
           << setw(width) << "Machine type:" << getMachineType() << "\n"
           << setw(width) << "Number of sections:" << hex << uppercase << pe32Header.FileHeader.NumberOfSections << "h\n"
           << setw(width) << "Characteristics:" << hex << uppercase << pe32Header.FileHeader.Characteristics << "h\n"
           << setw(width) << "Optional Header Signature:" << hex << uppercase << pe32Header.OptionalHeader.Magic << "h\n"
           << setw(width) << "Size of image:" << hex << uppercase << pe32Header.OptionalHeader.SizeOfImage << "h\n"
           << setw(width) << "Size of header:" << hex << uppercase << pe32Header.OptionalHeader.SizeOfHeaders << "h\n"
           << setw(width) << "NumberOfRvaAndSizes:" << hex << uppercase << pe32Header.OptionalHeader.NumberOfRvaAndSizes << "h\n"
           << setw(width) << "Subsystem:" << hex << uppercase << SubSystem << "h (" << getSubsystemName(SubSystem) << ")\n"
           << setw(width) << "EntryPoint Address:" << hex << uppercase << pe32Header.OptionalHeader.AddressOfEntryPoint << "h\n"
           << setw(width) << "Size of code:" << hex << uppercase << pe32Header.OptionalHeader.SizeOfCode << "h\n"
           << setw(width) << "Base of code:" << hex << uppercase << pe32Header.OptionalHeader.BaseOfCode << "h\n"
           << setw(width) << "Base of data:" << hex << uppercase << pe32Header.OptionalHeader.BaseOfData << "h\n";
        if (isPe32Plus)
            ss << setw(width) << "Image base:" << hex << uppercase << pe32plusHeader.OptionalHeader.ImageBase << "h\n";
        else
            ss << setw(width) << "Image base:" << hex << uppercase << pe32Header.OptionalHeader.ImageBase << "h\n";
    }
    else {
        ss << setw(width) << "TE signature:" << hex << uppercase << teHeader.Signature << "h (" << charToString((CHAR8*)&teHeader.Signature, sizeof(UINT16), false) << ")\n"
           << setw(width) << "Machine type:" << getMachineType() << "\n"
           << setw(width) << "Number of sections:" << hex << uppercase << (UINT32)teHeader.NumberOfSections << "h\n"
           << setw(width) << "Subsystem:" << hex << uppercase << (UINT16)teHeader.Subsystem << "h (" << getSubsystemName(teHeader.Subsystem) << ")\n"
           << setw(width) << "Stripped size:" << hex << uppercase << teHeader.StrippedSize << "h\n"
           << setw(width) << "Base of code:" << hex << uppercase << teHeader.BaseOfCode << "h\n"
           << setw(width) << "EntryPoint Address:" << hex << uppercase << teHeader.AddressOfEntryPoint << "h\n"
           << setw(width) << "Image base:" << hex << uppercase << teHeader.ImageBase << "h\n"
           << setw(width) << "VirtualAddress:" << hex << uppercase << teHeader.DataDirectory->VirtualAddress << "h\n";
    }
    InfoStr = QString::fromStdString(ss.str());
}

void PE32::convert2Pe() {
    if (!isTE) {
        return;
    }

    EFI_IMAGE_DOS_HEADER convertedDosHeader{};
    convertedDosHeader.e_magic = EFI_IMAGE_DOS_SIGNATURE;

    if (teHeader.Machine == IMAGE_FILE_MACHINE_I386) {
        EFI_IMAGE_NT_HEADERS32 convertedPe32Header{};
        convertedPe32Header.Signature = EFI_IMAGE_NT_SIGNATURE;
        convertedPe32Header.FileHeader.Machine = teHeader.Machine;
        convertedPe32Header.FileHeader.NumberOfSections = teHeader.NumberOfSections;
        convertedPe32Header.FileHeader.SizeOfOptionalHeader = sizeof(EFI_IMAGE_OPTIONAL_HEADER32);
        convertedPe32Header.FileHeader.Characteristics = 0;
        convertedPe32Header.OptionalHeader.Magic = EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC;
        convertedPe32Header.OptionalHeader.AddressOfEntryPoint = teHeader.AddressOfEntryPoint;
        convertedPe32Header.OptionalHeader.BaseOfCode = teHeader.BaseOfCode;
        convertedPe32Header.OptionalHeader.ImageBase = (UINT32)teHeader.ImageBase;
        convertedPe32Header.OptionalHeader.SizeOfImage = size;

        INT64 SectionsSize = convertedPe32Header.FileHeader.NumberOfSections * sizeof(EFI_IMAGE_SECTION_HEADER);
        convertedDosHeader.e_lfanew = teHeader.BaseOfCode - SectionsSize - sizeof(EFI_IMAGE_NT_HEADERS32);

        convertedPe32Header.OptionalHeader.SizeOfHeaders = teHeader.BaseOfCode;
        convertedPe32Header.OptionalHeader.NumberOfRvaAndSizes = 0x10;
        convertedPe32Header.OptionalHeader.SectionAlignment = 0x20;
        convertedPe32Header.OptionalHeader.FileAlignment = 0x20;
        convertedPe32Header.OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = teHeader.DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
        convertedPe32Header.OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = teHeader.DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
        convertedPe32Header.OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress = teHeader.DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
        convertedPe32Header.OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_DEBUG].Size = teHeader.DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_DEBUG].Size;

        UINT8* codeData = data + sizeof(EFI_TE_IMAGE_HEADER);
        INT64 sizeOfCode = size - sizeof(EFI_TE_IMAGE_HEADER);
        convertedPe32Size = convertedDosHeader.e_lfanew + sizeof(EFI_IMAGE_NT_HEADERS32) + sizeOfCode;
        convertedPe32Data = new UINT8[convertedPe32Size];
        memset(convertedPe32Data, 0, convertedPe32Size);
        memcpy(convertedPe32Data, &convertedDosHeader, sizeof(EFI_IMAGE_DOS_HEADER));
        memcpy(convertedPe32Data + convertedDosHeader.e_lfanew, &convertedPe32Header, sizeof(EFI_IMAGE_NT_HEADERS32));
        memcpy(convertedPe32Data + convertedDosHeader.e_lfanew + sizeof(EFI_IMAGE_NT_HEADERS32), codeData, sizeOfCode);
    } else if (teHeader.Machine == IMAGE_FILE_MACHINE_X64) {
        EFI_IMAGE_NT_HEADERS64 convertedPe64Header{};
        convertedPe64Header.Signature = EFI_IMAGE_NT_SIGNATURE;
        convertedPe64Header.FileHeader.Machine = teHeader.Machine;
        convertedPe64Header.FileHeader.NumberOfSections = teHeader.NumberOfSections;
        convertedPe64Header.FileHeader.SizeOfOptionalHeader = sizeof(EFI_IMAGE_OPTIONAL_HEADER64);
        convertedPe64Header.FileHeader.Characteristics = 0;
        convertedPe64Header.OptionalHeader.Magic = EFI_IMAGE_NT_OPTIONAL_HDR64_MAGIC;
        convertedPe64Header.OptionalHeader.AddressOfEntryPoint = teHeader.AddressOfEntryPoint;
        convertedPe64Header.OptionalHeader.BaseOfCode = teHeader.BaseOfCode;
        convertedPe64Header.OptionalHeader.ImageBase = teHeader.ImageBase;
        convertedPe64Header.OptionalHeader.SizeOfImage = size;

        INT64 SectionsSize = convertedPe64Header.FileHeader.NumberOfSections * sizeof(EFI_IMAGE_SECTION_HEADER);
        convertedDosHeader.e_lfanew = teHeader.BaseOfCode - SectionsSize - sizeof(EFI_IMAGE_NT_HEADERS64);

        convertedPe64Header.OptionalHeader.SizeOfHeaders = teHeader.BaseOfCode;
        convertedPe64Header.OptionalHeader.NumberOfRvaAndSizes = 0x10;
        convertedPe64Header.OptionalHeader.SectionAlignment = 0x20;
        convertedPe64Header.OptionalHeader.FileAlignment = 0x20;
        convertedPe64Header.OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = teHeader.DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
        convertedPe64Header.OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = teHeader.DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
        convertedPe64Header.OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress = teHeader.DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
        convertedPe64Header.OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_DEBUG].Size = teHeader.DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_DEBUG].Size;

        UINT8* codeData = data + sizeof(EFI_TE_IMAGE_HEADER);
        INT64 sizeOfCode = size - sizeof(EFI_TE_IMAGE_HEADER);
        convertedPe32Size = convertedDosHeader.e_lfanew + sizeof(EFI_IMAGE_NT_HEADERS64) + sizeOfCode;
        convertedPe32Data = new UINT8[convertedPe32Size];
        memset(convertedPe32Data, 0, convertedPe32Size);
        memcpy(convertedPe32Data, &convertedDosHeader, sizeof(EFI_IMAGE_DOS_HEADER));
        memcpy(convertedPe32Data + convertedDosHeader.e_lfanew, &convertedPe64Header, sizeof(EFI_IMAGE_NT_HEADERS64));
        memcpy(convertedPe32Data + convertedDosHeader.e_lfanew + sizeof(EFI_IMAGE_NT_HEADERS64), codeData, sizeOfCode);
    } else {
        return;
    }
}

string PE32::getMachineType() const {
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

string PE32::getSubsystemName(UINT16 subsystem) {
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
