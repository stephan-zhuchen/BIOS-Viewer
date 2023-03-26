#ifndef INFOWINDOW_H
#define INFOWINDOW_H

#include <QWidget>
#include <QSettings>
#include <QTableWidgetItem>
#include <QListWidgetItem>
#include "lib/UefiLib.h"

using UefiSpace::BiosImageVolume;

namespace Ui {
class InfoWindow;
}

class InfoWindow : public QWidget
{
    Q_OBJECT

public:
    explicit InfoWindow(QWidget *parent = nullptr);
    ~InfoWindow();

    void setBiosImage(BiosImageVolume *Image);
    void setParentWidget(QWidget *pWidget);
    void showFitTab();
    void showMicrocodeTab();
    void showAcmTab();
    void showBtgTab();
    void showFlashmapTab();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_microcodeListWidget_itemSelectionChanged();
    void on_acmListWidget_itemSelectionChanged();
    void on_BtgListWidget_itemSelectionChanged();

private:
    Ui::InfoWindow   *ui;
    BiosImageVolume  *BiosImage;
    QWidget          *parentWidget;
    QSettings setting{"./Setting.ini", QSettings::IniFormat};

    enum tableColNum {Address=0, Size, Version, C_V, Checksum, Type};
};

#endif // INFOWINDOW_H
