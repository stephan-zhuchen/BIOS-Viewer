#include <QMessageBox>
#include "mainwindow.h"
#include "include/GuidDefinition.h"
#include "./ui_mainwindow.h"

void MainWindow::setFvData()
{
    using namespace std;
    cout << "setFvData" << endl;

    buffer->setOffset(0);
    INT64 bufferSize = buffer->getBufferSize();
    INT64 offset = 0;
    BiosImage = new BiosImageVolume(buffer->getBytes(bufferSize), bufferSize);
    BiosImageModel = new DataModel(BiosImage, "Image Overview", "Image", "UEFI");

    buffer->setOffset(0);

    while (offset < bufferSize) {
        cout << "offset = " << hex << offset << endl;
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
            cout << "Search offset = " << hex << offset + EmptyVolumeLength << endl;
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

        cout << "offset = 0x" << hex << offset << ", FvLength = 0x" << hex << FvLength << endl;
        pushDataToVector(offset, FvLength);
        offset += FvLength;
    }
}

void MainWindow::setFfsData() {
    if (FirmwareVolumeData.size() == 1 && FirmwareVolumeData.at(0)->isEmpty) {
        FirmwareVolumeData.clear();
        BiosImageModel->setType("");
        BiosImageModel->setSubtype("");
    }
    for (int idx = 0; idx < FirmwareVolumeData.size(); ++idx) {
        FirmwareVolume *volume = FirmwareVolumeData.at(idx);
        volume->decodeFfs();
        FvModel* fvm = new FvModel(volume);
        FvModelData.push_back(fvm);
    }
}

void MainWindow::pushDataToVector(INT64 offset, INT64 length) {
    buffer->setOffset(offset);
    cout << "Remaining size = " << hex << buffer->getRemainingSize() << endl;
    cout << "length = " << hex << length << endl;
    UINT8* fvData = buffer->getBytes(length);  // heap memory
    cout << "After getBytes" << endl;
    FirmwareVolume *volume = new FirmwareVolume(fvData, length, offset);
    cout << "After new FirmwareVolume" << endl;

    FirmwareVolumeBuffer.push_back(fvData);
    cout << "After push FirmwareVolumeBuffer" << endl;
    FirmwareVolumeData.push_back(volume);
    cout << "After push FirmwareVolumeData" << endl;
}

void MainWindow::getBiosID() {
    for (size_t idx = FirmwareVolumeData.size(); idx > 0; --idx) {
        FirmwareVolume *volume = FirmwareVolumeData.at(idx - 1);
        for(auto file:volume->FfsFiles) {
            if (file->FfsHeader.Name == GuidDatabase::gBiosIdGuid) {
                if (file->Sections.size() == 0)
                    return;
                CommonSection *sec = file->Sections.at(0);
                CHAR16 *biosIdStr = (CHAR16*)(sec->data + sizeof(EFI_COMMON_SECTION_HEADER) + 8);
                BiosID = QString::fromStdString(Buffer::wstringToString(biosIdStr));
                BiosImage->BiosID = BiosID.toStdString();
                ui->titleInfomation->setText(BiosID);
                return;
            }
        }
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
