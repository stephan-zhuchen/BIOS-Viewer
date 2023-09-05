#include "HexWindow.h"
#include "Start/StartWindow.h"
#include <HexView/HexView.h>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFormLayout>
#include <QDialogButtonBox>


HexViewWindow::HexViewWindow(StartWindow *parent) :
    QWidget(parent), mWindow(parent), setting(QSettings("Intel", "BiosViewer"))
{
    setAttribute(Qt::WA_DeleteOnClose);
}

HexViewWindow::~HexViewWindow() = default;

void HexViewWindow::setupUi(QMainWindow *MainWindow, GeneralData *wData) {
    WindowData = wData;
    centralwidget = new QWidget(MainWindow);
    centralwidget->setObjectName("centralwidget");
    CentralwidgetVerticalLayout = new QVBoxLayout(centralwidget);
    CentralwidgetVerticalLayout->setObjectName("CentralwidgetVerticalLayout");
    CentralwidgetVerticalLayout->setContentsMargins(0, 0, 0, 0);

    m_hexview =  new QHexView(this, WindowData->DarkmodeFlag);
    m_hexview->setFrameShape(QFrame::NoFrame);
    m_hexview->setParentWidget(this, true);
    CentralwidgetVerticalLayout->addWidget( m_hexview );

    MainWindow->setCentralWidget(centralwidget);
}

void HexViewWindow::refresh() const {
    m_hexview->refresh();
}

void HexViewWindow::closeEvent(QCloseEvent *event) {
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
            mWindow->setWindowTitle(WindowData->WindowTitle);
            event->ignore();
        } else {
            event->ignore();
        }
    }
}

void HexViewWindow::setEditedState(bool edited) {
    if (edited == BinaryEdited) { // first save
        return;
    }
    BinaryEdited = edited;
    if (edited) {
        mWindow->setWindowTitle(WindowData->WindowTitle + " *");
    } else {
        mWindow->setWindowTitle(WindowData->WindowTitle);
    }
}

void HexViewWindow::saveImage() {
    // save backup image
    std::string NewFileName = WindowData->OpenedFileName.toStdString() + ".bak";
    if (rename(WindowData->OpenedFileName.toStdString().c_str(), NewFileName.c_str())) {
        qDebug("rename error");
    }

    // save edited image
    BaseLibrarySpace::saveBinary(WindowData->OpenedFileName.toStdString(), (UINT8*)NewHexBuffer.data(), 0, NewHexBuffer.size());
}

void HexViewWindow::setNewHexBuffer(QByteArray &buffer) {
    NewHexBuffer = buffer;
}

void HexViewWindow::ActionSearchHexTriggered() const {
    m_hexview->actionSearch();
}

void HexViewWindow::ActionGotoTriggered() const {
    m_hexview->actionGoto();
}

void HexViewWindow::loadBuffer(UINT8 *image, INT64 imageLength) {
    hexBuffer = QByteArray((CHAR8*)image, imageLength);
    if (setting.value("EnableHexEditing").toString() == "false") {
        m_hexview->setReadOnly(true);
    }
    m_hexview->loadFromBuffer(hexBuffer);
}
