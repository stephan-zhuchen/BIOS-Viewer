//
// Created by stephan on 9/5/2023.
//
#include "BaseLib.h"
#include "AcpiClass.h"

using namespace std;
using namespace BaseLibrarySpace;

AcpiClass::AcpiClass(UINT8 *buffer, INT64 length, INT64 offset, bool needValidation):
        data(buffer), size(length), offset(offset)
{
    if (needValidation && length < sizeof(EFI_ACPI_DESCRIPTION_HEADER)) {
        ValidFlag = false;
        return;
    }
    UINT32 size = *(UINT32*) (buffer + sizeof(UINT32));
    if (needValidation && size != length) {
        ValidFlag = false;
        return;
    }
    if (needValidation && CalculateSum8(buffer, length) != 0) {
        ValidFlag = false;
        return;
    }
    ValidFlag = true;
    AcpiHeader = *(EFI_ACPI_DESCRIPTION_HEADER*)buffer;
    AcpiTableSignature = charToString((CHAR8*)buffer, sizeof(UINT32), false);
    AcpiTableOemID = charToString((CHAR8*)&AcpiHeader.OemId, sizeof(UINT32), false);
    AcpiTableOemTableID = charToString((CHAR8*)&AcpiHeader.OemTableId, sizeof(UINT32), false);
}

bool AcpiClass::isValid() const {
    return ValidFlag;
}

bool AcpiClass::isAcpiHeader(const UINT8 *ImageBase, INT64 length) {
    if (length < sizeof(EFI_ACPI_DESCRIPTION_HEADER)) {
        return false;
    }
    UINT32 size = *(UINT32*) (ImageBase + sizeof(UINT32));
    if (size != length) {
        return false;
    }
    if (CalculateSum8(ImageBase, length) != 0) {
        return false;
    }
    return true;
}

void AcpiClass::setInfoStr() {
    INT32 width = 20;
    stringstream ss;
    stringstream guidInfo;
    ss.setf(ios::left);

    ss << setw(width) << "Signature:"   << AcpiTableSignature << "\n"
       << setw(width) << "Length:"      << hex << uppercase << AcpiHeader.Length << "h\n"
       << setw(width) << "Revision:"    << hex << uppercase << (UINT16)AcpiHeader.Revision << "h\n"
       << setw(width) << "OemId:"       << charToString((CHAR8*)&AcpiHeader.OemId, 6, false) << "\n"
       << setw(width) << "OemTableId:"  << charToString((CHAR8*)&AcpiHeader.OemTableId, 8, false) << "\n";

    InfoStr = QString::fromStdString(ss.str());
}

AcpiClass::~AcpiClass() = default;