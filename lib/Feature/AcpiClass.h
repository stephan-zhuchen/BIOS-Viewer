//
// Created by stephan on 9/5/2023.
//
#pragma once
#include "UEFI/Acpi.h"
#include <QString>

class AcpiClass {
private:
    UINT8*                        data;
    INT64                         size{};
    INT64                         offset{};
    EFI_ACPI_DESCRIPTION_HEADER   AcpiHeader{};
    bool                          ValidFlag{false};
    QString                       InfoStr;
public:
    std::string                   AcpiTableSignature;
    std::string                   AcpiTableOemID;
    std::string                   AcpiTableOemTableID;

    AcpiClass()=delete;
    AcpiClass(UINT8* buffer, INT64 length, INT64 offset, bool needValidation=true);
    ~AcpiClass();
    [[nodiscard]] bool isValid() const;
    static bool isAcpiHeader(const UINT8  *ImageBase, INT64 length);
    void setInfoStr();
    [[nodiscard]] QString getInfoText() const { return InfoStr; }
};