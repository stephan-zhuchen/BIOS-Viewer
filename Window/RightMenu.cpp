#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QProcess>
#include "mainwindow.h"
#include "HexViewDialog.h"
#include "PeCoffInfo.h"
#include "./ui_mainwindow.h"
#include "openssl/sha.h"
#include "openssl/md5.h"

void MainWindow::showTreeRightMenu(QPoint pos) {
    QIcon hexBinary, box_arrow_up, replace, key, windows;
    if (DarkmodeFlag) {
        hexBinary = QIcon(":/file-binary_light.svg");
        box_arrow_up = QIcon(":/box-arrow-up_light.svg");
        replace = QIcon(":/replace_light.svg");
        key = QIcon(":/key_light.svg");
        windows = QIcon(":/windows_light.svg");
    } else {
        hexBinary = QIcon(":/file-binary.svg");
        box_arrow_up = QIcon(":/box-arrow-up.svg");
        replace = QIcon(":/replace.svg");
        key = QIcon(":/key.svg");
        windows = QIcon(":/windows.svg");
    }
    QModelIndex index = ui->treeWidget->indexAt(pos);
    if (!index.isValid())
        return;
    QTreeWidgetItem *item = ui->treeWidget->itemAt(pos);
    RightClickeditemModel = item->data(MainWindow::Name, Qt::UserRole).value<DataModel*>();

    QMenu* menu = new QMenu;
    if (RightClickeditemModel->getSubType() == "PE32 image" ||
        RightClickeditemModel->getSubType() == "PE32+ image") {
        QString filepath = appDir + "/tool/dumpbin.exe";
        QFile file(filepath);
        if (file.exists()) {
            QAction* showPeCoff = new QAction("PE/COFF");
            showPeCoff->setIcon(windows);
            menu->addAction(showPeCoff);
            this->connect(showPeCoff,SIGNAL(triggered(bool)),this,SLOT(showPeCoffView()));
        }
    }
    QAction* showHex = new QAction("Hex View");
    showHex->setIcon(hexBinary);
    menu->addAction(showHex);
    this->connect(showHex,SIGNAL(triggered(bool)),this,SLOT(showHexView()));

    if (RightClickeditemModel->getType() == "Volume" || RightClickeditemModel->getType() == "File" || RightClickeditemModel->getType() == "Section") {
        QAction* showBodyHex = new QAction("Body Hex View");
        QAction* extractVolume = new QAction("Extract " + RightClickeditemModel->getType());
        QAction* extractBodyVolume = new QAction("Extract " + RightClickeditemModel->getType() + " Body");
        showBodyHex->setIcon(hexBinary);
        extractVolume->setIcon(box_arrow_up);
        extractBodyVolume->setIcon(box_arrow_up);
        menu->addAction(showBodyHex);
        menu->addAction(extractVolume);
        menu->addAction(extractBodyVolume);
        this->connect(showBodyHex,SIGNAL(triggered(bool)),this,SLOT(showBodyHexView()));
        this->connect(extractVolume,SIGNAL(triggered(bool)),this,SLOT(extractVolume()));
        this->connect(extractBodyVolume,SIGNAL(triggered(bool)),this,SLOT(extractBodyVolume()));
    }

    if (RightClickeditemModel->getType() == "Variable") {
        QAction* showNvHex = new QAction("Variable Data Hex View");
        showNvHex->setIcon(hexBinary);
        menu->addAction(showNvHex);
        this->connect(showNvHex,SIGNAL(triggered(bool)),this,SLOT(showNvHexView()));
    }

    QString RegionName = RightClickeditemModel->getName();
    if (RightClickeditemModel->getType() == "Region") {
        QAction* ExtractRegion = new QAction("Extract " + RegionName);
        ExtractRegion->setIcon(box_arrow_up);
        menu->addAction(ExtractRegion);
        this->connect(ExtractRegion,SIGNAL(triggered(bool)),this,SLOT(extractIfwiRegion()));
    }

    if (RightClickeditemModel->getType() == "Region" && RightClickeditemModel->getName() == "BIOS") {
        QAction* ReplaceRegion = new QAction("Replace " + RegionName);
        ReplaceRegion->setIcon(replace);
        menu->addAction(ReplaceRegion);
        this->connect(ReplaceRegion,SIGNAL(triggered(bool)),this,SLOT(replaceIfwiRegion()));
    }

    if (RightClickeditemModel->getName() == "Startup Acm" && RightClickeditemModel->getType() == "File") {
        QAction* ReplaceFile = new QAction("Replace " + RegionName);
        ReplaceFile->setIcon(replace);
        menu->addAction(ReplaceFile);
        this->connect(ReplaceFile,SIGNAL(triggered(bool)),this,SLOT(replaceFfsContent()));
    }

    QMenu* DigestMenu = new QMenu("Digest");
    DigestMenu->setIcon(key);
    QAction* md5_Menu = new QAction("MD5");
    DigestMenu->addAction(md5_Menu);
    this->connect(md5_Menu,SIGNAL(triggered(bool)),this,SLOT(getMD5()));
    QAction* sha1_Menu = new QAction("SHA1");
    DigestMenu->addAction(sha1_Menu);
    this->connect(sha1_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA1()));
    QAction* sha224_Menu = new QAction("SHA224");
    DigestMenu->addAction(sha224_Menu);
    this->connect(sha224_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA224()));
    QAction* sha256_Menu = new QAction("SHA256");
    DigestMenu->addAction(sha256_Menu);
    this->connect(sha256_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA256()));
    QAction* sha384_Menu = new QAction("SHA384");
    DigestMenu->addAction(sha384_Menu);
    this->connect(sha384_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA384()));
    QAction* sha512_Menu = new QAction("SHA512");
    DigestMenu->addAction(sha512_Menu);
    this->connect(sha512_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA512()));

    menu->addMenu(DigestMenu);
    menu->move(ui->treeWidget->cursor().pos());
    menu->show();
}

void MainWindow::showHexView() {
    HexViewDialog *hexDialog = new HexViewDialog();
    if (isDarkMode()) {
        hexDialog->setWindowIcon(QIcon(":/file-binary_light.svg"));
    }
    UINT8 *itemData = RightClickeditemModel->modelData->data;
    QByteArray *hexViewData = new QByteArray((char*)itemData, RightClickeditemModel->modelData->size);
    hexDialog->loadBuffer(*hexViewData,
                          InputImageModel->modelData,
                          RightClickeditemModel->modelData->offsetFromBegin,
                          RightClickeditemModel->getName(),
                          OpenedFileName,
                          RightClickeditemModel->modelData->isCompressed);
    hexDialog->exec();
    delete hexViewData;
}

void MainWindow::showBodyHexView() {
    HexViewDialog *hexDialog = new HexViewDialog();
    if (isDarkMode()) {
        hexDialog->setWindowIcon(QIcon(":/file-binary_light.svg"));
    }
    UINT8 *itemData = RightClickeditemModel->modelData->data;
    QByteArray *hexViewData = new QByteArray((char*)itemData, RightClickeditemModel->modelData->size);
    INT64 HeaderSize = RightClickeditemModel->modelData->getHeaderSize();
    QByteArray BodyHexViewData = hexViewData->mid(HeaderSize);
    hexDialog->loadBuffer(BodyHexViewData,
                          InputImageModel->modelData,
                          RightClickeditemModel->modelData->offsetFromBegin,
                          RightClickeditemModel->getName(),
                          OpenedFileName,
                          RightClickeditemModel->modelData->isCompressed);
    hexDialog->exec();
    delete hexViewData;
}

void MainWindow::showNvHexView() {
    HexViewDialog *hexDialog = new HexViewDialog();
    UINT8 *NvData = ((NvVariableEntry*)(RightClickeditemModel->modelData))->DataPtr;
    INT64 NvDataSize = ((NvVariableEntry*)(RightClickeditemModel->modelData))->DataSize;
    QByteArray *hexViewData = new QByteArray((char*)NvData, NvDataSize);
    hexDialog->loadBuffer(*hexViewData,
                          InputImageModel->modelData,
                          RightClickeditemModel->modelData->offsetFromBegin,
                          RightClickeditemModel->getName(),
                          OpenedFileName,
                          RightClickeditemModel->modelData->isCompressed);
    hexDialog->exec();
    delete hexViewData;
}

void MainWindow::showPeCoffView() {
    QString filepath = appDir + "/tool/temp.bin";
    QString toolpath = appDir + "/tool/dumpbin.exe";
    PeCoffInfo *PeCoffDialog = new PeCoffInfo();
    if (isDarkMode()) {
        PeCoffDialog->setWindowIcon(QIcon(":/windows_light.svg"));
    }
    INT64 HeaderSize = RightClickeditemModel->modelData->getHeaderSize();
    Buffer::saveBinary(filepath.toStdString(), RightClickeditemModel->modelData->data, HeaderSize, RightClickeditemModel->modelData->size - HeaderSize);

    QProcess *process = new QProcess(this);
    process->start(toolpath, QStringList() << "/DISASM" << filepath);
    process->waitForFinished();
    QString DisAssembly = process->readAllStandardOutput();
    PeCoffDialog->setAsmText(DisAssembly);

    process->start(toolpath, QStringList() << "/HEADERS" << filepath);
    process->waitForFinished();
    QString Header = process->readAllStandardOutput();
    PeCoffDialog->setHeaderText(Header);

    process->start(toolpath, QStringList() << "/RELOCATIONS" << filepath);
    process->waitForFinished();
    QString Relocation = process->readAllStandardOutput();
    PeCoffDialog->setRelocationText(Relocation);

    delete process;

    QFile file(filepath);
    file.remove();

    PeCoffDialog->exec();
}

void MainWindow::extractVolume() {
    QString filename = RightClickeditemModel->getName() + "_" + RightClickeditemModel->getType() + ".fd";
    QString outputPath = setting.value("LastFilePath").toString() + "/" + filename;
    QString DialogTitle = "Extract " + RightClickeditemModel->getType();
    QString extractVolumeName = QFileDialog::getSaveFileName(this,
                                                             DialogTitle,
                                                             outputPath,
                                                             tr("Files(*.rom *.bin *.fd);;All files (*.*)"));
    if (extractVolumeName.isEmpty()) {
        return;
    }
    Buffer::saveBinary(extractVolumeName.toStdString(), RightClickeditemModel->modelData->data, 0, RightClickeditemModel->modelData->size);
}

void MainWindow::extractBodyVolume() {
    QString filename = RightClickeditemModel->getName() + "_" + RightClickeditemModel->getType() + "_body.fd";
    QString outputPath = setting.value("LastFilePath").toString() + "/" + filename;
    QString DialogTitle = "Extract " + RightClickeditemModel->getType() + " Body";
    QString extractVolumeName = QFileDialog::getSaveFileName(this,
                                                             DialogTitle,
                                                             outputPath,
                                                             tr("Files(*.rom *.bin *.fd);;All files (*.*)"));
    if (extractVolumeName.isEmpty()) {
        return;
    }

    INT64 HeaderSize = RightClickeditemModel->modelData->getHeaderSize();
    Buffer::saveBinary(extractVolumeName.toStdString(), RightClickeditemModel->modelData->data, HeaderSize, RightClickeditemModel->modelData->size - HeaderSize);
}

void MainWindow::extractIfwiRegion() {
    QString filename = RightClickeditemModel->getName() + ".bin";
    QString outputPath = setting.value("LastFilePath").toString() + "/" + filename;
    QString DialogTitle = "Extract " + RightClickeditemModel->getName() + " " + RightClickeditemModel->getType();
    QString extractRegionName = QFileDialog::getSaveFileName(this,
                                                             DialogTitle,
                                                             outputPath,
                                                             tr("Files(*.rom *.bin *.fd);;All files (*.*)"));
    if (extractRegionName.isEmpty()) {
        return;
    }

    Buffer::saveBinary(extractRegionName.toStdString(), RightClickeditemModel->modelData->data, 0, RightClickeditemModel->modelData->size);
}

void MainWindow::replaceIfwiRegion() {
    // only support BIOS replacement
    ActionReplaceBIOSTriggered();
}

void MainWindow::replaceFfsContent() {
    QString lastPath = setting.value("LastFilePath").toString();
    QString FileName = QFileDialog::getOpenFileName(this,
                                                    tr("Replace File"),
                                                    lastPath,
                                                    tr("File image(*.rom *.bin *.fd);;All files (*.*)"));
    if (FileName.isEmpty()) {
        return;
    }

    Buffer *FileBuffer = new BaseLibrarySpace::Buffer(new std::ifstream(FileName.toStdString(), std::ios::in | std::ios::binary));
    INT64 NewFileSize = FileBuffer->getBufferSize();
    INT64 FileSize = ((FfsFile*)RightClickeditemModel->modelData)->FfsSize - ((FfsFile*)RightClickeditemModel->modelData)->getHeaderSize();
    if (NewFileSize > FileSize) {
        QMessageBox::critical(this, tr("About BIOS Viewer"), "File size does not match!");
        delete FileBuffer;
        return;
    }
    UINT8* NewFile = FileBuffer->getBytes(NewFileSize);
    delete FileBuffer;

    INT64 ReplaceOffset = RightClickeditemModel->modelData->offsetFromBegin + ((FfsFile*)RightClickeditemModel->modelData)->getHeaderSize();
    UINT8* NewImage = new UINT8[InputImageSize];
    for (INT64 IfwiIdx = 0; IfwiIdx < ReplaceOffset; ++IfwiIdx) {
        NewImage[IfwiIdx] = InputImage[IfwiIdx];
    }
    for (INT64 FileIdx = 0; FileIdx < NewFileSize; ++FileIdx) {
        NewImage[ReplaceOffset + FileIdx] = NewFile[FileIdx];
    }
    for (INT64 IfwiIdx = ReplaceOffset + NewFileSize; IfwiIdx < InputImageSize; ++IfwiIdx) {
        NewImage[IfwiIdx] = InputImage[IfwiIdx];
    }

    QFileInfo fileinfo {OpenedFileName};
    QString outputPath = setting.value("LastFilePath").toString() + "/" + fileinfo.baseName() + "_NewAcm.bin";
    Buffer::saveBinary(outputPath.toStdString(), NewImage, 0, InputImageSize);

    delete[] NewFile;
    delete[] NewImage;
}

void MainWindow::replaceMicrocodeFile() {

}

void MainWindow::getMD5() {
    UINT8 *itemData = RightClickeditemModel->modelData->data;
    UINT8 md[MD5_DIGEST_LENGTH];
    MD5(itemData, RightClickeditemModel->modelData->size, md);

    QString hash;
    for (INT32 i = 0; i < MD5_DIGEST_LENGTH; i++) {
        hash += QString("%1").arg(md[i], 2, 16, QLatin1Char('0'));
    }
    QMessageBox::about(this, tr("MD5"), hash);
}

void MainWindow::getSHA1() {
    UINT8 *itemData = RightClickeditemModel->modelData->data;
    UINT8 md[SHA_DIGEST_LENGTH];
    SHA1(itemData, RightClickeditemModel->modelData->size, md);

    QString hash;
    for (INT32 i = 0; i < SHA_DIGEST_LENGTH; i++) {
        hash += QString("%1").arg(md[i], 2, 16, QLatin1Char('0'));
    }
    QMessageBox::about(this, tr("SHA1"), hash);
}

void MainWindow::getSHA224() {
    UINT8 *itemData = RightClickeditemModel->modelData->data;
    UINT8 md[SHA224_DIGEST_LENGTH];
    SHA224(itemData, RightClickeditemModel->modelData->size, md);

    QString hash;
    for (INT32 i = 0; i < SHA224_DIGEST_LENGTH; i++) {
        hash += QString("%1").arg(md[i], 2, 16, QLatin1Char('0'));
    }
    QMessageBox::about(this, tr("SHA224"), hash);
}

void MainWindow::getSHA256() {
    UINT8 *itemData = RightClickeditemModel->modelData->data;
    UINT8 md[SHA256_DIGEST_LENGTH];
    SHA256(itemData, RightClickeditemModel->modelData->size, md);

    QString hash;
    for (INT32 i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        hash += QString("%1").arg(md[i], 2, 16, QLatin1Char('0'));
    }
    QMessageBox::about(this, tr("SHA256"), hash);
}

void MainWindow::getSHA384() {
    UINT8 *itemData = RightClickeditemModel->modelData->data;
    UINT8 md[SHA384_DIGEST_LENGTH];
    SHA384(itemData, RightClickeditemModel->modelData->size, md);

    QString hash;
    for (INT32 i = 0; i < SHA384_DIGEST_LENGTH; i++) {
        hash += QString("%1").arg(md[i], 2, 16, QLatin1Char('0'));
    }
    QMessageBox::about(this, tr("SHA384"), hash);
}

void MainWindow::getSHA512() {
    UINT8 *itemData = RightClickeditemModel->modelData->data;
    UINT8 md[SHA512_DIGEST_LENGTH];
    SHA512(itemData, RightClickeditemModel->modelData->size, md);

    QString hash;
    for (INT32 i = 0; i < SHA512_DIGEST_LENGTH; i++) {
        hash += QString("%1").arg(md[i], 2, 16, QLatin1Char('0'));
    }
    QMessageBox::about(this, tr("SHA512"), hash);
}
