#include <QMessageBox>
#include <QCloseEvent>
#include <iostream>
#include <string>
#include "HexViewDialog.h"
#include "ui_HexViewDialog.h"
#include "lib/QHexView/qhexview.h"
#include "lib/BaseLib.h"

HexViewDialog::HexViewDialog(QWidget *parent) :
    QDialog(parent),
    m_hexview ( new QHexView() ),
    ui(new Ui::HexViewDialog),
    m_layout ( new QVBoxLayout ),
    setting(QSettings("Intel", "BiosViewer"))
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    title = windowTitle();
    m_hexview->setFrameShape(QFrame::NoFrame);
    m_hexview->setParentWidget(this);
    m_layout->addWidget( m_hexview );
    this->setLayout ( m_layout );
}

HexViewDialog::~HexViewDialog()
{
    delete ui;
    delete m_hexview;
    delete m_layout;
}

void HexViewDialog::loadFile()
{
//  setWindowTitle("Hex Viewer - ");
//  try
//  {
//    m_hexview->loadFile ( m_fileName );
//    m_hexview->setAddressLength();
//    m_hexview->setfileOpened(true);
//  }
//  catch ( std::exception &e )
//  {
//    std::string errorMessage ( "Loading Error: " );
//    errorMessage.append ( e.what() );

//    QMessageBox msgBox;
//    msgBox.setText ( errorMessage.c_str() );
//    msgBox.exec();
//    return;
//  }
}

void HexViewDialog::loadBuffer(QByteArray buffer, Volume *image, INT64 imageOffset, const QString &bufferName, const QString &imageName, bool Compressed) {
    hexBuffer = buffer;
    NewHexBuffer = buffer;
    OpenedImage = image;
    OpenedImageOffset = imageOffset;
    title += bufferName;
    OpenedFileName = imageName;
    isCompressed = Compressed;
    setWindowTitle(title);
    m_hexview->setReadOnly(isCompressed);
    if (setting.value("EnableHexEditing") == "false") {
        m_hexview->setReadOnly(true);
    }
    m_hexview->loadFromBuffer(buffer);
}

void HexViewDialog::saveImage() {
    // save backup image
    std::string NewFileName = OpenedFileName.toStdString() + ".bak";
    if (rename(OpenedFileName.toStdString().c_str(), NewFileName.c_str())) {
        qDebug("rename error");
    }

    // save edited image
    UINT8 *ChangedBuffer = OpenedImage->data + OpenedImageOffset;
    memcpy(ChangedBuffer, NewHexBuffer.data(), NewHexBuffer.size());
    BaseLibrarySpace::Buffer::saveBinary(OpenedFileName.toStdString(), OpenedImage->data, 0, OpenedImage->size);
}

void HexViewDialog::setNewHexBuffer(QByteArray &buffer) {
    NewHexBuffer = buffer;
}

void HexViewDialog::keyPressEvent(QKeyEvent *event) {
    qDebug("HexViewDialog::keyPressEvent");
}

void HexViewDialog::closeEvent(QCloseEvent *event) {
    if (BinaryEdited) {
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
    if (!edited && BinaryEdited) { // first save
        saveImage();
    }
    BinaryEdited = edited;
    if (edited) {
        setWindowTitle(title + "*");
    } else {
        setWindowTitle(title);
    }
}
