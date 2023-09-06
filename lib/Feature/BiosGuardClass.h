//
// Created by stephan on 9/4/2023.
//
#pragma once
#include <QString>
#include "Volume.h"
#include "UEFI/BiosGuard.h"

struct BgslOperation {
    UINT16 OpCode;
    UINT8  Op1;
    UINT8  Op2;
    UINT32 OpNum;
    QString getOperation();
};

class BiosGuardClass: public Volume {
private:
    BGUP_HEADER   BgupHeader;
    QString       Content;
    QString       BiosGuardScript;
    BGUPC_HEADER  BgupCHeader;
    QString       Algorithm;
    INT32         ModulusSize;
    UINT8         *ModulusData{nullptr};
    INT32         RSAKeySize;
    UINT8         *UpdatePackageDigest{nullptr};
public:
    BiosGuardClass()=delete;
    BiosGuardClass(UINT8* buffer, INT64 length, INT64 offset);

    INT64 SelfDecode() override;
    ~BiosGuardClass() override;
    void setInfoStr() override;
    [[nodiscard]] QStringList getUserDefinedName() const override;

    void setContent(QString content);
    void decodeBgsl(UINT8* buffer, INT64 length);
};
