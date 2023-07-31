#include "CapsuleWindow.h"
#include "StartWindow.h"
#include "UEFI/GuidDefinition.h"

CapsuleWindow::CapsuleWindow(StartWindow *parent):
    QWidget(parent),
    ui(new Ui::CapsuleViewWindow)
{
}

CapsuleWindow::~CapsuleWindow() {
    delete ui;
    fini();
}

void CapsuleWindow::setupUi(QMainWindow *MainWindow, GeneralData *wData) {
    ui->setupUi(MainWindow);
    initSetting();
    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listWidget_itemClicked(QListWidgetItem*)));
    connect(ui->listWidget, SIGNAL(itemSelectionChanged()), this, SLOT(listWidget_itemSelectionChanged()));
    connect(ui->itemBox, SIGNAL(activated(int)), this, SLOT(itemBox_activated(int)));
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

    QListWidgetItem *item2 = new QListWidgetItem;
    item2->setText(itemName);
    ui->listWidget->addItem(item2);
}

void CapsuleWindow::listWidget_itemClicked(QListWidgetItem *item)
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

void CapsuleWindow::listWidget_itemSelectionChanged()
{
    if (ui->listWidget->currentRow() == currentRow)
    {
        ui->AddressPanel->clear();
        ui->textBrowser->setText(QString::fromStdString(CapsuleInfo.str()));
    }
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
    INT64 PayloadOffset = offset;
    QString CapsuleMode = "uCodeFull";
    try {
        // Try to find FV header.
        PayloadOffset = FirmwareVolumeHeader->Decode(*buffer, PayloadOffset, "uCode", true);
        addList(FirmwareVolumeHeader);
        FfsFileHeaderClass FfsFileHeaderUcodeVersion;
        PayloadOffset = FfsFileHeaderUcodeVersion.Decode(*buffer, PayloadOffset, false); //FFS that includes microcode version data
    }
    catch (CapsuleException&) {
        // If there is no FV header, then this is slot mode.
        PayloadOffset = FirmwareVolumeHeader->Decode(*buffer, PayloadOffset, "uCode", false);
        CapsuleMode = "uCodeSlot";
    }
    catch (CapsuleError& err) {
        cout << err.what() << endl;
        labelMode = "Binary corrupted!";
        return;
    }

    try {
        // Find microcode version data. It looks like this:
        // FwVersion = 0x0001
        // LowestSupportedVersion = 0x0001
        // FwVersionString = "Version 0.0.0.1"
        PayloadOffset = MicrocodeVersion->Decode(*buffer, PayloadOffset);
        addList(MicrocodeVersion);

        CapsuleInfo << "FwVersion = " << MicrocodeVersion->getFwVersion()
                    << "\nLowestSupportedVersion = " << MicrocodeVersion->getLSV()
                    << "\nFwVersionString = " << MicrocodeVersion->getFwVersionString() << "\n\n";

        if (CapsuleMode == "uCodeSlot") {
            auto SlotMicrocodeEntry = make_shared<CPUMicrocodeHeaderClass>();
            SlotMicrocodeEntry->Decode(*buffer, PayloadOffset + 4);
            MicrocodeHeaderVector.push_back(SlotMicrocodeEntry);
            CapsuleInfo << "uCode Version = " << SlotMicrocodeEntry->getUcodeRevision()
                        << " CPU ID = " << SlotMicrocodeEntry->getUcodeSignature()
                        << " uCode Size = 0x" << hex << SlotMicrocodeEntry->TotalSize << "\n\n";
            addList(MicrocodeHeaderVector.at(0));
        }
        else {
            // Assume there are more then one uCode in the FFS data area
            FfsFileHeaderClass FfsFileHeaderUcode;
            INT64 FfsUcodeOffset = FirmwareVolumeHeader->uCodeBeginOffset - 0x18;
            PayloadOffset = FfsFileHeaderUcode.Decode(*buffer, FfsUcodeOffset, false); //FFS that includes microcode binary
            INT64 FfsDataSize = FfsFileHeaderUcode.getSize() - FfsFileHeaderUcode.getFfsHeaderSize();
            vector<INT64> MicrocodeOffsetVector = CPUMicrocodeHeaderClass::SearchMicrocodeEntryNum(*buffer, FirmwareVolumeHeader->uCodeBeginOffset, FfsDataSize);
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
        }

        if (CapsuleMode != "uCodeSlot") {
            // Try to find Bgup data, if Bgup data doesn't exist, throw CapsuleException
            INT64 BgupOffset = FirmwareVolumeHeader->FvBeginOffset + FirmwareVolumeHeader->getFvLength();
            if (BgupHeader->SearchBgup(*buffer, BgupOffset))
            {
                CapsuleMode = "uCodeBgup";
                BgupHeader->Decode(*buffer, BgupOffset + 4);
                addList(BgupHeader);
                CapsuleInfo << "\nPlat ID: " << BgupHeader->getPlatId() << endl;
            }
            else
            {
                CapsuleMode = "uCodeFull";
            }
        }
    }
    catch (CapsuleError& err) {
        cout << err.what() << endl;
        labelMode = QString::fromStdString(err.what());
        return;
    }
    labelMode = CapsuleMode;
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
    BgupHeader->Decode(*buffer, PayloadOffset);
    addList(BgupHeader);

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
    BgupHeader->Decode(*buffer, BiosBgupPayloadOffset);
    addList(BgupHeader);

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
    BgupHeader->Decode(*buffer, BiosBgupPayloadOffset);
    addList(BgupHeader);

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
        ui->textBrowser->setText(QString::fromStdString(CapsuleInfo.str()));
    }
    catch (CapsuleError& err) {
        cout << err.what() << endl;
        LabelText = QString("Binary corrupted: ");
        labelMode = QString::fromStdString(err.what());
    }
    ui->label_mode->setText(LabelText + labelMode);
}

void CapsuleWindow::showPayloadInfomation(QString filePath)
{
    using namespace CapsuleToolSpace;
    string path = filePath.toStdString();
    buffer = new Buffer(new ifstream(path, ios::in | ios::binary));

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
