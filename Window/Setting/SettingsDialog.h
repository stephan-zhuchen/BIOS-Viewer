#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include "Start/StartWindow.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() override;

    void setParentWidget(StartWindow *pWidget);

private slots:
    void hexFontSizeBoxActivated(int index);
    void hexFontBoxActivated(int index);
    void asciiFontBoxActivated(int index);
    void BiosViewerThemeBoxActivated(int index);
    void BiosViewerFontSizeBoxActivated(int index);
    void BiosViewerFontBoxActivated(int index);
    void buttonBoxAccepted();
    void lineSpacingBoxActivated(int index);
    void infoFontSizeBoxActivated(int index);
    void infoLineSpacingBoxActivated(int index);
    void infoFontBoxActivated(int index);
    void showPaddingBoxStateChanged(int state);
    void enableEditingBoxStateChanged(int state);
    void onlyHexViewBoxStateChanged(int state);
    void pasteModeBoxActivated(int index);

private:
    Ui::SettingsDialog *ui;
    QSettings setting{"Intel", "BiosViewer"};
    StartWindow *parentWidget{};

    struct _CurrentSettings {
        QString Theme;
        QString BiosViewerFontSize;
        QString BiosViewerFont;
        QString ShowPaddingItem;
        QString InfoFontSize;
        QString InfoFont;
        QString InfoLineSpacing;
        QString HexFontSize;
        QString HexFont;
        QString AsciiFont;
        QString LineSpacing;
        QString EnableHexEditing;
        QString DisableBiosViewer;
        QString PasteMode;
    } CurrentSettings;
    static int lastTabIndex;
};

#endif // SETTINGSDIALOG_H
