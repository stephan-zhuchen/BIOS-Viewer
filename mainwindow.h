#pragma once

#include <QMainWindow>
#include <QFile>
#include <QTreeWidgetItem>
#include <QSettings>
#include <iostream>
#include <fstream>
#include "lib/BaseLib.h"
#include "lib/UefiLib.h"
#include "lib/Model.h"

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

    void OpenFile(std::string path);
    void parseBinaryInfo();
    void setFvData();
    void setFfsData();
    void getBiosID();

    // Tree Widget
    void initTree();
    void setTreeData();
    void addTreeItem(QTreeWidgetItem *parentItem, DataModel *modelData);
    void setPanelInfo(INT64 offset, INT64 size);

private slots:
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void showTreeRightMenu(QPoint);
    void showHexView();

    void on_OpenFile_triggered();
    void on_actionExit_triggered();

private:
    Ui::MainWindow *ui;
    Buffer *buffer;
    QByteArray *hexViewData;
    QMenu *popMenu;
    QString BiosID;
    QSettings setting{"./Setting.ini", QSettings::IniFormat};

    std::vector<UINT8*> FirmwareVolumeBuffer{};
    std::vector<FirmwareVolume*> FirmwareVolumeData{};
    std::vector<FvModel*> FvModelData{};
    enum treeColNum {Name=0, Type, SubType, Text};
};

