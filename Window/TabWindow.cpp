#include <QTextBrowser>
#include <QKeyEvent>
#include "TabWindow.h"
#include "ui_TabWindow.h"

TabWindow::TabWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabWindow)
{
    ui->setupUi(this);
    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setObjectName("verticalLayout");
    tabWidget = new QTabWidget(this);
    tabWidget->setObjectName("tabWidget");
    tabWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
}

TabWindow::~TabWindow()
{
    delete ui;
    delete verticalLayout;
    delete tabWidget;
}

void TabWindow::SetNewTabAndText(QString tabName, QString txt) {
    QWidget *NewTab = new QWidget();
//    NewTab->setObjectName("NewTab");
    QVBoxLayout *LayoutInThisTab;
    QTextBrowser *TabTextBrowser;
    LayoutInThisTab = new QVBoxLayout(NewTab);
//    LayoutInThisTab->setObjectName("LayoutInThisTab");
    LayoutInThisTab->setContentsMargins(0, 0, 0, 0);
    TabTextBrowser = new QTextBrowser(NewTab);
//    TabTextBrowser->setObjectName("TabTextBrowser");
    TabTextBrowser->setLineWrapMode(QTextEdit::NoWrap);
    TabTextBrowser->setText(txt);
    TabTextBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    LayoutInThisTab->addWidget(TabTextBrowser);
    tabWidget->addTab(NewTab, QString());
    tabWidget->setTabText(tabWidget->indexOf(NewTab), tabName);
}

void TabWindow::CollectTabAndShow() {
    verticalLayout->addWidget(tabWidget);
    this->show();
}

void TabWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        this->close();
    }
}
