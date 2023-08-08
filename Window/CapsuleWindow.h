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
#include <vector>
#include <iostream>
#include <memory>
#include "uefilib.h"
#include "CapsuleLib.h"
#include "./ui_CapsuleWindow.h"


using namespace std;
using namespace CapsuleToolSpace;
QT_BEGIN_NAMESPACE
namespace Ui { class CapsuleUiClass; }
QT_END_NAMESPACE

class GeneralData;
class StartWindow;

class CapsuleWindow : public QWidget
{
    Q_OBJECT

public:
    CapsuleWindow(StartWindow *parent);
    ~CapsuleWindow();
    void setupUi(QMainWindow *MainWindow, GeneralData *wData);

    Buffer          *buffer{nullptr};
    UefiSpace::Volume  *data{nullptr};
    GeneralData     *WindowData{nullptr};

    shared_ptr<CapsuleOverviewClass>      CapsuleOverview;
    shared_ptr<UefiCapsuleHeaderClass>    UefiCapsuleHeader;
    shared_ptr<FmpCapsuleHeaderClass>     FmpCapsuleHeader;
    shared_ptr<FmpAuthHeaderClass>        FmpAuthHeader;
    shared_ptr<FmpPayloadHeaderClass>     FmpPayloadHeader;
    shared_ptr<FirmwareVolumeHeaderClass> FirmwareVolumeHeader;
    shared_ptr<FfsFileHeaderClass>        FfsFileHeader;
    shared_ptr<MicrocodeVersionClass>     MicrocodeVersion;
    shared_ptr<ConfigIniClass>            ConfigIni;
    shared_ptr<BgupHeaderClass>           BgupHeader;
    vector<shared_ptr<BgupHeaderClass>>   BgupHeaderVector;
    shared_ptr<BiosClass>                 BiosInstance;
    shared_ptr<AcmClass>                  AcmInstance;
    shared_ptr<EcClass>                   EcInstance;
    shared_ptr<MeClass>                   MeInstance;

    vector<shared_ptr<CPUMicrocodeHeaderClass>> MicrocodeHeaderVector;
    vector<shared_ptr<EntryHeaderClass>> EntryList;
    int             currentRow;
    QString         LabelText;
    stringstream    CapsuleInfo;
    QSettings       setting{"Intel", "BiosViewer"};

    // Menu
    QMenu*         RightMenu{nullptr};
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

    void closeEvent(QCloseEvent *event) override;
    static bool tryOpenCapsule(UINT8 *image, INT64 imageLength);
    void OpenFile(QString path);
    void showPayloadInfomation(QString filePath);
    void parsePayloadAtIndex(INT64 index);
    void parseMicrocodeCapsuleInfo(INT64 offset, QString& labelMode, stringstream& CapsuleInfo);
    void parseBiosCapsuleInfo(INT64& offset, QString &labelMode, stringstream &CapsuleInfo);
    void parseIfwiCapsuleInfo(INT64& offset, QString &labelMode, stringstream &CapsuleInfo);
    void parseMonoCapsuleInfo(INT64& offset, QString &labelMode, stringstream &CapsuleInfo);
    void parseACMCapsuleInfo(INT64 offset, QString &labelMode, stringstream &CapsuleInfo);
    void parseEcCapsuleInfo(INT64 offset, QString &labelMode, stringstream &CapsuleInfo);
    void parseMeCapsuleInfo(INT64 offset, QString &labelMode, stringstream &CapsuleInfo);

    void initSetting();
    void fini();
    void initRightMenu();
    void finiRightMenu();
    void setPanelInfo(INT64 offset, INT64 size);
    void addList(const shared_ptr<EntryHeaderClass>& EntryHeader);

private slots:
    void listWidget_itemSelectionChanged();
    void itemBox_activated(int index);
    void showListRightMenu(const QPoint &pos);
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
