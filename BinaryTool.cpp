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
                                                    tr("Extract Capsule Component"),
                                                    outputPath,
                                                    tr("Capsule files(*.rom *.bin *.cap);;All files (*.*)"));
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
