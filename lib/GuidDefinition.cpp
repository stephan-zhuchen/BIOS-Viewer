#include "../include/GuidDefinition.h"
#include "BaseLib.h"

bool EFI_GUID::operator==(EFI_GUID guid) {

    if (this->Data1 != guid.Data1) {
        return false;
    } else if (this->Data2 != guid.Data2) {
        return false;
    } else if (this->Data3 != guid.Data3) {
        return false;
    }
    for (int var = 0; var < 8; ++var) {
        if (this->Data4[var] != guid.Data4[var]) {
            return false;
        }
    }
    return true;
}

std::string GuidDatabase::getNameFromGuid(EFI_GUID guid) {
    std::string name;
    switch (guid.Data1) {
    case gFvValidationUncompactGuid.Data1:
        name = "FvValidationUncompact";
        break;
    case gFvTestMenuFvGuid.Data1:
        name = "FvTestMenu";
        break;
    case gFvAdvancedFvGuid.Data1:
        name = "FvAdvanced";
        break;
    case gFvAdvancedUncompactGuid.Data1:
        name = "FvAdvancedUncompact";
        break;
    case gFvTrustedDeviceSetupUnCompactGuid.Data1:
        name = "FvTrustedDeviceSetupUnCompact";
        break;
    case gFvOsBootFvGuid.Data1:
        name = "FvOsBoot";
        break;
    case gFvOsBootUncompactGuid.Data1:
        name = "FvOsBootUncompact";
        break;
    case gFvLateSiliconGuid.Data1:
        name = "FvLateSilicon";
        break;
    case gFvUefiBootFvGuid.Data1:
        name = "FvUefiBoot";
        break;
    case gFvUefiBootUncompactGuid.Data1:
        name = "FvUefiBootUncompact";
        break;
    case gFvOptionalFvGuid.Data1:
        name = "FvOptional";
        break;
    case gFvCnvUnCompactGuid.Data1:
        name = "FvCnvUnCompact";
        break;
    case gFvWifiDxeUnCompactGuid.Data1:
        name = "FvWifiDxeUnCompact";
        break;
    case gFvBlueToothDxeUnCompactGuid.Data1:
        name = "FvBlueToothDxeUnCompact";
        break;
    case gFvNetworkDxeUnCompactGuid.Data1:
        name = "FvNetworkDxeUnCompact";
        break;
    case gFvAppUncompactGuid.Data1:
        name = "FvAppUncompact";
        break;
    case gFvSecurityFvNameGuid.Data1:
        name = "FvSecurity";
        break;
    case gFvSecurityLateGuid.Data1:
        name = "FvSecurityLate";
        break;
    case gFirmwareBinariesFvGuid.Data1:
        name = "FvFirmwareBinaries";
        break;
    case gFvFwBinariesInternalOnlyUncompactGuid.Data1:
        name = "FvFwBinariesInternalOnlyUncompact";
        break;
    case gFvPostMemoryFvGuid.Data1:
        name = "FvPostMemory";
        break;
    case gFvPostMemoryUncompactGuid.Data1:
        name = "FvPostMemoryUncompact";
        break;
    case gFspSiliconFvGuid.Data1:
        name = "FSP-S";
        break;
    case gFspMemoryFvGuid.Data1:
        name = "FSP-M";
        break;
    case gFspTempRamFvGuid.Data1:
        name = "FSP-T";
        break;
    case gFvPreMemoryFvGuid.Data1:
        name = "FvPreMemory";
        break;
    case gFvSecurityPreMemoryGuid.Data1:
        name = "FvSecurityPreMemory";
        break;
    case gFvSecurityPostMemoryGuid.Data1:
        name = "FvSecurityPostMemory";
        break;
    case gFvCryptoPeiGuid.Data1:
        name = "FvCryptoPei";
        break;
    case gFvCryptoDxeUnCompactGuid.Data1:
        name = "FvCryptoDxeUnCompact";
        break;
    case gFvExtendedPostMemoryFvNameGuid.Data1:
        name = "FvExtendedPostMemory";
        break;
    case gFvExtendedPostMemoryUncompactGuid.Data1:
        name = "FvExtendedPostMemoryUncompact";
        break;
    case gFvExtendedAdvancedFvNameGuid.Data1:
        name = "FvExtendedAdvanced";
        break;
    case gFvExtendedAdvancedUncompactGuid.Data1:
        name = "FvExtendedAdvancedUncompact";
        break;

    // FFS GUID
    case gEfiCertTypeRsa2048Sha256Guid.Data1:
        name = "EfiCertTypeRsa2048Sha256Guid";
        break;
    case gLzmaCustomDecompressGuid.Data1:
        name = "LzmaCustomDecompressGuid";
        break;
    case gPeiAprioriFileNameGuid.Data1:
        name = "Pei Apriori";
        break;
    case gAprioriGuid.Data1:
        name = "Dxe Apriori";
        break;
    case gIntelMicrocodeVersionFfsBinGuid.Data1:
        name = "Microcode Version";
        break;
    case gIntelMicrocodeArrayFfsBinGuid.Data1:
        name = "Microcode";
        break;
    case gBiosGuardModuleBinGuid.Data1:
        name = "Bios Guard Module";
        break;
    case gBiosGuardModuleSimBinGuid.Data1:
        name = "Bios Guard Module Sim";
        break;
    case gStartupAcmPeiBinGuid.Data1:
        name = "Startup Acm";
        break;
    case gObbSha256HashBinGuid.Data1:
        name = "Obb Sha256 Hash";
        break;
    case gObbRSha256HashBinGuid.Data1:
        name = "ObbR Sha256 Hash";
        break;
    case gIbbRBgslBinGuid.Data1:
        name = "IbbR Bgsl";
        break;
    case gObbRBgslBinGuid.Data1:
        name = "ObbR Bgsl";
        break;
    case gBiosIdGuid.Data1:
        name = "Bios ID";
        break;
    case gFspHeaderFileGuid.Data1:
        name = "Fsp Header";
        break;
    case gFspSecCoreTGuid.Data1:
        name = "FspSecCoreT";
        break;
    case gTianoLogoGuid.Data1:
        name = "Tiano Logo";
        break;
    default:
        name = BaseLibrarySpace::GUID(guid).str(true);
        break;
    }
    return name;
}
