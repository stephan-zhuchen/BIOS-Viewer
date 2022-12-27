#include "mainwindow.h"

void MainWindow::setFvData()
{
    using namespace std;
    qDebug("setFvData");
    buffer->setOffset(0);
    INT64 size = buffer->getBufferSize();
    qDebug("size = %x", size);
    UefiSpace::FirmwareVolumeHeaderClass fv = *(UefiSpace::FirmwareVolumeHeaderClass*)buffer->getBytes(0x40);
    GUID guid = fv.getFvGuid();
    cout << &guid << endl;
}
