#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QProcess>
#include <QClipboard>
#include "BiosWindow.h"
#include "HexViewDialog.h"
#include "TabWindow.h"
#include "openssl/sha.h"
#include "openssl/md5.h"
#include "ui_BiosWindow.h"

void BiosViewerWindow::initRightMenu() {
    RightMenu = new QMenu;
    DigestMenu = new QMenu("Digest");
    showPeCoff = new QAction("PE/COFF");
    showAcpiTable = new QAction("ACPI");
    this->connect(showPeCoff,SIGNAL(triggered(bool)),this,SLOT(showPeCoffView()));
    this->connect(showAcpiTable,SIGNAL(triggered(bool)),this,SLOT(showAcpiTableView()));

    showHex = new QAction("Hex View");
    this->connect(showHex,SIGNAL(triggered(bool)),this,SLOT(showHexView()));

    showBodyHex = new QAction("Body Hex View");
    this->connect(showBodyHex,SIGNAL(triggered(bool)),this,SLOT(showBodyHexView()));

    showDecompressedHex = new QAction("Decompressed Hex View");
    this->connect(showDecompressedHex,SIGNAL(triggered(bool)),this,SLOT(showDecompressedHexView()));

    extractVolumeAction = new QAction("Extract");
    this->connect(extractVolumeAction,SIGNAL(triggered(bool)),this,SLOT(extractVolume()));

    extractBodyVolumeAction = new QAction("Extract");
    this->connect(extractBodyVolumeAction,SIGNAL(triggered(bool)),this,SLOT(extractBodyVolume()));

    showNvHex = new QAction("Variable Data Hex View");
    this->connect(showNvHex,SIGNAL(triggered(bool)),this,SLOT(showNvHexView()));

    ExtractRegion = new QAction("Extract");
    this->connect(ExtractRegion,SIGNAL(triggered(bool)),this,SLOT(extractIfwiRegion()));

    ReplaceRegion = new QAction("Replace");
    this->connect(ReplaceRegion,SIGNAL(triggered(bool)),this,SLOT(replaceIfwiRegion()));

    ReplaceFile = new QAction("Replace");
    this->connect(ReplaceFile,SIGNAL(triggered(bool)),this,SLOT(replaceFfsContent()));

    md5_Menu = new QAction("MD5");
    this->connect(md5_Menu,SIGNAL(triggered(bool)),this,SLOT(getMD5()));

    sha1_Menu = new QAction("SHA1");
    this->connect(sha1_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA1()));

    sha224_Menu = new QAction("SHA224");
    this->connect(sha224_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA224()));

    sha256_Menu = new QAction("SHA256");
    this->connect(sha256_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA256()));

    sha384_Menu = new QAction("SHA384");
    this->connect(sha384_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA384()));

    sha512_Menu = new QAction("SHA512");
    this->connect(sha512_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA512()));
}

void BiosViewerWindow::finiRightMenu() {
    if (RightMenu != nullptr)
        delete RightMenu;
    if (DigestMenu != nullptr)
        delete DigestMenu;
    if (showPeCoff != nullptr)
        delete showPeCoff;
    if (showAcpiTable != nullptr)
        delete showAcpiTable;
    if (showHex != nullptr)
        delete showHex;
    if (showBodyHex != nullptr)
        delete showBodyHex;
    if (showDecompressedHex != nullptr)
        delete showDecompressedHex;
    if (extractVolumeAction != nullptr)
        delete extractVolumeAction;
    if (extractBodyVolumeAction != nullptr)
        delete extractBodyVolumeAction;
    if (showNvHex != nullptr)
        delete showNvHex;
    if (ExtractRegion != nullptr)
        delete ExtractRegion;
    if (ReplaceRegion != nullptr)
        delete ReplaceRegion;
    if (ReplaceFile != nullptr)
        delete ReplaceFile;
    if (md5_Menu != nullptr)
        delete md5_Menu;
    if (sha1_Menu != nullptr)
        delete sha1_Menu;
    if (sha224_Menu != nullptr)
        delete sha224_Menu;
    if (sha256_Menu != nullptr)
        delete sha256_Menu;
    if (sha384_Menu != nullptr)
        delete sha384_Menu;
    if (sha512_Menu != nullptr)
        delete sha512_Menu;
}

void BiosViewerWindow::showTreeRightMenu(QPoint pos) {
    QIcon hexBinary, box_arrow_up, replace, key, windows;
    if (isDarkMode()) {
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
    InputData->RightClickeditemModel = item->data(BiosViewerData::Name, Qt::UserRole).value<DataModel*>();
    DataModel *RightClickeditemModel = InputData->RightClickeditemModel;

    RightMenu->clear();
    if (RightClickeditemModel->getSubType() == "PE32 image" ||
        RightClickeditemModel->getSubType() == "PE32+ image") {
        QString filepath = WindowData->appDir + "/tool/PECOFF/dumpbin.exe";
        QFile file(filepath);
        if (file.exists()) {
            showPeCoff->setIcon(windows);
            RightMenu->addAction(showPeCoff);
            file.close();
        }
    }

    if (RightClickeditemModel->getName().mid(0, 10) == "ACPI Table") {
        QString filepath = WindowData->appDir + "/tool/ACPI/iasl.exe";
        QFile file(filepath);
        if (file.exists()) {
            showAcpiTable->setIcon(windows);
            RightMenu->addAction(showAcpiTable);
            file.close();
        }
    }

    showHex->setIcon(hexBinary);
    RightMenu->addAction(showHex);

    if (RightClickeditemModel->getType() == "Volume") {
        showDecompressedHex->setIcon(hexBinary);
        RightMenu->addAction(showDecompressedHex);
    }

    if (RightClickeditemModel->getType() == "Volume" || RightClickeditemModel->getType() == "File" || RightClickeditemModel->getType() == "Section") {
        showBodyHex->setIcon(hexBinary);
        extractVolumeAction->setText("Extract " + RightClickeditemModel->getType());
        extractVolumeAction->setIcon(box_arrow_up);
        extractBodyVolumeAction->setText("Extract " + RightClickeditemModel->getType() + " Body");
        extractBodyVolumeAction->setIcon(box_arrow_up);
        RightMenu->addAction(showBodyHex);
        RightMenu->addAction(extractVolumeAction);
        RightMenu->addAction(extractBodyVolumeAction);
    }

    if (RightClickeditemModel->getType() == "Variable") {
        showNvHex->setIcon(hexBinary);
        RightMenu->addAction(showNvHex);
    }

    QString RegionName = RightClickeditemModel->getName();
    if (RightClickeditemModel->getType() == "Region") {
        ExtractRegion->setText("Extract " + RegionName);
        ExtractRegion->setIcon(box_arrow_up);
        RightMenu->addAction(ExtractRegion);
    }

    if (RightClickeditemModel->getType() == "Region" && RightClickeditemModel->getName() == "BIOS") {
        ReplaceRegion->setText("Replace " + RegionName);
        ReplaceRegion->setIcon(replace);
        RightMenu->addAction(ReplaceRegion);
    }

    if (RightClickeditemModel->getName() == "Startup Acm" && RightClickeditemModel->getType() == "File") {
        ReplaceFile->setText("Replace " + RegionName);
        ReplaceFile->setIcon(replace);
        RightMenu->addAction(ReplaceFile);
    }

    DigestMenu->setIcon(key);
    DigestMenu->addAction(md5_Menu);
    DigestMenu->addAction(sha1_Menu);
    DigestMenu->addAction(sha224_Menu);
    DigestMenu->addAction(sha256_Menu);
    DigestMenu->addAction(sha384_Menu);
    DigestMenu->addAction(sha512_Menu);

    RightMenu->addMenu(DigestMenu);
    RightMenu->move(ui->treeWidget->cursor().pos());
    RightMenu->show();
}

void BiosViewerWindow::showHexView() {
    HexViewDialog *hexDialog = new HexViewDialog();
    if (isDarkMode()) {
        hexDialog->setWindowIcon(QIcon(":/file-binary_light.svg"));
    }
    UINT8 *itemData = InputData->RightClickeditemModel->modelData->data;
    QByteArray *hexViewData = new QByteArray((char*)itemData, InputData->RightClickeditemModel->modelData->size);
    hexDialog->loadBuffer(*hexViewData,
                          InputData->InputImageModel->modelData,
                          InputData->RightClickeditemModel->modelData->offsetFromBegin,
                          InputData->RightClickeditemModel->getName(),
                          WindowData->OpenedFileName,
                          InputData->RightClickeditemModel->modelData->isCompressed);
    hexDialog->show();
    delete hexViewData;
}

void BiosViewerWindow::showBodyHexView() {
    UINT8 *itemData = InputData->RightClickeditemModel->modelData->data;
    INT64 HeaderSize = InputData->RightClickeditemModel->modelData->getHeaderSize();
    if (HeaderSize >= InputData->RightClickeditemModel->modelData->size) {
        QMessageBox::critical(this, tr("BIOS Viewer"), "No Body!");
        return;
    }

    HexViewDialog *hexDialog = new HexViewDialog();
    if (isDarkMode()) {
        hexDialog->setWindowIcon(QIcon(":/file-binary_light.svg"));
    }
    QByteArray *hexViewData = new QByteArray((char*)itemData, InputData->RightClickeditemModel->modelData->size);
    QByteArray BodyHexViewData = hexViewData->mid(HeaderSize);
    hexDialog->loadBuffer(BodyHexViewData,
                          InputData->InputImageModel->modelData,
                          InputData->RightClickeditemModel->modelData->offsetFromBegin,
                          InputData->RightClickeditemModel->getName(),
                          WindowData->OpenedFileName,
                          InputData->RightClickeditemModel->modelData->isCompressed);
    hexDialog->show();
    delete hexViewData;
}

void BiosViewerWindow::showDecompressedHexView() {

}

void BiosViewerWindow::showNvHexView() {
    HexViewDialog *hexDialog = new HexViewDialog();
    UINT8 *NvData = ((NvVariableEntry*)(InputData->RightClickeditemModel->modelData))->DataPtr;
    INT64 NvDataSize = ((NvVariableEntry*)(InputData->RightClickeditemModel->modelData))->DataSize;
    QByteArray *hexViewData = new QByteArray((char*)NvData, NvDataSize);
    hexDialog->loadBuffer(*hexViewData,
                          InputData->InputImageModel->modelData,
                          InputData->RightClickeditemModel->modelData->offsetFromBegin,
                          InputData->RightClickeditemModel->getName(),
                          WindowData->OpenedFileName,
                          InputData->RightClickeditemModel->modelData->isCompressed);
    hexDialog->show();
    delete hexViewData;
}

void BiosViewerWindow::showPeCoffView() {
    QString filepath = WindowData->appDir + "/tool/temp.bin";
    QString toolpath = WindowData->appDir + "/tool/PECOFF/dumpbin.exe";
    INT64 HeaderSize = InputData->RightClickeditemModel->modelData->getHeaderSize();
    Buffer::saveBinary(filepath.toStdString(), InputData->RightClickeditemModel->modelData->data, HeaderSize, InputData->RightClickeditemModel->modelData->size - HeaderSize);
    std::ifstream tempFile(filepath.toStdString());
    if (!tempFile.good()) {
        QMessageBox::critical(this, tr("BIOS Viewer"), "Please run as Administrator!");
        return;
    }
    tempFile.close();
    TabWindow *TabView = new TabWindow();
    if (isDarkMode()) {
        TabView->setWindowIcon(QIcon(":/windows_light.svg"));
    }
    QProcess *process = new QProcess(this);
    process->start(toolpath, QStringList() << "/DISASM" << filepath);
    process->waitForFinished();
    QString DisAssembly = process->readAllStandardOutput();
    TabView->SetNewTabAndText("DisAssembly", DisAssembly);

    process->start(toolpath, QStringList() << "/HEADERS" << filepath);
    process->waitForFinished();
    QString Header = process->readAllStandardOutput();
    TabView->SetNewTabAndText("Header", Header);

    process->start(toolpath, QStringList() << "/RELOCATIONS" << filepath);
    process->waitForFinished();
    QString Relocation = process->readAllStandardOutput();
    TabView->SetNewTabAndText("Relocation", Relocation);

    delete process;

    QFile file(filepath);
    if(file.exists()) {
        file.remove();
    }

    TabView->CollectTabAndShow();
}

void BiosViewerWindow::showAcpiTableView() {
    QString filepath = WindowData->appDir + "/tool/temp.bin";
    QString Dslpath = WindowData->appDir + "/tool/temp.dsl";
    QString toolpath = WindowData->appDir + "/tool/ACPI/iasl.exe";
    INT64 HeaderSize = InputData->RightClickeditemModel->modelData->getHeaderSize();
    Buffer::saveBinary(filepath.toStdString(), InputData->RightClickeditemModel->modelData->data, HeaderSize, InputData->RightClickeditemModel->modelData->size - HeaderSize);
    std::ifstream tempFile(filepath.toStdString());
    if (!tempFile.good()) {
        QMessageBox::critical(this, tr("BIOS Viewer"), "Please run as Administrator!");
        return;
    }
    tempFile.close();
    TabWindow *TabView = new TabWindow();
    if (isDarkMode()) {
        TabView->setWindowIcon(QIcon(":/windows_light.svg"));
    }
    QProcess *process = new QProcess(this);
    process->start(toolpath, QStringList() << "-d" << filepath);
    process->waitForFinished();

    QString DisAssembly;
    QFile DslFile(Dslpath);
    if(DslFile.exists()) {
        DslFile.open(QIODevice::ReadOnly | QIODevice::Text);
        DisAssembly = DslFile.readAll();
        DslFile.close();
        DslFile.remove();
    }
    TabView->SetNewTabAndText("ACPI", DisAssembly);

    delete process;

    QFile file(filepath);
    if(file.exists()) {
        file.remove();
    }

    TabView->CollectTabAndShow();
}

void BiosViewerWindow::extractVolume() {
    QString filename = InputData->RightClickeditemModel->getName() + "_" + InputData->RightClickeditemModel->getType() + ".fd";
    QString outputPath = setting.value("LastFilePath").toString() + "/" + filename;
    QString DialogTitle = "Extract " + InputData->RightClickeditemModel->getType();
    QString extractVolumeName = QFileDialog::getSaveFileName(this,
                                                             DialogTitle,
                                                             outputPath,
                                                             tr("Files(*.rom *.bin *.fd);;All files (*.*)"));
    if (extractVolumeName.isEmpty()) {
        return;
    }
    Buffer::saveBinary(extractVolumeName.toStdString(), InputData->RightClickeditemModel->modelData->data, 0, InputData->RightClickeditemModel->modelData->size);
}

void BiosViewerWindow::extractBodyVolume() {
    INT64 HeaderSize = InputData->RightClickeditemModel->modelData->getHeaderSize();
    if (HeaderSize >= InputData->RightClickeditemModel->modelData->size) {
        QMessageBox::critical(this, tr("BIOS Viewer"), "No Body!");
        return;
    }
    QString filename = InputData->RightClickeditemModel->getName() + "_" + InputData->RightClickeditemModel->getType() + "_body.fd";
    QString outputPath = setting.value("LastFilePath").toString() + "/" + filename;
    QString DialogTitle = "Extract " + InputData->RightClickeditemModel->getType() + " Body";
    QString extractVolumeName = QFileDialog::getSaveFileName(this,
                                                             DialogTitle,
                                                             outputPath,
                                                             tr("Files(*.rom *.bin *.fd);;All files (*.*)"));
    if (extractVolumeName.isEmpty()) {
        return;
    }

    Buffer::saveBinary(extractVolumeName.toStdString(), InputData->RightClickeditemModel->modelData->data, HeaderSize, InputData->RightClickeditemModel->modelData->size - HeaderSize);
}

void BiosViewerWindow::extractIfwiRegion() {
    QString filename = InputData->RightClickeditemModel->getName() + ".bin";
    QString outputPath = setting.value("LastFilePath").toString() + "/" + filename;
    QString DialogTitle = "Extract " + InputData->RightClickeditemModel->getName() + " " + InputData->RightClickeditemModel->getType();
    QString extractRegionName = QFileDialog::getSaveFileName(this,
                                                             DialogTitle,
                                                             outputPath,
                                                             tr("Files(*.rom *.bin *.fd);;All files (*.*)"));
    if (extractRegionName.isEmpty()) {
        return;
    }

    Buffer::saveBinary(extractRegionName.toStdString(), InputData->RightClickeditemModel->modelData->data, 0, InputData->RightClickeditemModel->modelData->size);
}

void BiosViewerWindow::replaceIfwiRegion() {
    // only support BIOS replacement
    ActionReplaceBIOSTriggered();
}

void BiosViewerWindow::replaceFfsContent() {
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
    INT64 FileSize = ((FfsFile*)InputData->RightClickeditemModel->modelData)->FfsSize - ((FfsFile*)InputData->RightClickeditemModel->modelData)->getHeaderSize();
    if (NewFileSize > FileSize) {
        QMessageBox::critical(this, tr("BIOS Viewer"), "File size does not match!");
        delete FileBuffer;
        return;
    }
    UINT8* NewFile = FileBuffer->getBytes(NewFileSize);
    delete FileBuffer;

    INT64 ReplaceOffset = InputData->RightClickeditemModel->modelData->offsetFromBegin + ((FfsFile*)InputData->RightClickeditemModel->modelData)->getHeaderSize();
    UINT8* NewImage = new UINT8[WindowData->InputImageSize];
    for (INT64 IfwiIdx = 0; IfwiIdx < ReplaceOffset; ++IfwiIdx) {
        NewImage[IfwiIdx] = WindowData->InputImage[IfwiIdx];
    }
    for (INT64 FileIdx = 0; FileIdx < NewFileSize; ++FileIdx) {
        NewImage[ReplaceOffset + FileIdx] = NewFile[FileIdx];
    }
    for (INT64 IfwiIdx = ReplaceOffset + NewFileSize; IfwiIdx < WindowData->InputImageSize; ++IfwiIdx) {
        NewImage[IfwiIdx] = WindowData->InputImage[IfwiIdx];
    }

    QFileInfo fileinfo {WindowData->OpenedFileName};
    QString outputPath = setting.value("LastFilePath").toString() + "/" + fileinfo.baseName() + "_NewAcm.bin";
    Buffer::saveBinary(outputPath.toStdString(), NewImage, 0, WindowData->InputImageSize);

    delete[] NewFile;
    delete[] NewImage;
}

void BiosViewerWindow::getMD5() {
    UINT8 *itemData = InputData->RightClickeditemModel->modelData->data;
    UINT8 md[MD5_DIGEST_LENGTH];
    MD5(itemData, InputData->RightClickeditemModel->modelData->size, md);

    QString hash;
    for (INT32 i = 0; i < MD5_DIGEST_LENGTH; i++) {
        hash += QString("%1").arg(md[i], 2, 16, QLatin1Char('0'));
    }
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("MD5"));
    msgBox.setText(hash);

    QPushButton *copyButton = msgBox.addButton(tr("Copy"), QMessageBox::ActionRole);
    msgBox.exec();

    if (msgBox.clickedButton() == copyButton) {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(hash);
    }
}

void BiosViewerWindow::getSHA1() {
    UINT8 *itemData = InputData->RightClickeditemModel->modelData->data;
    UINT8 md[SHA_DIGEST_LENGTH];
    SHA1(itemData, InputData->RightClickeditemModel->modelData->size, md);

    QString hash;
    for (INT32 i = 0; i < SHA_DIGEST_LENGTH; i++) {
        hash += QString("%1").arg(md[i], 2, 16, QLatin1Char('0'));
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("SHA1"));
    msgBox.setText(hash);

    QPushButton *copyButton = msgBox.addButton(tr("Copy"), QMessageBox::ActionRole);
    msgBox.exec();

    if (msgBox.clickedButton() == copyButton) {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(hash);
    }
}

void BiosViewerWindow::getSHA224() {
    UINT8 *itemData = InputData->RightClickeditemModel->modelData->data;
    UINT8 md[SHA224_DIGEST_LENGTH];
    SHA224(itemData, InputData->RightClickeditemModel->modelData->size, md);

    QString hash;
    for (INT32 i = 0; i < SHA224_DIGEST_LENGTH; i++) {
        hash += QString("%1").arg(md[i], 2, 16, QLatin1Char('0'));
    }
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("SHA224"));
    msgBox.setText(hash);

    QPushButton *copyButton = msgBox.addButton(tr("Copy"), QMessageBox::ActionRole);
    msgBox.exec();

    if (msgBox.clickedButton() == copyButton) {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(hash);
    }
}

void BiosViewerWindow::getSHA256() {
    UINT8 *itemData = InputData->RightClickeditemModel->modelData->data;
    UINT8 md[SHA256_DIGEST_LENGTH];
    SHA256(itemData, InputData->RightClickeditemModel->modelData->size, md);

    QString hash;
    for (INT32 i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        hash += QString("%1").arg(md[i], 2, 16, QLatin1Char('0'));
    }
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("SHA256"));
    msgBox.setText(hash);

    QPushButton *copyButton = msgBox.addButton(tr("Copy"), QMessageBox::ActionRole);
    msgBox.exec();

    if (msgBox.clickedButton() == copyButton) {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(hash);
    }
}

void BiosViewerWindow::getSHA384() {
    UINT8 *itemData = InputData->RightClickeditemModel->modelData->data;
    UINT8 md[SHA384_DIGEST_LENGTH];
    SHA384(itemData, InputData->RightClickeditemModel->modelData->size, md);

    QString hash;
    for (INT32 i = 0; i < SHA384_DIGEST_LENGTH; i++) {
        hash += QString("%1").arg(md[i], 2, 16, QLatin1Char('0'));
    }
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("SHA384"));
    msgBox.setText(hash);

    QPushButton *copyButton = msgBox.addButton(tr("Copy"), QMessageBox::ActionRole);
    msgBox.exec();

    if (msgBox.clickedButton() == copyButton) {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(hash);
    }
}

void BiosViewerWindow::getSHA512() {
    UINT8 *itemData = InputData->RightClickeditemModel->modelData->data;
    UINT8 md[SHA512_DIGEST_LENGTH];
    SHA512(itemData, InputData->RightClickeditemModel->modelData->size, md);

    QString hash;
    for (INT32 i = 0; i < SHA512_DIGEST_LENGTH; i++) {
        hash += QString("%1").arg(md[i], 2, 16, QLatin1Char('0'));
    }
    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("SHA512"));
    msgBox.setText(hash);

    QPushButton *copyButton = msgBox.addButton(tr("Copy"), QMessageBox::ActionRole);
    msgBox.exec();

    if (msgBox.clickedButton() == copyButton) {
        QClipboard *clipboard = QGuiApplication::clipboard();
        clipboard->setText(hash);
    }
}
