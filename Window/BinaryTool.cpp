#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QDialogButtonBox>
#include "mainwindow.h"
#include "SearchDialog.h"
#include "inputdialog.h"
#include "./ui_mainwindow.h"

void MainWindow::ActionSeperateBinaryTriggered()
{
    if (InputImage == nullptr) {
        QMessageBox::critical(this, tr("Seperate Binary"), "No Binary Opened!");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Seperate Binary");
    QFormLayout form(&dialog);
    // Offset
    QString Offset = QString("Offset: ");
    HexSpinBox *OffsetSpinbox = new HexSpinBox(&dialog);
    OffsetSpinbox->setFocus();
    OffsetSpinbox->selectAll();
    form.addRow(Offset, OffsetSpinbox);
    // Add Cancel and OK button
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Process when OK button is clicked
    if (dialog.exec() == QDialog::Accepted) {
        INT64 InputOffset = OffsetSpinbox->value();
        if (InputOffset < 0 ||InputOffset >= InputImageSize) {
            QMessageBox::critical(this, tr("Extract Binary"), "Invalid offset!");
            return;
        }

        QFileInfo fileinfo {OpenedFileName};
        QString dirName = QFileDialog::getExistingDirectory(this,
                                                            tr("Open directory"),
                                                            fileinfo.dir().path(),
                                                            QFileDialog::ShowDirsOnly);
        QString upperFilePath = dirName + "/" + fileinfo.baseName() + "_upper.bin";
        QString lowerFilePath = dirName + "/" + fileinfo.baseName() + "_lower.bin";

        Buffer::saveBinary(upperFilePath.toStdString(), InputImage, 0, InputOffset);
        Buffer::saveBinary(lowerFilePath.toStdString(), InputImage, InputOffset, InputImageSize - InputOffset);
    }
}

void MainWindow::ActionExtractBIOSTriggered()
{
    if (InputImage == nullptr) {
        QMessageBox::critical(this, tr("Extract BIOS Tool"), "No Binary Opened!");
        return;
    }
    if (InputImageSize != 0x2000000) {
        QMessageBox::critical(this, tr("About BIOS Viewer"), "This is not an IFWI Binary!");
        return;
    }
    IfwiVolume *FlashDescriptor = IFWI_Sections.at(0);
    if (FlashDescriptor->RegionType != FLASH_REGION_TYPE::FlashRegionDescriptor) {
        QMessageBox::critical(this, tr("About BIOS Viewer"), "Invalid Flash Descriptor!");
        return;
    }
    FlashRegionBaseArea BiosRegion = ((FlashDescriptorClass*)FlashDescriptor)->RegionList.at(FLASH_REGION_TYPE::FlashRegionBios);
    INT64 BIOS_Offset = BiosRegion.getBase();
    INT64 BIOS_Size = BiosRegion.getSize();

    QFileInfo fileinfo {OpenedFileName};
    QString outputPath = setting.value("LastFilePath").toString() + "/" + fileinfo.baseName() + "_BIOS.bin";
    QString BiosName = QFileDialog::getSaveFileName(this,
                                                    tr("Extract BIOS"),
                                                    outputPath,
                                                    tr("BIOS image(*.rom *.bin *.cap);;All files (*.*)"));
    if (BiosName.isEmpty()) {
        return;
    }

    Buffer::saveBinary(BiosName.toStdString(), InputImage, BIOS_Offset, BIOS_Size);
}

void MainWindow::ActionSearchTriggered()
{
    if (!searchDialogOpened) {
        searchDialogOpened = true;
        searchDialog = new SearchDialog();
        searchDialog->setSearchMode(false);
        searchDialog->SetModelData(&IFWI_ModelData);
        searchDialog->setParentWidget(this);
        if (isDarkMode())
            searchDialog->setWindowIcon(QIcon(":/search_light.svg"));
        searchDialog->show();
    }
}

void MainWindow::RecursiveSearchOffset(DataModel* model, INT64 offset, vector<INT32> &SearchRows) {
    for (INT64 row = 0; row < (INT64)model->volumeModelData.size(); ++row) {
        DataModel* childModel = model->volumeModelData.at(row);
        if (!childModel->modelData->isCompressed && offset >= childModel->modelData->offsetFromBegin && offset < childModel->modelData->offsetFromBegin + childModel->modelData->size) {
            SearchRows.push_back(row);
            RecursiveSearchOffset(childModel, offset, SearchRows);
        }
    }
}

void MainWindow::ActionGotoTriggered()
{
    if ( BiosImage == nullptr )
      return;

    QDialog dialog(this);
    dialog.setWindowTitle("Goto ...");
    QFormLayout form(&dialog);
    // Offset
    QString Offset = QString("Offset: ");
    HexSpinBox *OffsetSpinbox = new HexSpinBox(&dialog);
    OffsetSpinbox->setFocus();
    OffsetSpinbox->selectAll();
    form.addRow(Offset, OffsetSpinbox);
    // Add Cancel and OK button
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Process when OK button is clicked
    if (dialog.exec() == QDialog::Accepted) {
        INT64 SearchOffset = OffsetSpinbox->value();
      if (SearchOffset < 0 || SearchOffset >= InputImageSize) {
            QMessageBox::critical(this, tr("Goto ..."), "Invalid offset!");
            return;
        }

        vector<INT32> SearchRows;
        for (UINT64 row = 0; row < IFWI_ModelData.size(); ++row) {
            DataModel *model = IFWI_ModelData.at(row);
            if (SearchOffset >= model->modelData->offsetFromBegin && SearchOffset < model->modelData->offsetFromBegin + model->modelData->size) {
                SearchRows.push_back(row + 1);
                RecursiveSearchOffset(model, SearchOffset, SearchRows);
            }
        }
        HighlightTreeItem(SearchRows);
    }
}

void MainWindow::ActionCollapseTriggered()
{
    ui->treeWidget->collapseAll();
}

void MainWindow::ActionReplaceBIOSTriggered()
{
    const INT64 IFWI_SIZE = 0x2000000;
//    const INT64 BLOCK_SIZE = 0x1000;
    if (InputImage == nullptr) {
        QMessageBox::critical(this, tr("Extract BIOS Tool"), "No Binary Opened!");
        return;
    }
    if (InputImageSize != IFWI_SIZE) {
        QMessageBox::critical(this, tr("About BIOS Viewer"), "This is not an IFWI Binary!");
        return;
    }
    QString lastPath = setting.value("LastFilePath").toString();
    QString BiosName = QFileDialog::getOpenFileName(this,
                                                    tr("Replace BIOS image"),
                                                    lastPath,
                                                    tr("BIOS image(*.rom *.bin *.fd);;All files (*.*)"));
    if (BiosName.isEmpty()) {
        return;
    }

    Buffer *NewBiosBuffer = new BaseLibrarySpace::Buffer(new std::ifstream(BiosName.toStdString(), std::ios::in | std::ios::binary));
    INT64 NewBiosSize = NewBiosBuffer->getBufferSize();
    if (NewBiosSize != BiosImage->size) {
        QMessageBox::critical(this, tr("About BIOS Viewer"), "Do not support replacing BIOS of different size!");
        delete NewBiosBuffer;
        return;
    }
    UINT8* NewBios = NewBiosBuffer->getBytes(NewBiosSize);
    delete NewBiosBuffer;

    IfwiVolume *FlashDescriptor = IFWI_Sections.at(0);
    if (FlashDescriptor->RegionType != FLASH_REGION_TYPE::FlashRegionDescriptor) {
        QMessageBox::critical(this, tr("About BIOS Viewer"), "Invalid Flash Descriptor!");
        delete[] NewBios;
        return;
    }

    vector<FlashRegionBaseArea> &RegionList = ((FlashDescriptorClass*)FlashDescriptor)->RegionList;
    vector<FlashRegionBaseArea> ValidRegionList;
    for (FlashRegionBaseArea &region:RegionList) {
        if (region.limit == 0) {
            continue;
        }
        ValidRegionList.push_back(region);
    }
    sort(ValidRegionList.begin(), ValidRegionList.end(), [](FlashRegionBaseArea &g1, FlashRegionBaseArea &g2) { return g1.limit < g2.limit; });

    FlashRegionBaseArea PaddingRegion = ValidRegionList.at(ValidRegionList.size() - 2);
    FlashRegionBaseArea BiosRegion = ValidRegionList.at(ValidRegionList.size() - 1);

    if (NewBiosSize > PaddingRegion.getSize() + BiosRegion.getSize()) {
        QMessageBox::critical(this, tr("Extract BIOS Tool"), "Binary size is too large!");
        delete[] NewBios;
        return;
    }

    PaddingRegion.setLimit(IFWI_SIZE - NewBiosSize);
    BiosRegion.setBase(IFWI_SIZE - NewBiosSize);

    UINT8* NewIFWI = new UINT8[IFWI_SIZE];
    for (UINT32 IfwiIdx = 0; IfwiIdx < PaddingRegion.getBase(); ++IfwiIdx) {
        NewIFWI[IfwiIdx] = InputImage[IfwiIdx];
    }
    for (UINT32 PaddingIdx = 0; PaddingIdx < PaddingRegion.getSize(); ++PaddingIdx) {
        NewIFWI[PaddingRegion.getBase() + PaddingIdx] = 0xFF;
    }
    for (UINT32 NewBiosIdx = 0; NewBiosIdx < NewBiosSize; ++NewBiosIdx) {
        NewIFWI[PaddingRegion.getLimit() + NewBiosIdx] = NewBios[NewBiosIdx];
    }

    // Fix flash descriptor (Not finished, hash etc.)
//    FlashRegionBaseArea *FRL = (FlashRegionBaseArea*)(NewIFWI + 0x40);
//    ((FlashRegionBaseArea*)(FRL + FLASH_REGION_TYPE::FlashRegionBios))->setBase(IFWI_SIZE - NewBiosSize);
//    ((FlashRegionBaseArea*)(FRL + FLASH_REGION_TYPE::FlashRegionDeviceExpansion2))->setLimit(IFWI_SIZE - NewBiosSize);
//    FlashRegionBaseArea *FRLB = (FlashRegionBaseArea*)(NewIFWI + BLOCK_SIZE + 0x40);
//    ((FlashRegionBaseArea*)(FRLB + FLASH_REGION_TYPE::FlashRegionBios))->setBase(IFWI_SIZE - NewBiosSize);
//    ((FlashRegionBaseArea*)(FRLB + FLASH_REGION_TYPE::FlashRegionDeviceExpansion2))->setLimit(IFWI_SIZE - NewBiosSize);

    QFileInfo fileinfo {OpenedFileName};
    QString outputPath = setting.value("LastFilePath").toString() + "/" + fileinfo.baseName() + "_IntegratedBIOS.bin";
    Buffer::saveBinary(outputPath.toStdString(), NewIFWI, 0, IFWI_SIZE);

    delete[] NewBios;
    delete[] NewIFWI;
}

void MainWindow::SearchButtonClicked()
{
    ActionSearchTriggered();
}

void MainWindow::ActionExtractBinaryTriggered()
{
    if (InputImage == nullptr) {
        QMessageBox::critical(this, tr("Extract Binary"), "No Binary Opened!");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Extract Binary");
    QFormLayout form(&dialog);
    // Offset
    QString Offset = QString("Offset: ");
    HexSpinBox *OffsetSpinbox = new HexSpinBox(&dialog);
    OffsetSpinbox->setFocus();
    OffsetSpinbox->selectAll();
    form.addRow(Offset, OffsetSpinbox);
    // Length
    QString Length = QString("Length: ");
    HexSpinBox *LengthSpinbox = new HexSpinBox(&dialog);
    form.addRow(Length, LengthSpinbox);
    // Add Cancel and OK button
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Process when OK button is clicked
    if (dialog.exec() == QDialog::Accepted) {
        INT64 InputOffset = OffsetSpinbox->value();
        INT64 InputLength = LengthSpinbox->value();
        if (InputOffset < 0 || InputLength <= 0 || InputOffset >= InputImageSize || InputLength + InputLength > InputImageSize) {
            QMessageBox::critical(this, tr("Extract Binary"), "Invalid offset and size!");
            return;
        }

        QFileInfo fileinfo {OpenedFileName};
        QString outputPath = fileinfo.dir().path() + "/" + fileinfo.baseName() + "_Extract.bin";
        QString BinaryName = QFileDialog::getSaveFileName(this,
                                                        tr("Extract Binary"),
                                                        outputPath,
                                                        tr("BIOS image(*.rom *.bin *.cap);;All files (*.*)"));

        Buffer::saveBinary(BinaryName.toStdString(), InputImage, InputOffset, InputLength);
    }
}
