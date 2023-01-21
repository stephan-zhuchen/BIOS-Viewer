#include <QFileDialog>
#include "mainwindow.h"
#include "HexViewDialog.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      buffer(nullptr),
      hexViewData(nullptr),
      popMenu(new QMenu)
{
    ui->setupUi(this);
    initTree();

    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    this->connect(ui->treeWidget,SIGNAL(customContextMenuRequested(QPoint)),
                      this,SLOT(showTreeRightMenu(QPoint)));
}

MainWindow::~MainWindow()
{
    cleanup();
    delete ui;
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
}

void MainWindow::OpenFile(std::string path)
{
    buffer = new BaseLibrarySpace::Buffer(new std::ifstream(path, std::ios::in | std::ios::binary));

    parseBinaryInfo();
}

void MainWindow::parseBinaryInfo() {
    setFvData();
    setFfsData();
    setTreeData();
}

void MainWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
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

void MainWindow::showTreeRightMenu(QPoint pos) {
    QMenu* menu = new QMenu;
    QAction* showHex = new QAction("Hex View");
    menu->addAction(showHex);

    QTreeWidgetItem *item = ui->treeWidget->itemAt(pos);
    DataModel * itemModel = item->data(MainWindow::Name, Qt::UserRole).value<DataModel*>();
    UINT8 *itemData = itemModel->modelData->data;
    hexViewData = new QByteArray((char*)itemData, itemModel->modelData->size);
    cout << hexViewData->size() << endl;

    this->connect(showHex,SIGNAL(triggered(bool)),this,SLOT(showHexView()));
    menu->move(ui->treeWidget->cursor().pos());
    menu->show();
}

void MainWindow::showHexView() {
    HexViewDialog *hexDialog = new HexViewDialog;
    hexDialog->m_hexview->loadFromBuffer(*hexViewData);
    hexDialog->exec();
}

void MainWindow::initTree() {
    ui->treeWidget->clear();
    ui->treeWidget->setStyleSheet("QTreeView::item{margin:2px;}");

    ui->treeWidget->setColumnWidth(MainWindow::Name, 400);
    ui->treeWidget->setColumnWidth(MainWindow::Type, 100);
//    ui->treeWidget->setColumnWidth(MainWindow::SubType, 120);
    ui->treeWidget->header()->setResizeContentsPrecision(QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setStretchLastSection(true);
}

void MainWindow::setTreeData() {
    for (auto FvModel:FvModelData) {
        QTreeWidgetItem *fvItem = new QTreeWidgetItem(FvModel->getData());
        fvItem->setData(MainWindow::Name, Qt::UserRole, QVariant::fromValue((DataModel*)FvModel));
        ui->treeWidget->addTopLevelItem(fvItem);

        for (auto FfsModel:FvModel->volumeModelData) {
            addTreeItem(fvItem, FfsModel);
        }
    }
}

void MainWindow::addTreeItem(QTreeWidgetItem *parentItem, DataModel *modelData) {
    QTreeWidgetItem *Item = new QTreeWidgetItem(modelData->getData());
    Item->setData(MainWindow::Name, Qt::UserRole, QVariant::fromValue(modelData));
    if (modelData->getSubType() == "UI") {
        DataModel * parentModel = parentItem->data(MainWindow::Name, Qt::UserRole).value<DataModel*>();
        if (parentModel->getType() == "Section") {
            parentItem->parent()->setText(MainWindow::Name, modelData->getText());
        } else {
            parentItem->setText(MainWindow::Name, modelData->getText());
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
    Info << "Offset = 0x";
    Info.width(25);
    Info << hex << uppercase << offset;

    Info << "Length = 0x";
    Info.width(10);
    Info << hex << uppercase << size;

    QString panelInfo = QString::fromStdString(Info.str());
    ui->AddressPanel->setText(panelInfo);
}

void MainWindow::on_OpenFile_triggered()
{
    QString lastPath = setting.value("LastFilePath").toString();
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open BIOS File"),
        lastPath, tr("Capsule files(*.rom *.bin *.cap);;All files (*.*)"));
    if (fileName.isEmpty()){
        return;
    }
    cleanup();
    ui->treeWidget->clear();
    QFileInfo fileinfo {fileName};
    setting.setValue("LastFilePath", fileinfo.path());
    OpenFile(fileName.toStdString());
}


void MainWindow::on_actionExit_triggered()
{
    this->close();
}

