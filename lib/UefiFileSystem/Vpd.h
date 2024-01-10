#ifndef VPD_H
#define VPD_H

#include <Volume.h>
// #include "BaseLib.h"
#include "UEFI/VariableFormat.h"
#include "UEFI/PcdDataBaseSignatureGuid.h"

class Vpd : public Volume
{
private:
    INT32                               VpdOffset{0};
    PCD_NV_STORE_DEFAULT_BUFFER_HEADER  VpdPcdHeader;
    PCD_DEFAULT_DATA                    PcdDefaultData;
    VARIABLE_STORE_HEADER               VariableStoreHeader;
public:
    Vpd()=delete;
    Vpd(UINT8* buffer, INT64 length, INT64 offset, bool Compressed=false, Volume* parent= nullptr);
    ~Vpd() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void  setInfoStr() override;
};

#endif // VPD_H
