#include "PeCoffInfo.h"
#include "ui_PeCoffInfo.h"

PeCoffInfo::PeCoffInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PeCoffInfo)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->AsmTextBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->HeaderTextBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->RelocationTextBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
}

PeCoffInfo::~PeCoffInfo()
{
    delete ui;
}

void PeCoffInfo::setAsmText(QString txt) {
    ui->AsmTextBrowser->setText(txt);
}

void PeCoffInfo::setHeaderText(QString txt) {
    ui->HeaderTextBrowser->setText(txt);
}

void PeCoffInfo::setRelocationText(QString txt) {
    ui->RelocationTextBrowser->setText(txt);
}
