//
// Created by stephan on 9/5/2023.
//
#include "UEFI/GuidDatabase.h"
#include "BaseLib.h"
#include "MicrocodeClass.h"

using namespace BaseLibrarySpace;

MicrocodeHeaderClass::~MicrocodeHeaderClass() = default;

MicrocodeHeaderClass::MicrocodeHeaderClass(UINT8* buffer, INT64 length, INT64 offset):
    Volume(buffer, length, offset, false, nullptr) { }

bool MicrocodeHeaderClass::CheckValidation() {
    microcodeHeader = *(CPU_MICROCODE_HEADER*)data;
    if (microcodeHeader.HeaderVersion == 0x1) {
        return true;
    }
    return false;
}

INT64 MicrocodeHeaderClass::SelfDecode() {
    Type = VolumeType::Microcode;
    microcodeHeader = *(CPU_MICROCODE_HEADER*)data;
    if (microcodeHeader.HeaderVersion == 0xFFFFFFFF) {
        isEmpty = true;
        return 0;
    }

    UINT32 ExtendedTableLength = microcodeHeader.TotalSize - (microcodeHeader.DataSize + sizeof(CPU_MICROCODE_HEADER));
    if (ExtendedTableLength != 0) {
        ExtendedTableHeader = (CPU_MICROCODE_EXTENDED_TABLE_HEADER *)(data + microcodeHeader.DataSize + sizeof(CPU_MICROCODE_HEADER));
        if ((ExtendedTableLength > sizeof(CPU_MICROCODE_EXTENDED_TABLE_HEADER)) && ((ExtendedTableLength & 0x3) == 0)) {
            UINT32 CheckSum32 = CalculateSum32((UINT32 *) ExtendedTableHeader, ExtendedTableLength);
            if (CheckSum32 != 0)
                return 0;
            UINT32 ExtendedTableCount = ExtendedTableHeader->ExtendedSignatureCount;
            if (ExtendedTableCount <= (ExtendedTableLength - sizeof(CPU_MICROCODE_EXTENDED_TABLE_HEADER)) / sizeof(CPU_MICROCODE_EXTENDED_TABLE)) {
                auto *ExtendedTable = (CPU_MICROCODE_EXTENDED_TABLE *)(ExtendedTableHeader + 1);
                for (UINT32 Index = 0; Index < ExtendedTableCount; Index++) {
                    ExtendedMicrocodeList.push_back(*ExtendedTable);
                    ExtendedTable += 1;
                }
            }
        }
    }
    size = microcodeHeader.TotalSize;
    return size;
}

void MicrocodeHeaderClass::setInfoStr() {
    INT32 width = 20;
    std::stringstream ss;
    ss.setf(std::ios::left);

    ss << "Microcode Info:" << "\n"
       << std::setw(width) << "Offset:" << std::hex << std::uppercase << offsetFromBegin << "h\n";
    if (!isEmpty) {
        ss << std::setw(width) << "HeaderVersion:" << std::hex << std::uppercase << microcodeHeader.HeaderVersion << "h\n"
           << std::setw(width) << "UpdateRevision:" << std::hex << std::uppercase << microcodeHeader.UpdateRevision << "h\n"
           << std::setw(width) << "Date:" << std::hex << std::uppercase << microcodeHeader.Date.Bits.Year << "-" << microcodeHeader.Date.Bits.Month << "-" << microcodeHeader.Date.Bits.Day << "\n"
           << std::setw(width) << "ProcessorSignature:" << std::hex << std::uppercase << microcodeHeader.ProcessorSignature.Uint32 << "h\n"
           << std::setw(width) << "Checksum:" << std::hex << std::uppercase << microcodeHeader.Checksum << "h\n"
           << std::setw(width) << "LoaderRevision:" << std::hex << std::uppercase << microcodeHeader.LoaderRevision << "h\n"
           << std::setw(width) << "ProcessorFlags:" << std::hex << std::uppercase << microcodeHeader.ProcessorFlags << "h\n"
           << std::setw(width) << "DataSize:" << std::hex << std::uppercase << microcodeHeader.DataSize << "h\n"
           << std::setw(width) << "TotalSize:" << std::hex << std::uppercase << microcodeHeader.TotalSize << "h\n";
    }

    if (!ExtendedMicrocodeList.empty()) {
        for (auto ExtendedMicrocode:ExtendedMicrocodeList) {
            ss << "\nExtended Microcode Info:" << "\n"
               << std::setw(width) << "ProcessorSignature:" << std::hex << std::uppercase << ExtendedMicrocode.ProcessorSignature.Uint32 << "h\n";
        }
    }

    InfoStr = QString::fromStdString(ss.str());
}

QVector<INT64> MicrocodeHeaderClass::SearchMicrocodeEntryNum(const UINT8* buffer, INT64 MicrocodeRegionSize) {
    INT64 searchOffset = 0;
    UINT32 HeaderVersion;
    QVector<INT64> MicrocodeEntryList;

    while (searchOffset < MicrocodeRegionSize) {
        HeaderVersion = *(UINT32 *)(buffer + searchOffset);
        if (HeaderVersion == 0x1) {
            MicrocodeEntryList.push_back(searchOffset);
        }
        searchOffset += 0x100;
    }

    return MicrocodeEntryList;
}

QString MicrocodeHeaderClass::getUserDefinedName() const {
    QString ItemName = "Microcode  " + QString::number(microcodeHeader.ProcessorSignature.Uint32, 16).toUpper();
    return ItemName;
}

MicrocodeVersion::MicrocodeVersion(UINT8 *buffer, INT64 length, INT64 offset):
    Volume(buffer, length, offset, false, nullptr) { }

INT64 MicrocodeVersion::SelfDecode() {
    Type = VolumeType::Microcodeversion;
    FwVersion = *(UINT32 *)data;
    LowestSupportedVersion = *(UINT32 *)(data + 4);
    FwVersionString = wcharToString((CHAR16 *)(data + 8), size, true);
    return size;
}

void MicrocodeVersion::setInfoStr() {
    INT32 width = 25;
    std::stringstream ss;
    ss.setf(std::ios::left);

    ss << setw(width) << "FwVersion:"              << hex << uppercase << FwVersion << "h\n"
       << setw(width) << "LowestSupportedVersion:" << hex << uppercase << LowestSupportedVersion << "h\n"
       << setw(width) << "FwVersionString:"        << FwVersionString << "\n";

    InfoStr = QString::fromStdString(ss.str());
}

MicrocodeVersion::~MicrocodeVersion() = default;
