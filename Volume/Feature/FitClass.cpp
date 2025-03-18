//
// Created by stephan on 9/5/2023.
//
#include "BaseLib.h"
#include "FitClass.h"
#include "Feature/AcmClass.h"
#include "Feature/MicrocodeClass.h"
#include "Feature/FspBootManifest.h"

using namespace BaseLibrarySpace;

FitTableClass::FitTableClass(UINT8* buffer, INT64 length, INT64 offset):
    Volume(buffer, length, offset, false, nullptr) { }

FitTableClass::~FitTableClass() {
    for (auto MicrocodeEntry:MicrocodeEntries)
        safeDelete(MicrocodeEntry);
    for (auto AcmEntry:AcmEntries)
        safeDelete(AcmEntry);
    safeDelete(FbmEntry);
}

INT64 FitTableClass::SelfDecode() {
    INT64 FitTableAddress = *(INT64*)(data + size - DEFAULT_FIT_TABLE_POINTER_OFFSET) & 0xFFFFFF;
    FitTableAddress = adjustBufferAddress(0x1000000, FitTableAddress, size); // get the relative address of FIT table
    if (FitTableAddress > size || FitTableAddress < 0) {
        isValid = false;
        return 0;
    }
    FitHeader = *(FIRMWARE_INTERFACE_TABLE_ENTRY*)(data + FitTableAddress);
    UINT64 FitSignature = FitHeader.Address;
    if (FitSignature == (UINT64)FIT_SIGNATURE) {
        isValid = true;
        FitEntryNum = *(UINT32*)(FitHeader.Size) & 0xFFFFFF;

        UINT8 Checksum = CalculateSum8((UINT8 *) (data + FitTableAddress),
                                       sizeof(FIRMWARE_INTERFACE_TABLE_ENTRY) * FitEntryNum);
        if (Checksum == 0) {
            isChecksumValid = true;
        }

        for (INT64 index = 1; index < FitEntryNum; ++index) {
            FIRMWARE_INTERFACE_TABLE_ENTRY FitEntry = *(FIRMWARE_INTERFACE_TABLE_ENTRY*)(data + FitTableAddress + sizeof(FIRMWARE_INTERFACE_TABLE_ENTRY) * index);
            FitEntries.push_back(FitEntry);
            if (FitEntry.Type == FIT_TABLE_TYPE_MICROCODE) {
                UINT64 MicrocodeAddress = FitEntry.Address & 0xFFFFFF;
                UINT64 RelativeMicrocodeAddress = adjustBufferAddress(0x1000000, MicrocodeAddress, size);
                if (RelativeMicrocodeAddress > (UINT64)size)
                    continue;
                auto *MicrocodeEntry = new MicrocodeHeaderClass(data + RelativeMicrocodeAddress, 0, MicrocodeAddress);
                MicrocodeEntry->SelfDecode();
                MicrocodeEntries.push_back(MicrocodeEntry);
            } else if (FitEntry.Type == FIT_TABLE_TYPE_STARTUP_ACM) {
                UINT64 AcmAddress = FitEntry.Address & 0xFFFFFF;
                UINT64 RelativeAcmAddress = adjustBufferAddress(0x1000000, AcmAddress, size);
                if (RelativeAcmAddress > (UINT64)size)
                    continue;
                auto *AcmEntry = new AcmHeaderClass(data + RelativeAcmAddress, 0, AcmAddress);
                AcmEntry->SelfDecode();
                if (AcmEntry->isValid()) {
                    AcmEntries.push_back(AcmEntry);
                } else {
                    delete AcmEntry;
                }
            } else if (FitEntry.Type == FIT_TABLE_TYPE_KEY_MANIFEST || FitEntry.Type == FIT_TABLE_TYPE_BOOT_POLICY_MANIFEST) {
                // Use external BpmGen2 tool to parse KM and BPM info
                continue;
            } else if (FitEntry.Type == FIT_TABLE_TYPE_BIOS_DATA_AREA) {
                UINT64 FbmAddress = FitEntry.Address & 0xFFFFFF;
                UINT64 RelativeFbmAddress = adjustBufferAddress(0x1000000, FbmAddress, size);
                FbmEntry = new FspBootManifestClass(data + RelativeFbmAddress, 0, FbmAddress);
                FbmEntry->SelfDecode();
                if (!FbmEntry->isValid()) {
                    safeDelete(FbmEntry);
                }
            }
        }
    } else {
        isValid = false;
        return 0;
    }
    return size;
}

string FitTableClass::getTypeName(UINT8 type) {
    string typeName;
    switch (type) {
        case FIT_TABLE_TYPE_HEADER:
            typeName = "Header";
            break;
        case FIT_TABLE_TYPE_MICROCODE:
            typeName = "Microcode (0x1)";
            break;
        case FIT_TABLE_TYPE_STARTUP_ACM:
            typeName = "Startup ACM (0x2)";
            break;
        case FIT_TABLE_TYPE_DIAGNST_ACM:
            typeName = "Diagnst ACM (0x3)";
            break;
        case FIT_TABLE_TYPE_PROT_BOOT_POLICY:
            typeName = "Port Boot Policy (0x4)";
            break;
        case FIT_TABLE_TYPE_BIOS_MODULE:
            typeName = "BIOS Module (0x7)";
            break;
        case FIT_TABLE_TYPE_TPM_POLICY:
            typeName = "TPM Policy (0x8)";
            break;
        case FIT_TABLE_TYPE_BIOS_POLICY:
            typeName = "BIOS Policy (0x9)";
            break;
        case FIT_TABLE_TYPE_TXT_POLICY:
            typeName = "TXT Policy (0xA)";
            break;
        case FIT_TABLE_TYPE_KEY_MANIFEST:
            typeName = "Key Manifest (0xB)";
            break;
        case FIT_TABLE_TYPE_BOOT_POLICY_MANIFEST:
            typeName = "Boot Policy Manifest (0xC)";
            break;
        case FIT_TABLE_TYPE_BIOS_DATA_AREA:
            typeName = "FSP Boot Manifest (0xD)";
            break;
        case FIT_TABLE_TYPE_CSE_SECURE_BOOT:
            typeName = "CSE Secure Boot (0x10)";
            break;
        case FIT_TABLE_TYPE_VAB_PROVISION_TABLE:
            typeName = "VAB Provision Table (0x1A)";
            break;
        case FIT_TABLE_TYPE_VAB_BOOT_IMAGE_MANIFEST:
            typeName = "Boot Image Manifest (0x1B)";
            break;
        case FIT_TABLE_TYPE_VAB_BOOT_KEY_MANIFEST:
            typeName = "Boot Key Manifest (0x1C)";
            break;
        default:
            typeName = "Unknown";
            break;
    }
    return typeName;
}
