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

#define __BiosViewerVersion__ "0.12"

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

    void OpenFile(QString path);
    void DoubleClickOpenFile(QString path);
    void parseBinaryInfo();
    bool detectIfwi(INT64 &BiosOffset);
    void setBiosFvData();
    void setFfsData();
    void pushDataToVector(INT64 offset, INT64 length);
    void HighlightTreeItem(vector<INT32> rows);
    bool isDarkMode();

    // Tree Widget
    void initSettings();
    void setTreeData();
    void addTreeItem(QTreeWidgetItem *parentItem, DataModel *modelData, bool ShowPadding);
    void setPanelInfo(INT64 offset, INT64 size);
    void setOpenedFileName(QString name);
    void RecursiveSearchOffset(DataModel* model, INT64 offset, vector<INT32> &SearchRows);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void showTreeRightMenu(QPoint);
    void showHexView();
    void showBodyHexView();
    void showNvHexView();
    void extractVolume();
    void extractBodyVolume();

    void OpenFileTriggered();
    void ActionExitTriggered();
    void ActionSettingsTriggered();
    void ActionAboutQtTriggered();
    void ActionAboutBiosViewerTriggered();
    void OpenInNewWindowTriggered();
    void TreeWidgetItemSelectionChanged();
    void InfoButtonClicked();
    void ActionSeperateBinaryTriggered();
    void ActionExtractBIOSTriggered();
    void ActionSearchTriggered();
    void ActionGotoTriggered();
    void ActionCollapseTriggered();
    void ActionReplaceBIOSTriggered();
    void SearchButtonClicked();

private:
    Ui::MainWindow *ui;
    Buffer         *buffer;
    DataModel      *RightClickeditemModel;
    QMenu          *popMenu;
    QLabel         *structureLabel;
    QLabel         *infoLabel;
    QString        OpenedFileName;
    bool           DarkmodeFlag;
    QSettings      setting{"./Setting.ini", QSettings::IniFormat};
    QSettings      SysSettings{"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat};

    UINT8                        *InputImage;
    INT64                        InputImageSize;
    DataModel                    *InputImageModel;
    std::vector<UINT8*>          FirmwareVolumeBuffer{};
    std::vector<FirmwareVolume*> FirmwareVolumeData{};
    std::vector<FvModel*>        FvModelData{};
    BiosImageVolume              *BiosImage;
    std::vector<Volume*>         IFWI_Sections;
    std::vector<DataModel*>      IFWI_ModelData;
    enum treeColNum {Name=0, Type, SubType, Text};
};

