#include "UefiLib.h"
#include "../include/PiDependency.h"
#include "../include/GuidDefinition.h"

namespace UefiSpace {
    PeCoff::PeCoff(UINT8* file, INT64 length, INT64 offset):Volume(file, length, offset) {
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
            throw exception("Wrong Magic number");
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
            break;
        case IMAGE_FILE_MACHINE_EBC:
            return "EBC";
            break;
        case IMAGE_FILE_MACHINE_X64:
            return "x86_64";
            break;
        case IMAGE_FILE_MACHINE_ARM:
            return "ARM";
            break;
        case IMAGE_FILE_MACHINE_ARMT:
            return "ARMT";
            break;
        case IMAGE_FILE_MACHINE_ARM64:
            return "ARM64";
            break;
        case IMAGE_FILE_MACHINE_RISCV64:
            return "RISC-V";
            break;
        case IMAGE_FILE_MACHINE_LOONGARCH64:
            return "LoongArch";
            break;
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

    Depex::Depex(UINT8* file, INT64 length):Volume(file, length) {
        INT64 offset = 0;
        UINT8 Opcode = this->getUINT8(offset);
        offset += sizeof(UINT8);
        while (Opcode != EFI_DEP_END) {
            if (Opcode == EFI_DEP_PUSH) {
                EFI_GUID depexGuid = this->getGUID(offset);
                offset += sizeof(EFI_GUID);
                OrganizedDepexList.push_back(guidData->getNameFromGuid(depexGuid));
            } else if (Opcode == EFI_DEP_BEFORE || Opcode == EFI_DEP_AFTER || Opcode == EFI_DEP_TRUE || Opcode == EFI_DEP_FALSE) {
                OrganizedDepexList.push_back(getOpcodeString(Opcode));
            } else if (Opcode == EFI_DEP_NOT) {
                string temp = OrganizedDepexList.at(OrganizedDepexList.size() - 1);
                OrganizedDepexList.pop_back();
                temp = "NOT " + temp;
                OrganizedDepexList.push_back(temp);
            } else if (Opcode == EFI_DEP_AND) {
                string top = OrganizedDepexList.at(OrganizedDepexList.size() - 1);
                OrganizedDepexList.pop_back();
                string second = OrganizedDepexList.at(OrganizedDepexList.size() - 1);
                OrganizedDepexList.pop_back();
                string newDepex = second + "\nAND\n" + top;
                OrganizedDepexList.push_back(newDepex);
            } else if (Opcode == EFI_DEP_OR) {
                string top = OrganizedDepexList.at(OrganizedDepexList.size() - 1);
                OrganizedDepexList.pop_back();
                string second = OrganizedDepexList.at(OrganizedDepexList.size() - 1);
                OrganizedDepexList.pop_back();
                string newDepex = second + "\nOR\n" + top;
                OrganizedDepexList.push_back(newDepex);
            }
            Opcode = this->getUINT8(offset);
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
            throw exception("Invalid opcode");
            break;
        }
        return opStr;
    }
}

