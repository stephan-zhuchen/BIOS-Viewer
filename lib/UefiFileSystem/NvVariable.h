//
// Created by stephan on 9/2/2023.
//

#pragma once

#include <string>
#include "Volume.h"
#include "UEFI/VariableFormat.h"

class NvVariableEntry : public Volume {
public:
    VARIABLE_HEADER               *VariableHeader;
    AUTHENTICATED_VARIABLE_HEADER *AuthVariableHeader;
    bool                          AuthFlag;
    std::string                   VariableName;
    UINT8                         *DataPtr;
    INT64                         DataSize;

    NvVariableEntry() = delete;
    NvVariableEntry(UINT8* buffer, INT64 offset, bool isAuth, Volume* parent= nullptr);
    ~NvVariableEntry() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void  setInfoStr() override;

    [[nodiscard]] INT64   getHeaderSize() const override;
    [[nodiscard]] QStringList getUserDefinedName() const override;
};

class NvStorageVariable : public Volume {
public:
    VARIABLE_STORE_HEADER       NvStoreHeader{};
    bool                        AuthFlag;

    NvStorageVariable() = delete;
    NvStorageVariable(UINT8* buffer, INT64 length, INT64 offset, bool Compressed=false, Volume* parent= nullptr);
    ~NvStorageVariable() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void  setInfoStr() override;
};

class FaultTolerantBlock : public Volume {
public:
    EFI_FAULT_TOLERANT_WORKING_BLOCK_HEADER TolerantHeader;

    FaultTolerantBlock() = delete;
    FaultTolerantBlock(UINT8* buffer, INT64 length, INT64 offset, bool Compressed=false, Volume* parent= nullptr);
    ~FaultTolerantBlock() override;

    bool  CheckValidation() override;
    INT64 SelfDecode() override;
    void  DecodeChildVolume() override;
    void  setInfoStr() override;
};