#ifndef ACPI_H
#define ACPI_H

#include "SymbolDefinition.h"
///
/// Common table header, this prefaces all ACPI tables, including FACS, but
/// excluding the RSD PTR structure.
///
typedef struct {
    UINT32    Signature;
    UINT32    Length;
} EFI_ACPI_COMMON_HEADER;

#pragma pack(1)
///
/// The common ACPI description table header.  This structure prefaces most ACPI tables.
///
typedef struct {
    UINT32    Signature;
    UINT32    Length;
    UINT8     Revision;
    UINT8     Checksum;
    UINT8     OemId[6];
    UINT64    OemTableId;
    UINT32    OemRevision;
    UINT32    CreatorId;
    UINT32    CreatorRevision;
} EFI_ACPI_DESCRIPTION_HEADER;
#pragma pack()

#endif // ACPI_H
