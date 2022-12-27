#include "UefiLib.h"

namespace UefiSpace {
    FirmwareVolume::FirmwareVolume()
    {

    }

    FirmwareVolumeHeaderClass::FirmwareVolumeHeaderClass(EFI_FIRMWARE_VOLUME_HEADER fv):
        FirmwareVolumeHeader(fv) {}

    GUID FirmwareVolumeHeaderClass::getFvGuid() const
    {
        EFI_GUID guid = FirmwareVolumeHeader.FileSystemGuid;
        return GUID(guid);
    }
}
