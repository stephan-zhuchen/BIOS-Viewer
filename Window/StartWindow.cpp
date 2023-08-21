#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMimeData>
#include <QStyleFactory>
#include <QInputDialog>
#include <QElapsedTimer>
#include <QProcess>
#include <QFontDatabase>
#include <utility>
#include "SettingsDialog.h"
#include "UEFI/GuidDefinition.h"
#include "StartWindow.h"
#include "CapsuleWindow.h"
#include "ui_StartWindow.h"

GuidDatabase *guidData = nullptr;
UINT32       OpenedWindow = 0;

StartWindow::StartWindow(QString appPath, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StartWindow),
    appPath(std::move(appPath))
{
    ui->setupUi(this);
    showHintWindow();

    QSettings windowSettings("Intel", "BiosViewer");
    restoreGeometry(windowSettings.value("mainwindow/geometry").toByteArray());
    InstallFonts("Fira Code");
    InstallFonts("Fira Code Light");
    InstallFonts("IntelOne Mono");
    initSettings();

    connect(ui->OpenFile,           SIGNAL(triggered()), this, SLOT(OpenFileTriggered()));
    connect(ui->OpenInHexView,      SIGNAL(triggered()), this, SLOT(OpenInHexViewTriggered()));
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

/**
 * @brief Initializes the settings for the start window.
 *
 * This function is responsible for initializing the settings for the start window
 * in the StartWindow class. It sets up any necessary properties, configurations,
 * or variables needed for the start window to function properly.
 *
 * @param None
 * @return None
 */

void StartWindow::initSettings() {
    DefaultSettings = {
        {"Theme",               "Light"},
        {"BiosViewerFontSize",  "12"},
        {"BiosViewerFont",      "Microsoft YaHei UI"},
        {"ShowPaddingItem",     "false"},
        {"InfoFontSize",        "12"},
        {"InfoFont",            "Fira Code"},
        {"InfoLineSpacing",     "2"},
        {"HexFontSize",         "12"},
        {"HexFont",             "IntelOne Mono"},
        {"AsciiFont",           "Fira Code Light"},
        {"LineSpacing",         "2"},
        {"EnableHexEditing",    "true"},
        {"DisableBiosViewer",   "false"},
        {"PasteMode",           "Ask Everytime"},
        {"HexAlign",            "None"}
    };

    for (const auto& defaultSetting : DefaultSettings) {
        if (!setting.contains(defaultSetting.first)) {
            setting.setValue(defaultSetting.first, defaultSetting.second);
        }
    }

    if (setting.value("DisableBiosViewer").toString() == "true") {
        DisableBiosViewer = true;
    } else if (setting.value("DisableBiosViewer").toString() == "false") {
        DisableBiosViewer = false;
    }

    if (setting.value("Theme").toString() == "System") {
        if (SysSettings.value("AppsUseLightTheme", 1).toInt() == 0) {
            DarkmodeFlag = true;
            QApplication::setStyle(QStyleFactory::create("Fusion"));
            QApplication::setPalette(QApplication::style()->standardPalette());
        } else {
            DarkmodeFlag = false;
            QApplication::setStyle(QStyleFactory::create("windowsvista"));
            QPalette palette = QApplication::style()->standardPalette();
            palette.setColor(QPalette::Base, Qt::white);
            palette.setColor(QPalette::Window, QColor(240, 240, 240));
            palette.setColor(QPalette::AlternateBase, QColor(240, 240, 240));
            QApplication::setPalette(palette);
        }
    }
    else if (setting.value("Theme").toString() == "Light") {
        DarkmodeFlag = false;
        QApplication::setStyle(QStyleFactory::create("windowsvista"));
        QPalette palette = QApplication::style()->standardPalette();
        palette.setColor(QPalette::Base, Qt::white);
        palette.setColor(QPalette::Window, QColor(240, 240, 240));
        palette.setColor(QPalette::AlternateBase, QColor(240, 240, 240));
        QApplication::setPalette(palette);
    } else if (setting.value("Theme").toString() == "Dark") {
        DarkmodeFlag = true;
        QApplication::setStyle(QStyleFactory::create("Fusion"));
        QPalette darkPalette;
        QColor darkColor = QColor(45, 45, 45);
        QColor disabledColor = QColor(127, 127, 127);
        darkPalette.setColor(QPalette::Window, darkColor);
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, disabledColor);
        darkPalette.setColor(QPalette::Base, QColor(36, 36, 36));
        darkPalette.setColor(QPalette::AlternateBase, darkColor);
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
        darkPalette.setColor(QPalette::Dark, QColor(12, 12, 12));
        darkPalette.setColor(QPalette::Shadow, Qt::black);
        darkPalette.setColor(QPalette::Button, darkColor);
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));
        QApplication::setPalette(darkPalette);
    }

    if (DarkmodeFlag) {
        ui->OpenFile->setIcon(QIcon(":/open_light.svg"));
        ui->OpenInHexView->setIcon(QIcon(":/file-binary_light.svg"));
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

void StartWindow::InstallFonts(QString FontName) {
    INT32 FontID = QFontDatabase::addApplicationFont(FontName);
    if (FontID == -1) {
        QString FontSource = ":Font/" + FontName + ".ttf";
        QFile fontFile(FontSource);
        fontFile.open(QFile::ReadOnly);
        QByteArray fontData = fontFile.readAll();
        fontFile.close();

        FontID = QFontDatabase::addApplicationFontFromData(fontData);
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(FontID);
        if (!fontFamilies.isEmpty()) {
            qDebug() << fontFamilies << " installed";
        }
    }
}

/**
 * @brief This function refreshes the StartWindow after settings are changed
 *
 * This function is responsible for refreshing the StartWindow. It updates the
 * display with the latest content and ensures that any changes made to the
 * window are reflected immediately.
 *
 * @note This function does not return any value.
 */

void StartWindow::refresh() {
    initSettings();
    if (TabData.empty())
        return;
    GeneralData *WindowData = TabData.at(MainTabWidget->currentIndex());
    if (WindowData->CurrentWindow == WindowMode::BIOS) {
        WindowData->BiosViewerUi->refresh();
    } else if (WindowData->CurrentWindow == WindowMode::Hex) {
        WindowData->HexViewerUi->refresh();
    }
}

void StartWindow::OpenFile(const QString& path, bool onlyHexView) {
    QFileInfo fileInfo(path);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        QMessageBox::critical(this, tr("BIOS Viewer"), "Invalid File Path!");
        return;
    }
    auto *buffer = new BaseLibrarySpace::Buffer(new std::ifstream(path.toStdString(), std::ios::in | std::ios::binary));
    INT32 bufferSize = buffer->getBufferSize();
    UINT8* data = buffer->getBytes(bufferSize);
    safeDelete(buffer);

    OpenBuffer(data, bufferSize, path, onlyHexView);
}

void StartWindow::OpenBuffer(UINT8* data, UINT64 length, const QString& path, bool onlyHexView) {
    if (TabData.empty())
        showTabWindow();

    this->setWindowTitle("BIOS Viewer -- " + path);
    auto *newTabData = new GeneralData(appPath);
    TabData.push_back(newTabData);
    newTabData->OpenedFileName = path;
    newTabData->WindowTitle += this->windowTitle();
    newTabData->InputImageSize = length;
    newTabData->InputImage = data;
    newTabData->DarkmodeFlag = DarkmodeFlag;
    newTabData->parentWindow = this;
    newTabData->BiosViewerUi = new BiosViewerWindow(this);
    newTabData->HexViewerUi = new HexViewWindow(this);
    newTabData->CapsuleViewerUi = new CapsuleWindow(this);
    QFileInfo FileInfo(path);
    bool onlyHex = DisableBiosViewer || onlyHexView;
    auto *tabWidget = new QMainWindow;
    if (!onlyHex && CapsuleWindow::tryOpenCapsule(newTabData->InputImage, newTabData->InputImageSize)) {
        // Show Capsule File
        newTabData->CurrentWindow = WindowMode::CAPSULE;
        newTabData->CapsuleViewerUi->setupUi(tabWidget, newTabData);
        newTabData->CapsuleViewerUi->OpenFile(path);
        MainTabWidget->addTab(tabWidget, FileInfo.fileName());
    } else if (!onlyHex && BiosViewerWindow::TryOpenBios(newTabData->InputImage, newTabData->InputImageSize)) {
        // Show BIOS File
        newTabData->CurrentWindow = WindowMode::BIOS;
        newTabData->BiosViewerUi->setupUi(tabWidget, newTabData);
        newTabData->BiosViewerUi->loadBios();
        ui->actionCollapse->setEnabled(true);
        ui->actionHex_View->setEnabled(true);
        ui->actionBios_View->setDisabled(true);
        MainTabWidget->addTab(tabWidget, FileInfo.fileName());
    } else {
        // Show Hex View
        newTabData->CurrentWindow = WindowMode::Hex;
        newTabData->HexViewerUi->setupUi(tabWidget, newTabData);
        newTabData->HexViewerUi->loadBuffer(newTabData->InputImage, newTabData->InputImageSize);
        ui->actionCollapse->setDisabled(true);
        ui->actionHex_View->setDisabled(true);
        ui->actionBios_View->setDisabled(true);
        MainTabWidget->addTab(tabWidget, FileInfo.fileName());
    }
    MainTabWidget->setCurrentIndex((INT32)TabData.size() - 1);
}

void StartWindow::showHintWindow() {
    ui->actionReplace_BIOS->setEnabled(false);
    ui->actionExtract_BIOS->setEnabled(false);
    ui->actionExtract_Binary->setEnabled(false);
    ui->actionSeperate_Binary->setEnabled(false);
    ui->actionCollapse->setEnabled(false);
    ui->actionSearch->setEnabled(false);
    ui->actionGoto->setEnabled(false);

    auto *centralWidget = new QWidget(this);
    auto *HintLabel = new QLabel("Drag and drop or \"Start -> Open\" any File");
    HintLabel->setStyleSheet("color:grey;");
    HintLabel->setAlignment(Qt::AlignCenter);
    HintLabel->setFont(QFont(setting.value("BiosViewerFont").toString(), 28));
    auto *StartLayout = new QVBoxLayout(centralWidget);
    StartLayout->addWidget(HintLabel);
    this->setCentralWidget(centralWidget);
}

void StartWindow::showTabWindow() {
    ui->actionExtract_Binary->setEnabled(true);
    ui->actionSeperate_Binary->setEnabled(true);
    ui->actionSearch->setEnabled(true);
    ui->actionGoto->setEnabled(true);

    auto *centralWidget = new QWidget(this);
    auto *StartLayout = new QVBoxLayout(centralWidget);
    StartLayout->setObjectName("StartLayout");
    StartLayout->setContentsMargins(0, 0, 0, 0);
    MainTabWidget = new QTabWidget(centralWidget);
    MainTabWidget->setObjectName("MainTabWidget");
    MainTabWidget->setTabsClosable(true);
    MainTabWidget->setCurrentIndex(-1);
    MainTabWidget->setTabBarAutoHide(true);

    connect(MainTabWidget,          SIGNAL(tabCloseRequested(int)), this, SLOT(MainTabWidgetCloseRequested(int)));
    connect(MainTabWidget,          SIGNAL(currentChanged(int)), this, SLOT(CurrentTabChanged(int)));

    StartLayout->addWidget(MainTabWidget);
    this->setCentralWidget(centralWidget);
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
                safeDelete(WindowData);
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

void StartWindow::OpenInHexViewTriggered() {
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
    OpenFile(fileName, true);
}

void StartWindow::ActionExitTriggered() {
    this->close();
}

void StartWindow::ActionSettingsTriggered() {
    auto *settingDialog = new SettingsDialog();
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
    auto xorLambda = [](const QString& str, char key) -> QString {
        QByteArray ba = QByteArray::fromHex(str.toLatin1());
        for (int i = 0; i < ba.size(); i++)
            ba[i] = ba[i] ^ key;
        return QString(ba);
    };

    QString strText= QString("<html>"
                             "<head/>"
                             "<body>"
                             "<p><span style=' font-size:14pt; font-weight:700;'>%1</span></p>"
                             "<p>%2</p>"
                             "<p>Built on %3 by <span style=' font-weight:700; color:#00aaff;'>%4</p>"
                             "</body>"
                             "</html>").arg(
                                            xorLambda("181315097a0c333f2d3f287a6b7463746b", 0x5A),
                                            xorLambda("13342e3f367a13342e3f28343b367a0f293f7a15343623", 0x5A),
                                            __DATE__,
                                            xorLambda("00322f767a19323f34", 0x5A));
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
    QFileInfo fileInfo {fileName};
    setting.setValue("LastFilePath", fileInfo.path());

    auto *newWindow = new StartWindow(appPath);
    newWindow->setAttribute(Qt::WA_DeleteOnClose);
    newWindow->show();
    newWindow->OpenFile(fileName);
}

void StartWindow::ActionExtractBIOSTriggered() {
    if (TabData.empty())
        return;
    GeneralData *WindowData = TabData.at(MainTabWidget->currentIndex());
    if (WindowData->CurrentWindow == WindowMode::BIOS) {
        WindowData->BiosViewerUi->ActionExtractBIOSTriggered();
    }
}

void StartWindow::ActionReplaceBIOSTriggered() {
    if (TabData.empty())
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
        safeDelete(data);
        TabData.erase(TabData.begin() + index);
        MainTabWidget->removeTab(index);
        if (TabData.empty())
            showHintWindow();
    }
}

void StartWindow::ActionTabWidgetClose() {
    if (TabData.empty()) {
        ActionExitTriggered();
        return;
    }
    MainTabWidgetCloseRequested(MainTabWidget->currentIndex());
}

void StartWindow::CurrentTabChanged(int index) {
    if (index >= 0 && TabData.at(index)->CurrentWindow == WindowMode::BIOS) {
        ui->actionCollapse->setEnabled(true);
        ui->actionSearch->setEnabled(true);
        ui->actionGoto->setEnabled(true);
        if (TabData.at(index)->InputImageSize == 0x2000000) {
            ui->actionReplace_BIOS->setEnabled(true);
            ui->actionExtract_BIOS->setEnabled(true);
        } else {
            ui->actionReplace_BIOS->setEnabled(false);
            ui->actionExtract_BIOS->setEnabled(false);
        }
    } else if (index >= 0 && TabData.at(index)->CurrentWindow == WindowMode::Hex) {
        ui->actionCollapse->setEnabled(false);
        ui->actionReplace_BIOS->setEnabled(false);
        ui->actionExtract_BIOS->setEnabled(false);
        ui->actionSearch->setEnabled(true);
        ui->actionGoto->setEnabled(true);
    } else if (index >= 0 && TabData.at(index)->CurrentWindow == WindowMode::CAPSULE) {
        ui->actionCollapse->setEnabled(false);
        ui->actionReplace_BIOS->setEnabled(false);
        ui->actionExtract_BIOS->setEnabled(false);
        ui->actionSearch->setEnabled(false);
        ui->actionGoto->setEnabled(false);
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
