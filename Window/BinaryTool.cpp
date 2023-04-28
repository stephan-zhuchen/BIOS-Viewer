#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include "mainwindow.h"
#include "SearchDialog.h"
#include "./ui_mainwindow.h"

void MainWindow::ActionSeperateBinaryTriggered()
{
    if ( BiosImage == nullptr )
      return;

    bool done;
    QString offset = QInputDialog::getText ( this, tr ( "Goto..." ),
                     tr ( "Offset (0x for hexadecimal):" ), QLineEdit::Normal,
                     nullptr, &done );

    if (done) {
        INT32 SeperateOffset = offset.toInt(nullptr, 16);

        if (SeperateOffset <= 0 || SeperateOffset >= BiosImage->size) {
            QMessageBox::critical(this, tr("Seperate Binary Tool"), "Invalid Offset!");
            return;
        }

        QFileInfo fileinfo {OpenedFileName};
        QString outputPath = setting.value("LastFilePath").toString();
        QString dirName = QFileDialog::getExistingDirectory(this,
                                                          tr("Open directory"),
                                                          outputPath,
                                                          QFileDialog::ShowDirsOnly);
        QString upperFilePath = dirName + "/" + fileinfo.baseName() + "_upper.bin";
        QString lowerFilePath = dirName + "/" + fileinfo.baseName() + "_lower.bin";
        Buffer::saveBinary(upperFilePath.toStdString(), BiosImage->data, 0, SeperateOffset);
        Buffer::saveBinary(lowerFilePath.toStdString(), BiosImage->data, SeperateOffset, BiosImage->size - SeperateOffset);
    }
}

void MainWindow::ActionExtractBIOSTriggered()
{
    if (BiosImage == nullptr) {
        QMessageBox::critical(this, tr("Extract BIOS Tool"), "No Binary Opened!");
        return;
    }
    INT64 BinarySize = BiosImage->size;
    if (BinarySize != 0x2000000) {
        QMessageBox::critical(this, tr("About BIOS Viewer"), "This is not an IFWI Binary!");
        return;
    }

    QFileInfo fileinfo {OpenedFileName};
    QString outputPath = setting.value("LastFilePath").toString() + "/" + fileinfo.baseName() + "_BIOS.bin";
    QString BiosName = QFileDialog::getSaveFileName(this,
                                                    tr("Extract BIOS"),
                                                    outputPath,
                                                    tr("BIOS image(*.rom *.bin *.cap);;All files (*.*)"));
    if (BiosName.isEmpty()) {
        return;
    }
    Buffer::saveBinary(BiosName.toStdString(), BiosImage->data, 0x1000000, 0x1000000);
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

    bool done;
    QString offset = QInputDialog::getText ( this, tr ( "Goto..." ),
                     tr ( "Offset (0x for hexadecimal):" ), QLineEdit::Normal,
                     nullptr, &done );

    INT64 SearchOffset = 0;
    if ( done && offset[0] == '0' && offset[1] == 'x' ) {
        SearchOffset = offset.toInt ( nullptr, 16 );
    } else {
        SearchOffset = offset.toInt ( nullptr, 10 );
    }

    if (SearchOffset < 0 || SearchOffset >= InputImageModel->modelData->size) {
        QMessageBox::critical(this, tr("BIOS Viewer"), tr("Invalid Address!"));
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

void MainWindow::ActionCollapseTriggered()
{
    ui->treeWidget->collapseAll();
}

void MainWindow::ActionReplaceBIOSTriggered()
{
    if (InputImage == nullptr) {
        QMessageBox::critical(this, tr("Extract BIOS Tool"), "No Binary Opened!");
        return;
    }
    if (InputImageSize != 0x2000000) {
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
    INT64 PaddingSize = 0x1000000 - NewBiosSize;
    UINT8* NewBios = NewBiosBuffer->getBytes(NewBiosSize);
    if (NewBiosSize > 0x1000000) {
        QMessageBox::critical(this, tr("Extract BIOS Tool"), "Do not support BIOS larger than 16MB!");
        return;
    }
    UINT8* NewIFWI = new UINT8[0x2000000];
    for (int IfwiIdx = 0; IfwiIdx < 0x1000000; ++IfwiIdx) {
        NewIFWI[IfwiIdx] = BiosImage->data[IfwiIdx];
    }
    for (int PaddingIdx = 0; PaddingIdx < PaddingSize; ++PaddingIdx) {
        NewIFWI[0x1000000 + PaddingIdx] = 0xFF;
    }
    for (int NewBiosIdx = 0; NewBiosIdx < NewBiosSize; ++NewBiosIdx) {
        NewIFWI[0x1000000 + PaddingSize + NewBiosIdx] = NewBios[NewBiosIdx];
    }

    QFileInfo fileinfo {OpenedFileName};
    QString outputPath = setting.value("LastFilePath").toString() + "/" + fileinfo.baseName() + "_IntegratedBIOS.bin";
    Buffer::saveBinary(outputPath.toStdString(), NewIFWI, 0, 0x2000000);

    delete NewBiosBuffer;
    delete[] NewBios;
    delete[] NewIFWI;
}

void MainWindow::SearchButtonClicked()
{
    ActionSearchTriggered();
}
