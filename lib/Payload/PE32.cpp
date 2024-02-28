#include "PE32.h"
#include "BaseLib.h"

using namespace BaseLibrarySpace;

PE32::PE32(UINT8* file, INT64 length, INT64 offset, bool Compressed, Volume* parent):
        Volume(file, length, offset, Compressed, parent) {}

PE32::~PE32() {}

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
    } else
        return 0;
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
        UINT16 peOptionalSignature;
        UINT16 SubSystem;
        e_magic = dosHeader.e_magic;
        peSignature = pe32Header.Signature;
        peOptionalSignature = pe32Header.OptionalHeader.Magic;
        SubSystem = pe32Header.OptionalHeader.Subsystem;

        ss << setw(width) << "DOS signature:" << hex << uppercase << e_magic << "h (" << charToString((CHAR8*)&e_magic, sizeof(UINT16), false) << ")\n"
           << setw(width) << "PE signature:" << hex << uppercase << peSignature << "h (" << charToString((CHAR8*)&peSignature, sizeof(UINT32), false) << ")\n"
           << setw(width) << "Machine type:" << getMachineType() << "\n"
           << setw(width) << "Number of sections:" << hex << uppercase << pe32Header.FileHeader.NumberOfSections << "h\n"
           << setw(width) << "Characteristics:" << hex << uppercase << pe32Header.FileHeader.Characteristics << "h\n"
           << setw(width) << "Optional header signature:" << hex << uppercase << peOptionalSignature << "h\n"
           << setw(width) << "Subsystem:" << hex << uppercase << SubSystem << "h (" << getSubsystemName(SubSystem) << ")\n"
           << setw(width) << "EntryPoint Address:" << hex << uppercase << pe32Header.OptionalHeader.AddressOfEntryPoint << "h\n"
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
           << setw(width) << "Subsystem:" << hex << uppercase << teHeader.Subsystem << "h (" << getSubsystemName(teHeader.Subsystem) << ")\n"
           << setw(width) << "Stripped size:" << hex << uppercase << teHeader.StrippedSize << "h\n"
           << setw(width) << "Base of code:" << hex << uppercase << teHeader.BaseOfCode << "h\n"
           << setw(width) << "EntryPoint Address:" << hex << uppercase << teHeader.AddressOfEntryPoint << "h\n"
           << setw(width) << "Image base:" << hex << uppercase << teHeader.ImageBase << "h\n"
           << setw(width) << "VirtualAddress:" << hex << uppercase << teHeader.DataDirectory->VirtualAddress << "h\n";
    }
    InfoStr = QString::fromStdString(ss.str());
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
