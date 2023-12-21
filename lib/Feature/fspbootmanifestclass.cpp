#include "BaseLib.h"
#include "FspBootManifestClass.h"

using namespace BaseLibrarySpace;

FspBootManifestClass::FspBootManifestClass(UINT8* buffer, INT64 length, INT64 offset):
    Volume(buffer, length, offset, false, nullptr) { }

FspBootManifestClass::~FspBootManifestClass() = default;

INT64 FspBootManifestClass::SelfDecode() {
    Type = VolumeType::FspBootManifest;
    FbmStruct = *(FSP_BOOT_MANIFEST_STRUCTURE*)data;
    QString StructureId = QString::fromStdString(charToString((CHAR8*)FbmStruct.StructureId, 8));
    if (StructureId != "__FBMS__") {
        ValidFlag = false;
        return 0;
    }

    INT64 FspRegionOffset = sizeof(FSP_BOOT_MANIFEST_STRUCTURE);
    for (INT32 idx = 0; idx < FbmStruct.FspRgnCnt; ++idx) {
        FSP_REGION FspRegion;
        FspRegion.FSP_REGION_Header = *(FSP_REGION_STRUCTURE*)(data + FspRegionOffset);
        FspRegionOffset += sizeof(FSP_REGION_STRUCTURE);
        for (INT32 count = 0; count < FspRegion.FSP_REGION_Header.SegmentCnt; ++count) {
            IBB_SEGMENT segment = *(IBB_SEGMENT*)(data + FspRegionOffset);
            FspRegion.SegmentArray.push_back(segment);
            FspRegionOffset += sizeof(IBB_SEGMENT);
        }
        FspRegions.push_back(FspRegion);
    }

    INT64 KeyAndSigOffset = FbmStruct.KeySignatureOffset;
    KeyAndSignature.Header = *(KEY_AND_SIGNATURE_STRUCT_HEADER*)(data + KeyAndSigOffset);
    KeyAndSigOffset += sizeof(KEY_AND_SIGNATURE_STRUCT_HEADER);
    KeyAndSignature.RsaKey = *(RSA_PUBKEY*)(data + KeyAndSigOffset);
    KeyAndSigOffset += sizeof(RSA_PUBKEY);
    KeyAndSignature.KEY_Modulus = QByteArray((CHAR8*)(data + KeyAndSigOffset), KeyAndSignature.RsaKey.KeySizeBits / 8);
    KeyAndSigOffset += KeyAndSignature.RsaKey.KeySizeBits / 8;
    KeyAndSignature.SigScheme = *(UINT16*)(data + KeyAndSigOffset);
    KeyAndSigOffset += sizeof(UINT16);
    KeyAndSignature.SignatureRsa = *(RSASSA_SIGNATURE*)(data + KeyAndSigOffset);
    KeyAndSigOffset += sizeof(RSASSA_SIGNATURE);
    KeyAndSignature.Signature = QByteArray((CHAR8*)(data + KeyAndSigOffset), KeyAndSignature.SignatureRsa.KeySizeBits / 8);

    return sizeof(FSP_BOOT_MANIFEST_STRUCTURE);
}

void FspBootManifestClass::setInfoStr() {
    INT32 width = 24;
    INT32 indentSize = 4;
    stringstream ss;
    ss.setf(ios::left);
    ss << setw(width) << "StructureId:"          << charToString((CHAR8*)FbmStruct.StructureId, 8) << "\n";
    ss << setw(width) << "StructVersion:"        << hex << uppercase << (UINT32)FbmStruct.StructVersion << "h\n"
       << setw(width) << "KeySignatureOffset:"   << hex << uppercase << FbmStruct.KeySignatureOffset << "h\n"
       << setw(width) << "FspVersion:"           << hex << uppercase << (UINT32)(FbmStruct.FspVersion >> 8) << "." << (UINT32)(FbmStruct.FspVersion & 0xFF) << "\n"
       << setw(width) << "FspSvn:"               << hex << uppercase << (UINT32)FbmStruct.FspSvn << "h\n"
       << setw(width) << "Flags:"                << hex << uppercase << FbmStruct.Flags << "h\n"
       << setw(width) << "FSP component Number:" << hex << uppercase << (UINT32)FbmStruct.CompDigestCnt << "h\n";

    for (INT32 idx = 0; idx < FbmStruct.CompDigestCnt; ++idx) {
        width = 20;
        ss << "FSP Component List[" << idx << "]:\n"
           << setw(indentSize) << setfill(' ') << "" << setw(width) << "Fsp Component:" << (UINT32)FbmStruct.ComponentDigests[idx].ComponentID << " (" << GetFspComponentFromID(FbmStruct.ComponentDigests[idx].ComponentID) <<")\n"
           << setw(indentSize) << setfill(' ') << "" << setw(width) << "Size:" << (UINT32)FbmStruct.ComponentDigests[idx].ComponentDigests.Size << "h\n"
           << setw(indentSize) << setfill(' ') << "" << setw(width) << "Count:" << (UINT32)FbmStruct.ComponentDigests[idx].ComponentDigests.Count << "h\n";

        width = 16;
        ss << setw(indentSize) << setfill(' ') << "" << "Digest List[0]:\n"
           << setw(indentSize * 2) << setfill(' ') << "" << setw(width) << "Hash Algorithm:" << FbmStruct.ComponentDigests[idx].ComponentDigests.Sha384Digest.HashAlg
           << "h (" << GetHashAlgFromID(FbmStruct.ComponentDigests[idx].ComponentDigests.Sha384Digest.HashAlg) << ")\n"
           << setw(indentSize * 2) << setfill(' ') << "" << setw(width) << "Hash Size:" << FbmStruct.ComponentDigests[idx].ComponentDigests.Sha384Digest.Size << "h\n"
           << setw(indentSize * 2) << setfill(' ') << "" << "Digest Content:\n"
           << DumpHex((UINT8*)&FbmStruct.ComponentDigests[idx].ComponentDigests.Sha384Digest.HashBuffer, FbmStruct.ComponentDigests[idx].ComponentDigests.Sha384Digest.Size, 16, false, indentSize * 2);

        ss << setw(indentSize) << setfill(' ') << "" << "Digest List[1]:\n"
           << setw(indentSize * 2) << setfill(' ') << "" << setw(width) << "Hash Algorithm:" << FbmStruct.ComponentDigests[idx].ComponentDigests.Sha1Digest.HashAlg
           << "h (" << GetHashAlgFromID(FbmStruct.ComponentDigests[idx].ComponentDigests.Sha1Digest.HashAlg) << ")\n"
           << setw(indentSize * 2) << setfill(' ') << "" << setw(width) << "Hash Size:" << FbmStruct.ComponentDigests[idx].ComponentDigests.Sha1Digest.Size << "h\n"
           << setw(indentSize * 2) << setfill(' ') << "" << "Digest Content:\n"
           << DumpHex((UINT8*)&FbmStruct.ComponentDigests[idx].ComponentDigests.Sha1Digest.HashBuffer, FbmStruct.ComponentDigests[idx].ComponentDigests.Sha1Digest.Size, 16, false, indentSize * 2);

        ss << setw(indentSize) << setfill(' ') << "" << "Digest List[2]:\n"
           << setw(indentSize * 2) << setfill(' ') << "" << setw(width) << "Hash Algorithm:" << FbmStruct.ComponentDigests[idx].ComponentDigests.Sha256Digest.HashAlg
           << "h (" << GetHashAlgFromID(FbmStruct.ComponentDigests[idx].ComponentDigests.Sha256Digest.HashAlg) << ")\n"
           << setw(indentSize * 2) << setfill(' ') << "" << setw(width) << "Hash Size:" << FbmStruct.ComponentDigests[idx].ComponentDigests.Sha256Digest.Size << "h\n"
           << setw(indentSize * 2) << setfill(' ') << "" << "Digest Content:\n"
           << DumpHex((UINT8*)&FbmStruct.ComponentDigests[idx].ComponentDigests.Sha256Digest.HashBuffer, FbmStruct.ComponentDigests[idx].ComponentDigests.Sha256Digest.Size, 16, false, indentSize * 2);

        ss << setw(indentSize) << setfill(' ') << "" << "Digest List[3]:\n"
           << setw(indentSize * 2) << setfill(' ') << "" << setw(width) << "Hash Algorithm:" << FbmStruct.ComponentDigests[idx].ComponentDigests.Sha512Digest.HashAlg
           << "h (" << GetHashAlgFromID(FbmStruct.ComponentDigests[idx].ComponentDigests.Sha512Digest.HashAlg) << ")\n"
           << setw(indentSize * 2) << setfill(' ') << "" << setw(width) << "Hash Size:" << FbmStruct.ComponentDigests[idx].ComponentDigests.Sha512Digest.Size << "h\n"
           << setw(indentSize * 2) << setfill(' ') << "" << "Digest Content:\n"
           << DumpHex((UINT8*)&FbmStruct.ComponentDigests[idx].ComponentDigests.Sha512Digest.HashBuffer, FbmStruct.ComponentDigests[idx].ComponentDigests.Sha512Digest.Size, 16, false, indentSize * 2);

        ss << setw(indentSize) << setfill(' ') << "" << "Digest List[4]:\n"
           << setw(indentSize * 2) << setfill(' ') << "" << setw(width) << "Hash Algorithm:" << FbmStruct.ComponentDigests[idx].ComponentDigests.ShaSm3Digest.HashAlg
           << "h (" << GetHashAlgFromID(FbmStruct.ComponentDigests[idx].ComponentDigests.ShaSm3Digest.HashAlg) << ")\n"
           << setw(indentSize * 2) << setfill(' ') << "" << setw(width) << "Hash Size:" << FbmStruct.ComponentDigests[idx].ComponentDigests.ShaSm3Digest.Size << "h\n"
           << setw(indentSize * 2) << setfill(' ') << "" << "Digest Content:\n"
           << DumpHex((UINT8*)&FbmStruct.ComponentDigests[idx].ComponentDigests.ShaSm3Digest.HashBuffer, FbmStruct.ComponentDigests[idx].ComponentDigests.ShaSm3Digest.Size, 16, false, indentSize * 2);
    }

    ss << setw(width) << "FSP Region Info Count:"    << hex << uppercase << (UINT32)FbmStruct.FspRgnCnt << "h\n";
    for (INT32 idx = 0; idx < FbmStruct.FspRgnCnt; ++idx) {
        width = 20;
        ss << "FSP Region[" << idx << "]:\n"
           << setw(indentSize) << setfill(' ') << "" << setw(width) << "Fsp Component:"    << (UINT32)FspRegions[idx].FSP_REGION_Header.ComponentID << " (" << GetFspComponentFromID(FspRegions[idx].FSP_REGION_Header.ComponentID) <<")\n"
           << setw(indentSize) << setfill(' ') << "" << setw(width) << "FSP Segment Count" << (UINT32)FspRegions[idx].FSP_REGION_Header.SegmentCnt << "h\n";
        for (int count = 0; count < FspRegions[idx].FSP_REGION_Header.SegmentCnt; ++count) {
            ss << setw(indentSize) << setfill(' ') << "" << "Segment Array[" << idx << "]\n"
               << setw(indentSize * 2) << setfill(' ') << "" << "Flags:" << FspRegions[idx].SegmentArray[count].Flags << "h\n"
               << setw(indentSize * 2) << setfill(' ') << "" << "Base: "  << FspRegions[idx].SegmentArray[count].Base << "h\n"
               << setw(indentSize * 2) << setfill(' ') << "" << "Size: "  << FspRegions[idx].SegmentArray[count].Size << "h\n";
        }
    }

    width = 20;
    ss << "Fbm KEY_AND_SIGNATURE_STRUCT:\n"
       << setw(indentSize) << setfill(' ') << "" << setw(width) << "Version:"     << (UINT32)KeyAndSignature.Header.Version << "h\n"
       << setw(indentSize) << setfill(' ') << "" << setw(width) << "KeyAlg:"      << KeyAndSignature.Header.KeyAlg << "h (" << GetRsaAlgFromID(KeyAndSignature.Header.KeyAlg) << ")\n"
       << setw(indentSize) << setfill(' ') << "" << setw(width) << "KeyVersion:"  << (UINT32)KeyAndSignature.RsaKey.Version << "h\n"
       << setw(indentSize) << setfill(' ') << "" << setw(width) << "KeySizeBits:" << KeyAndSignature.RsaKey.KeySizeBits << "h\n"
       << setw(indentSize) << setfill(' ') << "" << setw(width) << "Exponent:"    << KeyAndSignature.RsaKey.Exponent << "h\n"
       << setw(indentSize) << setfill(' ') << "" << "Modulus:\n"
       << DumpHex((UINT8*)KeyAndSignature.KEY_Modulus.data(), KeyAndSignature.KEY_Modulus.size(), 16, false, indentSize) << "\n"
       << setw(indentSize) << setfill(' ') << "" << setw(width) << "SigScheme:"       << KeyAndSignature.SigScheme<< "h\n"
       << setw(indentSize) << setfill(' ') << "" << setw(width) << "Sig_Version:"     << (UINT32)KeyAndSignature.SignatureRsa.Version<< "h\n"
       << setw(indentSize) << setfill(' ') << "" << setw(width) << "Sig_KeySizeBits:" << KeyAndSignature.SignatureRsa.KeySizeBits<< "h\n"
       << setw(indentSize) << setfill(' ') << "" << setw(width) << "Sig_HashAlg:"     << KeyAndSignature.SignatureRsa.HashAlg << "h (" << GetHashAlgFromID(KeyAndSignature.SignatureRsa.HashAlg) << ")\n"
       << setw(indentSize) << setfill(' ') << "" << "Signature:\n"
       << DumpHex((UINT8*)KeyAndSignature.Signature.data(), KeyAndSignature.Signature.size(), 16, false, indentSize) << "\n";

    InfoStr = QString::fromStdString(ss.str());
}

std::string FspBootManifestClass::GetFspComponentFromID(UINT8 ComponentID) {
    switch (ComponentID) {
    case 0:
        return "FSP-O/T";
    case 1:
        return "FSP-M";
    case 2:
        return "FSP-S";
    default:
        return "Unknown";
    }
}

std::string FspBootManifestClass::GetRsaAlgFromID(UINT8 RsaAlgID) {
    switch (RsaAlgID) {
    case TPM_ALG_RSA:
        return "RSA";
    case TPM_ALG_DES:
        return "DES";
    case TPM_ALG_3DES:
        return "3DES";
    case TPM_ALG_SHA:
        return "SHA1";
    case TPM_ALG_HMAC:
        return "RFC 2104 HMAC";
    case TPM_ALG_AES128:
        return "AES128";
    case TPM_ALG_MGF1:
        return "XOR - MGF1";
    case TPM_ALG_AES192:
        return "AES192";
    case TPM_ALG_AES256:
        return "AES256";
    case TPM_ALG_XOR:
        return "XOR";
    default:
        return "Unknown";
    }
}

std::string FspBootManifestClass::GetHashAlgFromID(UINT8 HashAlgID) {
    switch (HashAlgID) {
    case TPM_ALG_SHA1:
        return "SHA1";
    case TPM_ALG_AES:
        return "AES";
    case TPM_ALG_SHA256:
        return "SHA256";
    case TPM_ALG_SHA384:
        return "SHA384";
    case TPM_ALG_SHA512:
        return "SHA512";
    case TPM_ALG_SM3_256:
        return "SM3";
    case TPM_ALG_SM4:
        return "SM4";
    case TPM_ALG_RSASSA:
        return "RSASSA";
    default:
        return "Unknown";
        break;
    }
}
