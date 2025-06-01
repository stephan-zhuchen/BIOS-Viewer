//
// Created by stephan on 9/5/2023.
//
#pragma once
#include "UEFI/Acpi.h"
#include "Volume.h"

class AcpiClass : public Volume {
  private:
    EFI_ACPI_DESCRIPTION_HEADER AcpiHeader{};
    bool ValidFlag{false};
    bool needValidation;

  public:
    string AcpiTableSignature;
    string AcpiTableOemID;
    string AcpiTableOemTableID;

    AcpiClass() = delete;
    AcpiClass(UINT8 *buffer, INT64 length, INT64 offset, bool check = true);
    ~AcpiClass() override = default;

    bool CheckValidation() override;
    INT64 SelfDecode() override;

    [[nodiscard]] bool isValid() const;
    static bool isAcpiHeader(const UINT8 *ImageBase, INT64 length);
    void setInfoStr() override;
};
