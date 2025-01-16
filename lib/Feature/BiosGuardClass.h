//
// Created by stephan on 9/4/2023.
//
#pragma once
#include "Volume.h"
#include "UEFI/BiosGuard.h"

struct BgslOperation {
    UINT16 OpCode;
    UINT8  Op1;
    UINT8  Op2;
    UINT32 OpNum;
    string getOperation();
};

class BiosGuardClass: public Volume {
private:
    BGUP_HEADER   BgupHeader;
    string        Content;
    string        BiosGuardScript;
    BGUPC_HEADER  BgupCHeader;
    string        Algorithm;
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
    [[nodiscard]] vector<string> getUserDefinedName() const override;

    void setContent(string content);
    void decodeBgsl(UINT8* buffer, INT64 length);
    string getPlatID();
};
