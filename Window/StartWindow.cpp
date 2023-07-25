#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMimeData>
#include <QStyleFactory>
#include <QInputDialog>
#include <QElapsedTimer>
#include <QProcess>
#include "SettingsDialog.h"
#include "UEFI/GuidDefinition.h"
#include "StartWindow.h"
#include "ui_StartWindow.h"

GuidDatabase *guidData = nullptr;
UINT32       OpenedWindow = 0;

StartWindow::StartWindow(QString appPath, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StartWindow),
    appPath(appPath)
{
    ui->setupUi(this);
    showHintWindow();

    QSettings windowSettings("Intel", "BiosViewer");
    restoreGeometry(windowSettings.value("mainwindow/geometry").toByteArray());
    initSettings();

    connect(ui->OpenFile,           SIGNAL(triggered()), this, SLOT(OpenFileTriggered()));
    connect(ui->actionExit,         SIGNAL(triggered()), this, SLOT(ActionExitTriggered()));
    connect(ui->actionSettings,     SIGNAL(triggered()), this, SLOT(ActionSettingsTriggered()));
    connect(ui->actionAboutQt,      SIGNAL(triggered()), this, SLOT(ActionAboutQtTriggered()));
    connect(ui->actionAboutBiosViewer, SIGNAL(triggered()), this, SLOT(ActionAboutBiosViewerTriggered()));
    connect(ui->OpenInNewWindow,    SIGNAL(triggered()), this, SLOT(OpenInNewWindowTriggered()));
    connect(ui->actionSeperate_Binary, SIGNAL(triggered()), this, SLOT(ActionSeperateBinaryTriggered()));
    connect(ui->actionSearch,       SIGNAL(triggered()), this, SLOT(ActionSearchTriggered()));
    connect(ui->actionGoto,         SIGNAL(triggered()), this, SLOT(ActionGotoTriggered()));
    connect(ui->actionCollapse,     SIGNAL(triggered()), this, SLOT(ActionCollapseTriggered()));
    connect(ui->actionExtract_Binary, SIGNAL(triggered()), this, SLOT(ActionExtractBinaryTriggered()));
    connect(ui->actionExtract_BIOS, SIGNAL(triggered()), this, SLOT(ActionExtractBIOSTriggered()));
    connect(ui->actionReplace_BIOS, SIGNAL(triggered()), this, SLOT(ActionReplaceBIOSTriggered()));
//    connect(MainTabWidget,          SIGNAL(tabCloseRequested(int)), this, SLOT(MainTabWidgetCloseRequested(int)));
    connect(ui->actionCloseTab,     SIGNAL(triggered()), this, SLOT(ActionTabWidgetClose()));

    if (guidData == nullptr) {
        guidData = new GuidDatabase;
    }
    OpenedWindow += 1;
}

StartWindow::~StartWindow() {
    OpenedWindow -= 1;
    if (guidData != nullptr && OpenedWindow == 0)
        delete guidData;
    delete ui;
    for (GeneralData *data:TabData)
        delete data;
}

void StartWindow::initSettings() {
    if (!setting.contains("Theme"))
        setting.setValue("Theme", "System");
    if (!setting.contains("BiosViewerFontSize"))
        setting.setValue("BiosViewerFontSize", 12);
    if (!setting.contains("BiosViewerFont"))
        setting.setValue("BiosViewerFont", "Microsoft YaHei UI");
    if (!setting.contains("ShowPaddingItem"))
        setting.setValue("ShowPaddingItem", "false");
    if (!setting.contains("EnableMultiThread"))
        setting.setValue("EnableMultiThread", "true");

    if (!setting.contains("InfoFontSize"))
        setting.setValue("InfoFontSize", 12);
    if (!setting.contains("InfoFont"))
        setting.setValue("InfoFont", "Courier");
    if (!setting.contains("InfoLineSpacing"))
        setting.setValue("InfoLineSpacing", "2");

    if (!setting.contains("HexFontSize"))
        setting.setValue("HexFontSize", 12);
    if (!setting.contains("HexFont"))
        setting.setValue("HexFont", "Courier");
    if (!setting.contains("LineSpacing"))
        setting.setValue("LineSpacing", "2");

    if (!setting.contains("EnableHexEditing"))
        setting.setValue("EnableHexEditing", "true");
    if (!setting.contains("DisableBiosViewer"))
        setting.setValue("DisableBiosViewer", "false");
    if (!setting.contains("PasteMode"))
        setting.setValue("PasteMode", "Ask Everytime");

    if (setting.value("DisableBiosViewer").toString() == "true") {
        DisableBiosViewer = true;
    }

    if (setting.value("Theme").toString() == "System") {
        if (SysSettings.value("AppsUseLightTheme", 1).toInt() == 0) {
            DarkmodeFlag = true;
            QApplication::setStyle(QStyleFactory::create("Fusion"));
            QApplication::setPalette(QApplication::style()->standardPalette());
        }
    }

    if (DarkmodeFlag) {
        ui->OpenFile->setIcon(QIcon(":/open_light.svg"));
        ui->OpenInNewWindow->setIcon(QIcon(":/open_light.svg"));
        ui->actionSettings->setIcon(QIcon(":/gear_light.svg"));
        ui->actionCloseTab->setIcon(QIcon(":/close_light.svg"));
        ui->actionExit->setIcon(QIcon(":/Exit_light.svg"));
        ui->actionSearch->setIcon(QIcon(":/search_light.svg"));
        ui->actionGoto->setIcon(QIcon(":/bookmark_light.svg"));
        ui->actionCollapse->setIcon(QIcon(":/arrows-collapse_light.svg"));
        ui->actionExtract_BIOS->setIcon(QIcon(":/box-arrow-up_light.svg"));
        ui->actionExtract_Binary->setIcon(QIcon(":/box-arrow-up_light.svg"));
        ui->actionSeperate_Binary->setIcon(QIcon(":/scissors_light.svg"));
        ui->actionReplace_BIOS->setIcon(QIcon(":/replace_light.svg"));
        ui->actionAboutBiosViewer->setIcon(QIcon(":/about_light.svg"));
        ui->actionAboutQt->setIcon(QIcon(":/about_light.svg"));
    }
}

void StartWindow::refresh() {
    initSettings();
    if (TabData.size() == 0)
        return;
    GeneralData *WindowData = TabData.at(MainTabWidget->currentIndex());
    if (WindowData->CurrentWindow == WindowMode::BIOS) {
        WindowData->BiosViewerUi->refresh();
    } else if (WindowData->CurrentWindow == WindowMode::Hex) {
        WindowData->HexViewerUi->refresh();
    }
}

void StartWindow::OpenFile(QString path, bool onlyHexView) {
    if (TabData.size() == 0)
        showTabWindow();

    BaseLibrarySpace::Buffer *buffer = new BaseLibrarySpace::Buffer(new std::ifstream(path.toStdString(), std::ios::in | std::ios::binary));
    if (buffer != nullptr) {
        this->setWindowTitle("BIOS Viewer -- " + path);
        GeneralData *newTabData = new GeneralData(appPath);
        TabData.push_back(newTabData);
        newTabData->OpenedFileName = path;
        newTabData->WindowTitle += this->windowTitle();
        newTabData->InputImageSize = buffer->getBufferSize();
        newTabData->InputImage = buffer->getBytes(newTabData->InputImageSize);
        newTabData->buffer = buffer;
        newTabData->DarkmodeFlag = DarkmodeFlag;
        newTabData->BiosViewerUi = new BiosViewerWindow(this);
        newTabData->HexViewerUi = new HexViewWindow(this);

        QFileInfo FileInfo(path);
        bool onlyHex = DisableBiosViewer || onlyHexView;
        QMainWindow *tabWidget = new QMainWindow;
        if (!onlyHex && BiosViewerWindow::TryOpenBios(newTabData->InputImage, newTabData->InputImageSize)) {
            newTabData->CurrentWindow = WindowMode::BIOS;
            newTabData->BiosViewerUi->setupUi(tabWidget, newTabData);
            newTabData->BiosViewerUi->loadBios(buffer);
            ui->actionCollapse->setEnabled(true);
            ui->actionHex_View->setEnabled(true);
            ui->actionBios_View->setDisabled(true);
            MainTabWidget->addTab(tabWidget, FileInfo.fileName());
        } else {
            newTabData->CurrentWindow = WindowMode::Hex;
            newTabData->HexViewerUi->setupUi(tabWidget, newTabData);
            newTabData->HexViewerUi->loadBuffer(newTabData->InputImage, newTabData->InputImageSize);
            ui->actionCollapse->setDisabled(true);
            ui->actionHex_View->setDisabled(true);
            ui->actionBios_View->setDisabled(true);
            MainTabWidget->addTab(tabWidget, FileInfo.fileName());
        }
        MainTabWidget->setCurrentIndex(TabData.size() - 1);
        delete buffer;
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Can not open this file!"));
    }
}

void StartWindow::DoubleClickOpenFile(QString path) {
    OpenFile(path);
}

void StartWindow::showHintWindow() {
    QWidget *centralwidget = new QWidget(this);
    QLabel *HintLabel = new QLabel("Drag and drop or \"Start -> Open\" any File");
    HintLabel->setStyleSheet("color:grey;");
    HintLabel->setAlignment(Qt::AlignCenter);
    HintLabel->setFont(QFont(setting.value("BiosViewerFont").toString(), 28));
    QVBoxLayout *StartLayout = new QVBoxLayout(centralwidget);
    StartLayout->addWidget(HintLabel);
    this->setCentralWidget(centralwidget);
}

void StartWindow::showTabWindow() {
    QWidget *centralwidget = new QWidget(this);
    QVBoxLayout *StartLayout = new QVBoxLayout(centralwidget);
    StartLayout->setObjectName("StartLayout");
    StartLayout->setContentsMargins(0, 0, 0, 0);
    MainTabWidget = new QTabWidget(centralwidget);
    MainTabWidget->setObjectName("MainTabWidget");
    MainTabWidget->setTabsClosable(true);
    MainTabWidget->setCurrentIndex(-1);
    MainTabWidget->setTabBarAutoHide(true);

    connect(MainTabWidget,          SIGNAL(tabCloseRequested(int)), this, SLOT(MainTabWidgetCloseRequested(int)));
    connect(MainTabWidget,          SIGNAL(currentChanged(int)), this, SLOT(CurrentTabChanged(int)));

    StartLayout->addWidget(MainTabWidget);
    this->setCentralWidget(centralwidget);
    this->statusBar()->hide();
}

void StartWindow::closeEvent(QCloseEvent *event) {
    auto CleanTabData = [this, event]() {
        for (INT32 index = 0; index < TabData.size(); ++index) {
            GeneralData *WindowData = TabData.at(index);
            if (WindowData->CurrentWindow == WindowMode::Hex)
                WindowData->HexViewerUi->closeEvent(event);
            else if (WindowData->CurrentWindow == WindowMode::BIOS)
                WindowData->BiosViewerUi->closeEvent(event);

            if (event->isAccepted()) {
                delete WindowData;
                TabData.erase(TabData.begin() + index);
                index -= 1;
            }
        }

        QSettings windowSettings("Intel", "BiosViewer");
        windowSettings.setValue("mainwindow/geometry", saveGeometry());
    };

    if (TabData.size() <= 1) {
        CleanTabData();
        return;
    }
    QMessageBox msgBox;
    msgBox.setText("Close current Tab or Close all Tabs ?");
    QPushButton *CloseCurrentButton = msgBox.addButton(tr("Close current Tab"), QMessageBox::ActionRole);
    QPushButton *CloseAllButton = msgBox.addButton(tr("Close all Tabs"), QMessageBox::ActionRole);
    msgBox.addButton(QMessageBox::Cancel);
    msgBox.exec();

    if (msgBox.clickedButton() == CloseCurrentButton) {
        MainTabWidgetCloseRequested(MainTabWidget->currentIndex());
        event->ignore();
    } else if (msgBox.clickedButton() == CloseAllButton) {
        CleanTabData();
    } else {
        event->ignore();
        return;
    }
}

void StartWindow::dropEvent(QDropEvent* event) {
    QUrl url = event->mimeData()->urls().first();
    QFileInfo file(url.toLocalFile());
    if(file.isFile()) {
        OpenFile(file.filePath());
    }
}

void StartWindow::dragEnterEvent(QDragEnterEvent *event) {
    if(event->mimeData()->hasUrls())
        event->acceptProposedAction();
    else
        event->ignore();
}

void StartWindow::OpenFileTriggered() {
    QString lastPath = setting.value("LastFilePath").toString();
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Image File"),
                                                    lastPath,
                                                    tr("All files (*.*);;Image files(*.rom *.bin *.fd *.fv)"));
    if (fileName.isEmpty()){
        return;
    }
    QFileInfo fileinfo {fileName};
    setting.setValue("LastFilePath", fileinfo.path());
    OpenFile(fileName);
}

void StartWindow::ActionExitTriggered() {
    this->close();
}

void StartWindow::ActionSettingsTriggered() {
    SettingsDialog *settingDialog = new SettingsDialog();
    if (DarkmodeFlag) {
        settingDialog->setWindowIcon(QIcon(":/gear_light.svg"));
    }
    settingDialog->setParentWidget(this);
    settingDialog->exec();
}

void StartWindow::ActionAboutQtTriggered() {
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void StartWindow::ActionAboutBiosViewerTriggered() {
    QString strText= QString("<html><head/><body><p><span style=' font-size:14pt; font-weight:700;'>BIOS Viewer %1"
                              "</span></p><p>Intel Internal Use Only</p><p>Built on %2 by <span style=' font-weight:700; color:#00aaff;'>Zhu, Chen")
                          .arg(__BiosViewerVersion__).arg(__DATE__);
    QMessageBox::about(this, tr("About BIOS Viewer"), strText);
}

void StartWindow::OpenInNewWindowTriggered() {
    QString lastPath = setting.value("LastFilePath").toString();
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open Image File"),
        lastPath, tr("Image files(*.rom *.bin *.fd);;All files (*.*)"));
    if (fileName.isEmpty()){
        return;
    }
    QFileInfo fileinfo {fileName};
    setting.setValue("LastFilePath", fileinfo.path());

    StartWindow *newWindow = new StartWindow(appPath);
    newWindow->setAttribute(Qt::WA_DeleteOnClose);
    newWindow->show();
    newWindow->OpenFile(fileName);
}

void StartWindow::ActionExtractBIOSTriggered() {
    if (TabData.size() == 0)
        return;
    GeneralData *WindowData = TabData.at(MainTabWidget->currentIndex());
    if (WindowData->CurrentWindow == WindowMode::BIOS) {
        WindowData->BiosViewerUi->ActionExtractBIOSTriggered();
    }
}

void StartWindow::ActionReplaceBIOSTriggered() {
    if (TabData.size() == 0)
        return;
    GeneralData *WindowData = TabData.at(MainTabWidget->currentIndex());
    if (WindowData->CurrentWindow == WindowMode::BIOS) {
        WindowData->BiosViewerUi->ActionReplaceBIOSTriggered();
    }
}

void StartWindow::MainTabWidgetCloseRequested(int index) {
    GeneralData *data = TabData.at(index);
    QCloseEvent event;
    if (data->CurrentWindow == WindowMode::Hex)
        data->HexViewerUi->closeEvent(&event);
    else if (data->CurrentWindow == WindowMode::BIOS)
        data->BiosViewerUi->closeEvent(&event);

    if (event.isAccepted()) {
        delete data;
        TabData.erase(TabData.begin() + index);
        MainTabWidget->removeTab(index);
        if (TabData.size() == 0)
            showHintWindow();
    }
}

void StartWindow::ActionTabWidgetClose() {
    if (TabData.size() == 0) {
        ActionExitTriggered();
        return;
    }
    MainTabWidgetCloseRequested(MainTabWidget->currentIndex());
}

void StartWindow::CurrentTabChanged(int index) {
    if (index >= 0 && TabData.at(index)->CurrentWindow == WindowMode::BIOS) {
        ui->actionCollapse->setEnabled(true);
    } else if (index >= 0 && TabData.at(index)->CurrentWindow == WindowMode::Hex) {
        ui->actionCollapse->setEnabled(false);
    }

    for (GeneralData *WindowData:TabData) {
        if (WindowData->CurrentWindow == WindowMode::BIOS && WindowData->BiosViewerUi->InputData->searchDialogOpened) {
            WindowData->BiosViewerUi->InputData->BiosSearchDialog->close();
        }

    }

    if (index >= 0) {
        GeneralData *WindowData = TabData.at(index);
        if (WindowData->HexViewerUi->BinaryEdited)
            this->setWindowTitle("BIOS Viewer -- " + WindowData->OpenedFileName + " *");
        else
            this->setWindowTitle("BIOS Viewer -- " + WindowData->OpenedFileName);
    } else {
        this->setWindowTitle("BIOS Viewer");
    }

}
