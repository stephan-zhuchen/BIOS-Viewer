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
    ui->tabWidget->setCurrentIndex(lastTabIndex);
    setAttribute(Qt::WA_DeleteOnClose);

    if (!setting.contains("Theme"))
        setting.setValue("Theme", "Default");
    if (!setting.contains("BiosViewerFontSize"))
        setting.setValue("BiosViewerFontSize", 12);
    if (!setting.contains("BiosViewerFont"))
        setting.setValue("BiosViewerFont", "Microsoft YaHei UI");

    if (!setting.contains("InfoFontSize"))
        setting.setValue("InfoFontSize", 12);
    if (!setting.contains("InfoFont"))
        setting.setValue("InfoFont", "Fira Code");
    if (!setting.contains("InfoLineSpacing"))
        setting.setValue("InfoLineSpacing", "2");

    if (!setting.contains("HexFontSize"))
        setting.setValue("HexFontSize", 12);
    if (!setting.contains("HexFont"))
        setting.setValue("HexFont", "Courier");
    if (!setting.contains("LineSpacing"))
        setting.setValue("LineSpacing", "2");

    Theme = setting.value("Theme").toString();
    StructureFontSize = setting.value("BiosViewerFontSize").toString();
    StructureFont = setting.value("BiosViewerFont").toString();
    InfoFontSize = setting.value("InfoFontSize").toString();
    InfoFont = setting.value("InfoFont").toString();
    HexFontSize = setting.value("HexFontSize").toString();
    HexFont = setting.value("HexFont").toString();
    LineSpacing = setting.value("LineSpacing").toString();
    InfoLineSpacing = setting.value("InfoLineSpacing").toString();

    ui->biosViewerThemeBox->setCurrentText(setting.value("Theme").toString());
    ui->biosViewerFontSizeBox->setCurrentText(setting.value("BiosViewerFontSize").toString());
    ui->biosViewerFontBox->setCurrentText(setting.value("BiosViewerFont").toString());

    ui->infoFontSizeBox->setCurrentText(setting.value("InfoFontSize").toString());
    ui->infoFontBox->setCurrentText(setting.value("InfoFont").toString());
    ui->infoLineSpacingBox->setCurrentText(setting.value("InfoLineSpacing").toString());

    ui->hexFontSizeBox->setCurrentText(setting.value("HexFontSize").toString());
    ui->hexFontBox->setCurrentText(setting.value("HexFont").toString());
    ui->lineSpacingBox->setCurrentText(setting.value("LineSpacing").toString());

    if (setting.value("Theme").toString() == "Dark") {
        QFile styleFile(":/qdarkstyle/dark/darkstyle.qss");
        if(styleFile.open(QIODevice::ReadOnly)) {
            QString setStyleSheet(styleFile.readAll());
            this->setStyleSheet(setStyleSheet);
            styleFile.close();
        }
    } else if (setting.value("Theme").toString() == "Light") {
        QFile styleFile(":/qdarkstyle/light/lightstyle.qss");
        if(styleFile.open(QIODevice::ReadOnly)) {
            QString setStyleSheet(styleFile.readAll());
            this->setStyleSheet(setStyleSheet);
            styleFile.close();
        }
    }
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

int SettingsDialog::lastTabIndex = 0;

void SettingsDialog::paintEvent(QPaintEvent *)
{
//    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.setPen(QPen(Qt::gray,1));
//    QPainterPath path;
//    path.addRoundedRect(QRectF(40,55,520,120), 5, 5);
//    painter.fillPath(path, Qt::gray);
//    painter.drawPath(path);
}

void SettingsDialog::on_hexFontSizeBox_activated(int index)
{
    HexFontSize = ui->hexFontSizeBox->currentText();
}

void SettingsDialog::on_hexFontBox_activated(int index)
{
    HexFont = ui->hexFontBox->currentText();
}

void SettingsDialog::on_biosViewerThemeBox_activated(int index)
{
    Theme = ui->biosViewerThemeBox->currentText();
}

void SettingsDialog::on_biosViewerFontSizeBox_activated(int index)
{
    StructureFontSize = ui->biosViewerFontSizeBox->currentText();
}

void SettingsDialog::on_biosViewerFontBox_activated(int index)
{
    StructureFont = ui->biosViewerFontBox->currentText();
}

void SettingsDialog::on_lineSpacingBox_activated(int index)
{
    LineSpacing = ui->lineSpacingBox->currentText();
}

void SettingsDialog::on_infoFontSizeBox_activated(int index)
{
    InfoFontSize = ui->infoFontSizeBox->currentText();
}

void SettingsDialog::on_infoLineSpacingBox_activated(int index)
{
    InfoLineSpacing = ui->infoLineSpacingBox->currentText();
}

void SettingsDialog::on_infoFontBox_activated(int index)
{
    InfoFont = ui->infoFontBox->currentText();
}

void SettingsDialog::on_buttonBox_accepted()
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
    lastTabIndex = ui->tabWidget->currentIndex();
    parentWidget->refresh();
}

void SettingsDialog::setParentWidget(MainWindow *pWidget) {
    parentWidget = pWidget;
}
