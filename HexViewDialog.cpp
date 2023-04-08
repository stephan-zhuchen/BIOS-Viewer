#include <QMessageBox>
#include "HexViewDialog.h"
#include "ui_HexViewDialog.h"

HexViewDialog::HexViewDialog(QString &applicationDir, QWidget *parent) :
    QDialog(parent),
    m_hexview ( new QHexView(applicationDir) ),
    ui(new Ui::HexViewDialog),
    m_layout ( new QVBoxLayout )
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

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

void HexViewDialog::keyPressEvent(QKeyEvent *event) {
    qDebug("HexViewDialog::keyPressEvent");
}
