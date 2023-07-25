/* me.h

Copyright (c) 2015, Nikolaj Schlej. All rights reserved.
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHWARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*/

#ifndef ME_H
#define ME_H

#include "SymbolDefinition.h"

// Make sure we use right packing rules
#pragma pack(1)

typedef struct ME_VERSION_ {
    UINT32 Signature;
    UINT32 Reserved;
    UINT16 Major;
    UINT16 Minor;
    UINT16 Bugfix;
    UINT16 Build;
} ME_VERSION;

#define ME_VERSION_SIGNATURE 0x4E414D24 //$MAN
#define ME_VERSION_SIGNATURE2  0x324E4D24 //$MN2

// FPT
#define ME_ROM_BYPASS_VECTOR_SIZE 0x10
#define FPT_HEADER_SIGNATURE 0x54504624 //$FPT

// Header version 1.0 or 2.0, default
typedef struct FPT_HEADER_ {
    UINT32 Signature;
    UINT32 NumEntries;
    UINT8  HeaderVersion;  // 0x10 or 0x20
    UINT8  EntryVersion;
    UINT8  HeaderLength;
    UINT8  HeaderChecksum; // One bit for Redundant
    UINT16 FlashCycleLife; // Maybe also TicksToAdd
    UINT16 FlashCycleLimit;// Maybe also TokensToAdd
    UINT32 UmaSize;        // Maybe also Flags
    UINT32 Flags;          // Maybe also FlashLayout
    UINT16 FitcMajor;
    UINT16 FitcMinor;
    UINT16 FitcHotfix;
    UINT16 FitcBuild;
} FPT_HEADER;

// Header version 2.1, special case
#define FPT_HEADER_VERSION_21 0x21
typedef struct FPT_HEADER_21_ {
    UINT32 Signature;
    UINT32 NumEntries;
    UINT8  HeaderVersion;  // 0x21
    UINT8  EntryVersion;
    UINT8  HeaderLength;
    UINT8  Flags;          // One bit for Redundant
    UINT16 TicksToAdd;
    UINT16 TokensToAdd;
    UINT32 SPSFlags;
    UINT32 HeaderCrc32;    // Header + Entries sums to 0
    UINT16 FitcMajor;
    UINT16 FitcMinor;
    UINT16 FitcHotfix;
    UINT16 FitcBuild;
} FPT_HEADER_21;

typedef struct FPT_HEADER_ENTRY_{
    CHAR8  Name[4];
    CHAR8  Owner[4];
    UINT32 Offset;
    UINT32 Size;
    UINT32 Reserved[3];
    UINT8  Type             : 7;
    UINT8  CopyToDramCache  : 1;
    UINT8  Reserved1        : 7;
    UINT8  BuiltWithLength1 : 1;
    UINT8  BuiltWithLength2 : 1;
    UINT8  Reserved2        : 7;
    UINT8  EntryValid;
} FPT_HEADER_ENTRY;

#define BOOT_PARTITION_NUM  5

// IFWI
typedef struct IFWI_HEADER_ENTRY_ {
    UINT32 Offset;
    UINT32 Size;
} IFWI_HEADER_ENTRY;

// IFWI 1.6 (ME), 2.0 (BIOS)
typedef struct IFWI_16_LAYOUT_HEADER_ {
    UINT8             RomBypassVector[16];
    IFWI_HEADER_ENTRY DataPartition;
    IFWI_HEADER_ENTRY BootPartition[BOOT_PARTITION_NUM];
    UINT64            Checksum;
} IFWI_16_LAYOUT_HEADER;

// IFWI 1.7 (ME)
typedef struct IFWI_17_LAYOUT_HEADER_ {
    UINT8  RomBypassVector[16];
    UINT16 HeaderSize;
    UINT8  Flags;
    UINT8  Reserved;
    UINT32 Checksum;
    IFWI_HEADER_ENTRY DataPartition;
    IFWI_HEADER_ENTRY BootPartition[BOOT_PARTITION_NUM];
    IFWI_HEADER_ENTRY TempPage;
} IFWI_17_LAYOUT_HEADER;

#define ME_MANIFEST_HEADER_ID 0x324E4D24 //$MN2

// Restore previous packing rules
#pragma pack()

//*****************************************************************************
// IFWI
//*****************************************************************************

// BPDT
#define BPDT_GREEN_SIGNATURE  0x000055AA
#define BPDT_YELLOW_SIGNATURE 0x00AA55AA

typedef struct BPDT_HEADER_ {
    UINT32 Signature;
    UINT16 NumEntries;
    UINT8  HeaderVersion;
    UINT8  RedundancyFlag; // Reserved zero in version 1
    UINT32 Checksum;
    UINT32 IfwiVersion;
    UINT16 FitcMajor;
    UINT16 FitcMinor;
    UINT16 FitcHotfix;
    UINT16 FitcBuild;
} BPDT_HEADER;

#define BPDT_HEADER_VERSION_1 1
#define BPDT_HEADER_VERSION_2 2

typedef struct BPDT_ENTRY_ {
    UINT32 Type : 16;
    UINT32 SplitSubPartitionFirstPart : 1;
    UINT32 SplitSubPartitionSecondPart : 1;
    UINT32 CodeSubPartition : 1;
    UINT32 UmaCachable : 1;
    UINT32 Reserved: 12;
    UINT32 Offset;
    UINT32 Size;
} BPDT_ENTRY;

// https://github.com/platomav/MEAnalyzer/blob/master/MEA.py#L10595
#define BPDT_ENTRY_TYPE_SMIP        0
#define BPDT_ENTRY_TYPE_RBEP        1
#define BPDT_ENTRY_TYPE_FTPR        2
#define BPDT_ENTRY_TYPE_UCOD        3
#define BPDT_ENTRY_TYPE_IBBP        4
#define BPDT_ENTRY_TYPE_S_BPDT      5
#define BPDT_ENTRY_TYPE_OBBP        6
#define BPDT_ENTRY_TYPE_NFTP        7
#define BPDT_ENTRY_TYPE_ISHC        8
#define BPDT_ENTRY_TYPE_DLMP        9
#define BPDT_ENTRY_TYPE_UEBP        10
#define BPDT_ENTRY_TYPE_UTOK        11
#define BPDT_ENTRY_TYPE_UFS_PHY     12
#define BPDT_ENTRY_TYPE_UFS_GPP_LUN 13
#define BPDT_ENTRY_TYPE_PMCP        14
#define BPDT_ENTRY_TYPE_IUNP        15
#define BPDT_ENTRY_TYPE_NVMC        16
#define BPDT_ENTRY_TYPE_UEP         17
#define BPDT_ENTRY_TYPE_WCOD        18
#define BPDT_ENTRY_TYPE_LOCL        19
#define BPDT_ENTRY_TYPE_OEMP        20
#define BPDT_ENTRY_TYPE_FITC        21
#define BPDT_ENTRY_TYPE_PAVP        22
#define BPDT_ENTRY_TYPE_IOMP        23
#define BPDT_ENTRY_TYPE_XPHY        24
#define BPDT_ENTRY_TYPE_TBTP        25
#define BPDT_ENTRY_TYPE_PLTS        26
#define BPDT_ENTRY_TYPE_RES27       27
#define BPDT_ENTRY_TYPE_RES28       28
#define BPDT_ENTRY_TYPE_RES29       29
#define BPDT_ENTRY_TYPE_RES30       30
#define BPDT_ENTRY_TYPE_DPHY        31
#define BPDT_ENTRY_TYPE_PCHC        32
#define BPDT_ENTRY_TYPE_ISIF        33
#define BPDT_ENTRY_TYPE_ISIC        34
#define BPDT_ENTRY_TYPE_HBMI        35
#define BPDT_ENTRY_TYPE_OMSM        36
#define BPDT_ENTRY_TYPE_GTGP        37
#define BPDT_ENTRY_TYPE_MDFI        38
#define BPDT_ENTRY_TYPE_PUNP        39
#define BPDT_ENTRY_TYPE_PHYP        40
#define BPDT_ENTRY_TYPE_SAMF        41
#define BPDT_ENTRY_TYPE_PPHY        42
#define BPDT_ENTRY_TYPE_GBST        43
#define BPDT_ENTRY_TYPE_TCCP        44
#define BPDT_ENTRY_TYPE_PSEP        45
#define BPDT_ENTRY_TYPE_ESE         46
#define BPDT_ENTRY_TYPE_ACE         50
#define BPDT_ENTRY_TYPE_SPHY        54

#endif // ME_H
