#pragma once

#include <QMainWindow>
#include <QFile>
#include <QTreeWidgetItem>
#include <QSettings>
#include <QLabel>
#include <QCloseEvent>
#include <iostream>
#include <fstream>
#include <thread>
#include "lib/BaseLib.h"
#include "lib/UefiLib.h"
#include "lib/Model.h"
#include "lib/iwfi.h"
#include "InfoWindow.h"
#include "SearchDialog.h"

#define __BiosViewerVersion__ "1.0"

using namespace BaseLibrarySpace;
using namespace UefiSpace;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString applicationDir, QWidget *parent = nullptr);
    ~MainWindow();

    void cleanup();
    void refresh();

    void OpenFile(QString path);
    void DoubleClickOpenFile(QString path);
    void parseBinaryInfo();
    bool detectIfwi(INT64 &BiosOffset);
    void setBiosFvData();
    void setFfsData();
    void setInfoWindowState(bool opened);
    void setSearchDialogState(bool opened);
    void pushDataToVector(INT64 offset, INT64 length);
    void HighlightTreeItem(vector<INT32> rows);
    bool isDarkMode();

    // Tree Widget
    void initSettings();
    void erasePadding(vector<DataModel*> &items);
    void setTreeData();
    void addTreeItem(QTreeWidgetItem *parentItem, DataModel *modelData);
    void setPanelInfo(INT64 offset, INT64 size);
    void setOpenedFileName(QString name);
    void RecursiveSearchOffset(DataModel* model, INT64 offset, vector<INT32> &SearchRows);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void showTreeRightMenu(QPoint);
    void showHexView();
    void showBodyHexView();
    void showNvHexView();
    void extractVolume();
    void extractBodyVolume();
    void getMD5();
    void getSHA1();
    void getSHA224();
    void getSHA256();
    void getSHA384();
    void getSHA512();

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
    QString        appDir;
    QString        flashmap;
    bool           DarkmodeFlag;
    bool           BiosValidFlag;
    bool           IFWI_exist;
    InfoWindow     *infoWindow;
    bool           infoWindowOpened;
    SearchDialog   *searchDialog;
    bool           searchDialogOpened;
    QSettings      setting;
    QSettings      SysSettings{"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat};

    UINT8                        *InputImage;
    INT64                        InputImageSize;
    DataModel                    *InputImageModel;
    std::vector<UINT8*>          FirmwareVolumeBuffer{};
    std::vector<FirmwareVolume*> FirmwareVolumeData{};
    BiosImageVolume              *BiosImage;
    std::vector<IfwiVolume*>     IFWI_Sections;
    std::vector<DataModel*>      IFWI_ModelData;
    enum treeColNum {Name=0, Type, SubType, Text};
};

