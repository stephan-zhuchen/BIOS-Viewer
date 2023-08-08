#ifndef BIOSGUARD_H
#define BIOSGUARD_H

#include "SymbolDefinition.h"

///
/// BIOS Guard update Package Header
///
typedef struct {
    UINT16 Version;           ///< Version of the update package header.  Must be 0x0002.
    UINT8  Reserved3[2];      ///< Reserved bits.
    UINT8  PlatId[16];        ///< PLAT_ID used to be compared against the one found in the BGPDT to prevent cross platform flashing.
    /**
  If any bit set in this field then BGUP must be signed and valid BGUPC must be provided for BGUP to be processed.
  - BIT[0]: Indicates write/erase operations will be executed on protected flash area indicated in the BGPDT SFAM.
  - BIT[1]: Indicates protected EC operations included.
  **/
    UINT16 PkgAttributes;
    UINT8  Reserved4[2];      ///< Reserved bits.
    UINT16 PslMajorVer;       ///< Indicates the PSL major version. Must be 2.
    UINT16 PslMinorVer;       ///< Indicates the PSL minor version. Must be 0.
    UINT32 ScriptSectionSize; ///< Size in bytes of the script.
    UINT32 DataSectionSize;   ///< Size of the data region in bytes.
    UINT32 BiosSvn;           ///< BIOS SVN.
    UINT32 EcSvn;             ///< EC SVN.
    UINT32 VendorSpecific;    ///< Vendor specific data.
} BGUP_HEADER;

///
/// Memory location for BGUPC and BIOS Guard LOG inside BIOS Guard DPR allocated memory for Tool interface
///
#define BGUPC_MEMORY_SIZE            0x00008000                                          ///< 32KB
#define BIOSGUARD_LOG_MEMORY_SIZE    0x00020000                                          ///< 128KB
#define BGUPC_MEMORY_OFFSET          (BGUPC_MEMORY_SIZE + BIOSGUARD_LOG_MEMORY_SIZE)     ///< BiosGuardMemAddress + BiosGuardMemSize - BIOSGUARD_LOG_MEMORY_SIZE - 32KB
#define BIOSGUARD_LOG_MEMORY_OFFSET  BIOSGUARD_LOG_MEMORY_SIZE                           ///< BiosGuardMemAddress + BiosGuardMemSize - 128KB
#define MAX_BIOSGUARD_LOG_PAGE       ((BIOSGUARD_LOG_MEMORY_SIZE / EFI_PAGE_SIZE) - 2)   ///< 30 4KB Pages

///
/// Memory Size for BIOS Guard Update Package when in TSEG
///
#define BGUP_TSEG_BUFFER_SIZE        0x00014000  ///< 16KB Script + 64KB Flash Block.

///
/// BIOS Guard update package definition for BIOS SMM Initiated runtime calls
///
typedef struct {
    BGUP_HEADER BgupHeader;                            ///< BIOS Guard update package header.
    UINT64      BgupBuffer[BGUP_TSEG_BUFFER_SIZE / 8]; ///< BIOS Guard update buffer - Designed to contain the BIOS Guard Script followed immediately by the Update Data
} BGUP;

#define BGUPC_RESERVED_DEFAULT_VALUE  0x0000

#define BGUPC_HDR_VER_1                    0x0001
//
// PKCS1 1.5 digital signature (SHA-256 hash, RSA 2048 key)
//
#define BGUPC_ALG_PKCS1_15_SHA256_RSA2048  0x00000001
//
// PKCS1 v2.1 EMSA/PSS digital signature (SHA-256 hash, RSA 2048 key)
//
#define BGUPC_ALG_PKCS1_21_SHA256_RSA2048  0x00000002

#define BGUPC_HDR_VER_2                    0x0002
//
// PKCS1 1.5 digital signature (SHA-256 hash, RSA 3072 key)
//
#define BGUPC_ALG_PKCS1_15_SHA256_RSA3072  0x00000003
//
// PKCS1 v2.1 EMSA/PSS digital signature (SHA-256 hash, RSA 3072 key)
//
#define BGUPC_ALG_PKCS1_21_SHA256_RSA3072  0x00000004

#define BGUPC_HDR_VER_3                    0x0003
//
// PKCS1 1.5 digital signature (SHA-384 hash, RSA 3072 key)
//
#define BGUPC_ALG_PKCS1_15_SHA384_RSA3072  0x00000005
//
// PKCS1 v2.1 EMSA/PSS digital signature (SHA-384 hash, RSA 3072 key)
//
#define BGUPC_ALG_PKCS1_21_SHA384_RSA3072  0x00000006

typedef struct {
    UINT16  Version;
    UINT16  Reserved;
    UINT32  Algorithm;
} BGUPC_HEADER;

#endif // BIOSGUARD_H
