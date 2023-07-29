#include <QMessageBox>
#include <QElapsedTimer>
#include <thread>
#include "BiosWindow.h"
#include "iwfi.h"
#include "ui_BiosWindow.h"

bool BiosViewerWindow::detectIfwi(INT64 &BiosOffset) const {
    using namespace std;

    Buffer *buffer = WindowData->buffer;
    INT64 bufferSize = WindowData->InputImageSize;
    if (bufferSize < 0x4000) {
        return false;
    }
    buffer->setOffset(0x10);
    UINT32 FlashDescriptorSignature = buffer->getUINT32();
    if (FlashDescriptorSignature != V_FLASH_FDBAR_FLVALSIG) {
        return false;
    }
    buffer->setOffset(0x16);
    UINT32 FRBA_address = buffer->getUINT8() * 0x10;
    buffer->setOffset(FRBA_address);
    UINT32 temp;
    temp = buffer->getUINT32();
    auto *FlashRegion = (FlashRegionBaseArea*)(&temp);
    if (FlashRegion->getBase() != 0) {
        return false;
    }
    if (bufferSize < FlashRegion->getLimit()) {
        return false;
    }
    buffer->setOffset(0);
    auto *flashDescriptorVolume = new FlashDescriptorClass(buffer->getBytes(FlashRegion->getSize()), FlashRegion->getSize(), bufferSize);
    InputData->flashmap += QString::fromStdString(flashDescriptorVolume->getFlashmap());
    InputData->IFWI_Sections.push_back(flashDescriptorVolume);
    InputData->IFWI_ModelData.push_back(new DataModel(flashDescriptorVolume, "Flash Descriptor", "Region", ""));

    FlashRegionBaseArea BiosRegion = flashDescriptorVolume->RegionList.at(FLASH_REGION_TYPE::FlashRegionBios);
    FlashRegionBaseArea MeRegion = flashDescriptorVolume->RegionList.at(FLASH_REGION_TYPE::FlashRegionMe);
    FlashRegionBaseArea GbERegion = flashDescriptorVolume->RegionList.at(FLASH_REGION_TYPE::FlashRegionGbE);
    FlashRegionBaseArea EcRegion = flashDescriptorVolume->RegionList.at(FLASH_REGION_TYPE::FlashRegionEC);
    FlashRegionBaseArea OsseRegion = flashDescriptorVolume->RegionList.at(FLASH_REGION_TYPE::FlashRegionIE);

    if (EcRegion.getLimit() > bufferSize)
        return false;
    if (EcRegion.limit != 0) {
        buffer->setOffset(EcRegion.getBase());
        auto *EcVolume = new EC_RegionClass(buffer->getBytes(EcRegion.getSize()), EcRegion.getSize(), EcRegion.getBase());
        InputData->flashmap += QString::fromStdString(EcVolume->getFlashmap());
        InputData->IFWI_Sections.push_back(EcVolume);
        InputData->IFWI_ModelData.push_back(new DataModel(EcVolume, "EC", "Region", ""));
    }

    if (GbERegion.getLimit() > bufferSize)
        return false;
    if (GbERegion.limit != 0) {
        buffer->setOffset(GbERegion.getBase());
        auto *GbEVolume = new GbE_RegionClass(buffer->getBytes(GbERegion.getSize()), GbERegion.getSize(), GbERegion.getBase());
        InputData->flashmap += QString::fromStdString(GbEVolume->getFlashmap());
        InputData->IFWI_Sections.push_back(GbEVolume);
        InputData->IFWI_ModelData.push_back(new DataModel(GbEVolume, "GbE", "Region", ""));
    }

    if (MeRegion.getLimit() > bufferSize)
        return false;
    if (MeRegion.limit != 0) {
        buffer->setOffset(MeRegion.getBase());
        auto *MeVolume = new ME_RegionClass(buffer->getBytes(MeRegion.getSize()), MeRegion.getSize(), MeRegion.getBase());
        InputData->flashmap += QString::fromStdString(MeVolume->getFlashmap());
        InputData->flashmap += QString::fromStdString(MeVolume->CSE_Layout->getFlashmap());
        InputData->IFWI_Sections.push_back(MeVolume);
        auto *MeModel = new DataModel(MeVolume, "CSE", "Region", "");
        InputData->IFWI_ModelData.push_back(MeModel);
        if (MeVolume->CSE_Layout->isValid()) {
            MeModel->volumeModelData.push_back(new DataModel(MeVolume->CSE_Layout, "CSE Layout Table", "Partition", ""));
            for (CSE_PartitionClass* Partition:MeVolume->CSE_Layout->CSE_Partitions) {
                InputData->flashmap += QString::fromStdString(Partition->getFlashmap());
                QString MeModelName = QString::fromStdString(Partition->PartitionName);
                auto *PartitionModel = new DataModel(Partition, MeModelName, "Partition", "");
                for (CSE_PartitionClass* ChildPartition:Partition->ChildPartitions) {
                    InputData->flashmap += QString::fromStdString(ChildPartition->getFlashmap());
                    QString ChildPartitionName = QString::fromStdString(ChildPartition->PartitionName);
                    auto *ChildPartitionModel = new DataModel(ChildPartition, ChildPartitionName, "Partition", "");
                    PartitionModel->volumeModelData.push_back(ChildPartitionModel);
                }
                MeModel->volumeModelData.push_back(PartitionModel);
            }
        }
    }

    if (OsseRegion.getLimit() > bufferSize)
        return false;
    if (OsseRegion.limit != 0) {
        buffer->setOffset(OsseRegion.getBase());
        auto *OsseVolume = new OSSE_RegionClass(buffer->getBytes(OsseRegion.getSize()), OsseRegion.getSize(), OsseRegion.getBase());
        InputData->flashmap += QString::fromStdString(OsseVolume->getFlashmap());
        InputData->IFWI_Sections.push_back(OsseVolume);
        InputData->IFWI_ModelData.push_back(new DataModel(OsseVolume, "OSSE", "Region", ""));
    }

    if (BiosRegion.getLimit() > bufferSize)
        return false;
    if (BiosRegion.limit != 0) {
        buffer->setOffset(BiosRegion.getBase());
        InputData->BiosImage = new BiosImageVolume(buffer->getBytes(BiosRegion.getSize()), BiosRegion.getSize(), BiosRegion.getBase());
        InputData->IFWI_ModelData.push_back(new DataModel(InputData->BiosImage, "BIOS", "Region", ""));
        BiosOffset = BiosRegion.getBase();
    }
    return true;
}

void BiosViewerWindow::setBiosFvData()
{
    using namespace std;
    Buffer *buffer = WindowData->buffer;
    INT64 offset = 0;
    INT64 bufferSize = buffer->getBufferSize();
    InputData->IFWI_exist = detectIfwi(offset);

    if (!InputData->IFWI_exist) {
        buffer->setOffset(offset);
        InputData->BiosImage = new BiosImageVolume(buffer->getBytes(bufferSize), bufferSize);
        InputData->InputImageModel->setName("BIOS Image Overview");
    }

    while (offset < bufferSize) {
        buffer->setOffset(offset);
        if (bufferSize - offset < 0x40) {
            pushDataToVector(offset, bufferSize - offset);
            return;
        }
        auto *fvHeader = (EFI_FIRMWARE_VOLUME_HEADER*)buffer->getBytes(0x40);
        INT64 FvLength = fvHeader->FvLength;

        INT64 searchInterval = 0x100;
        INT64 EmptyVolumeLength = 0;
        while (!FirmwareVolume::isValidFirmwareVolume(fvHeader)) {
            safeDelete(fvHeader);
            EmptyVolumeLength += searchInterval;
            buffer->setOffset(offset + EmptyVolumeLength);
            if (offset + EmptyVolumeLength >= bufferSize) {
                pushDataToVector(offset, bufferSize - offset);
                return;
            }
            fvHeader = (EFI_FIRMWARE_VOLUME_HEADER*)buffer->getBytes(0x40);
        }
        safeDelete(fvHeader);

        if (offset + EmptyVolumeLength == bufferSize && offset == 0) {
            ui->titleInfomation->setText("No Firmware Found!");
            return;
        }

        if (EmptyVolumeLength != 0) {
            FvLength = EmptyVolumeLength;
        }

        pushDataToVector(offset, FvLength);
        offset += FvLength;
    }
    InputData->BiosImage->FvData = &InputData->FirmwareVolumeData;
}

void BiosViewerWindow::setFfsData() {
    if (InputData->FirmwareVolumeData.size() == 1 && InputData->FirmwareVolumeData.at(0)->isEmpty) {
        safeDelete(InputData->FirmwareVolumeData.at(0));
        InputData->FirmwareVolumeData.clear();
        InputData->InputImageModel->setName("Image Overview");
        InputData->InputImageModel->setType("");
        InputData->InputImageModel->setSubtype("");
        InputData->BiosValidFlag = false;
        return;
    }
    QElapsedTimer timer;
    timer.start();
    vector<class thread> threadPool;
    for (int idx = 0; idx < InputData->FirmwareVolumeData.size(); ++idx) {
        if (InputData->IFWI_exist) {
            InputData->IFWI_ModelData.at(InputData->IFWI_ModelData.size() - 1)->volumeModelData.push_back(nullptr);
        } else {
            InputData->IFWI_ModelData.push_back(nullptr);
        }
    }

    bool EnableMultiThread = false;
    if (setting.value("EnableMultiThread") == "true") {
        EnableMultiThread = true;
    }
    for (int idx = 0; idx < InputData->FirmwareVolumeData.size(); ++idx) {
        auto FvDecoder = [this, EnableMultiThread](int index) {
            FirmwareVolume *volume = InputData->FirmwareVolumeData.at(index);
            volume->decodeFfs(EnableMultiThread);
            auto* fvm = new FvModel(volume);
            if (InputData->IFWI_exist) {
                InputData->IFWI_ModelData.at(InputData->IFWI_ModelData.size() - 1)->volumeModelData.at(index) = fvm;
            } else {
                InputData->IFWI_ModelData.at(index) = fvm;
            }
        };
        if (EnableMultiThread) {
            threadPool.emplace_back(FvDecoder, idx);
        } else {
            FvDecoder(idx);
        }
    }
    for (class thread& t:threadPool) {
        t.join();
    }
    double time = (double)timer.nsecsElapsed()/(double)1000000;
    qDebug() << "setFfsData time = " << time << "ms";
}

void BiosViewerWindow::pushDataToVector(INT64 offset, INT64 length) const {
    Buffer *buffer = WindowData->buffer;
    buffer->setOffset(offset);
    INT64 RemainingSize = buffer->getRemainingSize();
    if (RemainingSize < length) {
        auto *volume = new FirmwareVolume(buffer->getBytes(RemainingSize), RemainingSize, offset, true);
        InputData->FirmwareVolumeData.push_back(volume);
        return;
    }
    try {
        UINT8* fvData = buffer->getBytes(length);  // heap memory
        auto *volume = new FirmwareVolume(fvData, length, offset);
        InputData->FirmwareVolumeBuffer.push_back(fvData);
        InputData->FirmwareVolumeData.push_back(volume);
    } catch (...) {
        auto *volume = new FirmwareVolume(buffer->getBytes(RemainingSize), RemainingSize, offset, true);
        InputData->FirmwareVolumeData.push_back(volume);
    }
}

void BiosViewerWindow::HighlightTreeItem(vector<INT32> rows) const {
    if (rows.empty())
        return;
    QModelIndex itemIndex =  ui->treeWidget->model()->index(rows.at(0), 0, QModelIndex());
    for (int var = 1; var < rows.size(); ++var) {
        itemIndex = ui->treeWidget->model()->index(rows.at(var), 0, itemIndex);
    }
    ui->treeWidget->setCurrentIndex(itemIndex);
}

void BiosViewerWindow::setTreeData() {
    auto *ImageOverviewItem = new QTreeWidgetItem(InputData->InputImageModel->getData());
    ImageOverviewItem->setData(BiosViewerData::Name, Qt::UserRole, QVariant::fromValue((DataModel*)InputData->InputImageModel));
    ImageOverviewItem->setFont(BiosViewerData::Name, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 2, 700));
    ImageOverviewItem->setFont(BiosViewerData::Type, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 2, 700));
    ImageOverviewItem->setFont(BiosViewerData::SubType, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 2, 700));
    ui->treeWidget->addTopLevelItem(ImageOverviewItem);

    if (setting.value("ShowPaddingItem") == "false") {
        erasePadding(InputData->IFWI_ModelData);
    }

    for (int i = 0; i < InputData->IFWI_ModelData.size(); ++i) {
        DataModel *FvModel = InputData->IFWI_ModelData.at(i);
        auto *fvItem = new QTreeWidgetItem(FvModel->getData());
        fvItem->setData(BiosViewerData::Name, Qt::UserRole, QVariant::fromValue((DataModel*)FvModel));
        ui->treeWidget->addTopLevelItem(fvItem);

        for (auto FfsModel:FvModel->volumeModelData) {
            addTreeItem(fvItem, FfsModel);
        }
    }
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
}

void BiosViewerWindow::addTreeItem(QTreeWidgetItem *parentItem, DataModel *modelData) {
    auto *Item = new QTreeWidgetItem(modelData->getData());
    Item->setData(BiosViewerData::Name, Qt::UserRole, QVariant::fromValue(modelData));
    parentItem->addChild(Item);
    for (auto volumeModel:modelData->volumeModelData) {
        addTreeItem(Item, volumeModel);
    }
}

void BiosViewerWindow::erasePadding(vector<DataModel*> &items) {
    for (int i = 0; i < items.size(); ++i) {
        DataModel *FvModel = items.at(i);

        if (FvModel->getSubType() == "Empty" || FvModel->getSubType() == "Pad") {
            safeDelete(FvModel);
            items.erase(items.begin() + i);
            if (i > 0) {
                i -= 1;
            }
            continue;
        }
        erasePadding(FvModel->volumeModelData);
    }
}

void BiosViewerWindow::setPanelInfo(INT64 offset, INT64 size) const
{
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
