//
// Created by stephan on 8/29/2023.
//

#pragma once
#include "Volume.h"
#include "UEFI/PiFirmwareVolume.h"

class FirmwareVolume: public Volume {
private:
    EFI_FIRMWARE_VOLUME_HEADER     FirmwareVolumeHeader{};
    EFI_FIRMWARE_VOLUME_EXT_HEADER FirmwareVolumeExtHeader{};
    INT64                          FirmwareVolumeHeaderSize;
    bool                           isExt{false};
    bool                           isNv{false};

public:
    FirmwareVolume() = delete;
    FirmwareVolume(UINT8* buffer, INT64 length, INT64 offset, bool Compressed=false, Volume* parent= nullptr);
    ~FirmwareVolume() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void  setInfoStr() override;
    [[nodiscard]] INT64 getHeaderSize() const override;
    [[nodiscard]] EFI_GUID getVolumeGuid() const override;
    [[nodiscard]] EFI_GUID getFvGuid(bool returnExt=true) const;

    INT64 GetFreeSpaceSize() const;

    static bool isValidFirmwareVolume(EFI_FIRMWARE_VOLUME_HEADER* address);
};

