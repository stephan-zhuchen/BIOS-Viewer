#include "CompressedVolume.h"
#include "Lz4Decompress/Lz4Decompress.h"
#include "LzmaDecompress/LzmaDecompressLib.h"
#include "UefiFileSystem/FirmwareVolume.h"
#include <QDebug>

CompressedVolume::CompressedVolume(UINT8* buffer, INT64 length, INT64 offset, Volume* parent):
    Volume(buffer, length, offset, false, parent) { }

CompressedVolume::~CompressedVolume() { }

QString CompressedVolume::GetComprssedType() {
    return ComprssedType;
}

bool CompressedVolume::IsCompressedVolume(LOADER_COMPRESSED_HEADER *buffer) {
    return IS_COMPRESSED(buffer);
}

bool CompressedVolume::CheckValidation() {
    return IS_COMPRESSED(data);
}

INT64 CompressedVolume::SelfDecode() {
    Type = VolumeType::Compressed;
    RETURN_STATUS status;
    UINT32 ScratchSize;
    VOID* scratch;
    CompressHdr = (LOADER_COMPRESSED_HEADER*)data;
    if (CompressHdr->Signature == LZ4_SIGNATURE) {
        ComprssedType = "Lz4";
        size = sizeof(LOADER_COMPRESSED_HEADER) + CompressHdr->CompressedSize;
        ScratchSize = 0;
        status = Lz4DecompressGetInfo(data + sizeof(LOADER_COMPRESSED_HEADER), CompressHdr->CompressedSize, &decompressedSize, &ScratchSize);
        if (status != RETURN_SUCCESS) {
            return 0;
        }

        DecompressedBufferOnHeap = new UINT8[decompressedSize];
        scratch = malloc(ScratchSize);
        status = Lz4Decompress(data + sizeof(LOADER_COMPRESSED_HEADER), CompressHdr->CompressedSize, DecompressedBufferOnHeap, scratch);
        if (status != RETURN_SUCCESS) {
            free(scratch);
            return 0;
        }
        free(scratch);
    } else if (CompressHdr->Signature == LZMA_SIGNATURE) {
        ComprssedType = "LZMA";
        size = sizeof(LOADER_COMPRESSED_HEADER) + CompressHdr->CompressedSize;
        ScratchSize = 0;
        status = LzmaUefiDecompressGetInfo(data + sizeof(LOADER_COMPRESSED_HEADER), CompressHdr->CompressedSize, &decompressedSize, &ScratchSize);
        if (status != RETURN_SUCCESS) {
            return 0;
        }

        DecompressedBufferOnHeap = new UINT8[decompressedSize];
        scratch = malloc(ScratchSize);
        status = LzmaUefiDecompress(data + sizeof(LOADER_COMPRESSED_HEADER), CompressHdr->CompressedSize, DecompressedBufferOnHeap, scratch);
        if (status != RETURN_SUCCESS) {
            free(scratch);
            return 0;
        }
        free(scratch);
    }

    return decompressedSize;
}

void CompressedVolume::DecodeChildVolume() {
    Volume *volume;
    INT64 VolumeOffset = 0;
    while (VolumeOffset < CompressHdr->Size) {
        auto fvHeader = (EFI_FIRMWARE_VOLUME_HEADER*)(DecompressedBufferOnHeap + VolumeOffset);
        if (FirmwareVolume::isValidFirmwareVolume(fvHeader)) {
            volume = new FirmwareVolume(DecompressedBufferOnHeap + VolumeOffset, CompressHdr->Size - VolumeOffset, VolumeOffset, true, this);
            if (volume->SelfDecode() != 0) {
                volume->DecodeChildVolume();
            }
            ChildVolume.push_back(volume);
            VolumeOffset += volume->getSize();
        } else {
            volume = new Volume(DecompressedBufferOnHeap + VolumeOffset, CompressHdr->Size - VolumeOffset, VolumeOffset, true, this);
            ChildVolume.push_back(volume);
            break;
        }
    }
}

void CompressedVolume::setInfoStr() {
    using namespace std;
    if (InfoStr != "") {
        return;
    }
    INT32 width = 15;
    stringstream ss;
    float rate;
    ss.setf(ios::left);

    ss << setw(width) << "Compression algorithm:" << ComprssedType.toStdString() << "\n"
       << setw(width) << "Decompressed size:" << hex << uppercase << CompressHdr->Size << "h\n";
    rate = (float)(CompressHdr->CompressedSize) / (float)CompressHdr->Size;
    ss << setprecision(4) << setw(width) << "Compresse rate:" << rate * 100 << "%\n";

    InfoStr = QString::fromStdString(ss.str());
}
