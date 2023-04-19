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

private slots:
    void hexFontSizeBoxActivated(int index);
    void hexFontBoxActivated(int index);
    void biosViewerThemeBoxActivated(int index);
    void biosViewerFontSizeBoxActivated(int index);
    void biosViewerFontBoxActivated(int index);
    void buttonBoxAccepted();
    void lineSpacingBoxActivated(int index);
    void infoFontSizeBoxActivated(int index);
    void infoLineSpacingBoxActivated(int index);
    void infoFontBoxActivated(int index);
    void showPaddingBoxStateChanged(int state);
    void enableEditingBoxStateChanged(int state);

private:
    Ui::SettingsDialog *ui;
    QSettings setting{"Intel", "BiosViewer"};
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
    QString ShowPaddingItem;
    QString EnableHexEditing;
    static int lastTabIndex;
};

#endif // SETTINGSDIALOG_H
