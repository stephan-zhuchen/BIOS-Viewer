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
#include "BiosSearch.h"

class QHexView;
class StartWindow;
class BiosViewerWindow;
class HexViewWindow;
using UefiSpace::Volume;

namespace Ui {
class BiosWindow;
}

enum class WindowMode { None, Hex, BIOS };

class GeneralData {
public:
    QString           appDir;
    QString           OpenedFileName;
    QString           WindowTitle;
    Buffer            *buffer{nullptr};
    bool              DarkmodeFlag{false};
    UINT8             *InputImage{nullptr};
    INT64             InputImageSize{};
    INT32             CurrentTabIndex{};
    WindowMode        CurrentWindow {WindowMode::None};
    BiosViewerWindow  *BiosViewerUi{nullptr};
    HexViewWindow     *HexViewerUi{nullptr};

    GeneralData(QString dir);
    ~GeneralData();
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
    BiosSearch     *BiosSearchDialog{nullptr};
    bool           searchDialogOpened{false};

    BiosViewerData()=default;
    ~BiosViewerData();
    static bool isValidBIOS(const UINT8 *image, INT64 imageLength);
};


class BiosViewerWindow : public QWidget {
    Q_OBJECT

public:
    // UI
    Ui::BiosWindow *ui;

    // Menu
    QMenu*         RightMenu{nullptr};
    QMenu*         DigestMenu{nullptr};
    QAction*       showPeCoff{nullptr};
    QAction*       showAcpiTable{nullptr};
    QAction*       showHex{nullptr};
    QAction*       showBodyHex{nullptr};
    QAction*       showDecompressedHex{nullptr};
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
    void refresh() const;
    bool detectIfwi(INT64 &BiosOffset) const;
    void setBiosFvData();
    void setFfsData();
    void setInfoWindowState(bool opened) const;
    void pushDataToVector(INT64 offset, INT64 length) const;
    bool isDarkMode() const;

    // Tree Widget
    void initSetting() const;
    void erasePadding(vector<DataModel*> &items);
    void setTreeData();
    void addTreeItem(QTreeWidgetItem *parentItem, DataModel *modelData);
    void setPanelInfo(INT64 offset, INT64 size) const;
    void RecursiveSearchOffset(DataModel* model, INT64 offset, vector<INT32> &SearchRows);

    // Data
    StartWindow     *mWindow{nullptr};
    QSettings       setting;
    GeneralData     *WindowData{nullptr};
    BiosViewerData  *InputData{nullptr};

    explicit BiosViewerWindow(StartWindow *parent);
    ~BiosViewerWindow();
    void setupUi(QMainWindow *MainWindow, GeneralData *wData);
    static bool TryOpenBios(UINT8 *image, INT64 imageLength);
    void loadBios(Buffer *buffer);
    void ActionSearchBiosTriggered();
    void ActionGotoTriggered();
    void ActionExtractBIOSTriggered();
    void ActionReplaceBIOSTriggered();

    // Event from Main Window
    bool eventFilter(QObject *obj, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

public slots:
    void HighlightTreeItem(vector<INT32> rows) const;

private slots:
    void TreeWidgetItemSelectionChanged();
    void InfoButtonClicked();
    void SearchButtonClicked();
    void setSearchDialogState(bool opened) const;

    // Menu
    void initRightMenu();
    void finiRightMenu();
    void showTreeRightMenu(QPoint);
    void showHexView();
    void showBodyHexView();
    void showDecompressedHexView();
    void showNvHexView() const;
    void showPeCoffView();
    void showAcpiTableView();
    void extractVolume();
    void extractBodyVolume();
    void extractIfwiRegion();
    void replaceIfwiRegion();
    void replaceFfsContent();
    void getMD5() const;
    void getSHA1() const;
    void getSHA224() const;
    void getSHA256() const;
    void getSHA384() const;
    void getSHA512() const;
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
