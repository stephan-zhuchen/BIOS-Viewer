//
// Created by stephan on 8/29/2023.
//

#include "BaseLib.h"
#include "FfsFile.h"
#include "CommonSection.h"
#include "UEFI/GuidDatabase.h"
#include "Feature/FspHeader.h"
#include "Feature/MicrocodeClass.h"
#include "Feature/AcmClass.h"
using namespace BaseLibrarySpace;

FfsFile::FfsFile(UINT8 *file, INT64 offset, bool Compressed, Volume* parent):
    Volume(file, 0, offset, Compressed, parent) {}

bool FfsFile::CheckValidation() {
    return Volume::CheckValidation();
}

INT64 FfsFile::SelfDecode() {
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
        headerSumValue = CalculateSum8((UINT8 *) &FfsExtHeader, sizeof(EFI_FFS_FILE_HEADER2));
        headerSumValue = headerSumValue + FfsExtHeader.State + FfsExtHeader.IntegrityCheck.Checksum.File;
    } else {
        headerSumValue = CalculateSum8((UINT8 *) &FfsHeader, sizeof(EFI_FFS_FILE_HEADER));
        headerSumValue = headerSumValue + FfsHeader.State + FfsHeader.IntegrityCheck.Checksum.File;
    }
    if (headerSumValue == 0)
        headerChecksumValid = true;

    // Calculate the data checksum
    if ((FfsHeader.Attributes | FFS_ATTRIB_CHECKSUM) == 0x0)
        dataChecksumValid = true;

    // Check if the file is an Apriori file
    if (FfsHeader.Name.Data1 == GuidDatabase::gPeiAprioriFileNameGuid.Data1 || FfsHeader.Name.Data1 == GuidDatabase::gAprioriGuid.Data1) {
        SubType = VolumeType::Apriori;
    }
    return size;
}

void FfsFile::DecodeChildVolume() {
    if (!headerChecksumValid)
        return;
    INT64 offset = sizeof(EFI_FFS_FILE_HEADER);
    if (isExtended) {
        offset = sizeof(EFI_FFS_FILE_HEADER2);
    }

    while (offset < size) {
        auto *SecHeader = (EFI_COMMON_SECTION_HEADER*)(data + offset);
        INT64 SecSize = (INT64)SECTION_SIZE(SecHeader);
        if (SecSize == 0xFFFFFF) {
            SecSize = this->getUINT32(offset + sizeof(EFI_COMMON_SECTION_HEADER));
        }

        if (SecSize + offset > size) {
            // Invalid Section, clear all sections under this FFS
            for(auto section:ChildVolume) {
                safeDelete(section);
            }
            ChildVolume.clear();
            break;
        }

        auto *Sec = new CommonSection(data + offset, SecSize, offsetFromBegin + offset, Compressed, this);
        if (!Sec->CheckValidation()) {
            safeDelete(Sec);
            break;
        }
        if (Sec->SelfDecode() == 0) {
            // Invalid Section, clear all sections under this FFS
            safeDelete(Sec);
            for(auto section:ChildVolume) {
                safeDelete(section);
            }
            ChildVolume.clear();
            break;
        }
        Sec->DecodeChildVolume();
        ChildVolume.push_back(Sec);
        // Align to four bytes
        offset += SecSize;
        Align(offset, 0, 0x4);
    }
}

void FfsFile::setInfoStr() {
    if (InfoStr != "") {
        return;
    }
    INT32 width = 18;
    stringstream ss;
    ss.setf(ios::left);

    ss << "File GUID:\n" << FfsHeader.Name.str(true) << "\n"
       << setw(width) << "Type:"        << hex << (UINT32)FfsHeader.Type << "h\n"
       << setw(width) << "Attributes:"  << hex << uppercase << (UINT32)FfsHeader.Attributes << "h\n"
       << setw(width) << "Full size:"   << hex << uppercase << getSize() << "h\n"
       << setw(width) << "Header size:" << hex << uppercase << getHeaderSize() << "h\n"
       << setw(width) << "Body size:"   << hex << uppercase << getSize() - getHeaderSize() << "h\n"
       << setw(width) << "State:"       << hex << (UINT32)FfsHeader.State << "h\n"
       << setw(width) << "Header Checksum:" << hex << uppercase << (UINT32)FfsHeader.IntegrityCheck.Checksum.Header << "h" << (headerChecksumValid ? ", valid":", not valid") << "\n"
       << setw(width) << "Data Checksum:"   << hex << uppercase << (UINT32)FfsHeader.IntegrityCheck.Checksum.File << "h" << (headerChecksumValid ? ", valid":", not valid") << "\n";

    string compressed = "No";
    if (isCompressed())
        compressed = "Yes";
    ss << "\nCompressed: " << compressed;

    InfoStr = QString::fromStdString(ss.str());
}

Volume* FfsFile::Reorganize() {
    Volume *newVolume = nullptr;
    bool   RemainChild = true;
    EFI_GUID guid = this->getFfsGuid();

    if (guid == GuidDatabase::gIntelMicrocodeArrayFfsBinGuid) {
        newVolume = new MicrocodeHeaderClass(data + this->getHeaderSize(), size - this->getHeaderSize(), offsetFromBegin + this->getHeaderSize());
        newVolume->SelfDecode();
    }
    else if (guid == GuidDatabase::gStartupAcmPeiBinGuid) {
        newVolume = new AcmHeaderClass(data + this->getHeaderSize(), size - this->getHeaderSize(), offsetFromBegin + this->getHeaderSize());
        newVolume->SelfDecode();
    }
    else if (FspHeader::isFspHeader(data + getHeaderSize() + sizeof(EFI_COMMON_SECTION_HEADER))) {
        INT64 FspOffset = getHeaderSize() + sizeof(EFI_COMMON_SECTION_HEADER);
        FspHeader *fspVolume = new FspHeader(data + FspOffset, size - FspOffset, offsetFromBegin + FspOffset);
        if (fspVolume->SelfDecode() != 0) {
            newVolume = fspVolume;
            RemainChild = false;
        } else {
            safeDelete(fspVolume);
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
        if (RemainChild) {
            for (auto child:this->ChildVolume) {
                newVolume->ChildVolume.append(child);
                child->ParentVolume = newVolume;
            }
        } else {
            for (auto child:this->ChildVolume) {
                safeDelete(child);
            }
        }

        this->ParentVolume = nullptr;
        this->ChildVolume.clear();
        return newVolume;
    }

    return nullptr;
}

INT64 FfsFile::getHeaderSize() const {
    if (isExtended)
        return sizeof(EFI_FFS_FILE_HEADER2);
    return sizeof(EFI_FFS_FILE_HEADER);
}

UINT8 FfsFile::getType() const {
    if (isExtended) {
        return FfsExtHeader.Type;
    }
    return FfsHeader.Type;
}

EFI_GUID FfsFile::getFfsGuid() const {
    if (isExtended) {
        return FfsExtHeader.Name;
    }
    return FfsHeader.Name;
}

EFI_GUID FfsFile::getVolumeGuid() const {
    return getFfsGuid();
}

FfsFile::~FfsFile() = default;
