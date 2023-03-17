#include <QMessageBox>
#include <QElapsedTimer>
#include "mainwindow.h"
#include "lib/iwfi.h"
#include "include/GuidDefinition.h"
#include "./ui_mainwindow.h"

#define V_FLASH_FDBAR_FLVALSIG  0x0FF0A55A

bool MainWindow::detectIfwi(INT64 &BiosOffset) {
    using namespace std;

    INT64 bufferSize = InputImageSize;
    if (bufferSize < 0x1000) {
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
    FlashRegionBaseArea *FlashRegion = (FlashRegionBaseArea*)(&temp);
    if (FlashRegion->getBase() != 0) {
        return false;
    }
    if (bufferSize < FlashRegion->getLimit()) {
        return false;
    }
    buffer->setOffset(0);
//    cout << "FlashRegion size = " << hex << FlashRegion->getSize() << endl;
    FlashDescriptorClass *flashDescriptorVolume = new FlashDescriptorClass(buffer->getBytes(FlashRegion->getSize()), FlashRegion->getSize(), bufferSize);
    IFWI_Sections.push_back(flashDescriptorVolume);
//    IFWI_ModelData.push_back(new DataModel(flashDescriptorVolume, "Flash Descriptor", "Region", ""));

    FlashRegionBaseArea BiosRegion = flashDescriptorVolume->RegionList.at(FLASH_REGION_TYPE::FlashRegionBios);
    FlashRegionBaseArea MeRegion = flashDescriptorVolume->RegionList.at(FLASH_REGION_TYPE::FlashRegionMe);
    FlashRegionBaseArea GbERegion = flashDescriptorVolume->RegionList.at(FLASH_REGION_TYPE::FlashRegionGbE);
    FlashRegionBaseArea EcRegion = flashDescriptorVolume->RegionList.at(FLASH_REGION_TYPE::FlashRegionEC);

    if (EcRegion.getLimit() > bufferSize)
        return false;
    buffer->setOffset(EcRegion.getBase());
    EC_RegionClass *EcVolume = new EC_RegionClass(buffer->getBytes(EcRegion.getSize()), EcRegion.getSize(), EcRegion.getBase());
    IFWI_Sections.push_back(EcVolume);
    IFWI_ModelData.push_back(new DataModel(EcVolume, "EC", "Region", ""));

    if (GbERegion.getLimit() > bufferSize)
        return false;
    buffer->setOffset(GbERegion.getBase());
    GbE_RegionClass *GbEVolume = new GbE_RegionClass(buffer->getBytes(GbERegion.getSize()), GbERegion.getSize(), GbERegion.getBase());
    IFWI_Sections.push_back(GbEVolume);
    IFWI_ModelData.push_back(new DataModel(GbEVolume, "GbE", "Region", ""));

    if (MeRegion.getLimit() > bufferSize)
        return false;
    buffer->setOffset(MeRegion.getBase());
    ME_RegionClass *MeVolume = new ME_RegionClass(buffer->getBytes(MeRegion.getSize()), MeRegion.getSize(), MeRegion.getBase());
    IFWI_Sections.push_back(MeVolume);
    IFWI_ModelData.push_back(new DataModel(MeVolume, "ME", "Region", ""));

    if (BiosRegion.getLimit() > bufferSize)
        return false;
    buffer->setOffset(BiosRegion.getBase());
    BiosImage = new BiosImageVolume(buffer->getBytes(BiosRegion.getSize()), BiosRegion.getSize(), BiosRegion.getBase());
//    IFWI_Sections.push_back(BiosImage);
    IFWI_ModelData.push_back(new DataModel(BiosImage, "BIOS", "Region", ""));
    BiosOffset = BiosRegion.getBase();
    return true;
}

void MainWindow::setBiosFvData()
{
    using namespace std;

    INT64 offset = 0;
    INT64 bufferSize = buffer->getBufferSize();
    bool IFWI_exist = detectIfwi(offset);

    if (!IFWI_exist) {
        buffer->setOffset(offset);
        BiosImage = new BiosImageVolume(buffer->getBytes(bufferSize), bufferSize);
        InputImageModel->setName("BIOS Image Overview");
    }

    while (offset < bufferSize) {
//        cout << "offset = " << hex << offset << endl;
        buffer->setOffset(offset);
        if (bufferSize - offset < 0x40) {
            pushDataToVector(offset, bufferSize - offset);
            return;
        }
        EFI_FIRMWARE_VOLUME_HEADER *fvHeader = (EFI_FIRMWARE_VOLUME_HEADER*)buffer->getBytes(0x40);
        INT64 FvLength = fvHeader->FvLength;

        INT64 searchInterval = 0x100;
        INT64 EmptyVolumeLength = 0;
        while (!FirmwareVolume::isValidFirmwareVolume(fvHeader)) {
            delete fvHeader;
            EmptyVolumeLength += searchInterval;
            buffer->setOffset(offset + EmptyVolumeLength);
//            cout << "Search offset = " << hex << offset + EmptyVolumeLength << endl;
            if (offset + EmptyVolumeLength >= bufferSize) {
                pushDataToVector(offset, bufferSize - offset);
                return;
            }
            fvHeader = (EFI_FIRMWARE_VOLUME_HEADER*)buffer->getBytes(0x40);
        }

        if (offset + EmptyVolumeLength == bufferSize && offset == 0) {
            ui->titleInfomation->setText("No Firmware Found!");
            return;
        }

        if (EmptyVolumeLength != 0) {
            FvLength = EmptyVolumeLength;
        }

//        cout << "offset = 0x" << hex << offset << ", FvLength = 0x" << hex << FvLength << endl;
        pushDataToVector(offset, FvLength);
        offset += FvLength;
    }
    BiosImage->FvData = &FirmwareVolumeData;
}

void MainWindow::setFfsData() {
    if (FirmwareVolumeData.size() == 1 && FirmwareVolumeData.at(0)->isEmpty) {
        FirmwareVolumeData.clear();
        InputImageModel->setName("Image Overview");
        InputImageModel->setType("");
        InputImageModel->setSubtype("");
    }
    QElapsedTimer timer;
    timer.start();
    vector<class thread*> threadPool;
    for (int idx = 0; idx < FirmwareVolumeData.size(); ++idx) {
        FvModelData.push_back(nullptr);
        threadPool.push_back(nullptr);
    }

    for (int idx = 0; idx < FirmwareVolumeData.size(); ++idx) {
        auto FvDecoder = [this](int index) {
            FirmwareVolume *volume = FirmwareVolumeData.at(index);
            volume->decodeFfs();
            FvModel* fvm = new FvModel(volume);
            FvModelData.at(index) = fvm;
        };
        class thread *th = new class thread(FvDecoder, idx);
        threadPool.at(idx) = th;
    }
    for (class thread* t:threadPool) {
        t->join();
        delete t;
    }
    float time = (double)timer.nsecsElapsed()/(double)1000000;
    qDebug() << "setFfsData time = " << time << "ms";
}

void MainWindow::pushDataToVector(INT64 offset, INT64 length) {
    buffer->setOffset(offset);
    INT64 RemainingSize = buffer->getRemainingSize();
    if (RemainingSize < length) {
        FirmwareVolume *volume = new FirmwareVolume(buffer->getBytes(RemainingSize), RemainingSize, offset, true);
        FirmwareVolumeData.push_back(volume);
        return;
    }
    try {
        UINT8* fvData = buffer->getBytes(length);  // heap memory
        FirmwareVolume *volume = new FirmwareVolume(fvData, length, offset);
        FirmwareVolumeBuffer.push_back(fvData);
        FirmwareVolumeData.push_back(volume);
    } catch (...) {
        FirmwareVolume *volume = new FirmwareVolume(buffer->getBytes(RemainingSize), RemainingSize, offset, true);
        FirmwareVolumeData.push_back(volume);
    }
}

void MainWindow::HighlightTreeItem(vector<INT32> rows) {
    if (rows.size() == 0)
        return;
    QModelIndex itemIndex =  ui->treeWidget->model()->index(rows.at(0), 0, QModelIndex());
    for (int var = 1; var < rows.size(); ++var) {
        itemIndex = ui->treeWidget->model()->index(rows.at(var), 0, itemIndex);
    }
    ui->treeWidget->setCurrentIndex(itemIndex);
}
