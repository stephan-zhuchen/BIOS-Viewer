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
#include "Feature/AcmClass.h"
#include "Feature/MicrocodeClass.h"
#include "IfwiRegion/EcRegion.h"
#include "IfwiRegion/MeRegion.h"
#include "IfwiRegion/PdtRegion.h"
#include "IfwiRegion/GbeRegion.h"
#include "UEFI/GuidDatabase.h"
#include "HexView/HexViewDialog.h"
#include "openssl/sha.h"
#include "openssl/md5.h"

using namespace BaseLibrarySpace;

CapsuleWindow::CapsuleWindow(StartWindow *parent):
    QWidget(parent),
    ui(new Ui::CapsuleViewWindow)
{
    InitCustomMenu();
}

CapsuleWindow::~CapsuleWindow() {
    delete ui;
    safeDelete(CapsuleData);
    CleanupCustomMenu();
}

void CapsuleWindow::setupUi(QMainWindow *MainWindow, GeneralData *wData) {
    WindowData = wData;
    ui->setupUi(MainWindow);
    InitSetting();
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(listWidget_itemSelectionChanged()));
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(QPoint)), this,SLOT(showListCustomMenu(QPoint)));
}

void CapsuleWindow::InitSetting() {
    ui->CapsuleTitle->clear();
    ui->listWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->listWidget->setStyleSheet(QString("QListView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));
    ui->textBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt() + 1));
    ui->AddressPanel->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
}

void CapsuleWindow::InitCustomMenu() {
    CustomMenu = new QMenu;
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

void CapsuleWindow::CleanupCustomMenu() {
    safeDelete(CustomMenu);
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

void CapsuleWindow::setPanelInfo(INT64 offset, INT64 size) {
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
        CapsuleData->CapsuleType = FmpHeader->getCapsuleType();
        if (CapsuleData->CapsuleType == "BIOS" || CapsuleData->CapsuleType == "Extended BIOS" || CapsuleData->CapsuleType == "BtgAcm")
            ParseStandardCapsule(itemOffset + offset, CapsuleData->CapsuleType);
        else if (CapsuleData->CapsuleType == "Monolithic")
            ParseMonolithicCapsule(itemOffset + offset);
        else if (CapsuleData->CapsuleType == "uCode")
            ParseMicrocodeCapsule(itemOffset + offset);
        else
            ParsePayloadCapsule(itemOffset + offset, WindowData->InputImageSize - itemOffset - offset, CapsuleData->CapsuleType);
    }

    addListItem(CapsuleData->VolumeDataList);
    ui->CapsuleTitle->setText("Capsule: " + CapsuleData->CapsuleType);
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

void CapsuleWindow::ParseMonolithicCapsule(INT64 CapsuleOffset) {
    /**
     * Monolithic Capsule Format has 7 parts:
     * 1. BIOS ini Config file;
     * 2. Client Bios Payload;
     * 3. BiosBgup payload.
     * 4. ME Payload.
     * 5. EC payload.
     * 6. Pdt Payload.
     * 7. GbE Payload.
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
    offset = ParsePayloadInFfs(offset, "BIOS");
    if (offset == 0) {
        return;
    }

    // FFS File Header + Bgup Payload
    offset = ParseBgupInFfs(offset, iniVolume);
    if (offset == 0) {
        return;
    }
    Align(offset, CapsuleOffset, 0x8);

    // FFS File Header + ME Payload
    auto MeFile = FfsFile(WindowData->InputImage + offset, WindowData->InputImageSize - offset, offset);
    INT64 MeSize = MeFile.SelfDecode();
    if (MeSize == 0) {
        return;
    }
    offset += MeFile.getHeaderSize();
    offset = ParsePayloadCapsule(offset, MeSize - MeFile.getHeaderSize(), "ME");
    if (offset == 0) {
        return;
    }
    Align(offset, CapsuleOffset, 0x8);

    // FFS File Header + EC Payload
    auto EcFile = FfsFile(WindowData->InputImage + offset, WindowData->InputImageSize - offset, offset);
    INT64 EcSize = EcFile.SelfDecode();
    if (EcSize == 0) {
        return;
    }
    offset += EcFile.getHeaderSize();
    offset = ParsePayloadCapsule(offset, EcSize - EcFile.getHeaderSize(), "EC");
    if (offset == 0) {
        return;
    }
    Align(offset, CapsuleOffset, 0x8);

    // FFS File Header + Pdt Payload
    auto PdtFile = FfsFile(WindowData->InputImage + offset, WindowData->InputImageSize - offset, offset);
    INT64 PdtSize = PdtFile.SelfDecode();
    if (PdtSize == 0) {
        return;
    }
    offset += PdtFile.getHeaderSize();
    offset = ParsePayloadCapsule(offset, PdtSize - PdtFile.getHeaderSize(), "IshPdt");
    if (offset == 0) {
        return;
    }
    Align(offset, CapsuleOffset, 0x8);

    // FFS File Header + GbE Payload
    auto GbeFile = FfsFile(WindowData->InputImage + offset, WindowData->InputImageSize - offset, offset);
    INT64 GbeSize = GbeFile.SelfDecode();
    if (GbeSize == 0) {
        return;
    }
    offset += GbeFile.getHeaderSize();
    offset = ParsePayloadCapsule(offset, GbeSize - GbeFile.getHeaderSize(), "GbE");
    if (offset == 0) {
        return;
    }
    Align(offset, CapsuleOffset, 0x8);
}

INT64 CapsuleWindow::ParsePayloadInFfs(INT64 FfsOffset, const QString& CapsuleType) {
    // FFS File Header + Payload
    auto payloadFile = FfsFile(WindowData->InputImage + FfsOffset, WindowData->InputImageSize - FfsOffset, FfsOffset);
    INT64 payloadSize = payloadFile.SelfDecode();
    if (payloadSize == 0) {
        return 0;
    }
    INT64 offset = FfsOffset + payloadFile.getHeaderSize();
    if (CapsuleType == "BIOS" || CapsuleType == "Extended BIOS") {
        auto biosVolume = new BiosRegion(WindowData->InputImage + offset, payloadFile.getSize() - payloadFile.getHeaderSize(), offset);
        biosVolume->SelfDecode();
        biosVolume->setBiosID();
        CapsuleData->VolumeDataList.append(biosVolume);
        offset += biosVolume->getSize();
    } else if (CapsuleType == "BtgAcm") {
        auto acmVolume = new AcmHeaderClass(WindowData->InputImage + offset, payloadFile.getSize() - payloadFile.getHeaderSize(), offset);
        acmVolume->SelfDecode();
        CapsuleData->VolumeDataList.append(acmVolume);
        offset += acmVolume->getSize();
    }
    return offset;
}

INT64 CapsuleWindow::ParseBgupInFfs(INT64 BgupOffset, IniConfigFile *ConfigIni) {
    // FFS File Header + Bgup Payload
    auto bgupFile = FfsFile(WindowData->InputImage + BgupOffset, WindowData->InputImageSize - BgupOffset, BgupOffset);
    INT64 bgupSize = bgupFile.SelfDecode();
    if (bgupSize == 0) {
        return 0;
    }
    INT64 offset = BgupOffset + bgupFile.getHeaderSize();
    for (BgupConfig &config : ConfigIni->BgupList) {
        auto bgup = new BiosGuardClass(WindowData->InputImage + offset, config.BgupSize, offset);
        bgup->SelfDecode();
        bgup->setVolumeType(VolumeType::UserDefined);
        bgup->setContent(QString::fromStdString(config.BgupContent));
        CapsuleData->VolumeDataList.append(bgup);
        offset += config.BgupOffset;
    }
    return offset;
}

void CapsuleWindow::ParseMicrocodeCapsule(INT64 CapsuleOffset) {
    /**
     * Microcode Capsule Format has 3 parts:
     * 1. Microcode Version file;
     * 2. Microcode Payload;
     * 3. Bgup payload (Optional).
     */
     // Do not Support Slot Mode Microcode Capsule (This mode doesn't follow standard capsule format)
    CapsuleOffset += 0x4; // XDR Header

    // Firmware Header
    INT64 offset = CapsuleOffset;
    auto fv = FirmwareVolume(WindowData->InputImage + offset, WindowData->InputImageSize - offset, offset);
    INT64 fvSize = fv.SelfDecode();
    if (fvSize == 0) {
        return;
    }
    offset += fv.getHeaderSize();
    Align(offset, CapsuleOffset, 0x8);

    // FFS File Header + Microcode Version file
    auto versionFfs = FfsFile(WindowData->InputImage + offset, WindowData->InputImageSize - offset, offset);
    INT64 versionFfsSize = versionFfs.SelfDecode();
    if (versionFfsSize == 0) {
        return;
    }
    offset += versionFfs.getHeaderSize();
    auto version = new MicrocodeVersion(WindowData->InputImage + offset, versionFfs.getSize() - versionFfs.getHeaderSize(), offset);
    version->SelfDecode();
    CapsuleData->VolumeDataList.append(version);
    offset += version->getSize();

    // FFS File Header + Microcode Payload
    // Microcode address is located at an offset of 0x1000 after the Firmware header.
    INT64 microcodeFfsOffset = CapsuleOffset + 0x1000 - sizeof(EFI_FFS_FILE_HEADER);
    auto microcodeFfs = FfsFile(WindowData->InputImage + microcodeFfsOffset, WindowData->InputImageSize - microcodeFfsOffset, microcodeFfsOffset);
    INT64 microcodeFfsSize = microcodeFfs.SelfDecode();
    if (microcodeFfsSize == 0) {
        return;
    }
    offset = microcodeFfsOffset + microcodeFfs.getHeaderSize();
    QVector<INT64> MicrocodeOffsetVector = MicrocodeHeaderClass::SearchMicrocodeEntryNum(WindowData->InputImage + offset, microcodeFfs.getSize() - microcodeFfs.getHeaderSize());
    for (INT64 microcodeOffset : MicrocodeOffsetVector) {
        auto microcode = new MicrocodeHeaderClass(WindowData->InputImage + offset + microcodeOffset, microcodeFfs.getSize() - microcodeFfs.getHeaderSize() - microcodeOffset, offset + microcodeOffset);
        INT64 microcodeSize = microcode->SelfDecode();
        microcode->setVolumeType(VolumeType::UserDefined);
        if (microcodeSize == 0) {
            continue;
        }
        CapsuleData->VolumeDataList.append(microcode);
    }

    // XDR Header + Bgup Payload (Optional)
    INT64 bgupFfsOffset = CapsuleOffset + fvSize;
    if (bgupFfsOffset < WindowData->InputImageSize) {
        UINT32 XDR = *(UINT32 *)(WindowData->InputImage + bgupFfsOffset);
        UINT32 BgupSize = swapEndian<UINT32>(XDR);
        bgupFfsOffset += 4;
        auto bgup = new BiosGuardClass(WindowData->InputImage + bgupFfsOffset, BgupSize, bgupFfsOffset);
        bgup->SelfDecode();
        bgup->setVolumeType(VolumeType::UserDefined);
        bgup->setContent("uCode");
        CapsuleData->VolumeDataList.append(bgup);
    }
}

INT64 CapsuleWindow::ParsePayloadCapsule(INT64 CapsuleOffset, INT64 PayloadSize, const QString& CapsuleType) {
    /**
     * Payload Capsule Format has only 1 parts:
     * 1. Payload file;
     */
    if (CapsuleType == "EC") {
        auto EcVolume = new EcRegion(WindowData->InputImage + CapsuleOffset, PayloadSize, CapsuleOffset);
        EcVolume->SelfDecode();
        CapsuleData->VolumeDataList.append(EcVolume);
        return EcVolume->getSize() + CapsuleOffset;
    } else if (CapsuleType == "ME") {
        auto MeVolume = new MeRegion(WindowData->InputImage + CapsuleOffset, PayloadSize, CapsuleOffset);
        MeVolume->SelfDecode();
        CapsuleData->VolumeDataList.append(MeVolume);
        return MeVolume->getSize() + CapsuleOffset;
    } else if (CapsuleType == "IshPdt") {
        auto PdtVolume = new PdtRegion(WindowData->InputImage + CapsuleOffset, PayloadSize, CapsuleOffset);
        PdtVolume->SelfDecode();
        CapsuleData->VolumeDataList.append(PdtVolume);
        return PdtVolume->getSize() + CapsuleOffset;
    } else if (CapsuleType == "GbE") {
        auto GbeVolume = new GbeRegion(WindowData->InputImage + CapsuleOffset, PayloadSize, CapsuleOffset);
        GbeVolume->SelfDecode();
        CapsuleData->VolumeDataList.append(GbeVolume);
        return GbeVolume->getSize() + CapsuleOffset;
    }

    return 0;
}

void CapsuleWindow::showListCustomMenu(const QPoint &pos) {
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

    CustomMenu->clear();
    showHex->setIcon(hexBinary);
    CustomMenu->addAction(showHex);

    ExtractRegion->setIcon(box_arrow_up);
    CustomMenu->addAction(ExtractRegion);

    openTab->setIcon(open);
    CustomMenu->addAction(openTab);

    DigestMenu->setIcon(key);
    DigestMenu->addAction(md5_Menu);
    DigestMenu->addAction(sha1_Menu);
    DigestMenu->addAction(sha224_Menu);
    DigestMenu->addAction(sha256_Menu);
    DigestMenu->addAction(sha384_Menu);
    DigestMenu->addAction(sha512_Menu);
    CustomMenu->addMenu(DigestMenu);

    CustomMenu->move(ui->listWidget->cursor().pos());
    CustomMenu->show();
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
    for (Volume *volume : VolumeDataList) {
        safeDelete(volume);
    }
}

bool CapsuleViewerData::isValidCapsule(UINT8 *image, INT64 imageLength) {
    return false;
}
