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
#include "BaseLib.h"
#include "UefiLib.h"
#include "Model.h"
#include "iwfi.h"
#include "InfoWindow.h"
#include "SearchDialog.h"

#define __BiosViewerVersion__ "1.5"

using namespace BaseLibrarySpace;
using namespace UefiSpace;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString appPath, QWidget *parent = nullptr);
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
    void initRightMenu();
    void finiRightMenu();
    void showTreeRightMenu(QPoint);
    void showHexView();
    void showBodyHexView();
    void showNvHexView();
    void showPeCoffView();
    void extractVolume();
    void extractBodyVolume();
    void extractIfwiRegion();
    void replaceIfwiRegion();
    void replaceFfsContent();
    void replaceMicrocodeFile();
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
    void ActionExtractBinaryTriggered();

private:
    Ui::MainWindow *ui;
    Buffer         *buffer;
    DataModel      *RightClickeditemModel;
    QLabel         *structureLabel;
    QLabel         *infoLabel;
    QString        OpenedFileName;
    QString        flashmap;
    bool           DarkmodeFlag;
    bool           BiosValidFlag;
    bool           IFWI_exist;
    InfoWindow     *infoWindow;
    bool           infoWindowOpened;
    SearchDialog   *searchDialog;
    bool           searchDialogOpened;
    QString        appDir;
    QMenu*         RightMenu{nullptr};
    QMenu*         DigestMenu{nullptr};
    QAction*       showPeCoff{nullptr};
    QAction*       showHex{nullptr};
    QAction*       showBodyHex{nullptr};
    QAction*       extractVolumeAction{nullptr};
    QAction*       extractBodyVolumeAction{nullptr};
    QAction*       showNvHex{nullptr};
    QAction*       ExtractRegion{nullptr};
    QAction*       ReplaceRegion{nullptr};
    QAction*       ReplaceFile{nullptr};
    QAction*       md5_Menu{nullptr};
    QAction*       sha1_Menu{nullptr};
    QAction*       sha224_Menu{nullptr};
    QAction*       sha256_Menu{nullptr};
    QAction*       sha384_Menu{nullptr};
    QAction*       sha512_Menu{nullptr};
    QSettings      setting{"Intel", "BiosViewer"};
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

