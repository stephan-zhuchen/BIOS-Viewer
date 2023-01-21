#include "hexviewwidget.h"
#include "ui_hexviewwidget.h"

HexViewWidget::HexViewWidget(QWidget *parent) :
    QWidget(parent),
    m_hexview ( new QHexView ),
    ui(new Ui::HexViewWidget),
    m_layout ( new QVBoxLayout )
{
    ui->setupUi(this);

    QColor color = QColor(Qt::white);
    QPalette p = this->palette();
    p.setColor(QPalette::Window,color);
    this->setPalette(p);

    m_hexview->setFrameShape(QFrame::Box);
//    m_hexview->setFocus();
//    m_hexview->setFocusProxy(this);
    m_hexview->setFocusPolicy(Qt::StrongFocus);
    m_layout->addWidget( m_hexview );
//    ui->widgetHex->setLayout ( m_layout );
    this->setLayout(m_layout);

//    setFocusPolicy(Qt::NoFocus);
}

HexViewWidget::~HexViewWidget()
{
    delete ui;
}

//void HexViewWidget::keyPressEvent(QKeyEvent *event) {
//    qDebug("keyPressEvent");
//}

//void HexViewWidget::mousePressEvent(QMouseEvent *event) {
//    qDebug("mousePressEvent");
//}
