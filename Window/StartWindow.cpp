#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMimeData>
#include <QStyleFactory>
#include <QInputDialog>
#include <QElapsedTimer>
#include <QProcess>
#include "SettingsDialog.h"
#include "GuidDefinition.h"
#include "StartWindow.h"
#include "ui_StartWindow.h"

GuidDatabase *guidData = nullptr;
UINT32       OpenedWindow = 0;

StartWindow::StartWindow(QString appPath, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StartWindow),
    BiosViewerUi(new BiosViewerWindow(this)),
    HexViewerUi(new HexViewWindow(this)),
    WindowData(new GeneralData(appPath))
{
    ui->setupUi(this);
    ui->HintLabel->setStyleSheet("color:grey;");
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
    connect(ui->actionHex_View,     SIGNAL(triggered()), this, SLOT(ActionHexViewTriggered()));
    connect(ui->actionBios_View,     SIGNAL(triggered()), this, SLOT(ActionBiosViewTriggered()));
    connect(ui->actionExtract_BIOS, SIGNAL(triggered()), this, SLOT(ActionExtractBIOSTriggered()));
    connect(ui->actionReplace_BIOS, SIGNAL(triggered()), this, SLOT(ActionReplaceBIOSTriggered()));

    if (guidData == nullptr) {
        guidData = new GuidDatabase;
    }
    OpenedWindow += 1;
}

StartWindow::~StartWindow()
{
    OpenedWindow -= 1;
    if (guidData != nullptr && OpenedWindow == 0)
        delete guidData;
    delete ui;
    delete BiosViewerUi;
    delete HexViewerUi;
    delete WindowData;
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
            WindowData->DarkmodeFlag = true;
            QApplication::setStyle(QStyleFactory::create("Fusion"));
            QApplication::setPalette(QApplication::style()->standardPalette());
        }
    }

    if (WindowData->DarkmodeFlag) {
        ui->OpenFile->setIcon(QIcon(":/open_light.svg"));
        ui->OpenInNewWindow->setIcon(QIcon(":/open_light.svg"));
        ui->actionSettings->setIcon(QIcon(":/gear_light.svg"));
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
    if (WindowData->CurrentWindow == WindowMode::BIOS) {
        BiosViewerUi->cleanup();
        BiosViewerUi->refresh();
        OpenFile(WindowData->OpenedFileName);
    } else if (WindowData->CurrentWindow == WindowMode::Hex) {
        HexViewerUi->refresh();
    }
}

void StartWindow::OpenFile(QString path, bool onlyHexView) {
    WindowData->OpenedFileName = path;

    BaseLibrarySpace::Buffer *buffer = new BaseLibrarySpace::Buffer(new std::ifstream(path.toStdString(), std::ios::in | std::ios::binary));
    if (buffer != nullptr) {
        this->setWindowTitle("Binary Viewer -- " + path);
        WindowData->cleanup();
        WindowData->WindowTitle += this->windowTitle();
        WindowData->InputImageSize = buffer->getBufferSize();
        WindowData->InputImage = buffer->getBytes(WindowData->InputImageSize);
        WindowData->buffer = buffer;

        bool onlyHex = DisableBiosViewer || onlyHexView;
        if (!onlyHex && BiosViewerUi->TryOpenBios(WindowData->InputImage, WindowData->InputImageSize)) {
            if (WindowData->CurrentWindow != WindowMode::BIOS) {
                WindowData->CurrentWindow = WindowMode::BIOS;
                BiosViewerUi->setupUi(this, WindowData);
                HexViewerUi->UiReady = false;
            }
            BiosViewerUi->cleanup();
            BiosViewerUi->loadBios(buffer);
            ui->actionCollapse->setEnabled(true);
            ui->actionHex_View->setEnabled(true);
            ui->actionBios_View->setDisabled(true);
        } else {
            if (WindowData->CurrentWindow != WindowMode::Hex) {
                WindowData->CurrentWindow = WindowMode::Hex;
                HexViewerUi->setupUi(this, WindowData);
                BiosViewerUi->UiReady = false;
            }
            HexViewerUi->loadBuffer(WindowData->InputImage, WindowData->InputImageSize);
            ui->actionCollapse->setDisabled(true);
            ui->actionHex_View->setDisabled(true);
            ui->actionBios_View->setDisabled(true);
        }

        delete buffer;
    }
}

void StartWindow::DoubleClickOpenFile(QString path) {
    OpenFile(path);
}

void StartWindow::closeEvent(QCloseEvent *event) {
    if (WindowData->CurrentWindow == WindowMode::Hex)
        HexViewerUi->closeEvent(event);
    else if (WindowData->CurrentWindow == WindowMode::BIOS)
        BiosViewerUi->closeEvent(event);

    QSettings windowSettings("Intel", "BiosViewer");
    windowSettings.setValue("mainwindow/geometry", saveGeometry());
}

void StartWindow::resizeEvent(QResizeEvent *event) {
    if (WindowData->CurrentWindow == WindowMode::Hex) {
        HexViewerUi->resizeEvent(event);
    } else if (WindowData->CurrentWindow == WindowMode::BIOS) {
        BiosViewerUi->resizeEvent(event);
    }
}

void StartWindow::dropEvent(QDropEvent* event) {
    QUrl url = event->mimeData()->urls().first();
    QFileInfo file(url.toLocalFile());
    if(file.isFile()) {
        OpenFile(file.filePath());
    }
}

void StartWindow::dragEnterEvent(QDragEnterEvent *event) //拖动文件到窗口，触发
{
    if(event->mimeData()->hasUrls())
        event->acceptProposedAction(); //事件数据中存在路径，方向事件
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

void StartWindow::ActionExitTriggered()
{
    this->close();
}

void StartWindow::ActionSettingsTriggered()
{
    SettingsDialog *settingDialog = new SettingsDialog();
    if (WindowData->DarkmodeFlag) {
        settingDialog->setWindowIcon(QIcon(":/gear_light.svg"));
    }
    settingDialog->setParentWidget(this);
    settingDialog->exec();
}

void StartWindow::ActionAboutQtTriggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void StartWindow::ActionAboutBiosViewerTriggered()
{
    QString strText= QString("<html><head/><body><p><span style=' font-size:14pt; font-weight:700;'>Binary Viewer %1"
                              "</span></p><p>Intel Internal Use Only</p><p>Built on %2 by <span style=' font-weight:700; color:#00aaff;'>Zhu, Chen")
                          .arg(__BinaryViewerVersion__).arg(__DATE__);
    QMessageBox::about(this, tr("About Binary Viewer"), strText);
}

void StartWindow::OpenInNewWindowTriggered()
{
    QString lastPath = setting.value("LastFilePath").toString();
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open Image File"),
        lastPath, tr("Image files(*.rom *.bin *.fd);;All files (*.*)"));
    if (fileName.isEmpty()){
        return;
    }
    QFileInfo fileinfo {fileName};
    setting.setValue("LastFilePath", fileinfo.path());

    StartWindow *newWindow = new StartWindow(WindowData->appDir);
    newWindow->setAttribute(Qt::WA_DeleteOnClose);
    //    newWindow->setOpenedFileName(fileName);
    newWindow->show();
    newWindow->OpenFile(fileName);
}

void StartWindow::ActionHexViewTriggered() {
    if (WindowData->CurrentWindow == WindowMode::BIOS) {
        BiosViewerUi->cleanup();
        BiosViewerUi->refresh();
        OpenFile(WindowData->OpenedFileName, true);
        ui->actionBios_View->setEnabled(true);
    }
}

void StartWindow::ActionBiosViewTriggered() {
    if (WindowData->CurrentWindow == WindowMode::Hex) {
        OpenFile(WindowData->OpenedFileName);
    }
}

void StartWindow::ActionExtractBIOSTriggered() {
    if (WindowData->CurrentWindow == WindowMode::BIOS) {
        BiosViewerUi->ActionExtractBIOSTriggered();
    }
}

void StartWindow::ActionReplaceBIOSTriggered() {
    if (WindowData->CurrentWindow == WindowMode::BIOS) {
        BiosViewerUi->ActionReplaceBIOSTriggered();
    }
}
