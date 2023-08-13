#include <QClipboard>
#include "CapsuleWindow.h"
#include "StartWindow.h"
#include "UEFI/GuidDefinition.h"
#include "HexViewDialog.h"
#include "openssl/sha.h"
#include "openssl/md5.h"

CapsuleWindow::CapsuleWindow(StartWindow *parent):
    QWidget(parent),
    ui(new Ui::CapsuleViewWindow)
{
    initRightMenu();
}

CapsuleWindow::~CapsuleWindow() {
    delete ui;
    safeDelete(data);
    fini();
    finiRightMenu();
}

void CapsuleWindow::setupUi(QMainWindow *MainWindow, GeneralData *wData) {
    WindowData = wData;
    ui->setupUi(MainWindow);
    initSetting();
    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(listWidget_itemSelectionChanged()));
    connect(ui->itemBox,    SIGNAL(activated(int)), this, SLOT(itemBox_activated(int)));
    connect(ui->listWidget, SIGNAL(customContextMenuRequested(QPoint)), this,SLOT(showListRightMenu(QPoint)));
}

void CapsuleWindow::initSetting()
{
    ui->label_mode->clear();
    ui->itemBox->setVisible(false);
    ui->listWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->listWidget->setStyleSheet(QString("QListView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));
    ui->textBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt() + 1));
    ui->AddressPanel->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
}

void CapsuleWindow::fini()
{
    EntryList.clear();
    CapsuleInfo.str("");
    MicrocodeHeaderVector.clear();
    if (buffer != nullptr){
        delete buffer;
    }
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

void CapsuleWindow::addList(const shared_ptr<EntryHeaderClass>& EntryHeader)
{
    EntryList.push_back(EntryHeader);
    QString itemName = QString::fromStdString(EntryHeader->getEntryName());

    QListWidgetItem *item = new QListWidgetItem;
    item->setText(itemName);
    ui->listWidget->addItem(item);
}

void CapsuleWindow::listWidget_itemSelectionChanged()
{
    stringstream Info;

    currentRow = ui->listWidget->currentRow();
    shared_ptr<EntryHeaderClass> EntryHeader = EntryList.at(currentRow);
    EntryHeader->collectInfo(Info);
    ui->textBrowser->setText(QString::fromStdString(Info.str()));
    INT64 offset = EntryHeader->panelGetOffset();
    INT64 size = EntryHeader->panelGetSize();
    setPanelInfo(offset, size);
}

bool CapsuleWindow::tryOpenCapsule(UINT8 *image, INT64 imageLength) {
    if (imageLength <= sizeof(EFI_GUID))
        return false;
    EFI_GUID CapsuleGuid = *(EFI_GUID*)image;
    if (CapsuleGuid != GuidDatabase::gEfiFmpCapsuleGuid) {
        return false;
    }
    return true;
}

void CapsuleWindow::OpenFile(QString path)
{
    fini();
    ui->listWidget->clear();
    showPayloadInfomation(path);
}

void CapsuleWindow::closeEvent(QCloseEvent *event)
{
}

void CapsuleWindow::itemBox_activated(int index)
{
    int itemIndex = ui->itemBox->currentIndex();
    parsePayloadAtIndex(itemIndex);
}


void CapsuleWindow::parseMicrocodeCapsuleInfo(INT64 offset, QString& labelMode, stringstream& CapsuleInfo)
{
    enum Capsule_Mode {FULL = 0, BGUP, SLOT};
    Capsule_Mode CapsuleMode = FULL;

    buffer->setOffset(offset);
    UINT32 XDR = buffer->getUINT32();
    EFI_FIRMWARE_VOLUME_HEADER *MicrocodeFvData = (EFI_FIRMWARE_VOLUME_HEADER*)buffer->getBytes(sizeof(EFI_FIRMWARE_VOLUME_HEADER));
    buffer->getUINT64();
    if (FirmwareVolume::isValidFirmwareVolume(MicrocodeFvData)) {
        UINT32 FvSize = swapEndian<UINT32>(XDR);
        if (offset + sizeof(XDR) + FvSize <= buffer->getBufferSize()) {
            CapsuleMode = FULL;

            EFI_FFS_FILE_HEADER *MicrocodeVersionFfs = (EFI_FFS_FILE_HEADER*)buffer->getBytes(sizeof(EFI_FFS_FILE_HEADER));
            // Find microcode version data. It looks like this:
            // FwVersion = 0x0001
            // LowestSupportedVersion = 0x0001
            // FwVersionString = "Version 0.0.0.1"
            MicrocodeVersion->Decode(*buffer, buffer->getOffset());
            addList(MicrocodeVersion);
            CapsuleInfo << "FwVersion = " << MicrocodeVersion->getFwVersion()
                        << "\nLowestSupportedVersion = " << MicrocodeVersion->getLSV()
                        << "\nFwVersionString = " << MicrocodeVersion->getFwVersionString() << "\n\n";
            safeDelete(MicrocodeVersionFfs);

            // Assume there are more then one uCode in the FFS data area
            FfsFileHeaderClass FfsFileHeaderUcode;
            INT64 FfsUcodeOffset = offset + sizeof(XDR) + 0x1000 - 0x18;
            FfsFileHeaderUcode.Decode(*buffer, FfsUcodeOffset, false); //FFS that includes microcode binary
            INT64 FfsDataSize = FfsFileHeaderUcode.getSize() - FfsFileHeaderUcode.getFfsHeaderSize();
            vector<INT64> MicrocodeOffsetVector = CPUMicrocodeHeaderClass::SearchMicrocodeEntryNum(*buffer, offset + sizeof(XDR) + 0x1000, FfsDataSize);
            INT64 uCodeEntryNum = MicrocodeOffsetVector.size();
            INT64 SlotSize = (INT64)(FfsDataSize / uCodeEntryNum);
            if (SlotSize * uCodeEntryNum != FfsDataSize)
            {
                throw CapsuleError("FfsDataSize is wrong!");
            }
            CapsuleInfo << "uCode SlotSize = " << hex << SlotSize << "\n";

            for (int i = 0; i < MicrocodeOffsetVector.size(); i++)
            {
                INT64 uCodeEntryOffset = MicrocodeOffsetVector[i];
                auto uCodeEntry = make_shared<CPUMicrocodeHeaderClass>();
                uCodeEntry->Decode(*buffer, uCodeEntryOffset);
                MicrocodeHeaderVector.push_back(uCodeEntry);
                CapsuleInfo << "uCode Version = " << uCodeEntry->getUcodeRevision()
                            << " CPU ID = " << uCodeEntry->getUcodeSignature()
                            << " uCode Size = 0x" << hex << uCodeEntry->TotalSize << "\n";
                addList(MicrocodeHeaderVector.at(i));
            }

            // Get Bgup data
            if (offset + sizeof(XDR) + FvSize < buffer->getBufferSize()) {
                CapsuleMode = BGUP;
                buffer->setOffset(offset + sizeof(XDR) + FvSize);
                UINT32 BgupXDR = buffer->getUINT32();
                BgupXDR = swapEndian<UINT32>(BgupXDR);

                INT64 BgupOffset = offset + sizeof(XDR) + FvSize + sizeof(BgupXDR);
                BgupHeader->Decode(*buffer, BgupOffset, BgupXDR, "uCode");
                addList(BgupHeader);
                CapsuleInfo << "\nPlat ID: " << BgupHeader->getPlatId() << endl;
            }
        }
    } else {
        //Slot Mode
        CapsuleMode = SLOT;
        buffer->setOffset(offset + 8);
        UINT32 MicrocodeVersionXDR = buffer->getUINT32();
        MicrocodeVersionXDR = swapEndian<UINT32>(MicrocodeVersionXDR);

        MicrocodeVersion->Decode(*buffer, offset + 8 + sizeof(MicrocodeVersionXDR));
        addList(MicrocodeVersion);
        CapsuleInfo << "FwVersion = " << MicrocodeVersion->getFwVersion()
                    << "\nLowestSupportedVersion = " << MicrocodeVersion->getLSV()
                    << "\nFwVersionString = " << MicrocodeVersion->getFwVersionString() << "\n\n";

        auto SlotMicrocodeEntry = make_shared<CPUMicrocodeHeaderClass>();
        SlotMicrocodeEntry->Decode(*buffer, offset + 8 + sizeof(MicrocodeVersionXDR) + MicrocodeVersionXDR);
        MicrocodeHeaderVector.push_back(SlotMicrocodeEntry);
        CapsuleInfo << "uCode Version = " << SlotMicrocodeEntry->getUcodeRevision()
                    << " CPU ID = " << SlotMicrocodeEntry->getUcodeSignature()
                    << " uCode Size = 0x" << hex << SlotMicrocodeEntry->TotalSize << "\n\n";
        addList(MicrocodeHeaderVector.at(0));
    }

    safeDelete(MicrocodeFvData);

    switch (CapsuleMode) {
    case FULL:
        labelMode = "uCode Full Mode";
        break;
    case BGUP:
        labelMode = "uCode BGUP Mode";
        break;
    case SLOT:
        labelMode = "uCode Slot Mode";
        break;
    default:
        labelMode = "Unknown";
        break;
    }
}

void CapsuleWindow::parseACMCapsuleInfo(INT64 offset, QString &labelMode, stringstream &CapsuleInfo)
{
    INT64 PayloadOffset = offset;
    labelMode = "ACM";
    PayloadOffset = FirmwareVolumeHeader->Decode(*buffer, PayloadOffset, "ACM", true);
    addList(FirmwareVolumeHeader);

    //decode FFS for BtGAcmUpdateConfig.ini
    FfsFileHeaderClass FfsFileHeaderAcmIni;
    PayloadOffset = FfsFileHeaderAcmIni.Decode(*buffer, PayloadOffset, true, offset);
    PayloadOffset = ConfigIni->Decode(*buffer, PayloadOffset, FfsFileHeaderAcmIni.getSize() - 0x18, "BtGAcmUpdateConfig.ini");
    addList(ConfigIni);

    //decode FFS for BtGAcm_pad.bin
    FfsFileHeaderClass FfsFileHeaderACM;
    PayloadOffset = FfsFileHeaderACM.Decode(*buffer, PayloadOffset, true, offset);

    //search BtGAcm_pad.bin
    INT64 FfsDataSize = FfsFileHeaderACM.getSize() - FfsFileHeaderACM.getFfsHeaderSize();
    AcmInstance->Decode(*buffer, PayloadOffset, FfsDataSize);
    addList(AcmInstance);

    //decode FFS for BtGAcmBgup.bin
    PayloadOffset = FfsFileHeader->Decode(*buffer, PayloadOffset + FfsDataSize, true, offset);
    for (BgupConfig &config : ConfigIni->BgupList) {
        auto BgupHeader = make_shared<BgupHeaderClass>();
        BgupHeader->Decode(*buffer, PayloadOffset + config.BgupOffset, config.BgupSize, config.BgupContent);
        BgupHeaderVector.emplace_back(BgupHeader);
        addList(BgupHeader);
    }

    CapsuleInfo << AcmInstance->getAcmVersion()
                << "\n\nPlat ID: " << BgupHeader->getPlatId() << endl;
}

void CapsuleWindow::parseEcCapsuleInfo(INT64 offset, QString &labelMode, stringstream &CapsuleInfo)
{
    INT64 EcPayloadOffset = offset;
    INT64 EcPayloadSize = 0x80000;
    labelMode = "EC";

    // search EC header
    EcInstance->Decode(*buffer, EcPayloadOffset, EcPayloadSize);
    addList(EcInstance);

    //save EC binary
    buffer->setOffset(EcPayloadOffset);
    if (buffer->getRemainingSize() < EcPayloadSize){
        throw CapsuleError("EC binary less than 512K");
    }

    CapsuleInfo << "EC Version : " << EcInstance->getEcVersion() << endl;
}

void CapsuleWindow::parseBiosCapsuleInfo(INT64& offset, QString& labelMode, stringstream& CapsuleInfo)
{
    INT64 PayloadOffset = offset;
    labelMode = "BIOS";
    PayloadOffset = FirmwareVolumeHeader->Decode(*buffer, PayloadOffset, "BIOS", true);
    addList(FirmwareVolumeHeader);

    //decode FFS for BiosUpdateConfig.ini
    FfsFileHeaderClass FfsFileHeaderBiosConfig;
    INT64 BiosConfigPayloadOffset = FfsFileHeaderBiosConfig.Decode(*buffer, PayloadOffset, true, offset);
    PayloadOffset = ConfigIni->Decode(*buffer, BiosConfigPayloadOffset, FfsFileHeaderBiosConfig.getSize() - 0x18, "BiosUpdateConfig.ini");
    addList(ConfigIni);

    //decode FFS for ClientBios_Ft.rom
    FfsFileHeaderClass FfsFileHeaderClientBios;
    INT64 ClientBiosPayloadOffset = FfsFileHeaderClientBios.Decode(*buffer, PayloadOffset, true, offset);
    INT64 ClientBiosPayloadSize = FfsFileHeaderClientBios.getSize() - FfsFileHeaderClientBios.getFfsHeaderSize();

    //search Bios
    BiosInstance->Decode(*buffer, ClientBiosPayloadOffset, ClientBiosPayloadSize);
    if (BiosInstance->isResiliency()) { labelMode = "Resiliency BIOS"; }
    addList(BiosInstance);

    //decode FFS for BiosBgup.bin
    FfsFileHeaderClass FfsFileHeaderBiosBgup;
    INT64 BiosBgupPayloadOffset = FfsFileHeaderBiosBgup.Decode(*buffer, ClientBiosPayloadOffset + ClientBiosPayloadSize, true, offset);
    for (BgupConfig &config : ConfigIni->BgupList) {
        auto BgupHeader = make_shared<BgupHeaderClass>();
        BgupHeader->Decode(*buffer, BiosBgupPayloadOffset + config.BgupOffset, config.BgupSize, config.BgupContent);
        BgupHeaderVector.emplace_back(BgupHeader);
        addList(BgupHeader);
    }

    offset = FfsFileHeaderBiosBgup.getSize() + BiosBgupPayloadOffset - FfsFileHeaderBiosBgup.getFfsHeaderSize();

    CapsuleInfo << "BIOS ID : " << BiosInstance->getBiosIDString()
                << "\n\nPlat ID: " << BgupHeader->getPlatId() << endl;
}

void CapsuleWindow::parseMonoCapsuleInfo(INT64& offset, QString &labelMode, stringstream &CapsuleInfo)
{
    parseBiosCapsuleInfo(offset, labelMode, CapsuleInfo);
    labelMode = "Monolithic";

    // decode FFS for ME
    FfsFileHeaderClass FfsFileHeaderME;
    INT64 MePayloadOffset = FfsFileHeaderME.Decode(*buffer, offset);
    INT64 MePayloadSize = FfsFileHeaderME.getSize() - FfsFileHeaderME.getFfsHeaderSize();
    MeInstance->Decode(*buffer, MePayloadOffset, MePayloadSize);
    addList(MeInstance);

    // decode FFS for EC
    FfsFileHeaderClass FfsFileHeaderEC;
    INT64 EcPayloadOffset = FfsFileHeaderEC.Decode(*buffer, MePayloadOffset + MePayloadSize);
    INT64 EcPayloadSize = FfsFileHeaderEC.getSize() - FfsFileHeaderEC.getFfsHeaderSize();
    EcInstance->Decode(*buffer, EcPayloadOffset, EcPayloadSize);
    addList(EcInstance);

    CapsuleInfo << "\nEC Version : " << EcInstance->getEcVersion() << endl;
}

void CapsuleWindow::parseMeCapsuleInfo(INT64 offset, QString &labelMode, stringstream &CapsuleInfo)
{
    INT64 MePayloadOffset = offset;
    buffer->setOffset(MePayloadOffset);
    INT64 MePayloadSize = buffer->getRemainingSize();
    MeInstance->Decode(*buffer, MePayloadOffset, MePayloadSize);
    addList(MeInstance);

    //save ME binary
    //prepareSaveButton(MePayloadOffset, MePayloadSize, "ME");
}

void CapsuleWindow::parseIfwiCapsuleInfo(INT64& offset, QString& labelMode, stringstream& CapsuleInfo)
{
    INT64 PayloadOffset = offset;
    labelMode = "IFWI";
    PayloadOffset = FirmwareVolumeHeader->Decode(*buffer, PayloadOffset, "IFWI", true);
    addList(FirmwareVolumeHeader);

    //decode FFS for IfwiUpdateConfig.ini
    FfsFileHeaderClass FfsFileHeaderIfwiConfig;
    INT64 BiosConfigPayloadOffset = FfsFileHeaderIfwiConfig.Decode(*buffer, PayloadOffset);
    PayloadOffset = ConfigIni->Decode(*buffer, BiosConfigPayloadOffset, FfsFileHeaderIfwiConfig.getSize() - 0x18, "IfwiUpdateConfig.ini");
    addList(ConfigIni);

    //decode FFS for Ifwi.bin
    FfsFileHeaderClass FfsFileHeaderIfwi;
    INT64 IfwiPayloadOffset = FfsFileHeaderIfwi.Decode(*buffer, PayloadOffset);
    INT64 IfwiPayloadSize = FfsFileHeaderIfwi.getSize() - FfsFileHeaderIfwi.getFfsHeaderSize();
    INT64 EcBeginOffset = IfwiPayloadOffset + 0x1000;
    INT64 MeBeginOffset = EcBeginOffset + 0x80000;
    INT64 BiosBeginOffset = IfwiPayloadOffset + 0x1000000;

    //decode EC
    EcInstance->Decode(*buffer, EcBeginOffset, 0x80000);
    addList(EcInstance);

    //decode ME
    MeInstance->Decode(*buffer, MeBeginOffset, 0); // unable to get ME size
    addList(MeInstance);

    //decode Bios
    BiosInstance->Decode(*buffer, BiosBeginOffset, 0x1000000);
    addList(BiosInstance);

    //decode FFS for IfwiBgup.bin
    FfsFileHeaderClass FfsFileHeaderIfwiBgup;
    INT64 BiosBgupPayloadOffset = FfsFileHeaderIfwiBgup.Decode(*buffer, IfwiPayloadOffset + IfwiPayloadSize);
    for (BgupConfig &config : ConfigIni->BgupList) {
        auto BgupHeader = make_shared<BgupHeaderClass>();
        BgupHeader->Decode(*buffer, BiosBgupPayloadOffset + config.BgupOffset, config.BgupSize, config.BgupContent);
        BgupHeaderVector.emplace_back(BgupHeader);
        addList(BgupHeader);
    }

    CapsuleInfo << "BIOS ID : " << BiosInstance->getBiosIDString()
                << "\n\nPlat ID: " << BgupHeader->getPlatId() << "\n"
                << "\nEC Version : " << EcInstance->getEcVersion() << endl;
}

void CapsuleWindow::parsePayloadAtIndex(INT64 index){
    QString labelMode;
    string CapsuleType = FmpCapsuleHeader->CapsuleTypeList[index];
    INT64 PayloadOffset = FmpCapsuleHeader->FmpCapsuleImageHeaderList[index]->PayloadOffset;

    EntryList.clear();
    CapsuleInfo.str("");
    MicrocodeHeaderVector.clear();
    ui->listWidget->clear();
    addList(CapsuleOverview);
    addList(UefiCapsuleHeader);
    addList(FmpCapsuleHeader);
    addList(FmpCapsuleHeader->FmpCapsuleImageHeaderList[index]);

    try {
        PayloadOffset = FmpAuthHeader->Decode(*buffer, PayloadOffset);
        addList(FmpAuthHeader);

        PayloadOffset = FmpPayloadHeader->Decode(*buffer, PayloadOffset);
        addList(FmpPayloadHeader);

        if (CapsuleType == "uCode") {
            parseMicrocodeCapsuleInfo(PayloadOffset, labelMode, CapsuleInfo);
        }
        else if (CapsuleType == "BtgAcm") {
            parseACMCapsuleInfo(PayloadOffset, labelMode, CapsuleInfo);
        }
        else if (CapsuleType == "BIOS") {
            parseBiosCapsuleInfo(PayloadOffset, labelMode, CapsuleInfo);
        }
        else if (CapsuleType == "Extended BIOS") {
            parseBiosCapsuleInfo(PayloadOffset, labelMode, CapsuleInfo);
            labelMode = "Extended BIOS";
        }
        else if (CapsuleType == "IFWI") {
            parseIfwiCapsuleInfo(PayloadOffset, labelMode, CapsuleInfo);
        }
        else if (CapsuleType == "Monolithic") {
            parseMonoCapsuleInfo(PayloadOffset, labelMode, CapsuleInfo);
        }
        else if (CapsuleType.substr(0, 2) == "ME") {
            labelMode = QString::fromStdString(CapsuleType);
            parseMeCapsuleInfo(PayloadOffset, labelMode, CapsuleInfo);
        }
        else if (CapsuleType == "EC") {
            parseEcCapsuleInfo(PayloadOffset, labelMode, CapsuleInfo);
        }
        else {
            labelMode = "Not supported!";
        }
        CapsuleOverview->setOverviewMsg(CapsuleInfo.str());
    }
    catch (CapsuleError& err) {
        cout << err.what() << endl;
        LabelText = QString("Binary corrupted: ");
        labelMode = QString::fromStdString(err.what());
    }
    ui->label_mode->setText(LabelText + labelMode);
    ui->listWidget->setCurrentRow(0);
}

void CapsuleWindow::showPayloadInfomation(QString filePath)
{
    using namespace CapsuleToolSpace;
    string path = filePath.toStdString();
    buffer = new Buffer(new ifstream(path, ios::in | ios::binary));

    CapsuleOverview      = make_shared<CapsuleOverviewClass>();
    UefiCapsuleHeader    = make_shared<UefiCapsuleHeaderClass>();
    FmpCapsuleHeader     = make_shared<FmpCapsuleHeaderClass>();
    FmpAuthHeader        = make_shared<FmpAuthHeaderClass>();
    FmpPayloadHeader     = make_shared<FmpPayloadHeaderClass>();
    FirmwareVolumeHeader = make_shared<FirmwareVolumeHeaderClass>();
    FfsFileHeader        = make_shared<FfsFileHeaderClass>();
    MicrocodeVersion     = make_shared<MicrocodeVersionClass>();
    ConfigIni            = make_shared<ConfigIniClass>();
    BgupHeader           = make_shared<BgupHeaderClass>();
    BiosInstance         = make_shared<BiosClass>();
    AcmInstance          = make_shared<AcmClass>();
    EcInstance           = make_shared<EcClass>();
    MeInstance           = make_shared<MeClass>();

    CapsuleOverview->Decode(*buffer, 0, buffer->getBufferSize());

    try {
        INT64 offset = UefiCapsuleHeader->Decode(*buffer, 0);
        FmpCapsuleHeader->Decode(*buffer, offset);
    }
    catch (CapsuleError& err) {
        cout << err.what() << endl;
        LabelText = "This is not Capsule file!";
        ui->label_mode->setText(LabelText);
        return;
    }

    //    UINT16 DriverItemCount = FmpCapsuleHeader->getDriverItemCount();


    UINT16 PayloadItemCount = FmpCapsuleHeader->getPayloadItemCount();
    if (PayloadItemCount == 0){
        LabelText = "PayloadItemCount is zero!";
        ui->label_mode->setText(LabelText);
        return;
    }
    else if (PayloadItemCount == 1){
        LabelText = "Capsule: ";
    }
    else if (PayloadItemCount > 1 && PayloadItemCount <= 10){
        ui->itemBox->setVisible(true);
        for(int i = 0; i < PayloadItemCount; ++i){
            ui->itemBox->addItem(QString::fromStdString(FmpCapsuleHeader->CapsuleTypeList[i]));
        }
        LabelText = "Mutiple Payload Capsule: ";
    }
    else{
        LabelText = "Too many Payload Items!";
        ui->label_mode->setText(LabelText);
        return;
    }

    parsePayloadAtIndex(0);
    return;
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

    currentRow = ui->listWidget->currentRow();
    shared_ptr<EntryHeaderClass> EntryHeader = EntryList.at(currentRow);
    INT64 offset = EntryHeader->panelGetOffset();
    INT64 size = EntryHeader->panelGetSize();

    buffer->setOffset(offset);
    UINT8 *itemData = buffer->getBytes(size);
    auto *hexViewData = new QByteArray((char*)itemData, size);

    data = new UefiSpace::Volume(WindowData->InputImage, WindowData->InputImageSize);
    hexDialog->loadBuffer(*hexViewData,
                          data,
                          offset,
                          WindowData->InputImageSize,
                          QString::fromStdString(EntryHeader->getEntryName()),
                          WindowData->OpenedFileName,
                          false);
    hexDialog->show();
    delete[] itemData;
    delete hexViewData;
}

void CapsuleWindow::openInNewTab() {
    currentRow = ui->listWidget->currentRow();
    shared_ptr<EntryHeaderClass> EntryHeader = EntryList.at(currentRow);
    INT64 offset = EntryHeader->panelGetOffset();
    INT64 size = EntryHeader->panelGetSize();

    buffer->setOffset(offset);
    UINT8 *itemData = buffer->getBytes(size);

    WindowData->parentWindow->OpenBuffer(itemData, size, QString::fromStdString(EntryHeader->getEntryName()));
}

void CapsuleWindow::extractCapsuleRegion() {
    currentRow = ui->listWidget->currentRow();
    shared_ptr<EntryHeaderClass> EntryHeader = EntryList.at(currentRow);
    INT64 offset = EntryHeader->panelGetOffset();
    INT64 size = EntryHeader->panelGetSize();
    buffer->setOffset(offset);
    UINT8 *itemData = buffer->getBytes(size);

    QString filename = QString::fromStdString(EntryHeader->getEntryName()) + ".bin";
    QString outputPath = setting.value("LastFilePath").toString() + "/" + filename;
    QString DialogTitle = "Extract " + QString::fromStdString(EntryHeader->getEntryName());
    QString extractVolumeName = QFileDialog::getSaveFileName(this,
                                                             DialogTitle,
                                                             outputPath,
                                                             tr("Files(*.rom *.bin *.fd);;All files (*.*)"));
    if (extractVolumeName.isEmpty()) {
        return;
    }
    saveBinary(extractVolumeName.toStdString(), itemData, 0, size);
    delete[] itemData;
}

void CapsuleWindow::getMD5() {
    currentRow = ui->listWidget->currentRow();
    shared_ptr<EntryHeaderClass> EntryHeader = EntryList.at(currentRow);
    INT64 offset = EntryHeader->panelGetOffset();
    INT64 size = EntryHeader->panelGetSize();
    buffer->setOffset(offset);
    UINT8 *itemData = buffer->getBytes(size);

    UINT8 md[MD5_DIGEST_LENGTH];
    MD5(itemData, size, md);
    delete[] itemData;

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
    currentRow = ui->listWidget->currentRow();
    shared_ptr<EntryHeaderClass> EntryHeader = EntryList.at(currentRow);
    INT64 offset = EntryHeader->panelGetOffset();
    INT64 size = EntryHeader->panelGetSize();
    buffer->setOffset(offset);
    UINT8 *itemData = buffer->getBytes(size);

    UINT8 md[SHA_DIGEST_LENGTH];
    SHA1(itemData, size, md);
    delete[] itemData;

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
    currentRow = ui->listWidget->currentRow();
    shared_ptr<EntryHeaderClass> EntryHeader = EntryList.at(currentRow);
    INT64 offset = EntryHeader->panelGetOffset();
    INT64 size = EntryHeader->panelGetSize();
    buffer->setOffset(offset);
    UINT8 *itemData = buffer->getBytes(size);

    UINT8 md[SHA224_DIGEST_LENGTH];
    SHA224(itemData, size, md);
    delete[] itemData;

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
    currentRow = ui->listWidget->currentRow();
    shared_ptr<EntryHeaderClass> EntryHeader = EntryList.at(currentRow);
    INT64 offset = EntryHeader->panelGetOffset();
    INT64 size = EntryHeader->panelGetSize();
    buffer->setOffset(offset);
    UINT8 *itemData = buffer->getBytes(size);

    UINT8 md[SHA256_DIGEST_LENGTH];
    SHA256(itemData, size, md);
    delete[] itemData;

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
    currentRow = ui->listWidget->currentRow();
    shared_ptr<EntryHeaderClass> EntryHeader = EntryList.at(currentRow);
    INT64 offset = EntryHeader->panelGetOffset();
    INT64 size = EntryHeader->panelGetSize();
    buffer->setOffset(offset);
    UINT8 *itemData = buffer->getBytes(size);

    UINT8 md[SHA384_DIGEST_LENGTH];
    SHA384(itemData, size, md);
    delete[] itemData;

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
    currentRow = ui->listWidget->currentRow();
    shared_ptr<EntryHeaderClass> EntryHeader = EntryList.at(currentRow);
    INT64 offset = EntryHeader->panelGetOffset();
    INT64 size = EntryHeader->panelGetSize();
    buffer->setOffset(offset);
    UINT8 *itemData = buffer->getBytes(size);

    UINT8 md[SHA512_DIGEST_LENGTH];
    SHA512(itemData, size, md);
    delete[] itemData;

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
