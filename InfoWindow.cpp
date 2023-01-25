#include <iostream>
#include "InfoWindow.h"
#include "lib/BaseLib.h"
#include "ui_InfoWindow.h"

using BaseLibrarySpace::Buffer;
using UefiSpace::FitTableClass;

InfoWindow::InfoWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InfoWindow)
{
    ui->setupUi(this);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->horizontalHeader()->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->tableWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
}

InfoWindow::~InfoWindow()
{
    delete ui;
}

void InfoWindow::setBiosImage(BiosImageVolume *Image) {
    BiosImage = Image;
}

void InfoWindow::showFitTable() {
    QTableWidgetItem    *item;
    FIRMWARE_INTERFACE_TABLE_ENTRY  FitHeader = BiosImage->FitTable->FitHeader;
    UINT64 address = FitHeader.Address;
    QString FitSignature = QString::fromStdString(Buffer::charToString((INT8*)&address, sizeof(UINT64), false));
    INT32 FitNum = BiosImage->FitTable->FitEntryNum;

    ui->tableWidget->setColumnCount(6);
    ui->tableWidget->setRowCount(FitNum);
//    std::cout << "tableWidget->width = " << std::dec << ui->tableWidget->width() << std::endl;
//    int length = ui->tableWidget->width() / 6;
//    ui->tableWidget->setColumnWidth(InfoWindow::Address, length * 1.5);
//    ui->tableWidget->setColumnWidth(InfoWindow::Size, length);
//    ui->tableWidget->setColumnWidth(InfoWindow::Version, length);
//    ui->tableWidget->setColumnWidth(InfoWindow::C_V, length / 2);
//    ui->tableWidget->setColumnWidth(InfoWindow::Checksum, length / 2);

    item = new QTableWidgetItem(FitSignature);
    ui->tableWidget->setItem(0, InfoWindow::Address, item);

    item = new QTableWidgetItem(QString("%1").arg(Buffer::getSizeFromUINT24(FitHeader.Size), 6, 16, QLatin1Char('0')).toUpper() + "h");
    ui->tableWidget->setItem(0, InfoWindow::Size, item);

    item = new QTableWidgetItem(QString("%1").arg(FitHeader.Version, 4, 16, QLatin1Char('0')).toUpper() + "h");
    ui->tableWidget->setItem(0, InfoWindow::Version, item);

    item = new QTableWidgetItem(QString::fromStdString(FitTableClass::getTypeName(FitHeader.Type)));
    ui->tableWidget->setItem(0, InfoWindow::Type, item);

    item = new QTableWidgetItem(QString::number(FitHeader.C_V, 16).toUpper() + "h");
    ui->tableWidget->setItem(0, InfoWindow::C_V, item);

    item = new QTableWidgetItem(QString::number(FitHeader.Checksum, 16).toUpper() + "h");
    ui->tableWidget->setItem(0, InfoWindow::Checksum, item);

    for (int index = 0; index < FitNum - 1; ++index) {
        FIRMWARE_INTERFACE_TABLE_ENTRY Entry = BiosImage->FitTable->FitEntries.at(index);
        item = new QTableWidgetItem(QString("%1").arg(Entry.Address, 8, 16, QLatin1Char('0')).toUpper() + "h");
        ui->tableWidget->setItem(index + 1, InfoWindow::Address, item);

        item = new QTableWidgetItem(QString("%1").arg(Buffer::getSizeFromUINT24(Entry.Size), 6, 16, QLatin1Char('0')).toUpper() + "h");
        ui->tableWidget->setItem(index + 1, InfoWindow::Size, item);

        item = new QTableWidgetItem(QString("%1").arg(Entry.Version, 4, 16, QLatin1Char('0')).toUpper() + "h");
        ui->tableWidget->setItem(index + 1, InfoWindow::Version, item);

        item = new QTableWidgetItem(QString::fromStdString(FitTableClass::getTypeName(Entry.Type)));
        ui->tableWidget->setItem(index + 1, InfoWindow::Type, item);

        item = new QTableWidgetItem(QString::number(Entry.C_V, 16).toUpper() + "h");
        ui->tableWidget->setItem(index + 1, InfoWindow::C_V, item);

        item = new QTableWidgetItem(QString::number(Entry.Checksum, 16).toUpper() + "h");
        ui->tableWidget->setItem(index + 1, InfoWindow::Checksum, item);
    }
}

void InfoWindow::resizeEvent(QResizeEvent *event) {
    std::cout << "ui->tableWidget->width = " << std::dec << ui->tableWidget->width() << std::endl;
    int length = ui->tableWidget->width() / 6;
//    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget->setColumnWidth(InfoWindow::Address, length * 1.5);
    ui->tableWidget->setColumnWidth(InfoWindow::Size, length);
    ui->tableWidget->setColumnWidth(InfoWindow::Version, length);
    ui->tableWidget->setColumnWidth(InfoWindow::Type, length * 1.5);
    ui->tableWidget->setColumnWidth(InfoWindow::C_V, length / 2);
}
