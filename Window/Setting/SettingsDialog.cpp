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
    connect(ui->asciiFontBox,          SIGNAL(activated(int)),    this, SLOT(asciiFontBoxActivated(int)));
    connect(ui->biosViewerThemeBox,    SIGNAL(activated(int)),    this, SLOT(BiosViewerThemeBoxActivated(int)));
    connect(ui->biosViewerFontSizeBox, SIGNAL(activated(int)),    this, SLOT(BiosViewerFontSizeBoxActivated(int)));
    connect(ui->biosViewerFontBox,     SIGNAL(activated(int)),    this, SLOT(BiosViewerFontBoxActivated(int)));
    connect(ui->buttonBox,             SIGNAL(accepted()),        this, SLOT(buttonBoxAccepted()));
    connect(ui->lineSpacingBox,        SIGNAL(activated(int)),    this, SLOT(lineSpacingBoxActivated(int)));
    connect(ui->infoFontSizeBox,       SIGNAL(activated(int)),    this, SLOT(infoFontSizeBoxActivated(int)));
    connect(ui->infoLineSpacingBox,    SIGNAL(activated(int)),    this, SLOT(infoLineSpacingBoxActivated(int)));
    connect(ui->infoFontBox,           SIGNAL(activated(int)),    this, SLOT(infoFontBoxActivated(int)));
    connect(ui->showPaddingBox,        SIGNAL(stateChanged(int)), this, SLOT(showPaddingBoxStateChanged(int)));
    connect(ui->enableEditingBox,      SIGNAL(stateChanged(int)), this, SLOT(enableEditingBoxStateChanged(int)));
    connect(ui->onlyHexViewBox,        SIGNAL(stateChanged(int)), this, SLOT(onlyHexViewBoxStateChanged(int)));
    connect(ui->pasteModeBox,          SIGNAL(activated(int)),    this, SLOT(pasteModeBoxActivated(int)));
    setAttribute(Qt::WA_DeleteOnClose);

    CurrentSettings.Theme = setting.value("Theme").toString();
    CurrentSettings.BiosViewerFontSize = setting.value("BiosViewerFontSize").toString();
    CurrentSettings.BiosViewerFont = setting.value("BiosViewerFont").toString();
    CurrentSettings.InfoFontSize = setting.value("InfoFontSize").toString();
    CurrentSettings.InfoFont = setting.value("InfoFont").toString();
    CurrentSettings.HexFontSize = setting.value("HexFontSize").toString();
    CurrentSettings.HexFont = setting.value("HexFont").toString();
    CurrentSettings.AsciiFont = setting.value("AsciiFont").toString();
    CurrentSettings.LineSpacing = setting.value("LineSpacing").toString();
    CurrentSettings.InfoLineSpacing = setting.value("InfoLineSpacing").toString();
    CurrentSettings.PasteMode = setting.value("PasteMode").toString();

    ui->biosViewerThemeBox->setCurrentText(CurrentSettings.Theme);
    ui->biosViewerFontSizeBox->setCurrentText(CurrentSettings.BiosViewerFontSize);
    ui->biosViewerFontBox->setCurrentText(CurrentSettings.BiosViewerFont);

    ui->infoFontSizeBox->setCurrentText(CurrentSettings.InfoFontSize);
    ui->infoFontBox->setCurrentText(CurrentSettings.InfoFont);
    ui->infoLineSpacingBox->setCurrentText(CurrentSettings.InfoLineSpacing);

    ui->hexFontSizeBox->setCurrentText(CurrentSettings.HexFontSize);
    ui->hexFontBox->setCurrentText(CurrentSettings.HexFont);
    ui->asciiFontBox->setCurrentText(CurrentSettings.AsciiFont);
    ui->lineSpacingBox->setCurrentText(CurrentSettings.LineSpacing);
    ui->pasteModeBox->setCurrentText(CurrentSettings.PasteMode);

    if (setting.value("ShowPaddingItem") == "true")
        ui->showPaddingBox->setCheckState(Qt::Checked);
    else if (setting.value("ShowPaddingItem") == "false")
        ui->showPaddingBox->setCheckState(Qt::Unchecked);

    if (setting.value("EnableHexEditing") == "true")
        ui->enableEditingBox->setCheckState(Qt::Checked);
    else if (setting.value("EnableHexEditing") == "false")
        ui->enableEditingBox->setCheckState(Qt::Unchecked);

    if (setting.value("DisableBiosViewer") == "true")
        ui->onlyHexViewBox->setCheckState(Qt::Checked);
    else if (setting.value("DisableBiosViewer") == "false")
        ui->onlyHexViewBox->setCheckState(Qt::Unchecked);
}

SettingsDialog::~SettingsDialog() {
    delete ui;
}

int SettingsDialog::lastTabIndex = 0;

void SettingsDialog::hexFontSizeBoxActivated(int index)
{
    CurrentSettings.HexFontSize = ui->hexFontSizeBox->currentText();
}

void SettingsDialog::hexFontBoxActivated(int index)
{
    CurrentSettings.HexFont = ui->hexFontBox->currentText();
}

void SettingsDialog::asciiFontBoxActivated(int index)
{
    CurrentSettings.AsciiFont = ui->asciiFontBox->currentText();
}

void SettingsDialog::BiosViewerThemeBoxActivated(int index)
{
    CurrentSettings.Theme = ui->biosViewerThemeBox->currentText();
}

void SettingsDialog::BiosViewerFontSizeBoxActivated(int index)
{
    CurrentSettings.BiosViewerFontSize = ui->biosViewerFontSizeBox->currentText();
}

void SettingsDialog::BiosViewerFontBoxActivated(int index)
{
    CurrentSettings.BiosViewerFont = ui->biosViewerFontBox->currentText();
}

void SettingsDialog::lineSpacingBoxActivated(int index)
{
    CurrentSettings.LineSpacing = ui->lineSpacingBox->currentText();
}

void SettingsDialog::infoFontSizeBoxActivated(int index)
{
    CurrentSettings.InfoFontSize = ui->infoFontSizeBox->currentText();
}

void SettingsDialog::infoLineSpacingBoxActivated(int index)
{
    CurrentSettings.InfoLineSpacing = ui->infoLineSpacingBox->currentText();
}

void SettingsDialog::infoFontBoxActivated(int index)
{
    CurrentSettings.InfoFont = ui->infoFontBox->currentText();
}

void SettingsDialog::buttonBoxAccepted()
{
    setting.setValue("BiosViewerFontSize", CurrentSettings.BiosViewerFontSize);
    setting.setValue("BiosViewerFont", CurrentSettings.BiosViewerFont);
    setting.setValue("InfoFontSize", CurrentSettings.InfoFontSize);
    setting.setValue("InfoFont", CurrentSettings.InfoFont);
    setting.setValue("HexFontSize", CurrentSettings.HexFontSize);
    setting.setValue("HexFont", CurrentSettings.HexFont);
    setting.setValue("AsciiFont", CurrentSettings.AsciiFont);
    setting.setValue("LineSpacing", CurrentSettings.LineSpacing);
    setting.setValue("InfoLineSpacing", CurrentSettings.InfoLineSpacing);
    setting.setValue("Theme", CurrentSettings.Theme);
    setting.setValue("ShowPaddingItem", CurrentSettings.ShowPaddingItem);
    setting.setValue("EnableHexEditing", CurrentSettings.EnableHexEditing);
    setting.setValue("DisableBiosViewer", CurrentSettings.DisableBiosViewer);
    setting.setValue("PasteMode", CurrentSettings.PasteMode);

    lastTabIndex = ui->tabWidget->currentIndex();
    parentWidget->refresh();
}

void SettingsDialog::setParentWidget(StartWindow *pWidget) {
    parentWidget = pWidget;
}

void SettingsDialog::showPaddingBoxStateChanged(int state)
{
    if (state == Qt::Checked) {
        CurrentSettings.ShowPaddingItem = "true";
    } else if (state == Qt::Unchecked) {
        CurrentSettings.ShowPaddingItem = "false";
    }
}

void SettingsDialog::enableEditingBoxStateChanged(int state)
{
    if (state == Qt::Checked) {
        CurrentSettings.EnableHexEditing = "true";
    } else if (state == Qt::Unchecked) {
        CurrentSettings.EnableHexEditing = "false";
    }
}

void SettingsDialog::onlyHexViewBoxStateChanged(int state)
{
    if (state == Qt::Checked) {
        CurrentSettings.DisableBiosViewer = "true";
    } else if (state == Qt::Unchecked) {
        CurrentSettings.DisableBiosViewer = "false";
    }
}

void SettingsDialog::pasteModeBoxActivated(int index) {
    CurrentSettings.PasteMode = ui->pasteModeBox->currentText();
}


