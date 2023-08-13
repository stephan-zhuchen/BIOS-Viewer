#include <QMessageBox>
#include <QCloseEvent>
#include <QMenuBar>
#include <QActionGroup>
#include <iostream>
#include <string>
#include "HexViewDialog.h"
#include "ui_HexViewDialog.h"
#include "QHexView/qhexview.h"
#include "BaseLib.h"

HexViewDialog::HexViewDialog(bool darkMode, QWidget *parent) :
    QDialog(parent),
    m_hexview ( new QHexView(this, darkMode) ),
    ui(new Ui::HexViewDialog),
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

    connect(gotoAction,       &QAction::triggered, this, &HexViewDialog::HexDialogGoto);
    connect(searchAction,     &QAction::triggered, this, &HexViewDialog::HexDialogSearch);
    connect(AlignNoneAction,  &QAction::triggered, this, &HexViewDialog::HexDialogAlignNone);
    connect(AlignImageAction, &QAction::triggered, this, &HexViewDialog::HexDialogAlignImage);
    connect(Align4GAction,    &QAction::triggered, this, &HexViewDialog::HexDialogAlign4G);

    title = windowTitle();
    m_hexview->setFrameShape(QFrame::NoFrame);
    m_hexview->setParentWidget(this, false);
    m_layout->addWidget( m_hexview );
    this->setLayout ( m_layout );
}

HexViewDialog::~HexViewDialog()
{
    delete ui;
    delete m_hexview;
    delete m_layout;
}

void HexViewDialog::loadBuffer(QByteArray buffer, Volume *image, INT64 imageOffset, INT64 imageSize, const QString &bufferName, const QString &imageName, bool Compressed) {
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

void HexViewDialog::saveImage() {
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
    UINT8 *ChangedBuffer = OpenedImage->data + OpenedImageOffset;
    memcpy(ChangedBuffer, NewHexBuffer.data(), NewHexBuffer.size());
    BaseLibrarySpace::saveBinary(OpenedFileName.toStdString(), OpenedImage->data, 0, OpenedImage->size);
}

void HexViewDialog::setNewHexBuffer(QByteArray &buffer) {
    NewHexBuffer = buffer;
}

void HexViewDialog::keyPressEvent(QKeyEvent *event) {
}

void HexViewDialog::closeEvent(QCloseEvent *event) {
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

void HexViewDialog::setEditedState(bool edited) {
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

void HexViewDialog::HexDialogGoto() {
    m_hexview->actionGoto();
}

void HexViewDialog::HexDialogSearch() {
    m_hexview->actionSearch();
}

void HexViewDialog::HexDialogAlignNone() {
    m_hexview->setRelativeAddress();
    setting.setValue("HexAlign", "None");
}

void HexViewDialog::HexDialogAlignImage() {
    m_hexview->setRelativeAddress(OpenedImageOffset);
    setting.setValue("HexAlign", "Image");
}

void HexViewDialog::HexDialogAlign4G() {
    const INT64 ADDRESS_4G = 0x100000000;
    m_hexview->setRelativeAddress(ADDRESS_4G - OpenedImageSize + OpenedImageOffset);
    setting.setValue("HexAlign", "4G");
}
