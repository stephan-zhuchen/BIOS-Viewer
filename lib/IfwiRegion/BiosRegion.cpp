//
// Created by stephan on 9/2/2023.
//
#include "BaseLib.h"
#include "BiosRegion.h"
#include "UEFI/GuidDatabase.h"
#include "UEFI/PiFirmwareFile.h"
#include "UefiFileSystem/CommonSection.h"
#include <thread>

using namespace BaseLibrarySpace;

BiosRegion::BiosRegion(UINT8 *buffer, INT64 length, INT64 offset):
        Volume(buffer, length, offset, false, nullptr) { }

bool BiosRegion::CheckValidation() {
    return Volume::CheckValidation();
}

INT64 BiosRegion::SelfDecode() {
    Type = VolumeType::BIOS;
    // Initialize the FIT table.
    try {
        FitValid = true;
        FitTable = new FitTableClass(data, size);
        const INT64 IBB_length = 0x400000;
        if (size >= IBB_length * 2) {
            // Check for resiliency.
            INT64 IBB_begin_address = size - IBB_length;
            INT64 IBBR_begin_address = size - IBB_length * 2;
            isResiliency = true;
            for (int index = 0; index < IBB_length; ++index) {
                if (data[IBB_begin_address + index] != data[IBBR_begin_address + index]) {
                    isResiliency = false;
                    break;
                }
            }
        }
    } catch (...) {
        FitValid = false;
        FitTable = nullptr;
        return 0;
    }
    return size;
}

void BiosRegion::DecodeChildVolume() {
    vector<thread> threadPool;
    auto FvDecoder = [this](int index) {
        Volume *volume = this->ChildVolume.at(index);
        volume->DecodeChildVolume();
    };
    for (int idx = 0; idx < this->ChildVolume.size(); ++idx) {
        threadPool.emplace_back(FvDecoder, idx);
    }
    for (class thread& t:threadPool) {
        t.join();
    }
}

void BiosRegion::setInfoStr() {
    INT32 width = 15;
    stringstream ss;
    ss.setf(ios::left);

    if (foundBiosID) {
        ss << setw(width) << "BIOS ID:" << BiosID << "\n\n";
        if (isResiliency)
            ss << "Resiliency ";
        if (DebugFlag)
            ss << "Debug BIOS" << "\n";
        else
            ss << "Release BIOS" << "\n";
    }

    InfoStr = QString::fromStdString(ss.str());
}

BiosRegion::~BiosRegion() {
    safeDelete(FitTable);
}

void BiosRegion::setBiosID() {
    for (INT64 idx = ChildVolume.size(); idx > 0; --idx) {
        Volume *volume = ChildVolume.at(idx - 1);
        for (auto file:volume->ChildVolume) {
            if (file->getVolumeGuid() == GuidDatabase::gBiosIdGuid) {
                if (file->ChildVolume.isEmpty())
                    return;
                Volume *sec = file->ChildVolume.at(0);
                auto *biosIdStr = (CHAR16*)(sec->getData() + sizeof(EFI_COMMON_SECTION_HEADER) + 8);
                BiosID = wstringToString(biosIdStr);
                foundBiosID = true;
                setDebugFlag();
                return;
            }
        }
    }
}

void BiosRegion::setDebugFlag() {
    INT64 pos = BiosID.find('.');
    if (pos != string::npos && pos + 1 < BiosID.size()) {
        if (BiosID[pos + 1] == 'R') {
            DebugFlag = false;
        }
        else if (BiosID[pos + 1] == 'D') {
            DebugFlag = true;
        }
    }
}

void BiosRegion::collectAcpiTable(Volume *parent) {
    if (parent == nullptr) {
        parent = this;
    }
    for (Volume *vol:parent->ChildVolume) {
        if (vol->getVolumeType() == VolumeType::AcpiTable) {
            CommonSection *sec = (CommonSection*)vol;
            AcpiTables.push_back(sec->getAcpiTable());
        }
    }
    for (Volume *vol:parent->ChildVolume) {
        collectAcpiTable(vol);
    }
}
