//
// Created by stephan on 8/29/2023.
//

#include <thread>
#include <mutex>
#include <algorithm>
#include "BaseLib.h"
#include "FirmwareVolume.h"
#include "FfsFile.h"
#include "UEFI/GuidDatabase.h"
#include "UEFI/PiFirmwareFile.h"
#include "NvVariable.h"

using namespace BaseLibrarySpace;

FirmwareVolume::FirmwareVolume(UINT8 *buffer, INT64 length, INT64 offset, bool Compressed, Volume* parent):
    Volume(buffer, length, offset, Compressed, parent) {}

bool FirmwareVolume::CheckValidation() {
    if (size < sizeof(EFI_FIRMWARE_VOLUME_HEADER))
        return false;
    auto* address = (EFI_FIRMWARE_VOLUME_HEADER*)data;
    if (address->Signature != 0x4856465F)
        return false;

    // ZeroVector is reused by SBL FV, so skip checking ZeroVector value
    // UINT8* ZeroVector = address->ZeroVector;
    // if ((*(UINT64*)ZeroVector != 0x0) || ((*((UINT64*)ZeroVector + 1) != 0x0)))
    //     return false;
    if (address->FileSystemGuid.Data1 == 0xFFFFFFFF || address->FileSystemGuid.Data1 == 0x0) {
        return false;
    }

    // Skip caculating ZeroVector checksum
    UINT16 sumValue = CalculateSum16((UINT16 *) ((UINT8*)address + 16),
                                     (sizeof(EFI_FIRMWARE_VOLUME_HEADER) - 16) / sizeof(UINT16));
    if (sumValue != 0)
        return false;
    return true;
}

INT64 FirmwareVolume::SelfDecode() {
    if (!CheckValidation())
        return 0;

    Type = VolumeType::FirmwareVolume;
    FirmwareVolumeHeader = *(EFI_FIRMWARE_VOLUME_HEADER*)data;

    // Check that the firmware volume is not longer than the volume
    if (FirmwareVolumeHeader.FvLength > (UINT64)size) {
        Corrupted = true;
        return 0;
    }
    size = (INT64)FirmwareVolumeHeader.FvLength;

    // Check if the firmware volume has an extended header
    if (FirmwareVolumeHeader.ExtHeaderOffset == 0) {
        FirmwareVolumeHeaderSize = 0x48;
        isExt = false;
    } else {
        auto *ExtFvFfs = (EFI_FFS_FILE_HEADER*)(data + 0x48);
        FirmwareVolumeHeaderSize = 0x48 + FFS_FILE_SIZE(ExtFvFfs);
        if (size < FirmwareVolumeHeader.ExtHeaderOffset) {
            Corrupted = true;
            return 0;
        }
        FirmwareVolumeExtHeader = *(EFI_FIRMWARE_VOLUME_EXT_HEADER*)(data + FirmwareVolumeHeader.ExtHeaderOffset);
        isExt = true;
    }

    // Check if the firmware volume is an NV volume
    if (FirmwareVolumeHeader.FileSystemGuid == GuidDatabase::gEfiSystemNvDataFvGuid) {
        isNv = true;
    }

    return size;
}

void FirmwareVolume::DecodeChildVolume() {
    INT64 offset = FirmwareVolumeHeaderSize;
    Align(offset, 0, 0x8);

    vector<thread> threadPool;
    std::mutex mtx;
    auto FvDecoder = [this, &mtx](INT64 off) {
        auto *Ffs = new FfsFile(data + off, offsetFromBegin + off, Compressed, this);
        Ffs->SelfDecode();

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
                Ffs->DecodeChildVolume();
                break;
            default:
                break;
        }
        std::lock_guard<std::mutex> lock(mtx);
        ChildVolume.push_back(Ffs);
    };
    while (offset < size) {
        EFI_FFS_FILE_HEADER  FfsHeader = *(EFI_FFS_FILE_HEADER*)(data + offset);
        INT64 FfsSize = FFS_FILE_SIZE(&FfsHeader);

        // If the size of the current FFS file is 0xFFFFFF and the file is marked as deleted (0xFF),
        // then the current FFS file is not used and can be overwritten.
        if (FfsSize == 0xFFFFFF && FfsHeader.State == 0xFF) {
            auto freeSpace = new Volume(data + offset, size - offset, offsetFromBegin + offset, Compressed, this);
            ChildVolume.push_back(freeSpace);
            break;
        }

        if (isNv) {
            isNv = false;
            auto NvStorage = new NvStorageVariable(data + offset, size - offset, offsetFromBegin + offset, Compressed, this);
            INT64 NvSize = NvStorage->SelfDecode();
            NvStorage->DecodeChildVolume();
            auto FaultTolerant = new FaultTolerantBlock(data + offset + NvSize, size - offset - NvSize, offsetFromBegin + offset + NvSize, Compressed, this);
            INT64 TolerantSize = FaultTolerant->SelfDecode();
            FaultTolerant->DecodeChildVolume();
            ChildVolume.push_back(NvStorage);
            ChildVolume.push_back(FaultTolerant);
            FfsSize = NvSize + TolerantSize;
        } else {
            threadPool.emplace_back(FvDecoder, offset);
        }

        // If the current FFS file size is valid, then update the offset to point to the next FFS file.
        if (offset + FfsSize > offset){
            offset += FfsSize;
            Align(offset, 0, 0x8);
        } else {
            // Otherwise, the current FFS file is not valid, so create a volume that represents the free space
            // between the current FFS file and the end of the FV.
            auto freeSpace = new Volume(data + offset, size - offset, offsetFromBegin + offset, Compressed, this);
            ChildVolume.push_back(freeSpace);
            break;
        }
    }
    for (thread &t:threadPool) {
        t.join();
    }
    std::sort(ChildVolume.begin(), ChildVolume.end(), [](Volume *v1, Volume *v2) { return v1->getOffset() < v2->getOffset(); });
}

void FirmwareVolume::setInfoStr() {
    if (InfoStr != "") {
        return;
    }
    INT32 width = 15;
    stringstream ss;
    ss.setf(ios::left);

    ss << "FileSystem GUID:\n" << FirmwareVolumeHeader.FileSystemGuid.str(true) << "\n"
       << setw(width) << "Signature:" << charToString((CHAR8*)(&FirmwareVolumeHeader.Signature), sizeof(UINT32)) << "\n"
       << setw(width) << "Full size:" << hex << uppercase << FirmwareVolumeHeader.FvLength << "h\n"
       << setw(width) << "Header size:" << hex << uppercase << FirmwareVolumeHeaderSize << "h\n"
       << setw(width) << "Body size:" << hex << uppercase << FirmwareVolumeHeader.FvLength - FirmwareVolumeHeaderSize << "h\n"
       << setw(width) << "Revision:"    << hex << (UINT32)FirmwareVolumeHeader.Revision << "\n"
       << setw(width) << "Attributes:"  << hex << uppercase << FirmwareVolumeHeader.Attributes << "h\n"
       << setw(width) << "Checksum:"    << hex << uppercase << FirmwareVolumeHeader.Checksum << "h (valid)\n";

    if (isExt) {
        ss << "Extended header size:" << hex << FirmwareVolumeExtHeader.ExtHeaderSize << "h\n"
           << "Volume GUID:\n" << FirmwareVolumeExtHeader.FvName.str(true) << "\n";
    }

    INT64 FreeSpaceSize = GetFreeSpaceSize();
    if (FreeSpaceSize != 0) {
        float rate = (float)FreeSpaceSize / (float)this->size;
        ss << setprecision(4) << setw(width) << "FV Space:" << (1 - rate) * 100 << "% Full\n";
    }

    string compressed = "No";
    if (isCompressed())
        compressed = "Yes";
    ss << "\nCompressed: " << compressed;

    InfoStr = QString::fromStdString(ss.str());
}

INT64 FirmwareVolume::getHeaderSize() const {
    return FirmwareVolumeHeaderSize;
}

EFI_GUID FirmwareVolume::getFvGuid(bool returnExt) const {
    if (returnExt) {
        if (isExt) {
            return FirmwareVolumeExtHeader.FvName;
        }
    }
    return FirmwareVolumeHeader.FileSystemGuid;
}

INT64 FirmwareVolume::GetFreeSpaceSize() const {
    for (Volume *vol : this->ChildVolume) {
        if (vol->getVolumeType() == VolumeType::Empty) {
            INT64 FreeSize = vol->getSize();
            return FreeSize;
        }
    }
    return 0;
}

bool FirmwareVolume::isValidFirmwareVolume(EFI_FIRMWARE_VOLUME_HEADER *address) {
    if (address->Signature != 0x4856465F)
        return false;

    // ZeroVector is reused by SBL FV, so skip checking ZeroVector value
    // UINT8* ZeroVector = address->ZeroVector;
    // if ((*(UINT64*)ZeroVector != 0x0) || ((*((UINT64*)ZeroVector + 1) != 0x0)))
    //     return false;
    if (address->FileSystemGuid.Data1 == 0xFFFFFFFF || address->FileSystemGuid.Data1 == 0x0) {
        return false;
    }

    // Skip caculating ZeroVector checksum
    UINT16 sumValue = CalculateSum16((UINT16 *) ((UINT8*)address + 16), (sizeof(EFI_FIRMWARE_VOLUME_HEADER) - 16) / sizeof(UINT16));
    if (sumValue != 0)
        return false;
    return true;
}

EFI_GUID FirmwareVolume::getVolumeGuid() const {
    return getFvGuid();
}

FirmwareVolume::~FirmwareVolume() {
}
