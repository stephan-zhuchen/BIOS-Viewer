#include "ElfLib.h"
#include "BaseLib.h"
#include "UefiLib.h"
#include <iostream>
#include <sstream>
#include <iomanip>

Elf::Elf(UINT8* fv, INT64 length, INT64 offset, bool Compressed):Volume(fv, length, offset, Compressed) {
    if (!IsElfFormat(fv)) {
        ValidFlag = false;
        return;
    }
    Type = UefiSpace::VolumeType::ELF;
    Ehdr.Elf32Hdr = (Elf32_Ehdr *)fv;
    Shdr.Elf32Shdr = (Elf32_Shdr *)(fv + Ehdr.Elf32Hdr->e_shoff);
    SectionListOffset = Ehdr.Elf32Hdr->e_shoff;
    SectionNum = Ehdr.Elf32Hdr->e_shnum;
    SectionHdrSize = sizeof(Elf32_Shdr);
    if (Ehdr.Elf32Hdr->e_ident[EI_CLASS] == ELFCLASS64) {
        isElf32 = false;
        Ehdr.Elf64Hdr = (Elf64_Ehdr *)fv;
        Shdr.Elf64Shdr = (Elf64_Shdr *)(fv + Ehdr.Elf64Hdr->e_shoff);
        SectionListOffset = Ehdr.Elf64Hdr->e_shoff;
        SectionNum = Ehdr.Elf64Hdr->e_shnum;
        SectionHdrSize = sizeof(Elf64_Shdr);
    }

    for (UINT32 idx = 0; idx < SectionNum; ++idx) {
        auto *sec = new SectionHeader;
        if (isElf32) {
            sec->Elf32Shdr = (Elf32_Shdr*)(fv + Ehdr.Elf32Hdr->e_shoff + idx * SectionHdrSize);
        }
        else {
            sec->Elf64Shdr = (Elf64_Shdr*)(fv + Ehdr.Elf64Hdr->e_shoff + idx * SectionHdrSize);
        }
        SectionList.push_back(sec);
    }

    if (isElf32 && SectionList.size() < Ehdr.Elf32Hdr->e_shstrndx + 1) {
        ValidFlag = false;
        return;
    } else if (!isElf32 && SectionList.size() < Ehdr.Elf64Hdr->e_shstrndx + 1) {
        ValidFlag = false;
        return;
    }

    if (isElf32) {
        Elf32_Shdr *StrSec32 = SectionList.at(Ehdr.Elf32Hdr->e_shstrndx)->Elf32Shdr;
        StrTableOffset = StrSec32->sh_offset;
        StrTableSize = StrSec32->sh_size;
    } else {
        Elf64_Shdr *StrSec64 = SectionList.at(Ehdr.Elf64Hdr->e_shstrndx)->Elf64Shdr;
        StrTableOffset = StrSec64->sh_offset;
        StrTableSize = StrSec64->sh_size;
    }

}

Elf::~Elf() {
    using namespace std;
    for(auto sec:SectionList) {
        delete sec;
    }
}

bool Elf::isValid() const {
    return ValidFlag;
}

void Elf::decodeSections() {
    for (UINT32 idx = 0; idx < SectionNum; ++idx) {
        UINT32 SecOff = SectionList.at(idx)->Elf32Shdr->sh_offset;
        UINT32 SecSize = SectionList.at(idx)->Elf32Shdr->sh_size;
        std::string SecName = getStringFromOffset(SectionList.at(idx)->Elf32Shdr->sh_name);
        if (!isElf32) {
            SecOff = SectionList.at(idx)->Elf64Shdr->sh_offset;
            SecSize = SectionList.at(idx)->Elf64Shdr->sh_size;
            SecName = getStringFromOffset(SectionList.at(idx)->Elf64Shdr->sh_name);
        }
        if (SecName.rfind(".upld_info") == 0) {
            auto *SecFile = new Volume(data + SecOff, SecSize, SecOff, this->isCompressed);
            SecFile->Type = UefiSpace::VolumeType::Other;
            SecFile->AdditionalMsg = "Universal Payload Info";
            UpldFiles.push_back(SecFile);
        }
        if (SecName.rfind(".upld.") == 0) {
            Volume *SecFile = new UefiSpace::FirmwareVolume(data + SecOff, SecSize, SecOff, false, this->isCompressed);
            UpldFiles.push_back(SecFile);
        }
    }
}

std::string Elf::getStringFromOffset(UINT32 off) const {
    UINT8* StrOff =  data + StrTableOffset + off;
    UINT32 StrSize = 0;
    for (UINT32 idx = 0; idx < StrTableSize; ++idx) {
        UINT8* val = StrOff + idx;
        if (*val == 0x0) {
            break;
        }
        StrSize += 1;
    }
    return BaseLibrarySpace::charToString((CHAR8*)StrOff, (INT64)StrSize);
}

void Elf::setInfoStr() {
    using namespace std;
    INT32 width = 25;
    stringstream ss;
    stringstream guidInfo;
    ss.setf(ios::left);

    if (isElf32) {
        ss << setw(width) << "File type: " << hex << Ehdr.Elf32Hdr->e_type << "\n"
           << setw(width) << "Machine type: " << hex << Ehdr.Elf32Hdr->e_machine << "\n"
           << setw(width) << "ELF Version: " << hex << Ehdr.Elf32Hdr->e_version << "\n"
           << setw(width) << "Entry point: " << hex << Ehdr.Elf32Hdr->e_entry << "\n"
           << setw(width) << "Program header offset: " << hex << Ehdr.Elf32Hdr->e_phoff << "\n"
           << setw(width) << "Section header offset: " << hex << Ehdr.Elf32Hdr->e_shoff << "\n"
           << setw(width) << "Architecture-specific flags: " << hex << Ehdr.Elf32Hdr->e_flags << "\n"
           << setw(width) << "ELF header Size: " << hex << Ehdr.Elf32Hdr->e_ehsize << "\n"
           << setw(width) << "program header Size: " << hex << Ehdr.Elf32Hdr->e_phentsize << "\n"
           << setw(width) << "Number of program header: " << hex << Ehdr.Elf32Hdr->e_phnum << "\n"
           << setw(width) << "section header Size: " << hex << Ehdr.Elf32Hdr->e_shentsize << "\n"
           << setw(width) << "Number of section header: " << hex << Ehdr.Elf32Hdr->e_shnum << "\n"
           << setw(width) << "Section name strings section: " << hex << Ehdr.Elf32Hdr->e_shstrndx << "\n";
    } else {
        ss << setw(width) << "File type: " << hex << Ehdr.Elf64Hdr->e_machine << "\n"
           << setw(width) << "Machine type: " << hex << Ehdr.Elf64Hdr->e_machine << "\n"
           << setw(width) << "ELF Version: " << hex << Ehdr.Elf64Hdr->e_version << "\n"
           << setw(width) << "Entry point: " << hex << Ehdr.Elf64Hdr->e_entry << "\n"
           << setw(width) << "Program header offset: " << hex << Ehdr.Elf64Hdr->e_phoff << "\n"
           << setw(width) << "Section header offset: " << hex << Ehdr.Elf64Hdr->e_shoff << "\n"
           << setw(width) << "Architecture-specific flags: " << hex << Ehdr.Elf64Hdr->e_flags << "\n"
           << setw(width) << "ELF header Size: " << hex << Ehdr.Elf64Hdr->e_ehsize << "\n"
           << setw(width) << "program header Size: " << hex << Ehdr.Elf64Hdr->e_phentsize << "\n"
           << setw(width) << "Number of program header: " << hex << Ehdr.Elf64Hdr->e_phnum << "\n"
           << setw(width) << "section header Size: " << hex << Ehdr.Elf64Hdr->e_shentsize << "\n"
           << setw(width) << "Number of section header: " << hex << Ehdr.Elf64Hdr->e_shnum << "\n"
           << setw(width) << "Section name strings section: " << hex << Ehdr.Elf64Hdr->e_shstrndx << "\n";
    }

    InfoStr = QString::fromStdString(ss.str());
}

bool Elf::IsElfFormat (const UINT8  *ImageBase) {
    Elf32_Ehdr  *Elf32Hdr;
    Elf64_Ehdr  *Elf64Hdr;

    ASSERT (ImageBase != NULL);

    Elf32Hdr = (Elf32_Ehdr *)ImageBase;

    //
    // Start with correct signature "\7fELF"
    //
    if ((Elf32Hdr->e_ident[EI_MAG0] != ELFMAG0) ||
        (Elf32Hdr->e_ident[EI_MAG1] != ELFMAG1) ||
        (Elf32Hdr->e_ident[EI_MAG1] != ELFMAG1) ||
        (Elf32Hdr->e_ident[EI_MAG2] != ELFMAG2)
        )
    {
        return FALSE;
    }

    //
    // Support little-endian only
    //
    if (Elf32Hdr->e_ident[EI_DATA] != ELFDATA2LSB) {
        return FALSE;
    }

    //
    // Check 32/64-bit architecture
    //
    if (Elf32Hdr->e_ident[EI_CLASS] == ELFCLASS64) {
        Elf64Hdr = (Elf64_Ehdr *)Elf32Hdr;
        Elf32Hdr = nullptr;
    } else if (Elf32Hdr->e_ident[EI_CLASS] == ELFCLASS32) {
        Elf64Hdr = NULL;
    } else {
        return FALSE;
    }

    if (Elf64Hdr != nullptr) {
        //
        // Support intel architecture only for now
        //
        if (Elf64Hdr->e_machine != EM_X86_64) {
            return FALSE;
        }

        //
        //  Support ELF types: EXEC (Executable file), DYN (Shared object file)
        //
        if ((Elf64Hdr->e_type != ET_EXEC) && (Elf64Hdr->e_type != ET_DYN)) {
            return FALSE;
        }

        //
        // Support current ELF version only
        //
        if (Elf64Hdr->e_version != EV_CURRENT) {
            return FALSE;
        }
    } else {
        //
        // Support intel architecture only for now
        //
        if (Elf32Hdr->e_machine != EM_386) {
            return FALSE;
        }

        //
        //  Support ELF types: EXEC (Executable file), DYN (Shared object file)
        //
        if ((Elf32Hdr->e_type != ET_EXEC) && (Elf32Hdr->e_type != ET_DYN)) {
            return FALSE;
        }

        //
        // Support current ELF version only
        //
        if (Elf32Hdr->e_version != EV_CURRENT) {
            return FALSE;
        }
    }

    return TRUE;
}
