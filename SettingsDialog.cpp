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
    setAttribute(Qt::WA_DeleteOnClose);

    if (!setting.contains("Theme"))
        setting.setValue("Theme", "Light");
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

    ui->biosViewerThemeBox->setCurrentText(setting.value("Theme").toString());
    ui->biosViewerFontSizeBox->setCurrentText(setting.value("BiosViewerFontSize").toString());
    ui->biosViewerFontBox->setCurrentText(setting.value("BiosViewerFont").toString());

    ui->infoFontSizeBox->setCurrentText(setting.value("InfoFontSize").toString());
    ui->infoFontBox->setCurrentText(setting.value("InfoFont").toString());
    ui->infoLineSpacingBox->setCurrentText(setting.value("InfoLineSpacing").toString());

    ui->hexFontSizeBox->setCurrentText(setting.value("HexFontSize").toString());
    ui->hexFontBox->setCurrentText(setting.value("HexFont").toString());
    ui->lineSpacingBox->setCurrentText(setting.value("LineSpacing").toString());
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

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
    QString FontSize = ui->hexFontSizeBox->currentText();
    setting.setValue("HexFontSize", FontSize);
}

void SettingsDialog::on_hexFontBox_activated(int index)
{
    QString FontFamily = ui->hexFontBox->currentText();
    setting.setValue("HexFont", FontFamily);
}

void SettingsDialog::on_biosViewerThemeBox_activated(int index)
{
    QString theme = ui->biosViewerThemeBox->currentText();
    setting.setValue("Theme", theme);
}

void SettingsDialog::on_biosViewerFontSizeBox_activated(int index)
{
    QString biosViewerFontSize = ui->biosViewerFontSizeBox->currentText();
    setting.setValue("BiosViewerFontSize", biosViewerFontSize);
}

void SettingsDialog::on_biosViewerFontBox_activated(int index)
{
    QString biosViewerFont = ui->biosViewerFontBox->currentText();
    setting.setValue("BiosViewerFont", biosViewerFont);
}

void SettingsDialog::on_buttonBox_accepted()
{
    parentWidget->refresh();
}

void SettingsDialog::setParentWidget(MainWindow *pWidget) {
    parentWidget = pWidget;
}

void SettingsDialog::on_lineSpacingBox_activated(int index)
{
    QString biosViewerFontSize = ui->lineSpacingBox->currentText();
    setting.setValue("LineSpacing", biosViewerFontSize);
}

void SettingsDialog::on_infoFontSizeBox_activated(int index)
{
    QString infoFontSize = ui->infoFontSizeBox->currentText();
    setting.setValue("InfoFontSize", infoFontSize);
}

void SettingsDialog::on_infoLineSpacingBox_activated(int index)
{
    QString infoLineSpacing = ui->infoLineSpacingBox->currentText();
    setting.setValue("InfoLineSpacing", infoLineSpacing);
}

void SettingsDialog::on_infoFontBox_activated(int index)
{
    QString infoFont = ui->infoFontBox->currentText();
    setting.setValue("InfoFont", infoFont);
}
