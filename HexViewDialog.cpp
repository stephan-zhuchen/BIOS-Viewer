#include <QMessageBox>
#include "HexViewDialog.h"
#include "ui_HexViewDialog.h"

HexViewDialog::HexViewDialog(QWidget *parent) :
    QDialog(parent),
    m_hexview ( new QHexView ),
    ui(new Ui::HexViewDialog),
    m_layout ( new QVBoxLayout )
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    m_hexview->setFrameShape(QFrame::NoFrame);
    m_hexview->setParentWidget(this);
    m_layout->addWidget( m_hexview );
    this->setLayout ( m_layout );

    QColor color = QColor(Qt::white);
    QPalette p = this->palette();
    p.setColor(QPalette::Window,color);
    this->setPalette(p);

//    if (setting.value("Theme").toString() == "Dark") {
//        QFile styleFile(":/qdarkstyle/dark/darkstyle.qss");
//        if(styleFile.open(QIODevice::ReadOnly)) {
//            QString setStyleSheet(styleFile.readAll());
//            this->setStyleSheet(setStyleSheet);
//            styleFile.close();
//        }
//    } else if (setting.value("Theme").toString() == "Light") {
//        QFile styleFile(":/qdarkstyle/light/lightstyle.qss");
//        if(styleFile.open(QIODevice::ReadOnly)) {
//            QString setStyleSheet(styleFile.readAll());
//            this->setStyleSheet(setStyleSheet);
//            styleFile.close();
//        }
//    }
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

void HexViewDialog::keyPressEvent(QKeyEvent *event) {
    qDebug("HexViewDialog::keyPressEvent");
}
