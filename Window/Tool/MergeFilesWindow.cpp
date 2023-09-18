#include <QSettings>
#include <QMenu>
#include <QEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QFileDialog>
#include <QListWidgetItem>
#include "BaseLib.h"
#include "MergeFilesWindow.h"
#include "ui_MergeFilesWindow.h"

MergeFilesWindow::MergeFilesWindow(bool darkMode, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MergeFilesWindow),
    darkModeFlag(darkMode)
{
    ui->setupUi(this);
    InitCustomMenu();
    setAttribute(Qt::WA_DeleteOnClose);
    ui->FileListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    restoreGeometry(setting.value("MergeFilesWindow/geometry").toByteArray());

    if (darkModeFlag) {
        ui->actionAddFile->setIcon(QIcon(":/open_light.svg"));
        ui->actionSave->setIcon(QIcon(":/save_light.svg"));
    } else {
        ui->actionAddFile->setIcon(QIcon(":/open.svg"));
        ui->actionSave->setIcon(QIcon(":/save.svg"));
    }

    ui->FileListWidget->setFont(QFont(setting.value("BiosViewerFont").toString(), setting.value("BiosViewerFontSize").toInt()));
    ui->FileListWidget->setStyleSheet(QString("QListView::item{margin:%1px;}").arg(setting.value("LineSpacing").toInt()));

    connect(ui->actionAddFile,  SIGNAL(triggered()), this, SLOT(actionAddFileTriggered()));
    connect(ui->actionSave,     SIGNAL(triggered()), this, SLOT(actionSaveTriggered()));
    connect(ui->FileListWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showListCustomMenu(QPoint)));

}

MergeFilesWindow::~MergeFilesWindow() {
    delete ui;
    CleanupCustomMenu();
    setting.setValue("MergeFilesWindow/geometry", saveGeometry());
}

void MergeFilesWindow::OpenFile(const QString& path) {
    QFile     file(path);
    QFileInfo fileInfo(path);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        OpenedFilesList.append(data);
        file.close();
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(fileInfo.fileName());
        ui->FileListWidget->addItem(item);
    }
}

void MergeFilesWindow::closeEvent(QCloseEvent *event) {

}

void MergeFilesWindow::dragEnterEvent(QDragEnterEvent *event) {
    if(event->mimeData()->hasUrls())
        event->acceptProposedAction();
    else
        event->ignore();
}

void MergeFilesWindow::dropEvent(QDropEvent *event) {
    const QList<QUrl> urlList = event->mimeData()->urls();
    foreach (const QUrl &url, urlList) {
        QFileInfo file(url.toLocalFile());
        if(file.isFile()) {
            OpenFile(file.filePath());
        }
    }
}

void MergeFilesWindow::InitCustomMenu() {
    CustomMenu = new QMenu;

    MoveUp = new QAction("Move Up");
    connect(MoveUp, SIGNAL(triggered(bool)), this, SLOT(MoveFileUp()));

    MoveDown = new QAction("Move Down");
    connect(MoveDown, SIGNAL(triggered(bool)), this, SLOT(MoveFileDown()));

    DeleteItem = new QAction("Delete");
    connect(DeleteItem, SIGNAL(triggered(bool)), this, SLOT(DeleteFile()));
}

void MergeFilesWindow::CleanupCustomMenu() {
    using BaseLibrarySpace::safeDelete;
    safeDelete(CustomMenu);
    safeDelete(MoveUp);
    safeDelete(MoveDown);
    safeDelete(DeleteItem);
}

void MergeFilesWindow::showListCustomMenu(const QPoint &pos) {
    QIcon up, down, deleteFile;
    if (darkModeFlag) {
        up = QIcon(":/arrowup_light.svg");
        down = QIcon(":/arrowdown_light.svg");
        deleteFile = QIcon(":/delete_light.svg");
    } else {
        up = QIcon(":/arrowup.svg");
        down = QIcon(":/arrowdown.svg");
        deleteFile = QIcon(":/delete.svg");
    }

    QModelIndex index = ui->FileListWidget->indexAt(pos);
    if (!index.isValid())
        return;

    CustomMenu->clear();
    MoveUp->setIcon(up);
    MoveDown->setIcon(down);
    DeleteItem->setIcon(deleteFile);

    INT32 currentRow = ui->FileListWidget->currentRow();
    if (currentRow > 0) {
        CustomMenu->addAction(MoveUp);
    }
    if (currentRow >= 0 && currentRow != ui->FileListWidget->count() - 1) {
        CustomMenu->addAction(MoveDown);
    }

    CustomMenu->addAction(DeleteItem);

    CustomMenu->move(ui->FileListWidget->cursor().pos());
    CustomMenu->show();
}

void MergeFilesWindow::MoveFileUp() {
    INT32 currentRow = ui->FileListWidget->currentRow();
    if (currentRow > 0) {
        OpenedFilesList.swapItemsAt(currentRow, currentRow - 1);
        QListWidgetItem *item = ui->FileListWidget->takeItem(currentRow - 1);
        ui->FileListWidget->insertItem(currentRow, item);
    }
}

void MergeFilesWindow::MoveFileDown() {
    INT32 currentRow = ui->FileListWidget->currentRow();
    INT32 RowCount = ui->FileListWidget->count();
    if (currentRow >= 0 && currentRow < RowCount - 1) {
        OpenedFilesList.swapItemsAt(currentRow, currentRow + 1);
        QListWidgetItem *item = ui->FileListWidget->takeItem(currentRow + 1);
        ui->FileListWidget->insertItem(currentRow, item);
    }
}

void MergeFilesWindow::DeleteFile() {
    INT32 currentRow = ui->FileListWidget->currentRow();
    QListWidgetItem* currentItem = ui->FileListWidget->currentItem();
    if (currentItem) {
        OpenedFilesList.removeAt(currentRow);
        QListWidgetItem* itemToRemove = ui->FileListWidget->takeItem(currentRow);
        delete itemToRemove;
    }
}

void MergeFilesWindow::actionAddFileTriggered() {
    QString lastPath = setting.value("LastFilePath").toString();
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image File"),
                                                    lastPath,
                                                    tr("All files (*.*);;Image files(*.rom *.bin *.fd *.fv)"));
    if (fileName.isEmpty()){
        return;
    }
    OpenFile(fileName);
}


void MergeFilesWindow::actionSaveTriggered() {
    if (OpenedFilesList.empty())
        return;

    QString lastPath = setting.value("LastFilePath").toString();
    QString NewFilePath = QDir(lastPath).filePath("NewFile.bin");
    QString NewFilePathName = QFileDialog::getSaveFileName(this,
                                                           tr("Save Merged File"),
                                                           NewFilePath,
                                                           tr("All files (*.*)"));
    if (NewFilePathName.isEmpty()) return;

    QByteArray mergedByteArray;
    for (INT32 i = 0; i < OpenedFilesList.size(); ++i) {
        QByteArray item = OpenedFilesList.at(i);
        mergedByteArray.append(item);
    }

    QFile file(NewFilePathName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(mergedByteArray);
        file.close();
    }
}

