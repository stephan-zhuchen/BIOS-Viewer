#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QProcess>
#include <QClipboard>
#include <Windows.h>
#include <QSharedMemory>
//#include <algorithm>
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
    showBgup = new QAction("BGUP");
    connect(showPeCoff,     SIGNAL(triggered(bool)),this,SLOT(showPeCoffView()));
    connect(showAcpiTable,  SIGNAL(triggered(bool)),this,SLOT(showAcpiTableView()));
    connect(showBgup,       SIGNAL(triggered(bool)),this,SLOT(showBgupView()));

    showHex = new QAction("Hex View");
    connect(showHex,SIGNAL(triggered(bool)),this,SLOT(showHexView()));

    showBodyHex = new QAction("Body Hex View");
    connect(showBodyHex,SIGNAL(triggered(bool)),this,SLOT(showBodyHexView()));

    showDecompressedHex = new QAction("Decompressed Hex View");
    connect(showDecompressedHex,SIGNAL(triggered(bool)),this,SLOT(showDecompressedHexView()));

    showDecompressedBiosHex = new QAction("Decompressed Hex View");
    connect(showDecompressedBiosHex,SIGNAL(triggered(bool)),this,SLOT(showDecompressedBiosHexView()));

    extractVolumeAction = new QAction("Extract");
    connect(extractVolumeAction,SIGNAL(triggered(bool)),this,SLOT(extractVolume()));

    extractBodyVolumeAction = new QAction("Extract");
    connect(extractBodyVolumeAction,SIGNAL(triggered(bool)),this,SLOT(extractBodyVolume()));

    showNvHex = new QAction("Variable Data Hex View");
    connect(showNvHex,SIGNAL(triggered(bool)),this,SLOT(showNvHexView()));

    ExtractRegion = new QAction("Extract");
    connect(ExtractRegion,SIGNAL(triggered(bool)),this,SLOT(extractIfwiRegion()));

    ReplaceRegion = new QAction("Replace");
    connect(ReplaceRegion,SIGNAL(triggered(bool)),this,SLOT(replaceIfwiRegion()));

    ReplaceFile = new QAction("Replace");
    connect(ReplaceFile,SIGNAL(triggered(bool)),this,SLOT(replaceFfsContent()));

    md5_Menu = new QAction("MD5");
    connect(md5_Menu,SIGNAL(triggered(bool)),this,SLOT(getMD5()));

    sha1_Menu = new QAction("SHA1");
    connect(sha1_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA1()));

    sha224_Menu = new QAction("SHA224");
    connect(sha224_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA224()));

    sha256_Menu = new QAction("SHA256");
    connect(sha256_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA256()));

    sha384_Menu = new QAction("SHA384");
    connect(sha384_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA384()));

    sha512_Menu = new QAction("SHA512");
    connect(sha512_Menu,SIGNAL(triggered(bool)),this,SLOT(getSHA512()));
}

void BiosViewerWindow::finiRightMenu() {
    safeDelete(RightMenu);
    safeDelete(DigestMenu);
    safeDelete(showPeCoff);
    safeDelete(showAcpiTable);
    safeDelete(showBgup);
    safeDelete(showHex);
    safeDelete(showBodyHex);
    safeDelete(showDecompressedHex);
    safeDelete(showDecompressedBiosHex);
    safeDelete(extractVolumeAction);
    safeDelete(extractBodyVolumeAction);
    safeDelete(showNvHex);
    safeDelete(ExtractRegion);
    safeDelete(ReplaceRegion);
    safeDelete(ReplaceFile);
    safeDelete(md5_Menu);
    safeDelete(sha1_Menu);
    safeDelete(sha224_Menu);
    safeDelete(sha256_Menu);
    safeDelete(sha384_Menu);
    safeDelete(sha512_Menu);
}

void BiosViewerWindow::showTreeRightMenu(QPoint pos) const {
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
    DataModel *RightClickedItemModel = InputData->RightClickeditemModel;

    RightMenu->clear();
    if (RightClickedItemModel->getSubType() == "PE32 image" ||
        RightClickedItemModel->getSubType() == "PE32+ image") {
        QString filepath = WindowData->appDir + "/tool/PECOFF/dumpbin.exe";
        QFile file(filepath);
        if (file.exists()) {
            showPeCoff->setIcon(windows);
            RightMenu->addAction(showPeCoff);
            file.close();
        }
    }

    if (RightClickedItemModel->getName().mid(0, 10) == "ACPI Table") {
        QString filepath = WindowData->appDir + "/tool/ACPI/iasl.exe";
        QFile file(filepath);
        if (file.exists()) {
            showAcpiTable->setIcon(windows);
            RightMenu->addAction(showAcpiTable);
            file.close();
        }
    }

    if (RightClickedItemModel->getName().right(4).toLower() == "bgsl") {
        showBgup->setIcon(windows);
        RightMenu->addAction(showBgup);
    }

    showHex->setIcon(hexBinary);
    RightMenu->addAction(showHex);

    if (RightClickedItemModel->getType() == "Volume") {
        showDecompressedHex->setIcon(hexBinary);
        RightMenu->addAction(showDecompressedHex);
    }

    if (RightClickedItemModel->getName() == "BIOS Image Overview") {
        showDecompressedBiosHex->setIcon(hexBinary);
        RightMenu->addAction(showDecompressedBiosHex);
    }


    if (RightClickedItemModel->getType() == "Volume" || RightClickedItemModel->getType() == "File" || RightClickedItemModel->getType() == "Section") {
        showBodyHex->setIcon(hexBinary);
        extractVolumeAction->setText("Extract " + RightClickedItemModel->getType());
        extractVolumeAction->setIcon(box_arrow_up);
        extractBodyVolumeAction->setText("Extract " + RightClickedItemModel->getType() + " Body");
        extractBodyVolumeAction->setIcon(box_arrow_up);
        RightMenu->addAction(showBodyHex);
        RightMenu->addAction(extractVolumeAction);
        RightMenu->addAction(extractBodyVolumeAction);
    }

    if (RightClickedItemModel->getType() == "Variable") {
        showNvHex->setIcon(hexBinary);
        RightMenu->addAction(showNvHex);
    }

    QString RegionName = RightClickedItemModel->getName();
    if (RightClickedItemModel->getType() == "Region") {
        ExtractRegion->setText("Extract " + RegionName);
        ExtractRegion->setIcon(box_arrow_up);
        RightMenu->addAction(ExtractRegion);
    }

    if (RightClickedItemModel->getType() == "Region" && RightClickedItemModel->getName() == "BIOS") {
        ReplaceRegion->setText("Replace " + RegionName);
        ReplaceRegion->setIcon(replace);
        RightMenu->addAction(ReplaceRegion);
    }

    if (RightClickedItemModel->getName() == "Startup Acm" && RightClickedItemModel->getType() == "File") {
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

void BiosViewerWindow::showHexView() const {
    auto *hexDialog = new HexViewDialog(WindowData->DarkmodeFlag);
    if (isDarkMode()) {
        hexDialog->setWindowIcon(QIcon(":/file-binary_light.svg"));
    }
    UINT8 *itemData = InputData->RightClickeditemModel->modelData->data;
    auto *hexViewData = new QByteArray((char*)itemData, InputData->RightClickeditemModel->modelData->size);
    hexDialog->loadBuffer(*hexViewData,
                          InputData->InputImageModel->modelData,
                          InputData->RightClickeditemModel->modelData->offsetFromBegin,
                          WindowData->InputImageSize,
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

    auto *hexDialog = new HexViewDialog(WindowData->DarkmodeFlag);
    if (isDarkMode()) {
        hexDialog->setWindowIcon(QIcon(":/file-binary_light.svg"));
    }
    auto *hexViewData = new QByteArray((char*)itemData, InputData->RightClickeditemModel->modelData->size);
    QByteArray BodyHexViewData = hexViewData->mid(HeaderSize);
    hexDialog->loadBuffer(BodyHexViewData,
                          InputData->InputImageModel->modelData,
                          InputData->RightClickeditemModel->modelData->offsetFromBegin + HeaderSize,
                          WindowData->InputImageSize,
                          InputData->RightClickeditemModel->getName(),
                          WindowData->OpenedFileName,
                          InputData->RightClickeditemModel->modelData->isCompressed);
    hexDialog->show();
    delete hexViewData;
}

void BiosViewerWindow::showDecompressedHexView() {
    Volume *volume = InputData->RightClickeditemModel->modelData;
    vector<UINT8> DecompressedVector;
    if (!volume->GetDecompressedVolume(DecompressedVector)) {
        QMessageBox::critical(this, tr("BIOS Viewer"), "No Compressed Section");
        return;
    }

    auto *hexViewData = new QByteArray((char*)DecompressedVector.data(), DecompressedVector.size());
    auto *hexDialog = new HexViewDialog(WindowData->DarkmodeFlag);
    if (isDarkMode()) {
        hexDialog->setWindowIcon(QIcon(":/file-binary_light.svg"));
    }

    hexDialog->loadBuffer(*hexViewData,
                          InputData->InputImageModel->modelData,
                          InputData->RightClickeditemModel->modelData->offsetFromBegin,
                          WindowData->InputImageSize,
                          InputData->RightClickeditemModel->getName() + " Decompressed",
                          WindowData->OpenedFileName,
                          true);

    hexDialog->show();
    delete hexViewData;
}

void BiosViewerWindow::showDecompressedBiosHexView() {
    vector<UINT8> DecompressedBios;
    for (Volume *volume:*InputData->BiosImage->FvData) {
        vector<UINT8> DecompressedVector;
        if (!volume->GetDecompressedVolume(DecompressedVector)) {
            DecompressedVector = vector<UINT8>(volume->data, volume->data + volume->size);
        }
        DecompressedBios.insert(DecompressedBios.end(), DecompressedVector.begin(), DecompressedVector.end());
    }

    auto *hexViewData = new QByteArray((char*)DecompressedBios.data(), DecompressedBios.size());
    auto *hexDialog = new HexViewDialog(WindowData->DarkmodeFlag);
    if (isDarkMode()) {
        hexDialog->setWindowIcon(QIcon(":/file-binary_light.svg"));
    }

    hexDialog->loadBuffer(*hexViewData,
                          InputData->InputImageModel->modelData,
                          InputData->RightClickeditemModel->modelData->offsetFromBegin,
                          WindowData->InputImageSize,
                          InputData->RightClickeditemModel->getName() + " Decompressed",
                          WindowData->OpenedFileName,
                          true);

    hexDialog->show();
    delete hexViewData;
}


void BiosViewerWindow::showNvHexView() const {
    auto *hexDialog = new HexViewDialog(WindowData->DarkmodeFlag);
    UINT8 *NvData = ((NvVariableEntry*)(InputData->RightClickeditemModel->modelData))->DataPtr;
    INT64 NvDataSize = ((NvVariableEntry*)(InputData->RightClickeditemModel->modelData))->DataSize;
    auto *hexViewData = new QByteArray((char*)NvData, NvDataSize);
    hexDialog->loadBuffer(*hexViewData,
                          InputData->InputImageModel->modelData,
                          InputData->RightClickeditemModel->modelData->offsetFromBegin,
                          WindowData->InputImageSize,
                          InputData->RightClickeditemModel->getName(),
                          WindowData->OpenedFileName,
                          InputData->RightClickeditemModel->modelData->isCompressed);
    hexDialog->show();
    delete hexViewData;
}

void BiosViewerWindow::showPeCoffView() {
    QString filepath = WindowData->appDir + "/tool/temp.bin";
    QString toolpath = WindowData->appDir + "/tool/PECOFF/dumpbin.exe";
    QString SaveToolPath = WindowData->appDir + "/tool/SaveFile/QtSaveFile.exe";
    INT64 HeaderSize = InputData->RightClickeditemModel->modelData->getHeaderSize();

    enum ToolType { NONE=0, PECOFF=1, ACPI=2 };
    struct SharedData {
        ToolType Type;
        CHAR8   stringData[256];
        INT64   Offset;
        INT64   Size;
    };
    SharedData PeCoffPara;
    std::memcpy(PeCoffPara.stringData, filepath.toStdString().c_str(), min(filepath.toStdString().size(), sizeof(PeCoffPara.stringData) - 1));
    PeCoffPara.stringData[min(filepath.toStdString().size(), sizeof(PeCoffPara.stringData) - 1)] = '\0';
    PeCoffPara.Type = ToolType::PECOFF;
    PeCoffPara.Offset = HeaderSize;
    PeCoffPara.Size = InputData->RightClickeditemModel->modelData->size - HeaderSize;

    QSharedMemory sharedParameter;
    sharedParameter.setKey("BiosViewerSharedParameter");
    sharedParameter.create(sizeof(PeCoffPara));
    char *para = (char*)sharedParameter.data();
    memcpy(para, &PeCoffPara, sizeof(PeCoffPara));

    QSharedMemory sharedMemory;
    sharedMemory.setKey("BiosViewerSharedData");
    sharedMemory.create(InputData->RightClickeditemModel->modelData->size);
    char *to = (char*)sharedMemory.data();
    memcpy(to, InputData->RightClickeditemModel->modelData->data, InputData->RightClickeditemModel->modelData->size);

    SHELLEXECUTEINFO shExecInfo = { sizeof(SHELLEXECUTEINFO) };
    shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shExecInfo.lpVerb = L"runas";
    shExecInfo.lpFile = SaveToolPath.toStdWString().c_str();;
    shExecInfo.lpParameters = L"elevate";
    shExecInfo.nShow = SW_HIDE;

    if (ShellExecuteEx(&shExecInfo)) {
        WaitForSingleObject(shExecInfo.hProcess, INFINITE);
        CloseHandle(shExecInfo.hProcess);
    } else {
        QMessageBox::critical(this, tr("BIOS Viewer"), "Please run as Administrator!");
    }

    std::ifstream tempFile(filepath.toStdString());
    if (!tempFile.good()) {
        QMessageBox::critical(this, tr("BIOS Viewer"), "Please run as Administrator!");
        return;
    }
    tempFile.close();
    auto *TabView = new TabWindow();
    TabView->SetTabViewTitle("PE/COFF");
    if (isDarkMode()) {
        TabView->setWindowIcon(QIcon(":/windows_light.svg"));
    }
    auto *process = new QProcess(this);
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
    QString AppPath = WindowData->appDir;
    QString Dslpath = WindowData->appDir + "/tool/acpitemp.dsl";
    QString SaveToolPath = WindowData->appDir + "/tool/SaveFile/QtSaveFile.exe";
    INT64 HeaderSize = InputData->RightClickeditemModel->modelData->getHeaderSize();

    enum ToolType { NONE=0, PECOFF=1, ACPI=2 };

    struct SharedData {
        ToolType Type;
        CHAR8   stringData[256];
        INT64   Offset;
        INT64   Size;
    };
    SharedData PeCoffPara;
    std::memcpy(PeCoffPara.stringData, AppPath.toStdString().c_str(), min(AppPath.toStdString().size(), sizeof(PeCoffPara.stringData) - 1));
    PeCoffPara.stringData[min(AppPath.toStdString().size(), sizeof(PeCoffPara.stringData) - 1)] = '\0';
    PeCoffPara.Type = ToolType::ACPI;
    PeCoffPara.Offset = HeaderSize;
    PeCoffPara.Size = InputData->RightClickeditemModel->modelData->size - HeaderSize;

    QSharedMemory sharedParameter;
    sharedParameter.setKey("BiosViewerSharedParameter");
    sharedParameter.create(sizeof(PeCoffPara));
    char *para = (char*)sharedParameter.data();
    memcpy(para, &PeCoffPara, sizeof(PeCoffPara));

    QSharedMemory sharedMemory;
    sharedMemory.setKey("BiosViewerSharedData");
    sharedMemory.create(InputData->RightClickeditemModel->modelData->size);
    char *to = (char*)sharedMemory.data();
    memcpy(to, InputData->RightClickeditemModel->modelData->data, InputData->RightClickeditemModel->modelData->size);

    SHELLEXECUTEINFO shExecInfo = { sizeof(SHELLEXECUTEINFO) };
    shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shExecInfo.lpVerb = L"runas";
    shExecInfo.lpFile = SaveToolPath.toStdWString().c_str();;
    shExecInfo.lpParameters = L"elevate";
    shExecInfo.nShow = SW_HIDE;

    if (ShellExecuteEx(&shExecInfo)) {
        WaitForSingleObject(shExecInfo.hProcess, INFINITE);
        CloseHandle(shExecInfo.hProcess);
    }
    sharedParameter.detach();
    sharedMemory.detach();

    auto *TabView = new TabWindow();
    TabView->SetTabViewTitle("ACPI Table");
    if (isDarkMode()) {
        TabView->setWindowIcon(QIcon(":/windows_light.svg"));
    }

    QString DisAssembly;
    QFile DslFile(Dslpath);
    if(DslFile.exists()) {
        DslFile.open(QIODevice::ReadOnly | QIODevice::Text);
        DisAssembly = DslFile.readAll();
        DslFile.close();
        DslFile.remove();
    } else {
        QMessageBox::critical(this, tr("BIOS Viewer"), "Please run as Administrator!");
        return;
    }
    TabView->SetNewTabAndText("ACPI", DisAssembly);
    TabView->CollectTabAndShow();
}

void BiosViewerWindow::showBgupView() {
    Volume *ClickedItemVolume = InputData->RightClickeditemModel->modelData;
    INT64 HeaderSize = ClickedItemVolume->getHeaderSize() + sizeof(EFI_COMMON_SECTION_HEADER);
    UINT8* BgupData = ClickedItemVolume->data + HeaderSize;
    BiosGuardClass bgup = BiosGuardClass(BgupData, ClickedItemVolume->offsetFromBegin + HeaderSize, ClickedItemVolume->size - HeaderSize);
    bgup.setInfoStr();
    auto *TabView = new TabWindow();
    TabView->SetTabViewTitle("BIOS Guard Update Package");
    if (isDarkMode()) {
        TabView->setWindowIcon(QIcon(":/windows_light.svg"));
    }
    TabView->SetNewTabAndText("BGUP", bgup.InfoStr);
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
    saveBinary(extractVolumeName.toStdString(), InputData->RightClickeditemModel->modelData->data, 0, InputData->RightClickeditemModel->modelData->size);
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

    saveBinary(extractVolumeName.toStdString(), InputData->RightClickeditemModel->modelData->data, HeaderSize, InputData->RightClickeditemModel->modelData->size - HeaderSize);
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

    saveBinary(extractRegionName.toStdString(), InputData->RightClickeditemModel->modelData->data, 0, InputData->RightClickeditemModel->modelData->size);
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

    auto *FileBuffer = new BaseLibrarySpace::Buffer(new std::ifstream(FileName.toStdString(), std::ios::in | std::ios::binary));
    INT64 NewFileSize = FileBuffer->getBufferSize();
    INT64 FileSize = ((FfsFile*)InputData->RightClickeditemModel->modelData)->FfsSize - ((FfsFile*)InputData->RightClickeditemModel->modelData)->getHeaderSize();
    if (NewFileSize > FileSize) {
        QMessageBox::critical(this, tr("BIOS Viewer"), "File size does not match!");
        safeDelete(FileBuffer);
        return;
    }
    UINT8* NewFile = FileBuffer->getBytes((INT32)NewFileSize);
    safeDelete(FileBuffer);

    INT64 ReplaceOffset = InputData->RightClickeditemModel->modelData->offsetFromBegin + ((FfsFile*)InputData->RightClickeditemModel->modelData)->getHeaderSize();
    auto* NewImage = new UINT8[WindowData->InputImageSize];
    for (INT64 IfwiIdx = 0; IfwiIdx < ReplaceOffset; ++IfwiIdx) {
        NewImage[IfwiIdx] = WindowData->InputImage[IfwiIdx];
    }
    for (INT64 FileIdx = 0; FileIdx < NewFileSize; ++FileIdx) {
        NewImage[ReplaceOffset + FileIdx] = NewFile[FileIdx];
    }
    for (INT64 IfwiIdx = ReplaceOffset + NewFileSize; IfwiIdx < WindowData->InputImageSize; ++IfwiIdx) {
        NewImage[IfwiIdx] = WindowData->InputImage[IfwiIdx];
    }

    QFileInfo fileInfo {WindowData->OpenedFileName};
    QString outputPath = setting.value("LastFilePath").toString() + "/" + fileInfo.baseName() + "_NewAcm.bin";
    saveBinary(outputPath.toStdString(), NewImage, 0, WindowData->InputImageSize);

    safeArrayDelete(NewFile);
    safeArrayDelete(NewImage);
}

void BiosViewerWindow::getMD5() const {
    UINT8 *itemData = InputData->RightClickeditemModel->modelData->data;
    UINT8 md[MD5_DIGEST_LENGTH];
    MD5(itemData, InputData->RightClickeditemModel->modelData->size, md);

    QString hash;
    for (UINT8 i : md) {
        hash += QString("%1").arg(i, 2, 16, QLatin1Char('0'));
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

void BiosViewerWindow::getSHA1() const {
    UINT8 *itemData = InputData->RightClickeditemModel->modelData->data;
    UINT8 md[SHA_DIGEST_LENGTH];
    SHA1(itemData, InputData->RightClickeditemModel->modelData->size, md);

    QString hash;
    for (UINT8 i : md) {
        hash += QString("%1").arg(i, 2, 16, QLatin1Char('0'));
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

void BiosViewerWindow::getSHA224() const {
    UINT8 *itemData = InputData->RightClickeditemModel->modelData->data;
    UINT8 md[SHA224_DIGEST_LENGTH];
    SHA224(itemData, InputData->RightClickeditemModel->modelData->size, md);

    QString hash;
    for (UINT8 i : md) {
        hash += QString("%1").arg(i, 2, 16, QLatin1Char('0'));
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

void BiosViewerWindow::getSHA256() const {
    UINT8 *itemData = InputData->RightClickeditemModel->modelData->data;
    UINT8 md[SHA256_DIGEST_LENGTH];
    SHA256(itemData, InputData->RightClickeditemModel->modelData->size, md);

    QString hash;
    for (UINT8 i : md) {
        hash += QString("%1").arg(i, 2, 16, QLatin1Char('0'));
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

void BiosViewerWindow::getSHA384() const {
    UINT8 *itemData = InputData->RightClickeditemModel->modelData->data;
    UINT8 md[SHA384_DIGEST_LENGTH];
    SHA384(itemData, InputData->RightClickeditemModel->modelData->size, md);

    QString hash;
    for (UINT8 i : md) {
        hash += QString("%1").arg(i, 2, 16, QLatin1Char('0'));
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

void BiosViewerWindow::getSHA512() const {
    UINT8 *itemData = InputData->RightClickeditemModel->modelData->data;
    UINT8 md[SHA512_DIGEST_LENGTH];
    SHA512(itemData, InputData->RightClickeditemModel->modelData->size, md);

    QString hash;
    for (UINT8 i : md) {
        hash += QString("%1").arg(i, 2, 16, QLatin1Char('0'));
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
