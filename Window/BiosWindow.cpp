#include "BiosWindow.h"
#include <QCloseEvent>
#include <QMessageBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <thread>
#include "inputdialog.h"
#include "StartWindow.h"

GeneralData::GeneralData(QString dir):appDir(dir) {}

GeneralData::~GeneralData() {
    qDebug() << "GeneralData::~GeneralData";
    cleanup();
}

void GeneralData::cleanup() {
    qDebug() << "GeneralData::cleanup()";
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
    QWidget(parent), mWindow(parent), setting(QSettings("Intel", "BiosViewer"))
{
    initRightMenu();
}

BiosViewerWindow::~BiosViewerWindow() {
    cleanup();
    finiRightMenu();
}

void BiosViewerWindow::setupUi(QMainWindow *MainWindow, GeneralData *wData) {
    UiReady = true;
    WindowData = wData;
    QFont font;
    font.setPointSize(9);
    QFont font1;
    font.setPointSize(10);

    centralwidget = new QWidget(MainWindow);
    centralwidget->setObjectName("centralwidget");
    CentralwidgetVerticalLayout = new QVBoxLayout(centralwidget);
    CentralwidgetVerticalLayout->setObjectName("CentralwidgetVerticalLayout");
    verticalSpacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);
    CentralwidgetVerticalLayout->addItem(verticalSpacer);

    TitleHorizontalLayout = new QHBoxLayout();
    TitleHorizontalLayout->setObjectName("TitleHorizontalLayout");
    TitleHorizontalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
    TitleHorizontalLayout->setContentsMargins(6, -1, 6, -1);
    titleInfomation = new QLabel(centralwidget);
    titleInfomation->setObjectName("titleInfomation");
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(titleInfomation->sizePolicy().hasHeightForWidth());
    titleInfomation->setSizePolicy(sizePolicy);
    QFont titleFont;
    titleFont.setPointSize(16);
    titleInfomation->setFont(titleFont);

    TitleHorizontalLayout->addWidget(titleInfomation);

    horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    TitleHorizontalLayout->addItem(horizontalSpacer);

    searchButton = new QPushButton(centralwidget);
    searchButton->setObjectName("searchButton");
    sizePolicy.setHeightForWidth(searchButton->sizePolicy().hasHeightForWidth());
    searchButton->setSizePolicy(sizePolicy);
    searchButton->setMinimumSize(QSize(36, 29));
    QFont font3;
    font3.setPointSize(12);
    QIcon icon6;
    icon6.addFile(QString::fromUtf8(":/search.svg"), QSize(), QIcon::Normal, QIcon::Off);
    searchButton->setFont(font3);
    searchButton->setIcon(icon6);
    searchButton->setIconSize(QSize(16, 16));
    TitleHorizontalLayout->addWidget(searchButton);

    HexViewButton = new QPushButton(centralwidget);
    HexViewButton->setObjectName("HexViewButton");
    HexViewButton->setEnabled(true);
    sizePolicy.setHeightForWidth(HexViewButton->sizePolicy().hasHeightForWidth());
    HexViewButton->setSizePolicy(sizePolicy);
    HexViewButton->setMinimumSize(QSize(105, 26));
    HexViewButton->setMaximumSize(QSize(120, 16777215));
    HexViewButton->setFont(font3);
    HexViewButton->setCheckable(false);
    HexViewButton->setText("Hex View");
    TitleHorizontalLayout->addWidget(HexViewButton);

    infoButton = new QPushButton(centralwidget);
    infoButton->setObjectName("infoButton");
    infoButton->setEnabled(true);
    sizePolicy.setHeightForWidth(infoButton->sizePolicy().hasHeightForWidth());
    infoButton->setSizePolicy(sizePolicy);
    infoButton->setMinimumSize(QSize(90, 26));
    infoButton->setMaximumSize(QSize(120, 16777215));
    infoButton->setFont(font3);
    infoButton->setCheckable(false);
    infoButton->setVisible(false);
    TitleHorizontalLayout->addWidget(infoButton);

    CentralwidgetVerticalLayout->addLayout(TitleHorizontalLayout);

    verticalSpacer_2 = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);
    CentralwidgetVerticalLayout->addItem(verticalSpacer_2);

    structureLabel = new QLabel("Structure", centralwidget);
    infoLabel = new QLabel("Infomation", centralwidget);

    LittleHorizontalLayout = new QHBoxLayout();
    LittleHorizontalLayout->setObjectName("LittleHorizontalLayout");
    LittleHorizontalLayout->setContentsMargins(6, -1, -1, -1);
    verticalSpacer_3 = new QSpacerItem(20, 16, QSizePolicy::Minimum, QSizePolicy::Fixed);
    LittleHorizontalLayout->addWidget(structureLabel);
    LittleHorizontalLayout->addItem(verticalSpacer_3);
    LittleHorizontalLayout->addWidget(infoLabel);
    CentralwidgetVerticalLayout->addLayout(LittleHorizontalLayout);

    MainHorizontalLayout = new QHBoxLayout();
    MainHorizontalLayout->setObjectName("MainHorizontalLayout");
    MainHorizontalLayout->setContentsMargins(6, -1, 6, -1);
    treeWidget = new QTreeWidget(centralwidget);
    treeWidget->setObjectName("treeWidget");
    treeWidget->setEnabled(true);
    QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy1.setHorizontalStretch(1);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(treeWidget->sizePolicy().hasHeightForWidth());
    treeWidget->setSizePolicy(sizePolicy1);
    treeWidget->setMinimumSize(QSize(600, 300));
    treeWidget->setMaximumSize(QSize(1200, 16777215));
    treeWidget->setFont(font);
    treeWidget->setFrameShape(QFrame::StyledPanel);
    treeWidget->setFrameShadow(QFrame::Plain);
    treeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeWidget->setUniformRowHeights(true);
    treeWidget->setSortingEnabled(false);
    treeWidget->setAnimated(true);
    treeWidget->setAllColumnsShowFocus(false);
    treeWidget->setHeaderHidden(false);
    treeWidget->header()->setCascadingSectionResizes(true);

    MainHorizontalLayout->addWidget(treeWidget);

    InfoVerticalLayout = new QVBoxLayout();
    InfoVerticalLayout->setObjectName("InfoVerticalLayout");
    AddressPanel = new QLineEdit(centralwidget);
    AddressPanel->setObjectName("AddressPanel");
    AddressPanel->setFont(font1);
    AddressPanel->setFrame(false);
    AddressPanel->setReadOnly(true);
    InfoVerticalLayout->addWidget(AddressPanel);

    infoBrowser = new QTextBrowser(centralwidget);
    infoBrowser->setObjectName("infoBrowser");
    infoBrowser->setEnabled(true);
    QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(infoBrowser->sizePolicy().hasHeightForWidth());
    infoBrowser->setSizePolicy(sizePolicy2);
    infoBrowser->setMinimumSize(QSize(350, 0));
    QFont InfoFont;
    InfoFont.setFamilies({QString::fromUtf8("Fira Code")});
    InfoFont.setPointSize(10);
    infoBrowser->setFont(InfoFont);
    infoBrowser->viewport()->setProperty("cursor", QVariant(QCursor(Qt::IBeamCursor)));
    infoBrowser->setFrameShape(QFrame::NoFrame);
    infoBrowser->setFrameShadow(QFrame::Plain);
    infoBrowser->setLineWrapMode(QTextEdit::WidgetWidth);

    InfoVerticalLayout->addWidget(infoBrowser);
    MainHorizontalLayout->addLayout(InfoVerticalLayout);
    CentralwidgetVerticalLayout->addLayout(MainHorizontalLayout);

    MainWindow->setCentralWidget(centralwidget);
    retranslateUi(MainWindow);
    initSetting();

    connect(treeWidget,       SIGNAL(itemSelectionChanged()), this, SLOT(TreeWidgetItemSelectionChanged()));
    connect(infoButton,       SIGNAL(clicked()),   this, SLOT(InfoButtonClicked()));
    connect(searchButton,     SIGNAL(clicked()),   this, SLOT(SearchButtonClicked()));
    connect(HexViewButton,    SIGNAL(clicked()),   this, SLOT(ShowHexViewClicked()));
    connect(treeWidget,       SIGNAL(customContextMenuRequested(QPoint)), this,SLOT(showTreeRightMenu(QPoint)));
}

void BiosViewerWindow::retranslateUi(QMainWindow *MainWindow) {
    titleInfomation->setText(QCoreApplication::translate("MainWindow", "TextLabel", nullptr));
    searchButton->setText(QString());
    infoButton->setText(QCoreApplication::translate("MainWindow", "More", nullptr));
    QTreeWidgetItem *___qtreewidgetitem = treeWidget->headerItem();
    ___qtreewidgetitem->setText(2, QCoreApplication::translate("MainWindow", "SubType", nullptr));
    ___qtreewidgetitem->setText(1, QCoreApplication::translate("MainWindow", "Type", nullptr));
    ___qtreewidgetitem->setText(0, QCoreApplication::translate("MainWindow", "Name", nullptr));
}

void BiosViewerWindow::initSetting() {
    treeWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    treeWidget->setStyleSheet(QString("QTreeView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));

    treeWidget->header()->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    treeWidget->header()->setStyleSheet(QString("QTreeView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));

    infoLabel->setGeometry(treeWidget->width() + 23, 80, 100, 20);

    infoBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    AddressPanel->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));

    if (WindowData->DarkmodeFlag) {
        searchButton->setIcon(QIcon(":/search_light.svg"));
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
    titleInfomation->setText(QString::fromStdString(InputData->BiosImage->BiosID));
    if (InputData->BiosImage->FitTable == nullptr)
        infoButton->setVisible(false);
    else
        infoButton->setVisible(true);
    treeWidget->setCurrentIndex(treeWidget->model()->index(0, 0, QModelIndex()));
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

void BiosViewerWindow::resizeEvent(QResizeEvent *event) {
    if (UiReady) {
        INT32 width = treeWidget->width();
        treeWidget->setColumnWidth(BiosViewerData::Name, width - 280);
        treeWidget->setColumnWidth(BiosViewerData::Type, 120);
        infoLabel->setGeometry(treeWidget->width() + 23, 80, 100, 20);
    }
}

bool BiosViewerWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == treeWidget->viewport()) {
        //点击树的空白,取消选中
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *e = (QMouseEvent *)event;
            if (e->buttons() & Qt::LeftButton) {
                QModelIndex index = treeWidget->indexAt(e->pos());
                if (!index.isValid()) {
                    treeWidget->setCurrentIndex(QModelIndex());
                    infoBrowser->clear();
                    AddressPanel->clear();
                }
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void BiosViewerWindow::closeEvent(QCloseEvent *event) {
    cleanup();
}

void BiosViewerWindow::TreeWidgetItemSelectionChanged()
{
    QModelIndex index = treeWidget->currentIndex();
    if (!index.isValid())
        return;
    QTreeWidgetItem *item = treeWidget->currentItem();
    DataModel * itemModel = item->data(BiosViewerData::Name, Qt::UserRole).value<DataModel*>();
    Volume* volume = itemModel->modelData;
    QPalette pal(AddressPanel->palette());
    //    if (volume->isCompressed) {
    //        pal.setColor(QPalette::Base, Qt::cyan);
    //    }
    AddressPanel->setPalette(pal);
    setPanelInfo(volume->offsetFromBegin, volume->size);

    volume->setInfoStr();
    infoBrowser->setText(volume->InfoStr);
}

void BiosViewerWindow::ShowHexViewClicked() {
    mWindow->ActionHexViewTriggered();
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

void BiosViewerWindow::cleanup() {
    if (UiReady) {
        treeWidget->clear();
        titleInfomation->clear();
        infoBrowser->clear();
        AddressPanel->clear();
    }
    if (InputData != nullptr) {
        delete InputData;
        InputData = nullptr;
    }
}

void BiosViewerWindow::refresh() {
    treeWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    treeWidget->setStyleSheet(QString("QTreeView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));

    treeWidget->header()->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    treeWidget->header()->setStyleSheet(QString("QTreeView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));

    infoBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    AddressPanel->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
}
