#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include "mainwindow.h"
#include "SearchDialog.h"
#include "./ui_mainwindow.h"

void MainWindow::on_actionSeperate_Binary_triggered()
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

void MainWindow::on_actionExtract_BIOS_triggered()
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

void MainWindow::on_actionSearch_triggered()
{
    SearchDialog *settingDialog = new SearchDialog;
    settingDialog->setSearchMode(false);
    settingDialog->SetModelData(&FvModelData);
    settingDialog->setParentWidget(this);
    settingDialog->show();
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

void MainWindow::on_actionGoto_triggered()
{
    if ( BiosImage == nullptr )
      return;

    bool done;
    QString offset = QInputDialog::getText ( this, tr ( "Goto..." ),
                     tr ( "Offset (0x for hexadecimal):" ), QLineEdit::Normal,
                     nullptr, &done );

    if ( done && offset[0] == '0' && offset[1] == 'x' ) {
        INT64 SearchOffset = offset.toInt ( nullptr, 16 );
        vector<INT32> SearchRows;
        for (INT64 row = 0; (INT64)row < FvModelData.size(); ++row) {
            DataModel *model = FvModelData.at(row);
            if (SearchOffset >= model->modelData->offsetFromBegin && SearchOffset < model->modelData->offsetFromBegin + model->modelData->size) {
                SearchRows.push_back(row + 1);
                RecursiveSearchOffset(model, SearchOffset, SearchRows);
            }
        }
        for (auto row:SearchRows) {
            cout << row << " ";
        }
        cout << endl;
        HighlightTreeItem(SearchRows);
    }
}

void MainWindow::on_actionCollapse_triggered()
{
    ui->treeWidget->collapseAll();
}

void MainWindow::on_actionReplace_BIOS_triggered()
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

void MainWindow::on_searchButton_clicked()
{
    on_actionSearch_triggered();
}
