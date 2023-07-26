#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include "StartWindow.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    void setParentWidget(StartWindow *pWidget);

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
    void enableMultiThreadStateChanged(int state);
    void enableEditingBoxStateChanged(int state);
    void onlyHexViewBoxStateChanged(int state);
    void pasteModeBoxActivated(int index);

private:
    Ui::SettingsDialog *ui;
    QSettings setting{"Intel", "BiosViewer"};
    StartWindow *parentWidget{};

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
    QString EnableMultiThread;
    QString EnableHexEditing;
    QString DisableBiosViewer;
    QString PasteMode;
    static int lastTabIndex;
};

#endif // SETTINGSDIALOG_H
