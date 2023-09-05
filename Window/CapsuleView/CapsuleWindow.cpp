#include <QClipboard>
#include <QMenu>
#include "BaseLib.h"
#include "CapsuleWindow.h"
#include "BiosView/BiosWindow.h"
#include "Start/StartWindow.h"
#include "Capsule/CapsuleHeader.h"
#include "UefiFileSystem/FirmwareVolume.h"
#include "UefiFileSystem/FfsFile.h"
#include "Feature/BiosGuardClass.h"
#include "UEFI/GuidDefinition.h"
#include "HexView/HexViewDialog.h"
#include "openssl/sha.h"
#include "openssl/md5.h"

using namespace BaseLibrarySpace;

CapsuleWindow::CapsuleWindow(StartWindow *parent):
    QWidget(parent),
    ui(new Ui::CapsuleViewWindow)
{
    initRightMenu();
}

CapsuleWindow::~CapsuleWindow() {
    delete ui;
    safeDelete(CapsuleData);
    fini();
    finiRightMenu();
}

void CapsuleWindow::setupUi(QMainWindow *MainWindow, GeneralData *wData) {
    WindowData = wData;
    ui->setupUi(MainWindow);
    initSetting();
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(listWidget_itemSelectionChanged()));
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(QPoint)), this,SLOT(showListRightMenu(QPoint)));
}

void CapsuleWindow::initSetting() {
    ui->label_mode->clear();
    ui->listWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->listWidget->setStyleSheet(QString("QListView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));
    ui->textBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt() + 1));
    ui->AddressPanel->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
}

void CapsuleWindow::fini() {
}

void CapsuleWindow::initRightMenu() {
    RightMenu = new QMenu;
    DigestMenu = new QMenu("Digest");

    showHex = new QAction("Hex View");
    connect(showHex,SIGNAL(triggered(bool)),this,SLOT(showHexView()));

    openTab = new QAction("Open in BIOS Viewer");
    connect(openTab,SIGNAL(triggered(bool)),this,SLOT(openInNewTab()));

    ExtractRegion = new QAction("Extract Binary");
    connect(ExtractRegion,SIGNAL(triggered(bool)),this,SLOT(extractCapsuleRegion()));

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

void CapsuleWindow::finiRightMenu() {
    safeDelete(RightMenu);
    safeDelete(DigestMenu);
    safeDelete(showHex);
    safeDelete(openTab);
    safeDelete(ExtractRegion);
    safeDelete(md5_Menu);
    safeDelete(sha1_Menu);
    safeDelete(sha224_Menu);
    safeDelete(sha256_Menu);
    safeDelete(sha384_Menu);
    safeDelete(sha512_Menu);
}

void CapsuleWindow::setPanelInfo(INT64 offset, INT64 size)
{
    stringstream Info;
    Info.setf(ios::left);
    Info << "Offset = 0x";
    Info.width(10);
    Info << hex << uppercase << offset;

    Info << "Length = 0x";
    Info << hex << uppercase << size;

    QString panelInfo = QString::fromStdString(Info.str());
    ui->AddressPanel->setText(panelInfo);
}

void CapsuleWindow::addListItem(const QList<Volume*> &volumeList) {
    for (Volume *volume : volumeList) {
        DataModel dataModel;
        dataModel.InitFromVolume(volume);
        QString volumeName = dataModel.getName();
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(volumeName);
        ui->listWidget->addItem(item);
    }
}

void CapsuleWindow::listWidget_itemSelectionChanged() {
    INT32 currentRow = ui->listWidget->currentRow();
    Volume *Entry = CapsuleData->VolumeDataList.at(currentRow);
    Entry->setInfoStr();
    ui->textBrowser->setText(Entry->getInfoText());
    INT64 offset = Entry->getOffset();
    INT64 size = Entry->getSize();
    setPanelInfo(offset, size);
}

bool CapsuleWindow::tryOpenCapsule(const UINT8 *image, INT64 imageLength) {
    if (imageLength <= sizeof(EFI_GUID))
        return false;
    EFI_GUID CapsuleGuid = *(EFI_GUID*)image;
    if (CapsuleGuid != GuidDatabase::gEfiFmpCapsuleGuid) {
        return false;
    }
    return true;
}

void CapsuleWindow::LoadCapsule() {
    CapsuleData = new CapsuleViewerData;
    CapsuleData->OverviewVolume = new Volume(WindowData->InputImage, WindowData->InputImageSize);
    CapsuleData->OverviewVolume->setVolumeType(VolumeType::Overview);
    CapsuleData->VolumeDataList.append(CapsuleData->OverviewVolume);

    auto *CapsuleOverview = new CapsuleCommonHeader(WindowData->InputImage, WindowData->InputImageSize, 0);
    INT64 offset = CapsuleOverview->SelfDecode();
    if (offset == 0) {
        delete CapsuleOverview;
        return;
    }
    CapsuleData->VolumeDataList.append(CapsuleOverview);

    for (INT64 itemOffset : CapsuleOverview->ItemOffsetVector) {
        auto *FmpHeader = new FirmwareManagementHeader(WindowData->InputImage + itemOffset, WindowData->InputImageSize - itemOffset, itemOffset);
        offset = FmpHeader->SelfDecode();
        if (offset == 0) {
            delete FmpHeader;
            continue;
        }
        CapsuleData->VolumeDataList.append(FmpHeader);
        QString capsuleType = FmpHeader->getCapsuleType();
        ParseStandardCapsule(itemOffset + offset, capsuleType);
    }

    addListItem(CapsuleData->VolumeDataList);
}

void CapsuleWindow::ParseStandardCapsule(INT64 CapsuleOffset, const QString& CapsuleType) {
    /**
     * Standard Capsule Format has 3 parts:
     * 1. ini Config file;
     * 2. Payload;
     * 3. Bgup payload.
     */

    // Firmware Header
    INT64 offset = CapsuleOffset;
    auto fv = FirmwareVolume(WindowData->InputImage + offset, WindowData->InputImageSize - offset, offset);
    INT64 fvSize = fv.SelfDecode();
    if (fvSize == 0) {
        return;
    }
    offset += fv.getHeaderSize();

    // FFS File Header + Ini Config File
    Align(offset, CapsuleOffset, 0x8);
    auto iniFile = FfsFile(WindowData->InputImage + offset, WindowData->InputImageSize - offset, offset);
    INT64 iniSize = iniFile.SelfDecode();
    if (iniSize == 0) {
        return;
    }
    offset += iniFile.getHeaderSize();
    auto iniVolume = new IniConfigFile(WindowData->InputImage + offset, iniFile.getSize() - iniFile.getHeaderSize(), offset);
    iniVolume->SelfDecode();
    CapsuleData->VolumeDataList.append(iniVolume);
    offset += iniVolume->getSize();
    Align(offset, CapsuleOffset, 0x8);

    // FFS File Header + Payload
    offset = ParsePayloadInFfs(offset, CapsuleType);
    if (offset == 0) {
        return;
    }

    // FFS File Header + Bgup Payload
    ParseBgupInFfs(offset, iniVolume);
}

INT64 CapsuleWindow::ParsePayloadInFfs(INT64 FfsOffset, const QString& CapsuleType) {
    // FFS File Header + Payload
    auto payloadFile = FfsFile(WindowData->InputImage + FfsOffset, WindowData->InputImageSize - FfsOffset, FfsOffset);
    INT64 payloadSize = payloadFile.SelfDecode();
    if (payloadSize == 0) {
        return 0;
    }
    INT64 offset = FfsOffset + payloadFile.getHeaderSize();
    if (CapsuleType == "BIOS") {
        auto biosVolume = new BiosRegion(WindowData->InputImage + offset, payloadFile.getSize() - payloadFile.getHeaderSize(), offset);
        biosVolume->SelfDecode();
        biosVolume->setBiosID();
        CapsuleData->VolumeDataList.append(biosVolume);
        offset += biosVolume->getSize();
    }
    return offset;
}

void CapsuleWindow::ParseBgupInFfs(INT64 BgupOffset, IniConfigFile *ConfigIni) {
    // FFS File Header + Bgup Payload
    auto bgupFile = FfsFile(WindowData->InputImage + BgupOffset, WindowData->InputImageSize - BgupOffset, BgupOffset);
    INT64 bgupSize = bgupFile.SelfDecode();
    if (bgupSize == 0) {
        return;
    }
    INT64 offset = BgupOffset + bgupFile.getHeaderSize();
    for (BgupConfig &config : ConfigIni->BgupList) {
        auto bgup = new BiosGuardClass(WindowData->InputImage + offset + config.BgupOffset, config.BgupSize, offset + config.BgupOffset);
        bgup->SelfDecode();
        CapsuleData->VolumeDataList.append(bgup);
    }
}

void CapsuleWindow::closeEvent(QCloseEvent *event)
{
}

void CapsuleWindow::showListRightMenu(const QPoint &pos) {
    QIcon hexBinary, box_arrow_up, open, key;
    if (WindowData->DarkmodeFlag) {
        hexBinary = QIcon(":/file-binary_light.svg");
        box_arrow_up = QIcon(":/box-arrow-up_light.svg");
        open = QIcon(":/open_light.svg");
        key = QIcon(":/key_light.svg");
    } else {
        hexBinary = QIcon(":/file-binary.svg");
        box_arrow_up = QIcon(":/box-arrow-up.svg");
        open = QIcon(":/open.svg");
        key = QIcon(":/key.svg");
    }

    QModelIndex index = ui->listWidget->indexAt(pos);
    if (!index.isValid())
        return;
    Volume *Entry = CapsuleData->VolumeDataList.at(ui->listWidget->currentRow());
    CapsuleData->RightClickedItemModel.InitFromVolume(Entry);

    RightMenu->clear();
    showHex->setIcon(hexBinary);
    RightMenu->addAction(showHex);

    ExtractRegion->setIcon(box_arrow_up);
    RightMenu->addAction(ExtractRegion);

    openTab->setIcon(open);
    RightMenu->addAction(openTab);

    DigestMenu->setIcon(key);
    DigestMenu->addAction(md5_Menu);
    DigestMenu->addAction(sha1_Menu);
    DigestMenu->addAction(sha224_Menu);
    DigestMenu->addAction(sha256_Menu);
    DigestMenu->addAction(sha384_Menu);
    DigestMenu->addAction(sha512_Menu);
    RightMenu->addMenu(DigestMenu);

    RightMenu->move(ui->listWidget->cursor().pos());
    RightMenu->show();
}

void CapsuleWindow::showHexView() {
    auto *hexDialog = new HexViewDialog(WindowData->DarkmodeFlag);
    if (WindowData->DarkmodeFlag) {
        hexDialog->setWindowIcon(QIcon(":/file-binary_light.svg"));
    }

    INT32 currentRow = ui->listWidget->currentRow();
    Volume *Entry = CapsuleData->VolumeDataList.at(currentRow);
    INT64 offset = Entry->getOffset();
    INT64 size = Entry->getSize();

    auto *hexViewData = new QByteArray((char*)Entry->getData(), size);

    hexDialog->loadBuffer(*hexViewData,
                          CapsuleData->OverviewVolume,
                          offset,
                          WindowData->InputImageSize,
                          CapsuleData->RightClickedItemModel.getName(),
                          WindowData->OpenedFileName,
                          false);
    hexDialog->show();
    delete hexViewData;
}

void CapsuleWindow::openInNewTab() {
    INT32 currentRow = ui->listWidget->currentRow();
    Volume *Entry = CapsuleData->VolumeDataList.at(currentRow);
    INT64 size = Entry->getSize();
    auto *itemData = new UINT8[size];
    memcpy(itemData, Entry->getData(), size);

    WindowData->parentWindow->OpenBuffer(itemData, size, CapsuleData->RightClickedItemModel.getName());
}

void CapsuleWindow::extractCapsuleRegion() {
    INT32 currentRow = ui->listWidget->currentRow();
    Volume *Entry = CapsuleData->VolumeDataList.at(currentRow);
    INT64 size = Entry->getSize();
    UINT8 *itemData = Entry->getData();

    QString filename = CapsuleData->RightClickedItemModel.getName() + ".bin";
    QString outputPath = setting.value("LastFilePath").toString() + "/" + filename;
    QString DialogTitle = "Extract " + CapsuleData->RightClickedItemModel.getName();
    QString extractVolumeName = QFileDialog::getSaveFileName(this,
                                                             DialogTitle,
                                                             outputPath,
                                                             tr("Files(*.rom *.bin *.fd);;All files (*.*)"));
    if (extractVolumeName.isEmpty()) {
        return;
    }
    saveBinary(extractVolumeName.toStdString(), itemData, 0, size);
}

void CapsuleWindow::getMD5() {
    INT32 currentRow = ui->listWidget->currentRow();
    Volume *Entry = CapsuleData->VolumeDataList.at(currentRow);
    INT64 size = Entry->getSize();
    UINT8 *itemData = Entry->getData();

    UINT8 md[MD5_DIGEST_LENGTH];
    MD5(itemData, size, md);

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

void CapsuleWindow::getSHA1() {
    INT32 currentRow = ui->listWidget->currentRow();
    Volume *Entry = CapsuleData->VolumeDataList.at(currentRow);
    INT64 size = Entry->getSize();
    UINT8 *itemData = Entry->getData();

    UINT8 md[SHA_DIGEST_LENGTH];
    SHA1(itemData, size, md);

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

void CapsuleWindow::getSHA224(){
    INT32 currentRow = ui->listWidget->currentRow();
    Volume *Entry = CapsuleData->VolumeDataList.at(currentRow);
    INT64 size = Entry->getSize();
    UINT8 *itemData = Entry->getData();

    UINT8 md[SHA224_DIGEST_LENGTH];
    SHA224(itemData, size, md);

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

void CapsuleWindow::getSHA256() {
    INT32 currentRow = ui->listWidget->currentRow();
    Volume *Entry = CapsuleData->VolumeDataList.at(currentRow);
    INT64 size = Entry->getSize();
    UINT8 *itemData = Entry->getData();

    UINT8 md[SHA256_DIGEST_LENGTH];
    SHA256(itemData, size, md);

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

void CapsuleWindow::getSHA384() {
    INT32 currentRow = ui->listWidget->currentRow();
    Volume *Entry = CapsuleData->VolumeDataList.at(currentRow);
    INT64 size = Entry->getSize();
    UINT8 *itemData = Entry->getData();

    UINT8 md[SHA384_DIGEST_LENGTH];
    SHA384(itemData, size, md);

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

void CapsuleWindow::getSHA512() {
    INT32 currentRow = ui->listWidget->currentRow();
    Volume *Entry = CapsuleData->VolumeDataList.at(currentRow);
    INT64 size = Entry->getSize();
    UINT8 *itemData = Entry->getData();

    UINT8 md[SHA512_DIGEST_LENGTH];
    SHA512(itemData, size, md);

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

CapsuleViewerData::~CapsuleViewerData() {
    safeDelete(OverviewVolume);
}

bool CapsuleViewerData::isValidCapsule(UINT8 *image, INT64 imageLength) {
    return false;
}
