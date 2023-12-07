#pragma once

#include <QtWidgets/QMainWindow>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QTime>
#include <QDir>
#include <QFile>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSettings>
#include "Capsule/CapsuleHeader.h"
#include "DataModel.h"
#include "./ui_CapsuleWindow.h"

using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class CapsuleUiClass; }
QT_END_NAMESPACE

class GeneralData;
class StartWindow;

class CapsuleViewerData {
public:
    QList<Volume*>               VolumeDataList{};
    Volume                       *OverviewVolume{nullptr};
    DataModel                    RightClickedItemModel;
    QString                      CapsuleType;
    QString                      OverviewInfo;

    CapsuleViewerData()=default;
    ~CapsuleViewerData();
    static bool isValidCapsule(UINT8 *image, INT64 imageLength);
};

class CapsuleWindow : public QWidget
{
    Q_OBJECT

public:
    CapsuleWindow(StartWindow *parent);
    ~CapsuleWindow();
    void setupUi(QMainWindow *MainWindow, GeneralData *wData);

    GeneralData         *WindowData{nullptr};
    CapsuleViewerData   *CapsuleData{nullptr};
    QSettings           setting{"Intel", "BiosViewer"};

    // Menu
    QMenu*         CustomMenu{nullptr};
    QMenu*         DigestMenu{nullptr};
    QAction*       showHex{nullptr};
    QAction*       openTab{nullptr};
    QAction*       ExtractRegion{nullptr};
    QAction*       md5_Menu{nullptr};
    QAction*       sha1_Menu{nullptr};
    QAction*       sha224_Menu{nullptr};
    QAction*       sha256_Menu{nullptr};
    QAction*       sha384_Menu{nullptr};
    QAction*       sha512_Menu{nullptr};

    static bool tryOpenCapsule(const UINT8 *image, INT64 imageLength);
    void  LoadCapsule();
    void  ParseStandardCapsule(INT64 CapsuleOffset, const QString& CapsuleType);
    void  ParseMonolithicCapsule(INT64 CapsuleOffset);
    INT64 ParsePayloadInFfs(INT64 FfsOffset, const QString& CapsuleType);
    INT64 ParseBgupInFfs(INT64 BgupOffset, IniConfigFile *ConfigIni);
    void  ParseMicrocodeCapsule(INT64 CapsuleOffset);
    INT64 ParsePayloadCapsule(INT64 CapsuleOffset, INT64 PayloadSize, const QString& CapsuleType);
    void  InitSetting();

    void InitCustomMenu();
    void CleanupCustomMenu();
    void setPanelInfo(INT64 offset, INT64 size);
    void addListItem(const QList<Volume*> &volumeList);

private slots:
    void listWidget_itemSelectionChanged();
    void showListCustomMenu(const QPoint &pos);
    void showHexView();
    void openInNewTab();
    void extractCapsuleRegion();
    void getMD5();
    void getSHA1();
    void getSHA224();
    void getSHA256();
    void getSHA384();
    void getSHA512();

private:
    Ui::CapsuleViewWindow *ui;
};
