#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"
#include <QPainter>
#include <QPainterPath>
#include <iostream>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    connect(ui->hexFontSizeBox,        SIGNAL(activated(int)),    this, SLOT(hexFontSizeBoxActivated(int)));
    connect(ui->hexFontBox,            SIGNAL(activated(int)),    this, SLOT(hexFontBoxActivated(int)));
    connect(ui->biosViewerThemeBox,    SIGNAL(activated(int)),    this, SLOT(biosViewerThemeBoxActivated(int)));
    connect(ui->biosViewerFontSizeBox, SIGNAL(activated(int)),    this, SLOT(biosViewerFontSizeBoxActivated(int)));
    connect(ui->biosViewerFontBox,     SIGNAL(activated(int)),    this, SLOT(biosViewerFontBoxActivated(int)));
    connect(ui->buttonBox,             SIGNAL(accepted()),        this, SLOT(buttonBoxAccepted()));
    connect(ui->lineSpacingBox,        SIGNAL(activated(int)),    this, SLOT(lineSpacingBoxActivated(int)));
    connect(ui->infoFontSizeBox,       SIGNAL(activated(int)),    this, SLOT(infoFontSizeBoxActivated(int)));
    connect(ui->infoLineSpacingBox,    SIGNAL(activated(int)),    this, SLOT(infoLineSpacingBoxActivated(int)));
    connect(ui->infoFontBox,           SIGNAL(activated(int)),    this, SLOT(infoFontBoxActivated(int)));
    connect(ui->showPaddingBox,        SIGNAL(stateChanged(int)), this, SLOT(showPaddingBoxStateChanged(int)));
    connect(ui->EnableMultiThread,     SIGNAL(stateChanged(int)), this, SLOT(enableMultiThreadStateChanged(int)));
    connect(ui->enableEditingBox,      SIGNAL(stateChanged(int)), this, SLOT(enableEditingBoxStateChanged(int)));
    connect(ui->onlyHexViewBox,        SIGNAL(stateChanged(int)), this, SLOT(onlyHexViewBoxStateChanged(int)));
    connect(ui->pasteModeBox,          SIGNAL(activated(int)),    this, SLOT(pasteModeBoxActivated(int)));

    ui->tabWidget->setCurrentIndex(lastTabIndex);
    setAttribute(Qt::WA_DeleteOnClose);

//    if (!setting.contains("Theme"))
//        setting.setValue("Theme", "System");
//    if (!setting.contains("BiosViewerFontSize"))
//        setting.setValue("BiosViewerFontSize", 12);
//    if (!setting.contains("BiosViewerFont"))
//        setting.setValue("BiosViewerFont", "Microsoft YaHei UI");

//    if (!setting.contains("InfoFontSize"))
//        setting.setValue("InfoFontSize", 12);
//    if (!setting.contains("InfoFont"))
//        setting.setValue("InfoFont", "Fira Code");
//    if (!setting.contains("InfoLineSpacing"))
//        setting.setValue("InfoLineSpacing", "2");

//    if (!setting.contains("HexFontSize"))
//        setting.setValue("HexFontSize", 12);
//    if (!setting.contains("HexFont"))
//        setting.setValue("HexFont", "Courier");
//    if (!setting.contains("LineSpacing"))
//        setting.setValue("LineSpacing", "2");

//    if (!setting.contains("ShowPaddingItem"))
//        setting.setValue("ShowPaddingItem", "false");
//    if (!setting.contains("EnableMultiThread"))
//        setting.setValue("EnableMultiThread", "true");
//    if (!setting.contains("EnableHexEditing"))
//        setting.setValue("EnableHexEditing", "true");

    Theme = setting.value("Theme").toString();
    StructureFontSize = setting.value("BiosViewerFontSize").toString();
    StructureFont = setting.value("BiosViewerFont").toString();
    InfoFontSize = setting.value("InfoFontSize").toString();
    InfoFont = setting.value("InfoFont").toString();
    HexFontSize = setting.value("HexFontSize").toString();
    HexFont = setting.value("HexFont").toString();
    LineSpacing = setting.value("LineSpacing").toString();
    InfoLineSpacing = setting.value("InfoLineSpacing").toString();
    PasteMode = setting.value("PasteMode").toString();

    ui->biosViewerThemeBox->setCurrentText(Theme);
    ui->biosViewerFontSizeBox->setCurrentText(StructureFontSize);
    ui->biosViewerFontBox->setCurrentText(StructureFont);

    ui->infoFontSizeBox->setCurrentText(InfoFontSize);
    ui->infoFontBox->setCurrentText(InfoFont);
    ui->infoLineSpacingBox->setCurrentText(InfoLineSpacing);

    ui->hexFontSizeBox->setCurrentText(HexFontSize);
    ui->hexFontBox->setCurrentText(HexFont);
    ui->lineSpacingBox->setCurrentText(LineSpacing);
    ui->pasteModeBox->setCurrentText(PasteMode);

    if (setting.value("ShowPaddingItem") == "true")
        ui->showPaddingBox->setCheckState(Qt::Checked);
    else if (setting.value("ShowPaddingItem") == "false")
        ui->showPaddingBox->setCheckState(Qt::Unchecked);

    if (setting.value("EnableMultiThread") == "true")
        ui->EnableMultiThread->setCheckState(Qt::Checked);
    else if (setting.value("EnableMultiThread") == "false")
        ui->EnableMultiThread->setCheckState(Qt::Unchecked);

    if (setting.value("EnableHexEditing") == "true")
        ui->enableEditingBox->setCheckState(Qt::Checked);
    else if (setting.value("EnableHexEditing") == "false")
        ui->enableEditingBox->setCheckState(Qt::Unchecked);

    if (setting.value("DisableBiosViewer") == "true")
        ui->onlyHexViewBox->setCheckState(Qt::Checked);
    else if (setting.value("DisableBiosViewer") == "false")
        ui->onlyHexViewBox->setCheckState(Qt::Unchecked);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

int SettingsDialog::lastTabIndex = 0;

void SettingsDialog::hexFontSizeBoxActivated(int index)
{
    HexFontSize = ui->hexFontSizeBox->currentText();
}

void SettingsDialog::hexFontBoxActivated(int index)
{
    HexFont = ui->hexFontBox->currentText();
}

void SettingsDialog::biosViewerThemeBoxActivated(int index)
{
    Theme = ui->biosViewerThemeBox->currentText();
}

void SettingsDialog::biosViewerFontSizeBoxActivated(int index)
{
    StructureFontSize = ui->biosViewerFontSizeBox->currentText();
}

void SettingsDialog::biosViewerFontBoxActivated(int index)
{
    StructureFont = ui->biosViewerFontBox->currentText();
}

void SettingsDialog::lineSpacingBoxActivated(int index)
{
    LineSpacing = ui->lineSpacingBox->currentText();
}

void SettingsDialog::infoFontSizeBoxActivated(int index)
{
    InfoFontSize = ui->infoFontSizeBox->currentText();
}

void SettingsDialog::infoLineSpacingBoxActivated(int index)
{
    InfoLineSpacing = ui->infoLineSpacingBox->currentText();
}

void SettingsDialog::infoFontBoxActivated(int index)
{
    InfoFont = ui->infoFontBox->currentText();
}

void SettingsDialog::buttonBoxAccepted()
{
    setting.setValue("BiosViewerFontSize", StructureFontSize);
    setting.setValue("BiosViewerFont", StructureFont);
    setting.setValue("InfoFontSize", InfoFontSize);
    setting.setValue("InfoFont", InfoFont);
    setting.setValue("HexFontSize", HexFontSize);
    setting.setValue("HexFont", HexFont);
    setting.setValue("LineSpacing", LineSpacing);
    setting.setValue("InfoLineSpacing", InfoLineSpacing);
    setting.setValue("Theme", Theme);
    setting.setValue("ShowPaddingItem", ShowPaddingItem);
    setting.setValue("EnableMultiThread", EnableMultiThread);
    setting.setValue("EnableHexEditing", EnableHexEditing);
    setting.setValue("DisableBiosViewer", DisableBiosViewer);
    setting.setValue("PasteMode", PasteMode);
    lastTabIndex = ui->tabWidget->currentIndex();
    parentWidget->refresh();
}

void SettingsDialog::setParentWidget(StartWindow *pWidget) {
    parentWidget = pWidget;
}

void SettingsDialog::showPaddingBoxStateChanged(int state)
{
    if (state == Qt::Checked) {
        ShowPaddingItem = "true";
    } else if (state == Qt::Unchecked) {
        ShowPaddingItem = "false";
    }
}

void SettingsDialog::enableMultiThreadStateChanged(int state) {
    if (state == Qt::Checked) {
        EnableMultiThread = "true";
    } else if (state == Qt::Unchecked) {
        EnableMultiThread = "false";
    }
}

void SettingsDialog::enableEditingBoxStateChanged(int state)
{
    if (state == Qt::Checked) {
        EnableHexEditing = "true";
    } else if (state == Qt::Unchecked) {
        EnableHexEditing = "false";
    }
}

void SettingsDialog::onlyHexViewBoxStateChanged(int state)
{
    if (state == Qt::Checked) {
        DisableBiosViewer = "true";
    } else if (state == Qt::Unchecked) {
        DisableBiosViewer = "false";
    }
}

void SettingsDialog::pasteModeBoxActivated(int index) {
    PasteMode = ui->pasteModeBox->currentText();
}
