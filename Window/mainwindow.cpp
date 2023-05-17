#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMimeData>
#include <QStyleFactory>
#include <QInputDialog>
#include <QElapsedTimer>
#include <QProcess>
#include "mainwindow.h"
#include "HexViewDialog.h"
#include "SettingsDialog.h"
#include "InfoWindow.h"
#include "./ui_mainwindow.h"
#include "GuidDefinition.h"

GuidDatabase *guidData = nullptr;
UINT32       OpenedWindow = 0;

MainWindow::MainWindow(QString appPath, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      buffer(nullptr),
      structureLabel(new QLabel("Structure:", this)),
      infoLabel(new QLabel("Infomation:", this)),
      DarkmodeFlag(false),
      BiosValidFlag(true),
      IFWI_exist(false),
      infoWindow(nullptr),
      infoWindowOpened(false),
      searchDialog(nullptr),
      searchDialogOpened(false),
      appDir(appPath),
      InputImage(nullptr),
      InputImageModel(nullptr),
      BiosImage(nullptr)
{
    ui->setupUi(this);

    // restore window position
    QSettings windowSettings("Intel", "BiosViewer");
    restoreGeometry(windowSettings.value("mainwindow/geometry").toByteArray());
    initSettings();
    initRightMenu();

    ui->titleInfomation->clear();
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

    ui->infoButton->setVisible(false);

    this->connect(ui->treeWidget,SIGNAL(customContextMenuRequested(QPoint)),
                      this,SLOT(showTreeRightMenu(QPoint)));

    if (guidData == nullptr) {
        guidData = new GuidDatabase;
    }
    OpenedWindow += 1;
}

MainWindow::~MainWindow()
{
    OpenedWindow -= 1;
    if (guidData != nullptr && OpenedWindow == 0)
        delete guidData;
    delete structureLabel;
    delete infoLabel;
    cleanup();
    finiRightMenu();
    delete ui;
}

void MainWindow::cleanup() {
    this->setWindowTitle("BIOS Viewer");
    BiosValidFlag = true;
    flashmap = "";

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
    if (InputImageModel != nullptr)
        delete InputImageModel;
}

void MainWindow::setInfoWindowState(bool opened) {
    infoWindowOpened = opened;
}

void MainWindow::setSearchDialogState(bool opened) {
    searchDialogOpened = opened;
}

bool MainWindow::isDarkMode() {
    return DarkmodeFlag;
}

void MainWindow::refresh() {
    initSettings();
    QTreeWidgetItem *ImageOverviewItem = ui->treeWidget->itemAt(0, 0);
    if (ui->treeWidget->indexOfTopLevelItem(ImageOverviewItem) == -1)
        return;
    ImageOverviewItem->setFont(MainWindow::Name, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 2, 700));
    ImageOverviewItem->setFont(MainWindow::Type, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 2, 700));
    ImageOverviewItem->setFont(MainWindow::SubType, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 2, 700));
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    INT32 width = ui->treeWidget->width();
    ui->treeWidget->setColumnWidth(MainWindow::Name, width - 280);
    ui->treeWidget->setColumnWidth(MainWindow::Type, 120);
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

void MainWindow::closeEvent(QCloseEvent *event) {
    if (infoWindowOpened) {
        infoWindow->close();
        infoWindow = nullptr;
    }
    if (searchDialogOpened) {
        searchDialog->close();
        searchDialog = nullptr;
    }
    // save window position
    QSettings windowSettings("Intel", "BiosViewer");
    windowSettings.setValue("mainwindow/geometry", saveGeometry());
}

void MainWindow::OpenFile(QString path)
{
    buffer = new BaseLibrarySpace::Buffer(new std::ifstream(path.toStdString(), std::ios::in | std::ios::binary));
    if (buffer != nullptr) {
        setOpenedFileName(path);
        this->setWindowTitle("BIOS Viewer -- " + path);
        InputImageSize = buffer->getBufferSize();
        InputImage = buffer->getBytes(InputImageSize);
        InputImageModel = new DataModel(new Volume(InputImage, InputImageSize), "IFWI Overview", "Image", "UEFI", "", true);
        QElapsedTimer timer;
        timer.start();
        parseBinaryInfo();
        float time = (double)timer.nsecsElapsed()/(double)1000000;
        qDebug() << time << "ms";
        delete buffer;
    }
    resizeEvent(nullptr);
}

void MainWindow::DoubleClickOpenFile(QString path) {
    OpenedFileName = path;
    OpenFile(path);
}

void MainWindow::parseBinaryInfo() {
    setBiosFvData();
    setFfsData();
    setTreeData();
    if (BiosValidFlag != false && BiosImage->FitTable != nullptr) {
        BiosImage->setBiosID();
        BiosImage->getObbDigest();
        BiosImage->setInfoStr();
        class thread getBiosFlashmap([this](){ flashmap += QString::fromStdString(BiosImage->getFlashmap()); });
        getBiosFlashmap.detach();
    }
    InputImageModel->modelData->InfoStr = BiosImage->InfoStr;
    ui->titleInfomation->setText(QString::fromStdString(BiosImage->BiosID));
    if (BiosImage->FitTable == nullptr)
        ui->infoButton->setVisible(false);
    else
        ui->infoButton->setVisible(true);
    ui->treeWidget->setCurrentIndex(ui->treeWidget->model()->index(0, 0, QModelIndex()));
}

void MainWindow::initSettings() {
    if (!setting.contains("Theme"))
        setting.setValue("Theme", "Light");
    if (!setting.contains("BiosViewerFontSize"))
        setting.setValue("BiosViewerFontSize", 12);
    if (!setting.contains("BiosViewerFont"))
        setting.setValue("BiosViewerFont", "Microsoft YaHei UI");
    if (!setting.contains("ShowPaddingItem"))
        setting.setValue("ShowPaddingItem", "false");
    if (!setting.contains("EnableMultiThread"))
        setting.setValue("EnableMultiThread", "false");

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

void MainWindow::erasePadding(vector<DataModel*> &items) {
    for (int i = 0; i < items.size(); ++i) {
        DataModel *FvModel = items.at(i);

        if (FvModel->getSubType() == "Empty" || FvModel->getSubType() == "Pad") {
            delete FvModel;
            items.erase(items.begin() + i);
            if (i > 0) {
                i -= 1;
            }
            continue;
        }
        erasePadding(FvModel->volumeModelData);
    }
}

void MainWindow::setTreeData() {
    QTreeWidgetItem *ImageOverviewItem = new QTreeWidgetItem(InputImageModel->getData());
    ImageOverviewItem->setData(MainWindow::Name, Qt::UserRole, QVariant::fromValue((DataModel*)InputImageModel));
    ImageOverviewItem->setFont(MainWindow::Name, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 2, 700));
    ImageOverviewItem->setFont(MainWindow::Type, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 2, 700));
    ImageOverviewItem->setFont(MainWindow::SubType, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 2, 700));
    ui->treeWidget->addTopLevelItem(ImageOverviewItem);

    if (setting.value("ShowPaddingItem") == "false") {
        erasePadding(IFWI_ModelData);
    }

    for (int i = 0; i < IFWI_ModelData.size(); ++i) {
        DataModel *FvModel = IFWI_ModelData.at(i);
        QTreeWidgetItem *fvItem = new QTreeWidgetItem(FvModel->getData());
        fvItem->setData(MainWindow::Name, Qt::UserRole, QVariant::fromValue((DataModel*)FvModel));
        ui->treeWidget->addTopLevelItem(fvItem);

        for (auto FfsModel:FvModel->volumeModelData) {
            addTreeItem(fvItem, FfsModel);
        }
    }
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
}

void MainWindow::addTreeItem(QTreeWidgetItem *parentItem, DataModel *modelData) {
    QTreeWidgetItem *Item = new QTreeWidgetItem(modelData->getData());
    Item->setData(MainWindow::Name, Qt::UserRole, QVariant::fromValue(modelData));
    parentItem->addChild(Item);
    for (auto volumeModel:modelData->volumeModelData) {
        addTreeItem(Item, volumeModel);
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
    SettingsDialog *settingDialog = new SettingsDialog();
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

    MainWindow *newWindow = new MainWindow(appDir);
    newWindow->setAttribute(Qt::WA_DeleteOnClose);
//    newWindow->setOpenedFileName(fileName);
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
    if (!infoWindowOpened) {
        infoWindowOpened = true;
        infoWindow = new InfoWindow;
        if (BiosImage != nullptr) {
            infoWindow->setBiosImage(BiosImage);
            infoWindow->setParentWidget(this);
            infoWindow->showFitTab();
            infoWindow->showFlashmapTab(flashmap);
        }
        infoWindow->show();
    }
}
