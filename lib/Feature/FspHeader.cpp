//
// Created by stephan on 9/5/2023.
//
#include <string>
#include "BaseLib.h"
#include "FspHeader.h"

using namespace BaseLibrarySpace;

FspHeader::FspHeader(UINT8 *buffer, INT64 length, INT64 offset):
        Volume(buffer, length, offset, false, nullptr) { }

bool FspHeader::isValid() const {
    return validFlag;
}

INT64 FspHeader::SelfDecode() {
    UINT32 s = sizeof(TABLES);
    if (s != size) {
        validFlag = false;
        return 0;
    }
    mTable = *(TABLES*)data;
    if ((mTable.FspInfoHeader.Signature == FSP_INFO_HEADER_SIGNATURE) &&
        (mTable.FspInfoExtendedHeader.Signature == FSP_INFO_EXTENDED_HEADER_SIGNATURE) &&
        (mTable.FspPatchTable.Signature == FSP_PATCH_TABLE_SIGNATURE)) {
        Type = VolumeType::FspHeader;
        validFlag = true;
        return size;
    } else {
        validFlag = false;
        return 0;
    }
}

void FspHeader::setInfoStr() {
    INT32 width = 25;
    stringstream ss;
    ss.setf(ios::left);
    ss << setw(width) << "SpecVersion:"      << hex << uppercase << (UINT16)mTable.FspInfoHeader.SpecVersion << "h\n"
       << setw(width) << "HeaderRevision:"   << hex << uppercase << (UINT16)mTable.FspInfoHeader.HeaderRevision << "h\n"
       << setw(width) << "ImageRevision:"    << hex << uppercase << (UINT16)mTable.FspInfoHeader.ImageRevision << "h\n"
       << setw(width) << "ImageId:"          << hex << uppercase << charToString(mTable.FspInfoHeader.ImageId, 8)  << "\n"
       << setw(width) << "ImageSize:"        << hex << uppercase << mTable.FspInfoHeader.ImageSize << "h\n"
       << setw(width) << "ImageBase:"        << hex << uppercase << mTable.FspInfoHeader.ImageBase << "h\n"
       << setw(width) << "ImageAttribute:"            << hex << uppercase << mTable.FspInfoHeader.ImageAttribute << "h\n"
       << setw(width) << "ComponentAttribute:"        << hex << uppercase << mTable.FspInfoHeader.ComponentAttribute << "h\n"
       << setw(width) << "CfgRegionOffset:"           << hex << uppercase << mTable.FspInfoHeader.CfgRegionOffset << "h\n"
       << setw(width) << "CfgRegionSize:"             << hex << uppercase << mTable.FspInfoHeader.CfgRegionSize << "h\n"
       << setw(width) << "TempRamInitEntryOffset:"    << hex << uppercase << mTable.FspInfoHeader.TempRamInitEntryOffset << "h\n"
       << setw(width) << "NotifyPhaseEntryOffset:"    << hex << uppercase << mTable.FspInfoHeader.NotifyPhaseEntryOffset << "h\n"
       << setw(width) << "FspMemoryInitEntryOffset:"  << hex << uppercase << mTable.FspInfoHeader.FspMemoryInitEntryOffset << "h\n"
       << setw(width) << "TempRamExitEntryOffset:"    << hex << uppercase << mTable.FspInfoHeader.TempRamExitEntryOffset << "h\n"
       << setw(width) << "FspSiliconInitEntryOffset:" << hex << uppercase << mTable.FspInfoHeader.FspSiliconInitEntryOffset << "h\n"
       << setw(width) << "FspMultiPhaseSiInitEntryOffset:"   << hex << uppercase << mTable.FspInfoHeader.FspMultiPhaseSiInitEntryOffset << "h\n"
       << setw(width) << "ExtendedImageRevision:"     << hex << uppercase << mTable.FspInfoHeader.ExtendedImageRevision << "h\n"
       << setw(width) << "FspMultiPhaseMemInitEntryOffset:"  << hex << uppercase << mTable.FspInfoHeader.FspMultiPhaseMemInitEntryOffset << "h\n"
       << setw(width) << "FspSmmInitEntryOffset:"     << hex << uppercase << mTable.FspInfoHeader.FspSmmInitEntryOffset << "h\n"
       << setw(width) << "FspProducerId:"             << hex << uppercase << charToString(mTable.FspInfoExtendedHeader.FspProducerId, 6) << "\n"
       << setw(width) << "FspProducerRevision:"       << hex << uppercase << mTable.FspInfoExtendedHeader.FspProducerRevision << "h\n"
       << setw(width) << "FspProducerDataSize:"       << hex << uppercase << mTable.FspInfoExtendedHeader.FspProducerDataSize << "h\n"
       << setw(width) << "BuildTimeStamp:"            << hex << uppercase << mTable.FspProduceDataType1.BuildTimeStamp << "\n"
       << setw(width) << "PatchEntryNum:"             << hex << uppercase << mTable.FspPatchTable.PatchEntryNum << "h\n";
    InfoStr = QString::fromStdString(ss.str());
}

bool FspHeader::isFspHeader(const UINT8 *ImageBase) {
    auto *signature = (UINT32*)ImageBase;
    if (*signature == FSP_INFO_HEADER_SIGNATURE) {
        return true;
    }
    return false;
}

FspHeader::~FspHeader() = default;
