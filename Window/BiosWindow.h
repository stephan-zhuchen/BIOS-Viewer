#ifndef BIOSWINDOW_H
#define BIOSWINDOW_H

#include <QWidget>
#include <QMainWindow>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "BaseLib.h"
#include "UefiLib.h"
#include "Model.h"
#include "iwfi.h"
#include "InfoWindow.h"
#include "SearchDialog.h"

class QHexView;
class StartWindow;
using UefiSpace::Volume;

enum class WindowMode { None, Hex, BIOS };

class GeneralData {
public:
    QString        appDir;
    QString        OpenedFileName;
    QString        WindowTitle;
    Buffer         *buffer;
    bool           DarkmodeFlag{false};
    UINT8          *InputImage{nullptr};
    INT64          InputImageSize;
    WindowMode     CurrentWindow {WindowMode::None};

    GeneralData(QString dir);
    ~GeneralData();
    void cleanup();
};

class BiosViewerData {
public:
    DataModel                    *InputImageModel{nullptr};
    std::vector<UINT8*>          FirmwareVolumeBuffer{};
    std::vector<FirmwareVolume*> FirmwareVolumeData{};
    BiosImageVolume              *BiosImage{nullptr};
    std::vector<IfwiVolume*>     IFWI_Sections;
    std::vector<DataModel*>      IFWI_ModelData;
    enum treeColNum {Name=0, Type, SubType, Text};

    DataModel      *RightClickeditemModel{nullptr};

    QString        flashmap;
    bool           BiosValidFlag{true};
    bool           IFWI_exist{false};
    InfoWindow     *infoWindow{nullptr};
    bool           infoWindowOpened{false};
    SearchDialog   *searchDialog{nullptr};
    bool           searchDialogOpened{false};

    BiosViewerData()=default;
    ~BiosViewerData();
    static bool isValidBIOS(UINT8 *image, INT64 imageLength);
};

class BiosViewerWindow : public QWidget {
    Q_OBJECT

public:
    // UI
    QWidget        *centralwidget;
    QVBoxLayout    *CentralwidgetVerticalLayout;
    QSpacerItem    *verticalSpacer;
    QHBoxLayout    *TitleHorizontalLayout;
    QLabel         *titleInfomation;
    QSpacerItem    *horizontalSpacer;
    QPushButton    *searchButton;
    QPushButton    *HexViewButton;
    QPushButton    *infoButton;
    QSpacerItem    *verticalSpacer_2;
    QHBoxLayout    *LittleHorizontalLayout;
    QLabel         *structureLabel;
    QLabel         *infoLabel;
    QSpacerItem    *verticalSpacer_3;
    QHBoxLayout    *MainHorizontalLayout;
    QTreeWidget    *treeWidget;
    QVBoxLayout    *InfoVerticalLayout;
    QLineEdit      *AddressPanel;
    QTextBrowser   *infoBrowser;

    // Menu
    QMenu*         RightMenu{nullptr};
    QMenu*         DigestMenu{nullptr};
    QAction*       showPeCoff{nullptr};
    QAction*       showAcpiTable{nullptr};
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

    // Parse and show
    void cleanup();
    void refresh();
    bool detectIfwi(INT64 &BiosOffset);
    void setBiosFvData();
    void setFfsData();
    void setInfoWindowState(bool opened);
    void setSearchDialogState(bool opened);
    void pushDataToVector(INT64 offset, INT64 length);
    void HighlightTreeItem(vector<INT32> rows);
    bool isDarkMode();

    // Tree Widget
    void initSetting();
    void erasePadding(vector<DataModel*> &items);
    void setTreeData();
    void addTreeItem(QTreeWidgetItem *parentItem, DataModel *modelData);
    void setPanelInfo(INT64 offset, INT64 size);
    void RecursiveSearchOffset(DataModel* model, INT64 offset, vector<INT32> &SearchRows);

    // Data
    bool            UiReady{false};
    StartWindow     *mWindow{nullptr};
    QSettings       setting;
    GeneralData     *WindowData{nullptr};
    BiosViewerData  *InputData{nullptr};

    explicit BiosViewerWindow(StartWindow *parent);
    ~BiosViewerWindow();
    void setupUi(QMainWindow *MainWindow, GeneralData *wData);
    void retranslateUi(QMainWindow *MainWindow);
    bool TryOpenBios(UINT8 *image, INT64 imageLength);
    void loadBios(Buffer *buffer);
    void ActionSearchBiosTriggered();
    void ActionGotoTriggered();
    void ActionExtractBIOSTriggered();
    void ActionReplaceBIOSTriggered();

    // Event from Main Window
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void TreeWidgetItemSelectionChanged();
    void InfoButtonClicked();
    void SearchButtonClicked();
    void ShowHexViewClicked();

    // Menu
    void initRightMenu();
    void finiRightMenu();
    void showTreeRightMenu(QPoint);
    void showHexView();
    void showBodyHexView();
    void showNvHexView();
    void showPeCoffView();
    void showAcpiTableView();
    void extractVolume();
    void extractBodyVolume();
    void extractIfwiRegion();
    void replaceIfwiRegion();
    void replaceFfsContent();
    void getMD5();
    void getSHA1();
    void getSHA224();
    void getSHA256();
    void getSHA384();
    void getSHA512();
};

class BiosException : public exception {
private:
    string message;
public:
    BiosException();
    explicit BiosException(const string& str);
    const char* what() const noexcept override;
};

#endif // BIOSWINDOW_H
