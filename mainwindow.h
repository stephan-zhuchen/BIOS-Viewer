#pragma once

#include <QMainWindow>
#include <QFile>
#include <QTreeWidgetItem>
#include <QSettings>
#include <QLabel>
#include <iostream>
#include <fstream>
#include "lib/BaseLib.h"
#include "lib/UefiLib.h"
#include "lib/Model.h"

#define __CapToolVersion__ "0.9.1"

using namespace BaseLibrarySpace;
using namespace UefiSpace;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void cleanup();
    void refresh();

    void OpenFile(std::string path);
    void parseBinaryInfo();
    void setFvData();
    void setFfsData();
    void getBiosID();

    // Tree Widget
    void initSettings();
    void setTreeData();
    void addTreeItem(QTreeWidgetItem *parentItem, DataModel *modelData);
    void setPanelInfo(INT64 offset, INT64 size);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void showTreeRightMenu(QPoint);
    void showHexView();

    void on_OpenFile_triggered();
    void on_actionExit_triggered();
    void on_actionSettings_triggered();
    void on_actionAboutQt_triggered();
    void on_actionAboutBiosViewer_triggered();
    void on_OpenInNewWindow_triggered();
    void on_treeWidget_itemSelectionChanged();

    void on_infoButton_clicked();

private:
    Ui::MainWindow *ui;
    Buffer *buffer;
    QByteArray *hexViewData;
    QMenu *popMenu;
    QString BiosID;
    QLabel *structureLabel;
    QLabel *infoLabel;
    QSettings setting{"./Setting.ini", QSettings::IniFormat};

    std::vector<UINT8*> FirmwareVolumeBuffer{};
    std::vector<FirmwareVolume*> FirmwareVolumeData{};
    std::vector<FvModel*> FvModelData{};
    BiosImageVolume *BiosImage;
    DataModel *BiosImageModel;
    enum treeColNum {Name=0, Type, SubType, Text};
};

