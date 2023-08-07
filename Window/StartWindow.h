#ifndef STARTWINDOW_H
#define STARTWINDOW_H

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
#include "BiosWindow.h"
#include "HexWindow.h"

#define __BiosViewerVersion__ "1.8"

namespace Ui {
class StartWindow;
}

class GeneralData;

class StartWindow : public QMainWindow
{
    Q_OBJECT

public:
    Ui::StartWindow  *ui;
    QTabWidget       *MainTabWidget{};
    vector<GeneralData*> TabData;
    bool             DisableBiosViewer{false};
    QList<QPair<QString, QString>> DefaultSettings;

    QString        appPath;
    bool           DarkmodeFlag{false};
    QSettings      setting{"Intel", "BiosViewer"};
    QSettings      SysSettings{R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)", QSettings::NativeFormat};


    explicit StartWindow(QString appPath, QWidget *parent = nullptr);
    ~StartWindow() override;

    void initSettings();
    void InstallFonts(QString FontName);
    void refresh();
    void OpenFile(const QString& path, bool onlyHexView = false);
    void OpenBuffer(UINT8* data, UINT64 length, const QString& path, bool onlyHexView=false);
    void showHintWindow();
    void showTabWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

public slots:
    void OpenFileTriggered();
    void ActionExitTriggered();
    void ActionSettingsTriggered();
    void ActionAboutQtTriggered();
    void ActionAboutBiosViewerTriggered();
    void OpenInNewWindowTriggered();
    void ActionSeperateBinaryTriggered();
    void ActionSearchTriggered();
    void ActionGotoTriggered();
    void ActionCollapseTriggered();
    void ActionExtractBinaryTriggered();
    void ActionExtractBIOSTriggered();
    void ActionReplaceBIOSTriggered();
    void MainTabWidgetCloseRequested(int index);
    void ActionTabWidgetClose();
    void CurrentTabChanged(int index);
};

#endif // STARTWINDOW_H
