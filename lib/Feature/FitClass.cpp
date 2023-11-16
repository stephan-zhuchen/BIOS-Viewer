//
// Created by stephan on 9/5/2023.
//
#include "BaseLib.h"
#include "FitClass.h"
#include "Feature/AcmClass.h"
#include "Feature/MicrocodeClass.h"

using namespace BaseLibrarySpace;

FitTableClass::FitTableClass(UINT8 *fv, INT64 length) {
    INT64 FitTableAddress = *(INT64*)(fv + length - DEFAULT_FIT_TABLE_POINTER_OFFSET) & 0xFFFFFF;
    FitTableAddress = adjustBufferAddress(0x1000000, FitTableAddress, length); // get the relative address of FIT table
    if (FitTableAddress > length || FitTableAddress < 0) {
        throw BiosException("NO FIT table!");
    }
    FitHeader = *(FIRMWARE_INTERFACE_TABLE_ENTRY*)(fv + FitTableAddress);
    UINT64 FitSignature = FitHeader.Address;
    if (FitSignature == (UINT64)FIT_SIGNATURE) {
        isValid = true;
        FitEntryNum = *(UINT32*)(FitHeader.Size) & 0xFFFFFF;

        UINT8 Checksum = CalculateSum8((UINT8 *) (fv + FitTableAddress),
                                       sizeof(FIRMWARE_INTERFACE_TABLE_ENTRY) * FitEntryNum);
        if (Checksum == 0) {
            isChecksumValid = true;
        }

        for (INT64 index = 1; index < FitEntryNum; ++index) {
            FIRMWARE_INTERFACE_TABLE_ENTRY FitEntry = *(FIRMWARE_INTERFACE_TABLE_ENTRY*)(fv + FitTableAddress + sizeof(FIRMWARE_INTERFACE_TABLE_ENTRY) * index);
            FitEntries.push_back(FitEntry);
            if (FitEntry.Type == FIT_TABLE_TYPE_MICROCODE) {
                UINT64 MicrocodeAddress = FitEntry.Address & 0xFFFFFF;
                UINT64 RelativeMicrocodeAddress = adjustBufferAddress(0x1000000, MicrocodeAddress, length);
                if (RelativeMicrocodeAddress > (UINT64)length)
                    continue;
                auto *MicrocodeEntry = new MicrocodeHeaderClass(fv + RelativeMicrocodeAddress, 0, MicrocodeAddress);
                if (!MicrocodeEntry->CheckValidation()) {
                    delete MicrocodeEntry;
                    continue;
                }
                MicrocodeEntry->SelfDecode();
                MicrocodeEntries.push_back(MicrocodeEntry);
            } else if (FitEntry.Type == FIT_TABLE_TYPE_STARTUP_ACM) {
                UINT64 AcmAddress = FitEntry.Address & 0xFFFFFF;
                UINT64 RelativeAcmAddress = adjustBufferAddress(0x1000000, AcmAddress, length);
                if (RelativeAcmAddress > (UINT64)length)
                    continue;
                auto *AcmEntry = new AcmHeaderClass(fv + RelativeAcmAddress, 0, AcmAddress);
                AcmEntry->SelfDecode();
                AcmEntries.push_back(AcmEntry);
            } else if (FitEntry.Type == FIT_TABLE_TYPE_KEY_MANIFEST || FitEntry.Type == FIT_TABLE_TYPE_BOOT_POLICY_MANIFEST) {
                // Use external BpmGen2 tool to parse KM and BPM info
                continue;
            }
        }
    } else {
        isValid = false;
        throw BiosException("NO FIT table!");
    }
}

FitTableClass::~FitTableClass() {
    for (auto MicrocodeEntry:MicrocodeEntries)
        safeDelete(MicrocodeEntry);
    for (auto AcmEntry:AcmEntries)
        safeDelete(AcmEntry);
}

QString FitTableClass::getTypeName(UINT8 type) {
    QString typeName;
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
            typeName = "BIOS Data Area (0xD)";
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
