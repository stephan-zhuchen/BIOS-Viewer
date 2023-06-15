#include "BiosWindow.h"
#include <QCloseEvent>
#include <QMessageBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <thread>
#include "inputdialog.h"
#include "StartWindow.h"
#include "ui_BiosWindow.h"

GeneralData::GeneralData(QString dir):appDir(dir) {}

GeneralData::~GeneralData() {
    qDebug() << "GeneralData::~GeneralData";
    if (BiosViewerUi != nullptr)
        delete BiosViewerUi;
    if (HexViewerUi != nullptr)
        delete HexViewerUi;
    if (InputImage != nullptr) {
        delete InputImage;
        InputImage = nullptr;
    }
}

BiosViewerData::~BiosViewerData() {
    qDebug() << "BiosViewerData::~BiosViewerData";
    BiosValidFlag = true;
    flashmap = "";

    if (infoWindowOpened) {
        infoWindow->close();
        infoWindow = nullptr;
    }
    if (searchDialogOpened) {
        searchDialog->close();
        searchDialog = nullptr;
    }

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
    if (InputImageModel != nullptr)
        delete InputImageModel;
}

bool BiosViewerData::isValidBIOS(UINT8 *image, INT64 imageLength) {
    if (imageLength == 0x2000000) {
        FLASH_DESCRIPTOR_HEADER FDH = *(FLASH_DESCRIPTOR_HEADER*)image;
        if (FDH.Signature == V_FLASH_FDBAR_FLVALSIG) {
            return true;
        }
    }

    EFI_FIRMWARE_VOLUME_HEADER fvHeader = *(EFI_FIRMWARE_VOLUME_HEADER*)image;
    if (FirmwareVolume::isValidFirmwareVolume(&fvHeader)) {
        return true;
    }
    return false;
}

BiosViewerWindow::BiosViewerWindow(StartWindow *parent):
    QWidget(parent),
    ui(new Ui::BiosWindow),
    mWindow(parent),
    setting(QSettings("Intel", "BiosViewer"))
{
    initRightMenu();
}

BiosViewerWindow::~BiosViewerWindow() {
    qDebug() << "BiosViewerWindow::~BiosViewerWindow";
    if (InputData != nullptr) {
        delete InputData;
        InputData = nullptr;
    }
    finiRightMenu();
}

void BiosViewerWindow::setupUi(QMainWindow *MainWindow, GeneralData *wData) {
    WindowData = wData;
    ui->setupUi(MainWindow);

    initSetting();
    connect(ui->treeWidget,       SIGNAL(itemSelectionChanged()), this, SLOT(TreeWidgetItemSelectionChanged()));
    connect(ui->infoButton,       SIGNAL(clicked()),   this, SLOT(InfoButtonClicked()));
    connect(ui->searchButton,     SIGNAL(clicked()),   this, SLOT(SearchButtonClicked()));
    connect(ui->treeWidget,       SIGNAL(customContextMenuRequested(QPoint)), this,SLOT(showTreeRightMenu(QPoint)));
}

void BiosViewerWindow::initSetting() {
    ui->treeWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->treeWidget->setStyleSheet(QString("QTreeView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));

    ui->treeWidget->header()->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->treeWidget->header()->setStyleSheet(QString("QTreeView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));
    ui->treeWidget->header()->setSectionResizeMode(BiosViewerData::Name, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(BiosViewerData::Type, QHeaderView::Fixed);
    ui->treeWidget->header()->setSectionResizeMode(BiosViewerData::SubType, QHeaderView::Fixed);
    ui->treeWidget->header()->resizeSection(BiosViewerData::Type, 120);
    ui->treeWidget->header()->resizeSection(BiosViewerData::SubType, 120);
    ui->treeWidget->header()->setStretchLastSection(false);

    ui->infoBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->AddressPanel->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));

    if (WindowData->DarkmodeFlag) {
        ui->searchButton->setIcon(QIcon(":/search_light.svg"));
    }
}

void BiosViewerWindow::setInfoWindowState(bool opened) {
    InputData->infoWindowOpened = opened;
}

void BiosViewerWindow::setSearchDialogState(bool opened) {
    InputData->searchDialogOpened = opened;
}

bool BiosViewerWindow::TryOpenBios(UINT8 *image, INT64 imageLength) {
    return BiosViewerData::isValidBIOS(image, imageLength);
}

void BiosViewerWindow::loadBios(Buffer *buffer) {
    InputData = new BiosViewerData;
    InputData->InputImageModel = new DataModel(new Volume(WindowData->InputImage, WindowData->InputImageSize), "IFWI Overview", "Image", "UEFI", "", true);
    setBiosFvData();
    setFfsData();
    setTreeData();
    if (InputData->BiosValidFlag != false && InputData->BiosImage->FitTable != nullptr) {
        InputData->BiosImage->setBiosID();
        InputData->BiosImage->getObbDigest();
        InputData->BiosImage->setInfoStr();
        class thread getBiosFlashmap([this](){ InputData->flashmap += QString::fromStdString(InputData->BiosImage->getFlashmap()); });
        getBiosFlashmap.detach();
    }
    InputData->InputImageModel->modelData->InfoStr = InputData->BiosImage->InfoStr;
    ui->titleInfomation->setText(QString::fromStdString(InputData->BiosImage->BiosID));
    if (InputData->BiosImage->FitTable == nullptr)
        ui->infoButton->setVisible(false);
    else
        ui->infoButton->setVisible(true);
    ui->treeWidget->setCurrentIndex(ui->treeWidget->model()->index(0, 0, QModelIndex()));
    resizeEvent(nullptr);
}

void BiosViewerWindow::ActionSearchBiosTriggered() {
    if (!InputData->searchDialogOpened) {
        InputData->searchDialogOpened = true;
        InputData->searchDialog = new SearchDialog();
        InputData->searchDialog->setSearchMode(false);
        InputData->searchDialog->SetModelData(&InputData->IFWI_ModelData);
        InputData->searchDialog->setParentWidget(this);
        if (isDarkMode())
            InputData->searchDialog->setWindowIcon(QIcon(":/search_light.svg"));
        InputData->searchDialog->show();
    }
}

void BiosViewerWindow::ActionGotoTriggered() {
    if ( InputData->BiosImage == nullptr )
        return;

    QDialog dialog(this);
    dialog.setWindowTitle("Goto ...");
    QFormLayout form(&dialog);
    // Offset
    QString Offset = QString("Offset: ");
    HexSpinBox *OffsetSpinbox = new HexSpinBox(&dialog);
    OffsetSpinbox->setFocus();
    OffsetSpinbox->selectAll();
    form.addRow(Offset, OffsetSpinbox);
    // Add Cancel and OK button
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Process when OK button is clicked
    if (dialog.exec() == QDialog::Accepted) {
        INT64 SearchOffset = OffsetSpinbox->value();
        if (SearchOffset < 0 || SearchOffset >= WindowData->InputImageSize) {
            QMessageBox::critical(this, tr("Goto ..."), "Invalid offset!");
            return;
        }

        vector<INT32> SearchRows;
        for (UINT64 row = 0; row < InputData->IFWI_ModelData.size(); ++row) {
            DataModel *model = InputData->IFWI_ModelData.at(row);
            if (SearchOffset >= model->modelData->offsetFromBegin && SearchOffset < model->modelData->offsetFromBegin + model->modelData->size) {
                SearchRows.push_back(row + 1);
                RecursiveSearchOffset(model, SearchOffset, SearchRows);
            }
        }
        HighlightTreeItem(SearchRows);
    }
}

bool BiosViewerWindow::eventFilter(QObject *obj, QEvent *event) {
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

void BiosViewerWindow::closeEvent(QCloseEvent *event) {
//    cleanup();
}

void BiosViewerWindow::TreeWidgetItemSelectionChanged()
{
    QModelIndex index = ui->treeWidget->currentIndex();
    if (!index.isValid())
        return;
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    DataModel * itemModel = item->data(BiosViewerData::Name, Qt::UserRole).value<DataModel*>();
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

void BiosViewerWindow::InfoButtonClicked()
{
    if (!InputData->infoWindowOpened) {
        InputData->infoWindowOpened = true;
        InputData->infoWindow = new InfoWindow(WindowData->appDir);
        if (InputData->BiosImage != nullptr) {
            InputData->infoWindow->setBiosImage(InputData->BiosImage);
            InputData->infoWindow->setOpenedFileName(WindowData->OpenedFileName);
            InputData->infoWindow->setParentWidget(this);
            InputData->infoWindow->showFitTab();
            InputData->infoWindow->showFlashmapTab(InputData->flashmap);
        }
        InputData->infoWindow->show();
    }
}

void BiosViewerWindow::refresh() {
    ui->treeWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->treeWidget->setStyleSheet(QString("QTreeView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));

    ui->treeWidget->header()->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->treeWidget->header()->setStyleSheet(QString("QTreeView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));

    ui->infoBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->AddressPanel->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
}
