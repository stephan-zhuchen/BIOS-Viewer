/** @file
  This file contains functions for creating TPM Event Log

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 - 2020 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification
**/

#include "Base.h"

//
// Data structure definitions
//
#pragma pack (1)
//
// ACM definition
//
#define ACM_MODULE_TYPE_CHIPSET_ACM                     2
#define ACM_MODULE_SUBTYPE_CAPABLE_OF_EXECUTE_AT_RESET  0x1
#define ACM_MODULE_SUBTYPE_ANC_MODULE                   0x2
#define ACM_HEADER_FLAG_DEBUG_SIGNED                    BIT15
#define ACM_NPW_SVN                                     0x2
#define ACM_HEADER_VERSION_3                            (3 << 16)
#define ACM_PKCS_1_5_RSA_SIGNATURE_SHA256_SIZE          256
#define ACM_PKCS_1_5_RSA_SIGNATURE_SHA384_SIZE          384

typedef struct {
  UINT16     ModuleType;
  UINT16     ModuleSubType;
  UINT32     HeaderLen;
  UINT32     HeaderVersion;
  UINT16     ChipsetId;
  UINT16     Flags;
  UINT32     ModuleVendor;
  UINT32     Date;
  UINT32     Size;
  UINT16     AcmSvn;
  UINT16     SeAcmSvn;
  UINT32     CodeControl;
  UINT32     ErrorEntryPoint;
  UINT32     GdtLimit;
  UINT32     GdtBasePtr;
  UINT32     SegSel;
  UINT32     EntryPoint;
  UINT8      Rsvd2[64];
  UINT32     KeySize;            // 64 DWORDS in the Key
  UINT32     ScratchSize;
} ACM_HEADER;

struct Ext_ACM_Header {
  UINT8      Rsa2048PubKey[256];
  UINT32     RsaPubExp;
  UINT8      Rsa2048Sig[256];
  UINT8      Scratch[572];       // 143 DWORDS = 572 BYTES Scratch Size
};

struct Ext_ACM_Header3{
  UINT8      Rsa3072PubKey[384];
  UINT8      Rsa3072Sig[384];
  UINT8      Scratch[832];       // 208 DWORDS = 832 BYTES Scratch Size
};

#pragma pack (push, 1)
typedef struct {
  EFI_GUID Guid;
  UINT8   ChipsetAcmType;
  UINT8   AitVersion;
  UINT16  AitLength;
  UINT32  ChipsetIdList;
  UINT32  OsSinitTblVer;
  UINT32  MinMleHeaderVer;
  UINT32  SinitCaps;
  UINT8   SinitVersion;
  UINT8   AitRevision[3];
  UINT32  ProcessorIdList;
  UINT32  TpmInfoList;
} ACM_INFO_TABLE;
#pragma pack (pop)

//
// BPM Policy:
//   FIT record type 12 points to Boot Policy Manifest.
//   FIT record type 11 points to Key Manifest.
//

#define RSA_KEY_SIZE_1K                     SIZE_1KB
#define RSA_KEY_SIZE_2K                     SIZE_2KB
#define RSA_KEY_SIZE_3K                     (SIZE_1KB + SIZE_2KB)
#define PKCS_1_5_RSA_SHA1_SIGNATURE_SIZE    (RSA_KEY_SIZE_1K / 8)
#define PKCS_1_5_RSA_SHA256_SIGNATURE_SIZE  (RSA_KEY_SIZE_2K / 8)
#define PKCS_1_5_RSA_SHA384_SIGNATURE_SIZE  (RSA_KEY_SIZE_3K / 8)
#define SHA1_DIGEST_SIZE                    20
#define SHA256_DIGEST_SIZE                  32
#define SHA384_DIGEST_SIZE                  48
#define SM3_256_DIGEST_SIZE                 32

typedef union {
    UINT8   digest8[SHA1_DIGEST_SIZE];
    UINT32  digest32[SHA1_DIGEST_SIZE / 4];
} BtgSha1Digest_u;

typedef union {
    UINT8   digest8[SHA256_DIGEST_SIZE];
    UINT32  digest32[SHA256_DIGEST_SIZE / 4];
    UINT64  digest64[SHA256_DIGEST_SIZE / 8];
} BtgSha256Digest_u;

typedef union {
    UINT8   digest8[SHA384_DIGEST_SIZE];
    UINT32  digest32[SHA384_DIGEST_SIZE / 4];
    UINT64  digest64[SHA384_DIGEST_SIZE / 8];
} BtgSha384Digest_u;

//
// Hash structure
//
typedef struct {
  UINT16               HashAlg;
  UINT16               Size;
  BtgSha1Digest_u      HashBuffer;
} SHA1_HASH_STRUCTURE;

typedef struct {
  UINT16               HashAlg;
  UINT16               Size;
  BtgSha256Digest_u    HashBuffer;
} SHA256_HASH_STRUCTURE;

typedef struct {
  UINT16               HashAlg;
  UINT16               Size;
  BtgSha384Digest_u    HashBuffer;
} SHA384_HASH_STRUCTURE;

typedef struct {
  UINT16               HashAlg;
  UINT16               Size;
  UINT8                HashBuffer[1];
} SHAX_HASH_STRUCTURE;

struct HASH_STRUCTURE {
  UINT16               HashAlg;
  UINT16               Size;
  UINT8                *HashBuffer;
};

typedef struct {
  UINT16               Size;           //Total number of bytes of HASH_LIST structure
  UINT16               Count;          //Number of Digest elements
} HASH_LIST;

typedef struct {
  UINT16                 Size;         //Total number of bytes of HASH_LIST structure
  UINT16                 Count;        //Number of Digest elements
  SHA1_HASH_STRUCTURE    Sha1Digest;   //Array of digests  {AlgID, Size, HashValue; ...}
  SHA256_HASH_STRUCTURE  Sha256Digest; //Array of digests  {AlgID, Size, HashValue; ...}
  SHA256_HASH_STRUCTURE  ShaSm3Digest; //Array of digests  {AlgID, Size, HashValue; ...}
  SHA384_HASH_STRUCTURE  Sha384Digest; //Array of digests  {AlgID, Size, HashValue; ...}
} MAX_HASH_LIST;

typedef struct {
  UINT64               Usage;          // Bit mask of usages
  HASH_STRUCTURE       Digest;         // Standard BtG hash structure primitive
} SHAX_KMHASH_STRUCT;


#define ALG_RSA             0x1
#define ALG_RSASSA          0x14

#define RSA_PUBLIC_KEY_STRUCT_VERSION_1_0  0x10
#define RSA_PUBLIC_KEY_STRUCT_KEY_EXPONENT_DEFAULT  0x11 // NOT 0x10001

typedef struct {
  UINT8    Version;                    // 0x10
  UINT16   KeySizeBits;                // 1024 or 2048 or 3072 bits
} KEY_STRUCT_HEADER;

typedef struct {
  UINT8    Version;                    // 0x10
  UINT16   KeySizeBits;                // Number of bits in the modulus.
  UINT32   Exponent;                   // The public exponent (must be RSA_KEY_EXPONENT_VALUE)
  UINT8    Modulus[RSA_KEY_SIZE_2K/8]; // The modulus in LSB format (256 bytes)
} RSA2K_PUBLIC_KEY_STRUCT;

typedef struct {
  UINT8    Version;                    // 0x10
  UINT16   KeySizeBits;                // Number of bits in the modulus.
  UINT32   Exponent;                   // The public exponent (must be RSA_KEY_EXPONENT_VALUE)
  UINT8    Modulus[RSA_KEY_SIZE_3K/8]; // The modulus in LSB format (384 bytes)
} RSA3K_PUBLIC_KEY_STRUCT;

typedef struct {
  UINT8    Version;                    // 0x10
  UINT16   KeySizeBits;                // 1024 or 2048 or 3072 bits
  UINT32   Exponent;                   // The public exponent
  UINT8    Modulus[1];                 // The modulus in LSB format
} RSA_PUBLIC_KEY_STRUCT;

#define ECC_PUBLIC_KEY_STRUCT_VERSION_1_0  0x10
#define ECC_PUBLIC_KEY_STRUCT_KEY_SIZE_DEFAULT  256
#define ECC_PUBLIC_KEY_STRUCT_KEY_LEN_DEFAULT   (ECC_PUBLIC_KEY_STRUCT_KEY_SIZE_DEFAULT/8)
typedef struct {
  UINT8  Version;                      // 0x10
  UINT16 KeySizeBits;                  // 256 - Number of bits in key. Fixed for SM2
  UINT8  Qx[ECC_PUBLIC_KEY_STRUCT_KEY_LEN_DEFAULT];  // X component. Fixed size for SM2
  UINT8  Qy[ECC_PUBLIC_KEY_STRUCT_KEY_LEN_DEFAULT];  // Y component. Fixed size for SM2
} ECC_PUBLIC_KEY_STRUCT;

#define RSASSA_SIGNATURE_STRUCT_VERSION_1_0  0x10

typedef struct {
  UINT8    Version;
  UINT16   SigSizeBits;                // 2048 or 3072 bits
} SIGNATURE_STRUCT_HEADER;

typedef struct {
  UINT8    Version;
  UINT16   KeySizeBits;                // 2048 or 3072 bits
  UINT16   HashAlg;
  UINT8    Signature[1];
} RSASSA_SIGNATURE_STRUCT;

#define ECC_SIGNATURE_STRUCT_VERSION_1_0  0x10
typedef struct {
  UINT8    Version;
  UINT16   KeySizeBits;                // 256 or 384 bits
  UINT16   HashAlg;
} ECC_SIGNATURE_STRUCT;

#define KEY_SIGNATURE_STRUCT_VERSION_1_0  0x10
typedef struct {
  UINT8                      Version;
  UINT16                     KeyAlg;       // TPM_ALG_RSA=0x1 or TPM_ALG_ECC=0x23
} KEY_AND_SIGNATURE_STRUCT_HEADER;
typedef struct {
  UINT8                      Version;
  UINT16                     KeyAlg;       // TPM_ALG_RSA=0x1 or TPM_ALG_ECC=0x23
  union {                                  // Based on KeyAlg
    RSA_PUBLIC_KEY_STRUCT    RsaKey;
    ECC_PUBLIC_KEY_STRUCT    EccKey;
  } Key;
  UINT16                     SigScheme;    // TPM_ALG_RSASSA=0x14 or TPM_ALG_RSAPSS=0x15 or TPM_ALG_SM2=0x1B
  union {                                  // Based on KeyAlg
    RSASSA_SIGNATURE_STRUCT  SignatureRsa;
    ECC_SIGNATURE_STRUCT     SignatureEcc;
  } Sig;
} KEY_AND_SIGNATURE_STRUCT;

#define BP_KEY_TYPE_BOOT_POLICY_MANIFEST  0
#define BP_KEY_TYPE_KEY_MANIFEST          1

#define BOOT_POLICY_MANIFEST_HEADER_STRUCTURE_ID  (*(UINT64 *)"__ACBP__")
#define BOOT_POLICY_MANIFEST_HEADER_VERSION_2_1          0x21
typedef struct {
  UINT8              StructureId[8];
  UINT8              StructVersion;        // 0x21
  UINT8              HdrStructVersion;     // 0x20
  UINT16             HdrSize;              // total number of bytes in Header (i.e., offset to first element)
  UINT16             KeySignatureOffset;   // Offset from start of Bpm to KeySignature field of Signature Element
  UINT8              BpmRevision;
  UINT8              BpmRevocation;
  UINT8              AcmRevocation;
  UINT8              Reserved;
  UINT16             NemPages;
} BOOT_POLICY_MANIFEST_HEADER;

#define IBB_SEGMENT_FLAG_IBB     0x0
#define IBB_SEGMENT_FLAG_NON_IBB 0x1
typedef struct {
  UINT16             Reserved;         // Alignment
  UINT16             Flags;            // Control flags
  UINT32             Base;             // Segment base
  UINT32             Size;             // Segment size
} IBB_SEGMENT;

#define BOOT_POLICY_MANIFEST_IBB_ELEMENT_STRUCTURE_ID  (*(UINT64 *)"__IBBS__")
#define BOOT_POLICY_MANIFEST_IBB_ELEMENT_DIGEST_ID     (*(UINT64 *)"__DIGE__")

#define BOOT_POLICY_MANIFEST_IBB_ELEMENT_VERSION_2_0       0x20
#define IBB_FLAG_INITIAL_MEASURE_LOC3  0x2
#define IBB_FLAG_AUTHORITY_MEASURE     0x4
//
// BIT7 will be used to control the stability of PCR[0] by masking out mutable policies from
// ACM_BIOS_POLICY_STS (0xfed30378) which is part of extend value into PCR[0]
//
#define IBB_FLAG_SRTM_AC               0x80 //Bit 7
#define SRTM_AC_MASK                   0x20FFF

typedef struct {
  UINT8               StructureId[8];
  UINT8               StructVersion;   // 0x20
  UINT8               Reserved0;
  UINT16              ElementSize;     // Total number of bytes in the element
  UINT8               Reserved1;
  UINT8               SetType;
  UINT8               Reserved;
  UINT8               PbetValue;
  UINT32              Flags;
  UINT64              IbbMchBar;
  UINT64              VtdBar;
  UINT32              DmaProtBase0;
  UINT32              DmaProtLimit0;
  UINT64              DmaProtBase1;
  UINT64              DmaProtLimit1;
  SHAX_HASH_STRUCTURE PostIbbHash;
} IBB_ELEMENT;

#define BOOT_POLICY_MANIFEST_TXT_ELEMENT_STRUCTURE_ID  (*(UINT64 *)"__TXTS__")
#define BOOT_POLICY_MANIFEST_TXT_ELEMENT_VERSION_2_0       0x20
typedef struct {
  UINT8               StructureId[8];
  UINT8               StructVersion;   // 0x20
  UINT8               Reserved0;
  UINT16              ElementSize;     // Total number of bytes in the element
  UINT8               Reserved1;
  UINT8               SetType;
  UINT16              Reserved;
  UINT32              Flags;
  UINT16              PwrDownInterval;
  UINT8               PttCmosOffset0;
  UINT8               PttCmosOffset1;
  UINT16              AcpiBaseOffset;
  UINT16              Reserved2;
  UINT32              PrwmBaseOffset;
  HASH_LIST           DigestList;
  UINT8               Reserved3[3];
  UINT8               SegmentCount;
  IBB_SEGMENT*        TxtSegment;      // TxtSegment[SegmentCount]
} TXT_ELEMENT;

typedef struct {
  UINT8               SetNumber;
  UINT8               Reserved[3];
  UINT8               Data[48];
} BIOS_SPECIFIC_SEGMENT_STRUCTURE;

typedef struct {
  UINT8                 MediaType;
  UINT32                NVIndex;
  UINT8                 BitFieldWidth;
  UINT8                 BitFieldPosition;
  UINT8                 ByteOffset;
} PDR_LOCATION_STRUCTURE;

#define BOOT_POLICY_MANIFEST_BIOS_SPECIFIC_IBB_STRUCTURE_STRUCTURE_ID  (*(UINT64 *)"__BSIS__")
#define BOOT_POLICY_MANIFEST_BIOS_SPECIFIC_IBB_STRUCTURE_VERSION_2_0       0x20
typedef struct {
  UINT8                             StructureId[8];
  UINT8                             StructVersion;    // 0x10
  UINT16                            SizeOfData;
  UINT8                             Reserved0;
  BIOS_SPECIFIC_SEGMENT_STRUCTURE*  Data;             // BSIS data. Could be an array of BSSS structures - BSSS []
} BIOS_SPECIFIC_IBB_STRUCTURE;

#define BPM_CNBS_ELEMENT_STRUCTURE_ID (*(UINT64 *)"__CNBS__")
#define BPM_CNBS_ELEMENT_VERSION 0x10
typedef struct {
  UINT8               StructureId[8];
  UINT8               StructVersion; // 0x10
  UINT16              SizeOfData;
  UINT8               Reserved;
  IBB_SEGMENT         BufferData;
} CNBS_SEGMENT;

typedef struct {
  UINT8                    StructureId[8];
  UINT8                    StructVersion; // 0x10
  UINT16                   SizeOfData;
  UINT8                    Reserved;
  PDR_LOCATION_STRUCTURE*  Data;
} PDRS_SEGMENT;

#define BOOT_POLICY_MANIFEST_PLATFORM_CONFIG_DATA_ELEMENT_STRUCTURE_ID  (*(UINT64 *)"__PCDS__")
#define BOOT_POLICY_MANIFEST_PLATFORM_CONFIG_DATA_ELEMENT_VERSION_2_0       0x20
typedef struct {
  UINT8               StructureId[8];
  UINT8               StructVersion;   // 0x20
  UINT8               Reserved0;
  UINT16              ElementSize;     // Total number of bytes in the element
  UINT16              Reserved1;
  UINT16              SizeOfData;
  UINT8*              Data;            //Data[SizeofData]  // Any data but starts from PDRS
} PLATFORM_CONFIG_DATA_ELEMENT;

#define BOOT_POLICY_MANIFEST_PLATFORM_MANUFACTURER_ELEMENT_STRUCTURE_ID  (*(UINT64 *)"__PMDA__")
#define BOOT_POLICY_MANIFEST_PLATFORM_MANUFACTURER_ELEMENT_VERSION_2_0       0x20
typedef struct {
  UINT8               StructureId[8];
  UINT8               StructVersion;       // 0x20
  UINT8               Reserved0;
  UINT16              ElementSize;         // Total number of bytes in the element
  UINT16              Reserved1;
  UINT16              PmDataSize;          // required to be 4 byte multiple
  UINT8*              PmData;              // PmData[PmDataSize]
} PLATFORM_MANUFACTURER_ELEMENT;

#define BOOT_POLICY_MANIFEST_SIGNATURE_ELEMENT_STRUCTURE_ID  (*(UINT64 *)"__PMSG__")
#define BOOT_POLICY_MANIFEST_SIGNATURE_ELEMENT_VERSION_1_0       0x10
typedef struct {
  UINT8               StructureId[8];
  UINT8               StructVersion;       // 0x20
  UINT8               Reserved[3];         // KeySignature must be DWORD aligned
  KEY_AND_SIGNATURE_STRUCT  KeySignature;  // this is now a variable Size
} BOOT_POLICY_MANIFEST_SIGNATURE_ELEMENT;

#define KEY_MANIFEST_STRUCTURE_ID  (*(UINT64 *)"__KEYM__")
#define KEY_MANIFEST_STRUCTURE_VERSION_2_1     0x21
typedef struct {
  UINT8               StructureId[8];
  UINT8               StructVersion;       // 0x21
  UINT8               reserved[3];         // 3 bytes to make KeySignatureOffset same offset as for BPM
  UINT16              KeySignatureOffset;  // Offset from start of KM to KeyManifestSignature
  UINT8               Reserved2[3];        // Alignment
  UINT8               KeyManifestRevision;
  UINT8               KmSvn;
  UINT8               KeyManifestId;
  UINT16              KmPubKeyHashAlg;
  UINT16              KeyCount;
} KEY_MANIFEST_STRUCTURE;

//
// Detail PCR data
//
typedef struct {
  UINT64 AcmPolicySts;
  UINT16 AcmSvn;                               // ACM_SVN from ACM Header
  UINT8  AcmRsaSignature[];                    // copy from ACM.HEADER.RSASIG
} DETAIL_PCR_DATA;

typedef struct {
  UINT64  AcmPolicySts;                        // lower 8 bits of the BP.RSTR
  UINT16  AcmSvn;                              // ACM_SVN from ACM Header
  UINT8   buffer[RSA_KEY_SIZE_3K/8 + RSA_KEY_SIZE_3K/8 + RSA_KEY_SIZE_3K/8 + SHA384_DIGEST_SIZE ] ;
} MAX_DETAIL_PCR_DATA;

//
// Authority PCR data
//
typedef struct {
  UINT64 AcmPolicySts;
  UINT16 AcmSvn;                               // ACM_SVN from ACM Header
  UINT8  AcmKeyHash[];                         // The hash of the key used to verify the ACM (SHAxxx)
} AUTHORITY_PCR_DATA;

typedef struct {
  UINT64 AcmPolicySts;
  UINT16 AcmSvn;                               // ACM_SVN from ACM Header
  UINT8  AcmKeyHash[SHA384_DIGEST_SIZE];       // The hash of the key used to verify the ACM (SHAxxx)
  UINT8  BpKeyHash[SHA384_DIGEST_SIZE];        // The hash of the key used to verify the Key Manifest (SHAxxx)
  UINT8  BpmKeyHashFromKm[SHA384_DIGEST_SIZE]; // The hash of the key used to verify the Boot Policy Manifest (SHAxxx)
} MAX_AUTHORITY_PCR_DATA;

//
// Efi Startup Locality Event Data
//
typedef struct {
  UINT8   Signature[16];
  UINT8   StartupLocality;
} TCG_EFI_STARTUP_LOCALITY_EVENT;

//typedef struct {
//  EFI_GUID                  *EventGuid;
//  EFI_TCG2_EVENT_LOG_FORMAT  LogFormat;
//} EFI_TCG2_EVENT_INFO_STRUCT;

typedef union {
  struct {
    UINT64 KmId               : 4;      // 0-3   Key Manifest ID used for verified Key Manifest
    UINT64 MeasuredBoot       : 1;      // 4     perform measured boot
    UINT64 VerifiedBoot       : 1;      // 5     perform verified boot
    UINT64 HAP                : 1;      // 6     high assurance platform
    UINT64 TxtSupported       : 1;      // 7     txt supported
    UINT64 BootMedia          : 1;      // 8     Boot media
    UINT64 DCD                : 1;      // 9     disable CPU debug
    UINT64 DBI                : 1;      // 10    disable BSP init
    UINT64 PBE                : 1;      // 11    protect BIOS environment
    UINT64 BBP                : 1;      // 12    bypass boot policy - fast S3 resume
    UINT64 TpmType            : 2;      // 13-14 TPM Type
    UINT64 TpmSuccess         : 1;      // 15    TPM Success
    UINT64 Reserved1          : 1;      // 16
    UINT64 BootPolicies       : 1;      // 17    PFR supported
    UINT64 BackupActions      : 2;      // 18-19 Backup actions
    UINT64 TxtProfile         : 5;      // 20-24 TXT profile selection
    UINT64 MemScrubPolicy     : 2;      // 25-26 Memory scrubbing policy
    UINT64 Reserved2          : 2;      // 27-28
    UINT64 DmaProtection      : 1;      // 29    DMA Protection
    UINT64 Reserved3          : 2;      // 30-31
    UINT64 SCrtmStatus        : 3;      // 32-34 S-CRTM status
    UINT64 Cosign             : 1;      // 35    CPU co-signing
    UINT64 TpmStartupLocality : 1;      // 36    TPM startup locality.
    UINT64 Reserved           :27;      // 37-63
  } Bits;
  UINT64 Data;
} ACM_BIOS_POLICY;

#pragma pack ()
