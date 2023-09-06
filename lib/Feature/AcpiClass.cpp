//
// Created by stephan on 9/5/2023.
//
#include "BaseLib.h"
#include "AcpiClass.h"

using namespace std;
using namespace BaseLibrarySpace;

AcpiClass::AcpiClass(UINT8 *buffer, INT64 length, INT64 offset, bool needValidation):
        Volume(buffer, length, offset, needValidation, nullptr), needValidation(needValidation) { }

bool AcpiClass::CheckValidation() {
    if (needValidation && size < sizeof(EFI_ACPI_DESCRIPTION_HEADER)) {
        ValidFlag = false;
        return false;
    }
    UINT32 length = *(UINT32*) (data + sizeof(UINT32));
    if (needValidation && length != size) {
        ValidFlag = false;
        return false;
    }
    if (needValidation && CalculateSum8(data, size) != 0) {
        ValidFlag = false;
        return false;
    }
    return true;
}

INT64 AcpiClass::SelfDecode() {
    if (!CheckValidation())
        return 0;
    Type = VolumeType::AcpiTable;
    ValidFlag = true;
    AcpiHeader = *(EFI_ACPI_DESCRIPTION_HEADER*)data;
    AcpiTableSignature = QString::fromStdString(charToString((CHAR8*)data, sizeof(UINT32), false));
    AcpiTableOemID = QString::fromStdString(charToString((CHAR8*)&AcpiHeader.OemId, sizeof(UINT32), false));
    AcpiTableOemTableID = QString::fromStdString(charToString((CHAR8*)&AcpiHeader.OemTableId, sizeof(UINT32), false));
    return size;
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

    ss << setw(width) << "Signature:"   << AcpiTableSignature.toStdString() << "\n"
       << setw(width) << "Length:"      << hex << uppercase << AcpiHeader.Length << "h\n"
       << setw(width) << "Revision:"    << hex << uppercase << (UINT16)AcpiHeader.Revision << "h\n"
       << setw(width) << "OemId:"       << charToString((CHAR8*)&AcpiHeader.OemId, 6, false) << "\n"
       << setw(width) << "OemTableId:"  << charToString((CHAR8*)&AcpiHeader.OemTableId, 8, false) << "\n";

    InfoStr = QString::fromStdString(ss.str());
}
