#include <iomanip>
#include "UefiLib.h"
#include "UEFI/GuidDefinition.h"

typedef UINT32 TPM_ALGORITHM_ID;
typedef UINT16 TPM_ALG_ID;
#define TPM_ALG_RSA     ((TPM_ALGORITHM_ID) 0x00000001)             ///< The RSA algorithm.
#define TPM_ALG_DES     ((TPM_ALGORITHM_ID) 0x00000002)             ///< The DES algorithm
#define TPM_ALG_3DES    ((TPM_ALGORITHM_ID) 0x00000003)             ///< The 3DES algorithm in EDE mode
#define TPM_ALG_SHA     ((TPM_ALGORITHM_ID) 0x00000004)             ///< The SHA1 algorithm
#define TPM_ALG_HMAC    ((TPM_ALGORITHM_ID) 0x00000005)             ///< The RFC 2104 HMAC algorithm
#define TPM_ALG_AES128  ((TPM_ALGORITHM_ID) 0x00000006)             ///< The AES algorithm, key size 128
#define TPM_ALG_MGF1    ((TPM_ALGORITHM_ID) 0x00000007)             ///< The XOR algorithm using MGF1 to create a string the size of the encrypted block
#define TPM_ALG_AES192  ((TPM_ALGORITHM_ID) 0x00000008)             ///< AES, key size 192
#define TPM_ALG_AES256  ((TPM_ALGORITHM_ID) 0x00000009)             ///< AES, key size 256
#define TPM_ALG_XOR     ((TPM_ALGORITHM_ID) 0x0000000A)             ///< XOR using the rolling nonces

#define TPM_ALG_ERROR  (TPM_ALG_ID)(0x0000)
#define TPM_ALG_FIRST  (TPM_ALG_ID)(0x0001)
// #define TPM_ALG_RSA            (TPM_ALG_ID)(0x0001)
// #define TPM_ALG_SHA            (TPM_ALG_ID)(0x0004)
#define TPM_ALG_SHA1  (TPM_ALG_ID)(0x0004)
// #define TPM_ALG_HMAC           (TPM_ALG_ID)(0x0005)
#define TPM_ALG_AES  (TPM_ALG_ID)(0x0006)
// #define TPM_ALG_MGF1           (TPM_ALG_ID)(0x0007)
#define TPM_ALG_KEYEDHASH  (TPM_ALG_ID)(0x0008)
// #define TPM_ALG_XOR            (TPM_ALG_ID)(0x000A)
#define TPM_ALG_SHA256          (TPM_ALG_ID)(0x000B)
#define TPM_ALG_SHA384          (TPM_ALG_ID)(0x000C)
#define TPM_ALG_SHA512          (TPM_ALG_ID)(0x000D)
#define TPM_ALG_NULL            (TPM_ALG_ID)(0x0010)
#define TPM_ALG_SM3_256         (TPM_ALG_ID)(0x0012)
#define TPM_ALG_SM4             (TPM_ALG_ID)(0x0013)
#define TPM_ALG_RSASSA          (TPM_ALG_ID)(0x0014)
#define TPM_ALG_RSAES           (TPM_ALG_ID)(0x0015)
#define TPM_ALG_RSAPSS          (TPM_ALG_ID)(0x0016)
#define TPM_ALG_OAEP            (TPM_ALG_ID)(0x0017)
#define TPM_ALG_ECDSA           (TPM_ALG_ID)(0x0018)
#define TPM_ALG_ECDH            (TPM_ALG_ID)(0x0019)
#define TPM_ALG_ECDAA           (TPM_ALG_ID)(0x001A)
#define TPM_ALG_SM2             (TPM_ALG_ID)(0x001B)
#define TPM_ALG_ECSCHNORR       (TPM_ALG_ID)(0x001C)
#define TPM_ALG_ECMQV           (TPM_ALG_ID)(0x001D)
#define TPM_ALG_KDF1_SP800_56a  (TPM_ALG_ID)(0x0020)
#define TPM_ALG_KDF2            (TPM_ALG_ID)(0x0021)
#define TPM_ALG_KDF1_SP800_108  (TPM_ALG_ID)(0x0022)
#define TPM_ALG_ECC             (TPM_ALG_ID)(0x0023)
#define TPM_ALG_SYMCIPHER       (TPM_ALG_ID)(0x0025)
#define TPM_ALG_CTR             (TPM_ALG_ID)(0x0040)
#define TPM_ALG_OFB             (TPM_ALG_ID)(0x0041)
#define TPM_ALG_CBC             (TPM_ALG_ID)(0x0042)
#define TPM_ALG_CFB             (TPM_ALG_ID)(0x0043)
#define TPM_ALG_ECB             (TPM_ALG_ID)(0x0044)
#define TPM_ALG_LAST            (TPM_ALG_ID)(0x0044)

namespace UefiSpace {

    BootGuardClass::BootGuardClass(UINT8* fv, INT64 length, INT64 address):data(fv), size(length), offset(address) {}

    BootGuardClass::~BootGuardClass() {}

    void BootGuardClass::setInfoStr() {}

    void BootGuardClass::InternalDumpData(stringstream &ss, UINT8* Data, INT64 Size) {
        for (INT64 Index = 0; Index < Size; Index++) {
            ss << setw(2) << setfill('0') << hex << (UINT16)Data[Index] << " ";
        }
    }

    string BootGuardClass::DumpHex(UINT8* HexData, INT64 length, bool SingleLine) {
        #define COLUME_SIZE  16
        stringstream ss;

        if (SingleLine) {
            for (int i = 0; i < length; ++i) {
                ss << setw(2) << setfill('0') << hex << (UINT16)HexData[i];
            }
            return ss.str();
        }

        INT64 Index;
        INT64 Count = length / COLUME_SIZE;
        INT64 Left  = length % COLUME_SIZE;
        for (Index = 0; Index < Count; Index++) {
            ss << setw(3) << setfill('0') << hex << Index * COLUME_SIZE << ": ";
            InternalDumpData(ss, HexData + Index * COLUME_SIZE, COLUME_SIZE);
            ss << "\n";
        }
        if (Left != 0) {
            ss << setw(3) << setfill('0') << hex << Index * COLUME_SIZE << ": ";
            InternalDumpData (ss, HexData + Index * COLUME_SIZE, Left);
            ss << "\n";
        }
        return ss.str();
    }

    string BootGuardClass::getAlgName(UINT16 Alg) {
        string AlgName;
        switch (Alg) {
        case TPM_ALG_RSA:
            AlgName = "RSA";
            break;
        case TPM_ALG_DES:
            AlgName = "DES";
            break;
        case TPM_ALG_3DES:
            AlgName = "3DES";
            break;
        case TPM_ALG_SHA:
            AlgName = "SHA";
            break;
        case TPM_ALG_HMAC:
            AlgName = "HMAC";
            break;
        case TPM_ALG_AES128:
            AlgName = "AES128";
            break;
        case TPM_ALG_MGF1:
            AlgName = "MGF1";
            break;
        case TPM_ALG_AES192:
            AlgName = "AES192";
            break;
        case TPM_ALG_AES256:
            AlgName = "AES256";
            break;
        case TPM_ALG_SHA256:
            AlgName = "SHA256";
            break;
        case TPM_ALG_SHA384:
            AlgName = "SHA384";
            break;
        case TPM_ALG_SHA512:
            AlgName = "SHA512";
            break;
        case TPM_ALG_SM3_256:
            AlgName = "SM3_256";
            break;
        case TPM_ALG_SM4:
            AlgName = "SM4";
            break;
        case TPM_ALG_RSASSA:
            AlgName = "RSASSA";
            break;
        case TPM_ALG_RSAES:
            AlgName = "RSAES";
            break;
        case TPM_ALG_RSAPSS:
            AlgName = "RSAPSS";
            break;
        default:
            break;
        }
        return AlgName;
    }

    AcmHeaderClass::AcmHeaderClass(UINT8* fv, INT64 address):BootGuardClass(fv, 0, address) {
        acmHeader = *(ACM_HEADER*)fv;
        ValidFlag = (acmHeader.ModuleType == ACM_MODULE_TYPE_CHIPSET_ACM) ? true : false;
        ProdFlag = (acmHeader.Flags & 0x8000) ? false : true;
        if (acmHeader.HeaderVersion < ACM_HEADER_VERSION_3) {
            ExtAcmHeader = *(Ext_ACM_Header*)(fv + sizeof(ACM_HEADER));
        } else {
            isAcm3 = true;
            ExtAcmHeader3 = *(Ext_ACM_Header3*)(fv + sizeof(ACM_HEADER));
        }
        UINT8 *AcmPtr = (UINT8 *)fv;
        AcmPtr += acmHeader.HeaderLen * 4;
        AcmPtr += acmHeader.ScratchSize * 4;
        AcmInfoTable = (ACM_INFO_TABLE *)AcmPtr;
        if (AcmInfoTable->AitVersion < ACM_INFO_TABLE_VERSION_9) {
            if (AcmInfoTable->Guid == guidData->gTxtAcmInfoTableGuid) {
                AcmVersion.AcmMajorVersion = AcmInfoTable->AitRevision[0];
                AcmVersion.AcmMinorVersion = AcmInfoTable->AitRevision[1];
                AcmVersion.AcmRevision = AcmInfoTable->AitRevision[2];
            }
        } else if (AcmInfoTable->AitVersion == ACM_INFO_TABLE_VERSION_9) {
            AcmPtr += sizeof(ACM_INFO_TABLE);
            UINT8 *StartPtr = AcmPtr;
            bool TableFound = false;
            while (memcmp(((VAR_LIST *)AcmPtr)->Id, NULL_TERMINATOR_ID, 4) && ((AcmPtr - StartPtr) < 300)) {
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
    }

    AcmHeaderClass::~AcmHeaderClass() {}

    bool AcmHeaderClass::isValid() const {
        return ValidFlag;
    }

    bool AcmHeaderClass::isProd() const {
        return ProdFlag;
    }

    void AcmHeaderClass::setInfoStr() {
        INT64 width = 20;
        stringstream ss;
        ss.setf(ios::left);
        ss << setw(width) << "Guid:"    << GUID(AcmInfoTable->Guid).str(true) << "\n"
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

        if (isAcm3) {
            ss << "ACM RSA Public Key:\n" << DumpHex(ExtAcmHeader3.Rsa3072PubKey, 384) << "\n"
               << "ACM RSA Signature:\n" << DumpHex(ExtAcmHeader3.Rsa3072Sig, 384) << "\n"
               << "Scratch:\n" << DumpHex(ExtAcmHeader3.Scratch, 832);
        } else {
            ss << setw(width) << "ACM RSA Public Key Exponent:" << hex << uppercase << ExtAcmHeader.RsaPubExp << "h\n"
               << "ACM RSA Public Key:\n" << DumpHex(ExtAcmHeader.Rsa2048PubKey, 256) << "\n"
               << "ACM RSA Signature:\n" << DumpHex(ExtAcmHeader.Rsa2048Sig, 256) << "\n"
               << "Scratch:\n" << DumpHex(ExtAcmHeader.Scratch, 572);
        }

        InfoStr = QString::fromStdString(ss.str());
    }

    KeyManifestClass::KeyManifestClass(UINT8* fv, INT64 length):BootGuardClass(fv, length, 0) {
        KM_Header = *(KEY_MANIFEST_STRUCTURE*)data;
        if (KM_Header.KeySignatureOffset > length) {
            isValid = false;
            return;
        }

        INT64 KeyHashOffset = sizeof(KEY_MANIFEST_STRUCTURE);
        for (int i = 0; i < KM_Header.KeyCount; ++i) {
            if (KeyHashOffset < KM_Header.KeySignatureOffset) {
                SHAX_KMHASH_STRUCT KmHash = *(SHAX_KMHASH_STRUCT*)(fv + KeyHashOffset);
                KmHash.Digest.HashBuffer = fv + KeyHashOffset + sizeof(SHAX_KMHASH_STRUCT) - sizeof(UINT8*);
                KmHashList.push_back(KmHash);
                KeyHashOffset += sizeof(SHAX_KMHASH_STRUCT) - sizeof(UINT8*) + KmHash.Digest.Size;
            } else {
                break;
            }
        }

        KeySig = (KEY_AND_SIGNATURE_STRUCT*)(data + KM_Header.KeySignatureOffset);
        if (KeySig->KeyAlg == TPM_ALG_RSA) {
            SigSchemeOffset = sizeof(UINT8) + sizeof(UINT16) + sizeof(RSA_PUBLIC_KEY_STRUCT) - 1 + KeySig->Key.RsaKey.KeySizeBits / 8;
            SigScheme = *(UINT16*)(data + KM_Header.KeySignatureOffset + SigSchemeOffset);
            SigOffset = SigSchemeOffset + sizeof(UINT16);
            sig.SignatureRsa = *(RSASSA_SIGNATURE_STRUCT*)(data + KM_Header.KeySignatureOffset + SigOffset);
        } else if (KeySig->KeyAlg == TPM_ALG_ECC) {
            SigSchemeOffset = sizeof(UINT8) + sizeof(UINT16) + sizeof(ECC_PUBLIC_KEY_STRUCT);
            SigScheme = *(UINT16*)(data + KM_Header.KeySignatureOffset + SigSchemeOffset);
            SigOffset = SigSchemeOffset + sizeof(UINT16);
            sig.SignatureEcc = *(ECC_SIGNATURE_STRUCT*)(data + KM_Header.KeySignatureOffset + SigOffset);
        }
    }

    KeyManifestClass::~KeyManifestClass() {}

    void KeyManifestClass::setInfoStr() {
        if (!isValid)
            return;
        INT64 width = 22;
        stringstream ss;
        ss.setf(ios::left);
        ss << setw(width) << "Structure ID:"    << hex << uppercase << Buffer::charToString((INT8*)(KM_Header.StructureId), 8, false) << "\n"
           << setw(width) << "StructVersion:"   << hex << uppercase << (UINT32)KM_Header.StructVersion << "h\n"
           << setw(width) << "KeySigOffset:"    << hex << uppercase << KM_Header.KeySignatureOffset << "h\n"
           << setw(width) << "KeyManifestVer:"  << hex << uppercase << (UINT32)KM_Header.KeyManifestRevision << "h\n"
           << setw(width) << "KmSVN:"           << hex << uppercase << (UINT32)KM_Header.KmSvn << "h\n"
           << setw(width) << "KeyManifestID:"   << hex << uppercase << (UINT32)KM_Header.KeyManifestId << "h\n"
           << setw(width) << "KmPubKey Alg:"    << hex << uppercase << KM_Header.KmPubKeyHashAlg << "h (" << getAlgName(KM_Header.KmPubKeyHashAlg) << ")\n"
           << setw(width) << "Number of Manifest Key Digests:  " << hex << uppercase << KM_Header.KeyCount << "\n";
        if (KM_Header.KeyCount > 0) {
            ss << "KeyHashes:\n";
        }
        for (int i = 0; i < KM_Header.KeyCount; ++i) {
            ss << "[" << i << "] " << setw(width - 4) << "Usage:"  << hex << uppercase << KmHashList.at(i).Usage << "h (Boot Policy Manifest)\n"
               << "    "           << setw(width - 4) << "HashAlg:" << hex << uppercase << KmHashList.at(i).Digest.HashAlg << "h (" << getAlgName(KmHashList.at(i).Digest.HashAlg) << ")\n"
               << "    "           << setw(width - 4) << "Size:"    << hex << uppercase << KmHashList.at(i).Digest.Size << "h\n"
               << "    "           << setw(width - 4) << "HashBuffer:" << DumpHex(KmHashList.at(i).Digest.HashBuffer, KmHashList.at(i).Digest.Size, true) << "\n";
        }
        ss << "Signature Structure:\n"
           << setw(width) << "Version:"         << hex << uppercase << (UINT32)KeySig->Version << "h\n"
           << setw(width) << "KeyAlg:"          << hex << uppercase << KeySig->KeyAlg << "h (" << getAlgName(KeySig->KeyAlg) << ")\n";
        if (KeySig->KeyAlg == TPM_ALG_RSA) {
            ss << "RsaPublicKeyStructure:\n"
               << setw(width) << "Version:"     << hex << uppercase << (UINT32)KeySig->Key.RsaKey.Version << "h\n"
               << setw(width) << "KeySize:"     << hex << uppercase << KeySig->Key.RsaKey.KeySizeBits << "h\n"
               << setw(width) << "Exponent:"    << hex << uppercase << KeySig->Key.RsaKey.Exponent << "h\n"
               << "Modulus:\n" << DumpHex(KeySig->Key.RsaKey.Modulus, KeySig->Key.RsaKey.KeySizeBits / 8) << "\n"
               << setw(width) << "SigScheme:"   << hex << uppercase << SigScheme << "h\n";
            if (SigScheme == TPM_ALG_RSASSA || SigScheme == TPM_ALG_RSAPSS) {
                ss << "RsaSsaSigStructure:\n"
                   << setw(width) << "Version:"     << hex << uppercase << (UINT32)sig.SignatureRsa.Version << "h\n"
                   << setw(width) << "KeySize:"     << hex << uppercase << sig.SignatureRsa.KeySizeBits << "h\n"
                   << setw(width) << "HashAlg:"     << hex << uppercase << sig.SignatureRsa.HashAlg << "h (" << getAlgName(sig.SignatureRsa.HashAlg) << ")\n"
                   << "Signature:\n" << DumpHex(data + KM_Header.KeySignatureOffset + SigOffset + sizeof(RSASSA_SIGNATURE_STRUCT) - 1, sig.SignatureRsa.KeySizeBits / 8) << "\n";
            }
        } else if (KeySig->KeyAlg == TPM_ALG_ECC) {
            ss << "RsaPublicKeyStructure:\n"
               << setw(width) << "Version:"     << hex << uppercase << (UINT32)KeySig->Key.EccKey.Version << "h\n"
               << setw(width) << "KeySize:"     << hex << uppercase << KeySig->Key.EccKey.KeySizeBits << "h\n"
               << "X component:\n" << DumpHex(KeySig->Key.EccKey.Qx, ECC_PUBLIC_KEY_STRUCT_KEY_LEN_DEFAULT) << "\n"
               << "Y component:\n" << DumpHex(KeySig->Key.EccKey.Qy, ECC_PUBLIC_KEY_STRUCT_KEY_LEN_DEFAULT) << "\n"
               << setw(width) << "SigScheme:"   << hex << uppercase << SigScheme << "h\n"
               << "Ecc SigStructure:\n"
               << setw(width) << "Version:"     << hex << uppercase << (UINT32)sig.SignatureEcc.Version << "h\n"
               << setw(width) << "KeySize:"     << hex << uppercase << sig.SignatureEcc.KeySizeBits << "h\n"
               << setw(width) << "HashAlg:"     << hex << uppercase << sig.SignatureEcc.HashAlg << "h (" << getAlgName(sig.SignatureRsa.HashAlg) << ")\n";
        }

        InfoStr = QString::fromStdString(ss.str());
    }

    BootPolicyManifestClass::BootPolicyManifestClass(UINT8* fv, INT64 length):BootGuardClass(fv, length, 0) {
        BPM_Header = *(BOOT_POLICY_MANIFEST_HEADER*)fv;
        if (BPM_Header.KeySignatureOffset > length) {
            isValid = false;
            return;
        }

        INT64 IbbElementOffset = sizeof(BOOT_POLICY_MANIFEST_HEADER);
        INT64 ElementsOffset = IbbElementOffset;
        while (ElementsOffset < length) {
            string StructID = Buffer::charToString((INT8*)(fv + ElementsOffset), 8, false);
            if (StructID == "__IBBS__") {
                IBBS_Class *IBBS = new IBBS_Class(fv + ElementsOffset, length - ElementsOffset);
                IBBS->setInfoStr();
                BpmElementList.push_back(IBBS);
                ElementsOffset += IBBS->getBpmElementSize();
            }
            else if (StructID == "__TXTS__") {
                TXTS_Class *TXTS = new TXTS_Class(fv + ElementsOffset, length - ElementsOffset);
                TXTS->setInfoStr();
                BpmElementList.push_back(TXTS);
                ElementsOffset += TXTS->getBpmElementSize();
            }
            else if (StructID == "__PCDS__") {
                PCDS_Class *PCDS = new PCDS_Class(fv + ElementsOffset, length - ElementsOffset);
                PCDS->setInfoStr();
                BpmElementList.push_back(PCDS);
                ElementsOffset += PCDS->getBpmElementSize();
            }
            else if (StructID == "__PMSG__") {
                PMSG_Class *PSMG = new PMSG_Class(fv + ElementsOffset, length - ElementsOffset);
                PSMG->setInfoStr();
                BpmElementList.push_back(PSMG);
                ElementsOffset += PSMG->getBpmElementSize();
            }
            else {
                break;
            }
        }

        KeySig = (KEY_AND_SIGNATURE_STRUCT*)(data + BPM_Header.KeySignatureOffset);
        if (KeySig->KeyAlg == TPM_ALG_RSA) {
            SigSchemeOffset = sizeof(UINT8) + sizeof(UINT16) + sizeof(RSA_PUBLIC_KEY_STRUCT) - 1 + KeySig->Key.RsaKey.KeySizeBits / 8;
            SigScheme = *(UINT16*)(data + BPM_Header.KeySignatureOffset + SigSchemeOffset);
            SigOffset = SigSchemeOffset + sizeof(UINT16);
            sig.SignatureRsa = *(RSASSA_SIGNATURE_STRUCT*)(data + BPM_Header.KeySignatureOffset + SigOffset);
        }
    }

    BootPolicyManifestClass::~BootPolicyManifestClass() {
        for (auto Element:BpmElementList) {
            delete Element;
        }
    }

    void BootPolicyManifestClass::setInfoStr() {
        if (!isValid)
            return;
        INT64 width = 22;
        stringstream ss;
        ss.setf(ios::left);
        ss << setw(width) << "Structure ID:"  << hex << uppercase << Buffer::charToString((INT8*)(BPM_Header.StructureId), 8, false) << "\n"
           << setw(width) << "StructVersion:" << hex << uppercase << (UINT32)BPM_Header.StructVersion << "h\n"
           << setw(width) << "HdrVersion:"    << hex << uppercase << (UINT32)BPM_Header.HdrStructVersion << "h\n"
           << setw(width) << "HdrSize:"       << hex << uppercase << (UINT32)BPM_Header.HdrSize << "h\n"
           << setw(width) << "KeySigOffset:"  << hex << uppercase << BPM_Header.KeySignatureOffset << "h\n"
           << setw(width) << "BpmRevision:"   << hex << uppercase << (UINT32)BPM_Header.BpmRevision << "h\n"
           << setw(width) << "BpmRevocation:" << hex << uppercase << (UINT32)BPM_Header.BpmRevocation << "h\n"
           << setw(width) << "AcmRevocation:" << hex << uppercase << (UINT32)BPM_Header.AcmRevocation << "h\n"
           << setw(width) << "NEMPages:"      << hex << uppercase << BPM_Header.NemPages << "h\n\n";

        for (auto Element:BpmElementList) {
            ss << Element->InfoStr;
        }

        ss << "Signature Structure:\n"
           << setw(width) << "Version:"         << hex << uppercase << (UINT32)KeySig->Version << "h\n"
           << setw(width) << "KeyAlg:"          << hex << uppercase << KeySig->KeyAlg << "h (" << getAlgName(KeySig->KeyAlg) << ")\n";
        if (KeySig->KeyAlg == TPM_ALG_RSA) {
            ss << "RsaPublicKeyStructure:\n"
               << setw(width) << "Version:"     << hex << uppercase << (UINT32)KeySig->Key.RsaKey.Version << "h\n"
               << setw(width) << "KeySize:"     << hex << uppercase << KeySig->Key.RsaKey.KeySizeBits << "h\n"
               << setw(width) << "Exponent:"    << hex << uppercase << KeySig->Key.RsaKey.Exponent << "h\n"
               << "Modulus:\n" << DumpHex(KeySig->Key.RsaKey.Modulus, KeySig->Key.RsaKey.KeySizeBits / 8) << "\n"
               << setw(width) << "SigScheme:"   << hex << uppercase << SigScheme << "h\n";
            if (SigScheme == TPM_ALG_RSASSA || SigScheme == TPM_ALG_RSAPSS) {
                ss << "RsaSsaSigStructure:\n"
                   << setw(width) << "Version:"     << hex << uppercase << (UINT32)sig.SignatureRsa.Version << "h\n"
                   << setw(width) << "KeySize:"     << hex << uppercase << sig.SignatureRsa.KeySizeBits << "h\n"
                   << setw(width) << "HashAlg:"     << hex << uppercase << sig.SignatureRsa.HashAlg << "h (" << getAlgName(sig.SignatureRsa.HashAlg) << ")\n"
                   << "Signature:\n" << DumpHex(data + BPM_Header.KeySignatureOffset + SigOffset + sizeof(RSASSA_SIGNATURE_STRUCT) - 1, sig.SignatureRsa.KeySizeBits / 8) << "\n";
            }
        }

        InfoStr = QString::fromStdString(ss.str());
    }

    BpmElement::BpmElement(UINT8* fv, INT64 length):BootGuardClass(fv, length, 0) {}

    BpmElement::~BpmElement() {}

    void BpmElement::setInfoStr() {}

    INT64 BpmElement::getBpmElementSize() const {
        return BpmElementSize;
    }

    IBBS_Class::IBBS_Class(UINT8* fv, INT64 length):BpmElement(fv, length) {
        IbbElement = *(IBB_ELEMENT*)fv;
        if (Buffer::charToString((INT8*)IbbElement.StructureId, 8, false) == "__IBBS__") {
            IbbElementValid = true;
            INT64 IbbEntryPointOffset = sizeof(IBB_ELEMENT) + IbbElement.PostIbbHash.Size - 1;
            IbbEntryPoint = *(UINT32*)(fv + IbbEntryPointOffset);
            HashList = *(HASH_LIST*)(fv + IbbEntryPointOffset + sizeof(UINT32));
            INT64 HashStructOffset = IbbEntryPointOffset + sizeof(UINT32) + sizeof(HASH_LIST);
            for (int i = 0; i < HashList.Count; ++i) {
                HASH_STRUCTURE HashStruct = *(HASH_STRUCTURE*)(fv + HashStructOffset);
                HashStruct.HashBuffer = fv + HashStructOffset + sizeof(UINT16) + sizeof(UINT16);
                IbbHashList.push_back(HashStruct);
                HashStructOffset += sizeof(UINT16) + sizeof(UINT16) + HashStruct.Size;
            }

            INT64 ObbDigestOffset = IbbEntryPointOffset + sizeof(UINT32) + HashList.Size;
            HASH_STRUCTURE ObbHashStruct = *(HASH_STRUCTURE*)(fv + ObbDigestOffset);
            ObbHashStruct.HashBuffer = fv + ObbDigestOffset + sizeof(UINT16) + sizeof(UINT16);
            ObbHashList.push_back(ObbHashStruct);

            INT64 IbbSegmentOffset = ObbDigestOffset + sizeof(UINT16) + sizeof(UINT16) + ObbHashStruct.Size;
            SegmentCount = *(fv + IbbSegmentOffset + sizeof(Reserved));
            IbbSegmentOffset += sizeof(UINT32);
            for (int i = 0; i < SegmentCount; ++i) {
                IBB_SEGMENT segment = *(IBB_SEGMENT*)(fv + IbbSegmentOffset);
                IbbSegment.push_back(segment);
                IbbSegmentOffset += sizeof(IBB_SEGMENT);
            }
            BpmElementSize = IbbSegmentOffset;
        }
    }

    IBBS_Class::~IBBS_Class() {}

    void IBBS_Class::setInfoStr() {
        INT64 width = 22;
        stringstream ss;
        ss.setf(ios::left);
        if (IbbElementValid) {
            ss << "IbbElement:\n"
               << setw(width) << "Structure ID:"  << hex << uppercase << Buffer::charToString((INT8*)(IbbElement.StructureId), 8, false) << "\n"
               << setw(width) << "StructVersion:" << hex << uppercase << (UINT32)IbbElement.StructVersion << "h\n"
               << setw(width) << "ElementSize:"   << hex << uppercase << IbbElement.ElementSize << "h\n"
               << setw(width) << "SetType:"       << hex << uppercase << (UINT32)IbbElement.SetType << "h\n"
               << setw(width) << "PbetValue:"     << hex << uppercase << (UINT32)IbbElement.PbetValue << "h\n"
               << setw(width) << "Flags:"         << hex << uppercase << IbbElement.Flags << "h\n"
               << setw(width) << "IbbMchBar:"     << hex << uppercase << IbbElement.IbbMchBar << "h\n"
               << setw(width) << "VtdBar:"        << hex << uppercase << IbbElement.VtdBar << "h\n"
               << setw(width) << "DmaProtBase0:"  << hex << uppercase << IbbElement.DmaProtBase0 << "h\n"
               << setw(width) << "DmaProtLimit0:" << hex << uppercase << IbbElement.DmaProtLimit0 << "h\n"
               << setw(width) << "DmaProtBase1:"  << hex << uppercase << IbbElement.DmaProtBase1 << "h\n"
               << setw(width) << "DmaProtLimit1:" << hex << uppercase << IbbElement.DmaProtLimit1 << "h\n"
               << "PostIbbHash:\n"
               << setw(width) << "HashAlg:"       << hex << uppercase << IbbElement.PostIbbHash.HashAlg << "h (" << getAlgName(IbbElement.PostIbbHash.HashAlg) << ")\n"
               << setw(width) << "Size:"          << hex << uppercase << IbbElement.PostIbbHash.Size << "h\n"
               << "HashBuffer:\n" << DumpHex(data + sizeof(IBB_ELEMENT) - 1, IbbElement.PostIbbHash.Size) << "\n"
               << setw(width) << "IbbEntry:"      << hex << uppercase << IbbEntryPoint << "h\n"
               << setw(width) << "HashList:"      << "(Number of Digests: " << HashList.Count << ", Total Size: " << hex << uppercase << HashList.Size << "h)\n";
            for (int i = 0; i < HashList.Count; ++i) {
                ss << "[" << i << "] " << setw(width - 4) << "HashAlg:" << hex << uppercase << IbbHashList.at(i).HashAlg << "h (" << getAlgName(IbbHashList.at(i).HashAlg) << ")\n"
                   << "    "           << setw(width - 4) << "Size:"    << hex << uppercase << IbbHashList.at(i).Size << "h\n"
                   << "    "           << setw(width - 4) << "HashBuffer:" << DumpHex(IbbHashList.at(i).HashBuffer, IbbHashList.at(i).Size, true) << "\n";
            }
            ss << "OBB Digest:\n"
               << setw(width) << "HashAlg:" << hex << uppercase << ObbHashList.at(0).HashAlg << "h (" << getAlgName(ObbHashList.at(0).HashAlg) << ")\n"
               << setw(width) << "Size:"    << hex << uppercase << ObbHashList.at(0).Size << "h\n"
               << setw(width) << "HashBuffer:" << DumpHex(ObbHashList.at(0).HashBuffer, ObbHashList.at(0).Size, true) << "\n\n";
            ss << setw(width) << "Segment Count:" << dec << (UINT32)SegmentCount << "\n";
            if (SegmentCount == 0) {
                ss << "--No Segments\n";
            } else {
                ss << setw(9) << "Seg#" << setw(12) << "Base" << setw(9) << "Size" << setw(10) << "Flags" << setw(11) << "Measured" << setw(10) << "Cache Type\n";
            }
            for (int i = 0; i < SegmentCount; ++i) {
                ss.setf(ios::right);
                ss << "[" << i << "]   0x" << hex << uppercase << IbbSegment.at(i).Base
                   << "  0x" << setw(8) << setfill('0') << IbbSegment.at(i).Size
                   << "  0x" << setw(4) << setfill('0') << IbbSegment.at(i).Flags
                   << "      Yes     Write Protect\n";
            }
        }
        InfoStr = ss.str() + "\n";
    }

    TXTS_Class::TXTS_Class(UINT8* fv, INT64 length):BpmElement(fv, length) {
        TxtElement = *(TXT_ELEMENT*)fv;
        if (Buffer::charToString((INT8*)TxtElement.StructureId, 8, false) == "__TXTS__") {
            TxtElementValid = true;

            INT64 TxtSegmentOffset = sizeof(TXT_ELEMENT) - sizeof(IBB_SEGMENT*);
            for (int i = 0; i < TxtElement.SegmentCount; ++i) {
                IBB_SEGMENT segment = *(IBB_SEGMENT*)(fv + TxtSegmentOffset);
                TxtSegment.push_back(segment);
                TxtSegmentOffset += sizeof(IBB_SEGMENT);
            }

            BpmElementSize = TxtSegmentOffset + sizeof(IBB_SEGMENT) * TxtElement.SegmentCount;
        }
    }

    TXTS_Class::~TXTS_Class() {}

    void TXTS_Class::setInfoStr() {
        INT64 width = 22;
        stringstream ss;
        ss.setf(ios::left);
        if (TxtElementValid) {
            ss << "TxtElement:\n"
               << setw(width) << "  Structure ID:"    << hex << uppercase << Buffer::charToString((INT8*)(TxtElement.StructureId), 8, false) << "\n"
               << setw(width) << "  StructVersion:"   << hex << uppercase << (UINT32)TxtElement.StructVersion << "h\n"
               << setw(width) << "  ElementSize:"     << hex << uppercase << TxtElement.ElementSize << "h\n"
               << setw(width) << "  SetType:"         << hex << uppercase << (UINT32)TxtElement.SetType << "h\n"
               << setw(width) << "  Flags:"           << hex << uppercase << TxtElement.Flags << "h\n"
               << setw(width) << "  PwrDownInterval:" << hex << uppercase << TxtElement.PwrDownInterval << "h\n"
               << setw(width) << "  PttCmosOffset0:"  << hex << uppercase << (UINT32)TxtElement.PttCmosOffset0 << "h\n"
               << setw(width) << "  PttCmosOffset1:"  << hex << uppercase << (UINT32)TxtElement.PttCmosOffset1 << "h\n"
               << setw(width) << "  AcpiBaseOffset:"  << hex << uppercase << TxtElement.AcpiBaseOffset << "h\n"
               << setw(width) << "  PrwmBaseOffset:"  << hex << uppercase << TxtElement.PrwmBaseOffset << "h\n"
               << "  Digest List:\n"
               << setw(width) << "  HashList"         << "(Number of Digests: " << TxtElement.DigestList.Count << ", Total Size: " << hex << uppercase << TxtElement.DigestList.Size << "h)\n"
               << setw(width) << "  SegmentCount:"    << dec << uppercase << (UINT32)TxtElement.SegmentCount << "\n";
            if (TxtElement.SegmentCount == 0) {
                ss << "  --No Segments\n";
            } else {
                ss << setw(9) << "Seg#" << setw(12) << "Base" << setw(9) << "Size" << setw(10) << "Flags" << setw(11) << "Measured" << setw(10) << "Cache Type\n";
            }
            for (int i = 0; i < TxtElement.SegmentCount; ++i) {
                ss.setf(ios::right);
                ss << "[" << i << "]   0x" << hex << uppercase << TxtSegment.at(i).Base
                   << "  0x" << setw(8) << setfill('0') << TxtSegment.at(i).Size
                   << "  0x" << setw(4) << setfill('0') << TxtSegment.at(i).Flags
                   << "      Yes     Write Protect\n";
            }
        }
        InfoStr = ss.str() + "\n";
    }

    PCDS_Class::PCDS_Class(UINT8* fv, INT64 length):BpmElement(fv, length) {
        PcdElement = *(PLATFORM_CONFIG_DATA_ELEMENT*)fv;
        if (Buffer::charToString((INT8*)PcdElement.StructureId, 8, false) == "__PCDS__") {
            PcdElementValid = true;
            BpmElementSize += sizeof(PLATFORM_CONFIG_DATA_ELEMENT) - sizeof(UINT8*);
        }
        PdrElement = *(PDRS_SEGMENT*)(fv + BpmElementSize);
        if (Buffer::charToString((INT8*)PdrElement.StructureId, 8, false) == "__PDRS__") {
            PdrElementValid = true;
            BpmElementSize += sizeof(PDRS_SEGMENT) - sizeof(PDR_LOCATION_STRUCTURE*);
            INT64 PdrDataSize = PdrElement.SizeOfData - sizeof(UINT8);
            for (int i = 0; (i + 1) * sizeof(PDR_LOCATION_STRUCTURE) <= PdrDataSize; ++i) {
                PDR_LOCATION_STRUCTURE PdrLocation = *(PDR_LOCATION_STRUCTURE*)(fv + BpmElementSize + i * sizeof(PDR_LOCATION_STRUCTURE));
                PdrLocations.push_back(PdrLocation);
            }
            BpmElementSize += PdrElement.SizeOfData - sizeof(UINT8);
        }
        CnbsElement = *(CNBS_SEGMENT*)(fv + BpmElementSize);
        if (Buffer::charToString((INT8*)CnbsElement.StructureId, 8, false) == "__CNBS__") {
            CnbsElementValid = true;
            BpmElementSize += sizeof(CNBS_SEGMENT);
        }
    }

    PCDS_Class::~PCDS_Class() {}

    void PCDS_Class::setInfoStr() {
        INT64 width = 22;
        stringstream ss;
        ss.setf(ios::left);
        if (PcdElementValid) {
            ss << "PcdElement:\n"
               << setw(width) << "  Structure ID:"    << hex << uppercase << Buffer::charToString((INT8*)(PcdElement.StructureId), 8, false) << "\n"
               << setw(width) << "  StructVersion:"   << hex << uppercase << (UINT32)PcdElement.StructVersion << "h\n"
               << setw(width) << "  ElementSize:"     << hex << uppercase << PcdElement.ElementSize << "h\n"
               << setw(width) << "  PDR Size:"        << hex << uppercase << PcdElement.SizeOfData << "h\n";
        }
        if (PdrElementValid) {
            ss << setw(width) << "  Structure ID:"    << hex << uppercase << Buffer::charToString((INT8*)(PdrElement.StructureId), 8, false) << "\n"
               << setw(width) << "  StructVersion:"   << hex << uppercase << (UINT32)PdrElement.StructVersion << "h\n"
               << setw(width) << "  Struct Size:"     << hex << uppercase << PdrElement.SizeOfData << "h\n";
            for (auto Pdr:PdrLocations) {
                ss << "  TPM Power Down Request Location:\n"
                   << setw(width) << "    Media Type:"       << hex << uppercase << (UINT32)Pdr.MediaType << "h\n"
                   << setw(width) << "    NVIndex:"          << hex << uppercase << Pdr.NVIndex << "h\n"
                   << setw(width) << "    BitFieldWidth:"    << hex << uppercase << (UINT32)Pdr.BitFieldWidth << "h\n"
                   << setw(width) << "    BitFieldPosition:" << hex << uppercase << (UINT32)Pdr.BitFieldPosition << "h\n"
                   << setw(width) << "    ByteOffset:"       << hex << uppercase << (UINT32)Pdr.ByteOffset << "h\n";
            }
        }
        if (CnbsElementValid) {
            ss << setw(width) << "  Structure ID:"    << hex << uppercase << Buffer::charToString((INT8*)(CnbsElement.StructureId), 8, false) << "\n"
               << setw(width) << "  StructVersion:"   << hex << uppercase << (UINT32)CnbsElement.StructVersion << "h\n"
               << setw(width) << "  Struct Size:"     << hex << uppercase << CnbsElement.SizeOfData << "h\n"
               << setw(width) << "  Base:"            << hex << uppercase << CnbsElement.BufferData.Base << "h\n"
               << setw(width) << "  Size:"            << hex << uppercase << CnbsElement.BufferData.Size << "h\n"
               << setw(width) << "  Flags:";
            ss.setf(ios::right);
            ss << hex << uppercase << setw(4) << setfill('0') << CnbsElement.BufferData.Flags << "h\n";
        }
        InfoStr = ss.str() + "\n";
    }

    PMSG_Class::PMSG_Class(UINT8* fv, INT64 length):BpmElement(fv, length) {
        PmsgElement = *(BOOT_POLICY_MANIFEST_SIGNATURE_ELEMENT*)fv;
        if (Buffer::charToString((INT8*)PmsgElement.StructureId, 8, false) == "__PMSG__") {
            PmsgElementValid = true;
            BpmElementSize += sizeof(BOOT_POLICY_MANIFEST_SIGNATURE_ELEMENT);
        }
    }

    PMSG_Class::~PMSG_Class() {}

    void PMSG_Class::setInfoStr() {
        INT64 width = 22;
        stringstream ss;
        ss.setf(ios::left);
        if (PmsgElementValid) {
            ss << "Boot Policy Manifest Signature Element:\n"
               << setw(width) << "  Structure ID:"    << hex << uppercase << Buffer::charToString((INT8*)(PmsgElement.StructureId), 8, false) << "\n"
               << setw(width) << "  StructVersion:"   << hex << uppercase << (UINT32)PmsgElement.StructVersion << "h\n";
        }
        InfoStr = ss.str() + "\n";
    }

}
