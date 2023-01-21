#pragma once

#include <string>
#include "SymbolDefinition.h"

#define VNAME(name) (#name)

///
/// WIN_CERTIFICATE_UEFI_GUID.CertType
///

#define EFI_FIRMWARE_FILE_SYSTEM2_GUID \
  { 0x8c8ce578, 0x8a3d, 0x4f1c, { 0x99, 0x35, 0x89, 0x61, 0x85, 0xc3, 0x2d, 0xd3 } }

#define EFI_FIRMWARE_FILE_SYSTEM3_GUID \
  { 0x5473c07a, 0x3dcb, 0x4dca, { 0xbd, 0x6f, 0x1e, 0x96, 0x89, 0xe7, 0x34, 0x9a }}

class GuidDatabase {
public:
    static std::string getNameFromGuid(EFI_GUID guid);

    // FV Name GUID
    constexpr static EFI_GUID gEfiSystemNvDataFvGuid           = {0xFFF12B8D, 0x7696, 0x4C8B, {0xA9, 0x85, 0x27, 0x47, 0x07, 0x5B, 0x4F, 0x50}};

    constexpr static EFI_GUID gFvValidationUncompactGuid       = {0x3EAFA9F8, 0xE5FE, 0x46B7, {0x8E, 0xD0, 0xAB, 0x9F, 0xB1, 0x9A, 0x18, 0x76}};

    constexpr static EFI_GUID gFvTestMenuFvGuid                = {0xEE49E761, 0xCFC3, 0x4567, {0x96, 0xC9, 0xE1, 0xD5, 0x77, 0xAE, 0xFE, 0xF2}};

    constexpr static EFI_GUID gFvAdvancedFvGuid                = {0xB23E7388, 0x9953, 0x45C7, {0x92, 0x01, 0x04, 0x73, 0xDD, 0xE5, 0x48, 0x7A}};
    constexpr static EFI_GUID gFvAdvancedUncompactGuid         = {0x8461A948, 0xA2FC, 0x4977, {0xB8, 0x28, 0x21, 0x76, 0x33, 0xC0, 0x69, 0x99}};
    constexpr static EFI_GUID gFvTrustedDeviceSetupUnCompactGuid = {0x30F01DDB, 0x2B1E, 0x49EF, {0x85, 0x81, 0x51, 0x6D, 0x98, 0xB8, 0xDE, 0x27}};

    constexpr static EFI_GUID gFvOsBootFvGuid                  = {0x13BF8810, 0x75FD, 0x4B1A, {0x91, 0xE6, 0xE1, 0x6C, 0x42, 0x01, 0xF8, 0x0A}};
    constexpr static EFI_GUID gFvOsBootUncompactGuid           = {0xA0F04529, 0xB715, 0x44C6, {0xBC, 0xA4, 0x2D, 0xEB, 0xDD, 0x01, 0xEE, 0xEC}};
    constexpr static EFI_GUID gFvLateSiliconGuid               = {0x97F09B89, 0x9E83, 0x4DDC, {0xA3, 0xD1, 0x10, 0xC4, 0xAF, 0x53, 0x9D, 0x1E}};

    constexpr static EFI_GUID gFvUefiBootFvGuid                = {0x0496D33D, 0xEA79, 0x495C, {0xB6, 0x5D, 0xAB, 0xF6, 0x07, 0x18, 0x4E, 0x3B}};
    constexpr static EFI_GUID gFvUefiBootUncompactGuid         = {0xA881D567, 0x6CB0, 0x4EEE, {0x84, 0x35, 0x2E, 0x72, 0xD3, 0x3E, 0x45, 0xB5}};

    constexpr static EFI_GUID gFvOptionalFvGuid                = {0x9574B1CE, 0xEE93, 0x451E, {0xB5, 0x00, 0x3E, 0x5F, 0x56, 0x42, 0x44, 0xDE}};
    constexpr static EFI_GUID gFvCnvUnCompactGuid              = {0xB92CF322, 0x8AFA, 0x4AA4, {0xB9, 0x46, 0x00, 0x5D, 0xF1, 0xD6, 0x97, 0x79}};
    constexpr static EFI_GUID gFvWifiDxeUnCompactGuid          = {0x33C702B0, 0x459E, 0x4AB8, {0x90, 0x14, 0x49, 0x3B, 0x9A, 0x99, 0xBF, 0x26}};
    constexpr static EFI_GUID gFvBlueToothDxeUnCompactGuid     = {0x115e059e, 0x2cc1, 0x4954, {0xb7, 0x39, 0x7f, 0xa7, 0x46, 0xac, 0x8e, 0x4c}};
    constexpr static EFI_GUID gFvNetworkDxeUnCompactGuid       = {0x830c49df, 0x3d0c, 0x4d56, {0xa3, 0xa6, 0x25, 0xab, 0x36, 0x4e, 0xfd, 0x39}};
    constexpr static EFI_GUID gFvAppUncompactGuid              = {0xCE2FCBB0, 0x8662, 0x4E26, {0xA9, 0xEA, 0xB7, 0xEA, 0x54, 0x18, 0x21, 0x2E}};

    constexpr static EFI_GUID gFvSecurityFvNameGuid            = {0xB58325AF, 0x77D0, 0x4A3B, {0xBB, 0xBB, 0x11, 0x05, 0xAB, 0x84, 0x97, 0xCA}};
    constexpr static EFI_GUID gFvSecurityLateGuid              = {0xF753FE9A, 0xEEFD, 0x485B, {0x84, 0x0B, 0xE0, 0x32, 0xD5, 0x38, 0x10, 0x2C}};

    constexpr static EFI_GUID gFirmwareBinariesFvGuid          = {0x8B98AB22, 0xE354, 0x42f0, {0x88, 0xB9, 0x04, 0x98, 0x10, 0xF0, 0xFD, 0xAA}};
    constexpr static EFI_GUID gFvFwBinariesInternalOnlyUncompactGuid = {0x8882EF3F, 0x6F21, 0x40F2, {0xA6, 0x96, 0xE5, 0xCF, 0xE0, 0x14, 0x9B, 0xFE}};

    constexpr static EFI_GUID gFvPostMemoryFvGuid              = {0x9DFE49DB, 0x8EF0, 0x4D9C, {0xB2, 0x73, 0x00, 0x36, 0x14, 0x4D, 0xE9, 0x17}};
    constexpr static EFI_GUID gFvPostMemoryUncompactGuid       = {0x7C4DCFC6, 0xAECA, 0x4707, {0x85, 0xB9, 0xFD, 0x4B, 0x2E, 0xEA, 0x49, 0xE7}};

    constexpr static EFI_GUID gFspSiliconFvGuid                = {0x1B5C27FE, 0xF01C, 0x4fbc, {0xAE, 0xAE, 0x34, 0x1B, 0x2E, 0x99, 0x2A, 0x17}};
    constexpr static EFI_GUID gFspMemoryFvGuid                 = {0x52F1AFB6, 0x78A6, 0x448f, {0x82, 0x74, 0xF3, 0x70, 0x54, 0x9A, 0xC5, 0xD0}};
    constexpr static EFI_GUID gFspTempRamFvGuid                = {0x7BEBD21A, 0xA1E5, 0x4C4C, {0x9C, 0xA1, 0xA0, 0xC1, 0x68, 0xBC, 0xBD, 0x9D}};

    constexpr static EFI_GUID gFvPreMemoryFvGuid               = {0xFC8FE6B5, 0xCD9B, 0x411E, {0xBD, 0x8F, 0x31, 0x82, 0x4D, 0x0C, 0xDE, 0x3D}};
    constexpr static EFI_GUID gFvSecurityPreMemoryGuid         = {0x9B7FA59D, 0x71C6, 0x4A36, {0x90, 0x6E, 0x97, 0x25, 0xEA, 0x6A, 0xDD, 0x5B}};
    constexpr static EFI_GUID gFvSecurityPostMemoryGuid        = {0x4199E560, 0x54AE, 0x45E5, {0x91, 0xA4, 0xF7, 0xBC, 0x38, 0x04, 0xE1, 0x4A}};
    constexpr static EFI_GUID gFvCryptoPeiGuid                 = {0x707b1736, 0xd03f, 0x489a, {0xbf, 0xe7, 0xd6, 0x57, 0x10, 0x40, 0x1a, 0xcf}};
    constexpr static EFI_GUID gFvCryptoDxeUnCompactGuid        = {0xa90774e1, 0xc2ba, 0x4d6b, {0xa0, 0x16, 0xd3, 0x50, 0x64, 0x41, 0x4a, 0x53}};

    constexpr static EFI_GUID gFvExtendedPostMemoryFvNameGuid  = {0x21D78FA8, 0xB8E5, 0x4574, {0x9A, 0x67, 0xC0, 0x5B, 0x53, 0x29, 0x69, 0x36}};
    constexpr static EFI_GUID gFvExtendedPostMemoryUncompactGuid = {0x61141985, 0xF036, 0x4E0C, {0x9B, 0x39, 0x84, 0x1B, 0x7E, 0x39, 0xA5, 0x2F}};

    constexpr static EFI_GUID gFvExtendedAdvancedFvNameGuid    = {0xD18A7412, 0xE2A8, 0x4A45, {0x93, 0xB8, 0xAF, 0x97, 0x4D, 0xFC, 0x75, 0x99}};
    constexpr static EFI_GUID gFvExtendedAdvancedUncompactGuid = {0xC8EC1BED, 0xD6C4, 0x4263, {0x99, 0x80, 0xC1, 0x41, 0xA8, 0x57, 0x4D, 0x28}};

    // FFS Name GUID
    constexpr static EFI_GUID gEfiCertTypeRsa2048Sha256Guid    = { 0xa7717414, 0xc616, 0x4977, { 0x94, 0x20, 0x84, 0x47, 0x12, 0xa7, 0x35, 0xbf }};
    constexpr static EFI_GUID gLzmaCustomDecompressGuid        = { 0xEE4E5898, 0x3914, 0x4259, { 0x9D, 0x6E, 0xDC, 0x7B, 0xD7, 0x94, 0x03, 0xCF }};
    constexpr static EFI_GUID gPeiAprioriFileNameGuid          = { 0x1b45cc0a, 0x156a, 0x428a, { 0XAF, 0x62, 0x49, 0x86, 0x4d, 0xa0, 0xe6, 0xe6 }};
    constexpr static EFI_GUID gAprioriGuid                     = { 0xFC510EE7, 0xFFDC, 0x11D4, { 0xBD, 0x41, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81 }};
    constexpr static EFI_GUID gEfiFirmwareFileSystem2Guid      = { 0x8c8ce578, 0x8a3d, 0x4f1c, { 0x99, 0x35, 0x89, 0x61, 0x85, 0xc3, 0x2d, 0xd3 }};
    constexpr static EFI_GUID gEfiFirmwareFileSystem3Guid      = { 0x5473c07a, 0x3dcb, 0x4dca, { 0xbd, 0x6f, 0x1e, 0x96, 0x89, 0xe7, 0x34, 0x9a }};
    constexpr static EFI_GUID gIntelMicrocodeVersionFfsBinGuid = { 0xE0562501, 0xB41B, 0x4566, { 0xAC, 0x0F, 0x7E, 0xA8, 0xEE, 0x81, 0x7F, 0x20 }};
    constexpr static EFI_GUID gIntelMicrocodeArrayFfsBinGuid   = { 0x197DB236, 0xF856, 0x4924, { 0x90, 0xF8, 0xCD, 0xF1, 0x2F, 0xB8, 0x75, 0xF3 }};
    constexpr static EFI_GUID gBiosGuardModuleBinGuid          = { 0x7934156d, 0xcfce, 0x460e, { 0x92, 0xf5, 0xa0, 0x79, 0x09, 0xa5, 0x9e, 0xca }};
    constexpr static EFI_GUID gBiosGuardModuleSimBinGuid       = { 0x2cbc10e3, 0x8cd3, 0x442b, { 0x8b, 0x61, 0xd9, 0xa6, 0xff, 0xa7, 0xcb, 0xe7 }};
    constexpr static EFI_GUID gStartupAcmPeiBinGuid            = { 0x26FDAA3D, 0xB7ED, 0x4714, { 0x85, 0x09, 0xEE, 0xCF, 0x15, 0x93, 0x80, 0x0D }};
    constexpr static EFI_GUID gObbSha256HashBinGuid            = { 0xf57757fc, 0x2603, 0x404f, { 0xaa, 0xe2, 0x34, 0xc6, 0x23, 0x23, 0x88, 0xe8 }};
    constexpr static EFI_GUID gObbRSha256HashBinGuid           = { 0x7ed53efb, 0xbb3d, 0x4066, { 0xb3, 0xed, 0x82, 0x62, 0xe2, 0xd2, 0x1e, 0x70 }};
    constexpr static EFI_GUID gIbbRBgslBinGuid                 = { 0xf53fc14b, 0x025c, 0x4477, { 0x9b, 0x48, 0x7a, 0x1b, 0x19, 0xf8, 0x0f, 0x30 }};
    constexpr static EFI_GUID gObbRBgslBinGuid                 = { 0x318d30b7, 0xf669, 0x4af2, { 0xad, 0xe1, 0xe3, 0xf8, 0x4d, 0x09, 0x7b, 0xb3 }};
    constexpr static EFI_GUID gBiosIdGuid                      = { 0xC3E36D09, 0x8294, 0x4b97, { 0xA8, 0x57, 0xD5, 0x28, 0x8F, 0xE3, 0x3E, 0x28 }};
    constexpr static EFI_GUID gFspHeaderFileGuid               = { 0x912740BE, 0x2284, 0x4734, { 0xB9, 0x71, 0x84, 0xB0, 0x27, 0x35, 0x3F, 0x0C }};
    constexpr static EFI_GUID gFspSecCoreTGuid                 = { 0x5B94E419, 0xC795, 0x414D, { 0xA0, 0xD4, 0xB8, 0x0A, 0x87, 0x7B, 0xE5, 0xFE }};

    constexpr static EFI_GUID gTianoLogoGuid                   = { 0x7BB28B99, 0x61BB, 0x11D5, { 0x9A, 0x5D, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D }};
};