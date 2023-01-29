#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMimeData>
#include <QStyleFactory>
#include <QInputDialog>
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
      hexViewData(nullptr),
      popMenu(new QMenu),
      structureLabel(new QLabel("Structure:", this)),
      infoLabel(new QLabel("Infomation:", this)),
      BiosImage(nullptr),
      BiosImageModel(nullptr)
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

//    QColor color = QColor(Qt::gray);
//    QPalette p = this->palette();
//    p.setColor(QPalette::Window,color);
//    this->setPalette(p);
//    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    qDebug() << QStyleFactory::keys();

    initSettings();

    this->connect(ui->treeWidget,SIGNAL(customContextMenuRequested(QPoint)),
                      this,SLOT(showTreeRightMenu(QPoint)));

    guidData = new GuidDatabase;
    OpenedWindow += 1;
}

MainWindow::~MainWindow()
{
    OpenedWindow -= 1;
    cout << "OpenedWindow = " << OpenedWindow << endl;
    if (guidData != nullptr && OpenedWindow == 0)
        delete guidData;
    delete structureLabel;
    delete infoLabel;
    delete ui;
    cleanup();
}

void MainWindow::cleanup() {
    for (auto FvModel:FvModelData) {
        delete FvModel;
    }
    FvModelData.clear();

    for (auto FirmwareVolume:FirmwareVolumeData) {
        delete FirmwareVolume;
    }
    FirmwareVolumeData.clear();

    for (UINT8* fvBuffer:FirmwareVolumeBuffer) {
        delete[] fvBuffer;
    }
    FirmwareVolumeBuffer.clear();

    if (hexViewData != nullptr)
        delete hexViewData;

    if (buffer != nullptr)
        delete buffer;

    if (BiosImage != nullptr)
        delete BiosImage;
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
        OpenFile(OpenedFileName.toStdString());
    }
}

void MainWindow::OpenFile(std::string path)
{
    buffer = new BaseLibrarySpace::Buffer(new std::ifstream(path, std::ios::in | std::ios::binary));
    parseBinaryInfo();
}

void MainWindow::DoubleClickOpenFile(std::string path) {
    OpenFile(path);
}

void MainWindow::parseBinaryInfo() {
    setFvData();
    setFfsData();
    getBiosID();
    setTreeData();
}

void MainWindow::showTreeRightMenu(QPoint pos) {
    QMenu* menu = new QMenu;
    QAction* showHex = new QAction("Hex View");
    menu->addAction(showHex);

    QModelIndex index = ui->treeWidget->indexAt(pos);
    if (!index.isValid())
        return;
    QTreeWidgetItem *item = ui->treeWidget->itemAt(pos);
    DataModel * itemModel = item->data(MainWindow::Name, Qt::UserRole).value<DataModel*>();
    UINT8 *itemData = itemModel->modelData->data;
    hexViewData = new QByteArray((char*)itemData, itemModel->modelData->size);

    this->connect(showHex,SIGNAL(triggered(bool)),this,SLOT(showHexView()));
    menu->move(ui->treeWidget->cursor().pos());
    menu->show();
}

void MainWindow::showHexView() {
    HexViewDialog *hexDialog = new HexViewDialog;
    hexDialog->m_hexview->loadFromBuffer(*hexViewData);
    hexDialog->exec();
}

void MainWindow::initSettings() {
    if (!setting.contains("Theme"))
        setting.setValue("Theme", "Light");
    if (!setting.contains("BiosViewerFontSize"))
        setting.setValue("BiosViewerFontSize", 12);
    if (!setting.contains("BiosViewerFont"))
        setting.setValue("BiosViewerFont", "Microsoft YaHei UI");

    if (!setting.contains("InfoFontSize"))
        setting.setValue("InfoFontSize", 12);
    if (!setting.contains("InfoFont"))
        setting.setValue("InfoFont", "Fira Code");
    if (!setting.contains("InfoLineSpacing"))
        setting.setValue("InfoLineSpacing", "2");

    if (!setting.contains("HexFontSize"))
        setting.setValue("HexFontSize", 12);
    if (!setting.contains("HexFont"))
        setting.setValue("HexFont", "Courier");
    if (!setting.contains("LineSpacing"))
        setting.setValue("LineSpacing", "2");

    ui->treeWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->treeWidget->setStyleSheet(QString("QTreeView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));

    ui->treeWidget->header()->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->treeWidget->header()->setStyleSheet(QString("QTreeView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));

    ui->infoBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->AddressPanel->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
}

void MainWindow::setTreeData() {
    QTreeWidgetItem *ImageOverviewItem = new QTreeWidgetItem(BiosImageModel->getData());
    ImageOverviewItem->setData(MainWindow::Name, Qt::UserRole, QVariant::fromValue((DataModel*)BiosImageModel));
    ImageOverviewItem->setFont(MainWindow::Name, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 2, 700));
    ImageOverviewItem->setFont(MainWindow::Type, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 2, 700));
    ImageOverviewItem->setFont(MainWindow::SubType, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt() + 2, 700));
    ui->treeWidget->addTopLevelItem(ImageOverviewItem);

    for (auto FvModel:FvModelData) {
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
    if (modelData->getSubType() == "UI") {
        DataModel * parentModel = parentItem->data(MainWindow::Name, Qt::UserRole).value<DataModel*>();
        if (parentModel->getType() == "Section") {
            parentItem->parent()->setText(MainWindow::Name, modelData->getText());
            parentItem->parent()->data(MainWindow::Name, Qt::UserRole).value<DataModel*>()->setText(modelData->getText());
        } else {
            parentItem->setText(MainWindow::Name, modelData->getText());
            parentItem->data(MainWindow::Name, Qt::UserRole).value<DataModel*>()->setText(modelData->getText());
        }
    }
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

void MainWindow::on_OpenFile_triggered()
{
    QString lastPath = setting.value("LastFilePath").toString();
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open Image File"),
        lastPath, tr("Image files(*.rom *.bin *.fd);;All files (*.*)"));
    if (fileName.isEmpty()){
        return;
    }
    OpenedFileName = fileName;
    cout << OpenedFileName.toStdString() << endl;
    ui->treeWidget->clear();
    ui->titleInfomation->clear();
    ui->infoBrowser->clear();
    ui->AddressPanel->clear();
    cleanup();
    QFileInfo fileinfo {fileName};
    setting.setValue("LastFilePath", fileinfo.path());
    OpenFile(fileName.toStdString());
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog *settingDialog = new SettingsDialog;
    settingDialog->setParentWidget(this);
    settingDialog->exec();
}

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::on_actionAboutBiosViewer_triggered()
{
    QString strText= QString("<html><head/><body><p><span style=' font-size:14pt; font-weight:700;'>BIOS Viewer %1"
                             "</span></p><p>Internal Use Only</p><p>Built on %2 by <span style=' font-weight:700; color:#00aaff;'>Chen, Zhu")
                             .arg(__BiosViewerVersion__).arg(__DATE__);
    QMessageBox::about(this, tr("About BIOS Viewer"), strText);
}

void MainWindow::on_OpenInNewWindow_triggered()
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
    newWindow->OpenFile(fileName.toStdString());
}

void MainWindow::on_treeWidget_itemSelectionChanged()
{
    QModelIndex index = ui->treeWidget->currentIndex();
    if (!index.isValid())
        return;
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    DataModel * itemModel = item->data(MainWindow::Name, Qt::UserRole).value<DataModel*>();
    Volume* volume = itemModel->modelData;
    QPalette pal(ui->AddressPanel->palette());
    if (volume->isCompressed) {
        pal.setColor(QPalette::Base, Qt::cyan);
    } else {
        pal.setColor(QPalette::Base, Qt::white);
    }
    ui->AddressPanel->setPalette(pal);
    setPanelInfo(volume->offsetFromBegin, volume->size);

    volume->setInfoStr();
    ui->infoBrowser->setText(volume->InfoStr);
}

void MainWindow::on_infoButton_clicked()
{
    InfoWindow *infoWindow = new InfoWindow;
    if (BiosImage != nullptr) {
        infoWindow->setBiosImage(BiosImage);
        infoWindow->showFitTable();
    }

    infoWindow->show();
}

