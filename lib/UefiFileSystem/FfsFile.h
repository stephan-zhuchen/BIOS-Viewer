//
// Created by stephan on 8/29/2023.
//

#pragma once
#include "Volume.h"
#include "UEFI/PiFirmwareFile.h"

class FfsFile: public Volume {
private:
    EFI_FFS_FILE_HEADER    FfsHeader{};
    EFI_FFS_FILE_HEADER2   FfsExtHeader{};
    INT64                  FfsSize;
    bool                   isExtended{false};
    bool                   headerChecksumValid{false};
    bool                   dataChecksumValid{false};
public:
    FfsFile() = delete;
    FfsFile(UINT8* file, INT64 offset, bool Compressed=false, Volume* parent= nullptr);
    ~FfsFile() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void  setInfoStr() override;
    [[nodiscard]] INT64 getHeaderSize() const override;
    [[nodiscard]] EFI_GUID getVolumeGuid() const override;

    [[nodiscard]] UINT8 getType() const;
    [[nodiscard]] EFI_GUID getFfsGuid() const;
};

