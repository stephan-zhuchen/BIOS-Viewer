#include "Model.h"

#include <utility>
#include "UEFI/GuidDefinition.h"

DataModel::DataModel(Volume* model, QString nm, QString typ, QString sbtyp, QString txt, bool recycle):
    name(std::move(nm)),
    type(std::move(typ)),
    subtype(std::move(sbtyp)),
    text(std::move(txt)),
    needRecycle(recycle),
    modelData(model)
{}

DataModel::~DataModel() {
    for (auto volumeModel:volumeModelData) {
        delete volumeModel;
    }
    if (needRecycle) {
        delete modelData;
    }
}

void DataModel::setName(QString txt) {
    name = std::move(txt);
}

void DataModel::setType(QString txt) {
    type = std::move(txt);
}

void DataModel::setSubtype(QString txt) {
    subtype = std::move(txt);
}

void DataModel::setText(QString txt) {
    text = std::move(txt);
}

QString DataModel::getName() const {
    return name;
}

QString DataModel::getText() const {
    return text;
}

QString DataModel::getType() const {
    return type;
}

QString DataModel::getSubType() const {
    return subtype;
}

QStringList DataModel::getData() const {
    return QStringList() << name << type << subtype;
}

SectionModel::SectionModel(CommonSection *section, FfsModel *parent) {
    parentModel = parent;
    modelData = section;
    type = "Section";
    text = "";
    switch (section->CommonHeader.Type) {
    case EFI_SECTION_COMPRESSION:
        name = "Compressed Section";
        subtype = "Compressed";
        break;
    case EFI_SECTION_GUID_DEFINED:
        EFI_GUID guid;
        guid = section->SectionDefinitionGuid;
        name = QString::fromStdString(guidData->getNameFromGuid(guid));
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
        text = QString::fromStdString(section->VersionString);
        break;
    case EFI_SECTION_USER_INTERFACE:
        name = "UI Section";
        subtype = "UI";
        text = QString::fromStdString(section->FileNameString);
        if (text == "FmpDxe") {
            parentModel->setName(text + " " + parentModel->getFmpDeviceName());
        } else {
            parentModel->setName(text);
        }
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
        name = QString::fromStdString(guidData->getNameFromGuid(section->SubTypeGuid));
        subtype = "Freeform GUID";
        break;
    case EFI_SECTION_RAW:
        name = "Raw Section";
        if (section->isElfFormat) {
            name = "ELF";
        } else if (section->isAcpiHeader) {
            QString Signature = QString::fromStdString(section->AcpiTable->AcpiTableSignature);
            name = "ACPI Table - " + Signature;
        }
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

    for (auto volume:section->ChildFile) {
        switch (volume->Type) {
        case VolumeType::FirmwareVolume:
            FvModel *fvModel;
            fvModel = new FvModel((FirmwareVolume*)volume);
            volumeModelData.push_back(fvModel);
            break;
        case VolumeType::FfsFile:
            FfsModel *ffsModel;
            ffsModel = new FfsModel((FfsFile*)volume, this->parentModel->parentModel);
            volumeModelData.push_back(ffsModel);
            break;
        case VolumeType::CommonSection:
            SectionModel *secModel;
            secModel = new SectionModel((CommonSection*)volume, this->parentModel);
            if (((CommonSection*)volume)->isElfFormat) {
                secModel->name = "ELF";
            }
            volumeModelData.push_back(secModel);
            break;
        case VolumeType::ELF:
            DataModel *dataModel;
            dataModel = new DataModel(volume, "ELF");
            volumeModelData.push_back(dataModel);
            break;
        case VolumeType::Other:
            dataModel = new DataModel(volume, volume->AdditionalMsg);
            volumeModelData.push_back(dataModel);
            break;
        default:
            dataModel = new DataModel(volume, "Unknown");
            volumeModelData.push_back(dataModel);
            break;
        }
    }
}

SectionModel::~SectionModel() = default;

FfsModel::FfsModel(FfsFile *ffs, FvModel *parent) {
    parentModel = parent;
    modelData = ffs;
    name = QString::fromStdString(guidData->getNameFromGuid(ffs->FfsHeader.Name));
    type = "File";
    text = "";
    switch (ffs->FfsHeader.Type) {
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

    for (auto section:ffs->Sections) {
        auto *secModel = new SectionModel(section, this);
        volumeModelData.push_back(secModel);
    }
}

FfsModel::~FfsModel() = default;

QString FfsModel::getFmpDeviceName() {
    QString FmpName;
    EFI_GUID FmpGuid = ((FfsFile*)modelData)->FfsHeader.Name;
    switch (FmpGuid.Data1) {
        case GuidDatabase::gFmpDeviceMonolithicDefaultGuid.Data1:
        FmpName = "Monolithic";
        break;
    case GuidDatabase::gFmpDeviceIfwiDefaultGuid.Data1:
        FmpName = "IFWI";
        break;
    case GuidDatabase::gFmpDeviceBiosDefaultGuid.Data1:
        FmpName = "Bios";
        break;
    case GuidDatabase::gFmpDeviceMeDefaultGuid.Data1:
        FmpName = "Me";
        break;
    case GuidDatabase::gFmpDeviceEcDefaultGuid.Data1:
        FmpName = "EC";
        break;
    case GuidDatabase::gFmpDeviceBtGAcmDefaultGuid.Data1:
        FmpName = "BtgAcm";
        break;
    case GuidDatabase::gFmpDeviceMicrocodeDefaultGuid.Data1:
        FmpName = "uCode";
        break;
    case GuidDatabase::gFmpDevicePlatformCypressPDGuid.Data1:
        FmpName = "CypressPD";
        break;
    case GuidDatabase::gFmpDevicePlatformTiPDGuid.Data1:
        FmpName = "TiPD";
        break;
    case GuidDatabase::gFmpDevicePlatformRetimerGuid.Data1:
        FmpName = "Retimer";
        break;
    case GuidDatabase::gFmpDevicePlatformDiscreteTBTGuid.Data1:
        FmpName = "DiscreteTBT";
        break;
    case GuidDatabase::gFmpDeviceMeFwAdlLpConsGuid.Data1:
        FmpName = "MeFwAdlLpCons";
        break;
    case GuidDatabase::gFmpDeviceMeFwAdlHConsGuid.Data1:
        FmpName = "MeFwAdlHCons";
        break;
    case GuidDatabase::gFmpDeviceMeFwAdlLpCorpGuid.Data1:
        FmpName = "MeFwAdlLpCorp";
        break;
    case GuidDatabase::gFmpDeviceMeFwAdlHCorpGuid.Data1:
        FmpName = "MeFwAdlHCorp";
        break;
    case GuidDatabase::gFmpDeviceIshPdtDefaultGuid.Data1:
        FmpName = "ISH PDT";
        break;
    default:
        break;
    }
    return FmpName;
}

FvModel::FvModel(FirmwareVolume *fv) {
    modelData = fv;
    if (fv->isEmpty) {
        if (fv->size == 0x1000000) {
            name = "IFWI Image";
            type = "Image";
            subtype = "Intel";
        } else {
            name = "Padding";
            type = "Padding";
            subtype = "Empty";
        }
        return;
    }

    name = QString::fromStdString(guidData->getNameFromGuid(fv->getFvGuid().GuidData));
    type = "Volume";
    text = "";

    EFI_GUID guid = fv->getFvGuid(false).GuidData;
    if (guid == GuidDatabase::gEfiFirmwareFileSystem2Guid) {
        subtype = "FFSv2";
    } else if (guid == GuidDatabase::gEfiFirmwareFileSystem3Guid) {
        subtype = "FFSv3";
    } else if (guid == GuidDatabase::gEfiSystemNvDataFvGuid) {
        subtype = "NVRAM";
    }

    for(auto ffs:fv->FfsFiles) {
        auto *ffsmodel = new FfsModel(ffs, this);
        volumeModelData.push_back(ffsmodel);
    }

    if (fv->freeSpace != nullptr) {
        auto *freeModel = new DataModel(fv->freeSpace, "Volume free space", "Free space", "Empty");
        volumeModelData.push_back(freeModel);
    }

    if (fv->NvStorage != nullptr) {
        auto *NvStorageModel = new VariableModel(fv->NvStorage);
        volumeModelData.push_back(NvStorageModel);
    }
}

FvModel::~FvModel() = default;

VariableModel::VariableModel(NvStorageVariable* Nv) {
    modelData = Nv;
    name = "Nv Storage";
    for(NvVariableEntry* NvEntry:Nv->VariableList) {
        QString NvName = QString::fromStdString(NvEntry->VariableName);
        auto *NvEntryModel = new DataModel(NvEntry, NvName, "Variable");
        volumeModelData.push_back(NvEntryModel);
    }
}

VariableModel::~VariableModel() = default;
