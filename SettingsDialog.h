#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include "mainwindow.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    void setParentWidget(MainWindow *pWidget);

protected:
    void paintEvent(QPaintEvent *) override;

private slots:
    void on_hexFontSizeBox_activated(int index);
    void on_hexFontBox_activated(int index);
    void on_biosViewerThemeBox_activated(int index);
    void on_biosViewerFontSizeBox_activated(int index);
    void on_biosViewerFontBox_activated(int index);
    void on_buttonBox_accepted();
    void on_lineSpacingBox_activated(int index);
    void on_infoFontSizeBox_activated(int index);
    void on_infoLineSpacingBox_activated(int index);
    void on_infoFontBox_activated(int index);

private:
    Ui::SettingsDialog *ui;
    QSettings setting{"./Setting.ini", QSettings::IniFormat};
    MainWindow *parentWidget;

    QString StructureFontSize;
    QString StructureFont;
    QString InfoFontSize;
    QString InfoFont;
    QString Theme;
    QString HexFontSize;
    QString HexFont;
    QString LineSpacing;
    QString InfoLineSpacing;
    static int lastTabIndex;
};

#endif // SETTINGSDIALOG_H
