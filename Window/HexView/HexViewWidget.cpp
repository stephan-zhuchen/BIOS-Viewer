#include <QMessageBox>
#include <QCloseEvent>
#include <QMenuBar>
#include <QActionGroup>
#include <iostream>
#include <string>
#include "HexViewWidget.h"
#include "ui_HexViewWidget.h"
#include "HexView/HexView.h"
#include "BaseLib.h"

HexViewWidget::HexViewWidget(bool darkMode, QWidget *parent) :
    QWidget(parent),
    m_hexview ( new QHexView(this) ),
    ui(new Ui::HexViewWidget),
    m_layout ( new QVBoxLayout ),
    setting(QSettings("Intel", "BiosViewer"))
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    QSettings windowSettings("Intel", "BiosViewer");
    restoreGeometry(windowSettings.value("HexDialog/geometry").toByteArray());

    QMenuBar *HexMenuBar = new QMenuBar;
    QMenu *StartMenu = new QMenu(tr("Start"), this);
    QAction *searchAction = StartMenu->addAction(tr("Search"));
    QAction *gotoAction = StartMenu->addAction(tr("Goto"));
    if (setting.value("Theme") == "Light") {
        searchAction->setIcon(QIcon(":/search.svg"));
        gotoAction->setIcon(QIcon(":/bookmark.svg"));
    } else {
        searchAction->setIcon(QIcon(":/search_light.svg"));
        gotoAction->setIcon(QIcon(":/bookmark_light.svg"));
    }
    HexMenuBar->addMenu(StartMenu);

    QMenu *addressMenu = new QMenu(tr("Address"), this);
    QActionGroup *alignGroup = new QActionGroup(this);
    alignGroup->setExclusive(true);

    AlignNoneAction = addressMenu->addAction(tr("No Align"));
    AlignNoneAction->setCheckable(true);
    alignGroup->addAction(AlignNoneAction);

    AlignImageAction = addressMenu->addAction(tr("Align to whole Image"));
    AlignImageAction->setCheckable(true);
    alignGroup->addAction(AlignImageAction);

    Align4GAction = addressMenu->addAction(tr("Align to 4G"));
    Align4GAction->setCheckable(true);
    alignGroup->addAction(Align4GAction);

    HexMenuBar->addMenu(addressMenu);
    m_layout->setMenuBar(HexMenuBar);

    connect(gotoAction,       &QAction::triggered, this, &HexViewWidget::HexDialogGoto);
    connect(searchAction,     &QAction::triggered, this, &HexViewWidget::HexDialogSearch);
    connect(AlignNoneAction,  &QAction::triggered, this, &HexViewWidget::HexDialogAlignNone);
    connect(AlignImageAction, &QAction::triggered, this, &HexViewWidget::HexDialogAlignImage);
    connect(Align4GAction,    &QAction::triggered, this, &HexViewWidget::HexDialogAlign4G);

    title = windowTitle();
    m_hexview->setFrameShape(QFrame::NoFrame);
    m_hexview->setParentWidget(this, false);
    m_layout->addWidget( m_hexview );
    this->setLayout ( m_layout );
}

HexViewWidget::~HexViewWidget()
{
    delete ui;
    delete m_hexview;
    delete m_layout;
}

void HexViewWidget::loadBuffer(QByteArray buffer, Volume *image, INT64 imageOffset, INT64 imageSize, const QString &bufferName, const QString &imageName, bool Compressed) {
    hexBuffer = buffer;
    NewHexBuffer = buffer;
    OpenedImage = image;
    OpenedImageOffset = imageOffset;
    OpenedImageSize = imageSize;
    title += " - " + bufferName;
    OpenedFileName = imageName;
    isCompressed = Compressed;
    setWindowTitle(title);
    m_hexview->setReadOnly(isCompressed);
    if (setting.value("EnableHexEditing") == "false") {
        m_hexview->setReadOnly(true);
    }
    m_hexview->loadFromBuffer(buffer);

    if (isCompressed) {
        AlignNoneAction->setDisabled(true);
        AlignImageAction->setDisabled(true);
        Align4GAction->setDisabled(true);
        m_hexview->setRelativeAddress();
    } else {
        if (setting.value("HexAlign") == "None") {
            AlignNoneAction->setChecked(true);
            HexDialogAlignNone();
        } else if (setting.value("HexAlign") == "Image") {
            AlignImageAction->setChecked(true);
            HexDialogAlignImage();
        } else if (setting.value("HexAlign") == "4G") {
            Align4GAction->setChecked(true);
            HexDialogAlign4G();
        }
    }
}

void HexViewWidget::saveImage() {
    if (isCompressed) {
        int choice = QMessageBox::warning(this,
                                          tr("Hex Viewer"),
                                          tr("Compressed contents can not be edited!"),
                                          QMessageBox::Discard);
        if (choice == QMessageBox::Discard) {
            m_hexview->loadFromBuffer(hexBuffer);
            BinaryEdited = false;
            setWindowTitle(title);
        }
        return;
    }

    // save backup image
    std::string NewFileName = OpenedFileName.toStdString() + ".bak";
    if (rename(OpenedFileName.toStdString().c_str(), NewFileName.c_str())) {
        qDebug("rename error");
    }

    // save edited image
    UINT8 *ChangedBuffer = OpenedImage->getData() + OpenedImageOffset;
    memcpy(ChangedBuffer, NewHexBuffer.data(), NewHexBuffer.size());
    BaseLibrarySpace::saveBinary(OpenedFileName.toStdString(), OpenedImage->getData(), 0, OpenedImage->getSize());
}

void HexViewWidget::setNewHexBuffer(QByteArray &buffer) {
    NewHexBuffer = buffer;
}

void HexViewWidget::keyPressEvent(QKeyEvent *event) {
}

void HexViewWidget::closeEvent(QCloseEvent *event) {
    QSettings windowSettings("Intel", "BiosViewer");
    windowSettings.setValue("HexDialog/geometry", saveGeometry());

    if (BinaryEdited && isCompressed) {
        int choice = QMessageBox::warning(this,
                                          tr("Hex Viewer"),
                                          tr("Compressed contents can not be edited!"),
                                          QMessageBox::Discard | QMessageBox::Cancel);
        if (choice == QMessageBox::Discard) {
            m_hexview->loadFromBuffer(hexBuffer);
            BinaryEdited = false;
            setWindowTitle(title);
        } else {
            event->ignore();
        }
        return;
    }
    else if (BinaryEdited) {
        int choice = QMessageBox::warning(this,
                                          tr("Hex Viewer"),
                                          tr("Unsaved changes"),
                                          QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                          QMessageBox::Save);
        if (choice == QMessageBox::Save) {
            saveImage();
        } else if (choice == QMessageBox::Discard) {
            m_hexview->loadFromBuffer(hexBuffer);
            BinaryEdited = false;
            setWindowTitle(title);
            event->ignore();
        } else {
            event->ignore();
        }
    }
}

void HexViewWidget::setEditedState(bool edited) {
    if (edited == BinaryEdited) { // first save
        return;
    }
    BinaryEdited = edited;
    if (edited) {
        setWindowTitle(title + "*");
    } else {
        setWindowTitle(title);
    }
}

void HexViewWidget::HexDialogGoto() {
    m_hexview->actionGoto();
}

void HexViewWidget::HexDialogSearch() {
    m_hexview->actionSearch();
}

void HexViewWidget::HexDialogAlignNone() {
    m_hexview->setRelativeAddress();
    setting.setValue("HexAlign", "None");
}

void HexViewWidget::HexDialogAlignImage() {
    m_hexview->setRelativeAddress(OpenedImageOffset);
    setting.setValue("HexAlign", "Image");
}

void HexViewWidget::HexDialogAlign4G() {
    const INT64 ADDRESS_4G = 0x100000000;
    m_hexview->setRelativeAddress(ADDRESS_4G - OpenedImageSize + OpenedImageOffset);
    setting.setValue("HexAlign", "4G");
}
