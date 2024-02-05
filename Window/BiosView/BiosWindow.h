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
#include <exception>
#include "Volume.h"
#include "DataModel.h"
#include "IfwiRegion/BiosRegion.h"
#include "InfoWindow/InfoWindow.h"
#include "Search/BiosSearch.h"

class QHexView;
class StartWindow;
class BiosViewerWindow;
class HexViewWindow;
class CapsuleWindow;

namespace Ui {
class BiosWindow;
}

enum class WindowMode { None, Hex, BIOS, CAPSULE };

class GeneralData {
public:
    QString           appDir;
    QString           OpenedFileName;
    QString           WindowTitle;
    bool              DarkmodeFlag{false};
    UINT8             *InputImage{nullptr};
    INT64             InputImageSize{};
    INT32             CurrentTabIndex{};
    WindowMode        CurrentWindow {WindowMode::None};
    StartWindow       *parentWindow{nullptr};
    BiosViewerWindow  *BiosViewerUi{nullptr};
    HexViewWindow     *HexViewerUi{nullptr};
    CapsuleWindow     *CapsuleViewerUi{nullptr};

    explicit GeneralData(QString dir);
    ~GeneralData();
};

enum treeColNum {Name=0, Type, SubType};

class BiosViewerData {
public:
    QList<Volume*>               VolumeDataList{};
    Volume                       *OverviewVolume{nullptr};
    BiosRegion                   *BiosImage{nullptr};
    DataModel                    *OverviewImageModel{nullptr};
    DataModel                    RightClickedItemModel;

    bool                         BiosValidFlag{true};
    bool                         IFWI_exist{false};
    InfoWindow                   *infoWindow{nullptr};
    bool                         infoWindowOpened{false};
    BiosSearch                   *BiosSearchDialog{nullptr};
    bool                         searchDialogOpened{false};

    BiosViewerData()=default;
    ~BiosViewerData();
    static bool isValidBIOS(UINT8 *image, INT64 imageLength);
};


class BiosViewerWindow : public QWidget {
    Q_OBJECT

public:
    // UI
    Ui::BiosWindow *ui;

    // Menu
    QMenu*         CustomMenu{nullptr};
    QMenu*         DigestMenu{nullptr};
    QAction*       showPeCoff{nullptr};
    QAction*       showAcpiTable{nullptr};
    QAction*       showBgup{nullptr};
    QAction*       showHex{nullptr};
    QAction*       showBodyHex{nullptr};
    QAction*       showDecompressedHex{nullptr};
    QAction*       showDecompressedBiosHex{nullptr};
    QAction*       extractVolumeAction{nullptr};
    QAction*       extractBodyVolumeAction{nullptr};
    QAction*       showNvHex{nullptr};
    QAction*       ExtractRegion{nullptr};
    QAction*       ReplaceRegion{nullptr};
    QAction*       ReplaceAcm{nullptr};
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
    void DecodeBiosFileSystem();
    void ReorganizeVolume(Volume *volume);
    void setInfoWindowState(bool opened) const;
    void AddVolumeList(INT64 offset, INT64 length, Volume *parent, VolumeType type) const;
    bool isDarkMode() const;

    // Tree Widget
    void initSetting() const;
    void setTreeData();
    void addTreeItem(QTreeWidgetItem *parentItem, Volume *volume, bool ShowPadding);
    void setPanelInfo(INT64 offset, INT64 size) const;
//    void RecursiveSearchOffset(DataModel* model, INT64 offset, vector<INT32> &SearchRows);

    // Data
    StartWindow     *mWindow{nullptr};
    QSettings       setting;
    GeneralData     *WindowData{nullptr};
    BiosViewerData  *BiosData{nullptr};

    explicit BiosViewerWindow(StartWindow *parent);
    ~BiosViewerWindow() override;
    void setupUi(QMainWindow *MainWindow, GeneralData *wData);
    static bool TryOpenBios(UINT8 *image, INT64 imageLength);
    void loadBios();
    void ActionSearchBiosTriggered();
    void ActionGotoTriggered();
    void ActionExtractBIOSTriggered();
    void ActionReplaceBIOSTriggered();

    // Event from Main Window
    bool eventFilter(QObject *obj, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void TreeWidgetItemSelectionChanged() const;
    void InfoButtonClicked();
    void SearchButtonClicked();
    void setSearchDialogState(bool opened) const;

    // Menu
    void InitCustomMenu();
    void CleanupCustomMenu();
    void showTreeCustomMenu(QPoint pos) const;
    void showHexView() const;
    void showBodyHexView();
    void showDecompressedHexView();
    void showDecompressedBiosHexView();
    void showNvHexView() const;
    void showPeCoffView();
    void showAcpiTableView();
    void showBgupView();
    void extractVolume();
    void extractBodyVolume();
    void extractIfwiRegion();
    void replaceIfwiRegion();
    void replaceAcmContent();
    void getMD5() const;
    void getSHA1() const;
    void getSHA224() const;
    void getSHA256() const;
    void getSHA384() const;
    void getSHA512() const;
};

#endif // BIOSWINDOW_H
