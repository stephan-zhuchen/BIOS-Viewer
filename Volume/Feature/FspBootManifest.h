#ifndef FSPBOOTMANIFESTCLASS_H
#define FSPBOOTMANIFESTCLASS_H

#include "Volume.h"
#include "UEFI/FbmDef.h"

struct FSP_REGION {
    FSP_REGION_STRUCTURE    FSP_REGION_Header;
    vector<REGION_SEGMENT>  SegmentArray;
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
    vector<UINT8>                   KEY_Modulus;
    UINT16                          SigScheme;
    RSASSA_SIGNATURE                SignatureRsa;
    vector<UINT8>                   Signature;
};

class FspBootManifestClass : public Volume
{
private:
    bool                        ValidFlag{true};
    FSP_BOOT_MANIFEST_STRUCTURE FbmStruct;
    vector<FSP_REGION>          FspRegions;
    KEY_AND_SIGNATURE           KeyAndSignature;
public:
    FspBootManifestClass()=delete;
    FspBootManifestClass(UINT8* buffer, INT64 length, INT64 offset);
    ~FspBootManifestClass() override;

    INT64 SelfDecode() override;
    void setInfoStr() override;
    string GetFspComponentFromID(UINT8 ComponentID);
    string GetRsaAlgFromID(UINT32 RsaAlgID);
    string GetHashAlgFromID(UINT16 HashAlgID);

    [[nodiscard]] inline bool isValid() const { return ValidFlag; };
};

#endif // FSPBOOTMANIFESTCLASS_H
