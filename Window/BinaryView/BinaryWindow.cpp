#include "BinaryWindow.h"
#include "ui_BinaryWindow.h"
#include "Start/StartWindow.h"
#include "Payload/Elf.h"
#include "Payload/PE32.h"

enum BinaryTreeColNum {Name=0, Type};

BinaryWindow::BinaryWindow(StartWindow *parent):
    QWidget(parent),
    ui(new Ui::BinaryWindow),
    mWindow(parent),
    setting(QSettings("Intel", "BiosViewer"))
{}

BinaryWindow::~BinaryWindow() {
    delete ui;
    delete elf;
    delete pe;
    delete imageModel;
}

void BinaryWindow::setupUi(QMainWindow *MainWindow, WindowData *wData) {
    winData = wData;
    ui->setupUi(MainWindow);
    connect(ui->treeWidget,       SIGNAL(itemSelectionChanged()), this, SLOT(TreeWidgetItemSelectionChanged()));
}

void BinaryWindow::setTreeData() {
    auto *ImageOverviewItem = new QTreeWidgetItem(vectorToQStringList(imageModel->getData()));
    ImageOverviewItem->setData(BinaryTreeColNum::Name, Qt::UserRole, QVariant::fromValue(imageModel->getVolume()));
    ImageOverviewItem->setFont(BinaryTreeColNum::Name, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt(), 700));
    ImageOverviewItem->setFont(BinaryTreeColNum::Type, QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt(), 700));
    ui->treeWidget->addTopLevelItem(ImageOverviewItem);

    // for (auto volume : BiosData->VolumeDataList) {
    //     addTreeItem(nullptr, volume, ShowPadding);
    // }
    // ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
}

void BinaryWindow::loadBinary() {
    if (ELF::IsElfFormat(winData->InputImage)) {
        binaryKind = Binary::ELF;
        elf = new ELF(winData->InputImage, winData->InputImageSize, 0);
        elf->SelfDecode();
        imageModel = new DataModel(elf, "ELF", "image");
        setTreeData();
    } else if (PE32::IsPe32Format(winData->InputImage)) {
        binaryKind = Binary::PE32;
        pe = new PE32(winData->InputImage, winData->InputImageSize, 0);
        pe->SelfDecode();
        imageModel = new DataModel(pe, "PE32", "image");
        setTreeData();
    }
}

bool BinaryWindow::TryOpenBinary(UINT8 *image, INT64 imageLength) {
    // check ELF size to avoid access nullptr
    if (ELF::IsElfFormat(image)) {
        return true;
    } else if (PE32::IsPe32Format(image)) {
        return true;
    }
    return false;
}

void BinaryWindow::TreeWidgetItemSelectionChanged() const {
    QModelIndex index = ui->treeWidget->currentIndex();
    if (!index.isValid())
        return;
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    auto *itemVolume = item->data(BinaryTreeColNum::Name, Qt::UserRole).value<Volume*>();
    QPalette pal(ui->AddressPanel->palette());
    ui->AddressPanel->setPalette(pal);
    // setPanelInfo(itemVolume->getOffset(), itemVolume->getSize());

    itemVolume->setInfoStr();
    ui->infoBrowser->setText(QString::fromStdString(itemVolume->getInfoText()));
}
