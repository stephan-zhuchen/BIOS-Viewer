//
// Created by stephan on 9/3/2023.
//
#pragma once
#include "Volume.h"
#include "UEFI/ELF.h"

class ELF: public Volume  {
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
    vector<SectionHeader*> SectionList;
    UINT64 SectionListOffset;
    UINT64 SectionNum;
    UINT64 SectionHdrSize;
    UINT64 StrTableOffset;
    UINT64 StrTableSize;
public:
    ELF() = delete;
    ELF(UINT8* file, INT64 length, INT64 offset, bool Compressed=false, Volume* parent= nullptr);
    ~ELF() override;
    bool isValid() const;
    std::string getStringFromOffset(UINT32 off) const;
    static bool IsElfFormat(const UINT8* ImageBase);

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void  setInfoStr() override;
    [[nodiscard]] INT64 getHeaderSize() const override;
};

