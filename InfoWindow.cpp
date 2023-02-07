#include <iostream>
#include <QKeyEvent>
#include <QFile>
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

    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->MicrocodeTextBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->AcmTextBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));

    if (setting.value("Theme").toString() == "Dark") {
        QFile styleFile(":/qdarkstyle/dark/darkstyle.qss");
        if(styleFile.open(QIODevice::ReadOnly)) {
            QString setStyleSheet(styleFile.readAll());
            this->setStyleSheet(setStyleSheet);
            styleFile.close();
        }
    } else if (setting.value("Theme").toString() == "Light") {
        QFile styleFile(":/qdarkstyle/light/lightstyle.qss");
        if(styleFile.open(QIODevice::ReadOnly)) {
            QString setStyleSheet(styleFile.readAll());
            this->setStyleSheet(setStyleSheet);
            styleFile.close();
        }
    }
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

    showMicrocodeTable();
    showAcmTable();
}

void InfoWindow::showMicrocodeTable() {
    for (auto MicrocodeEntry:BiosImage->FitTable->MicrocodeEntries) {
        QString ItemName = "Microcode";
        if (MicrocodeEntry->isEmpty)
            ItemName += " (Empty)";
        else
            ItemName = ItemName + "  " + QString::number(MicrocodeEntry->microcodeHeader.ProcessorSignature.Uint32, 16).toUpper();
        QListWidgetItem *MicrocodeItem = new QListWidgetItem(ItemName);
        ui->microcodeListWidget->addItem(MicrocodeItem);
    }

    if (BiosImage->FitTable->MicrocodeEntries.size() != 0) {
        ui->microcodeListWidget->setCurrentRow(0);
        auto EntryHeader = BiosImage->FitTable->MicrocodeEntries.at(0);
        EntryHeader->setInfoStr();
        ui->MicrocodeTextBrowser->setText(EntryHeader->InfoStr);
    }
}

void InfoWindow::showAcmTable() {
    for (auto AcmEntry:BiosImage->FitTable->AcmEntries) {
        QString ItemName;
        if (!AcmEntry->isValid())
            ItemName = "Corrupted ACM";
        else if (AcmEntry->isProd())
            ItemName = "Prod ACM";
        else
            ItemName = "Non Prod ACM";
        QListWidgetItem *AcmItem = new QListWidgetItem(ItemName);
        ui->acmListWidget->addItem(AcmItem);
    }

    if (BiosImage->FitTable->AcmEntries.size() != 0) {
        ui->acmListWidget->setCurrentRow(0);
        auto EntryHeader = BiosImage->FitTable->AcmEntries.at(0);
        EntryHeader->setInfoStr();
        ui->AcmTextBrowser->setText(EntryHeader->InfoStr);
    }
}

void InfoWindow::resizeEvent(QResizeEvent *event) {
    int length = ui->tableWidget->width() / 6;
//    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget->setColumnWidth(InfoWindow::Address, length * 1.5);
    ui->tableWidget->setColumnWidth(InfoWindow::Size, length);
    ui->tableWidget->setColumnWidth(InfoWindow::Version, length);
    ui->tableWidget->setColumnWidth(InfoWindow::Type, length * 1.5);
    ui->tableWidget->setColumnWidth(InfoWindow::C_V, length / 2);
}

void InfoWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        this->close();
    }
}

void InfoWindow::on_microcodeListWidget_itemClicked(QListWidgetItem *item) {
    INT32 currentRow = ui->microcodeListWidget->currentRow();
    auto EntryHeader = BiosImage->FitTable->MicrocodeEntries.at(currentRow);
    EntryHeader->setInfoStr();
    ui->MicrocodeTextBrowser->setText(EntryHeader->InfoStr);
}

void InfoWindow::on_acmListWidget_itemClicked(QListWidgetItem *item)
{
    INT32 currentRow = ui->acmListWidget->currentRow();
    auto EntryHeader = BiosImage->FitTable->AcmEntries.at(currentRow);
    EntryHeader->setInfoStr();
    ui->AcmTextBrowser->setText(EntryHeader->InfoStr);
}
