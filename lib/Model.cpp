#include "Model.h"
#include "../include/GuidDefinition.h"

DataModel::DataModel(Volume* model, QString nm, QString typ, QString sbtyp, QString txt):
    name(nm),
    type(typ),
    subtype(sbtyp),
    text(txt),
    modelData(model)
{
    rowData = QStringList() << name << type << subtype;
}

DataModel::~DataModel() {
    for (auto volumeModel:volumeModelData) {
        delete volumeModel;
    }
}

void DataModel::setText(QString txt) {
    text = txt;
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
    return rowData;
}

SectionModel::SectionModel(CommonSection *section) {
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
        text = QString::fromWCharArray((wchar_t*)section->VersionString);
        break;
    case EFI_SECTION_USER_INTERFACE:
        name = "UI Section";
        subtype = "UI";
        text = QString::fromWCharArray((wchar_t*)section->FileNameString);
        break;
    case EFI_SECTION_COMPATIBILITY16:
        name = "Compatibility16 Section";
        subtype = "Compatibility16";
        break;
    case EFI_SECTION_FIRMWARE_VOLUME_IMAGE:
        name = "Volume Image Section";
        subtype = "Volume image";
        break;
    case EFI_SECTION_FREEFORM_SUBTYPE_GUID:
        name = QString::fromStdString(guidData->getNameFromGuid(section->SubTypeGuid));
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

    rowData = QStringList() << name << type << subtype;

    for (auto volume:section->ChildFile) {
        switch (volume->Type) {
        case VolumeType::FirmwareVolume:
            FvModel *fvModel;
            fvModel = new FvModel((FirmwareVolume*)volume);
            volumeModelData.push_back(fvModel);
            break;
        case VolumeType::FfsFile:
            FfsModel *ffsModel;
            ffsModel = new FfsModel((FfsFile*)volume);
            volumeModelData.push_back(ffsModel);
            break;
        case VolumeType::CommonSection:
            SectionModel *secModel;
            secModel = new SectionModel((CommonSection*)volume);
            volumeModelData.push_back(secModel);
            break;
        default:
            throw exception();
            break;
        }
    }
}

SectionModel::~SectionModel() {
}

FfsModel::FfsModel(FfsFile *ffs) {
    modelData = ffs;
    name = QString::fromStdString(guidData->getNameFromGuid(ffs->FfsHeader.Name));
    type = "File";
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

    text = "";
    rowData = QStringList() << name << type << subtype;

    for (auto section:ffs->Sections) {
        SectionModel *secModel = new SectionModel(section);
        volumeModelData.push_back(secModel);
    }
}

FfsModel::~FfsModel() {
}

FvModel::FvModel(FirmwareVolume *fv) {
    modelData = fv;
    if (fv->isEmpty) {
        name = "Padding";
        type = "Padding";
        subtype = "Empty";
        rowData = QStringList() << name << type << subtype;
        return;
    }
    if (fv->isNv) {
        name = "Non Volatile Variable";
        type = "Volume";
        subtype = "NVRAM";
        rowData = QStringList() << name << type << subtype;
        return;
    }
    name = QString::fromStdString(guidData->getNameFromGuid(fv->getFvGuid().GuidData));
    type = "Volume";

    EFI_GUID guid = fv->getFvGuid(false).GuidData;
    if (guid == GuidDatabase::gEfiFirmwareFileSystem2Guid) {
        subtype = "FFSv2";
    } else if (guid == GuidDatabase::gEfiFirmwareFileSystem3Guid) {
        subtype = "FFSv3";
    }

    text = "";
    rowData = QStringList() << name << type << subtype;

    for(auto ffs:fv->FfsFiles) {
        FfsModel *ffsmodel = new FfsModel(ffs);
        volumeModelData.push_back(ffsmodel);
    }

    if (fv->freeSpace != nullptr) {
        DataModel *freeModel = new DataModel(fv->freeSpace, "Volume free space", "Free space");
        volumeModelData.push_back(freeModel);
    }
}

FvModel::~FvModel() {
}
