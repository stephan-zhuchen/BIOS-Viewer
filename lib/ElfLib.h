#pragma once

#include "UEFI/ELF.h"
#include "UefiLib.h"
#include <vector>
#include <string>

using UefiSpace::Volume;

class Elf: public Volume {
public:
    bool ValidFlag {true};
    bool isElf32   {true};
    union Elf_Ehdr {
        Elf32_Ehdr  *Elf32Hdr;
        Elf64_Ehdr  *Elf64Hdr;
    } Ehdr;
    union SectionHeader {
        Elf32_Shdr *Elf32Shdr;
        Elf64_Shdr *Elf64Shdr;
    } Shdr;
    std::vector<SectionHeader*> SectionList;
    UINT32 SectionListOffset;
    UINT32 SectionNum;
    UINT32 SectionHdrSize;
    UINT32 StrTableOffset;
    UINT32 StrTableSize;
    std::vector<Volume*> SectionFiles;
    std::vector<Volume*> UpldFiles;
public:
    Elf() = delete;
    Elf(UINT8* fv, INT64 length, INT64 offset, bool Compressed=false);
    virtual ~Elf();
    bool isValid() const;
    void decodeSections();
    void setInfoStr() override;
    std::string getStringFromOffset(UINT32 off) const;
    static bool IsElfFormat(const UINT8* ImageBase);
};
