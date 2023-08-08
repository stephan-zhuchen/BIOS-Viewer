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
        ValidFlag = (acmHeader.ModuleType == ACM_MODULE_TYPE_CHIPSET_ACM);
        ProdFlag = (acmHeader.Flags & 0x8000) == 0;
        if (acmHeader.HeaderVersion < ACM_HEADER_VERSION_3) {
            ExtAcmHeader = *(Ext_ACM_Header*)(fv + sizeof(ACM_HEADER));
        } else {
            isAcm3 = true;
            ExtAcmHeader3 = *(Ext_ACM_Header3*)(fv + sizeof(ACM_HEADER));
        }
        auto *AcmPtr = (UINT8 *)fv;
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
    }

    AcmHeaderClass::~AcmHeaderClass() = default;

    bool AcmHeaderClass::isValid() const {
        return ValidFlag;
    }

    bool AcmHeaderClass::isProd() const {
        return ProdFlag;
    }

    void AcmHeaderClass::setInfoStr() {
        INT32 width = 20;
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
}
