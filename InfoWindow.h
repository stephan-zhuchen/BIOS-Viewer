#ifndef INFOWINDOW_H
#define INFOWINDOW_H

#include <QWidget>
#include <QSettings>
#include <QTableWidgetItem>
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

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::InfoWindow *ui;
    BiosImageVolume *BiosImage;
    QSettings setting{"./Setting.ini", QSettings::IniFormat};

    enum tableColNum {Address=0, Size, Version, C_V, Checksum, Type};
};

#endif // INFOWINDOW_H
