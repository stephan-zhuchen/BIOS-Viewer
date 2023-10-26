#include <QMessageBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include "BaseLib.h"
#include "Start/StartWindow.h"
#include "BiosWindow.h"
#include "HexView/HexWindow.h"
#include "Input/inputdialog.h"
#include "IfwiRegion/FlashDescriptorRegion.h"
#include "ui_BiosWindow.h"

using namespace BaseLibrarySpace;

void StartWindow::ActionSeparateBinaryTriggered()
{
    if (TabData.empty())
        return;
    GeneralData *WindowData = TabData.at(MainTabWidget->currentIndex());
    if (WindowData->InputImage == nullptr) {
        QMessageBox::critical(this, tr("Seperate Binary"), "No Binary Opened!");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Seperate Binary");
    QFormLayout form(&dialog);
    // Offset
    QString Offset = QString("Offset: ");
    auto *OffsetSpinbox = new HexSpinBox(&dialog);
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
        if (InputOffset < 0 ||InputOffset >= WindowData->InputImageSize) {
            QMessageBox::critical(this, tr("Extract Binary"), "Invalid offset!");
            return;
        }

        QFileInfo fileInfo {WindowData->OpenedFileName};
        QString dirName = QFileDialog::getExistingDirectory(this,
                                                            tr("Open directory"),
                                                            fileInfo.dir().path(),
                                                            QFileDialog::ShowDirsOnly);
        QString upperFilePath = dirName + "/" + fileInfo.baseName() + "_upper.bin";
        QString lowerFilePath = dirName + "/" + fileInfo.baseName() + "_lower.bin";

        saveBinary(upperFilePath.toStdString(), WindowData->InputImage, 0, InputOffset);
        saveBinary(lowerFilePath.toStdString(), WindowData->InputImage, InputOffset, WindowData->InputImageSize - InputOffset);
    }
}

void BiosViewerWindow::ActionExtractBIOSTriggered() {
    if (WindowData->InputImage == nullptr) {
        QMessageBox::critical(this, tr("Extract BIOS Tool"), "No Binary Opened!");
        return;
    }
    if (WindowData->InputImageSize != 0x2000000) {
        QMessageBox::critical(this, tr("About BIOS Viewer"), "This is not an IFWI Binary!");
        return;
    }
    Volume *FlashDescriptor =  BiosData->VolumeDataList.at(0);
    if (FlashDescriptor->getVolumeType() != VolumeType::FlashDescriptor) {
        QMessageBox::critical(this, tr("About BIOS Viewer"), "Invalid Flash Descriptor!");
        return;
    }
    FlashRegionBaseArea BiosRegion = ((FlashDescriptorRegion*)FlashDescriptor)->RegionList.at(FLASH_REGION_TYPE::FlashRegionBios);
    INT64 BIOS_Offset = BiosRegion.getBase();
    INT64 BIOS_Size = BiosRegion.getSize();

    QFileInfo fileinfo {WindowData->OpenedFileName};
    QString outputPath = setting.value("LastFilePath").toString() + "/" + fileinfo.baseName() + "_BIOS.bin";
    QString BiosName = QFileDialog::getSaveFileName(this,
                                                    tr("Extract BIOS"),
                                                    outputPath,
                                                    tr("BIOS image(*.rom *.bin *.cap);;All files (*.*)"));
    if (BiosName.isEmpty()) {
        return;
    }

    saveBinary(BiosName.toStdString(), WindowData->InputImage, BIOS_Offset, BIOS_Size);
}

void StartWindow::ActionSearchTriggered() const {
    if (TabData.empty())
        return;
    GeneralData *WindowData = TabData.at(MainTabWidget->currentIndex());
    if (WindowData->CurrentWindow == WindowMode::BIOS) {
        WindowData->BiosViewerUi->ActionSearchBiosTriggered();
    } else if (WindowData->CurrentWindow == WindowMode::Hex) {
        WindowData->HexViewerUi->ActionSearchHexTriggered();
    }
}

void StartWindow::ActionGotoTriggered() const {
    if (TabData.empty())
        return;
    GeneralData *WindowData = TabData.at(MainTabWidget->currentIndex());
    if (WindowData->CurrentWindow == WindowMode::BIOS) {
        WindowData->BiosViewerUi->ActionGotoTriggered();
    } else if (WindowData->CurrentWindow == WindowMode::Hex) {
        WindowData->HexViewerUi->ActionGotoTriggered();
    }
}

void StartWindow::ActionCollapseTriggered() const {
    if (TabData.empty())
        return;
    GeneralData *WindowData = TabData.at(MainTabWidget->currentIndex());
    if (WindowData->CurrentWindow == WindowMode::BIOS) {
        WindowData->BiosViewerUi->ui->treeWidget->collapseAll();
    }
}

void BiosViewerWindow::ActionReplaceBIOSTriggered() {
    const INT64 IFWI_SIZE = 0x2000000;
//    const INT64 BLOCK_SIZE = 0x1000;
    if (WindowData->InputImage == nullptr) {
        QMessageBox::critical(this, tr("Extract BIOS Tool"), "No Binary Opened!");
        return;
    }
    if (WindowData->InputImageSize != IFWI_SIZE) {
        QMessageBox::critical(this, tr("About BIOS Viewer"), "This is not an IFWI Binary!");
        return;
    }
    Volume *FlashDescriptor =  BiosData->VolumeDataList.at(0);
    if (FlashDescriptor->getVolumeType() != VolumeType::FlashDescriptor) {
        QMessageBox::critical(this, tr("About BIOS Viewer"), "Invalid Flash Descriptor!");
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

    QFile file(BiosName);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    QByteArray byteArray = file.readAll();
    file.close();

    QFileInfo fileInfo(BiosName);
    setting.setValue("LastFilePath", fileInfo.path());

    auto NewBios = new UINT8[byteArray.size()];
    INT64 NewBiosSize = byteArray.size();
    memcpy(NewBios, byteArray.data(), NewBiosSize);

    QVector<FlashRegionBaseArea> &RegionList = ((FlashDescriptorRegion*)FlashDescriptor)->RegionList;
    QVector<FlashRegionBaseArea> ValidRegionList;
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
        safeArrayDelete(NewBios);
        return;
    }

    PaddingRegion.setLimit(IFWI_SIZE - NewBiosSize);
    BiosRegion.setBase(IFWI_SIZE - NewBiosSize);

    UINT8* NewIFWI = new UINT8[IFWI_SIZE];
    for (UINT32 IfwiIdx = 0; IfwiIdx < PaddingRegion.getBase(); ++IfwiIdx) {
        NewIFWI[IfwiIdx] = WindowData->InputImage[IfwiIdx];
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

    QFileInfo fileinfo {WindowData->OpenedFileName};
    QString outputPath = setting.value("LastFilePath").toString() + "/" + fileinfo.baseName() + "_IntegratedBIOS.bin";
    saveBinary(outputPath.toStdString(), NewIFWI, 0, IFWI_SIZE);

    safeArrayDelete(NewBios);
    safeArrayDelete(NewIFWI);
}

void BiosViewerWindow::SearchButtonClicked() {
    ActionSearchBiosTriggered();
}

void StartWindow::ActionExtractBinaryTriggered() {
    if (TabData.empty())
        return;
    GeneralData *WindowData = TabData.at(MainTabWidget->currentIndex());
    if (WindowData->InputImage == nullptr) {
        QMessageBox::critical(this, tr("Extract Binary"), "No Binary Opened!");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Extract Binary");
    QFormLayout form(&dialog);
    // Offset
    QString Offset = QString("Offset: ");
    auto *OffsetSpinbox = new HexSpinBox(&dialog);
    OffsetSpinbox->setFocus();
    OffsetSpinbox->selectAll();
    form.addRow(Offset, OffsetSpinbox);
    // Length
    QString Length = QString("Length: ");
    auto *LengthSpinbox = new HexSpinBox(&dialog);
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
        if (InputOffset < 0 || InputLength <= 0 || InputOffset >= WindowData->InputImageSize || InputLength + InputLength > WindowData->InputImageSize) {
            QMessageBox::critical(this, tr("Extract Binary"), "Invalid offset and size!");
            return;
        }

        QFileInfo fileInfo {WindowData->OpenedFileName};
        QString outputPath = fileInfo.dir().path() + "/" + fileInfo.baseName() + "_Extract.bin";
        QString BinaryName = QFileDialog::getSaveFileName(this,
                                                        tr("Extract Binary"),
                                                        outputPath,
                                                        tr("BIOS image(*.rom *.bin *.cap);;All files (*.*)"));

        saveBinary(BinaryName.toStdString(), WindowData->InputImage, InputOffset, InputLength);
    }
}
