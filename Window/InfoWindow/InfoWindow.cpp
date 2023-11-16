#include <iostream>
#include <QKeyEvent>
#include <QFile>
#include <QProcess>
#include <QDir>
#include <thread>
#include <utility>
#include "BiosView/BiosWindow.h"
#include "InfoWindow.h"
#include "BaseLib.h"
#include "ui_InfoWindow.h"
#include "Feature/AcmClass.h"
#include "Feature/MicrocodeClass.h"

using namespace BaseLibrarySpace;

InfoWindow::InfoWindow(QString Dir, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InfoWindow),
    appDir(std::move(Dir))
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->horizontalHeader()->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->tableWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));

    ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->microcodeListWidget->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->MicrocodeTextBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->acmListWidget->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->AcmTextBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->BtgListWidget->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->BtgTextBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->AcpiListWidget->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));
    ui->AcpiTextBrowser->setFont(QFont(setting.value("InfoFont").toString(), setting.value("InfoFontSize").toInt()));

    connect(ui->microcodeListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(microcodeListWidgetItemSelectionChanged()));
    connect(ui->acmListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(acmListWidgetItemSelectionChanged()));
    connect(ui->BtgListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(BtgListWidgetItemSelectionChanged()));
    connect(ui->AcpiListWidget, SIGNAL(itemSelectionChanged()), this, SLOT(AcpiListWidgetItemSelectionChanged()));

    restoreGeometry(setting.value("InfoDialog/geometry").toByteArray());
}

InfoWindow::~InfoWindow() {
    delete ui;
}

void InfoWindow::setBiosImage(BiosRegion *Image) {
    BiosImage = Image;
}

void InfoWindow::setOpenedFileName(QString name) {
    OpenedFileName = std::move(name);
}

void InfoWindow::setParentWidget(QWidget *pWidget) {
    parentWidget = pWidget;
}

void InfoWindow::showTab() {
    std::thread showFit(&InfoWindow::showFitTab, this);
    showFit.detach();

    std::thread showMicrocode(&InfoWindow::showMicrocodeTab, this);
    showMicrocode.detach();

    std::thread showAcm(&InfoWindow::showAcmTab, this);
    showAcm.detach();

    std::thread showBtg(&InfoWindow::showBtgTab, this);
    showBtg.detach();

    std::thread showAcpi(&InfoWindow::showAcpiTab, this);
    showAcpi.detach();

//    showFceTab();
}

void InfoWindow::showFitTab() {
    if (!BiosImage->isFitValid()) {
        return;
    }

    QTableWidgetItem    *item;
    FIRMWARE_INTERFACE_TABLE_ENTRY FitHeader = BiosImage->FitTable->FitHeader;
    UINT64 address = FitHeader.Address;
    QString FitSignature = QString::fromStdString(charToString((CHAR8*)&address, sizeof(UINT64), false));
    INT32 FitNum = (INT32)BiosImage->FitTable->FitEntryNum;

    QStringList headerLabels;
    headerLabels << "Address" << "Size" << "Version" << "C_V" << "Checksum" << "Type";

    ui->tableWidget->setColumnCount(headerLabels.size());
    ui->tableWidget->setHorizontalHeaderLabels(headerLabels);
    ui->tableWidget->setRowCount(FitNum);

    item = new QTableWidgetItem(FitSignature);
    ui->tableWidget->setItem(0, InfoWindow::Address, item);

    item = new QTableWidgetItem(QString("%1").arg(getSizeFromUINT24(FitHeader.Size), 6, 16, QLatin1Char('0')).toUpper() + "h");
    ui->tableWidget->setItem(0, InfoWindow::Size, item);

    item = new QTableWidgetItem(QString("%1").arg(FitHeader.Version, 4, 16, QLatin1Char('0')).toUpper() + "h");
    ui->tableWidget->setItem(0, InfoWindow::Version, item);

    item = new QTableWidgetItem(FitTableClass::getTypeName(FitHeader.Type));
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

        item = new QTableWidgetItem(QString("%1").arg(getSizeFromUINT24(Entry.Size), 6, 16, QLatin1Char('0')).toUpper() + "h");
        ui->tableWidget->setItem(index + 1, InfoWindow::Size, item);

        item = new QTableWidgetItem(QString("%1").arg(Entry.Version, 4, 16, QLatin1Char('0')).toUpper() + "h");
        ui->tableWidget->setItem(index + 1, InfoWindow::Version, item);

        item = new QTableWidgetItem(FitTableClass::getTypeName(Entry.Type));
        ui->tableWidget->setItem(index + 1, InfoWindow::Type, item);

        item = new QTableWidgetItem(QString::number(Entry.C_V, 16).toUpper() + "h");
        ui->tableWidget->setItem(index + 1, InfoWindow::C_V, item);

        item = new QTableWidgetItem(QString::number(Entry.Checksum, 16).toUpper() + "h");
        ui->tableWidget->setItem(index + 1, InfoWindow::Checksum, item);
    }
}

void InfoWindow::showMicrocodeTab() {
    if (!BiosImage->isFitValid()) {
        return;
    }
    for (auto MicrocodeEntry:BiosImage->FitTable->MicrocodeEntries) {
        QString ItemName = "Microcode";
        if (MicrocodeEntry->isEmpty)
            ItemName += " (Empty)";
        else
            ItemName = ItemName + "  " + QString::number(MicrocodeEntry->microcodeHeader.ProcessorSignature.Uint32, 16).toUpper();
        auto *MicrocodeItem = new QListWidgetItem(ItemName);
        ui->microcodeListWidget->addItem(MicrocodeItem);
    }

    if (ui->microcodeListWidget->model()->rowCount() != 0)
        ui->microcodeListWidget->setCurrentRow(0);
}

void InfoWindow::showAcmTab() {
    if (!BiosImage->isFitValid()) {
        return;
    }
    for (auto AcmEntry:BiosImage->FitTable->AcmEntries) {
        QString ItemName;
        if (!AcmEntry->isValid())
            ItemName = "Corrupted ACM";
        else if (AcmEntry->isProd())
            ItemName = "Prod ACM";
        else
            ItemName = "Non Prod ACM";
        auto *AcmItem = new QListWidgetItem(ItemName);
        ui->acmListWidget->addItem(AcmItem);
    }

    if (ui->acmListWidget->model()->rowCount() != 0)
        ui->acmListWidget->setCurrentRow(0);
}

void InfoWindow::showBtgTab() {
    QString ItemName;
    ItemName = "Key Manifest";
    auto *KmItem = new QListWidgetItem(ItemName);
    ui->BtgListWidget->addItem(KmItem);

    ItemName = "Boot Policy Manifest";
    auto *BpItem = new QListWidgetItem(ItemName);
    ui->BtgListWidget->addItem(BpItem);

    ItemName = "BPM DEF";
    auto *BpDefItem = new QListWidgetItem(ItemName);
    ui->BtgListWidget->addItem(BpDefItem);

    if (ui->BtgListWidget->model()->rowCount() != 0)
        ui->BtgListWidget->setCurrentRow(0);
}

void InfoWindow::showAcpiTab() {
    BiosImage->collectAcpiTable();
    for (AcpiClass* AcpiTable:BiosImage->AcpiTables) {
        QString AcpiItemName = AcpiTable->AcpiTableSignature + " - " + AcpiTable->AcpiTableOemID + " - " + AcpiTable->AcpiTableOemTableID;
        auto *AcpiItem = new QListWidgetItem(AcpiItemName);
        ui->AcpiListWidget->addItem(AcpiItem);
    }
    if (ui->AcpiListWidget->model()->rowCount() != 0)
        ui->AcpiListWidget->setCurrentRow(0);
}

void InfoWindow::showFceTab() {
//    QString text;
//    QString toolpath = appDir + "/tool/FCE/FCE.exe";
//    QFile ToolFile(toolpath);
//    if(!ToolFile.exists()) {
//        text = "FCE tool not found!";
//        ui->fceText->setText(text);
//        return;
//    }
//
//    auto *process = new QProcess(this);
//    QString lastPath = setting.value("LastFilePath").toString();
//    QString TempFilepath = QDir(lastPath).filePath("ClientBios.config");
//    QStringList arguments;
//    arguments << "read" << "-i" << OpenedFileName << "0006" <<  "005C" << "0078" << "0030" << "0034" << "0039" << "0046" << ">" << TempFilepath;
//    QString command = toolpath + " " + arguments.join(" ");
//    qDebug() << "Command:" << command;
//    process->start(toolpath, arguments);
//    process->waitForFinished();
//
//    text = process->readAllStandardOutput();
//    delete process;

//    QFile TempFile(TempFilepath);
//    if(TempFile.exists()) {
//        TempFile.open(QIODevice::ReadOnly | QIODevice::Text);
//        text = TempFile.readAll();
//        TempFile.close();
//        TempFile.remove();
//    }

//    ui->fceText->setText(text);
}

void InfoWindow::showEvent(QShowEvent *event) {
    resizeEvent(nullptr);
}

void InfoWindow::resizeEvent(QResizeEvent *event) {
    int length = ui->tableWidget->width() / 6;
    ui->tableWidget->setColumnWidth(InfoWindow::Address, length);
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
    ((BiosViewerWindow*)parentWidget)->setInfoWindowState(false);
    setting.setValue("InfoDialog/geometry", saveGeometry());
}

void InfoWindow::microcodeListWidgetItemSelectionChanged() {
    INT32 currentRow = ui->microcodeListWidget->currentRow();
    MicrocodeHeaderClass* EntryHeader = BiosImage->FitTable->MicrocodeEntries.at(currentRow);
    EntryHeader->setInfoStr();
    ui->MicrocodeTextBrowser->setText(EntryHeader->getInfoText());
}

void InfoWindow::acmListWidgetItemSelectionChanged() {
    INT32 currentRow = ui->acmListWidget->currentRow();
    AcmHeaderClass* EntryHeader = BiosImage->FitTable->AcmEntries.at(currentRow);
    EntryHeader->setInfoStr();
    ui->AcmTextBrowser->setText(EntryHeader->getInfoText());
}

void InfoWindow::BtgListWidgetItemSelectionChanged()
{
    QModelIndex index = ui->BtgListWidget->currentIndex();
    if (!index.isValid())
        return;
    QListWidgetItem *item = ui->BtgListWidget->currentItem();

    QString text;
    QString toolpath = appDir + "/tool/BpmGen2/BpmGen2.exe";
    QString BpmHeaderStr("######################\r\n"
                         "# BootPolicyManifest #\r\n"
                         "######################");
    QString KmHeaderStr("################\r\n"
                        "# Key Manifest #\r\n"
                        "################");

    auto *process = new QProcess(this);
    process->start(toolpath, QStringList() << "INFO" << OpenedFileName);
    process->waitForFinished();
    QString BpmGen2Text = process->readAllStandardOutput();
    delete process;

    INT64 FirstLineLength = BpmGen2Text.indexOf("\r\n");
    INT64 BpmIndex = BpmGen2Text.indexOf(BpmHeaderStr);
    INT64 KmIndex = BpmGen2Text.indexOf(KmHeaderStr);

    QString BpmToolVersion = BpmGen2Text.mid(0, FirstLineLength) + "\n\n";

    if (item->text() == "Key Manifest") {
        text = BpmToolVersion + BpmGen2Text.mid(KmIndex);
    } else if (item->text() == "Boot Policy Manifest") {
        text = BpmToolVersion + BpmGen2Text.mid(BpmIndex, KmIndex - BpmIndex);
    } else if (item->text() == "BPM DEF") {
        process = new QProcess(this);
        QString lastPath = setting.value("LastFilePath").toString();
        QString TempFilepath = QDir(lastPath).filePath("temp.txt");
        process->start(toolpath, QStringList() << "PARSE" << OpenedFileName << "-o" << TempFilepath);
        process->waitForFinished();
        delete process;
        QFile TempFile(TempFilepath);
        if (TempFile.exists()) {
            TempFile.open(QIODevice::ReadOnly | QIODevice::Text);
            text = TempFile.readAll();
            TempFile.close();
            TempFile.remove();
        }
    }

    ui->BtgTextBrowser->setText(text);
}

void InfoWindow::AcpiListWidgetItemSelectionChanged() {
    INT32 currentRow = ui->AcpiListWidget->currentRow();
    AcpiClass* ACPI_Entry = BiosImage->AcpiTables.at(currentRow);

    QString lastPath = setting.value("LastFilePath").toString();
    QString filePath = QDir(lastPath).filePath("temp.bin");
    QString Dslpath = QDir(lastPath).filePath("temp.dsl");
    QString toolpath = appDir + "/tool/ACPI/iasl.exe";
    QString AcpiText;

    QFile ToolFile(toolpath);
    if(!ToolFile.exists()) {
        AcpiText = "ACPI tool not found!";
        ui->AcpiTextBrowser->setText(AcpiText);
        return;
    }

    saveBinary(filePath.toStdString(), ACPI_Entry->getData(), 0, ACPI_Entry->getSize());
    auto *process = new QProcess(this);
    process->start(toolpath, QStringList() << "-d" << filePath);
    process->waitForFinished();

    QFile DslFile(Dslpath);
    if(DslFile.exists()) {
        DslFile.open(QIODevice::ReadOnly | QIODevice::Text);
        AcpiText = DslFile.readAll();
        DslFile.close();
        DslFile.remove();
    }
    QFile TempFile(filePath);
    if(TempFile.exists()) {
        TempFile.close();
        TempFile.remove();
    }
    ui->AcpiTextBrowser->setText(AcpiText);
}
