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
    void showFitTable();
    void showMicrocodeTable();
    void showAcmTable();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void on_microcodeListWidget_itemClicked(QListWidgetItem *item);

    void on_acmListWidget_itemClicked(QListWidgetItem *item);

private:
    Ui::InfoWindow *ui;
    BiosImageVolume *BiosImage;
    QSettings setting{"./Setting.ini", QSettings::IniFormat};

    enum tableColNum {Address=0, Size, Version, C_V, Checksum, Type};
};

#endif // INFOWINDOW_H
