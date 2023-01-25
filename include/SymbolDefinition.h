#pragma once

#define CONST   const
#define STATIC  static
#define VOID    void
#define UINTN   UINT64
#define INTN    INT64
#define IN
#define OUT
#define OPTIONAL
#define EFIAPI
#define SCRATCH_BUFFER_REQUEST_SIZE  SIZE_64KB

using UINT8  = unsigned char;
using BOOLEAN = UINT8;
using CHAR8  = char;
using UINT16 = unsigned short;
using CHAR16 = UINT16;
using UINT32 = unsigned int;
using UINT64 = unsigned long long;
using INT8   = char;
using INT16  = short;
using INT32  = int;
using INT64  = long long;

struct EFI_GUID {
    UINT32    Data1;
    UINT16    Data2;
    UINT16    Data3;
    UINT8     Data4[8];
    bool operator==(EFI_GUID guid);
};

//
// FIT spec
//
#pragma pack (1)
struct FIRMWARE_INTERFACE_TABLE_ENTRY {
  UINT64     Address;
  UINT8      Size[3];
  UINT8      Rsvd;
  UINT16     Version;
  UINT8      Type:7;
  UINT8      C_V:1;
  UINT8      Checksum;
};
#pragma pack ()

#define DEFAULT_FIT_TABLE_POINTER_OFFSET  0x40
#define DEFAULT_FIT_ENTRY_VERSION         0x0100
#define FIT_SIGNATURE                     0x2020205F5449465F

#define FIT_TABLE_TYPE_HEADER                      0
#define FIT_TABLE_TYPE_MICROCODE                   1
#define FIT_TABLE_TYPE_STARTUP_ACM                 2
#define FIT_TABLE_TYPE_DIAGNST_ACM                 3
#define FIT_TABLE_TYPE_PROT_BOOT_POLICY            4
#define FIT_TABLE_TYPE_BIOS_MODULE                 7
#define FIT_TABLE_TYPE_TPM_POLICY                  8
#define FIT_TABLE_TYPE_BIOS_POLICY                 9
#define FIT_TABLE_TYPE_TXT_POLICY                  10
#define FIT_TABLE_TYPE_KEY_MANIFEST                11
#define FIT_TABLE_TYPE_BOOT_POLICY_MANIFEST        12
#define FIT_TABLE_TYPE_BIOS_DATA_AREA              13
#define FIT_TABLE_TYPE_CSE_SECURE_BOOT             16
#define FIT_TABLE_SUBTYPE_FIT_PATCH_MANIFEST       12
#define FIT_TABLE_SUBTYPE_ACM_MANIFEST             13
#define FIT_TABLE_TYPE_VAB_PROVISION_TABLE         26
#define FIT_TABLE_TYPE_VAB_BOOT_IMAGE_MANIFEST     27
#define FIT_TABLE_TYPE_VAB_BOOT_KEY_MANIFEST       28
