#include "mainwindow.h"

void MainWindow::setFvData()
{
    using namespace std;

    buffer->setOffset(0);
    INT64 bufferSize = buffer->getBufferSize();
    INT64 offset = 0;

    while (offset < bufferSize) {
        buffer->setOffset(offset);
        EFI_FIRMWARE_VOLUME_HEADER *fvHeader = (EFI_FIRMWARE_VOLUME_HEADER*)buffer->getBytes(0x40);
        INT64 FvLength = fvHeader->FvLength;

        INT64 searchInterval = 0x100;
        INT64 EmptyVolumeLength = 0;
        while (!FirmwareVolume::isValidFirmwareVolume(fvHeader)) {
            delete fvHeader;
            EmptyVolumeLength += searchInterval;
            buffer->setOffset(offset + EmptyVolumeLength);
            fvHeader = (EFI_FIRMWARE_VOLUME_HEADER*)buffer->getBytes(0x40);
//            cout << "EmptyVolumeLength = 0x" << hex << EmptyVolumeLength << endl;
        }
        if (EmptyVolumeLength != 0) {
            FvLength = EmptyVolumeLength;
        }

        buffer->setOffset(offset);
        cout << "offset = 0x" << hex << offset << ", FvLength = 0x" << hex << FvLength << endl;
        UINT8* fvData = buffer->getBytes(FvLength);  // heap memory
        FirmwareVolume *volume = new FirmwareVolume(fvData, FvLength, offset);
        FirmwareVolumeBuffer.push_back(fvData);
        FirmwareVolumeData.push_back(volume);
        offset += FvLength;
//        if (offset > 0x70000)
//            break;
    }
}

void MainWindow::setFfsData() {
    for (int idx = 0; idx < FirmwareVolumeData.size(); ++idx) {
        FirmwareVolume *volume = FirmwareVolumeData.at(idx);
        volume->decodeFfs();
        FvModel* fvm = new FvModel(volume);
        FvModelData.push_back(fvm);
//        break;
    }
}
