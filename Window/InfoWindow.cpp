#include <iostream>
#include <QKeyEvent>
#include <QFile>
#include "mainwindow.h"
#include "InfoWindow.h"
#include "BaseLib.h"
#include "ui_InfoWindow.h"

using BaseLibrarySpace::Buffer;
using UefiSpace::FitTableClass;

InfoWindow::InfoWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InfoWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->horizontalHeader()->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->tableWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));

    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->MicrocodeTextBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->AcmTextBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->BtgTextBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->flashmapText->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));

    connect(ui->microcodeListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(microcodeListWidgetItemSelectionChanged()));
    connect(ui->acmListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(acmListWidgetItemSelectionChanged()));
    connect(ui->BtgListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(BtgListWidgetItemSelectionChanged()));
}

InfoWindow::~InfoWindow()
{
    delete ui;
}

void InfoWindow::setBiosImage(BiosImageVolume *Image) {
    BiosImage = Image;
}

void InfoWindow::setParentWidget(QWidget *pWidget) {
    parentWidget = pWidget;
}

void InfoWindow::showFitTab() {
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

    if (BiosImage->FitTable->isChecksumValid) {
        item = new QTableWidgetItem(QString::number(FitHeader.Checksum, 16).toUpper() + "h (Valid)");
    } else {
        item = new QTableWidgetItem(QString::number(FitHeader.Checksum, 16).toUpper() + "h (Invalid)");
    }
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

    showMicrocodeTab();
    showAcmTab();
    showBtgTab();
}

void InfoWindow::showMicrocodeTab() {
    for (auto MicrocodeEntry:BiosImage->FitTable->MicrocodeEntries) {
        QString ItemName = "Microcode";
        if (MicrocodeEntry->isEmpty)
            ItemName += " (Empty)";
        else
            ItemName = ItemName + "  " + QString::number(MicrocodeEntry->microcodeHeader.ProcessorSignature.Uint32, 16).toUpper();
        QListWidgetItem *MicrocodeItem = new QListWidgetItem(ItemName);
        ui->microcodeListWidget->addItem(MicrocodeItem);
    }

    if (ui->microcodeListWidget->model()->rowCount() != 0)
        ui->microcodeListWidget->setCurrentRow(0);
}

void InfoWindow::showAcmTab() {
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

    if (ui->acmListWidget->model()->rowCount() != 0)
        ui->acmListWidget->setCurrentRow(0);
}

void InfoWindow::showBtgTab() {
    QString ItemName;
    if (BiosImage->FitTable->KmEntry != nullptr) {
        ItemName = "Key Manifest";
        if (!BiosImage->FitTable->KmEntry->isValid)
            ItemName += " (Empty)";
        QListWidgetItem *KmItem = new QListWidgetItem(ItemName);
        ui->BtgListWidget->addItem(KmItem);
    }
    if (BiosImage->FitTable->BpmEntry != nullptr) {
        ItemName = "Boot Policy Manifest";
        if (!BiosImage->FitTable->BpmEntry->isValid)
            ItemName += " (Empty)";
        QListWidgetItem *BpItem = new QListWidgetItem(ItemName);
        ui->BtgListWidget->addItem(BpItem);
    }

    if (ui->BtgListWidget->model()->rowCount() != 0)
        ui->BtgListWidget->setCurrentRow(0);
}

void InfoWindow::showFlashmapTab(const QString &SectionFlashMap) {
    QString header = "Start (hex)   End (hex)     Length (hex)  Area Name\n"
                     "-----------   ---------     ------------  ---------\n\n";
    header += SectionFlashMap;
    ui->flashmapText->setText(header);
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

void InfoWindow::closeEvent(QCloseEvent *event) {
    ((MainWindow*)parentWidget)->setInfoWindowState(false);
}

void InfoWindow::microcodeListWidgetItemSelectionChanged() {
    using UefiSpace::MicrocodeHeaderClass;
    INT32 currentRow = ui->microcodeListWidget->currentRow();
    MicrocodeHeaderClass* EntryHeader = BiosImage->FitTable->MicrocodeEntries.at(currentRow);
    EntryHeader->setInfoStr();
    ui->MicrocodeTextBrowser->setText(EntryHeader->InfoStr);
}

void InfoWindow::acmListWidgetItemSelectionChanged()
{
    using UefiSpace::AcmHeaderClass;
    INT32 currentRow = ui->acmListWidget->currentRow();
    AcmHeaderClass* EntryHeader = BiosImage->FitTable->AcmEntries.at(currentRow);
    EntryHeader->setInfoStr();
    ui->AcmTextBrowser->setText(EntryHeader->InfoStr);
}

void InfoWindow::BtgListWidgetItemSelectionChanged()
{
    using UefiSpace::KeyManifestClass;
    using UefiSpace::BootPolicyManifestClass;
    QModelIndex index = ui->BtgListWidget->currentIndex();
    if (!index.isValid())
        return;
    QListWidgetItem *item = ui->BtgListWidget->currentItem();

    QString text;
    if (item->text() == "Key Manifest") {
        KeyManifestClass* EntryHeader = BiosImage->FitTable->KmEntry;
        EntryHeader->setInfoStr();
        text = EntryHeader->InfoStr;
    } else if (item->text() == "Boot Policy Manifest") {
        BootPolicyManifestClass* EntryHeader = BiosImage->FitTable->BpmEntry;
        EntryHeader->setInfoStr();
        text = EntryHeader->InfoStr;
    }

    ui->BtgTextBrowser->setText(text);
}
