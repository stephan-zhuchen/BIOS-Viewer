//
// Created by stephan on 9/5/2023.
//
#include <string>
#include "AcmClass.h"
#include "BaseLib.h"
#include "UEFI/GuidDatabase.h"

using namespace std;
using namespace BaseLibrarySpace;

AcmHeaderClass::AcmHeaderClass(UINT8* buffer, INT64 length, INT64 offset):
    Volume(buffer, length, offset, false, nullptr) { }

void AcmHeaderClass::setInfoStr() {
    INT32 width = 20;
    stringstream ss;
    ss.setf(ios::left);
    ss << setw(width) << "Guid:"    << AcmInfoTable->Guid.str(true) << "\n"
       << setw(width) << "Acm Version:"   << (UINT32)AcmVersion.AcmMajorVersion << "." << (UINT32)AcmVersion.AcmMinorVersion << "." << (UINT32)AcmVersion.AcmRevision << "\n";

    ss << setw(width) << "ModuleType:"    << hex << uppercase << acmHeader.ModuleType << "h\n"
       << setw(width) << "ModuleSubType:" << hex << uppercase << acmHeader.ModuleSubType << "h\n"
       << setw(width) << "HeaderLen:"     << hex << uppercase << acmHeader.HeaderLen << "h\n"
       << setw(width) << "HeaderVersion:" << hex << uppercase << acmHeader.HeaderVersion << "h\n"
       << setw(width) << "ChipsetId:"     << hex << uppercase << acmHeader.ChipsetId << "h\n"
       << setw(width) << "Flags:"         << hex << uppercase << acmHeader.Flags << "h\n"
       << setw(width) << "ModuleVendor:"  << hex << uppercase << acmHeader.ModuleVendor << "h\n"
       << setw(width) << "Date:"          << hex << uppercase << (acmHeader.Date >> 16) << "-" << ((acmHeader.Date & 0xFF00) >> 8) << "-" << (acmHeader.Date & 0xFF) << "\n"
       << setw(width) << "Size:"          << hex << uppercase << acmHeader.Size << "h\n"
       << setw(width) << "AcmSvn:"        << hex << uppercase << acmHeader.AcmSvn << "h\n"
       << setw(width) << "SeAcmSvn:"      << hex << uppercase << acmHeader.SeAcmSvn << "h\n"
       << setw(width) << "CodeControl:"   << hex << uppercase << acmHeader.CodeControl << "h\n"
       << setw(width) << "ErrorEntryPoint:" << hex << uppercase << acmHeader.ErrorEntryPoint << "h\n"
       << setw(width) << "GdtLimit:"      << hex << uppercase << acmHeader.GdtLimit << "h\n"
       << setw(width) << "GdtBasePtr:"    << hex << uppercase << acmHeader.GdtBasePtr << "h\n"
       << setw(width) << "SegSel:"        << hex << uppercase << acmHeader.SegSel << "h\n"
       << setw(width) << "EntryPoint:"    << hex << uppercase << acmHeader.EntryPoint << "h\n"
       << setw(width) << "KeySize:"       << hex << uppercase << acmHeader.KeySize * 4 << "h\n"
       << setw(width) << "ScratchSize:"   << hex << uppercase << acmHeader.ScratchSize * 4 << "h\n";

//    if (isAcm3) {
//        ss << "ACM RSA Public Key:\n" << DumpHex(ExtAcmHeader3.Rsa3072PubKey, 384) << "\n"
//           << "ACM RSA Signature:\n" << DumpHex(ExtAcmHeader3.Rsa3072Sig, 384) << "\n"
//           << "Scratch:\n" << DumpHex(ExtAcmHeader3.Scratch, 832);
//    } else {
//        ss << setw(width) << "ACM RSA Public Key Exponent:" << hex << uppercase << ExtAcmHeader.RsaPubExp << "h\n"
//           << "ACM RSA Public Key:\n" << DumpHex(ExtAcmHeader.Rsa2048PubKey, 256) << "\n"
//           << "ACM RSA Signature:\n" << DumpHex(ExtAcmHeader.Rsa2048Sig, 256) << "\n"
//           << "Scratch:\n" << DumpHex(ExtAcmHeader.Scratch, 572);
//    }

    InfoStr = QString::fromStdString(ss.str());
}

INT64 AcmHeaderClass::SelfDecode() {
    Type = VolumeType::BtgAcm;
    acmHeader = *(ACM_HEADER*)data;
    ValidFlag = (acmHeader.ModuleType == ACM_MODULE_TYPE_CHIPSET_ACM);
    ProdFlag = (acmHeader.Flags & 0x8000) == 0;
    if (acmHeader.HeaderVersion < ACM_HEADER_VERSION_3) {
        ExtAcmHeader = *(Ext_ACM_Header*)(data + sizeof(ACM_HEADER));
    } else {
        isAcm3 = true;
        ExtAcmHeader3 = *(Ext_ACM_Header3*)(data + sizeof(ACM_HEADER));
    }
    auto *AcmPtr = (UINT8 *)data;
    AcmPtr += acmHeader.HeaderLen * 4;
    AcmPtr += acmHeader.ScratchSize * 4;
    AcmInfoTable = (ACM_INFO_TABLE *)AcmPtr;
    if (AcmInfoTable->AitVersion < ACM_INFO_TABLE_VERSION_9) {
        if (AcmInfoTable->Guid == GuidDatabase::gTxtAcmInfoTableGuid) {
            AcmVersion.AcmMajorVersion = AcmInfoTable->AitRevision[0];
            AcmVersion.AcmMinorVersion = AcmInfoTable->AitRevision[1];
            AcmVersion.AcmRevision = AcmInfoTable->AitRevision[2];
        }
    } else if (AcmInfoTable->AitVersion == ACM_INFO_TABLE_VERSION_9) {
        AcmPtr += sizeof(ACM_INFO_TABLE);
        UINT8 *StartPtr = AcmPtr;
        bool TableFound = false;
        while (memcmp(((VAR_LIST *)AcmPtr)->Id, NULL_TERMINATOR_ID, 4) != 0 && ((AcmPtr - StartPtr) < 300)) {
            //
            // Check if ACM VERSION INFO table found
            //
            if (!memcmp(((VAR_LIST *)AcmPtr)->Id, ACM_VERSION_INFORMATION_LIST_ID, 4)) {
                TableFound = TRUE;
                break;
            }
            AcmPtr += ((VAR_LIST *)AcmPtr)->Length;
        }
        if (TableFound) {
            AcmVersion.AcmMajorVersion = ((ACM_VER_INFO_TABLE *)AcmPtr)->AcmRev[0];
            AcmVersion.AcmMinorVersion = ((ACM_VER_INFO_TABLE *)AcmPtr)->AcmRev[1];
            AcmVersion.AcmRevision = ((ACM_VER_INFO_TABLE *)AcmPtr)->AcmRev[2];
        }
    }
    return size;
}

AcmHeaderClass::~AcmHeaderClass() = default;
