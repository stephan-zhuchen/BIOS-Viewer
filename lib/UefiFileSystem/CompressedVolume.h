#ifndef COMPRESSEDVOLUME_H
#define COMPRESSEDVOLUME_H

#include "C/Base.h"
#include "Volume.h"

#define  LZMA_SIGNATURE     SIGNATURE_32 ('L', 'Z', 'M', 'A')
#define  LZDM_SIGNATURE     SIGNATURE_32 ('L', 'Z', 'D', 'M')
#define  LZ_SIGNATURE_16    SIGNATURE_16 ('L', 'Z')
#define  IS_COMPRESSED(x)   (*(UINT16 *)(UINTN)(x) == LZ_SIGNATURE_16)

#pragma pack(1)
struct LOADER_COMPRESSED_HEADER{
    UINT32        Signature;
    UINT32        CompressedSize;
    UINT32        Size;
    UINT16        Version;
    UINT8         Svn;
    UINT8         Attribute;
    // UINT8         Data[];
};
#pragma pack()

class CompressedVolume : public Volume
{
private:
    LOADER_COMPRESSED_HEADER  *CompressHdr;
    QString                   ComprssedType;
public:
    CompressedVolume()=delete;
    CompressedVolume(UINT8* buffer, INT64 length, INT64 offset, Volume* parent= nullptr);
    ~CompressedVolume() override;
    QString GetComprssedType();

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void  setInfoStr() override;

    static bool IsCompressedVolume(LOADER_COMPRESSED_HEADER *buffer);
};

#endif // COMPRESSEDVOLUME_H
