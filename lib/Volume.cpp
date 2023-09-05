//
// Created by stephan on 8/28/2023.
//

#include "Volume.h"
#include "BaseLib.h"
#include <string>

using  namespace BaseLibrarySpace;
const std::string SizeErrorMsg = "offset larger than size!";

Volume::Volume(UINT8* buffer, INT64 length, INT64 offset, bool Compressed, Volume* parent):
        data(buffer),
        size(length),
        offsetFromBegin(offset),
        Compressed(Compressed),
        ParentVolume(parent) {}

Volume::~Volume() {
    if (Type == VolumeType::Empty)
        return;
    cout << "~Volume at offset: 0x" << hex << offsetFromBegin << ", Type:" << static_cast<int>(Type) << endl;
    for (Volume* vol : ChildVolume) {
        safeDelete(vol);
    }
    safeArrayDelete(DecompressedBufferOnHeap);
}

EFI_GUID Volume::getGUID(INT64 offset) {
    if (offset > size) {
        throw BiosException(SizeErrorMsg);
    }
    EFI_GUID guid = *(EFI_GUID*)(data + offset);
    return guid;
}

UINT8 Volume::getUINT8(INT64 offset)  {
    if (offset > size) {
        throw BiosException(SizeErrorMsg);
    }
    return data[offset];
}

UINT16 Volume::getUINT16(INT64 offset) {
    if (offset > size) {
        throw BiosException(SizeErrorMsg);
    }
    return *(UINT16*)(data + offset);
}

UINT32 Volume::getUINT32(INT64 offset) {
    if (offset > size) {
        throw BiosException(SizeErrorMsg);
    }
    return *(UINT32*)(data + offset);
}

UINT8* Volume::getBytes(INT64 offset, INT64 length) {
    if (offset > size) {
        throw BiosException(SizeErrorMsg);
    }
    auto *value = new UINT8[length];
    for (INT64 i = 0; i < length; i++) {
        value[i] = data[offset + i];
    }
    return value;
}

UINT64 Volume::getUINT64(INT64 offset) {
    if (offset > size) {
        throw BiosException(SizeErrorMsg);
    }
    return *(UINT64*)(data + offset);
}

CHAR8 Volume::getINT8(INT64 offset) {
    if (offset > size) {
        throw BiosException(SizeErrorMsg);
    }
    return (CHAR8) data[offset];
}

INT16 Volume::getINT16(INT64 offset) {
    if (offset > size) {
        throw BiosException(SizeErrorMsg);
    }
    return *(INT16*)(data + offset);
}

INT32 Volume::getINT24(INT64 offset) {
    if (offset > size) {
        throw BiosException(SizeErrorMsg);
    }
    UINT8 value[4] {0};
    for (INT64 i = 0; i < 3; i++) {
        value[i] = data[offset + i];
    }
    return *(INT32*)value;
}

INT32 Volume::getINT32(INT64 offset) {
    if (offset > size) {
        throw BiosException(SizeErrorMsg);
    }
    return *(INT32*)(data + offset);
}

INT64 Volume::getINT64(INT64 offset) {
    if (offset > size) {
        throw BiosException(SizeErrorMsg);
    }
    return *(INT64*)(data + offset);
}

INT64 Volume::getHeaderSize() const {
    return 0;
}

bool Volume::CheckValidation() {
    return true;
}

INT64 Volume::SelfDecode() {
    return size;
}

void Volume::DecodeChildVolume() {}

void Volume::setInfoStr() {
    if (InfoStr != "") {
        return;
    }
}

EFI_GUID Volume::getVolumeGuid() const {
    return {};
}

void Volume::setInfoText(const QString &text) {
    InfoStr = text;
}

QString Volume::getUserDefinedName() const {
    return {};
}

void Volume::SearchDecompressedVolume(Volume *volume, std::vector<Decompressed *> &DecompressedVolumeList) {
    for (Volume* childVolume:volume->ChildVolume) {
        if (volume->Type == VolumeType::CommonSection && volume->DecompressedBufferOnHeap != nullptr) {
            UINT32 DecompressedSectionSize = volume->getHeaderSize() + volume->decompressedSize;

            vector<UINT8> DecompressedVolume;
            DecompressedVolume.reserve(DecompressedSectionSize);
            DecompressedVolume.insert(DecompressedVolume.end(), volume->data, volume->data + volume->getHeaderSize());
            DecompressedVolume.insert(DecompressedVolume.end(), volume->DecompressedBufferOnHeap,
                                      volume->DecompressedBufferOnHeap + volume->decompressedSize);

            Decompressed *decompressed = new Decompressed;
            decompressed->decompressedBuffer = DecompressedVolume;
            decompressed->decompressedOffset = volume->offsetFromBegin;
            decompressed->CompressedSize = volume->size;
            DecompressedVolumeList.push_back(decompressed);
            return;
        } else {
            SearchDecompressedVolume(childVolume, DecompressedVolumeList);
        }
    }
    return;
}

bool Volume::GetDecompressedVolume(vector<UINT8> &DecompressedVolume) {
    vector<Decompressed*> DecompressedVolumeList;
    SearchDecompressedVolume(this, DecompressedVolumeList);
    if (DecompressedVolumeList.empty()) {
        return false;
    }
    std::sort(DecompressedVolumeList.begin(), DecompressedVolumeList.end(),
              [](Decompressed * v1, Decompressed * v2){ return v1->decompressedOffset < v2->decompressedOffset; });
    DecompressedVolume = vector<UINT8>(this->data, this->data + size);
    UINT32 OffsetCorrection = 0;
    for (Decompressed *decompressed:DecompressedVolumeList) {
        UINT32 ReplaceOffset = decompressed->decompressedOffset - this->offsetFromBegin + OffsetCorrection;
        DecompressedVolume.erase(DecompressedVolume.begin() + ReplaceOffset, DecompressedVolume.begin() + ReplaceOffset + decompressed->CompressedSize);
        OffsetCorrection += decompressed->decompressedBuffer.size() - decompressed->CompressedSize;
        DecompressedVolume.insert(DecompressedVolume.begin() + ReplaceOffset, decompressed->decompressedBuffer.begin(), decompressed->decompressedBuffer.end());
    }

    for (Decompressed *decompressed:DecompressedVolumeList) {
        safeDelete(decompressed);
    }

    return true;
}
