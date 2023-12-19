#ifndef FSPBOOTMANIFESTCLASS_H
#define FSPBOOTMANIFESTCLASS_H

#include <string>
#include <QVector>
#include "Volume.h"
#include "UEFI/FbmDef.h"

struct FSP_REGION {
    FSP_REGION_STRUCTURE FSP_REGION_Header;
    QVector<IBB_SEGMENT> SegmentArray;
};

#pragma pack(push, 1)
struct RSA_PUBKEY {
    UINT8                       Version;                    // 0x10
    UINT16                      KeySizeBits;                // 1024 or 2048 or 3072 bits
    UINT32                      Exponent;
};

struct RSASSA_SIGNATURE {
    UINT8                       Version;
    UINT16                      KeySizeBits;                // 2048 or 3072 bits
    UINT16                      HashAlg;
};
#pragma pack(pop)

struct KEY_AND_SIGNATURE {
    KEY_AND_SIGNATURE_STRUCT_HEADER Header;
    RSA_PUBKEY                      RsaKey;
    QByteArray                      KEY_Modulus;
    UINT16                          SigScheme;
    RSASSA_SIGNATURE                SignatureRsa;
    QByteArray                      Signature;
};

class FspBootManifestClass : public Volume
{
private:
    bool                        ValidFlag{true};
    FSP_BOOT_MANIFEST_STRUCTURE FbmStruct;
    QVector<FSP_REGION>         FspRegions;
    KEY_AND_SIGNATURE           KeyAndSignature;
public:
    FspBootManifestClass()=delete;
    FspBootManifestClass(UINT8* buffer, INT64 length, INT64 offset);
    ~FspBootManifestClass() override;

    INT64 SelfDecode() override;
    void setInfoStr() override;
    std::string GetFspComponentFromID(UINT8 ComponentID);
    std::string GetRsaAlgFromID(UINT8 RsaAlgID);
    std::string GetHashAlgFromID(UINT8 HashAlgID);

    [[nodiscard]] inline bool isValid() const { return ValidFlag; };
};

#endif // FSPBOOTMANIFESTCLASS_H
