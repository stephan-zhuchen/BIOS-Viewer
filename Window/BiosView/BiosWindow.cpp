#include "BiosWindow.h"
#include <QCloseEvent>
#include <QMessageBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <thread>
#include <utility>
#include <QFileInfo>
#include "Input/inputdialog.h"
#include "Start/StartWindow.h"
#include "IfwiRegion/FlashDescriptorRegion.h"
#include "UefiFileSystem/FirmwareVolume.h"
#include "CapsuleView/CapsuleWindow.h"
#include "ui_BiosWindow.h"

using namespace BaseLibrarySpace;

GeneralData::GeneralData(QString dir):appDir(std::move(dir)) {}

GeneralData::~GeneralData() {
    safeDelete(BiosViewerUi);
    safeDelete(HexViewerUi);
    safeDelete(CapsuleViewerUi);
    safeDelete(InputImage);
}

BiosViewerData::~BiosViewerData() {
    BiosValidFlag = true;

    if (infoWindowOpened) {
        infoWindow->close();
        infoWindow = nullptr;
    }
    if (searchDialogOpened) {
        BiosSearchDialog->close();
        BiosSearchDialog = nullptr;
    }

    for (Volume *vol:VolumeDataList) {
        safeDelete(vol);
    }

    safeDelete(OverviewVolume);
    if (!IFWI_exist) {
        BiosImage->ChildVolume.clear();
        safeDelete(BiosImage);
    }
    safeDelete(OverviewImageModel);
}

bool BiosViewerData::isValidBIOS(UINT8 *image, INT64 imageLength) {
    if (imageLength == 0x2000000) {
        FlashDescriptorRegion flashDescriptor = FlashDescriptorRegion(image, imageLength, 0);
        if (flashDescriptor.CheckValidation()) {
            return true;
        }
    }

    INT64 SearchOffset = 0;
    INT64 SearchInterval = 0x1000;
    while (SearchOffset <= imageLength - SearchInterval) {
        FirmwareVolume fvHeader = FirmwareVolume(image + SearchOffset, imageLength - SearchOffset, SearchOffset);
        if (fvHeader.CheckValidation()) {
            return true;
        }
        SearchOffset += SearchInterval;
    }

    return false;
}

BiosViewerWindow::BiosViewerWindow(StartWindow *parent):
    QWidget(parent),
    ui(new Ui::BiosWindow),
    mWindow(parent),
    setting(QSettings("Intel", "BiosViewer"))
{
    InitCustomMenu();
}

BiosViewerWindow::~BiosViewerWindow() {
    delete ui;
    safeDelete(BiosData);
    CleanupCustomMenu();
}

void BiosViewerWindow::setupUi(QMainWindow *MainWindow, GeneralData *wData) {
    WindowData = wData;
    ui->setupUi(MainWindow);

    initSetting();
    connect(ui->treeWidget,       SIGNAL(itemSelectionChanged()), this, SLOT(TreeWidgetItemSelectionChanged()));
    connect(ui->infoButton,       SIGNAL(clicked()),   this, SLOT(InfoButtonClicked()));
    connect(ui->searchButton,     SIGNAL(clicked()),   this, SLOT(SearchButtonClicked()));
    connect(ui->treeWidget,       SIGNAL(customContextMenuRequested(QPoint)), this,SLOT(showTreeCustomMenu(QPoint)));
}

void BiosViewerWindow::initSetting() const {
    ui->treeWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->treeWidget->setStyleSheet(QString("QTreeView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));

    ui->treeWidget->header()->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->treeWidget->header()->setStyleSheet(QString("QTreeView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));
    ui->treeWidget->header()->setSectionResizeMode(treeColNum::Name, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(treeColNum::Type, QHeaderView::Fixed);
    ui->treeWidget->header()->setSectionResizeMode(treeColNum::SubType, QHeaderView::Fixed);
    ui->treeWidget->header()->resizeSection(treeColNum::Type, 120);
    ui->treeWidget->header()->resizeSection(treeColNum::SubType, 140);
    ui->treeWidget->header()->setStretchLastSection(false);

    ui->infoBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->AddressPanel->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));

    if (WindowData->DarkmodeFlag) {
        ui->searchButton->setIcon(QIcon(":/search_light.svg"));
    }
}

void BiosViewerWindow::setInfoWindowState(bool opened) const {
    BiosData->infoWindowOpened = opened;
}

bool BiosViewerWindow::TryOpenBios(UINT8 *image, INT64 imageLength) {
    return BiosViewerData::isValidBIOS(image, imageLength);
}

void BiosViewerWindow::loadBios() {
    BiosData = new BiosViewerData;
    BiosData->OverviewVolume = new Volume(WindowData->InputImage, WindowData->InputImageSize);
    BiosData->OverviewImageModel = new DataModel(BiosData->OverviewVolume, "IFWI Overview", "Image");
    setBiosFvData();
    setFfsData();
    setTreeData();
    if (BiosData->BiosValidFlag && BiosData->BiosImage->isFitValid()) {
        if (!BiosData->IFWI_exist) {
            for (auto vol:BiosData->VolumeDataList) {
                BiosData->BiosImage->ChildVolume.push_back(vol);
            }
        }
        BiosData->BiosImage->setBiosID();
        BiosData->BiosImage->setInfoStr();
        BiosData->OverviewVolume->setInfoText(BiosData->BiosImage->getInfoText());
    }

    QString title;
    if (BiosData->BiosImage->getBiosID() == "") {
        QFileInfo fileInfo(WindowData->OpenedFileName);
        title = fileInfo.fileName();
    } else
        title = BiosData->BiosImage->getBiosID();

    ui->titleInfomation->setText(title);
    if (BiosData->BiosImage->isFitValid())
        ui->infoButton->setVisible(true);
    else
        ui->infoButton->setVisible(false);
    ui->treeWidget->setCurrentIndex(ui->treeWidget->model()->index(0, 0, QModelIndex()));
    resizeEvent(nullptr);
}

void BiosViewerWindow::setSearchDialogState(bool opened) const {
    BiosData->searchDialogOpened = opened;
}

void BiosViewerWindow::ActionSearchBiosTriggered() {
    if (!BiosData->searchDialogOpened) {
        BiosData->searchDialogOpened = true;
        BiosData->BiosSearchDialog = new BiosSearch;
        BiosData->BiosSearchDialog->setDarkMode(isDarkMode());
        BiosData->BiosSearchDialog->SetTreeData(ui->treeWidget);
        if (isDarkMode())
            BiosData->BiosSearchDialog->setWindowIcon(QIcon(":/search_light.svg"));
        BiosData->BiosSearchDialog->show();
        connect(BiosData->BiosSearchDialog, SIGNAL(closeSignal(bool)), this, SLOT(setSearchDialogState(bool)));
    } else {
        BiosData->BiosSearchDialog->activateWindow();
    }
}

void BiosViewerWindow::ActionGotoTriggered() {
    if ( BiosData->BiosImage == nullptr )
        return;

    QDialog dialog(this);
    dialog.setWindowTitle("Goto ...");
    QFormLayout form(&dialog);
    // Offset
    QString Offset = QString("Offset: ");
    auto *OffsetSpinbox = new HexSpinBox(&dialog);
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

        QList<QTreeWidgetItem *> allItems = ui->treeWidget->findItems(QString("*"), Qt::MatchWildcard|Qt::MatchRecursive);
        for (QTreeWidgetItem *item : allItems) {
            auto *itemVolume = item->data(treeColNum::Name, Qt::UserRole).value<Volume*>();
            INT64 VolumeBase = itemVolume->getOffset();
            INT64 VolumeLimit = itemVolume->getOffset() + itemVolume->getSize();
            if (!itemVolume->isCompressed() && (SearchOffset >= VolumeBase) && (SearchOffset < VolumeLimit)) {
                ui->treeWidget->expandItem(item);
                ui->treeWidget->scrollToItem(item);
                ui->treeWidget->setCurrentItem(item);
            }
        }
    }
}

bool BiosViewerWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->treeWidget->viewport()) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto *e = (QMouseEvent *)event;
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

void BiosViewerWindow::TreeWidgetItemSelectionChanged() const {
    QModelIndex index = ui->treeWidget->currentIndex();
    if (!index.isValid())
        return;
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    auto *itemVolume = item->data(treeColNum::Name, Qt::UserRole).value<Volume*>();
    QPalette pal(ui->AddressPanel->palette());
    ui->AddressPanel->setPalette(pal);
    setPanelInfo(itemVolume->getOffset(), itemVolume->getSize());

    itemVolume->setInfoStr();
    ui->infoBrowser->setText(itemVolume->getInfoText());
}

void BiosViewerWindow::InfoButtonClicked() {
    if (!BiosData->infoWindowOpened) {
        BiosData->infoWindowOpened = true;
        BiosData->infoWindow = new InfoWindow(WindowData->appDir);
        if (BiosData->BiosImage != nullptr) {
            BiosData->infoWindow->setBiosImage(BiosData->BiosImage);
            BiosData->infoWindow->setOpenedFileName(WindowData->OpenedFileName);
            BiosData->infoWindow->setParentWidget(this);
            BiosData->infoWindow->showTab();
        }
        BiosData->infoWindow->show();
    } else {
        BiosData->infoWindow->activateWindow();
    }
}

void BiosViewerWindow::refresh() const {
    ui->treeWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->treeWidget->setStyleSheet(QString("QTreeView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));

    ui->treeWidget->header()->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->treeWidget->header()->setStyleSheet(QString("QTreeView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));

    ui->infoBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->AddressPanel->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
}
