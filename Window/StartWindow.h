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

#define __BinaryViewerVersion__ "0.9"

namespace Ui {
class StartWindow;
}

class GeneralData;

class StartWindow : public QMainWindow
{
    Q_OBJECT

public:
    Ui::StartWindow  *ui;
    QTabWidget       *tabWidget;
    BiosViewerWindow *BiosViewerUi;
    HexViewWindow    *HexViewerUi;
    GeneralData      *WindowData;
    bool             DisableBiosViewer{false};

    QSettings      setting{"Intel", "BiosViewer"};
    QSettings      SysSettings{"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat};


    explicit StartWindow(QString appPath, QWidget *parent = nullptr);
    ~StartWindow();

    void initSettings();
    void refresh();
    void OpenFile(QString path, bool onlyHexView = false);
    void DoubleClickOpenFile(QString path);

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
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
    void ActionHexViewTriggered();
    void ActionBiosViewTriggered();
    void ActionExtractBIOSTriggered();
    void ActionReplaceBIOSTriggered();
};

#endif // STARTWINDOW_H
