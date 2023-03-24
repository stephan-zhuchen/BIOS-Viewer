#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMimeData>
#include <QStyleFactory>
#include <QInputDialog>
#include <QElapsedTimer>
#include "mainwindow.h"
#include "HexViewDialog.h"
#include "SettingsDialog.h"
#include "InfoWindow.h"
#include "./ui_mainwindow.h"
#include "include/GuidDefinition.h"

GuidDatabase *guidData = nullptr;
UINT32       OpenedWindow = 0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      buffer(nullptr),
      popMenu(new QMenu),
      structureLabel(new QLabel("Structure:", this)),
      infoLabel(new QLabel("Infomation:", this)),
      DarkmodeFlag(false),
      BiosValidFlag(true),
      InputImage(nullptr),
      InputImageModel(nullptr),
      BiosImage(nullptr)
{
    ui->setupUi(this);
    ui->titleInfomation->clear();
    ui->treeWidget->header()->setResizeContentsPrecision(QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setStretchLastSection(true);
    ui->treeWidget->viewport()->installEventFilter(this);
    structureLabel->setFont(QFont("Microsoft YaHei UI", 10));
    infoLabel->setFont(QFont("Microsoft YaHei UI", 10));
    structureLabel->setGeometry(15, 80, 100, 20);
    infoLabel->setGeometry(ui->treeWidget->width() + 23, 80, 100, 20);

    connect(ui->OpenFile,           SIGNAL(triggered()), this, SLOT(OpenFileTriggered()));
    connect(ui->actionExit,         SIGNAL(triggered()), this, SLOT(ActionExitTriggered()));
    connect(ui->actionSettings,     SIGNAL(triggered()), this, SLOT(ActionSettingsTriggered()));
    connect(ui->actionAboutQt,      SIGNAL(triggered()), this, SLOT(ActionAboutQtTriggered()));
    connect(ui->actionAboutBiosViewer, SIGNAL(triggered()), this, SLOT(ActionAboutBiosViewerTriggered()));
    connect(ui->OpenInNewWindow,    SIGNAL(triggered()), this, SLOT(OpenInNewWindowTriggered()));
    connect(ui->treeWidget,         SIGNAL(itemSelectionChanged()), this, SLOT(TreeWidgetItemSelectionChanged()));
    connect(ui->infoButton,         SIGNAL(clicked()),   this, SLOT(InfoButtonClicked()));
    connect(ui->searchButton,       SIGNAL(clicked()),   this, SLOT(SearchButtonClicked()));
    connect(ui->actionSeperate_Binary, SIGNAL(triggered()), this, SLOT(ActionSeperateBinaryTriggered()));
    connect(ui->actionExtract_BIOS, SIGNAL(triggered()), this, SLOT(ActionExtractBIOSTriggered()));
    connect(ui->actionSearch,       SIGNAL(triggered()), this, SLOT(ActionSearchTriggered()));
    connect(ui->actionGoto,         SIGNAL(triggered()), this, SLOT(ActionGotoTriggered()));
    connect(ui->actionCollapse,     SIGNAL(triggered()), this, SLOT(ActionCollapseTriggered()));
    connect(ui->actionReplace_BIOS, SIGNAL(triggered()), this, SLOT(ActionReplaceBIOSTriggered()));

    initSettings();

    this->connect(ui->treeWidget,SIGNAL(customContextMenuRequested(QPoint)),
                      this,SLOT(showTreeRightMenu(QPoint)));

    guidData = new GuidDatabase;
    OpenedWindow += 1;
}

MainWindow::~MainWindow()
{
    OpenedWindow -= 1;
    if (guidData != nullptr && OpenedWindow == 0)
        delete guidData;
    delete structureLabel;
    delete infoLabel;
    delete ui;
    cleanup();
}

void MainWindow::cleanup() {
    this->setWindowTitle("BIOS Viewer");
    BiosValidFlag = true;
    for (auto FvModel:FvModelData) {
        delete FvModel;
    }
    FvModelData.clear();

    for (auto ME_Model:ME_ModelData) {
        delete ME_Model;
    }
    ME_ModelData.clear();

    for (auto IWFI_Model:IFWI_ModelData) {
        delete IWFI_Model;
    }
    IFWI_ModelData.clear();

    for (auto FirmwareVolume:FirmwareVolumeData) {
        delete FirmwareVolume;
    }
    FirmwareVolumeData.clear();

    for (auto IWFI_Section:IFWI_Sections) {
        delete IWFI_Section;
    }
    IFWI_Sections.clear();

    for (UINT8* fvBuffer:FirmwareVolumeBuffer) {
        delete[] fvBuffer;
    }
    FirmwareVolumeBuffer.clear();

    if (BiosImage != nullptr)
        delete BiosImage;
    if (InputImage != nullptr)
        delete InputImage;
}

bool MainWindow::isDarkMode() {
    return DarkmodeFlag;
}

void MainWindow::refresh() {
    initSettings();
    QTreeWidgetItem *ImageOverviewItem = ui->treeWidget->itemAt(0, 0);
    if (ui->treeWidget->indexOfTopLevelItem(ImageOverviewItem) == -1)
        return;
    ImageOverviewItem->setFont(MainWindow::Name, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 1, 700));
    ImageOverviewItem->setFont(MainWindow::Type, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 1, 700));
    ImageOverviewItem->setFont(MainWindow::SubType, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 1, 700));
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    INT32 width = ui->treeWidget->width();
    ui->treeWidget->setColumnWidth(MainWindow::Name, width - 300);
    ui->treeWidget->setColumnWidth(MainWindow::Type, 100);
    infoLabel->setGeometry(ui->treeWidget->width() + 23, 80, 100, 20);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->treeWidget->viewport()) {
        //点击树的空白,取消选中
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *e = (QMouseEvent *)event;
            if (e->buttons() & Qt::LeftButton) {
                QModelIndex index = ui->treeWidget->indexAt(e->pos());
                if (!index.isValid()) {
                    ui->treeWidget->setCurrentIndex(QModelIndex());
                    ui->infoBrowser->clear();
                    ui->AddressPanel->clear();
                }
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) //拖动文件到窗口，触发
{
    if(event->mimeData()->hasUrls())
        event->acceptProposedAction(); //事件数据中存在路径，方向事件
    else
        event->ignore();
}

void MainWindow::dropEvent(QDropEvent* event) {
    QUrl url = event->mimeData()->urls().first();
    QFileInfo file(url.toLocalFile());
    QString suffixs = "rom bin fd";
    if( file.isFile() && suffixs.contains(file.suffix()))
    {
        OpenedFileName = file.filePath();
        ui->treeWidget->clear();
        ui->titleInfomation->clear();
        ui->infoBrowser->clear();
        ui->AddressPanel->clear();
        cleanup();
        OpenFile(OpenedFileName);
    }
}

void MainWindow::OpenFile(QString path)
{
    buffer = new BaseLibrarySpace::Buffer(new std::ifstream(path.toStdString(), std::ios::in | std::ios::binary));
    if (buffer != nullptr) {
        this->setWindowTitle("BIOS Viewer -- " + path);
        InputImageSize = buffer->getBufferSize();
        InputImage = buffer->getBytes(InputImageSize);
        InputImageModel = new DataModel(new Volume(InputImage, InputImageSize), "IFWI Overview", "Image", "UEFI");
        QElapsedTimer timer;
        timer.start();
        parseBinaryInfo();
        float time = (double)timer.nsecsElapsed()/(double)1000000;
        qDebug() << time << "ms";
        delete buffer;
    }
}

void MainWindow::DoubleClickOpenFile(QString path) {
    OpenFile(path);
}

void MainWindow::parseBinaryInfo() {
    setBiosFvData();
    setFfsData();
    setTreeData();
    if (BiosValidFlag != false) {
        BiosImage->setBiosID();
        BiosImage->setInfoStr();
    }
    InputImageModel->modelData->InfoStr = BiosImage->InfoStr;
    ui->titleInfomation->setText(QString::fromStdString(BiosImage->BiosID));
    if (BiosImage->FitTable == nullptr)
        ui->infoButton->setVisible(false);
    else
        ui->infoButton->setVisible(true);
    ui->treeWidget->setCurrentIndex(ui->treeWidget->model()->index(0, 0, QModelIndex()));
}

void MainWindow::showTreeRightMenu(QPoint pos) {
    QIcon hexBinary, box_arrow_up;
    if (DarkmodeFlag) {
        hexBinary = QIcon(":/file-binary_light.svg");
        box_arrow_up = QIcon(":/box-arrow-up_light.svg");
    } else {
        hexBinary = QIcon(":/file-binary.svg");
        box_arrow_up = QIcon(":/box-arrow-up.svg");
    }
    QModelIndex index = ui->treeWidget->indexAt(pos);
    if (!index.isValid())
        return;
    QTreeWidgetItem *item = ui->treeWidget->itemAt(pos);
    RightClickeditemModel = item->data(MainWindow::Name, Qt::UserRole).value<DataModel*>();

    QMenu* menu = new QMenu;
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

    menu->move(ui->treeWidget->cursor().pos());
    menu->show();
}

void MainWindow::showHexView() {
    HexViewDialog *hexDialog = new HexViewDialog;
    if (isDarkMode()) {
        hexDialog->setWindowIcon(QIcon(":/file-binary_light.svg"));
    }
    UINT8 *itemData = RightClickeditemModel->modelData->data;
    QByteArray *hexViewData = new QByteArray((char*)itemData, RightClickeditemModel->modelData->size);
    hexDialog->m_hexview->loadFromBuffer(*hexViewData);
    hexDialog->exec();
    delete hexViewData;
}

void MainWindow::showBodyHexView() {
    HexViewDialog *hexDialog = new HexViewDialog;
    if (isDarkMode()) {
        hexDialog->setWindowIcon(QIcon(":/file-binary_light.svg"));
    }
    UINT8 *itemData = RightClickeditemModel->modelData->data;
    QByteArray *hexViewData = new QByteArray((char*)itemData, RightClickeditemModel->modelData->size);
    INT64 HeaderSize = RightClickeditemModel->modelData->getHeaderSize();
    QByteArray BodyHexViewData = hexViewData->mid(HeaderSize);
    hexDialog->m_hexview->loadFromBuffer(BodyHexViewData);
    hexDialog->exec();
    delete hexViewData;
}

void MainWindow::showNvHexView() {
    HexViewDialog *hexDialog = new HexViewDialog;
    UINT8 *NvData = ((NvVariableEntry*)(RightClickeditemModel->modelData))->DataPtr;
    INT64 NvDataSize = ((NvVariableEntry*)(RightClickeditemModel->modelData))->DataSize;
    QByteArray *hexViewData = new QByteArray((char*)NvData, NvDataSize);
    hexDialog->m_hexview->loadFromBuffer(*hexViewData);
    hexDialog->exec();
    delete hexViewData;
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

void MainWindow::initSettings() {
    if (!setting.contains("Theme"))
        setting.setValue("Theme", "System");
    if (!setting.contains("BiosViewerFontSize"))
        setting.setValue("BiosViewerFontSize", 12);
    if (!setting.contains("BiosViewerFont"))
        setting.setValue("BiosViewerFont", "Microsoft YaHei UI");
    if (!setting.contains("ShowPaddingItem"))
        setting.setValue("ShowPaddingItem", "false");

    if (!setting.contains("InfoFontSize"))
        setting.setValue("InfoFontSize", 11);
    if (!setting.contains("InfoFont"))
        setting.setValue("InfoFont", "Fira Code");
    if (!setting.contains("InfoLineSpacing"))
        setting.setValue("InfoLineSpacing", "2");

    if (!setting.contains("HexFontSize"))
        setting.setValue("HexFontSize", 11);
    if (!setting.contains("HexFont"))
        setting.setValue("HexFont", "Courier");
    if (!setting.contains("LineSpacing"))
        setting.setValue("LineSpacing", "2");

    if (setting.value("Theme").toString() == "System") {
        if (SysSettings.value("AppsUseLightTheme", 1).toInt() == 0) {
            DarkmodeFlag = true;
            QApplication::setStyle(QStyleFactory::create("Fusion"));
            QApplication::setPalette(QApplication::style()->standardPalette());
        }
    }

    ui->treeWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->treeWidget->setStyleSheet(QString("QTreeView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));

    ui->treeWidget->header()->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->treeWidget->header()->setStyleSheet(QString("QTreeView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));

    ui->infoBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->AddressPanel->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));

    if (DarkmodeFlag) {
        ui->searchButton->setIcon(QIcon(":/search_light.svg"));
        ui->OpenFile->setIcon(QIcon(":/open_light.svg"));
        ui->OpenInNewWindow->setIcon(QIcon(":/open_light.svg"));
        ui->actionSettings->setIcon(QIcon(":/gear_light.svg"));
        ui->actionExit->setIcon(QIcon(":/Exit_light.svg"));
        ui->actionSearch->setIcon(QIcon(":/search_light.svg"));
        ui->actionGoto->setIcon(QIcon(":/bookmark_light.svg"));
        ui->actionCollapse->setIcon(QIcon(":/arrows-collapse_light.svg"));
        ui->actionExtract_BIOS->setIcon(QIcon(":/scissors_light.svg"));
        ui->actionSeperate_Binary->setIcon(QIcon(":/scissors_light.svg"));
        ui->actionReplace_BIOS->setIcon(QIcon(":/replace_light.svg"));
        ui->actionAboutBiosViewer->setIcon(QIcon(":/about_light.svg"));
        ui->actionAboutQt->setIcon(QIcon(":/about_light.svg"));
    }
}

void MainWindow::setTreeData() {
    QTreeWidgetItem *ImageOverviewItem = new QTreeWidgetItem(InputImageModel->getData());
    ImageOverviewItem->setData(MainWindow::Name, Qt::UserRole, QVariant::fromValue((DataModel*)InputImageModel));
    ImageOverviewItem->setFont(MainWindow::Name, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 2, 700));
    ImageOverviewItem->setFont(MainWindow::Type, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 2, 700));
    ImageOverviewItem->setFont(MainWindow::SubType, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 2, 700));
    ui->treeWidget->addTopLevelItem(ImageOverviewItem);

    bool ShowPaddingData;
    if (setting.value("ShowPaddingItem") == "false")
        ShowPaddingData = false;
    else
        ShowPaddingData = true;

    for (auto IfwiModel:IFWI_ModelData) {
        if (!ShowPaddingData && IfwiModel->getSubType() == "Empty")
            continue;
        QTreeWidgetItem *IfwiItem = new QTreeWidgetItem(IfwiModel->getData());
        IfwiItem->setData(MainWindow::Name, Qt::UserRole, QVariant::fromValue((DataModel*)IfwiModel));
        ui->treeWidget->addTopLevelItem(IfwiItem);
        if (IfwiModel->getName() == "CSE") {
            for (auto MeModel:ME_ModelData) {
                 QTreeWidgetItem *MeItem = new QTreeWidgetItem(MeModel->getData());
                 MeItem->setData(MainWindow::Name, Qt::UserRole, QVariant::fromValue((DataModel*)MeModel));
                 IfwiItem->addChild(MeItem);
                 for (auto ChildModel:MeModel->volumeModelData) {
                     QTreeWidgetItem *ChildPartitionItem = new QTreeWidgetItem(ChildModel->getData());
                     ChildPartitionItem->setData(MainWindow::Name, Qt::UserRole, QVariant::fromValue((DataModel*)ChildModel));
                     MeItem->addChild(ChildPartitionItem);
                 }
            }
        }
        if (IfwiModel->getName() == "BIOS") {
            for (auto FvModel:FvModelData) {
                addTreeItem(IfwiItem, FvModel, ShowPaddingData);
            }
            ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
            return;
        }
    }

    for (auto FvModel:FvModelData) {
        if (!ShowPaddingData && FvModel->getSubType() == "Empty")
            continue;
        QTreeWidgetItem *fvItem = new QTreeWidgetItem(FvModel->getData());
        fvItem->setData(MainWindow::Name, Qt::UserRole, QVariant::fromValue((DataModel*)FvModel));
        ui->treeWidget->addTopLevelItem(fvItem);

        for (auto FfsModel:FvModel->volumeModelData) {
            addTreeItem(fvItem, FfsModel, ShowPaddingData);
        }
    }
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
}

void MainWindow::addTreeItem(QTreeWidgetItem *parentItem, DataModel *modelData, bool ShowPadding) {
    if (!ShowPadding && (modelData->getSubType() == "Pad" || modelData->getSubType() == "Empty"))
        return;
    QTreeWidgetItem *Item = new QTreeWidgetItem(modelData->getData());
    Item->setData(MainWindow::Name, Qt::UserRole, QVariant::fromValue(modelData));
    parentItem->addChild(Item);
    for (auto volumeModel:modelData->volumeModelData) {
        addTreeItem(Item, volumeModel, ShowPadding);
    }
}

void MainWindow::setPanelInfo(INT64 offset, INT64 size)
{
    stringstream Info;
    Info.setf(ios::left);
    Info << "Offset: 0x";
    Info.width(10);
    Info << hex << uppercase << offset;

    Info << "Size: 0x";
    Info << hex << uppercase << size;

    QString panelInfo = QString::fromStdString(Info.str());
    ui->AddressPanel->setText(panelInfo);
}

void MainWindow::setOpenedFileName(QString name) {
    this->OpenedFileName = name;
}

void MainWindow::OpenFileTriggered()
{
    QString lastPath = setting.value("LastFilePath").toString();
    QString fileName = QFileDialog::getOpenFileName(
                                                    this, tr("Open Image File"),
                                                    lastPath, tr("Image files(*.rom *.bin *.fd *.fv);;All files (*.*)"));
    if (fileName.isEmpty()){
        return;
    }
    OpenedFileName = fileName;
    ui->treeWidget->clear();
    ui->titleInfomation->clear();
    ui->infoBrowser->clear();
    ui->AddressPanel->clear();
    cleanup();
    QFileInfo fileinfo {fileName};
    setting.setValue("LastFilePath", fileinfo.path());
    OpenFile(fileName);
}

void MainWindow::ActionExitTriggered()
{
    this->close();
}

void MainWindow::ActionSettingsTriggered()
{
    SettingsDialog *settingDialog = new SettingsDialog;
    if (isDarkMode()) {
        settingDialog->setWindowIcon(QIcon(":/gear_light.svg"));
    }
    settingDialog->setParentWidget(this);
    settingDialog->exec();
}

void MainWindow::ActionAboutQtTriggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::ActionAboutBiosViewerTriggered()
{
    QString strText= QString("<html><head/><body><p><span style=' font-size:14pt; font-weight:700;'>BIOS Viewer %1"
                             "</span></p><p>Internal Use Only</p><p>Built on %2 by <span style=' font-weight:700; color:#00aaff;'>Chen, Zhu")
                             .arg(__BiosViewerVersion__).arg(__DATE__);
    QMessageBox::about(this, tr("About BIOS Viewer"), strText);
}

void MainWindow::OpenInNewWindowTriggered()
{
    QString lastPath = setting.value("LastFilePath").toString();
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open Image File"),
        lastPath, tr("Image files(*.rom *.bin *.fd);;All files (*.*)"));
    if (fileName.isEmpty()){
        return;
    }
    QFileInfo fileinfo {fileName};
    setting.setValue("LastFilePath", fileinfo.path());

    MainWindow *newWindow = new MainWindow;
    newWindow->setAttribute(Qt::WA_DeleteOnClose);
    newWindow->setOpenedFileName(fileName);
    newWindow->show();
    newWindow->OpenFile(fileName);
}

void MainWindow::TreeWidgetItemSelectionChanged()
{
    QModelIndex index = ui->treeWidget->currentIndex();
    if (!index.isValid())
        return;
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    DataModel * itemModel = item->data(MainWindow::Name, Qt::UserRole).value<DataModel*>();
    Volume* volume = itemModel->modelData;
    QPalette pal(ui->AddressPanel->palette());
//    if (volume->isCompressed) {
//        pal.setColor(QPalette::Base, Qt::cyan);
//    }
    ui->AddressPanel->setPalette(pal);
    setPanelInfo(volume->offsetFromBegin, volume->size);

    volume->setInfoStr();
    ui->infoBrowser->setText(volume->InfoStr);
}

void MainWindow::InfoButtonClicked()
{
    InfoWindow *infoWindow = new InfoWindow;
    if (BiosImage != nullptr) {
        infoWindow->setBiosImage(BiosImage);
        infoWindow->showFitTab();
    }

    infoWindow->show();
}
