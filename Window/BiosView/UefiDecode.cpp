#include <QMessageBox>
#include <QElapsedTimer>
#include <thread>
#include "BaseLib.h"
#include "BiosWindow.h"
#include "IfwiRegion/FlashDescriptorRegion.h"
#include "IfwiRegion/EcRegion.h"
#include "IfwiRegion/GbeRegion.h"
#include "IfwiRegion/MeRegion.h"
#include "IfwiRegion/OsseRegion.h"
#include "IfwiRegion/BiosRegion.h"
#include "UefiFileSystem/FirmwareVolume.h"
#include "UefiFileSystem/CompressedVolume.h"
#include <iostream>
#include "ui_BiosWindow.h"

using namespace BaseLibrarySpace;

bool BiosViewerWindow::detectIfwi(INT64 &BiosOffset) const {
    using namespace std;

    INT64 bufferSize = WindowData->InputImageSize;
    if (bufferSize < 0x4000) {
        return false;
    }

    auto CleanVolumeDataList = [this](){
        for (Volume *vol : BiosData->VolumeDataList) {
            safeDelete(vol);
        }
        BiosData->VolumeDataList.clear();
    };

    INT64 IfwiOffset = 0;
    auto *flashDescriptor = new FlashDescriptorRegion(WindowData->InputImage, bufferSize, IfwiOffset);
    if (flashDescriptor->SelfDecode() == 0) {
        safeDelete(flashDescriptor);
        CleanVolumeDataList();
        return false;
    }
    BiosData->VolumeDataList.push_back(flashDescriptor);

    FlashRegionBaseArea BiosRegionArea  = flashDescriptor->RegionList.at(FLASH_REGION_TYPE::FlashRegionBios);
    FlashRegionBaseArea MeRegionArea    = flashDescriptor->RegionList.at(FLASH_REGION_TYPE::FlashRegionMe);
    FlashRegionBaseArea GbERegionArea   = flashDescriptor->RegionList.at(FLASH_REGION_TYPE::FlashRegionGbE);
    FlashRegionBaseArea EcRegionArea    = flashDescriptor->RegionList.at(FLASH_REGION_TYPE::FlashRegionEC);
    FlashRegionBaseArea OsseRegionArea  = flashDescriptor->RegionList.at(FLASH_REGION_TYPE::FlashRegionIE);

    if (EcRegionArea.getLimit() > bufferSize) {
        CleanVolumeDataList();
        return false;
    }

    if (EcRegionArea.limit != 0) {
        UINT8* EcBuffer = WindowData->InputImage + EcRegionArea.getBase();
        auto *EcVolume = new EcRegion(EcBuffer, EcRegionArea.getSize(), EcRegionArea.getBase());
        if (EcVolume->SelfDecode() == 0) {
            safeDelete(EcVolume);
            CleanVolumeDataList();
            return false;
        }
        BiosData->VolumeDataList.push_back(EcVolume);
    }

    if (GbERegionArea.getLimit() > bufferSize) {
        CleanVolumeDataList();
        return false;
    }
    if (GbERegionArea.limit != 0) {
        UINT8* GbeBuffer = WindowData->InputImage + GbERegionArea.getBase();
        auto *GbEVolume = new GbeRegion(GbeBuffer, GbERegionArea.getSize(), GbERegionArea.getBase());
        if (GbEVolume->SelfDecode() == 0) {
            safeDelete(GbEVolume);
            CleanVolumeDataList();
            return false;
        }
        BiosData->VolumeDataList.push_back(GbEVolume);
    }

    if (MeRegionArea.getLimit() > bufferSize) {
        CleanVolumeDataList();
        return false;
    }
    if (MeRegionArea.limit != 0) {
        UINT8* MeBuffer = WindowData->InputImage + MeRegionArea.getBase();
        auto *MeVolume = new MeRegion(MeBuffer, MeRegionArea.getSize(), MeRegionArea.getBase());
        if (MeVolume->SelfDecode() == 0) {
            safeDelete(MeVolume);
            CleanVolumeDataList();
            return false;
        }
        BiosData->VolumeDataList.push_back(MeVolume);
    }

    if (OsseRegionArea.getLimit() > bufferSize) {
        CleanVolumeDataList();
        return false;
    }
    if (OsseRegionArea.limit != 0) {
        UINT8* GbeBuffer = WindowData->InputImage + OsseRegionArea.getBase();
        auto *OsseVolume = new OsseRegion(GbeBuffer, OsseRegionArea.getSize(), OsseRegionArea.getBase());
        if (OsseVolume->SelfDecode() == 0) {
            safeDelete(OsseVolume);
            CleanVolumeDataList();
            return false;
        }
        BiosData->VolumeDataList.push_back(OsseVolume);
    }

    if (BiosRegionArea.getLimit() > bufferSize) {
        CleanVolumeDataList();
        return false;
    }
    if (BiosRegionArea.limit != 0) {
        UINT8* BiosBuffer = WindowData->InputImage + BiosRegionArea.getBase();
        BiosData->BiosImage = new BiosRegion(BiosBuffer, BiosRegionArea.getSize(), BiosRegionArea.getBase());
        BiosData->VolumeDataList.push_back(BiosData->BiosImage);
        BiosOffset = BiosRegionArea.getBase();
    }
    return true;
}

void BiosViewerWindow::setBiosFvData() {
    using namespace std;
    INT64 offset = 0;
    INT64 bufferSize = WindowData->InputImageSize;
    BiosData->IFWI_exist = detectIfwi(offset);

    Volume *parentVolume = BiosData->BiosImage;
    if (!BiosData->IFWI_exist) {
        parentVolume = nullptr;
        BiosData->BiosImage = new BiosRegion(WindowData->InputImage, bufferSize);
        BiosData->OverviewImageModel->setName("BIOS Image Overview");
    }
    BiosData->BiosImage->SelfDecode();

    while (offset < bufferSize) {
        if (bufferSize - offset < 0x40) {
            AddVolumeList(offset, bufferSize - offset, parentVolume, VolumeType::Empty);
            return;
        }
        auto fvHeader = (EFI_FIRMWARE_VOLUME_HEADER*)(WindowData->InputImage + offset);
        INT64 FvLength = (INT64)fvHeader->FvLength;
        bool IsFirmwareVolume = FirmwareVolume::isValidFirmwareVolume(fvHeader);

        auto CompressedVolumeHeader = (LOADER_COMPRESSED_HEADER*)(WindowData->InputImage + offset);
        INT64 CompressedVolumeLength = sizeof(LOADER_COMPRESSED_HEADER) + CompressedVolumeHeader->CompressedSize;
        bool IsCompressedVolume = CompressedVolume::IsCompressedVolume(CompressedVolumeHeader);

        INT64 searchInterval = 0x40;
        INT64 EmptyVolumeLength = 0;
        while (!IsFirmwareVolume && !IsCompressedVolume) {
            EmptyVolumeLength += searchInterval;
            if (offset + EmptyVolumeLength >= bufferSize) {
                AddVolumeList(offset, bufferSize - offset, parentVolume, VolumeType::Empty);
                return;
            }
            fvHeader = (EFI_FIRMWARE_VOLUME_HEADER*)(WindowData->InputImage + offset + EmptyVolumeLength);
            CompressedVolumeHeader = (LOADER_COMPRESSED_HEADER*)(WindowData->InputImage + offset + EmptyVolumeLength);
            IsFirmwareVolume = FirmwareVolume::isValidFirmwareVolume(fvHeader);
            IsCompressedVolume = CompressedVolume::IsCompressedVolume(CompressedVolumeHeader);
        }

        if (offset + EmptyVolumeLength == bufferSize && offset == 0) {
            return;
        }

        if (EmptyVolumeLength != 0) {
            AddVolumeList(offset, EmptyVolumeLength, parentVolume, VolumeType::Empty);
            offset += EmptyVolumeLength;
            continue;
        }

        if (IsFirmwareVolume) {
            AddVolumeList(offset, FvLength, parentVolume, VolumeType::FirmwareVolume);
            offset += FvLength;
        } else if (IsCompressedVolume) {
            AddVolumeList(offset, CompressedVolumeLength, parentVolume, VolumeType::Compressed);
            offset += CompressedVolumeLength;
            Align(offset, 0, 0x1000);
        }

    }
}

void BiosViewerWindow::DecodeBiosFileSystem() {
    using namespace std;
    if (BiosData->VolumeDataList.size() == 1 && BiosData->VolumeDataList.at(0)->getVolumeType() == VolumeType::Empty) {
        delete BiosData->VolumeDataList.at(0);
        BiosData->VolumeDataList.clear();
        BiosData->BiosValidFlag = false;
        return;
    }

    QElapsedTimer timer;
    timer.start();
    vector<class thread> threadPool;
    auto FvDecoder = [this](int index) {
        Volume *volume = BiosData->VolumeDataList.at(index);
        volume->DecodeChildVolume();
    };
    for (int idx = 0; idx < BiosData->VolumeDataList.size(); ++idx) {
        threadPool.emplace_back(FvDecoder, idx);
    }
    for (class thread& t:threadPool) {
        t.join();
    }
    double time = (double)timer.nsecsElapsed()/(double)1000000;
    qDebug() << "setFfsData time = " << time << "ms";
}

void BiosViewerWindow::ReorganizeVolume(Volume *volume) {
    Volume *newVolume = volume->Reorganize();
    if (newVolume != nullptr) {
        safeDelete(volume);
        volume = newVolume;
    }
    for (auto childVolume:volume->ChildVolume) {
        ReorganizeVolume(childVolume);
    }
}

void BiosViewerWindow::AddVolumeList(INT64 offset, INT64 length, Volume *parent, VolumeType type) const {
    Volume *volume{nullptr};
    UINT8* volumeData = WindowData->InputImage + offset;
    if (type == VolumeType::Empty) {
        volume = new Volume(volumeData, length, offset, false, parent);
    } else if (type == VolumeType::FirmwareVolume) {
        try {
            volume = new FirmwareVolume(volumeData, length, offset, false, parent);
            if (volume->SelfDecode() == 0) {
                safeDelete(volume);
                volume = new Volume(volumeData, length, offset, false, parent);
            }
        } catch (...) {
            safeDelete(volume);
            volume = new Volume(volumeData, length, offset, false, parent);
        }
    } else if (type == VolumeType::Compressed) {
        try {
            volume = new CompressedVolume(volumeData, length, offset, parent);
            if (volume->SelfDecode() == 0) {
                safeDelete(volume);
                volume = new Volume(volumeData, length, offset, false, parent);
            }
        } catch (...) {
            safeDelete(volume);
            volume = new Volume(volumeData, length, offset, false, parent);
        }
    }

    if (parent == nullptr) {
        BiosData->VolumeDataList.push_back(volume);
    } else {
        parent->ChildVolume.push_back(volume);
    }
}

void BiosViewerWindow::setTreeData() {
    auto *ImageOverviewItem = new QTreeWidgetItem(BiosData->OverviewImageModel->getData());
    ImageOverviewItem->setData(treeColNum::Name, Qt::UserRole, QVariant::fromValue(BiosData->OverviewVolume));
    ImageOverviewItem->setFont(treeColNum::Name, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 2, 700));
    ImageOverviewItem->setFont(treeColNum::Type, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 2, 700));
    ImageOverviewItem->setFont(treeColNum::SubType, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 2, 700));
    ui->treeWidget->addTopLevelItem(ImageOverviewItem);

    bool ShowPadding {true};
    if (setting.value("ShowPaddingItem") == "false") {
        ShowPadding = false;
    }

    for (auto volume : BiosData->VolumeDataList) {
        addTreeItem(nullptr, volume, ShowPadding);
    }
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
}

void BiosViewerWindow::addTreeItem(QTreeWidgetItem *parentItem, Volume *volume, bool ShowPadding) {
    if (!ShowPadding && volume->getVolumeType() == VolumeType::Empty)
        return;
    DataModel item;
    item.InitFromVolume(volume);

    auto *treeItem = new QTreeWidgetItem(item.getData());
    treeItem->setData(treeColNum::Name, Qt::UserRole, QVariant::fromValue(volume));
    if (parentItem == nullptr) {
        ui->treeWidget->addTopLevelItem(treeItem);
    } else {
        parentItem->addChild(treeItem);
    }

    for (auto childVolume:volume->ChildVolume) {
        addTreeItem(treeItem, childVolume, ShowPadding);
    }
}

void BiosViewerWindow::setPanelInfo(INT64 offset, INT64 size) const {
    stringstream Info;
    Info.setf(ios::left);
    Info << "Offset: 0x";
    Info.width(10);
    Info << hex << uppercase << offset;

    Info << "Size: 0x";
    Info << hex << uppercase << size;

    QString panelInfo = QString::fromStdString(Info.str());
    ui->AddressPanel->setText(panelInfo);
}

bool BiosViewerWindow::isDarkMode() const {
    return WindowData->DarkmodeFlag;
}
