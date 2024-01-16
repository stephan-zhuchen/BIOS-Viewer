//
// Created by stephan on 9/2/2023.
//

#include "DataModel.h"
#include "BaseLib.h"
#include "UefiFileSystem/CommonSection.h"
#include "UefiFileSystem/FfsFile.h"
#include "UefiFileSystem/FirmwareVolume.h"
#include "UefiFileSystem/NvVariable.h"
#include "UEFI/GuidDatabase.h"

using namespace BaseLibrarySpace;

DataModel::DataModel(Volume *vol, QString nm, QString typ, QString sbtyp):
        name(std::move(nm)),
        type(std::move(typ)),
        subtype(std::move(sbtyp)),
        modelData(vol) { }

void DataModel::setSectionModel(Volume *sec) {
    if (sec->getVolumeType() != VolumeType::CommonSection) {
        throw BiosException("");
    }
    auto *section = (CommonSection*)sec;
    type = "Section";
    switch (section->getSectionType()) {
        case EFI_SECTION_COMPRESSION:
            name = "Compressed Section";
            subtype = "Compressed";
            break;
        case EFI_SECTION_GUID_DEFINED:
            EFI_GUID guid;
            guid = section->getSectionDefinitionGuid();
            name = QString::fromStdString(guidData->getNameFromGuid(guid, true));
            subtype = "GUID defined";
            break;
        case EFI_SECTION_DISPOSABLE:
            name = "Disposable Section";
            subtype = "Disposable";
            break;
        case EFI_SECTION_PE32:
            if (section->peCoffHeader->isPe32Plus){
                name = "PE32+ Image Section";
                subtype = "PE32+ image";
            } else {
                name = "PE32 Image Section";
                subtype = "PE32 image";
            }
            break;
        case EFI_SECTION_PIC:
            name = "PIC Section";
            subtype = "PIC";
            break;
        case EFI_SECTION_TE:
            name = "TE Image Section";
            subtype = "TE image";
            break;
        case EFI_SECTION_DXE_DEPEX:
            name = "DXE dependency Section";
            subtype = "DXE dependency";
            break;
        case EFI_SECTION_VERSION:
            name = "Version Section";
            subtype = "Version";
            break;
        case EFI_SECTION_USER_INTERFACE:
            name = "UI Section";
            subtype = "UI";
            break;
        case EFI_SECTION_COMPATIBILITY16:
            name = "Compatibility16 Section";
            subtype = "Compatibility16";
            break;
        case EFI_SECTION_FIRMWARE_VOLUME_IMAGE:
            name = "Volume Image Section";
            subtype = "Volume Image";
            break;
        case EFI_SECTION_FREEFORM_SUBTYPE_GUID:
            name = QString::fromStdString(guidData->getNameFromGuid(section->getSubTypeGuid(), true));
            subtype = "Freeform GUID";
            break;
        case EFI_SECTION_RAW:
            name = "Raw Section";
            subtype = "Raw";
            break;
        case EFI_SECTION_PEI_DEPEX:
            name = "PEI dependency Section";
            subtype = "PEI dependency";
            break;
        case EFI_SECTION_MM_DEPEX:
            name = "SMM dependency Section";
            subtype = "SMM dependency";
            break;
        default:
            break;
    }
}

void DataModel::setFfsModel(Volume *file) {
    auto ffs = (FfsFile*)file;
    name = QString::fromStdString(guidData->getNameFromGuid(ffs->getFfsGuid(), true));
    type = "File";

    switch (ffs->getType()) {
        case EFI_FV_FILETYPE_RAW:
            subtype = "Raw";
            break;
        case EFI_FV_FILETYPE_FREEFORM:
            subtype = "Free Form";
            break;
        case EFI_FV_FILETYPE_SECURITY_CORE:
            subtype = "SEC Core";
            break;
        case EFI_FV_FILETYPE_PEI_CORE:
            subtype = "PEI Core";
            break;
        case EFI_FV_FILETYPE_DXE_CORE:
            subtype = "DXE Core";
            break;
        case EFI_FV_FILETYPE_PEIM:
            subtype = "PEIM";
            break;
        case EFI_FV_FILETYPE_DRIVER:
            subtype = "DXE Driver";
            break;
        case EFI_FV_FILETYPE_COMBINED_PEIM_DRIVER:
            subtype = "PEIM Driver";
            break;
        case EFI_FV_FILETYPE_APPLICATION:
            subtype = "Application";
            break;
        case EFI_FV_FILETYPE_MM:
            subtype = "SMM module";
            break;
        case EFI_FV_FILETYPE_FIRMWARE_VOLUME_IMAGE:
            subtype = "Volume image";
            break;
        case EFI_FV_FILETYPE_COMBINED_MM_DXE:
            subtype = "SMM DXE";
            break;
        case EFI_FV_FILETYPE_MM_CORE:
            subtype = "SMM Core";
            break;
        case EFI_FV_FILETYPE_MM_STANDALONE:
            subtype = "SMM Standalone";
            break;
        case EFI_FV_FILETYPE_MM_CORE_STANDALONE:
            subtype = "SMM Core Standalone";
            break;
//    case EFI_FV_FILETYPE_OEM_MIN ... EFI_FV_FILETYPE_OEM_MAX:
//        subtype = "OEM";
//        break;
//    case EFI_FV_FILETYPE_DEBUG_MIN ... EFI_FV_FILETYPE_DEBUG_MAX:
//        subtype = "Debug";
//        break;
//    case EFI_FV_FILETYPE_FFS_MIN ... EFI_FV_FILETYPE_FFS_MAX:
//        subtype = "FFS";
//        break;
        case EFI_FV_FILETYPE_FFS_PAD:
            name = "Pad file";
            subtype = "Pad";
            break;
        default:
            subtype = "";
            break;
    }
}

void DataModel::setFirmwareVolumeModel(Volume *vol) {
    auto fv = (FirmwareVolume*)vol;
    name = QString::fromStdString(guidData->getNameFromGuid(fv->getFvGuid(), true));
    type = "Volume";

    EFI_GUID guid = fv->getFvGuid(false);
    if (guid == GuidDatabase::gEfiFirmwareFileSystem2Guid) {
        subtype = "FFSv2";
    } else if (guid == GuidDatabase::gEfiFirmwareFileSystem3Guid) {
        subtype = "FFSv3";
    } else if (guid == GuidDatabase::gEfiSystemNvDataFvGuid) {
        subtype = "NVRAM";
    }
}

void DataModel::setNvVariableHeaderModel(Volume *var) {
    name = "Nv Storage";
}

void DataModel::setNvVariableEntryModel(Volume *entry) {
    auto NvEntry = (NvVariableEntry*)entry;
    name = QString::fromStdString(NvEntry->VariableName);
    type = "Variable";
}

void DataModel::InitFromVolume(Volume *vol) {
    modelData = vol;
    name.clear();
    type.clear();
    subtype.clear();
    VolumeType volType = vol->getVolumeType();
    switch (volType) {
        case VolumeType::Overview:
            name = "Overview";
            type = "Image";
            break;
        case VolumeType::FirmwareVolume:
            setFirmwareVolumeModel(vol);
            break;
        case VolumeType::FfsFile:
            setFfsModel(vol);
            break;
        case VolumeType::CommonSection:
            setSectionModel(vol);
            break;
        case VolumeType::ELF:
            name = "ELF";
            type = "Section";
            subtype = "Raw";
            break;
        case VolumeType::Apriori:
            name = "Apriori";
            break;
        case VolumeType::FspHeader:
            name = "FSP Header";
            type = "File";
            subtype = "Raw";
            break;
        case VolumeType::AcpiTable:
            name = "ACPI Table";
            type = "Section";
            subtype = "Raw";
            break;
        case VolumeType::Empty:
            name = "Empty Volume";
            break;
        case VolumeType::Other:
            name = "Empty Volume";
            break;
        case VolumeType::FlashDescriptor:
            name = "Flash Descriptor";
            type = "Region";
            break;
        case VolumeType::EC:
            name = "EC";
            type = "Region";
            break;
        case VolumeType::GbE:
            name = "GbE";
            type = "Region";
            break;
        case VolumeType::ME:
            name = "CSE";
            type = "Region";
            break;
        case VolumeType::OSSE:
            name = "OSSE";
            type = "Region";
            break;
        case VolumeType::BIOS:
            name = "BIOS";
            type = "Region";
            break;
        case VolumeType::UplInfo:
            name = "Universal Payload Info";
            break;
        case VolumeType::NvStorage:
            name = "NV Storage";
            break;
        case VolumeType::UserDefined:
            if (vol->getUserDefinedName().size() >= 1) {
                name = vol->getUserDefinedName()[0];
            }
            if (vol->getUserDefinedName().size() >= 2) {
                type = vol->getUserDefinedName()[1];
            }
            if (vol->getUserDefinedName().size() >= 3) {
                subtype = vol->getUserDefinedName()[2];
            }
            break;
        case VolumeType::FaultTolerantBlock:
            name = "Fault Tolerant Working Block";
            break;
        case VolumeType::CapsuleCommonHeader:
            name = "Capsule Common Header";
            break;
        case VolumeType::FirmwareManagementHeader:
            name = "Firmware Management Header";
            break;
        case VolumeType::IniConfig:
            name = "Ini Config File";
            type = "File";
            subtype = "Raw";
            break;
        case VolumeType::BiosGuardPackage:
            name = "BGUP";
            type = "File";
            subtype = "Raw";
            break;
        case VolumeType::BtgAcm:
            name = "Startup Acm";
            type = "File";
            subtype = "Raw";
            break;
        case VolumeType::Microcodeversion:
            name = "Microcode Version";
            break;
        case VolumeType::Microcode:
            name = "Microcode";
            type = "File";
            subtype = "Raw";
            break;
        case VolumeType::IshPdt:
            name = "ISH PDT";
            break;
        case VolumeType::Vpd:
            name = "VPD Region";
            type = "Section";
            subtype = "Raw";
            break;
    }
    if (vol->getUniqueVolumeName().size() > 0) {
        name = vol->getUniqueVolumeName();
    }
}
