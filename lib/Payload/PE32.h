#ifndef PE32_H
#define PE32_H

//#include "string"
#include "Volume.h"
#include "UEFI/PeImage.h"

using std::string;

class PE32 : public Volume
{
public:
    EFI_IMAGE_DOS_HEADER      dosHeader{};
    EFI_TE_IMAGE_HEADER       teHeader{};
    EFI_IMAGE_NT_HEADERS32    pe32Header{};
    EFI_IMAGE_NT_HEADERS64    pe32plusHeader{};
    bool                      isTE{false};
    bool                      isPe32Plus{false};
    bool                      isValid{true};
    UINT8                     *convertedPe32Data{nullptr};
    INT64                     convertedPe32Size{0};

    PE32()=delete;
    PE32(UINT8* file, INT64 length, INT64 offset, bool Compressed=false, Volume* parent= nullptr);
    ~PE32() override;

    // bool  CheckValidation() override;
    INT64 SelfDecode() override;
    // void  DecodeChildVolume() override;
    void  setInfoStr() override;

    void convert2Pe();
    [[nodiscard]] string getMachineType() const;
    static string getSubsystemName(UINT16 subsystem);
};

#endif // PE32_H
