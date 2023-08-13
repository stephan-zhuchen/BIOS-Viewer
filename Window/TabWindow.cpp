#include <QTextBrowser>
#include <QKeyEvent>
#include "TabWindow.h"
#include "ui_TabWindow.h"

TabWindow::TabWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setObjectName("verticalLayout");
    tabWidget = new QTabWidget(this);
    tabWidget->setObjectName("tabWidget");
    tabWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));

    QSettings windowSettings("Intel", "BiosViewer");
    restoreGeometry(windowSettings.value("TabWindow/geometry").toByteArray());
}

TabWindow::~TabWindow()
{
    delete ui;
    delete verticalLayout;
    delete tabWidget;
    QSettings windowSettings("Intel", "BiosViewer");
    windowSettings.setValue("TabWindow/geometry", saveGeometry());
}

void TabWindow::SetNewTabAndText(const QString& tabName, const QString& txt) {
    auto *NewTab = new QWidget();
    QVBoxLayout *LayoutInThisTab;
    QTextBrowser *TabTextBrowser;
    LayoutInThisTab = new QVBoxLayout(NewTab);
    LayoutInThisTab->setContentsMargins(0, 0, 0, 0);
    TabTextBrowser = new QTextBrowser(NewTab);
    TabTextBrowser->setLineWrapMode(QTextEdit::NoWrap);
    TabTextBrowser->setText(txt);
    TabTextBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    LayoutInThisTab->addWidget(TabTextBrowser);
    tabWidget->addTab(NewTab, QString());
    tabWidget->setTabText(tabWidget->indexOf(NewTab), tabName);
}

void TabWindow::SetTabViewTitle(const QString& title) {
    this->setWindowTitle(title);
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
